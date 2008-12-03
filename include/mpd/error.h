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

#ifndef MPD_ERROR_H
#define MPD_ERROR_H

enum mpd_error {
	/** no error */
	MPD_ERROR_SUCCESS = 0,

	/** timeout trying to talk to mpd */
	MPD_ERROR_TIMEOUT = 10,

	/** system error */
	MPD_ERROR_SYSTEM,

	/** unknown host */
	MPD_ERROR_UNKHOST,

	/** problems connecting to port on host */
	MPD_ERROR_CONNPORT,

	/** mpd not running on port at host */
	MPD_ERROR_NOTMPD,

	/** no response on attempting to connect */
	MPD_ERROR_NORESPONSE,

	/** error sending command */
	MPD_ERROR_SENDING,

	/** connection closed by mpd */
	MPD_ERROR_CONNCLOSED,

	/** ACK returned! */
	MPD_ERROR_ACK,

	/** Buffer was overrun! */
	MPD_ERROR_BUFFEROVERRUN,
};

#endif
