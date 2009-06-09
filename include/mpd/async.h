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
 * \brief Asynchronous MPD connections
 *
 * This class provides a very basic interface to MPD connections.  It
 * does not know much about the MPD protocol, it does not know any
 * specific MPD command.
 *
 * The constructor expects a socket descriptor which is already
 * connected to MPD.  The first thing it does is read the server's
 * handshake code ("OK MPD 0.15.0").
 */

#ifndef MPD_ASYNC_H
#define MPD_ASYNC_H

#include <mpd/error.h>

#include <stdbool.h>
#include <stdarg.h>

/**
 * Event bit mask for polling.
 */
enum mpd_async_events {
	/** ready to read from the file descriptor */
	MPD_ASYNC_EVENT_READ = 1,

	/** ready to write to the file descriptor */
	MPD_ASYNC_EVENT_WRITE = 2,

	/** hangup detected */
	MPD_ASYNC_EVENT_HUP = 4,

	/** I/O error */
	MPD_ASYNC_EVENT_ERROR = 8,
};

/**
 * This opaque object represents an asynchronous connection to a MPD
 * server.  Call mpd_async_new() to create a new instance.
 */
struct mpd_async;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new asynchronous MPD connection, based on a stream socket
 * connected with MPD.
 *
 * @param fd the socket file descriptor of the stream connection to MPD
 * @return a mpd_async object, or NULL on out of memory
 */
struct mpd_async *
mpd_async_new(int fd);

/**
 * Closes the socket and frees memory.
 */
void
mpd_async_free(struct mpd_async *async);

/**
 * Determines whether this #mpd_async object is still connected and
 * valid.  If not, you may use mpd_async_get_error() and
 * mpd_async_get_error_message() to inspect the cause.
 */
bool
mpd_async_is_alive(const struct mpd_async *async);

/**
 * If mpd_async_is_alive() returns false, this function returns the
 * error code which caused this.
 */
enum mpd_error
mpd_async_get_error(const struct mpd_async *async);

/**
 * If mpd_async_is_alive() returns false, this function returns the
 * human readable error message which caused this.  This message is
 * optional, and may be NULL.  The pointer is invalidated by
 * mpd_async_free().
 */
const char *
mpd_async_get_error_message(const struct mpd_async *async);

/**
 * Returns the file descriptor which should be polled by the caller.
 * Do not use the file descriptor for anything except polling!  The
 * file descriptor never changes during the lifetime of this
 * #mpd_async object.
 */
int
mpd_async_fd(const struct mpd_async *async);

/**
 * Returns a bit mask of events which should be polled for.
 */
enum mpd_async_events
mpd_async_events(const struct mpd_async *async);

/**
 * Call this function when poll() has returned events for this
 * object's file descriptor.  libmpdclient will attempt to perform I/O
 * operations.
 *
 * @return false if the connection was closed due to an error
 */
bool
mpd_async_io(struct mpd_async *async, enum mpd_async_events events);

/**
 * Appends a command to the output buffer.
 *
 * @param async the connection
 * @param command the command name, followed by arguments, terminated by
 * NULL
 * @param args the argument list
 * @return true on success, false if the buffer is full
 */
bool
mpd_async_send_command_v(struct mpd_async *async, const char *command,
			 va_list args);

/**
 * Appends a command to the output buffer.
 *
 * @param async the connection
 * @param command the command name, followed by arguments, terminated by
 * NULL
 * @return true on success, false if the buffer is full
 */
bool
mpd_async_send_command(struct mpd_async *async, const char *command, ...);

/**
 * Receives a line from the input buffer.  The result will be
 * null-terminated, without the newline character.  The pointer is
 * only valid until the next async function is called.
 *
 * @param async the connection
 * @return a line on success, NULL otherwise
 */
char *
mpd_async_recv_line(struct mpd_async *async);

#ifdef __cplusplus
}
#endif

#endif
