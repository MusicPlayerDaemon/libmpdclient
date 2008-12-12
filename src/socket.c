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

#include "socket.h"
#include "resolver.h"
#include "ierror.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef WIN32
#  include <ws2tcpip.h>
#  include <winsock.h>
#else
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <sys/un.h>
#endif

#ifndef MSG_DONTWAIT
#  define MSG_DONTWAIT 0
#endif

#ifdef WIN32
#  define SELECT_ERRNO_IGNORE   (errno == WSAEINTR || errno == WSAEINPROGRESS)
#  define SENDRECV_ERRNO_IGNORE SELECT_ERRNO_IGNORE
#else
#  define SELECT_ERRNO_IGNORE   (errno == EINTR)
#  define SENDRECV_ERRNO_IGNORE (errno == EINTR || errno == EAGAIN)
#  define winsock_dll_error(c)  0
#  define closesocket(s)        close(s)
#  define WSACleanup()          do { /* nothing */ } while (0)
#endif

void
mpd_socket_deinit(struct mpd_socket *s)
{
	if (s->fd >= 0)
		closesocket(s->fd);
}

#ifdef WIN32

static int do_connect_fail(struct mpd_socket *s,
                           const struct sockaddr *serv_addr, int addrlen)
{
	int iMode = 1; /* 0 = blocking, else non-blocking */
	if (connect(s->fd, serv_addr, addrlen) == SOCKET_ERROR)
		return 1;
	ioctlsocket(s->fd, FIONBIO, (u_long FAR*) &iMode);
	return 0;
}

#else /* !WIN32 (sane operating systems) */

