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

static void
mpd_sendInfoCommand(struct mpd_connection *connection, char *command)
{
	mpd_send_command(connection, command, NULL);
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
	while (connection->pair != NULL) {
		const struct mpd_pair *pair = connection->pair;

		if (strcmp(pair->name, name) == 0)
			return strdup(pair->value);

		mpd_getNextReturnElement(connection);
	}

	return NULL;
}

char *mpd_getNextTag(struct mpd_connection *connection, int type)
{
	if (type < 0 || type >= MPD_TAG_TYPE_COUNT ||
	    type == MPD_TAG_TYPE_ANY)
		return NULL;
	if (type == MPD_TAG_TYPE_FILENAME)
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
mpd_sendSearchCommand(struct mpd_connection *connection, int table,
		      const char *str)
{
	mpd_startSearch(connection, 0);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

void mpd_send_find(struct mpd_connection *connection, int table,
			 const char * str)
{
	mpd_startSearch(connection, 1);
	mpd_addConstraintSearch(connection, table, str);
	mpd_commitSearch(connection);
}

int
mpd_sendAddIdCommand(struct mpd_connection *connection, const char *file)
{
	bool ret;
	int retval = -1;
	char *string;

	ret = mpd_send_addid(connection, file);
	if (!ret)
		return -1;

	string = mpd_getNextReturnElementNamed(connection, "Id");
	if (string) {
		retval = atoi(string);
		free(string);
	}
	
	return retval;
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

void mpd_sendCommandListBegin(struct mpd_connection *connection)
{
	if (connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "already in command list mode");
		return;
	}
	connection->commandList = COMMAND_LIST;
	mpd_send_command(connection, "command_list_begin", NULL);
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
	mpd_send_command(connection, "command_list_ok_begin", NULL);
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
	mpd_send_command(connection, "command_list_end", NULL);
}

/**
 * Get the next returned command
 */
char * mpd_getNextCommand(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection, "command");
}

char * mpd_getNextHandler(struct mpd_connection *connection)
{
	return mpd_getNextReturnElementNamed(connection, "handler");
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

	if (type < 0 || type >= MPD_TAG_TYPE_COUNT) {
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

	if (type < 0 || type >= MPD_TAG_TYPE_COUNT) {
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
	if (!connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return;
	}

	mpd_sendInfoCommand(connection, connection->request);

	connection->request = NULL;
}
