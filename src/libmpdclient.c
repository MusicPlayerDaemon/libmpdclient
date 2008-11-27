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

#include <mpd/client.h>
#include <mpd/return_element.h>
#include "resolver.h"
#include "str_pool.h"

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>

/* (bits+1)/3 (plus the sign character) */
#define INTLEN      ((sizeof(int)       * CHAR_BIT + 1) / 3 + 1)
#define LONGLONGLEN ((sizeof(long long) * CHAR_BIT + 1) / 3 + 1)

const char *const mpdTagItemKeys[MPD_TAG_NUM_OF_ITEM_TYPES] =
{
	"Artist",
	"Album",
	"Title",
	"Track",
	"Name",
	"Genre",
	"Date",
	"Composer",
	"Performer",
	"Comment",
	"Disc",
	"Filename",
	"Any"
};

static char * mpd_sanitizeArg(const char * arg) {
	size_t i;
	char * ret;
	register const char *c;
	register char *rc;

	/* instead of counting in that loop above, just
	 * use a bit more memory and half running time
	 */
	ret = malloc(strlen(arg) * 2 + 1);

	c = arg;
	rc = ret;
	for(i = strlen(arg)+1; i != 0; --i) {
		if(*c=='"' || *c=='\\')
			*rc++ = '\\';
		*(rc++) = *(c++);
	}

	return ret;
}

void mpd_finishCommand(struct mpd_connection *connection)
{
	while(!connection->doneProcessing) {
		if(connection->doneListOk) connection->doneListOk = 0;
		mpd_getNextReturnElement(connection);
	}
}

static void mpd_finishListOkCommand(struct mpd_connection *connection)
{
	while(!connection->doneProcessing && connection->listOks &&
			!connection->doneListOk)
	{
		mpd_getNextReturnElement(connection);
	}
}

int mpd_nextListOkCommand(struct mpd_connection *connection)
{
	mpd_finishListOkCommand(connection);
	if(!connection->doneProcessing) connection->doneListOk = 0;
	if(connection->listOks == 0 || connection->doneProcessing) return -1;
	return 0;
}

void mpd_sendStatusCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"status\n");
}

mpd_Status * mpd_getStatus(struct mpd_connection *connection)
{
	mpd_Status * status;

	/*mpd_executeCommand(connection,"status\n");

	if(connection->error) return NULL;*/

	if(connection->doneProcessing || (connection->listOks &&
	   connection->doneListOk))
	{
		return NULL;
	}

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	status = malloc(sizeof(mpd_Status));
	status->volume = -1;
	status->repeat = 0;
	status->random = 0;
	status->playlist = -1;
	status->playlistLength = -1;
	status->state = -1;
	status->song = 0;
	status->songid = 0;
	status->elapsedTime = 0;
	status->totalTime = 0;
	status->bitRate = 0;
	status->sampleRate = 0;
	status->bits = 0;
	status->channels = 0;
	status->crossfade = -1;
	status->error = NULL;
	status->updatingDb = 0;

	if(connection->error) {
		free(status);
		return NULL;
	}
	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;
		if(strcmp(re->name,"volume")==0) {
			status->volume = atoi(re->value);
		}
		else if(strcmp(re->name,"repeat")==0) {
			status->repeat = atoi(re->value);
		}
		else if(strcmp(re->name,"random")==0) {
			status->random = atoi(re->value);
		}
		else if(strcmp(re->name,"playlist")==0) {
			status->playlist = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"playlistlength")==0) {
			status->playlistLength = atoi(re->value);
		}
		else if(strcmp(re->name,"bitrate")==0) {
			status->bitRate = atoi(re->value);
		}
		else if(strcmp(re->name,"state")==0) {
			if(strcmp(re->value,"play")==0) {
				status->state = MPD_STATUS_STATE_PLAY;
			}
			else if(strcmp(re->value,"stop")==0) {
				status->state = MPD_STATUS_STATE_STOP;
			}
			else if(strcmp(re->value,"pause")==0) {
				status->state = MPD_STATUS_STATE_PAUSE;
			}
			else {
				status->state = MPD_STATUS_STATE_UNKNOWN;
			}
		}
		else if(strcmp(re->name,"song")==0) {
			status->song = atoi(re->value);
		}
		else if(strcmp(re->name,"songid")==0) {
			status->songid = atoi(re->value);
		}
		else if(strcmp(re->name,"time")==0) {
			char * tok = strchr(re->value,':');
			/* the second strchr below is a safety check */
			if (tok && (strchr(tok,0) > (tok+1))) {
				/* atoi stops at the first non-[0-9] char: */
				status->elapsedTime = atoi(re->value);
				status->totalTime = atoi(tok+1);
			}
		}
		else if(strcmp(re->name,"error")==0) {
			status->error = strdup(re->value);
		}
		else if(strcmp(re->name,"xfade")==0) {
			status->crossfade = atoi(re->value);
		}
		else if(strcmp(re->name,"updating_db")==0) {
			status->updatingDb = atoi(re->value);
		}
		else if(strcmp(re->name,"audio")==0) {
			char * tok = strchr(re->value,':');
			if (tok && (strchr(tok,0) > (tok+1))) {
				status->sampleRate = atoi(re->value);
				status->bits = atoi(++tok);
				tok = strchr(tok,':');
				if (tok && (strchr(tok,0) > (tok+1)))
					status->channels = atoi(tok+1);
			}
		}

		mpd_getNextReturnElement(connection);
		if(connection->error) {
			free(status);
			return NULL;
		}
	}

	if(connection->error) {
		free(status);
		return NULL;
	}
	else if(status->state<0) {
		strcpy(connection->errorStr,"state not found");
		connection->error = 1;
		free(status);
		return NULL;
	}

	return status;
}

