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

#include <mpd/stats.h>
#include <mpd/pair.h>
#include <mpd/send.h>
#include "internal.h"

#include <stdlib.h>
#include <string.h>

struct mpd_stats {
	unsigned number_of_artists;
	unsigned number_of_albums;
	unsigned number_of_songs;
	unsigned long uptime;
	unsigned long db_update_time;
	unsigned long play_time;
	unsigned long db_play_time;
};

bool
mpd_send_stats(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "stats", NULL);
}

struct mpd_stats *
mpd_recv_stats(struct mpd_connection *connection)
{
	struct mpd_stats * stats;
	struct mpd_pair *pair;

	assert(connection != NULL);

	if (mpd_error_is_defined(&connection->error))
		return NULL;

	stats = malloc(sizeof(struct mpd_stats));
	stats->number_of_artists = 0;
	stats->number_of_albums = 0;
	stats->number_of_songs = 0;
	stats->uptime = 0;
	stats->db_update_time = 0;
	stats->play_time = 0;
	stats->db_play_time = 0;

	while ((pair = mpd_recv_pair(connection)) != NULL) {
		if (strcmp(pair->name, "artists") == 0) {
			stats->number_of_artists = atoi(pair->value);
		}
		else if (strcmp(pair->name, "albums") == 0) {
			stats->number_of_albums = atoi(pair->value);
		}
		else if (strcmp(pair->name, "songs") == 0) {
			stats->number_of_songs = atoi(pair->value);
		}
		else if (strcmp(pair->name, "uptime") == 0) {
			stats->uptime = strtol(pair->value,NULL,10);
		}
		else if (strcmp(pair->name, "db_update") == 0) {
			stats->db_update_time = strtol(pair->value,NULL,10);
		}
		else if (strcmp(pair->name, "playtime") == 0) {
			stats->play_time = strtol(pair->value,NULL,10);
		}
		else if (strcmp(pair->name, "db_playtime") == 0) {
			stats->db_play_time = strtol(pair->value,NULL,10);
		}

		mpd_pair_free(pair);
	}

	if (mpd_error_is_defined(&connection->error)) {
		free(stats);
		return NULL;
	}

	return stats;
}


struct mpd_stats *
mpd_get_stats(struct mpd_connection * connection)
{
	return mpd_send_stats(connection)
		? mpd_recv_stats(connection)
		: NULL;
}

void mpd_stats_free(struct mpd_stats * stats) {
	assert(stats != NULL);

	free(stats);
}

unsigned
mpd_stats_get_number_of_artists(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->number_of_artists;
}

unsigned
mpd_stats_get_number_of_albums(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->number_of_albums;
}

unsigned
mpd_stats_get_number_of_songs(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->number_of_songs;
}

unsigned long mpd_stats_get_uptime(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->uptime;
}

unsigned long mpd_stats_get_db_update_time(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->db_update_time;
}

unsigned long mpd_stats_get_play_time(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->play_time;
}

unsigned long mpd_stats_get_db_play_time(struct mpd_stats * stats)
{
	assert(stats != NULL);

	return stats->db_play_time;
}
