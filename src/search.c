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

#include <mpd/search.h>
#include <mpd/send.h>
#include <mpd/pair.h>
#include "internal.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
mpd_search_db_songs(struct mpd_connection *connection,
		    bool exact)
{
	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	if (exact)
		connection->request = strdup("find");
	else
		connection->request = strdup("search");
}

void
mpd_search_playlist_songs(struct mpd_connection *connection,
			  bool exact)
{
	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	if (exact)
		connection->request = strdup("playlistfind");
	else
		connection->request = strdup("playlistsearch");
}

void
mpd_search_db_tags(struct mpd_connection *connection,
		   enum mpd_tag_type type)
{
	const char *strtype;
	int len;

	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	if (type >= MPD_TAG_TYPE_COUNT) {
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
mpd_count_db_songs(struct mpd_connection *connection)
{
	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return;
	}

	connection->request = strdup("count");
}


static char *
mpd_sanitize_arg(const char * arg)
{
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

void
mpd_search_add_constraint(struct mpd_connection *connection,
			  enum mpd_tag_type type,
			  const char *name)
{
	size_t old_length;
	const char *strtype;
	char *arg;
	int len;

	if (!connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return;
	}

	if (type >= MPD_TAG_TYPE_COUNT) {
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

	old_length = strlen(connection->request);

	strtype = mpdTagItemKeys[type];
	arg = mpd_sanitize_arg(name);

	len = 1 + strlen(strtype) + 2 + strlen(arg) + 2;
	connection->request = realloc(connection->request, old_length + len);
	snprintf(connection->request + old_length, len, " %c%s \"%s\"",
		 tolower(strtype[0]), strtype+1, arg);

	free(arg);
}

void
mpd_search_commit(struct mpd_connection *connection)
{
	if (!connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return;
	}

	mpd_send_command(connection, connection->request, NULL);

	connection->request = NULL;
}

char *mpd_get_next_tag(struct mpd_connection *connection,
		       enum mpd_tag_type type)
{
	if (type >= MPD_TAG_TYPE_COUNT ||
	    type == MPD_TAG_TYPE_ANY)
		return NULL;
	if (type == MPD_TAG_TYPE_FILENAME)
		return mpd_recv_value_named(connection, "file");
	return mpd_recv_value_named(connection, mpdTagItemKeys[type]);
}