void mpd_freeStatus(mpd_Status * status) {
	if(status->error) free(status->error);
	free(status);
}

void mpd_sendStatsCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"stats\n");
}

mpd_Stats * mpd_getStats(struct mpd_connection *connection)
{
	mpd_Stats * stats;

	/*mpd_executeCommand(connection,"stats\n");

	if(connection->error) return NULL;*/

	if(connection->doneProcessing || (connection->listOks &&
	   connection->doneListOk))
	{
		return NULL;
	}

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	stats = malloc(sizeof(mpd_Stats));
	stats->numberOfArtists = 0;
	stats->numberOfAlbums = 0;
	stats->numberOfSongs = 0;
	stats->uptime = 0;
	stats->dbUpdateTime = 0;
	stats->playTime = 0;
	stats->dbPlayTime = 0;

	if(connection->error) {
		free(stats);
		return NULL;
	}
	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;
		if(strcmp(re->name,"artists")==0) {
			stats->numberOfArtists = atoi(re->value);
		}
		else if(strcmp(re->name,"albums")==0) {
			stats->numberOfAlbums = atoi(re->value);
		}
		else if(strcmp(re->name,"songs")==0) {
			stats->numberOfSongs = atoi(re->value);
		}
		else if(strcmp(re->name,"uptime")==0) {
			stats->uptime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"db_update")==0) {
			stats->dbUpdateTime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"playtime")==0) {
			stats->playTime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"db_playtime")==0) {
			stats->dbPlayTime = strtol(re->value,NULL,10);
		}

		mpd_getNextReturnElement(connection);
		if(connection->error) {
			free(stats);
			return NULL;
		}
	}

	if(connection->error) {
		free(stats);
		return NULL;
	}

	return stats;
}

void mpd_freeStats(mpd_Stats * stats) {
	free(stats);
}

mpd_SearchStats * mpd_getSearchStats(struct mpd_connection *connection)
{
	mpd_SearchStats * stats;
	mpd_ReturnElement * re;

	if (connection->doneProcessing ||
	    (connection->listOks && connection->doneListOk)) {
		return NULL;
	}

	if (!connection->returnElement) mpd_getNextReturnElement(connection);

	if (connection->error)
		return NULL;

	stats = malloc(sizeof(mpd_SearchStats));
	stats->numberOfSongs = 0;
	stats->playTime = 0;

	while (connection->returnElement) {
		re = connection->returnElement;

		if (strcmp(re->name, "songs") == 0) {
			stats->numberOfSongs = atoi(re->value);
		} else if (strcmp(re->name, "playtime") == 0) {
			stats->playTime = strtol(re->value, NULL, 10);
		}

		mpd_getNextReturnElement(connection);
		if (connection->error) {
			free(stats);
			return NULL;
		}
	}

	if (connection->error) {
		free(stats);
		return NULL;
	}

	return stats;
}

