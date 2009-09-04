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

#ifndef MPD_SONG_H
#define MPD_SONG_H

#include <mpd/tag.h>

#include <stdbool.h>
#include <time.h>

#define MPD_SONG_NO_TIME	-1
#define MPD_SONG_NO_NUM		-1
#define MPD_SONG_NO_ID		-1

struct mpd_pair;
struct mpd_connection;

/**
 * \struct mpd_song
 *
 * An opaque representation for a song in MPD's database or playlist.
 * Use the functions provided by this header to access the object's
 * attributes.
 */
struct mpd_song;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a new #mpd_song object.  Use mpd_song_free() to release
 * it when you don't need it anymore.
 *
 * @param uri the song URI
 * @returns the new object, or NULL if out of memory
 */
struct mpd_song *
mpd_song_new(const char *uri);

/**
 * Free memory allocated by the #mpd_song object.
 */
void mpd_song_free(struct mpd_song *song);

/**
 * Duplicates the specified #mpd_song object.
 *
 * @returns the copy, or NULL if out of memory
 */
struct mpd_song *
mpd_song_dup(const struct mpd_song *song);

/**
 * Returns the URI of the song.  It always returns a value, because a
 * song cannot exist without an URI.
 */
const char *
mpd_song_get_uri(const struct mpd_song *song);

/**
 * Adds a tag value to the song.
 *
 * @return true on success, false if the tag is not supported or if no
 * memory could be allocated
 */
bool
mpd_song_add_tag(struct mpd_song *song,
		 enum mpd_tag_type type, const char *value);

/**
 * Removes all values of the specified tag.
 */
void
mpd_song_clear_tag(struct mpd_song *song, enum mpd_tag_type type);

/**
 * Queries a tag value.
 *
 * @param song the song object
 * @param type the tag type; MPD_TAG_ANY and MPD_TAG_COUNT are invalid
 * values
 * @param idx pass 0 to get the first value for this tag type.  This
 * argument may be used to iterate all values, until this function
 * returns NULL
 * @return the tag value, or NULL if this tag type (or this index)
 * does not exist
 */
const char *
mpd_song_get_tag(const struct mpd_song *song,
		 enum mpd_tag_type type, unsigned idx);

/**
 * Sets the song duration in seconds.
 */
void
mpd_song_set_time(struct mpd_song *song, int t);

/**
 * Returns the duration of this song in seconds.  #MPD_SONG_NO_TIME is
 * a special value for "unknown".
 */
int
mpd_song_get_time(const struct mpd_song *song);

/**
 * Sets the POSIX UTC time stamp of the last modification.
 */
void
mpd_song_set_last_modified(struct mpd_song *song, time_t mtime);

/**
 * @return the POSIX UTC time stamp of the last modification, or 0 if
 * that is unknown
 */
time_t
mpd_song_get_last_modified(const struct mpd_song *song);

/**
 * Sets the position within the playlist.  This value is not used for
 * songs which are not in the playlist.
 */
void
mpd_song_set_pos(struct mpd_song *song, int pos);

/**
 * Returns the position of this song in the playlist.
 * #MPD_SONG_NO_NUM is a special value for "unknown".
 */
int
mpd_song_get_pos(const struct mpd_song *song);

/**
 * Sets the id within the playlist.  This value is not used for songs
 * which are not in the playlist.
 */
void
mpd_song_set_id(struct mpd_song *song, int id);

/**
 * Returns the id of this song in the playlist.  #MPD_SONG_NO_ID is a
 * special value for "unknown".
 */
int
mpd_song_get_id(const struct mpd_song *song);

/**
 * Begins parsing a new song.
 *
 * @param pair the first pair in this song (name must be "file")
 * @return the new #mpd_entity object, or NULL on error (out of
 * memory, or pair name is not "file")
 */
struct mpd_song *
mpd_song_begin(const struct mpd_pair *pair);

/**
 * Parses the pair, adding its information to the specified
 * #mpd_song object.
 *
 * @return true if the pair was parsed and added to the song (or if
 * the pair was not understood and ignored), false if this pair is the
 * beginning of the next song
 */
bool
mpd_song_feed(struct mpd_song *song, const struct mpd_pair *pair);

/**
 * Receives the next song from the MPD server.
 *
 * @return a #mpd_song object, or NULL on error or if the song list is
 * finished
 */
struct mpd_song *
mpd_recv_song(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
