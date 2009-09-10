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

#include <mpd/playlist.h>
#include <mpd/send.h>
#include <mpd/recv.h>
#include <mpd/pair.h>
#include <mpd/response.h>
#include "isend.h"
#include "run.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

bool
mpd_send_playlistinfo(struct mpd_connection *connection, int song_pos)
{
	return mpd_send_int_command(connection, "playlistinfo", song_pos);
}

bool
mpd_send_playlistid(struct mpd_connection *connection, int id)
{
	return mpd_send_int_command(connection, "playlistid", id);
}

bool
mpd_send_plchanges(struct mpd_connection *connection, long long playlist)
{
	return mpd_send_ll_command(connection, "plchanges", playlist);
}

bool
mpd_send_plchangesposid(struct mpd_connection *connection, long long playlist)
{
	return mpd_send_ll_command(connection, "plchangesposid", playlist);
}

bool
mpd_send_add(struct mpd_connection *connection, const char *file)
{
	return mpd_send_command(connection, "add", file, NULL);
}

bool
mpd_send_addid(struct mpd_connection *connection, const char *file)
{
	return mpd_send_command(connection, "addid", file, NULL);
}

int
mpd_recv_song_id(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	int id = -1;

	pair = mpd_recv_pair_named(connection, "Id");
	if (pair != NULL) {
		id = atoi(pair->value);
		mpd_return_pair(connection, pair);
	}

	return id;
}

int
mpd_run_addid(struct mpd_connection *connection, const char *file)
{
	int id;

	if (!mpd_run_check(connection))
		return false;

	if (!mpd_send_addid(connection, file))
		return -1;

	id = mpd_recv_song_id(connection);

	if (!mpd_response_finish(connection))
		id = -1;

	return id;
}

bool
mpd_send_delete(struct mpd_connection *connection, int song_pos)
{
	return mpd_send_int_command(connection, "delete", song_pos);
}

bool
mpd_send_deleteid(struct mpd_connection *connection, int id)
{
	return mpd_send_int_command(connection, "deleteid", id);
}

bool
mpd_send_shuffle(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "shuffle", NULL);
}

bool
mpd_send_shuffle_range(struct mpd_connection *connection, unsigned start, unsigned end)
{
	return mpd_send_range_command(connection, "shuffle", start, end);
}

bool
mpd_send_clear(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "clear", NULL);
}

bool
mpd_send_move(struct mpd_connection *connection, int from, int to)
{
	return mpd_send_int2_command(connection, "move", from, to);
}

bool
mpd_send_moveid(struct mpd_connection *connection, int from, int to)
{
	return mpd_send_int2_command(connection, "moveid", from, to);
}

bool
mpd_send_swap(struct mpd_connection *connection, int pos1, int pos2)
{
	return mpd_send_int2_command(connection, "swap", pos1, pos2);
}

bool
mpd_send_swapid(struct mpd_connection *connection, int id1, int id2)
{
	return mpd_send_int2_command(connection, "swapid", id1, id2);
}