void mpd_freeSearchStats(mpd_SearchStats * stats)
{
	free(stats);
}

static void mpd_initDirectory(mpd_Directory * directory) {
	directory->path = NULL;
}

static void mpd_finishDirectory(mpd_Directory * directory) {
	if (directory->path)
		str_pool_put(directory->path);
}

mpd_Directory * mpd_newDirectory(void) {
	mpd_Directory * directory = malloc(sizeof(mpd_Directory));;

	mpd_initDirectory(directory);

	return directory;
}

void mpd_freeDirectory(mpd_Directory * directory) {
	mpd_finishDirectory(directory);

	free(directory);
}

mpd_Directory * mpd_directoryDup(const mpd_Directory * directory) {
	mpd_Directory * ret = mpd_newDirectory();

	if (directory->path)
		ret->path = str_pool_dup(directory->path);

	return ret;
}

static void mpd_initPlaylistFile(mpd_PlaylistFile * playlist) {
	playlist->path = NULL;
}

static void mpd_finishPlaylistFile(mpd_PlaylistFile * playlist) {
	if (playlist->path)
		str_pool_put(playlist->path);
}

mpd_PlaylistFile * mpd_newPlaylistFile(void) {
	mpd_PlaylistFile * playlist = malloc(sizeof(mpd_PlaylistFile));

	mpd_initPlaylistFile(playlist);

	return playlist;
}

void mpd_freePlaylistFile(mpd_PlaylistFile * playlist) {
	mpd_finishPlaylistFile(playlist);
	free(playlist);
}

mpd_PlaylistFile * mpd_playlistFileDup(const mpd_PlaylistFile * playlist) {
	mpd_PlaylistFile * ret = mpd_newPlaylistFile();

	if (playlist->path)
		ret->path = str_pool_dup(playlist->path);

	return ret;
}

static void mpd_initInfoEntity(mpd_InfoEntity * entity) {
	entity->info.directory = NULL;
}

static void mpd_finishInfoEntity(mpd_InfoEntity * entity) {
	if(entity->info.directory) {
		if(entity->type == MPD_INFO_ENTITY_TYPE_DIRECTORY) {
			mpd_freeDirectory(entity->info.directory);
		}
		else if(entity->type == MPD_INFO_ENTITY_TYPE_SONG) {
			mpd_freeSong(entity->info.song);
		}
		else if(entity->type == MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
			mpd_freePlaylistFile(entity->info.playlistFile);
		}
	}
}

mpd_InfoEntity * mpd_newInfoEntity(void) {
	mpd_InfoEntity * entity = malloc(sizeof(mpd_InfoEntity));

	mpd_initInfoEntity(entity);

	return entity;
}

void mpd_freeInfoEntity(mpd_InfoEntity * entity) {
	mpd_finishInfoEntity(entity);
	free(entity);
}

static void
mpd_sendInfoCommand(struct mpd_connection *connection, char *command)
{
	mpd_executeCommand(connection,command);
}

