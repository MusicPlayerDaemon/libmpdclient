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

static void
mpd_initInfoEntity(mpd_entity * entity) {
	entity->info.directory = NULL;
}

static void
mpd_finishInfoEntity(mpd_entity * entity) {
	if (entity->info.directory) {
		if (entity->type == MPD_ENTITY_TYPE_DIRECTORY)
			mpd_directory_free(entity->info.directory);
		else if (entity->type == MPD_ENTITY_TYPE_SONG)
			mpd_song_free(entity->info.song);
		else if (entity->type == MPD_ENTITY_TYPE_PLAYLISTFILE)
			mpd_stored_playlist_free(entity->info.playlistFile);
	}
}

mpd_entity *
mpd_entity_new(void) {
	mpd_entity * entity = malloc(sizeof(mpd_entity));

	mpd_initInfoEntity(entity);

	return entity;
}

void
mpd_entity_free(mpd_entity * entity) {
	mpd_finishInfoEntity(entity);
	free(entity);
}

static void
parse_song_pair(struct mpd_song *song, const char *name, char *value)
{
	if (*value == 0)
		return;

	if (song->artist == NULL && strcmp(name, "Artist") == 0)
		song->artist = str_pool_dup(value);
	else if (song->album == NULL && strcmp(name, "Album") == 0)
		song->album = str_pool_dup(value);
	else if (song->title == NULL && strcmp(name, "Title") == 0)
		song->title = str_pool_dup(value);
	else if (song->track == NULL && strcmp(name, "Track") == 0)
		song->track = str_pool_dup(value);
	else if (song->name == NULL && strcmp(name, "Name") == 0)
		song->name = str_pool_dup(value);
	else if (song->time == MPD_SONG_NO_TIME && strcmp(name, "Time") == 0)
		song->time = atoi(value);
	else if (song->pos == MPD_SONG_NO_NUM && strcmp(name, "Pos") == 0)
		song->pos = atoi(value);
	else if (song->id == MPD_SONG_NO_ID && strcmp(name, "Id") == 0)
		song->id = atoi(value);
	else if (song->date == NULL && strcmp(name, "Date") == 0)
		song->date = str_pool_dup(value);
	else if (song->genre == NULL && strcmp(name, "Genre") == 0)
		song->genre = str_pool_dup(value);
	else if (song->composer == NULL && strcmp(name, "Composer") == 0)
		song->composer = str_pool_dup(value);
	else if (song->performer == NULL && strcmp(name, "Performer") == 0)
		song->performer = str_pool_dup(value);
	else if (song->disc == NULL && strcmp(name, "Disc") == 0)
		song->disc = str_pool_dup(value);
	else if (song->comment == NULL && strcmp(name, "Comment") == 0)
		song->comment = str_pool_dup(value);
}

mpd_entity *
mpd_get_next_entity(struct mpd_connection *connection)
{
	const struct mpd_pair *pair;
	mpd_entity * entity = NULL;

	pair = mpd_get_pair(connection);
	if (pair == NULL)
		return NULL;

	if (strcmp(pair->name, "file") == 0) {
		entity = mpd_entity_new();
		entity->type = MPD_ENTITY_TYPE_SONG;
		entity->info.song = mpd_song_new();
		entity->info.song->file = str_pool_dup(pair->value);
	} else if (strcmp(pair->name, "directory") == 0) {
		entity = mpd_entity_new();
		entity->type = MPD_ENTITY_TYPE_DIRECTORY;
		entity->info.directory = mpd_directory_new();
		entity->info.directory->path = str_pool_dup(pair->value);
	} else if (strcmp(pair->name, "playlist") == 0) {
		entity = mpd_entity_new();
		entity->type = MPD_ENTITY_TYPE_PLAYLISTFILE;
		entity->info.playlistFile = mpd_stored_playlist_new();
		entity->info.playlistFile->path = str_pool_dup(pair->value);
	} else if (strcmp(pair->name, "cpos") == 0){
		entity = mpd_entity_new();
		entity->type = MPD_ENTITY_TYPE_SONG;
		entity->info.song = mpd_song_new();
		entity->info.song->pos = atoi(pair->value);
	} else {
		mpd_error_code(&connection->error, MPD_ERROR_MALFORMED);
		mpd_error_message(&connection->error,
				  "problem parsing song info");
		return NULL;
	}

	while ((pair = mpd_get_next_pair(connection)) != NULL) {
		if (strcmp(pair->name, "file") == 0 ||
		    strcmp(pair->name, "directory") == 0 ||
		    strcmp(pair->name, "playlist") == 0 ||
		    strcmp(pair->name, "cpos") == 0)
			return entity;

		if (entity->type == MPD_ENTITY_TYPE_SONG)
			parse_song_pair(entity->info.song, pair->name, pair->value);
		else if (entity->type == MPD_ENTITY_TYPE_DIRECTORY) {
		}
		else if (entity->type == MPD_ENTITY_TYPE_PLAYLISTFILE) {
		}
	}

	return entity;
}
