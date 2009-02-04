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

#ifndef MPD_CONNECTION_H
#define MPD_CONNECTION_H

#include <mpd/protocol.h>
#include <mpd/error.h>

#include <stdlib.h>

/* mpd_Connection
 * holds info about connection to mpd
 * use error, and errorStr to detect errors
 */
struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/* mpd_newConnection
 * use this to open a new connection
 * you should use mpd_closeConnection, when your done with the connection,
 * even if an error has occurred
 * _timeout_ is the connection timeout period in seconds
 */
struct mpd_connection *
mpd_newConnection(const char *host, int port, float timeout);

void mpd_setConnectionTimeout(struct mpd_connection *connection,
			      float timeout);

/* mpd_closeConnection
 * use this to close a connection and free'ing subsequent memory
 */
void mpd_closeConnection(struct mpd_connection *connection);

/**
 * Returns the libmpdclient error code.  MPD_ERROR_SUCCESS means no
 * error occured.
 */
enum mpd_error
mpd_get_error(const struct mpd_connection *connection);

/**
 * Returns the human-readable (English) libmpdclient error string.
 * Calling this function is only valid if an error really occured.
 * Check with mpd_get_error().
 */
const char *
mpd_get_error_string(const struct mpd_connection *connection);

/**
 * Returns the error code returned from the server.  Calling this
 * function is only valid if mpd_get_error() returned MPD_ERROR_ACK.
 */
enum mpd_ack
mpd_get_server_error(const struct mpd_connection *connection);

/* mpd_clearError
 * clears error
 */
void mpd_clearError(struct mpd_connection *connection);

/**
 * Returns a three-tuple containing the major, minor and patch version
 * of the MPD server.
 */
const int *
mpd_get_server_version(const struct mpd_connection *connection);

/*
 * TODO: Following methods should be internal
 */
#define COMMAND_LIST    1
#define COMMAND_LIST_OK 2

void mpd_getNextReturnElement(struct mpd_connection *connection);

void
mpd_executeCommand(struct mpd_connection *connection, const char *command);

#ifdef __cplusplus
}
#endif

#endif