mpd_InfoEntity *
mpd_getNextInfoEntity(struct mpd_connection *connection)
{
	mpd_InfoEntity * entity = NULL;

	if(connection->doneProcessing || (connection->listOks &&
	   connection->doneListOk)) {
		return NULL;
	}

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	if(connection->returnElement) {
		if(strcmp(connection->returnElement->name,"file")==0) {
			entity = mpd_newInfoEntity();
			entity->type = MPD_INFO_ENTITY_TYPE_SONG;
			entity->info.song = mpd_newSong();
			entity->info.song->file =
				str_pool_dup(connection->returnElement->value);
		}
		else if(strcmp(connection->returnElement->name,
					"directory")==0) {
			entity = mpd_newInfoEntity();
			entity->type = MPD_INFO_ENTITY_TYPE_DIRECTORY;
			entity->info.directory = mpd_newDirectory();
			entity->info.directory->path =
				str_pool_dup(connection->returnElement->value);
		}
		else if(strcmp(connection->returnElement->name,"playlist")==0) {
			entity = mpd_newInfoEntity();
			entity->type = MPD_INFO_ENTITY_TYPE_PLAYLISTFILE;
			entity->info.playlistFile = mpd_newPlaylistFile();
			entity->info.playlistFile->path =
				str_pool_dup(connection->returnElement->value);
		}
		else if(strcmp(connection->returnElement->name, "cpos") == 0){
			entity = mpd_newInfoEntity();
			entity->type = MPD_INFO_ENTITY_TYPE_SONG;
			entity->info.song = mpd_newSong();
			entity->info.song->pos = atoi(connection->returnElement->value);
		}
		else {
			connection->error = 1;
			strcpy(connection->errorStr,"problem parsing song info");
			return NULL;
		}
	}
	else return NULL;

	mpd_getNextReturnElement(connection);
	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;

		if(strcmp(re->name,"file")==0) return entity;
		else if(strcmp(re->name,"directory")==0) return entity;
		else if(strcmp(re->name,"playlist")==0) return entity;
		else if(strcmp(re->name,"cpos")==0) return entity;

		if(entity->type == MPD_INFO_ENTITY_TYPE_SONG &&
				strlen(re->value)) {
			if(!entity->info.song->artist &&
					strcmp(re->name,"Artist")==0) {
				entity->info.song->artist = str_pool_dup(re->value);
			}
			else if(!entity->info.song->album &&
					strcmp(re->name,"Album")==0) {
				entity->info.song->album = str_pool_dup(re->value);
			}
			else if(!entity->info.song->title &&
					strcmp(re->name,"Title")==0) {
				entity->info.song->title = str_pool_dup(re->value);
			}
			else if(!entity->info.song->track &&
					strcmp(re->name,"Track")==0) {
				entity->info.song->track = str_pool_dup(re->value);
			}
			else if(!entity->info.song->name &&
					strcmp(re->name,"Name")==0) {
				entity->info.song->name = str_pool_dup(re->value);
			}
			else if(entity->info.song->time==MPD_SONG_NO_TIME &&
					strcmp(re->name,"Time")==0) {
				entity->info.song->time = atoi(re->value);
			}
			else if(entity->info.song->pos==MPD_SONG_NO_NUM &&
					strcmp(re->name,"Pos")==0) {
				entity->info.song->pos = atoi(re->value);
			}
			else if(entity->info.song->id==MPD_SONG_NO_ID &&
					strcmp(re->name,"Id")==0) {
				entity->info.song->id = atoi(re->value);
			}
			else if(!entity->info.song->date &&
					strcmp(re->name, "Date") == 0) {
				entity->info.song->date = str_pool_dup(re->value);
			}
			else if(!entity->info.song->genre &&
					strcmp(re->name, "Genre") == 0) {
				entity->info.song->genre = str_pool_dup(re->value);
			}
			else if(!entity->info.song->composer &&
					strcmp(re->name, "Composer") == 0) {
				entity->info.song->composer = str_pool_dup(re->value);
			}
			else if(!entity->info.song->performer &&
					strcmp(re->name, "Performer") == 0) {
				entity->info.song->performer = strdup(re->value);
			}
			else if(!entity->info.song->disc &&
					strcmp(re->name, "Disc") == 0) {
				entity->info.song->disc = str_pool_dup(re->value);
			}
			else if(!entity->info.song->comment &&
					strcmp(re->name, "Comment") == 0) {
				entity->info.song->comment = str_pool_dup(re->value);
			}
		}
		else if(entity->type == MPD_INFO_ENTITY_TYPE_DIRECTORY) {
		}
		else if(entity->type == MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
		}

		mpd_getNextReturnElement(connection);
	}

	return entity;
}

static char *
mpd_getNextReturnElementNamed(struct mpd_connection *connection,
			      const char *name)
{
	if(connection->doneProcessing || (connection->listOks &&
				connection->doneListOk))
	{
		return NULL;
	}

	mpd_getNextReturnElement(connection);
	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;

		if(strcmp(re->name,name)==0) return strdup(re->value);
		mpd_getNextReturnElement(connection);
	}

	return NULL;
}

