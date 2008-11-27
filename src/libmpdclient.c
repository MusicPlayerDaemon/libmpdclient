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

#include "libmpdclient.h"
#include "resolver.h"
#include "str_pool.h"
#include "return_element.h"

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

void mpd_finishCommand(mpd_Connection * connection) {
	while(!connection->doneProcessing) {
		if(connection->doneListOk) connection->doneListOk = 0;
		mpd_getNextReturnElement(connection);
	}
}

static void mpd_finishListOkCommand(mpd_Connection * connection) {
	while(!connection->doneProcessing && connection->listOks &&
			!connection->doneListOk)
	{
		mpd_getNextReturnElement(connection);
	}
}

int mpd_nextListOkCommand(mpd_Connection * connection) {
	mpd_finishListOkCommand(connection);
	if(!connection->doneProcessing) connection->doneListOk = 0;
	if(connection->listOks == 0 || connection->doneProcessing) return -1;
	return 0;
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

static void mpd_sendInfoCommand(mpd_Connection * connection, char * command) {
	mpd_executeCommand(connection,command);
}

mpd_InfoEntity * mpd_getNextInfoEntity(mpd_Connection * connection) {
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

static char * mpd_getNextReturnElementNamed(mpd_Connection * connection,
		const char * name)
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

char *mpd_getNextTag(mpd_Connection *connection, int type)
{
	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES ||
	    type == MPD_TAG_ITEM_ANY)
		return NULL;
	if (type == MPD_TAG_ITEM_FILENAME)
		return mpd_getNextReturnElementNamed(connection, "file");
	return mpd_getNextReturnElementNamed(connection, mpdTagItemKeys[type]);
}

char * mpd_getNextArtist(mpd_Connection * connection) {
	return mpd_getNextReturnElementNamed(connection,"Artist");
}

char * mpd_getNextAlbum(mpd_Connection * connection) {
	return mpd_getNextReturnElementNamed(connection,"Album");
}

void mpd_sendPlaylistInfoCommand(mpd_Connection * connection, int songPos) {
	int len = strlen("playlistinfo")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistinfo \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendPlaylistIdCommand(mpd_Connection * connection, int id) {
	int len = strlen("playlistid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistid \"%i\"\n", id);
	mpd_sendInfoCommand(connection, string);
	free(string);
}

void mpd_sendPlChangesCommand(mpd_Connection * connection, long long playlist) {
	int len = strlen("plchanges")+2+LONGLONGLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "plchanges \"%lld\"\n", playlist);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendPlChangesPosIdCommand(mpd_Connection * connection, long long playlist) {
	int len = strlen("plchangesposid")+2+LONGLONGLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "plchangesposid \"%lld\"\n", playlist);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendListallCommand(mpd_Connection * connection, const char * dir) {
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("listall")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "listall \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void mpd_sendListallInfoCommand(mpd_Connection * connection, const char * dir) {
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("listallinfo")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "listallinfo \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void mpd_sendLsInfoCommand(mpd_Connection * connection, const char * dir) {
	char * sDir = mpd_sanitizeArg(dir);
	int len = strlen("lsinfo")+2+strlen(sDir)+3;
	char *string = malloc(len);
	snprintf(string, len, "lsinfo \"%s\"\n", sDir);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sDir);
}

void mpd_sendCurrentSongCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"currentsong\n");
}

void mpd_sendSearchCommand(mpd_Connection * connection, int table,
		const char * str)
{
	mpd_startSearch(connection, 0);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

void mpd_sendFindCommand(mpd_Connection * connection, int table,
		const char * str)
{
	mpd_startSearch(connection, 1);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

void mpd_sendListCommand(mpd_Connection * connection, int table,
		const char * arg1)
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

void mpd_sendAddCommand(mpd_Connection * connection, const char * file) {
	char * sFile = mpd_sanitizeArg(file);
	int len = strlen("add")+2+strlen(sFile)+3;
	char *string = malloc(len);
	snprintf(string, len, "add \"%s\"\n", sFile);
	mpd_executeCommand(connection,string);
	free(string);
	free(sFile);
}

int mpd_sendAddIdCommand(mpd_Connection *connection, const char *file)
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

void mpd_sendDeleteCommand(mpd_Connection * connection, int songPos) {
	int len = strlen("delete")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "delete \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendDeleteIdCommand(mpd_Connection * connection, int id) {
	int len = strlen("deleteid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "deleteid \"%i\"\n", id);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendSaveCommand(mpd_Connection * connection, const char * name) {
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("save")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "save \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void mpd_sendLoadCommand(mpd_Connection * connection, const char * name) {
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("load")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "load \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void mpd_sendRmCommand(mpd_Connection * connection, const char * name) {
	char * sName = mpd_sanitizeArg(name);
	int len = strlen("rm")+2+strlen(sName)+3;
	char *string = malloc(len);
	snprintf(string, len, "rm \"%s\"\n", sName);
	mpd_executeCommand(connection,string);
	free(string);
	free(sName);
}

void mpd_sendRenameCommand(mpd_Connection *connection, const char *from,
                           const char *to)
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

void mpd_sendShuffleCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"shuffle\n");
}

void mpd_sendClearCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"clear\n");
}

