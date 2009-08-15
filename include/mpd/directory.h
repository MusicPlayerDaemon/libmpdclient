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

#ifndef MPD_DIRECTORY_H
#define MPD_DIRECTORY_H

/**
 * A directory object.  This is a container for more songs,
 * directories or playlists.
 */
struct mpd_directory {
	/**
	 * The full path of this directory.  It does not begin with a
	 * slash.
	 */
	char *path;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocates a new directory object.  Call mpd_directory_free() to
 * dispose it.
 *
 * @return the new object, or NULL on out of memory
 */
struct mpd_directory *
mpd_directory_new(void);

/**
 * Duplicates a #mpd_directory object.
 *
 * @return the new object, or NULL on out of memory
 */
struct mpd_directory *
mpd_directory_dup(const struct mpd_directory *directory);

/**
 * Free memory allocated by the #mpd_directory object.  used to free
 */
void mpd_directory_free(struct mpd_directory *directory);

/**
 * Returns the full path of this directory.  It does not begin with a
 * slash.
 */
const char *
mpd_directory_get_path(const struct mpd_directory *directory);

#ifdef __cplusplus
}
#endif

#endif