char *mpd_getNextTag(struct mpd_connection *connection, int type)
{
	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES ||
	    type == MPD_TAG_ITEM_ANY)
		return NULL;
	if (type == MPD_TAG_ITEM_FILENAME)
		return mpd_getNextReturnElementNamed(connection, "file");
	return mpd_getNextReturnElementNamed(connection, mpdTagItemKeys[type]);
}

char * mpd_getNextArtist(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection,"Artist");
}

char * mpd_getNextAlbum(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection,"Album");
}

void
mpd_sendPlaylistInfoCommand(struct mpd_connection *connection, int songPos)
{
	int len = strlen("playlistinfo")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistinfo \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendPlaylistIdCommand(struct mpd_connection *connection, int id)
{
	int len = strlen("playlistid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistid \"%i\"\n", id);
	mpd_sendInfoCommand(connection, string);
	free(string);
}

void
mpd_sendPlChangesCommand(struct mpd_connection *connection,
			 long long playlist)
{
	int len = strlen("plchanges")+2+LONGLONGLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "plchanges \"%lld\"\n", playlist);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendPlChangesPosIdCommand(struct mpd_connection *connection,
			      long long playlist)
{
	int len = strlen("plchangesposid")+2+LONGLONGLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "plchangesposid \"%lld\"\n", playlist);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendListallCommand(struct mpd_connection *connection, const char *dir)
{
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("listall")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "listall \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void
mpd_sendListallInfoCommand(struct mpd_connection *connection, const char *dir)
{
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("listallinfo")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "listallinfo \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void
mpd_sendLsInfoCommand(struct mpd_connection *connection, const char *dir)
{
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("lsinfo")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "lsinfo \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void
mpd_sendCurrentSongCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"currentsong\n");
}

void
mpd_sendSearchCommand(struct mpd_connection *connection, int table,
		      const char *str)
{
	mpd_startSearch(connection, 0);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

void mpd_sendFindCommand(struct mpd_connection *connection, int table,
			 const char * str)
{
	mpd_startSearch(connection, 1);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

void
mpd_sendListCommand(struct mpd_connection *connection, int table,
		    const char *arg1)
{
	char st[10];
	int len;
	char *string;
	if(table == MPD_TABLE_ARTIST) strcpy(st,"artist");
	else if(table == MPD_TABLE_ALBUM) strcpy(st,"album");
	else {
		connection->error = 1;
		strcpy(connection->errorStr,"unknown table for list");
		return;
	}
	if(arg1) {
		char * sanitArg1 = mpd_sanitizeArg(arg1);
		len = strlen("list")+1+strlen(sanitArg1)+2+strlen(st)+3;
		string = malloc(len);
		snprintf(string, len, "list %s \"%s\"\n", st, sanitArg1);
		free(sanitArg1);
	}
	else {
		len = strlen("list")+1+strlen(st)+2;
		string = malloc(len);
		snprintf(string, len, "list %s\n", st);
	}
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendAddCommand(struct mpd_connection *connection, const char * file)
{
	char * sFile = mpd_sanitizeArg(file);
	int len = strlen("add")+2+strlen(sFile)+3;
	char *string = malloc(len);
	snprintf(string, len, "add \"%s\"\n", sFile);
	mpd_executeCommand(connection,string);
	free(string);
	free(sFile);
}

int
mpd_sendAddIdCommand(struct mpd_connection *connection, const char *file)
{
	int retval = -1;
	char *sFile = mpd_sanitizeArg(file);
	int len = strlen("addid")+2+strlen(sFile)+3;
	char *string = malloc(len);

	snprintf(string, len, "addid \"%s\"\n", sFile);
	mpd_sendInfoCommand(connection, string);
	free(string);
	free(sFile);

	string = mpd_getNextReturnElementNamed(connection, "Id");
	if (string) {
		retval = atoi(string);
		free(string);
	}
	
	return retval;
}

void
mpd_sendDeleteCommand(struct mpd_connection *connection, int songPos)
{
	int len = strlen("delete")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "delete \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendDeleteIdCommand(struct mpd_connection *connection, int id)
{
	int len = strlen("deleteid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "deleteid \"%i\"\n", id);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendSaveCommand(struct mpd_connection *connection, const char *name)
{
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("save")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "save \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void
mpd_sendLoadCommand(struct mpd_connection *connection, const char *name)
{
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("load")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "load \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void
mpd_sendRmCommand(struct mpd_connection *connection, const char *name)
{
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("rm")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "rm \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void
mpd_sendRenameCommand(struct mpd_connection *connection,
		      const char *from, const char *to)
{
	char *sFrom = mpd_sanitizeArg(from);
	char *sTo = mpd_sanitizeArg(to);
	int len = strlen("rename")+2+strlen(sFrom)+3+strlen(sTo)+3;
	char *string = malloc(len);
	snprintf(string, len, "rename \"%s\" \"%s\"\n", sFrom, sTo);
	mpd_executeCommand(connection, string);
	free(string);
	free(sFrom);
	free(sTo);
}

void mpd_sendShuffleCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"shuffle\n");
}

void mpd_sendClearCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"clear\n");
}

void mpd_sendPlayCommand(struct mpd_connection *connection, int songPos)
{
	int len = strlen("play")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "play \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendPlayIdCommand(struct mpd_connection *connection, int id)
{
	int len = strlen("playid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playid \"%i\"\n", id);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendStopCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"stop\n");
}

void mpd_sendPauseCommand(struct mpd_connection *connection, int pauseMode)
{
	int len = strlen("pause")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "pause \"%i\"\n", pauseMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendNextCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"next\n");
}

void mpd_sendMoveCommand(struct mpd_connection *connection, int from, int to)
{
	int len = strlen("move")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "move \"%i\" \"%i\"\n", from, to);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendMoveIdCommand(struct mpd_connection *connection, int id, int to)
{
	int len = strlen("moveid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "moveid \"%i\" \"%i\"\n", id, to);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendSwapCommand(struct mpd_connection *connection, int song1, int song2)
{
	int len = strlen("swap")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "swap \"%i\" \"%i\"\n", song1, song2);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendSwapIdCommand(struct mpd_connection *connection, int id1, int id2)
{
	int len = strlen("swapid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "swapid \"%i\" \"%i\"\n", id1, id2);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendSeekCommand(struct mpd_connection *connection, int song, int time)
{
	int len = strlen("seek")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "seek \"%i\" \"%i\"\n", song, time);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendSeekIdCommand(struct mpd_connection *connection, int id, int time)
{
	int len = strlen("seekid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "seekid \"%i\" \"%i\"\n", id, time);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void
mpd_sendUpdateCommand(struct mpd_connection *connection, const char *path)
{
	char * sPath = mpd_sanitizeArg(path);
	int len = strlen("update")+2+strlen(sPath)+3;
	char *string = malloc(len);
	snprintf(string, len, "update \"%s\"\n", sPath);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sPath);
}

int mpd_getUpdateId(struct mpd_connection *connection)
{
	char * jobid;
	int ret = 0;

	jobid = mpd_getNextReturnElementNamed(connection,"updating_db");
	if(jobid) {
		ret = atoi(jobid);
		free(jobid);
	}

	return ret;
}

void mpd_sendPrevCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"previous\n");
}

void mpd_sendRepeatCommand(struct mpd_connection *connection, int repeatMode)
{
	int len = strlen("repeat")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "repeat \"%i\"\n", repeatMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendRandomCommand(struct mpd_connection *connection, int randomMode)
{
	int len = strlen("random")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "random \"%i\"\n", randomMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendSetvolCommand(struct mpd_connection *connection, int volumeChange)
{
	int len = strlen("setvol")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "setvol \"%i\"\n", volumeChange);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendVolumeCommand(struct mpd_connection *connection, int volumeChange)
{
	int len = strlen("volume")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "volume \"%i\"\n", volumeChange);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendCrossfadeCommand(struct mpd_connection *connection, int seconds)
{
	int len = strlen("crossfade")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "crossfade \"%i\"\n", seconds);
	mpd_executeCommand(connection,string);
	free(string);
}

void
mpd_sendPasswordCommand(struct mpd_connection *connection, const char *pass)
{
	char * sPass = mpd_sanitizeArg(pass);
	int len = strlen("password")+2+strlen(sPass)+3;
	char *string = malloc(len);
	snprintf(string, len, "password \"%s\"\n", sPass);
	mpd_executeCommand(connection,string);
	free(string);
	free(sPass);
}

void mpd_sendCommandListBegin(struct mpd_connection *connection)
{
	if(connection->commandList) {
		strcpy(connection->errorStr,"already in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = COMMAND_LIST;
	mpd_executeCommand(connection,"command_list_begin\n");
}

void mpd_sendCommandListOkBegin(struct mpd_connection *connection)
{
	if(connection->commandList) {
		strcpy(connection->errorStr,"already in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = COMMAND_LIST_OK;
	mpd_executeCommand(connection,"command_list_ok_begin\n");
	connection->listOks = 0;
}

void mpd_sendCommandListEnd(struct mpd_connection *connection)
{
	if(!connection->commandList) {
		strcpy(connection->errorStr,"not in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = 0;
	mpd_executeCommand(connection,"command_list_end\n");
}

void mpd_sendOutputsCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"outputs\n");
}

mpd_OutputEntity * mpd_getNextOutput(struct mpd_connection *connection)
{
	mpd_OutputEntity * output = NULL;

	if(connection->doneProcessing || (connection->listOks &&
				connection->doneListOk))
	{
		return NULL;
	}

	if(connection->error) return NULL;

	output = malloc(sizeof(mpd_OutputEntity));
	output->id = -10;
	output->name = NULL;
	output->enabled = 0;

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;
		if(strcmp(re->name,"outputid")==0) {
			if(output!=NULL && output->id>=0) return output;
			output->id = atoi(re->value);
		}
		else if(strcmp(re->name,"outputname")==0) {
			output->name = strdup(re->value);
		}
		else if(strcmp(re->name,"outputenabled")==0) {
			output->enabled = atoi(re->value);
		}

		mpd_getNextReturnElement(connection);
		if(connection->error) {
			free(output);
			return NULL;
		}

	}

	return output;
}

void
mpd_sendEnableOutputCommand(struct mpd_connection *connection, int outputId)
{
	int len = strlen("enableoutput")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "enableoutput \"%i\"\n", outputId);
	mpd_executeCommand(connection,string);
	free(string);
}

void
mpd_sendDisableOutputCommand(struct mpd_connection *connection, int outputId)
{
	int len = strlen("disableoutput")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "disableoutput \"%i\"\n", outputId);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_freeOutputElement(mpd_OutputEntity * output) {
	free(output->name);
	free(output);
}

/**
 * mpd_sendNotCommandsCommand
 * odd naming, but it gets the not allowed commands
 */

void mpd_sendNotCommandsCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection, "notcommands\n");
}

/**
 * mpd_sendCommandsCommand
 * odd naming, but it gets the allowed commands
 */
void mpd_sendCommandsCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection, "commands\n");
}

/**
 * Get the next returned command
 */
char * mpd_getNextCommand(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection, "command");
}

void mpd_sendUrlHandlersCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection, "urlhandlers\n");
}

char * mpd_getNextHandler(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection, "handler");
}

