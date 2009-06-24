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
#include <mpd/send.h>
#include <mpd/connection.h>
#include <mpd/pair.h>
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

enum mpd_idle
mpd_recv_idle(struct mpd_connection *connection)
{
	enum mpd_idle flags = 0;
	struct mpd_pair *pair;

	assert(connection != NULL);

	if (mpd_error_is_defined(&connection->error))
		return 0;

	while ((pair = mpd_recv_pair_named(connection, "changed")) != NULL) {
		for (unsigned i = 0; idle_names[i] != NULL; ++i)
			if (strcmp(pair->value, idle_names[i]) == 0)
				flags |= (1 << i);
		mpd_pair_free(pair);
	}

	return flags;
}

bool
mpd_send_idle(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "idle", NULL);
}

bool
mpd_send_noidle(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "noidle", NULL);
}
