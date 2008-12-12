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

static int do_connect_fail(struct mpd_connection *connection,
                           const struct sockaddr *serv_addr, int addrlen)
{
	int iMode = 1; /* 0 = blocking, else non-blocking */
	if (connect(connection->sock, serv_addr, addrlen) == SOCKET_ERROR)
		return 1;
	ioctlsocket(connection->sock, FIONBIO, (u_long FAR*) &iMode);
	return 0;
}
#else /* !WIN32 (sane operating systems) */
static int do_connect_fail(struct mpd_connection *connection,
                           const struct sockaddr *serv_addr, int addrlen)
{
	int flags;
	if (connect(connection->sock, serv_addr, addrlen) < 0)
		return 1;
	flags = fcntl(connection->sock, F_GETFL, 0);
	fcntl(connection->sock, F_SETFL, flags | O_NONBLOCK);
	return 0;
}
#endif /* !WIN32 */

/**
 * Wait for the socket to become readable.
 */
static int mpd_wait(struct mpd_connection *connection)
{
	struct timeval tv;
	fd_set fds;
	int ret;

	assert(connection->sock >= 0);

	while (1) {
		tv = connection->timeout;
		FD_ZERO(&fds);
		FD_SET(connection->sock, &fds);

		ret = select(connection->sock + 1, &fds, NULL, NULL, &tv);
		if (ret > 0)
			return 0;

		if (ret == 0 || !SELECT_ERRNO_IGNORE)
			return -1;
	}
}

/**
 * Wait until the socket is connected and check its result.  Returns 1
 * on success, 0 on timeout, -errno on error.
 */
static int mpd_wait_connected(struct mpd_connection *connection)
{
	int ret;
	int s_err = 0;
	socklen_t s_err_size = sizeof(s_err);

	ret = mpd_wait(connection);
	if (ret < 0)
		return 0;

	ret = getsockopt(connection->sock, SOL_SOCKET, SO_ERROR,
			 (char*)&s_err, &s_err_size);
	if (ret < 0)
		return -errno;

	if (s_err != 0)
		return -s_err;

	return 1;
}

static int
mpd_connect(struct mpd_connection *connection, const char * host, int port)
{
	struct resolver *resolver;
	const struct resolver_address *address;
	int ret;

	resolver = resolver_new(host, port);
	if (resolver == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_UNKHOST);
		mpd_error_printf(&connection->error,
				 "host \"%s\" not found", host);
		return -1;
	}

	while ((address = resolver_next(resolver)) != NULL) {
		connection->sock = socket(address->family, SOCK_STREAM,
					  address->protocol);
		if (connection->sock < 0) {
			mpd_error_clear(&connection->error);
			mpd_error_code(&connection->error, MPD_ERROR_SYSTEM);
			mpd_error_printf(&connection->error,
					 "problems creating socket: %s",
					 strerror(errno));
			continue;
		}

		ret = do_connect_fail(connection,
				      address->addr, address->addrlen);
		if (ret != 0) {
			mpd_error_clear(&connection->error);
			mpd_error_code(&connection->error, MPD_ERROR_CONNPORT);
			mpd_error_printf(&connection->error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(errno));

			closesocket(connection->sock);
			connection->sock = -1;
			continue;
		}

		ret = mpd_wait_connected(connection);
		if (ret > 0) {
			resolver_free(resolver);
			mpd_clearError(connection);
			return 0;
		}

		if (ret == 0) {
			mpd_error_clear(&connection->error);
			mpd_error_code(&connection->error,
				       MPD_ERROR_NORESPONSE);
			mpd_error_printf(&connection->error,
					 "timeout in attempting to get a response from \"%s\" on port %i",
					 host, port);
		} else if (ret < 0) {
			mpd_error_clear(&connection->error);
			mpd_error_code(&connection->error, MPD_ERROR_CONNPORT);
			mpd_error_printf(&connection->error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(-ret));
		}

		closesocket(connection->sock);
		connection->sock = -1;
	}

	resolver_free(resolver);
	return -1;
}

static int
mpd_parseWelcome(struct mpd_connection *connection,
		 const char *host, int port,
		 char *output)
{
	char * tmp;
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
	char * rt;
	struct mpd_connection *connection = malloc(sizeof(*connection));

	connection->sock = -1;
	connection->buflen = 0;
	connection->bufstart = 0;
	mpd_error_init(&connection->error);
	connection->doneProcessing = 0;
	connection->commandList = 0;
	connection->listOks = 0;
	connection->doneListOk = 0;
	connection->returnElement = NULL;
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

	while (!(rt = memchr(connection->buffer, '\n', connection->buflen))) {
		err = mpd_recv(connection);
		if (err < 0)
			return connection;
	}

	*rt = '\0';
	if (mpd_parseWelcome(connection, host, port, connection->buffer) == 0)
		connection->doneProcessing = 1;

	connection->buflen -= rt + 1 - connection->buffer;
	memmove(connection->buffer, rt + 1, connection->buflen);

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
	closesocket(connection->sock);
	if (connection->returnElement) free(connection->returnElement);
	if (connection->request) free(connection->request);

	mpd_error_deinit(&connection->error);

	free(connection);
	WSACleanup();
}

