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
#include "internal.h"
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
	for (i = strlen(arg)+1; i != 0; --i) {
		if (*c=='"' || *c=='\\')
			*rc++ = '\\';
		*(rc++) = *(c++);
	}

	return ret;
}

void mpd_finishCommand(struct mpd_connection *connection)
{
	while (!connection->doneProcessing) {
		if (connection->doneListOk) connection->doneListOk = 0;
		mpd_getNextReturnElement(connection);
	}
}

static void mpd_finishListOkCommand(struct mpd_connection *connection)
{
	while (!connection->doneProcessing && connection->listOks &&
			!connection->doneListOk)
	{
		mpd_getNextReturnElement(connection);
	}
}

int mpd_nextListOkCommand(struct mpd_connection *connection)
{
	mpd_finishListOkCommand(connection);
	if (!connection->doneProcessing) connection->doneListOk = 0;
	if (connection->listOks == 0 || connection->doneProcessing) return -1;
	return 0;
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

static void
mpd_sendInfoCommand(struct mpd_connection *connection, char *command)
{
	mpd_executeCommand(connection,command);
}

static char *
mpd_getNextReturnElementNamed(struct mpd_connection *connection,
			      const char *name)
{
	if (connection->doneProcessing || (connection->listOks &&
				connection->doneListOk))
	{
		return NULL;
	}

	mpd_getNextReturnElement(connection);
	while (connection->returnElement) {
		struct mpd_return_element *re = connection->returnElement;

		if (strcmp(re->name,name)==0) return strdup(re->value);
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
	const char *st;
	int len;
	char *string;

	if (table == MPD_TABLE_ARTIST)
		st = "artist";
	else if (table == MPD_TABLE_ALBUM)
		st = "album";
	else {
		mpd_error_code(&connection->error, MPD_ERROR_ARG);
		mpd_error_message(&connection->error,
				  "unknown table for list");
		return;
	}
	if (arg1) {
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
	if (jobid) {
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
	if (connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already in command list mode");
		return;
	}
	connection->commandList = COMMAND_LIST;
	mpd_executeCommand(connection,"command_list_begin\n");
}

void mpd_sendCommandListOkBegin(struct mpd_connection *connection)
{
	if (connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already in command list mode");
		return;
	}
	connection->commandList = COMMAND_LIST_OK;
	mpd_executeCommand(connection,"command_list_ok_begin\n");
	connection->listOks = 0;
}

void mpd_sendCommandListEnd(struct mpd_connection *connection)
{
	if (!connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "not in command list mode");
		return;
	}
	connection->commandList = 0;
	mpd_executeCommand(connection,"command_list_end\n");
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
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	if (exact) connection->request = strdup("find");
	else connection->request = strdup("search");
}

void mpd_startStatsSearch(struct mpd_connection *connection)
{
	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	connection->request = strdup("count");
}

void mpd_startPlaylistSearch(struct mpd_connection *connection, int exact)
{
	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
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
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES) {
		mpd_error_code(&connection->error, MPD_ERROR_ARG);
		mpd_error_message(&connection->error,
				  "invalid type specified");
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
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return;
	}

	if (type < 0 || type >= MPD_TAG_NUM_OF_ITEM_TYPES) {
		mpd_error_code(&connection->error, MPD_ERROR_ARG);
		mpd_error_message(&connection->error,
				  "invalid type specified");
		return;
	}

	if (name == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_ARG);
		mpd_error_message(&connection->error, "no name specified");
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
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
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

