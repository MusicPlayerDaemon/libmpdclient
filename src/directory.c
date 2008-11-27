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

#include <stdlib.h>

static void
mpd_initDirectory(struct mpd_directory *directory)
{
	directory->path = NULL;
}

static void
mpd_finishDirectory(struct mpd_directory *directory)
{
	if (directory->path)
		str_pool_put(directory->path);
}

struct mpd_directory *
mpd_newDirectory(void)
{
	struct mpd_directory *directory = malloc(sizeof(*directory));

	mpd_initDirectory(directory);

	return directory;
}

void mpd_freeDirectory(struct mpd_directory *directory)
{
	mpd_finishDirectory(directory);

	free(directory);
}

struct mpd_directory *
mpd_directoryDup(const struct mpd_directory *directory)
{
	struct mpd_directory *ret = mpd_newDirectory();

	if (directory->path)
		ret->path = str_pool_dup(directory->path);

	return ret;
}