void mpd_sendPlayCommand(mpd_Connection * connection, int songPos) {
	int len = strlen("play")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "play \"%i\"\n", songPos);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendPlayIdCommand(mpd_Connection * connection, int id) {
	int len = strlen("playid")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "playid \"%i\"\n", id);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendStopCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"stop\n");
}

void mpd_sendPauseCommand(mpd_Connection * connection, int pauseMode) {
	int len = strlen("pause")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "pause \"%i\"\n", pauseMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendNextCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"next\n");
}

void mpd_sendMoveCommand(mpd_Connection * connection, int from, int to) {
	int len = strlen("move")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "move \"%i\" \"%i\"\n", from, to);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendMoveIdCommand(mpd_Connection * connection, int id, int to) {
	int len = strlen("moveid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "moveid \"%i\" \"%i\"\n", id, to);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendSwapCommand(mpd_Connection * connection, int song1, int song2) {
	int len = strlen("swap")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "swap \"%i\" \"%i\"\n", song1, song2);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendSwapIdCommand(mpd_Connection * connection, int id1, int id2) {
	int len = strlen("swapid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "swapid \"%i\" \"%i\"\n", id1, id2);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendSeekCommand(mpd_Connection * connection, int song, int time) {
	int len = strlen("seek")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "seek \"%i\" \"%i\"\n", song, time);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendSeekIdCommand(mpd_Connection * connection, int id, int time) {
	int len = strlen("seekid")+2+INTLEN+3+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "seekid \"%i\" \"%i\"\n", id, time);
	mpd_sendInfoCommand(connection,string);
	free(string);
}

void mpd_sendUpdateCommand(mpd_Connection * connection, const char * path) {
	char * sPath = mpd_sanitizeArg(path);
	int len = strlen("update")+2+strlen(sPath)+3;
	char *string = malloc(len);
	snprintf(string, len, "update \"%s\"\n", sPath);
	mpd_sendInfoCommand(connection,string);
	free(string);
	free(sPath);
}

int mpd_getUpdateId(mpd_Connection * connection) {
	char * jobid;
	int ret = 0;

	jobid = mpd_getNextReturnElementNamed(connection,"updating_db");
	if(jobid) {
		ret = atoi(jobid);
		free(jobid);
	}

	return ret;
}

void mpd_sendPrevCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"previous\n");
}

