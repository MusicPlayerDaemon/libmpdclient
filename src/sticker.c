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

#include <mpd/sticker.h>
#include <mpd/connection.h>
#include <mpd/send.h>
#include <mpd/recv.h>
#include <mpd/pair.h>
#include "internal.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct mpd_sticker {
	struct mpd_sticker* next;  /** next sticker in linked list */

	char* uri;                 /** uri of sticker */
	char* name;                /** sticker key */
	char* value;               /** sticker value */
};

/** Create a new sticker object. */
static struct mpd_sticker* mpd_sticker_new(const char* uri, const char* name, const char* value)
{
	struct mpd_sticker* new = malloc(sizeof(struct mpd_sticker));
	new->uri = strdup(uri);
	new->name = strdup(name);
	new->value = strdup(value);
	new->next = NULL;
	return new;
}

const char* mpd_sticker_get_uri(const struct mpd_sticker* sticker)
{
	return sticker->uri;
}

const char* mpd_sticker_get_name(const struct mpd_sticker* sticker)
{
	return sticker->name;
}

const char* mpd_sticker_get_value(const struct mpd_sticker* sticker)
{
	return sticker->value;
}

struct mpd_sticker* mpd_sticker_free(struct mpd_sticker* sticker)
{
	struct mpd_sticker* next = sticker->next;

	free(sticker->uri);
	free(sticker->name);
	free(sticker->value);
	free(sticker);

	return next;
}

bool mpd_sticker_song_set(struct mpd_connection* conn, const char* uri, const char* key, const char* value)
{
	return mpd_send_command(conn, "sticker", "set", "song", uri, key, value, NULL);
}

struct mpd_sticker* mpd_sticker_song_get(struct mpd_connection* conn, const char* uri, const char* key)
{
	struct mpd_pair *pair;
	struct mpd_sticker* sticker = NULL;

	if(!mpd_send_command(conn, "sticker", "get", "song", uri, key, NULL))
		return NULL;

	pair = mpd_recv_pair_named(conn, "sticker");
	if (pair != NULL) {
		const char* eq = strchr(pair->value, '=');
		if(eq)
			sticker = mpd_sticker_new(uri, key, eq + 1);
		mpd_return_pair(conn, pair);
	}
	return sticker;
}

/** Receive list of stickers attached to a file */
static struct mpd_sticker* mpd_sticker_recv_list(struct mpd_connection* conn, struct mpd_sticker* head, const char* uri)
{
	struct mpd_pair *pair;
	while((pair = mpd_recv_pair(conn)) && !strcmp(pair->name, "sticker"))
	{
		struct mpd_sticker* new;
		char* eq = strchr(pair->value, '=');
		if(eq)
		{
			*eq++ = 0;
			new = mpd_sticker_new(uri, pair->value, eq);
			new->next = head;
			head = new;
		}
		mpd_return_pair(conn, pair);
	}
	mpd_enqueue_pair(conn, pair);

	if (mpd_error_is_defined(&conn->error))
		while(head)
			head = mpd_sticker_free(head);

	return head;
}

struct mpd_sticker* mpd_sticker_song_list(struct mpd_connection* conn, const char* uri)
{
	if(!mpd_send_command(conn, "sticker", "list", "song", uri, NULL))
		return NULL;

	return mpd_sticker_recv_list(conn, NULL, uri);
}

bool mpd_sticker_song_delete(struct mpd_connection* conn, const char* uri, const char* key)
{
	return mpd_send_command(conn, "sticker", "delete", "song", uri, key, NULL);
}

struct mpd_sticker* mpd_sticker_song_find(struct mpd_connection* conn, const char* dir, const char* value)
{
	struct mpd_pair* pair;
	struct mpd_sticker* sticker = NULL;

	if(!mpd_send_command(conn, "sticker", "find", "song", dir, value, NULL))
		return NULL;

	while((pair = mpd_recv_pair_named(conn, "file")))
	{
		char* uri = strdup(pair->value);

		mpd_return_pair(conn, pair);
		sticker = mpd_sticker_recv_list(conn, sticker, uri);
		free(uri);
		if(!sticker)
			return NULL;
	}

	return sticker;
}

