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

/*! \file
 * \brief MPD client library
 *
 * Manipulate stickers.
 *
 * Do not include this header directly.  Use mpd/client.h instead.
 */

#ifndef MPD_STICKER_H
#define MPD_STICKER_H

#include <mpd/compiler.h>

#include <stdbool.h>

struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/** Describes a sticker on a file. */
struct mpd_sticker;

/** Destroy a mpd_sticker.
 *
 * The next value is not destroyed and is returned.
 */
struct mpd_sticker* mpd_sticker_free(struct mpd_sticker* sticker);

/** Get uri from sticker. */
const char* mpd_sticker_get_uri(const struct mpd_sticker* sticker);

/** Get name from sticker. */
const char* mpd_sticker_get_name(const struct mpd_sticker* sticker);

/** Get value from sticker. */
const char* mpd_sticker_get_value(const struct mpd_sticker* sticker);

/**
 * Adds or replaces a sticker value.
 *
 * @param connection the connection to MPD
 * @param type the object type, e.g. "song"
 * @param uri the URI of the object
 * @param name the name of the sticker
 * @param value the value of the sticker
 * @return true on success, false on error
 */
bool
mpd_send_sticker_set(struct mpd_connection *connection, const char *type,
		     const char *uri, const char *name, const char *value);

/**
 * Shortcut for mpd_send_sticker_set() and mpd_response_finish().
 *
 * @param connection the connection to MPD
 * @param type the object type, e.g. "song"
 * @param uri the URI of the object
 * @param name the name of the sticker
 * @param value the value of the sticker
 * @return true on success, false on error
 */
bool
mpd_run_sticker_set(struct mpd_connection *connection, const char *type,
		    const char *uri, const char *name, const char *value);

/**
 * Deletes a sticker value.
 *
 * @param connection the connection to MPD
 * @param type the object type, e.g. "song"
 * @param uri the URI of the object
 * @param name the name of the sticker
 * @return true on success, false on error
 */
bool
mpd_send_sticker_delete(struct mpd_connection *connection, const char *type,
			const char *uri, const char *name);

/**
 * Shortcut for mpd_send_sticker_delete() and mpd_response_finish().
 *
 * @param connection the connection to MPD
 * @param type the object type, e.g. "song"
 * @param uri the URI of the object
 * @param name the name of the sticker
 * @return true on success, false on error
 */
bool
mpd_run_sticker_delete(struct mpd_connection *connection, const char *type,
		       const char *uri, const char *name);

/** Get a specific song's sticker.
 *
 * @param conn  current connection to MPD.
 * @param uri  uri of song.
 * @param key  sticker key.
 * @return  a mpd_sticker object. You need to destroy it yourself with \
 *          mpd_sticker_free().
 */
struct mpd_sticker* mpd_sticker_song_get(struct mpd_connection* conn, const char* uri, const char* key);

/** List every stickers on a song.
 *
 * @param conn  current connection to MPD.
 * @param uri  uri of song.
 * @return  a linked-list of mpd_sticker objects. You need to destroy them \
 *          yourself with mpd_sticker_free().
 */
struct mpd_sticker* mpd_sticker_song_list(struct mpd_connection* conn, const char* uri);

/** Find every files in a directory with a specific stickey set.
 *
 * @param conn  current connection to MPD.
 * @param dir  directory where to find.
 * @param key  key to find on songs.
 * @return  a linked-list of mpd_sticker objects. You need to destroy them \
 *          yourself with mpd_sticker_free().
 */
struct mpd_sticker* mpd_sticker_song_find(struct mpd_connection* conn, const char* dir, const char* key);

#ifdef __cplusplus
}
#endif

#endif /* MPD_STICKER_H */

