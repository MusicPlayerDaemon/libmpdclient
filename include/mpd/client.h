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

#ifndef MPD_CLIENT_H
#define MPD_CLIENT_H

#include <mpd/connection.h>
#include <mpd/send.h>
#include <mpd/command.h>
#include <mpd/song.h>
#include <mpd/directory.h>
#include <mpd/pair.h>
#include <mpd/stored_playlist.h>

#ifdef WIN32
#  define __W32API_USE_DLLIMPORT__ 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* INFO COMMANDS AND STUFF */

/* SIMPLE COMMANDS */

int mpd_sendAddIdCommand(struct mpd_connection *connection, const char *file);

/* returns the update job id, call this after a update command*/
int mpd_getUpdateId(struct mpd_connection *connection);

/* after executing a command, when your done with it to get its status
 * (you want to check connection->error for an error)
 */
void mpd_finishCommand(struct mpd_connection *connection);

/* command list stuff, use this to do things like add files very quickly */
void mpd_sendCommandListBegin(struct mpd_connection *connection);

void mpd_sendCommandListOkBegin(struct mpd_connection *connection);

void mpd_sendCommandListEnd(struct mpd_connection *connection);

/* advance to the next listOk
 * returns 0 if advanced to the next list_OK,
 * returns -1 if it advanced to an OK or ACK */
int mpd_nextListOkCommand(struct mpd_connection *connection);

/**
 * @param connection a #mpd_connection
 *
 * returns the next supported command.
 *
 * @returns a string, needs to be free'ed
 */
char *mpd_get_next_command(struct mpd_connection *connection);

char *mpd_get_next_handler(struct mpd_connection *connection);

char *mpd_get_next_tag_type(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
