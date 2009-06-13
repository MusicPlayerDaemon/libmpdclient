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

#include <mpd/client.h>
#include "internal.h"
#include "resolver.h"
#include "str_pool.h"

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>

void mpd_finishCommand(struct mpd_connection *connection)
{
	struct mpd_pair *pair;

	while (connection->receiving) {
		if (connection->doneListOk) connection->doneListOk = 0;

		pair = mpd_recv_pair(connection);
		if (pair != NULL)
			mpd_pair_free(pair);
	}
}

static void mpd_finishListOkCommand(struct mpd_connection *connection)
{
	while (connection->receiving && connection->listOks &&
			!connection->doneListOk)
	{
		struct mpd_pair *pair = mpd_recv_pair(connection);
		if (pair != NULL)
			mpd_pair_free(pair);
	}
}

int mpd_nextListOkCommand(struct mpd_connection *connection)
{
	mpd_finishListOkCommand(connection);
	if (connection->receiving)
		connection->doneListOk = 0;
	if (connection->listOks == 0 || !connection->receiving)
		return -1;
	return 0;
}

int
mpd_sendAddIdCommand(struct mpd_connection *connection, const char *file)
{
	bool ret;
	struct mpd_pair *pair;
	int retval = -1;

	ret = mpd_send_addid(connection, file);
	if (!ret)
		return -1;

	pair = mpd_recv_pair_named(connection, "Id");
	if (pair != NULL) {
		retval = atoi(pair->value);
		mpd_pair_free(pair);
	}
	
	return retval;
}

int mpd_getUpdateId(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	int ret = 0;

	pair = mpd_recv_pair_named(connection, "updating_db");
	if (pair != NULL) {
		ret = atoi(pair->value);
		mpd_pair_free(pair);
	}

	return ret;
}

void mpd_sendCommandListBegin(struct mpd_connection *connection)
{
	if (connection->sending_command_list) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already in command list mode");
		return;
	}

	connection->sending_command_list = true;
	connection->sending_command_list_ok = false;
	mpd_send_command(connection, "command_list_begin", NULL);
}

void mpd_sendCommandListOkBegin(struct mpd_connection *connection)
{
	if (connection->sending_command_list) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already in command list mode");
		return;
	}

	connection->sending_command_list = true;
	connection->sending_command_list_ok = true;
	mpd_send_command(connection, "command_list_ok_begin", NULL);
	connection->listOks = 0;
}

void mpd_sendCommandListEnd(struct mpd_connection *connection)
{
	if (!connection->sending_command_list) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "not in command list mode");
		return;
	}

	connection->sending_command_list = false;
	mpd_send_command(connection, "command_list_end", NULL);
}

/**
 * Get the next returned command
 */
char * mpd_get_next_command(struct mpd_connection *connection)
{
	return mpd_recv_value_named(connection, "command");
}

char * mpd_get_next_handler(struct mpd_connection *connection)
{
	return mpd_recv_value_named(connection, "handler");
}

char * mpd_get_next_tag_type(struct mpd_connection *connection)
{
	return mpd_recv_value_named(connection, "tagtype");
}

