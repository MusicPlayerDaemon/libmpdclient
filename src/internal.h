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

#ifndef MPD_INTERNAL_H
#define MPD_INTERNAL_H

#include "ierror.h"
#include <mpd/connection.h>

/* mpd_Connection
 * holds info about connection to mpd
 * use error, and errorStr to detect errors
 */
struct mpd_connection {
	/* use this to check the version of mpd */
	int version[3];

	struct mpd_error_info error;

	/* DON'T TOUCH any of the rest of this stuff */
	int sock;
	char buffer[16384];
	size_t buflen;
	size_t bufstart;
	int doneProcessing;
	int listOks;
	int doneListOk;
	int commandList;
	struct mpd_return_element *returnElement;
	struct timeval timeout;
	char *request;
	int idle;
	void (*notify_cb)(struct mpd_connection *connection,
			  unsigned flags, void *userdata);
	void (*startIdle)(struct mpd_connection *connection);
	void (*stopIdle)(struct mpd_connection *connection);
	void *userdata;
#ifdef MPD_GLIB
        int source_id;
#endif
};

#endif
