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

#ifndef STATUS_H
#define STATUS_H

#include "connection.h"

/* use these with status.state to determine what state the player is in */
#define MPD_STATUS_STATE_UNKNOWN	0
#define MPD_STATUS_STATE_STOP		1
#define MPD_STATUS_STATE_PLAY		2
#define MPD_STATUS_STATE_PAUSE		3

/* us this with status.volume to determine if mpd has volume support */
#define MPD_STATUS_NO_VOLUME		-1

/* mpd_Status
 * holds info return from status command
 */
typedef struct mpd_Status {
	/* 0-100, or MPD_STATUS_NO_VOLUME when there is no volume support */
	int volume;
	/* 1 if repeat is on, 0 otherwise */
	int repeat;
	/* 1 if random is on, 0 otherwise */
	int random;
	/* playlist length */
	int playlistLength;
	/* playlist, use this to determine when the playlist has changed */
	long long playlist;
	/* use with MPD_STATUS_STATE_* to determine state of player */
	int state;
	/* crossfade setting in seconds */
	int crossfade;
	/* if a song is currently selected (always the case when state is
	 * PLAY or PAUSE), this is the position of the currently
	 * playing song in the playlist, beginning with 0
	 */
	int song;
	/* Song ID of the currently selected song */
	int songid;
	/* time in seconds that have elapsed in the currently playing/paused
	 * song
	 */
	int elapsedTime;
	/* length in seconds of the currently playing/paused song */
	int totalTime;
	/* current bit rate in kbs */
	int bitRate;
	/* audio sample rate */
	unsigned int sampleRate;
	/* audio bits */
	int bits;
	/* audio channels */
	int channels;
	/* 1 if mpd is updating, 0 otherwise */
	int updatingDb;
	/* error */
	char * error;
} mpd_Status;

void mpd_sendStatusCommand(mpd_Connection * connection);

/* mpd_getStatus
 * returns status info, be sure to free it with mpd_freeStatus()
 * call this after mpd_sendStatusCommand()
 */
mpd_Status * mpd_getStatus(mpd_Connection * connection);

/* mpd_freeStatus
 * free's status info malloc'd and returned by mpd_getStatus
 */
void mpd_freeStatus(mpd_Status * status);

#endif
