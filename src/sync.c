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

#include "sync.h"
#include <mpd/async.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static bool
ignore_errno(int e)
{
#ifdef WIN32
	return e == WSAEINTR || e == WSAEINPROGRESS;
#else
	return e == EINTR;
#endif
}

static enum mpd_async_event
mpd_sync_poll(struct mpd_async *async, struct timeval *tv)
{
	int fd;
	fd_set rfds, wfds, efds;
	int ret;
	enum mpd_async_event events;

	if (mpd_async_get_error(async) != MPD_ERROR_SUCCESS)
		return 0;

	fd = mpd_async_fd(async);

	while (1) {
		events = mpd_async_events(async);
		if (events == 0)
			return 0;

		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&efds);

		if (events & MPD_ASYNC_EVENT_READ)
			FD_SET(fd, &rfds);
		if (events & MPD_ASYNC_EVENT_WRITE)
			FD_SET(fd, &wfds);
		if (events & (MPD_ASYNC_EVENT_HUP|MPD_ASYNC_EVENT_ERROR))
			FD_SET(fd, &efds);

		ret = select(fd + 1, &rfds, &wfds, &efds, tv);
		if (ret > 0) {
			if (!FD_ISSET(fd, &rfds))
				events &= ~MPD_ASYNC_EVENT_READ;
			if (!FD_ISSET(fd, &wfds))
				events &= ~MPD_ASYNC_EVENT_WRITE;
			if (!FD_ISSET(fd, &efds))
				events &= ~(MPD_ASYNC_EVENT_HUP|
					    MPD_ASYNC_EVENT_ERROR);

			return events;
		}

		if (ret == 0 || ignore_errno(errno))
			return 0;
	}
}

static bool
mpd_sync_io(struct mpd_async *async, struct timeval *tv)
{
	enum mpd_async_event events = mpd_sync_poll(async, tv);

	if (events)
		return mpd_async_io(async, events);
	else
		return false;
}

bool
mpd_sync_send_command_v(struct mpd_async *async, const struct timeval *tv0,
			const char *command, va_list args)
{
	struct timeval tv = *tv0;
	va_list copy;
	bool success;

	while (true) {
		va_copy(copy, args);
		success = mpd_async_send_command_v(async, command, copy);
		va_end(copy);

		if (success)
			return true;

		if (mpd_async_get_error(async) != MPD_ERROR_SUCCESS ||
		    !mpd_sync_io(async, &tv))
			return false;
	}
}

bool
mpd_sync_send_command(struct mpd_async *async, const struct timeval *tv,
		      const char *command, ...)
{
	va_list args;
	bool success;

	va_start(args, command);
	success = mpd_sync_send_command_v(async, tv, command, args);
	va_end(args);

	return success;
}

char *
mpd_sync_recv_line(struct mpd_async *async, const struct timeval *tv0)
{
	struct timeval tv = *tv0;
	char *line;

	while (true) {
		line = mpd_async_recv_line(async);
		if (line != NULL)
			return line;

		if (mpd_async_get_error(async) != MPD_ERROR_SUCCESS ||
		    !mpd_sync_io(async, &tv))
			return NULL;
	}
}
