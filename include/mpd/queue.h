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
 * Sends the "playlistinfo" command: list all songs in the queue
 * including meta information.
 */
bool
mpd_send_list_queue_meta(struct mpd_connection *connection);

/**
 * Requests information (including tags) about one song in the
 * playlist (command "playlistid").
 *
 * @param connection the connection to MPD
 * @param pos the position of the requested song
 */
bool
mpd_send_get_queue_song_pos(struct mpd_connection *connection, unsigned pos);

/**
 * Requests information (including tags) about one song in the
 * playlist (command "playlistid").
 *
 * @param connection the connection to MPD
 * @param id the id of the requested song
 */
bool
mpd_send_get_queue_song_id(struct mpd_connection *connection, unsigned id);

/**
 * Request the queue changes from MPD since the specified version.
 */
bool
mpd_send_plchanges(struct mpd_connection *connection, unsigned version);

/**
 * A more bandwidth efficient version of the mpd_send_plchanges.
 * It only returns the pos+id of the changes song.
 *
 * @param connection A valid and connected mpd_connection.
 * @param version The playlist version you want the diff with.
 */
bool
mpd_send_plchangesposid(struct mpd_connection *connection, unsigned version);

bool
mpd_send_add(struct mpd_connection *connection, const char *file);

bool
mpd_send_add_id(struct mpd_connection *connection, const char *file);

/**
 * Returns the id of the new song in the playlist.  To be called after
 * mpd_send_add_id().
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
mpd_run_add_id(struct mpd_connection *connection, const char *file);

bool
mpd_send_delete(struct mpd_connection *connection, unsigned pos);

bool
mpd_send_delete_id(struct mpd_connection *connection, unsigned id);

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
mpd_send_move(struct mpd_connection *connection, unsigned from, unsigned to);

bool
mpd_run_move(struct mpd_connection *connection, unsigned from, unsigned to);

bool
mpd_send_move_id(struct mpd_connection *connection, unsigned from, unsigned to);

bool
mpd_run_move_id(struct mpd_connection *connection, unsigned from, unsigned to);

bool
mpd_send_swap(struct mpd_connection *connection, unsigned pos1, unsigned pos2);

bool
mpd_run_swap(struct mpd_connection *connection, unsigned pos1, unsigned pos2);

bool
mpd_send_swap_id(struct mpd_connection *connection, unsigned id1, unsigned id2);

bool
mpd_run_swap_id(struct mpd_connection *connection, unsigned id1, unsigned id2);

#ifdef __cplusplus
}
#endif

#endif
