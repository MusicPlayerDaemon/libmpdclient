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

#ifndef LIBMPDCLIENT_STORED_PLAYLIST_H
#define LIBMPDCLIENT_STORED_PLAYLIST_H

#include <time.h>

/**
 * An opaque representation for a stored playlist stored in MPD's
 * playlist directory.  Use the functions provided by this header to
 * access the object's attributes.
 */
struct mpd_stored_playlist;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a new #mpd_stored_playlist object.  Use
 * mpd_stored_playlist_free() to release it when you don't need it
 * anymore.
 *
 * @param path the path of the playlist fle relative to the MPD
 * playlist directory.  It must not begin or end with a slash
 * @returns the new object, or NULL if out of memory
 */
struct mpd_stored_playlist *
mpd_stored_playlist_new(const char *path);

/**
 * Free memory allocated by the #mpd_stored_playlist object.
 */
void
mpd_stored_playlist_free(struct mpd_stored_playlist *playlist);

/**
 * Duplicates a #mpd_stored_playlist object.
 *
 * @return the new object, or NULL on out of memory
 */
struct mpd_stored_playlist *
mpd_stored_playlist_dup(const struct mpd_stored_playlist *playlist);

/**
 * Returns the path name of this playlist file.  It does not begin
 * with a slash.
 */
const char *
mpd_stored_playlist_get_path(const struct mpd_stored_playlist *playlist);

/**
 * @return the POSIX UTC time stamp of the last modification, or 0 if
 * that is unknown
 */
time_t
mpd_stored_playlist_get_last_modified(const struct mpd_stored_playlist *playlist);

#ifdef __cplusplus
}
#endif

#endif
