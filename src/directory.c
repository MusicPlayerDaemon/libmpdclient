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

#include <mpd/directory.h>
#include "str_pool.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct mpd_directory *
mpd_directory_new(const char *path)
{
	struct mpd_directory *directory;

	assert(path != NULL);
	assert(*path != '/');
	assert(*path != 0);
	assert(path[strlen(path) - 1] != '/');

	directory = malloc(sizeof(*directory));
	if (directory == NULL)
		/* out of memory */
		return NULL;

	directory->path = str_pool_get(path);
	if (directory->path == NULL) {
		/* out of memory */
		free(directory);
		return NULL;
	}

	return directory;
}

void mpd_directory_free(struct mpd_directory *directory)
{
	assert(directory != NULL);

	if (directory->path)
		str_pool_put(directory->path);

	free(directory);
}

struct mpd_directory *
mpd_directory_dup(const struct mpd_directory *directory)
{
	struct mpd_directory *ret;

	assert(directory != NULL);

	ret = malloc(sizeof(*directory));
	if (ret == NULL)
		/* out of memory */
		return NULL;

	ret->path = str_pool_dup(directory->path);
	if (directory->path == NULL) {
		/* out of memory */
		free(ret);
		return NULL;
	}

	return ret;
}

const char *
mpd_directory_get_path(const struct mpd_directory *directory)
{
	assert(directory != NULL);
	assert(directory->path != NULL);

	return directory->path;
}

