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

#include <mpd/connection.h>
#include <mpd/async.h>
#include <mpd/parser.h>
#include "resolver.h"
#include "sync.h"
#include "socket.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MPD_WELCOME_MESSAGE	"OK MPD "

static bool
mpd_parse_welcome(struct mpd_connection *connection,
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
		return false;
	}

	tmp = &output[strlen(MPD_WELCOME_MESSAGE)];

	for (i=0;i<3;i++) {
		if (tmp) connection->version[i] = strtol(tmp,&test,10);

		if (!tmp || (test[0] != '.' && test[0] != '\0')) {
			mpd_error_code(&connection->error, MPD_ERROR_NOTMPD);
			mpd_error_printf(&connection->error,
					 "error parsing version number at \"%s\"",
					 &output[strlen(MPD_WELCOME_MESSAGE)]);
			return false;
		}

		tmp = ++test;
	}

	return true;
}

static void
mpd_copy_async_error(struct mpd_error_info *error,
		     const struct mpd_async *async)
{
	assert(!mpd_async_is_alive(async));

	mpd_error_code(error, mpd_async_get_error(async));
	mpd_error_message(error, mpd_async_get_error_message(async));
}

static void
mpd_connection_async_error(struct mpd_connection *connection)
{
	mpd_copy_async_error(&connection->error, connection->async);
}

void
mpd_connection_sync_error(struct mpd_connection *connection)
{
	if (mpd_async_is_alive(connection->async)) {
		/* no error noticed by async: must be a timeout in the
		   sync.c code */
		mpd_error_code(&connection->error, MPD_ERROR_TIMEOUT);
		mpd_error_message(&connection->error, "Timeout");
	} else
		mpd_connection_async_error(connection);
}

struct mpd_connection *
mpd_connection_new(const char *host, int port, float timeout)
{
	const char *line;
	struct mpd_connection *connection = malloc(sizeof(*connection));
	int fd;

	if (connection == NULL)
		return NULL;

	mpd_error_init(&connection->error);
	connection->async = NULL;
	connection->parser = NULL;
	connection->receiving = false;
	connection->sending_command_list = false;
	connection->pair_state = PAIR_STATE_NONE;
	connection->request = NULL;

	if (!mpd_socket_global_init(&connection->error))
		return connection;

	mpd_connection_set_timeout(connection,timeout);

	fd = mpd_socket_connect(host, port, &connection->timeout,
				&connection->error);
	if (fd < 0)
		return connection;

	connection->async = mpd_async_new(fd);
	if (connection->async == NULL) {
		mpd_socket_close(fd);
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return connection;
	}

	connection->parser = mpd_parser_new();
	if (connection->parser == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return connection;
	}

	line = mpd_sync_recv_line(connection->async, &connection->timeout);
	if (line == NULL) {
		mpd_connection_sync_error(connection);
		return connection;
	}

	mpd_parse_welcome(connection, host, port, line);

	return connection;
}

enum mpd_error
mpd_get_error(const struct mpd_connection *connection)
{
	return connection->error.code;
}

const char *
mpd_get_error_message(const struct mpd_connection *connection)
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

bool
mpd_clear_error(struct mpd_connection *connection)
{
	if (mpd_error_is_fatal(&connection->error))
		/* impossible to recover */
		return false;

	mpd_error_clear(&connection->error);
	return true;
}

void mpd_connection_free(struct mpd_connection *connection)
{
	assert(connection->pair_state != PAIR_STATE_FLOATING);

	if (connection->parser != NULL)
		mpd_parser_free(connection->parser);

	if (connection->async != NULL)
		mpd_async_free(connection->async);

	if (connection->pair_state == PAIR_STATE_QUEUED)
		free(connection->pair);

	if (connection->request) free(connection->request);

	mpd_error_deinit(&connection->error);

	free(connection);
}

void
mpd_connection_set_timeout(struct mpd_connection *connection, float timeout)
{
	connection->timeout.tv_sec = (long)timeout;
	connection->timeout.tv_usec = ((long)(timeout * 1e6)) % 1000000;
}

const unsigned *
mpd_get_server_version(const struct mpd_connection *connection)
{
	return connection->version;
}

int
mpd_cmp_server_version(const struct mpd_connection *connection, unsigned major,
		       unsigned minor, unsigned patch)
{
	const unsigned *v = connection->version;

	if (v[0] > major || (v[0] == major &&
			     (v[1] > minor || (v[1] == minor &&
					       v[2] > patch))))
		return 1;
	else if (v[0] == major && v[1] == minor && v[2] == patch)
		return 0;
	else
		return -1;
}
