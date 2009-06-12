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

#include <mpd/idle.h>
#include <mpd/pair.h>
#include <mpd/send.h>
#include "internal.h"

#include <string.h>

static const char *const idle_names[] = {
	"database",
	"stored_playlist",
	"playlist",
	"player",
	"mixer",
	"output",
	"options",
	NULL
};

static unsigned
mpd_readChanges(struct mpd_connection *connection)
{
	unsigned i;
	unsigned flags = 0;

	if (connection->pair == NULL)
		mpd_get_next_pair(connection);

	if (connection->error.code == MPD_ERROR_CONNCLOSED)
		return IDLE_DISCONNECT;

	while (connection->pair != NULL) {
		const struct mpd_pair *pair = connection->pair;

		if (pair->name != NULL &&
		    strncmp(pair->name, "changed", strlen("changed")) == 0) {
			for (i = 0; idle_names[i]; ++i) {
				if (strcmp(pair->value, idle_names[i]) == 0) {
					flags |= (1 << i);
				}
			}
		}
		mpd_get_next_pair(connection);
	}

	return flags;
}

void
mpd_startIdle(struct mpd_connection *connection)
{
	if (connection->idle)
		return;

	mpd_send_command(connection, "idle", NULL);
	connection->idle = 1;
}

unsigned
mpd_stopIdle(struct mpd_connection *connection)
{
	connection->idle = 0;
	mpd_send_command(connection, "noidle", NULL);
	return mpd_readChanges(connection);
}