void mpd_sendTagTypesCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection, "tagtypes\n");
}

char * mpd_getNextTagType(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection, "tagtype");
}

void mpd_startSearch(struct mpd_connection *connection, int exact)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	if (exact) connection->request = strdup("find");
	else connection->request = strdup("search");
}

void mpd_startStatsSearch(struct mpd_connection *connection)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	connection->request = strdup("count");
}

void mpd_startPlaylistSearch(struct mpd_connection *connection, int exact)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	if (exact) connection->request = strdup("playlistfind");
	else connection->request = strdup("playlistsearch");
}

void mpd_startFieldSearch(struct mpd_connection *connection, int type)
{
	const char *strtype;
	int len;

	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES) {
		strcpy(connection->errorStr, "invalid type specified");
		connection->error = 1;
		return;
	}

	strtype = mpdTagItemKeys[type];

	len = 5+strlen(strtype)+1;
	connection->request = malloc(len);

	snprintf(connection->request, len, "list %c%s",
	         tolower(strtype[0]), strtype+1);
}

void
mpd_addConstraintSearch(struct mpd_connection *connection,
			int type, const char *name)
{
	const char *strtype;
	char *arg;
	int len;
	char *string;

	if (!connection->request) {
		strcpy(connection->errorStr, "no search in progress");
		connection->error = 1;
		return;
	}

	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES) {
		strcpy(connection->errorStr, "invalid type specified");
		connection->error = 1;
		return;
	}

	if (name == NULL) {
		strcpy(connection->errorStr, "no name specified");
		connection->error = 1;
		return;
	}

	string = strdup(connection->request);
	strtype = mpdTagItemKeys[type];
	arg = mpd_sanitizeArg(name);

	len = strlen(string)+1+strlen(strtype)+2+strlen(arg)+2;
	connection->request = realloc(connection->request, len);
	snprintf(connection->request, len, "%s %c%s \"%s\"",
	         string, tolower(strtype[0]), strtype+1, arg);

	free(string);
	free(arg);
}

