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

#ifndef MPD_IERROR_H
#define MPD_IERROR_H

#include <mpd/error.h>
#include <mpd/protocol.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Structure which holds detailed information about an error, both
 * machine and human readable.
 */
struct mpd_error_info {
	/**
	 * The error code of this error.  None of the variables below
	 * are valid if this is set to MPD_ERROR_SUCCESS.
	 */
	enum mpd_error code;

	/**
	 * An ACK code returned by MPD.  This field is only valid if
	 * #code is MPD_ERROR_ACK.
	 */
	enum mpd_ack ack;

	/**
	 * The command list index of the command which emitted this
	 * error.  Zero if no command list was used.
	 */
	int at;

	/**
	 * Human readable error message; may be NULL if not available.
	 * This pointer is allocated on the heap, and must be freed by
	 * calling mpd_error_clear() or mpd_error_deinit().
	 */
	char *message;
};

/**
 * Initialize a new mpd_error_info struct.
 */
static inline void
mpd_error_init(struct mpd_error_info *error)
{
	assert(error != NULL);

	error->code = MPD_ERROR_SUCCESS;
}

/**
 * Free memory allocated by an mpd_error_info struct.
 */
void
mpd_error_deinit(struct mpd_error_info *error);

/**
 * Clear the error (if any), and free its memory.
 */
static inline void
mpd_error_clear(struct mpd_error_info *error)
{
	mpd_error_deinit(error);

	error->code = MPD_ERROR_SUCCESS;
}

/**
 * Returns true if an error has occured.
 */
static inline bool
mpd_error_is_defined(const struct mpd_error_info *error)
{
	return error->code != MPD_ERROR_SUCCESS;
}

/**
 * Sets an error code.
 */
static inline void
mpd_error_code(struct mpd_error_info *error, enum mpd_error code)
{
	assert(!mpd_error_is_defined(error));

	error->code = code;
	error->message = NULL;
}

/**
 * Sets an ACK error code.
 */
static inline void
mpd_error_ack(struct mpd_error_info *error, enum mpd_ack ack, int at)
{
	mpd_error_code(error, MPD_ERROR_ACK);
	error->ack = ack;
	error->at = at;
}

/**
 * Sets an error message.  Prior to that, an error code must have been
 * set.
 */
void
mpd_error_message(struct mpd_error_info *error, const char *message);

/**
 * Sets an error message (non-terminated string with specified
 * length).  Prior to that, an error code must have been set.
 */
void
mpd_error_message_n(struct mpd_error_info *error,
		    const char *message, size_t length);

/**
 * Sets an error message (printf() like format).  Prior to that, an
 * error code must have been set.
 */
void
mpd_error_printf(struct mpd_error_info *error, const char *fmt, ...);

#endif
