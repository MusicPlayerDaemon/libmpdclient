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

#include <mpd/status.h>
#include <mpd/pair.h>
#include <mpd/send.h>
#include <mpd/connection.h>
#include <mpd/recv.h>
#include "internal.h"

#include <stdlib.h>
#include <string.h>

/**
 * Information about MPD's current status.
 */
struct mpd_status {
	/** 0-100, or MPD_STATUS_NO_VOLUME when there is no volume support */
	int volume;

	/** Playlist repeat mode enabled? */
	bool repeat;

	/** Random mode enabled? */
	bool random;

	/** Single song mode enabled? */
	bool single;

	/** Song consume mode enabled? */
	bool consume;

	/** Number of songs in the playlist */
	int playlist_length;

	/** playlist, use this to determine when the playlist has changed */
	long long playlist;

	/** MPD's current playback state */
	enum mpd_state state;

	/** crossfade setting in seconds */
	int crossfade;

	/**
	 * If a song is currently selected (always the case when state
	 * is PLAY or PAUSE), this is the position of the currently
	 * playing song in the playlist, beginning with 0.
	 */
	int song;

	/** Song ID of the currently selected song */
	int songid;

	/**
	 * Time in seconds that have elapsed in the currently
	 * playing/paused song.
	 */
	int elapsed_time;

	/** length in seconds of the currently playing/paused song */
	int total_time;

	/** current bit rate in kbps */
	int bit_rate;

	/** audio sample rate */
	unsigned int sample_rate;

	/** audio bits */
	int bits;

	/** audio channels */
	int channels;

	/** non-zero if MPD is updating, 0 otherwise */
	int updatingdb;

	/** error message */
	char *error;
};

struct mpd_status *
mpd_status_new(void)
{
	struct mpd_status *status = malloc(sizeof(*status));
	if (status == NULL)
		return NULL;

	status->volume = -1;
	status->repeat = false;
	status->random = false;
	status->single = false;
	status->consume = false;
	status->playlist = -1;
	status->playlist_length = -1;
	status->state = MPD_STATE_UNKNOWN;
	status->song = 0;
	status->songid = 0;
	status->elapsed_time = 0;
	status->total_time = 0;
	status->bit_rate = 0;
	status->sample_rate = 0;
	status->bits = 0;
	status->channels = 0;
	status->crossfade = -1;
	status->error = NULL;
	status->updatingdb = 0;

	return status;
}

static enum mpd_state
parse_mpd_state(const char *p)
{
	if (strcmp(p, "play") == 0)
		return MPD_STATE_PLAY;
	else if (strcmp(p, "stop") == 0)
		return MPD_STATE_STOP;
	else if (strcmp(p, "pause") == 0)
		return MPD_STATE_PAUSE;
	else
		return MPD_STATE_UNKNOWN;
}