void mpd_commitSearch(struct mpd_connection *connection)
{
	int len;

	if (!connection->request) {
		strcpy(connection->errorStr, "no search in progress");
		connection->error = 1;
		return;
	}

	len = strlen(connection->request)+2;
	connection->request = realloc(connection->request, len);
	connection->request[len-2] = '\n';
	connection->request[len-1] = '\0';
	mpd_sendInfoCommand(connection, connection->request);

	free(connection->request);
	connection->request = NULL;
}

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content, with full metadata, of a stored playlist.
 *
 */
void
mpd_sendListPlaylistInfoCommand(struct mpd_connection *connection, char *path)
{
	char *arg = mpd_sanitizeArg(path);
	int len = strlen("listplaylistinfo")+2+strlen(arg)+3;
	char *query = malloc(len);
	snprintf(query, len, "listplaylistinfo \"%s\"\n", arg);
	mpd_sendInfoCommand(connection, query);
	free(arg);
	free(query);
}

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content of a stored playlist.
 *
 */
void
mpd_sendListPlaylistCommand(struct mpd_connection *connection, char *path)
{
	char *arg = mpd_sanitizeArg(path);
	int len = strlen("listplaylist")+2+strlen(arg)+3;
	char *query = malloc(len);
	snprintf(query, len, "listplaylist \"%s\"\n", arg);
	mpd_sendInfoCommand(connection, query);
	free(arg);
	free(query);
}

