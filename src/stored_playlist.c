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

#include <mpd/stored_playlist.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct mpd_stored_playlist {
	char *path;
};

struct mpd_stored_playlist *
mpd_stored_playlist_new(const char *path)
{
	struct mpd_stored_playlist *playlist;

	assert(path != NULL);
	assert(*path != '/');
	assert(*path != 0);
	assert(path[strlen(path) - 1] != '/');

	playlist = malloc(sizeof(*playlist));
	if (playlist == NULL)
		/* out of memory */
		return NULL;

	playlist->path = strdup(path);
	if (playlist->path == NULL) {
		/* out of memory */
		free(playlist);
		return NULL;
	}

	return playlist;
}

void
mpd_stored_playlist_free(struct mpd_stored_playlist *playlist)
{
	assert(playlist != NULL);
	assert(playlist->path != NULL);

	free(playlist->path);
	free(playlist);
}

struct mpd_stored_playlist *
mpd_stored_playlist_dup(const struct mpd_stored_playlist *playlist)
{
	assert(playlist != NULL);
	assert(playlist->path != NULL);

	return mpd_stored_playlist_new(playlist->path);
}

const char *
mpd_stored_playlist_get_path(const struct mpd_stored_playlist *playlist)
{
	assert(playlist != NULL);

	return playlist->path;
}