void
mpd_status_feed(struct mpd_status *status, const struct mpd_pair *pair)
{
	if (strcmp(pair->name, "volume") == 0)
		status->volume = atoi(pair->value);
	else if (strcmp(pair->name, "repeat") == 0)
		status->repeat = !!atoi(pair->value);
	else if (strcmp(pair->name, "random") == 0)
		status->random = !!atoi(pair->value);
	else if (strcmp(pair->name, "single") == 0)
		status->single = !!atoi(pair->value);
	else if (strcmp(pair->name, "consume") == 0)
		status->consume = !!atoi(pair->value);
	else if (strcmp(pair->name, "playlist") == 0)
		status->playlist = strtol(pair->value,NULL,10);
	else if (strcmp(pair->name, "playlistlength") == 0)
		status->playlist_length = atoi(pair->value);
	else if (strcmp(pair->name, "bitrate") == 0)
		status->bit_rate = atoi(pair->value);
	else if (strcmp(pair->name, "state") == 0)
		status->state = parse_mpd_state(pair->value);
	else if (strcmp(pair->name, "song") == 0)
		status->song = atoi(pair->value);
	else if (strcmp(pair->name, "songid") == 0)
		status->songid = atoi(pair->value);
	else if (strcmp(pair->name, "time") == 0) {
		char * tok = strchr(pair->value,':');
		/* the second strchr below is a safety check */
		if (tok && (strchr(tok,0) > (tok+1))) {
			/* atoi stops at the first non-[0-9] char: */
			status->elapsed_time = atoi(pair->value);
			status->total_time = atoi(tok+1);
		}
	} else if (strcmp(pair->name, "error") == 0) {
		if (status->error != NULL)
			free(status->error);

		status->error = strdup(pair->value);
	} else if (strcmp(pair->name, "xfade") == 0)
		status->crossfade = atoi(pair->value);
	else if (strcmp(pair->name, "updating_db") == 0)
		status->updatingdb = atoi(pair->value);
	else if (strcmp(pair->name, "audio") == 0) {
		char * tok = strchr(pair->value,':');
		if (tok && (strchr(tok,0) > (tok+1))) {
			status->sample_rate = atoi(pair->value);
			status->bits = atoi(++tok);
			tok = strchr(tok,':');
			if (tok && (strchr(tok,0) > (tok+1)))
				status->channels = atoi(tok+1);
		}
	}

}

struct mpd_status * mpd_get_status(struct mpd_connection * connection) {
	struct mpd_status * status;
	struct mpd_pair *pair;

	if (mpd_error_is_defined(&connection->error))
		return NULL;

	/*mpd_send_command(connection, "status", NULL);

	if (connection->error) return NULL;*/

	status = mpd_status_new();
	if (status == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return NULL;
	}

	while ((pair = mpd_recv_pair(connection)) != NULL) {
		mpd_status_feed(status, pair);
		mpd_return_pair(connection, pair);
	}

	if (mpd_error_is_defined(&connection->error)) {
		mpd_status_free(status);
		return NULL;
	}

	return status;
}

void mpd_status_free(struct mpd_status * status) {
	if (status->error) free(status->error);
	free(status);
}

int mpd_status_get_volume(const struct mpd_status *status)
{
	return status->volume;
}

bool
mpd_status_get_repeat(const struct mpd_status *status)
{
	return status->repeat;
}

bool
mpd_status_get_random(const struct mpd_status *status)
{
	return status->random;
}

bool
mpd_status_get_single(const struct mpd_status *status)
{
	return status->single;
}

bool
mpd_status_get_consume(const struct mpd_status *status)
{
	return status->consume;
}

int mpd_status_get_playlist_length(const struct mpd_status *status)
{
	return status->playlist_length;
}

long long mpd_status_get_playlist(const struct mpd_status *status)
{
	return status->playlist;
}

enum mpd_state
mpd_status_get_state(const struct mpd_status *status)
{
	return status->state;
}

int mpd_status_get_crossfade(const struct mpd_status *status)
{
	return status->crossfade;
}

int mpd_status_get_song(const struct mpd_status *status)
{
	return status->song;
}

int mpd_status_get_songid(const struct mpd_status *status)
{
	return status->songid;
}

int mpd_status_get_elapsed_time(const struct mpd_status *status)
{
	return status->elapsed_time;
}

int mpd_status_get_total_time(const struct mpd_status *status)
{
	return status->total_time;
}

int mpd_status_get_bit_rate(const struct mpd_status *status)
{
	return status->bit_rate;
}

unsigned int mpd_status_get_sample_rate(const struct mpd_status *status)
{
	return status->sample_rate;
}

int mpd_status_get_bits(const struct mpd_status *status)
{
	return status->bits;
}

int mpd_status_get_channels(const struct mpd_status *status)
{
	return status->channels;
}

int mpd_status_get_updatingdb(const struct mpd_status *status)
{
	return status->updatingdb;
}

const char *
mpd_status_get_error(const struct mpd_status *status)
{
	return status->error;
}
