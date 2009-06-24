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

#ifndef MPD_RESPONSE_H
#define MPD_RESPONSE_H

#include <stdbool.h>

struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Finishes the response and checks if the command was successful.  If
 * there are data pairs left, they are discarded.
 *
 * @return true on success, false on error
 */
bool
mpd_response_finish(struct mpd_connection *connection);

/**
 * Finishes the response of the current list command.  If there are
 * data pairs left, they are discarded.
 *
 * @return true on success, false on error
 */
bool
mpd_response_next(struct mpd_connection *connection);

/**
 * Returns the id of the new song in the playlist.  To be called after
 * mpd_send_addid().
 *
 * @return the new song id, -1 on error or if MPD did not send an id
 */
int
mpd_recv_song_id(struct mpd_connection *connection);

/**
 * Receives the id the of the update job which was submitted by
 * mpd_send_update().
 */
int
mpd_recv_update_id(struct mpd_connection *connection);

/**
 * Receives the next supported command.  Call this in a loop after
 * mpd_send_commands() or mpd_send_notcommands().
 *
 * @param connection a #mpd_connection
 * @returns a string, needs to be free'ed
 */
char *
mpd_recv_command_name(struct mpd_connection *connection);

/**
 * Receives one line of the mpd_send_urlhandlers() response.
 */
char *
mpd_recv_handler(struct mpd_connection *connection);

/**
 * Receives the next tag type name.  Call this in a loop after
 * mpd_send_tagtypes().
 */
char *
mpd_recv_tag_type_name(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif