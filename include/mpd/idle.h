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

#ifndef MPD_IDLE_H
#define MPD_IDLE_H

struct mpd_connection;

enum {
	/** song database has been updated*/
	IDLE_DATABASE = 0x1,

	/** a stored playlist has been modified, created, deleted or
	    renamed */
	IDLE_STORED_PLAYLIST = 0x2,

	/** the current playlist has been modified */
	IDLE_PLAYLIST = 0x4,

	/** the player state has changed: play, stop, pause, seek, ... */
	IDLE_PLAYER = 0x8,

	/** the volume has been modified */
	IDLE_MIXER = 0x10,

	/** an audio output device has been enabled or disabled */
	IDLE_OUTPUT = 0x20,

	/** options have changed: crossfade, random, repeat, ... */
	IDLE_OPTIONS = 0x40,

	/** MPD closed the connection or the connection was lost */
	IDLE_DISCONNECT = 0x80,
};

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mpd_NotificationCb) (struct mpd_connection *connection, unsigned flags, void *userdata);

void mpd_startIdle(struct mpd_connection *connection, mpd_NotificationCb notify_cb, void *userdata);

void mpd_stopIdle(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
