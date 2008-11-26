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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdlib.h>
#include "return_element.h"

enum mpd_ack {
	MPD_ACK_ERROR_UNK = -1,

	MPD_ACK_ERROR_NOT_LIST = 1,
	MPD_ACK_ERROR_ARG = 2,
	MPD_ACK_ERROR_PASSWORD = 3,
	MPD_ACK_ERROR_PERMISSION = 4,
	MPD_ACK_ERROR_UNKNOWN_CMD = 5,

	MPD_ACK_ERROR_NO_EXIST = 50,
	MPD_ACK_ERROR_PLAYLIST_MAX = 51,
	MPD_ACK_ERROR_SYSTEM = 52,
	MPD_ACK_ERROR_PLAYLIST_LOAD = 53,
	MPD_ACK_ERROR_UPDATE_ALREADY = 54,
	MPD_ACK_ERROR_PLAYER_SYNC = 55,
	MPD_ACK_ERROR_EXIST = 56,
};

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

/* mpd_Connection
 * holds info about connection to mpd
 * use error, and errorStr to detect errors
 */
typedef struct _mpd_Connection {
	/* use this to check the version of mpd */
	int version[3];
	/* IMPORTANT, you want to get the error messages from here */
	char errorStr[512];
	enum mpd_ack errorCode;
	int errorAt;
	/* this will be set to MPD_ERROR_* if there is an error, 0 if not */
	enum mpd_error error;
	/* DON'T TOUCH any of the rest of this stuff */
	int sock;
	char buffer[16384];
	size_t buflen;
	size_t bufstart;
	int doneProcessing;
	int listOks;
	int doneListOk;
	int commandList;
	mpd_ReturnElement * returnElement;
	struct timeval timeout;
	char *request;
	int idle;
	void (*notify_cb) (struct _mpd_Connection *connection, unsigned flags, void *userdata);
	void (*startIdle) (struct _mpd_Connection *connection);
	void (*stopIdle) (struct _mpd_Connection *connection);
	void *userdata;
#ifdef MPD_GLIB
        int source_id;
#endif
} mpd_Connection;

/* mpd_newConnection
 * use this to open a new connection
 * you should use mpd_closeConnection, when your done with the connection,
 * even if an error has occurred
 * _timeout_ is the connection timeout period in seconds
 */
mpd_Connection * mpd_newConnection(const char * host, int port, float timeout);

void mpd_setConnectionTimeout(mpd_Connection * connection, float timeout);

/* mpd_closeConnection
 * use this to close a connection and free'ing subsequent memory
 */
void mpd_closeConnection(mpd_Connection * connection);

/* mpd_clearError
 * clears error
 */
void mpd_clearError(mpd_Connection * connection);

/*
 * TODO: Following methods should be internal
 */
#define COMMAND_LIST    1
#define COMMAND_LIST_OK 2

void mpd_getNextReturnElement(mpd_Connection * connection);

int mpd_recv(mpd_Connection *connection);

void mpd_executeCommand(mpd_Connection *connection,
			       const char *command);

#endif
