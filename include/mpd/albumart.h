/* libmpdclient
   (c) 2003-2019 The Music Player Daemon Project
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
 * Do not include this header directly.  Use mpd/client.h instead.
 */

#ifndef MPD_ALBUMART_H
#define MPD_ALBUMART_H

#include "compiler.h"

#include <stdbool.h>
#include <stddef.h>

struct mpd_connection;

struct mpd_albumart {
        /** the binary data */
        char *data;

        /** the size of the albumart */
        unsigned size;
        /** bytes read */
        unsigned binary;
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sends the "getalbumart" command to MPD.  Call mpd_recv_pair() to
 * read response lines.  Use mpd_parse_fingerprint_type() to check
 * each pair's name; the pair's value then contains the actual
 * fingerprint.
 *
 * @param connection a valid and connected #mpd_connection
 * @return true on success
 */
bool
mpd_send_getalbumart(struct mpd_connection *connection, 
                                   const char *uri, 
                                   const unsigned offset);

bool
mpd_free_albumart(struct mpd_albumart * albumart);

mpd_malloc
struct mpd_albumart *
mpd_recv_albumart(struct mpd_connection *connection);

/**
 * Shortcut for mpd_send_getalbumart(), mpd_recv_albumart() and
 * mpd_response_finish().
 *
 * @param connection a valid and connected #mpd_connection
 * @return a pointer to the struct mpd_albumart on success or NULL on error
 */
mpd_malloc
struct mpd_albumart *
mpd_run_getalbumart(struct mpd_connection *connection,
				   const char *uri,
				   const unsigned offset);

#ifdef __cplusplus
}
#endif

#endif
