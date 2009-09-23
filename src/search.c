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

#include <mpd/search.h>
#include <mpd/send.h>
#include <mpd/pair.h>
#include <mpd/recv.h>
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool
mpd_search_db_songs(struct mpd_connection *connection, bool exact)
{
	if (mpd_error_is_defined(&connection->error))
		return false;

	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return false;
	}

	if (exact)
		connection->request = strdup("find");
	else
		connection->request = strdup("search");
	if (connection->request == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	return true;
}

bool
mpd_search_queue_songs(struct mpd_connection *connection, bool exact)
{
	if (mpd_error_is_defined(&connection->error))
		return false;

	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return false;
	}

	if (exact)
		connection->request = strdup("playlistfind");
	else
		connection->request = strdup("playlistsearch");
	if (connection->request == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	return true;
}

bool
mpd_search_db_tags(struct mpd_connection *connection, enum mpd_tag_type type)
{
	const char *strtype;
	int len;

	if (mpd_error_is_defined(&connection->error))
		return false;

	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return false;
	}

	strtype = mpd_tag_name(type);
	if (strtype == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_ARGUMENT);
		mpd_error_message(&connection->error,
				  "invalid type specified");
		return false;
	}

	len = 5+strlen(strtype)+1;
	connection->request = malloc(len);
	if (connection->request == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	snprintf(connection->request, len, "list %s", strtype);

	return true;
}

bool
mpd_count_db_songs(struct mpd_connection *connection)
{
	if (mpd_error_is_defined(&connection->error))
		return false;

	if (connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "search already in progress");
		return false;
	}

	connection->request = strdup("count");
	if (connection->request == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	return true;
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
	if (ret == NULL)
		return NULL;

	c = arg;
	rc = ret;
	for (i = strlen(arg)+1; i != 0; --i) {
		if (*c=='"' || *c=='\\')
			*rc++ = '\\';
		*(rc++) = *(c++);
	}

	return ret;
}

bool
mpd_search_add_constraint(struct mpd_connection *connection,
			  enum mpd_tag_type type, const char *name)
{
	size_t old_length;
	const char *strtype;
	char *arg, *request;
	int len;

	assert(name != NULL);

	if (mpd_error_is_defined(&connection->error))
		return false;

	if (!connection->request) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return false;
	}

	old_length = strlen(connection->request);

	strtype = mpd_tag_name(type);
	if (strtype == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_ARGUMENT);
		mpd_error_message(&connection->error,
				  "invalid type specified");
		return false;
	}

	arg = mpd_sanitize_arg(name);
	if (arg == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	len = 1 + strlen(strtype) + 2 + strlen(arg) + 2;
	request = realloc(connection->request, old_length + len);
	if (request == NULL) {
		free(arg);
		mpd_error_code(&connection->error, MPD_ERROR_OOM);
		return false;
	}

	connection->request = request;
	snprintf(connection->request + old_length, len, " %s \"%s\"",
		 strtype, arg);

	free(arg);
	return true;
}

bool
mpd_search_commit(struct mpd_connection *connection)
{
	bool success;

	if (mpd_error_is_defined(&connection->error))
		return false;

	if (connection->request == NULL) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "no search in progress");
		return false;
	}

	success = mpd_send_command(connection, connection->request, NULL);
	free(connection->request);
	connection->request = NULL;

	return success;
}

struct mpd_pair *
mpd_recv_pair_tag(struct mpd_connection *connection, enum mpd_tag_type type)
{
	const char *name;

	name = mpd_tag_name(type);
	if (name == NULL)
		return NULL;

	return mpd_recv_pair_named(connection, name);
}
