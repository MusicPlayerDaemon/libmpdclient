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

#ifndef MPD_COMMAND_H
#define MPD_COMMAND_H

#include <stdbool.h>

struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mixer commands
 *
 */

bool
mpd_send_setvol(struct mpd_connection *connection, int change);

/**
 * WARNING: don't use volume command, its deprecated
 */
bool
mpd_send_volume(struct mpd_connection *connection, int change);


/*
 * Connection commands
 *
 */

/**
 * @param connection a #mpd_connection
 *
 * Queries mpd for the allowed commands
 */
bool
mpd_send_commands(struct mpd_connection *connection);

/**
 * @param connection a #mpd_connection
 *
 * Queries mpd for the not allowed commands
 */
bool
mpd_send_notcommands(struct mpd_connection *connection);

bool
mpd_send_urlhandlers(struct mpd_connection *connection);

bool
mpd_send_tagtypes(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
