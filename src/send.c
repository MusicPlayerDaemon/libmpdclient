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

#include <mpd/send.h>
#include "internal.h"
#include "quote.h"

#include <stdarg.h>
#include <string.h>

static bool
mpd_can_send(struct mpd_connection *connection)
{
	if (!mpd_socket_defined(&connection->socket)) {
		mpd_error_code(&connection->error, MPD_ERROR_CONNCLOSED);
		mpd_error_message(&connection->error, "connection closed");
		return false;
	}

	if (connection->receiving && !connection->commandList) {
		mpd_error_code(&connection->error, MPD_ERROR_STATE);
		mpd_error_message(&connection->error,
				  "not done processing current command");
		return false;
	}

	return true;
}

/**
 * Sends a buffer to the MPD server.
 *
 * @param connection the connection to the MPD server
 * @param p a pointer to the first byte of the buffer
 * @param length the length of the buffer
 * @return true on success
 */
static bool
mpd_send(struct mpd_connection *connection, const void *p, size_t length)
{
	if (!mpd_can_send(connection))
		return false;

	mpd_clear_error(connection);

	return mpd_socket_send(&connection->socket, p, length,
			       &connection->error);
}

bool
mpd_send_command(struct mpd_connection *connection, const char *command, ...)
{
	char buffer[1024];
	size_t length;
	va_list ap;
	const char *p;
	bool ret;

	if (!mpd_can_send(connection))
		return false;

	length = strlen(command);
	if (length + 1 >= sizeof(buffer)) {
		mpd_error_code(&connection->error, MPD_ERROR_ARG);
		mpd_error_message(&connection->error, "command too long");
		return false;
	}

	memcpy(buffer, command, length);

	va_start(ap, command);

	while ((p = va_arg(ap, const char *)) != NULL) {
		assert(length < sizeof(buffer));

		buffer[length++] = ' ';

		p = quote(buffer + length, buffer + sizeof(buffer), p);
		assert(p == NULL ||
		       (p >= buffer && p <= buffer + sizeof(buffer)));
		if (p == NULL || p >= buffer + sizeof(buffer)) {
			mpd_error_code(&connection->error, MPD_ERROR_ARG);
			mpd_error_message(&connection->error,
					  "argument list too long");
			return false;
		}

		length = p - buffer;
	}

	va_end(ap);

	assert(length < sizeof(buffer));
	buffer[length++] = '\n';

	ret = mpd_send(connection, buffer, length);
	if (!ret)
		return false;

	if (!connection->commandList)
		connection->receiving = true;
	else if (connection->commandList == COMMAND_LIST_OK)
		connection->listOks++;

	return true;
}
