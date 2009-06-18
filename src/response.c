/* libmpdclient
   (c) 2003-2009 The Music Player Daemon Project
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

#include <mpd/response.h>
#include <mpd/connection.h>
#include <mpd/pair.h>
#include "internal.h"

#include <assert.h>
#include <stdlib.h>

void
mpd_response_finish(struct mpd_connection *connection)
{
	struct mpd_pair *pair;

	while (connection->receiving) {
		assert(!mpd_error_is_defined(&connection->error));

		if (connection->doneListOk) connection->doneListOk = 0;

		pair = mpd_recv_pair(connection);
		assert(pair != NULL || !connection->receiving ||
		       mpd_error_is_defined(&connection->error));

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

int
mpd_response_next(struct mpd_connection *connection)
{
	mpd_finishListOkCommand(connection);
	if (connection->receiving)
		connection->doneListOk = 0;
	if (connection->listOks == 0 || !connection->receiving)
		return -1;
	return 0;
}

int
mpd_get_update_id(struct mpd_connection *connection)
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
