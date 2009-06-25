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

#include <mpd/entity.h>
#include <mpd/stored_playlist.h>
#include "internal.h"
#include "str_pool.h"

#include <stdlib.h>
#include <string.h>

void
mpd_entity_free(struct mpd_entity *entity) {
	assert(entity != NULL);

	switch (entity->type) {
	case MPD_ENTITY_TYPE_DIRECTORY:
		mpd_directory_free(entity->info.directory);
		break;

	case MPD_ENTITY_TYPE_SONG:
		mpd_song_free(entity->info.song);
		break;

	case MPD_ENTITY_TYPE_PLAYLISTFILE:
		mpd_stored_playlist_free(entity->info.playlistFile);
		break;
	}

	free(entity);
}

static void
parse_song_pair(struct mpd_song *song, const char *name, char *value)
{
	if (*value == 0)
		return;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		if (strcmp(name, mpd_tag_type_names[i]) == 0) {
			mpd_song_add_tag(song, (enum mpd_tag_type)i, value);
			return;
		}
	}

	if (strcmp(name, "Time") == 0)
		mpd_song_set_time(song, atoi(value));
	else if (strcmp(name, "Pos") == 0)
		mpd_song_set_pos(song, atoi(value));
	else if (strcmp(name, "Id") == 0)
		mpd_song_set_id(song, atoi(value));
}

struct mpd_entity *
mpd_get_next_entity(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	struct mpd_entity *entity;

	if (mpd_error_is_defined(&connection->error))
		return NULL;

	pair = mpd_recv_pair(connection);
	if (pair == NULL)
		return NULL;

	if (strcmp(pair->name, "file") == 0) {
		entity = malloc(sizeof(*entity));
		if (entity == NULL) {
			mpd_pair_free(pair);
			mpd_error_code(&connection->error, MPD_ERROR_OOM);
			return NULL;
		}

		entity->type = MPD_ENTITY_TYPE_SONG;
		entity->info.song = mpd_song_new();
		mpd_song_add_tag(entity->info.song,
				 MPD_TAG_FILENAME, pair->value);

		mpd_pair_free(pair);
	} else if (strcmp(pair->name, "directory") == 0) {
		entity = malloc(sizeof(*entity));
		if (entity == NULL) {
			mpd_pair_free(pair);
			mpd_error_code(&connection->error, MPD_ERROR_OOM);
			return NULL;
		}

		entity->type = MPD_ENTITY_TYPE_DIRECTORY;
		entity->info.directory = mpd_directory_new();
		entity->info.directory->path = str_pool_dup(pair->value);

		mpd_pair_free(pair);
	} else if (strcmp(pair->name, "playlist") == 0) {
		entity = malloc(sizeof(*entity));
		if (entity == NULL) {
			mpd_pair_free(pair);
			mpd_error_code(&connection->error, MPD_ERROR_OOM);
			return NULL;
		}

		entity->type = MPD_ENTITY_TYPE_PLAYLISTFILE;
		entity->info.playlistFile = mpd_stored_playlist_new();
		entity->info.playlistFile->path = str_pool_dup(pair->value);

		mpd_pair_free(pair);
	} else if (strcmp(pair->name, "cpos") == 0){
		entity = malloc(sizeof(*entity));
		if (entity == NULL) {
			mpd_pair_free(pair);
			mpd_error_code(&connection->error, MPD_ERROR_OOM);
			return NULL;
		}

		entity->type = MPD_ENTITY_TYPE_SONG;
		entity->info.song = mpd_song_new();
		mpd_song_set_pos(entity->info.song, atoi(pair->value));

		mpd_pair_free(pair);
	} else {
		mpd_pair_free(pair);

		mpd_error_code(&connection->error, MPD_ERROR_MALFORMED);
		mpd_error_message(&connection->error,
				  "problem parsing song info");
		return NULL;
	}

	while ((pair = mpd_recv_pair(connection)) != NULL) {
		if (strcmp(pair->name, "file") == 0 ||
		    strcmp(pair->name, "directory") == 0 ||
		    strcmp(pair->name, "playlist") == 0 ||
		    strcmp(pair->name, "cpos") == 0)
			break;

		if (entity->type == MPD_ENTITY_TYPE_SONG)
			parse_song_pair(entity->info.song, pair->name, pair->value);
		else if (entity->type == MPD_ENTITY_TYPE_DIRECTORY) {
		}
		else if (entity->type == MPD_ENTITY_TYPE_PLAYLISTFILE) {
		}

		mpd_pair_free(pair);
	}

	if (mpd_error_is_defined(&connection->error)) {
		mpd_entity_free(entity);
		return NULL;
	}

	/* unread this pair for the next mpd_get_next_entity() call */
	mpd_enqueue_pair(connection, pair);

	return entity;
}
