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

#include <mpd/song.h>
#include <mpd/pair.h>
#include <mpd/recv.h>
#include "str_pool.h"
#include "internal.h"
#include "iso8601.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct mpd_tag_value {
	struct mpd_tag_value *next;

	char *value;
};

struct mpd_song {
	struct mpd_tag_value tags[MPD_TAG_COUNT];

	/* length of song in seconds, check that it is not MPD_SONG_NO_TIME  */
	int time;

	/**
	 * The POSIX UTC time stamp of the last modification, or 0 if
	 * that is unknown.
	 */
	time_t last_modified;

	/* if plchanges/playlistinfo/playlistid used, is the position of the
	 * song in the playlist */
	int pos;
	/* song id for a song in the playlist */
	int id;
};

struct mpd_song *
mpd_song_new(const char *uri)
{
	struct mpd_song *song;
	bool success;

	assert(uri != NULL);

	song = malloc(sizeof(*song));
	if (song == NULL)
		/* out of memory */
		return NULL;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i)
		song->tags[i].value = NULL;

	song->time = MPD_SONG_NO_TIME;
	song->last_modified = 0;
	song->pos = MPD_SONG_NO_NUM;
	song->id = MPD_SONG_NO_ID;

	success = mpd_song_add_tag(song, MPD_TAG_FILENAME, uri);
	if (!success) {
		free(song);
		return NULL;
	}

	return song;
}

void mpd_song_free(struct mpd_song *song) {
	assert(song != NULL);

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		struct mpd_tag_value *tag = &song->tags[i], *next;

		if (tag->value == NULL)
			continue;

		str_pool_put(tag->value);

		tag = tag->next;

		while (tag != NULL) {
			assert(tag->value != NULL);
			str_pool_put(tag->value);

			next = tag->next;
			free(tag);
			tag = next;
		}
	}

	free(song);
}

struct mpd_song *
mpd_song_dup(const struct mpd_song *song)
{
	struct mpd_song *ret;
	bool success;

	assert(song != NULL);

	ret = mpd_song_new(mpd_song_get_tag(song, MPD_TAG_FILENAME, 0));
	if (ret == NULL)
		/* out of memory */
		return NULL;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		const struct mpd_tag_value *src_tag = &song->tags[i];

		if (src_tag->value == NULL)
			continue;

		do {
			success = mpd_song_add_tag(ret, i, src_tag->value);
			if (!success) {
				mpd_song_free(ret);
				return NULL;
			}

			src_tag = src_tag->next;
		} while (src_tag != NULL);
	}

	ret->time = song->time;
	ret->pos = song->pos;
	ret->id = song->id;

	return ret;
}

const char *
mpd_song_get_uri(const struct mpd_song *song)
{
	return mpd_song_get_tag(song, MPD_TAG_FILENAME, 0);
}

bool
mpd_song_add_tag(struct mpd_song *song,
		 enum mpd_tag_type type, const char *value)
{
	struct mpd_tag_value *tag = &song->tags[type], *prev;

	if ((int)type < 0 || type == MPD_TAG_ANY || type >= MPD_TAG_COUNT)
		return false;

	if (tag->value == NULL) {
		tag->next = NULL;
		tag->value = str_pool_get(value);
		if (tag->value == NULL)
			return false;
	} else {
		while (tag->next != NULL)
			tag = tag->next;

		prev = tag;
		tag = malloc(sizeof(*tag));
		if (tag == NULL)
			return NULL;

		tag->value = str_pool_get(value);
		if (tag->value == NULL) {
			free(tag);
			return false;
		}

		tag->next = NULL;
		prev->next = tag;
	}

	return true;
}

void
mpd_song_clear_tag(struct mpd_song *song, enum mpd_tag_type type)
{
	struct mpd_tag_value *tag = &song->tags[type];

	if (tag->value == NULL)
		/* this tag type is empty */
		return;

	/* free and clear the first value */
	free(tag->value);
	tag->value = NULL;

	/* free all other values; no need to clear the "next" pointer,
	   because it is "undefined" as long as value==NULL */
	while ((tag = tag->next) != NULL)
		free(tag->value);
}

const char *
mpd_song_get_tag(const struct mpd_song *song,
		 enum mpd_tag_type type, unsigned idx)
{
	const struct mpd_tag_value *tag = &song->tags[type];

	if ((int)type < 0 || type == MPD_TAG_ANY || type >= MPD_TAG_COUNT)
		return NULL;

	if (tag->value == NULL)
		return NULL;

	while (idx-- > 0) {
		tag = tag->next;
		if (tag == NULL)
			return NULL;
	}

	return tag->value;
}

void
mpd_song_set_time(struct mpd_song *song, int t)
{
	song->time = t;
}

int
mpd_song_get_time(const struct mpd_song *song)
{
	return song->time;
}

void
mpd_song_set_last_modified(struct mpd_song *song, time_t mtime)
{
	song->last_modified = mtime;
}

time_t
mpd_song_get_last_modified(const struct mpd_song *song)
{
	return song->last_modified;
}

void
mpd_song_set_pos(struct mpd_song *song, int pos)
{
	song->pos = pos;
}

int
mpd_song_get_pos(const struct mpd_song *song)
{
	return song->pos;
}

void
mpd_song_set_id(struct mpd_song *song, int id)
{
	song->id = id;
}

int
mpd_song_get_id(const struct mpd_song *song)
{
	return song->id;
}

struct mpd_song *
mpd_song_begin(const struct mpd_pair *pair)
{
	assert(pair != NULL);
	assert(pair->name != NULL);
	assert(pair->value != NULL);

	if (strcmp(pair->name, "file") != 0)
		return NULL;

	return mpd_song_new(pair->value);
}

bool
mpd_song_feed(struct mpd_song *song, const struct mpd_pair *pair)
{
	assert(pair != NULL);
	assert(pair->name != NULL);
	assert(pair->value != NULL);

	if (strcmp(pair->name, "file") == 0)
		return false;

	if (*pair->value == 0)
		return true;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		if (strcmp(pair->name, mpd_tag_type_names[i]) == 0) {
			mpd_song_add_tag(song, (enum mpd_tag_type)i, pair->value);
			return true;
		}
	}

	if (strcmp(pair->name, "Time") == 0)
		mpd_song_set_time(song, atoi(pair->value));
	else if (strcmp(pair->name, "Last-Modified") == 0)
		mpd_song_set_last_modified(song, iso8601_datetime_parse(pair->value));
	else if (strcmp(pair->name, "Pos") == 0)
		mpd_song_set_pos(song, atoi(pair->value));
	else if (strcmp(pair->name, "Id") == 0)
		mpd_song_set_id(song, atoi(pair->value));

	return true;
}

struct mpd_song *
mpd_recv_song(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	struct mpd_song *song;

	pair = mpd_recv_pair_named(connection, "file");
	if (pair == NULL)
		return NULL;

	song = mpd_song_begin(pair);
	mpd_return_pair(connection, pair);
	if (song == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return NULL;
	}

	while ((pair = mpd_recv_pair(connection)) != NULL &&
	       mpd_song_feed(song, pair))
		mpd_return_pair(connection, pair);

	if (mpd_error_is_defined(&connection->error)) {
		mpd_song_free(song);
		return NULL;
	}

	/* unread this pair for the next mpd_recv_song() call */
	mpd_enqueue_pair(connection, pair);

	return song;
}
