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

/*! \file
 * \brief MPD client library
 *
 * Search songs in the database or the queue.
 *
 * Do not include this header directly.  Use mpd/client.h instead.
 */

#ifndef MPD_DB_H
#define MPD_DB_H

#include <mpd/connection.h>
#include <mpd/tag.h>
#include <mpd/compiler.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @param connection a #mpd_connection
 * @param exact if to match exact
 *
 * Search for songs in the db given certain constraints
 * Use this method with mpd_search_add_constraint and mpd_search_commit
 * Use mpd_recv_entity to get results of this method
 */
bool
mpd_search_db_songs(struct mpd_connection *connection, bool exact);

/**
 * @param connection a #mpd_connection
 * @param exact if to match exact
 *
 * Search for songs in the queue given certain constraints.
 * Use this method with mpd_search_add_constraint and mpd_search_commit
 * Use mpd_recv_entity to get results of this method
 */
bool
mpd_search_queue_songs(struct mpd_connection *connection, bool exact);

/**
 * @param connection a #mpd_connection
 * @param type The type of the tags to search for
 *
 * Search for tags in the db given certain constraints
 * Use this method with mpd_search_add_constraint and mpd_search_commit
 * Use mpd_get_next_tag to get results of this method
 */
bool
mpd_search_db_tags(struct mpd_connection *connection, enum mpd_tag_type type);

/**
 * @param connection a #mpd_connection
 *
 * Counts the number of songs and the total playtime given certain constraints
 * Use this method with mpd_search_add_constraint and mpd_search_commit
 * Use mpd_get_stats to get results of this method
 */
bool mpd_count_db_songs(struct mpd_connection *connection);

/**
 * @param connection a #mpd_connection
 * @param type The tag type of the constraint
 * @param value The value of the constaint
 *
 * Add a constraint to a search.
 */
bool
mpd_search_add_constraint(struct mpd_connection *connection,
			  enum mpd_tag_type type, const char *value);

/**
 * @param connection a #mpd_connection
 *
 * Starts the real search with constraints added with
 * mpd_search_add_constraint.
 */
bool
mpd_search_commit(struct mpd_connection *connection);

/**
 * Same as mpd_recv_pair_named(), but the pair name is specified as
 * #mpd_tag_type.
 *
 * @param connection the connection to MPD
 * @param type one of the "real" tag types; #MPD_TAG_UNKNOWN and
 * #MPD_TAG_ANY are not allowed, while #MPD_TAG_FILE is
 * @return a pair, or NULL on error or if there are no more matching
 * pairs in this response
 */
mpd_malloc
struct mpd_pair *
mpd_recv_pair_tag(struct mpd_connection *connection, enum mpd_tag_type type);

#ifdef __cplusplus
}
#endif

#endif
