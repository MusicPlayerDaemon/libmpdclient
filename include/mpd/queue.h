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

#ifndef MPD_QUEUE_H
#define MPD_QUEUE_H

#include <stdbool.h>

struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * song_pos of -1, means to display the whole list
 */
bool
mpd_send_playlistinfo(struct mpd_connection *connection, int song_pos);

/**
 * songId of -1, means to display the whole list
 */
bool
mpd_send_playlistid(struct mpd_connection *connection, int id);

/**
 * use this to get the changes in the playlist since version _playlist_
 */
bool
mpd_send_plchanges(struct mpd_connection *connection, long long playlist);

/**
 * A more bandwidth efficient version of the mpd_send_plchanges.
 * It only returns the pos+id of the changes song.
 *
 * @param connection A valid and connected mpd_connection.
 * @param playlist The playlist version you want the diff with.
 */
bool
mpd_send_plchangesposid(struct mpd_connection *connection, long long playlist);

bool
mpd_send_add(struct mpd_connection *connection, const char *file);

bool
mpd_send_addid(struct mpd_connection *connection, const char *file);

/**
 * Returns the id of the new song in the playlist.  To be called after
 * mpd_send_addid().
 *
 * @return the new song id, -1 on error or if MPD did not send an id
 */
int
mpd_recv_song_id(struct mpd_connection *connection);

/**
 * Executes the "addid" command and reads the response.
 *
 * @return the new song id, -1 on error or if MPD did not send an id
 */
int
mpd_run_addid(struct mpd_connection *connection, const char *file);

bool
mpd_send_delete(struct mpd_connection *connection, int song_pos);

bool
mpd_send_deleteid(struct mpd_connection *connection, int id);

bool
mpd_send_shuffle(struct mpd_connection *connection);

bool
mpd_run_shuffle(struct mpd_connection *connection);

bool
mpd_send_shuffle_range(struct mpd_connection *connection, unsigned start, unsigned end);

bool
mpd_run_shuffle_range(struct mpd_connection *connection,
		      unsigned start, unsigned end);

bool
mpd_send_clear(struct mpd_connection *connection);

bool
mpd_run_clear(struct mpd_connection *connection);

bool
mpd_send_move(struct mpd_connection *connection, int from, int to);

int
mpd_run_move(struct mpd_connection *connection, int from, int to);

bool
mpd_send_moveid(struct mpd_connection *connection, int from, int to);

int
mpd_run_moveid(struct mpd_connection *connection, int from, int to);

bool
mpd_send_swap(struct mpd_connection *connection, int pos1, int pos2);

int
mpd_run_swap(struct mpd_connection *connection, int pos1, int pos2);

bool
mpd_send_swapid(struct mpd_connection *connection, int id1, int id2);

int
mpd_run_swapid(struct mpd_connection *connection, int id1, int id2);

#ifdef __cplusplus
}
#endif

#endif