static int do_connect_fail(struct mpd_socket *s,
                           const struct sockaddr *serv_addr, int addrlen)
{
	int flags;
	if (connect(s->fd, serv_addr, addrlen) < 0)
		return 1;
	flags = fcntl(s->fd, F_GETFL, 0);
	fcntl(s->fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

#endif /* !WIN32 */

/**
 * Wait for the socket to become readable.
 */
static int
mpd_socket_wait(struct mpd_socket *s)
{
	struct timeval tv;
	fd_set fds;
	int ret;

	assert(s->fd >= 0);

	while (1) {
		tv = s->timeout;
		FD_ZERO(&fds);
		FD_SET(s->fd, &fds);

		ret = select(s->fd + 1, &fds, NULL, NULL, &tv);
		if (ret > 0)
			return 0;

		if (ret == 0 || !SELECT_ERRNO_IGNORE)
			return -1;
	}
}

/**
 * Wait until the socket is connected and check its result.  Returns 1
 * on success, 0 on timeout, -errno on error.
 */
static int
mpd_socket_wait_connected(struct mpd_socket *s)
{
	int ret;
	int s_err = 0;
	socklen_t s_err_size = sizeof(s_err);

	ret = mpd_socket_wait(s);
	if (ret < 0)
		return 0;

	ret = getsockopt(s->fd, SOL_SOCKET, SO_ERROR,
			 (char*)&s_err, &s_err_size);
	if (ret < 0)
		return -errno;

	if (s_err != 0)
		return -s_err;

	return 1;
}

bool
mpd_socket_connect(struct mpd_socket *s, const char *host, int port,
		   struct mpd_error_info *error)
{
	struct resolver *resolver;
	const struct resolver_address *address;
	int ret;

	resolver = resolver_new(host, port);
	if (resolver == NULL) {
		mpd_error_code(error, MPD_ERROR_UNKHOST);
		mpd_error_printf(error, "host \"%s\" not found", host);
		return false;
	}

	while ((address = resolver_next(resolver)) != NULL) {
		s->fd = socket(address->family, SOCK_STREAM,
					  address->protocol);
		if (!mpd_socket_defined(s)) {
			mpd_error_code(error, MPD_ERROR_SYSTEM);
			mpd_error_printf(error,
					 "problems creating socket: %s",
					 strerror(errno));
			continue;
		}

		ret = do_connect_fail(s, address->addr, address->addrlen);
		if (ret != 0) {
			mpd_error_code(error, MPD_ERROR_CONNPORT);
			mpd_error_printf(error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(errno));

			closesocket(s->fd);
			s->fd = -1;
			continue;
		}

		ret = mpd_socket_wait_connected(s);
		if (ret > 0) {
			resolver_free(resolver);
			mpd_error_clear(error);
			return true;
		}

		if (ret == 0) {
			mpd_error_code(error, MPD_ERROR_NORESPONSE);
			mpd_error_printf(error,
					 "timeout in attempting to get a response from \"%s\" on port %i",
					 host, port);
		} else if (ret < 0) {
			mpd_error_code(error, MPD_ERROR_CONNPORT);
			mpd_error_printf(error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(-ret));
		}

		closesocket(s->fd);
		s->fd = -1;
	}

	resolver_free(resolver);
	return false;
}

static inline bool
mpd_socket_buffer_full(const struct mpd_socket *s)
{
	assert(s->buflen <= sizeof(s->buffer));

	return s->buflen == sizeof(s->buffer);
}

/**
 * Attempt to read data from the socket into the input buffer.
 *
 * @return true if data was received, false on error or timeout
 */
static bool
mpd_socket_recv(struct mpd_socket *s, struct mpd_error_info *error)
{
	int ret;
	ssize_t nbytes;

	assert(s != NULL);
	assert(s->buflen <= sizeof(s->buffer));
	assert(s->bufstart <= s->buflen);

	if (!mpd_socket_defined(s)) {
		mpd_error_code(error, MPD_ERROR_CONNCLOSED);
		mpd_error_message(error, "not connected");
		return false;
	}

	if (mpd_socket_buffer_full(s)) {
		/* delete consumed data from beginning of buffer */
		s->buflen -= s->bufstart;
		memmove(s->buffer, s->buffer + s->bufstart, s->buflen);
		s->bufstart = 0;
	}

	if (s->buflen >= sizeof(s->buffer)) {
		mpd_error_code(error, MPD_ERROR_BUFFEROVERRUN);
		mpd_error_message(error, "buffer overrun");
		return false;
	}

	while (1) {
		ret = mpd_socket_wait(s);
		if (ret < 0) {
			mpd_error_code(error, MPD_ERROR_TIMEOUT);
			mpd_error_message(error, "connection timeout");
			return false;
		}

		nbytes = read(s->fd, s->buffer + s->buflen,
			      sizeof(s->buffer) - s->buflen);
		if (nbytes > 0) {
			s->buflen += nbytes;
			return true;
		}

		if (nbytes == 0 || !SENDRECV_ERRNO_IGNORE) {
			mpd_error_code(error, MPD_ERROR_CONNCLOSED);
			mpd_error_message(error, "connection closed");
			return false;
		}
	}
}

char *
mpd_socket_recv_line(struct mpd_socket *s, struct mpd_error_info *error)
{
	char *newline;
	bool ret;

	assert(!mpd_error_is_defined(error));

	while (true) {
		char *start = s->buffer + s->bufstart;

		newline = memchr(start, '\n', s->buflen - s->bufstart);
		if (newline != NULL) {
			*newline++ = 0;
			s->bufstart = newline - s->buffer;
			return start;
		}

		ret = mpd_socket_recv(s, error);
		if (!ret)
			return NULL;
	}
}

bool
mpd_socket_send(struct mpd_socket *s, const void *data0, size_t length,
		struct mpd_error_info *error)
{
	const unsigned char *data = data0;
	struct timeval tv = s->timeout;
	ssize_t nbytes;

	while (length > 0) {
		nbytes = send(s->fd, data, length, MSG_DONTWAIT);
		if (nbytes > 0) {
			data += nbytes;
			length -= nbytes;
		} else if (SENDRECV_ERRNO_IGNORE) {
			fd_set fds;
			int ret;

			FD_ZERO(&fds);
			FD_SET(s->fd, &fds);

			ret = select(s->fd + 1, NULL, &fds, NULL, &tv);
			if (ret == 0) {
				mpd_error_code(error, MPD_ERROR_TIMEOUT);
				mpd_error_message(error,
						  "timeout while sending to MPD");
				return false;
			} else if (ret < 0 && !SELECT_ERRNO_IGNORE) {
				mpd_error_code(error, MPD_ERROR_SYSTEM);
				mpd_error_printf(error, "select() failed: %s",
						 strerror(errno));
				return false;
			}
		} else {
			mpd_error_code(error, MPD_ERROR_SENDING);
			mpd_error_printf(error,
					 "Sending to MPD failed: %s",
					 strerror(errno));
			return false;
		}
	}

	return true;
}
