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

#include <mpd/pair.h>
#include "str_pool.h"

#include <assert.h>
#include <stdlib.h>

struct mpd_pair *
mpd_pair_new(const char *name, const char *value)
{
	struct mpd_pair *pair = malloc(sizeof(*pair));

	assert(name != NULL);
	assert(value != NULL);

	if (pair == NULL)
		return NULL;

	pair->name = str_pool_get(name);
	if (pair->name == NULL) {
		free(pair);
		return NULL;
	}

	pair->value = str_pool_get(value);
	if (pair->value == NULL) {
		str_pool_put(pair->name);
		free(pair);
		return NULL;
	}

	return pair;
}

void
mpd_pair_free(struct mpd_pair *pair) {
	assert(pair != NULL);
	assert(pair->name != NULL);
	assert(pair->value != NULL);

	str_pool_put(pair->name);
	str_pool_put(pair->value);
	free(pair);
}
