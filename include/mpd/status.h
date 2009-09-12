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

#ifndef MPD_STATUS_H
#define MPD_STATUS_H

#include <stdbool.h>

/**
 * MPD's playback state.
 */
enum mpd_state {
	/** no information available */
	MPD_STATE_UNKNOWN = 0,

	/** not playing */
	MPD_STATE_STOP = 1,

	/** playing */
	MPD_STATE_PLAY = 2,

	/** playing, but paused */
	MPD_STATE_PAUSE = 3,
};

/* us this with status.volume to determine if mpd has volume support */
#define MPD_STATUS_NO_VOLUME		-1

struct mpd_connection;
struct mpd_pair;

/**
 * \struct mpd_status
 *
 * Holds information about MPD's status.
 */
struct mpd_status;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new empty #mpd_status object.  Free it with
 * mpd_status_free().
 *
 * @return the newly allocated #mpd_status object, or NULL if out of
 * memory
 */
struct mpd_status *
mpd_status_new(void);

/**
 * Parses the pair, adding its information to the specified
 * #mpd_status object.
 */
void
mpd_status_feed(struct mpd_status *status, const struct mpd_pair *pair);

/**
 * Sends the "status" command to MPD.  Call mpd_recv_status() to read
 * the response.
 *
 * @return true on success
 */
bool
mpd_send_status(struct mpd_connection *connection);

/**
 * Receives a #mpd_status object from the server.
 *
 * @return the received #mpd_status object, or NULL on error
 */
struct mpd_status *
mpd_recv_status(struct mpd_connection *connection);

/**
 * Executes the "status" command and reads the response.
 *
 * @return the #mpd_status object returned by the server, or NULL on
 * error
 */
struct mpd_status *
mpd_run_status(struct mpd_connection *connection);

/**
 * Releases a #mpd_status object.
 */
void mpd_status_free(struct mpd_status * status);

/**
 * Returns the current volume: 0-100, or MPD_STATUS_NO_VOLUME when there is no
 * volume support
 */
int mpd_status_get_volume(const struct mpd_status *status);

/**
 * Returns true if repeat mode is on.
 */
bool
mpd_status_get_repeat(const struct mpd_status *status);

/**
 * Returns true if random mode is on.
 */
bool
mpd_status_get_random(const struct mpd_status *status);

/**
 * Returns true if single mode is on.
 */
bool
mpd_status_get_single(const struct mpd_status *status);

/**
 * Returns true if consume mode is on.
 */
bool
mpd_status_get_consume(const struct mpd_status *status);

/**
 * Returns the number of songs in the playlist.  If MPD did not
 * specify that, this function returns 0.
 */
unsigned
mpd_status_get_playlist_length(const struct mpd_status *status);

/**
 * Returns playlist version number.  You may use this to determine
 * when the playlist has changed since you have last queried it.
 */
unsigned
mpd_status_get_playlist_version(const struct mpd_status *status);

/**
 * Returns the state of the player (use with MPD_STATUS_STATE_*)
 */
enum mpd_state
mpd_status_get_state(const struct mpd_status *status);

/**
 * Returns crossfade setting in seconds
 */
int mpd_status_get_crossfade(const struct mpd_status *status);

/**
 * Returns the position of the currently playing song in the playlist
 * (beginning with 0) if a song is currently selected (always the case when
 * state is PLAY or PAUSE)
 */
int mpd_status_get_song(const struct mpd_status *status);

/**
 * Returns Song ID of the currently selected song
 */
int mpd_status_get_songid(const struct mpd_status *status);

/**
 * Returns time in seconds that have elapsed in the currently playing/paused
 * song
 */
int mpd_status_get_elapsed_time(const struct mpd_status *status);

/**
 * Returns the length in seconds of the currently playing/paused song
 */
int mpd_status_get_total_time(const struct mpd_status *status);

/**
 * Returns current bit rate in kbs
 */
int mpd_status_get_bit_rate(const struct mpd_status *status);

/**
 * Returns audio sample rate
 */
unsigned int mpd_status_get_sample_rate(const struct mpd_status *status);

/**
 * Returns audio bits
 */
int mpd_status_get_bits(const struct mpd_status *status);

/**
 * Returns audio channels
 */
int mpd_status_get_channels(const struct mpd_status *status);

/**
 * Returns 1 if mpd is updating, 0 otherwise
 */
unsigned
mpd_status_get_update_id(const struct mpd_status *status);

/**
 * Returns the error message
 */
const char *
mpd_status_get_error(const struct mpd_status *status);

#ifdef __cplusplus
}
#endif

#endif