void
mpd_sendPlaylistClearCommand(struct mpd_connection *connection, char *path)
{
	char *sPath = mpd_sanitizeArg(path);
	int len = strlen("playlistclear")+2+strlen(sPath)+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistclear \"%s\"\n", sPath);
	mpd_executeCommand(connection, string);
	free(sPath);
	free(string);
}

void mpd_sendPlaylistAddCommand(struct mpd_connection *connection,
                                char *playlist, char *path)
{
	char *sPlaylist = mpd_sanitizeArg(playlist);
	char *sPath = mpd_sanitizeArg(path);
	int len = strlen("playlistadd")+2+strlen(sPlaylist)+3+strlen(sPath)+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistadd \"%s\" \"%s\"\n", sPlaylist, sPath);
	mpd_executeCommand(connection, string);
	free(sPlaylist);
	free(sPath);
	free(string);
}

void mpd_sendPlaylistMoveCommand(struct mpd_connection *connection,
                                 char *playlist, int from, int to)
{
	char *sPlaylist = mpd_sanitizeArg(playlist);
	int len = strlen("playlistmove")+
	          2+strlen(sPlaylist)+3+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistmove \"%s\" \"%i\" \"%i\"\n",
	         sPlaylist, from, to);
	mpd_executeCommand(connection, string);
	free(sPlaylist);
	free(string);
}

void mpd_sendPlaylistDeleteCommand(struct mpd_connection *connection,
                                   char *playlist, int pos)
{
	char *sPlaylist = mpd_sanitizeArg(playlist);
	int len = strlen("playlistdelete")+2+strlen(sPlaylist)+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistdelete \"%s\" \"%i\"\n", sPlaylist, pos);
	mpd_executeCommand(connection, string);
	free(sPlaylist);
	free(string);
}

