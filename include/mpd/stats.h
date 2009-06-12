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

#ifndef MPD_STATS_H
#define MPD_STATS_H

#include <mpd/connection.h>

struct mpd_stats;

struct mpd_search_stats;

#ifdef __cplusplus
extern "C" {
#endif

void mpd_send_stats(struct mpd_connection * connection);

struct mpd_stats * mpd_get_stats(struct mpd_connection * connection);

void mpd_stats_free(struct mpd_stats * stats);

struct mpd_search_stats * mpd_get_search_stats(struct mpd_connection * connection);

void mpd_stats_search_free(struct mpd_search_stats * stats);

int mpd_stats_get_number_of_artists(struct mpd_stats * stats);

int mpd_stats_get_number_of_albums(struct mpd_stats * stats);

int mpd_stats_get_number_of_songs(struct mpd_stats * stats);

unsigned long mpd_stats_get_uptime(struct mpd_stats * stats);

unsigned long mpd_stats_get_db_update_time(struct mpd_stats * stats);

unsigned long mpd_stats_get_play_time(struct mpd_stats * stats);

unsigned long mpd_stats_get_db_play_time(struct mpd_stats * stats);

int mpd_search_stats_get_number_of_songs(struct mpd_search_stats * stats);

unsigned long mpd_search_stats_get_play_time(struct mpd_search_stats * stats);

#ifdef __cplusplus
}
#endif

#endif
