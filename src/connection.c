/* libmpdclient
   (c) 2003-2008 The Music Player Daemon Project
   This project's homepage is: http://www.musicpd.org

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Music Player Daemon nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "internal.h"
#include <mpd/connection.h>
#include <mpd/idle.h>
#include <mpd/pair.h>
#include "resolver.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef WIN32
#  include <ws2tcpip.h>
#  include <winsock.h>
#else
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <sys/un.h>
#endif

#ifndef MSG_DONTWAIT
#  define MSG_DONTWAIT 0
#endif

#ifdef WIN32
#  define SELECT_ERRNO_IGNORE   (errno == WSAEINTR || errno == WSAEINPROGRESS)
#  define SENDRECV_ERRNO_IGNORE SELECT_ERRNO_IGNORE
#else
#  define SELECT_ERRNO_IGNORE   (errno == EINTR)
#  define SENDRECV_ERRNO_IGNORE (errno == EINTR || errno == EAGAIN)
#  define winsock_dll_error(c)  0
#  define closesocket(s)        close(s)
#  define WSACleanup()          do { /* nothing */ } while (0)
#endif

#define MPD_WELCOME_MESSAGE	"OK MPD "

#define MPD_ERROR_AT_UNK	-1

#ifdef WIN32
static int winsock_dll_error(struct mpd_connection *connection)
{
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0 ||
			LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2 ) {
		snprintf(connection->errorStr, sizeof(connection->errorStr),
			 "Could not find usable WinSock DLL.");
		connection->error = MPD_ERROR_SYSTEM;
		return 1;
	}
	return 0;
}
#endif /* !WIN32 */

static int
mpd_connect(struct mpd_connection *connection, const char * host, int port)
{
	bool ret;

	ret = mpd_socket_connect(&connection->socket, host, port,
				 &connection->error);
	if (!ret)
		return -1;

	return 0;
}

static int
mpd_parseWelcome(struct mpd_connection *connection,
		 const char *host, int port,
		 const char *output)
{
	const char *tmp;
	char * test;
	int i;

	if (strncmp(output,MPD_WELCOME_MESSAGE,strlen(MPD_WELCOME_MESSAGE))) {
		mpd_error_code(&connection->error, MPD_ERROR_NOTMPD);
		mpd_error_printf(&connection->error,
				 "mpd not running on port %i on host \"%s\"",
				 port, host);
		return 1;
	}

	tmp = &output[strlen(MPD_WELCOME_MESSAGE)];

	for (i=0;i<3;i++) {
		if (tmp) connection->version[i] = strtol(tmp,&test,10);

		if (!tmp || (test[0] != '.' && test[0] != '\0')) {
			mpd_error_code(&connection->error, MPD_ERROR_NOTMPD);
			mpd_error_printf(&connection->error,
					 "error parsing version number at \"%s\"",
					 &output[strlen(MPD_WELCOME_MESSAGE)]);
			return 1;
		}
		tmp = ++test;
	}

	return 0;
}

struct mpd_connection *
mpd_newConnection(const char *host, int port, float timeout)
{
	int err;
	const char *line;
	struct mpd_connection *connection = malloc(sizeof(*connection));
	const struct timeval tv = {
		.tv_sec = (long)timeout,
		.tv_usec = ((long)(timeout * 1e6)) % 1000000,
	};

	mpd_socket_init(&connection->socket, &tv);
	mpd_error_init(&connection->error);
	connection->doneProcessing = 0;
	connection->commandList = 0;
	connection->listOks = 0;
	connection->doneListOk = 0;
	connection->pair = NULL;
	connection->request = NULL;
#ifdef MPD_GLIB
	connection->source_id = 0;
#endif
	connection->idle = 0;
	connection->startIdle = NULL;
	connection->stopIdle = NULL;
	connection->notify_cb = NULL;

	if (winsock_dll_error(connection))
		return connection;

	mpd_setConnectionTimeout(connection,timeout);

	err = mpd_connect(connection, host, port);
	if (err < 0)
		return connection;

	line = mpd_socket_recv_line(&connection->socket, &connection->error);
	if (line == NULL)
		return connection;

	if (mpd_parseWelcome(connection, host, port, line) == 0)
		connection->doneProcessing = 1;

	return connection;
}

enum mpd_error
mpd_get_error(const struct mpd_connection *connection)
{
	return connection->error.code;
}

