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

#ifndef MPD_SONG_H
#define MPD_SONG_H

#include <mpd/tag.h>

#include <stdbool.h>

#define MPD_SONG_NO_TIME	-1
#define MPD_SONG_NO_NUM		-1
#define MPD_SONG_NO_ID		-1

/* mpd_Song
 * for storing song info returned by mpd
 */
struct mpd_song;

#ifdef __cplusplus
extern "C" {
#endif

/* mpd_song_new
 * use to allocate memory for a new mpd_Song
 * file, artist, etc all initialized to NULL
 * if your going to assign values to file, artist, etc
 * be sure to malloc or strdup the memory
 * use mpd_song_free to free the memory for the mpd_Song, it will also
 * free memory for file, artist, etc, so don't do it yourself
 *
 * @param uri the song URI
 */
struct mpd_song *
mpd_song_new(const char *uri);

/* mpd_song_free
 * use to free memory allocated by mpd_song_new
 * also it will free memory pointed to by file, artist, etc, so be careful
 */
void mpd_song_free(struct mpd_song *song);

/* mpd_song_dup
 * works like strDup, but for a mpd_Song
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

#ifdef __cplusplus
}
#endif

#endif