void mpd_sendRepeatCommand(mpd_Connection * connection, int repeatMode) {
	int len = strlen("repeat")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "repeat \"%i\"\n", repeatMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendRandomCommand(mpd_Connection * connection, int randomMode) {
	int len = strlen("random")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "random \"%i\"\n", randomMode);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendSetvolCommand(mpd_Connection * connection, int volumeChange) {
	int len = strlen("setvol")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "setvol \"%i\"\n", volumeChange);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendVolumeCommand(mpd_Connection * connection, int volumeChange) {
	int len = strlen("volume")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "volume \"%i\"\n", volumeChange);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendCrossfadeCommand(mpd_Connection * connection, int seconds) {
	int len = strlen("crossfade")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "crossfade \"%i\"\n", seconds);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendPasswordCommand(mpd_Connection * connection, const char * pass) {
	char * sPass = mpd_sanitizeArg(pass);
	int len = strlen("password")+2+strlen(sPass)+3;
	char *string = malloc(len);
	snprintf(string, len, "password \"%s\"\n", sPass);
	mpd_executeCommand(connection,string);
	free(string);
	free(sPass);
}

void mpd_sendCommandListBegin(mpd_Connection * connection) {
	if(connection->commandList) {
		strcpy(connection->errorStr,"already in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = COMMAND_LIST;
	mpd_executeCommand(connection,"command_list_begin\n");
}

void mpd_sendCommandListOkBegin(mpd_Connection * connection) {
	if(connection->commandList) {
		strcpy(connection->errorStr,"already in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = COMMAND_LIST_OK;
	mpd_executeCommand(connection,"command_list_ok_begin\n");
	connection->listOks = 0;
}

void mpd_sendCommandListEnd(mpd_Connection * connection) {
	if(!connection->commandList) {
		strcpy(connection->errorStr,"not in command list mode");
		connection->error = 1;
		return;
	}
	connection->commandList = 0;
	mpd_executeCommand(connection,"command_list_end\n");
}

void mpd_sendOutputsCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"outputs\n");
}

mpd_OutputEntity * mpd_getNextOutput(mpd_Connection * connection) {
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

void mpd_sendEnableOutputCommand(mpd_Connection * connection, int outputId) {
	int len = strlen("enableoutput")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "enableoutput \"%i\"\n", outputId);
	mpd_executeCommand(connection,string);
	free(string);
}

void mpd_sendDisableOutputCommand(mpd_Connection * connection, int outputId) {
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

void mpd_sendNotCommandsCommand(mpd_Connection * connection)
{
	mpd_executeCommand(connection, "notcommands\n");
}

/**
 * mpd_sendCommandsCommand
 * odd naming, but it gets the allowed commands
 */
void mpd_sendCommandsCommand(mpd_Connection * connection)
{
	mpd_executeCommand(connection, "commands\n");
}

/**
 * Get the next returned command
 */
char * mpd_getNextCommand(mpd_Connection * connection)
{
	return mpd_getNextReturnElementNamed(connection, "command");
}

void mpd_sendUrlHandlersCommand(mpd_Connection * connection)
{
	mpd_executeCommand(connection, "urlhandlers\n");
}

char * mpd_getNextHandler(mpd_Connection * connection)
{
	return mpd_getNextReturnElementNamed(connection, "handler");
}

void mpd_sendTagTypesCommand(mpd_Connection * connection)
{
	mpd_executeCommand(connection, "tagtypes\n");
}

char * mpd_getNextTagType(mpd_Connection * connection)
{
	return mpd_getNextReturnElementNamed(connection, "tagtype");
}

void mpd_startSearch(mpd_Connection *connection, int exact)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	if (exact) connection->request = strdup("find");
	else connection->request = strdup("search");
}

void mpd_startStatsSearch(mpd_Connection *connection)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	connection->request = strdup("count");
}

void mpd_startPlaylistSearch(mpd_Connection *connection, int exact)
{
	if (connection->request) {
		strcpy(connection->errorStr, "search already in progress");
		connection->error = 1;
		return;
	}

	if (exact) connection->request = strdup("playlistfind");
	else connection->request = strdup("playlistsearch");
}

void mpd_startFieldSearch(mpd_Connection *connection, int type)
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

void mpd_addConstraintSearch(mpd_Connection *connection, int type, const char *name)
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

void mpd_commitSearch(mpd_Connection *connection)
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
void mpd_sendListPlaylistInfoCommand(mpd_Connection *connection, char *path)
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
void mpd_sendListPlaylistCommand(mpd_Connection *connection, char *path)
{
	char *arg = mpd_sanitizeArg(path);
	int len = strlen("listplaylist")+2+strlen(arg)+3;
	char *query = malloc(len);
	snprintf(query, len, "listplaylist \"%s\"\n", arg);
	mpd_sendInfoCommand(connection, query);
	free(arg);
	free(query);
}

void mpd_sendPlaylistClearCommand(mpd_Connection *connection, char *path)
{
	char *sPath = mpd_sanitizeArg(path);
	int len = strlen("playlistclear")+2+strlen(sPath)+3;
	char *string = malloc(len);
	snprintf(string, len, "playlistclear \"%s\"\n", sPath);
	mpd_executeCommand(connection, string);
	free(sPath);
	free(string);
}

void mpd_sendPlaylistAddCommand(mpd_Connection *connection,
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

void mpd_sendPlaylistMoveCommand(mpd_Connection *connection,
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

void mpd_sendPlaylistDeleteCommand(mpd_Connection *connection,
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