void
mpd_setConnectionTimeout(struct mpd_connection *connection, float timeout)
{
	connection->timeout.tv_sec = (int)timeout;
	connection->timeout.tv_usec = (int)(timeout*1e6 -
	                                    connection->timeout.tv_sec*1000000 +
					    0.5);
}

const int *
mpd_get_server_version(const struct mpd_connection *connection)
{
	return connection->version;
}

/**
 * Attempt to read data from the socket into the input buffer.
 * Returns 0 on success, -1 on error.
 */
int mpd_recv(struct mpd_connection *connection)
{
	int ret;
	ssize_t nbytes;

	assert(connection != NULL);
	assert(connection->buflen <= sizeof(connection->buffer));
	assert(connection->bufstart <= connection->buflen);

	if (connection->sock < 0) {
		mpd_error_code(&connection->error, MPD_ERROR_CONNCLOSED);
		mpd_error_message(&connection->error, "not connected");
		connection->doneProcessing = 1;
		connection->doneListOk = 0;
		return -1;
	}

	if (connection->buflen >= sizeof(connection->buffer)) {
		/* delete consumed data from beginning of buffer */
		connection->buflen -= connection->bufstart;
		memmove(connection->buffer,
			connection->buffer + connection->bufstart,
			connection->buflen);
		connection->bufstart = 0;
	}

	if (connection->buflen >= sizeof(connection->buffer)) {
		mpd_error_code(&connection->error, MPD_ERROR_BUFFEROVERRUN);
		mpd_error_message(&connection->error, "buffer overrun");
		connection->doneProcessing = 1;
		connection->doneListOk = 0;
		return -1;
	}

	while (1) {
		ret = mpd_wait(connection);
		if (ret < 0) {
			mpd_error_code(&connection->error, MPD_ERROR_TIMEOUT);
			mpd_error_message(&connection->error,
					  "connection timeout");
			connection->doneProcessing = 1;
			connection->doneListOk = 0;
			return -1;
		}

		nbytes = read(connection->sock,
			      connection->buffer + connection->buflen,
			      sizeof(connection->buffer) - connection->buflen);
		if (nbytes > 0) {
			connection->buflen += nbytes;
			return 0;
		}

		if (nbytes == 0 || !SENDRECV_ERRNO_IGNORE) {
			mpd_error_code(&connection->error,
				       MPD_ERROR_CONNCLOSED);
			mpd_error_message(&connection->error,
					  "connection closed");
			connection->doneProcessing = 1;
			connection->doneListOk = 0;
			return -1;
		}
	}
}

void
mpd_executeCommand(struct mpd_connection *connection, const char *command)
{
	int ret;
	struct timeval tv;
	fd_set fds;
	const char *commandPtr = command;
	int commandLen = strlen(command);

	if (connection->sock < 0) {
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

	FD_ZERO(&fds);
	FD_SET(connection->sock,&fds);
	tv.tv_sec = connection->timeout.tv_sec;
	tv.tv_usec = connection->timeout.tv_usec;

	while ((ret = select(connection->sock+1,NULL,&fds,NULL,&tv)==1) ||
			(ret==-1 && SELECT_ERRNO_IGNORE)) {
		ret = send(connection->sock,commandPtr,commandLen,MSG_DONTWAIT);
		if (ret<=0)
		{
			if (SENDRECV_ERRNO_IGNORE) continue;
			mpd_error_code(&connection->error, MPD_ERROR_SENDING);
			mpd_error_printf(&connection->error,
					 "problems giving command \"%s\"",
					 command);
			return;
		}
		else {
			commandPtr+=ret;
			commandLen-=ret;
		}

		if (commandLen<=0) break;
	}

	if (commandLen>0) {
		mpd_error_code(&connection->error, MPD_ERROR_TIMEOUT);
		mpd_error_printf(&connection->error,
				 "timeout sending command \"%s\"", command);
		return;
	}

	if (!connection->commandList)
		connection->doneProcessing = 0;
	else if (connection->commandList == COMMAND_LIST_OK)
		connection->listOks++;
}

void mpd_getNextReturnElement(struct mpd_connection *connection)
{
	char * output = NULL;
	char * rt = NULL;
	char * name = NULL;
	char * value = NULL;
	char * tok = NULL;
	int err;
	int pos;

	if (connection->returnElement) mpd_freeReturnElement(connection->returnElement);
	connection->returnElement = NULL;

	if (connection->doneProcessing ||
	    (connection->listOks && connection->doneListOk)) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already done processing current command");
		return;
	}

	while (!(rt = memchr(connection->buffer + connection->bufstart, '\n',
			     connection->buflen - connection->bufstart))) {
		err = mpd_recv(connection);
		if (err < 0)
			return;
	}

	*rt = '\0';
	output = connection->buffer+connection->bufstart;
	connection->bufstart = rt - connection->buffer + 1;

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
		connection->returnElement = mpd_newReturnElement(name,&(value[1]));
	}
	else {
		mpd_error_code(&connection->error, MPD_ERROR_MALFORMED);
		mpd_error_printf(&connection->error,
				 "error parsing: %s:%s", name, value);
	}
}

