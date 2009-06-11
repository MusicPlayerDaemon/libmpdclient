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

#ifndef MPD_INTERNAL_H
#define MPD_INTERNAL_H

#include "ierror.h"
#include <mpd/connection.h>
#include <mpd/status.h>
#include "socket.h"

#include <sys/select.h>

/* mpd_connection
 * holds info about connection to mpd
 * use error, and errorStr to detect errors
 */
struct mpd_connection {
	/* use this to check the version of mpd */
	unsigned version[3];

	struct mpd_error_info error;

	/* DON'T TOUCH any of the rest of this stuff */

	struct mpd_async *async;
	struct timeval timeout;

	struct mpd_parser *parser;

	/**
	 * Are we currently receiving the response of a command?
	 */
	bool receiving;

	int listOks;
	int doneListOk;
	int commandList;
	struct mpd_pair *pair;
	char *request;
	int idle;
	void (*notify_cb)(struct mpd_connection *connection,
			  unsigned flags, void *userdata);
	void (*startIdle)(struct mpd_connection *connection);
	void (*stopIdle)(struct mpd_connection *connection);
	void *userdata;
#ifdef MPD_GLIB
        int source_id;
#endif
};

extern const char *const mpdTagItemKeys[];

/* struct mpd_status
 * holds info about MPD status
 */
struct mpd_status {
	/* 0-100, or MPD_STATUS_NO_VOLUME when there is no volume support */
	int volume;

	/** Playlist repeat mode enabled? */
	bool repeat;

	/** Random mode enabled? */
	bool random;

	/** Single song mode enabled? */
	bool single;

	/** Song consume mode enabled? */
	bool consume;

	/* playlist length */
	int playlist_length;
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
	int elapsed_time;
	/* length in seconds of the currently playing/paused song */
	int total_time;
	/* current bit rate in kbs */
	int bit_rate;
	/* audio sample rate */
	unsigned int sample_rate;
	/* audio bits */
	int bits;
	/* audio channels */
	int channels;
	/* 1 if mpd is updating, 0 otherwise */
	int updatingdb;
	/* error */
	char * error;
};

struct mpd_stats {
	int number_of_artists;
	int number_of_albums;
	int number_of_songs;
	unsigned long uptime;
	unsigned long db_update_time;
	unsigned long play_time;
	unsigned long db_play_time;
};

struct mpd_search_stats {
	int number_of_songs;
	unsigned long play_time;
};

/**
 * Copies the error state from connection->sync to connection->error.
 */
void
mpd_connection_sync_error(struct mpd_connection *connection);

#endif