const char *
mpd_get_error_string(const struct mpd_connection *connection)
{
	assert(connection->error.code != MPD_ERROR_SUCCESS);
	assert(connection->error.message != NULL ||
	       connection->error.code == MPD_ERROR_OOM);

	if (connection->error.message == NULL)
		return "Out of memory";

	return connection->error.message;
}

enum mpd_ack
mpd_get_server_error(const struct mpd_connection *connection)
{
	assert(connection->error.code == MPD_ERROR_ACK);

	return connection->error.ack;
}

void mpd_clearError(struct mpd_connection *connection)
{
	mpd_error_clear(&connection->error);
}

void mpd_closeConnection(struct mpd_connection *connection)
{
	mpd_socket_deinit(&connection->socket);

	if (connection->pair != NULL)
		free(connection->pair);

	if (connection->request) free(connection->request);

	mpd_error_deinit(&connection->error);

	free(connection);
	WSACleanup();
}

void
mpd_setConnectionTimeout(struct mpd_connection *connection, float timeout)
{
	const struct timeval tv = {
		.tv_sec = (long)timeout,
		.tv_usec = ((long)(timeout * 1e6)) % 1000000,
	};

	mpd_socket_set_timeout(&connection->socket, &tv);
}

const int *
mpd_get_server_version(const struct mpd_connection *connection)
{
	return connection->version;
}

void
mpd_executeCommand(struct mpd_connection *connection, const char *command)
{
	bool ret;

	if (!mpd_socket_defined(&connection->socket)) {
		mpd_error_code(&connection->error, MPD_ERROR_CONNCLOSED);
		mpd_error_message(&connection->error, "connection closed");
		return;
	}

	if (connection->idle)
		mpd_stopIdle(connection);

	if (!connection->doneProcessing && !connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "not done processing current command");
		return;
	}

	mpd_clearError(connection);

	ret = mpd_socket_send(&connection->socket, command, strlen(command),
			      &connection->error);
	if (!ret)
		return;

	if (!connection->commandList)
		connection->doneProcessing = 0;
	else if (connection->commandList == COMMAND_LIST_OK)
		connection->listOks++;
}

void mpd_getNextReturnElement(struct mpd_connection *connection)
{
	char * output = NULL;
	char * name = NULL;
	char * value = NULL;
	char * tok = NULL;
	int pos;

	if (connection->pair != NULL) {
		mpd_pair_free(connection->pair);
		connection->pair = NULL;
	}

	if (connection->doneProcessing ||
	    (connection->listOks && connection->doneListOk)) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already done processing current command");
		return;
	}

	output = mpd_socket_recv_line(&connection->socket, &connection->error);
	if (output == NULL)
		return;

	if (strcmp(output, "OK")==0) {
		if (connection->listOks > 0) {
			mpd_error_code(&connection->error,
				       MPD_ERROR_MALFORMED);
			mpd_error_message(&connection->error,
					  "expected more list_OK's");
		}
		connection->listOks = 0;
		connection->doneProcessing = 1;
		connection->doneListOk = 0;
		return;
	} else if (strcmp(output, "list_OK") == 0) {
		if (!connection->listOks) {
			mpd_error_code(&connection->error,
				       MPD_ERROR_MALFORMED);
			mpd_error_message(&connection->error,
					  "got an unexpected list_OK");
		}
		else {
			connection->doneListOk = 1;
			connection->listOks--;
		}
		return;
	} else if (strncmp(output, "ACK", strlen("ACK"))==0) {
		size_t length = strlen(output);
		char * test;
		char * needle;
		int val;

		mpd_error_ack(&connection->error,
			      MPD_ACK_ERROR_UNK, MPD_ERROR_AT_UNK);
		mpd_error_message_n(&connection->error, output, length);
		connection->doneProcessing = 1;
		connection->doneListOk = 0;

		needle = strchr(output, '[');
		if (!needle) return;
		val = strtol(needle+1, &test, 10);
		if (*test != '@') return;
		connection->error.ack = val;
		val = strtol(test+1, &test, 10);
		if (*test != ']') return;
		connection->error.at = val;
		return;
	}

	tok = strchr(output, ':');
	if (!tok) return;
	pos = tok - output;
	value = ++tok;
	name = output;
	name[pos] = '\0';

	if (value[0]==' ') {
		connection->pair = mpd_pair_new(name, value + 1);
	}
	else {
		mpd_error_code(&connection->error, MPD_ERROR_MALFORMED);
		mpd_error_printf(&connection->error,
				 "error parsing: %s:%s", name, value);
	}
}

