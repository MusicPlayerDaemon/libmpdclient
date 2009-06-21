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
#  define WSACleanup()          do { /* nothing */ } while (0)
#endif

#ifdef WIN32

bool
mpd_socket_global_init(struct mpd_error_info *error)
{
	WSADATA wsaData;

	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0 ||
			LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2 ) {
		mpd_error_code(error, MPD_ERROR_SYSTEM);
		mpd_error_message(error,
				  "Could not find usable WinSock DLL");
		return false;
	}

	return true;
}

#endif

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

static int do_connect_fail(int fd,
                           const struct sockaddr *serv_addr, int addrlen)
{
	int flags;
	if (connect(fd, serv_addr, addrlen) < 0)
		return 1;
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

#endif /* !WIN32 */

/**
 * Wait for the socket to become readable.
 */
static int
mpd_socket_wait(int fd, struct timeval *tv)
{
	fd_set fds;
	int ret;

	assert(fd >= 0);

	while (1) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		ret = select(fd + 1, &fds, NULL, NULL, tv);
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
mpd_socket_wait_connected(int fd, struct timeval *tv)
{
	int ret;
	int s_err = 0;
	socklen_t s_err_size = sizeof(s_err);

	ret = mpd_socket_wait(fd, tv);
	if (ret < 0)
		return 0;

	ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,
			 (char*)&s_err, &s_err_size);
	if (ret < 0)
		return -errno;

	if (s_err != 0)
		return -s_err;

	return 1;
}

int
mpd_socket_connect(const char *host, int port, const struct timeval *tv0,
		   struct mpd_error_info *error)
{
	struct timeval tv = *tv0;
	struct resolver *resolver;
	const struct resolver_address *address;
	int fd, ret;

	resolver = resolver_new(host, port);
	if (resolver == NULL) {
		mpd_error_code(error, MPD_ERROR_UNKHOST);
		mpd_error_printf(error, "host \"%s\" not found", host);
		return -1;
	}

	assert(!mpd_error_is_defined(error));

	while ((address = resolver_next(resolver)) != NULL) {
		fd = socket(address->family, SOCK_STREAM, address->protocol);
		if (fd < 0) {
			mpd_error_clear(error);
			mpd_error_code(error, MPD_ERROR_SYSTEM);
			mpd_error_printf(error,
					 "problems creating socket: %s",
					 strerror(errno));
			continue;
		}

		ret = do_connect_fail(fd, address->addr, address->addrlen);
		if (ret != 0) {
			mpd_error_clear(error);
			mpd_error_code(error, MPD_ERROR_CONNPORT);
			mpd_error_printf(error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(errno));

			mpd_socket_close(fd);
			continue;
		}

		ret = mpd_socket_wait_connected(fd, &tv);
		if (ret > 0) {
			resolver_free(resolver);
			mpd_error_clear(error);
			return fd;
		}

		if (ret == 0) {
			mpd_error_clear(error);
			mpd_error_code(error, MPD_ERROR_NORESPONSE);
			mpd_error_printf(error,
					 "timeout in attempting to get a response from \"%s\" on port %i",
					 host, port);
		} else if (ret < 0) {
			mpd_error_clear(error);
			mpd_error_code(error, MPD_ERROR_CONNPORT);
			mpd_error_printf(error,
					 "problems connecting to \"%s\" on port %i: %s",
					 host, port, strerror(-ret));
		}

		mpd_socket_close(fd);
	}

	resolver_free(resolver);
	return -1;
}

int
mpd_socket_close(int fd)
{
#ifndef WIN32
	return close(fd);
#else
	return closesocket(fd);
#endif
}
