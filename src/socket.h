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

#ifndef MPD_SOCKET_H
#define MPD_SOCKET_H

#include <stdbool.h>
#include <sys/select.h>

struct mpd_error_info;

/**
 * A socket connection between the client (that's us) and the MPD
 * server.
 */
struct mpd_socket {
	/** the socket file descriptor */
	int fd;

	struct timeval timeout;
};

/**
 * Checks whether the socket object is defined, i.e. the OS level
 * socket was created, but it may not be connected yet.
 */
static inline bool
mpd_socket_defined(const struct mpd_socket *s)
{
	return s->fd >= 0;
}

/**
 * Modifies the timeout for sending and receiving.
 */
static inline void
mpd_socket_set_timeout(struct mpd_socket *s, const struct timeval *timeout)
{
	s->timeout = *timeout;
}

/**
 * Initialize a socket object.  This does not create the OS level
 * socket, and does not attempt to connect it.
 */
static inline void
mpd_socket_init(struct mpd_socket *s, const struct timeval *timeout)
{
	s->fd = -1;
	mpd_socket_set_timeout(s, timeout);
}

/**
 * Free all resources of the socket object.
 */
void
mpd_socket_deinit(struct mpd_socket *s);

/**
 * Connects the socket to the specified host and port.
 *
 * @return false if an error occured
 */
bool
mpd_socket_connect(struct mpd_socket *s, const char *host, int port,
		   struct mpd_error_info *error);

#endif
