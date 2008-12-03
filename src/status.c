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

#include <mpd/status.h>
#include "internal.h"

#include <stdlib.h>
#include <string.h>

void mpd_sendStatusCommand(struct mpd_connection * connection) {
	mpd_executeCommand(connection,"status\n");
}

struct mpd_status * mpd_getStatus(struct mpd_connection * connection) {
	struct mpd_status * status;

	/*mpd_executeCommand(connection,"status\n");

	if (connection->error) return NULL;*/

	if (connection->doneProcessing || (connection->listOks &&
	   connection->doneListOk))
	{
		return NULL;
	}

	if (!connection->returnElement) mpd_getNextReturnElement(connection);
	if (connection->error)
		return NULL;

	status = malloc(sizeof(struct mpd_status));
	status->volume = -1;
	status->repeat = 0;
	status->random = 0;
	status->playlist = -1;
	status->playlistLength = -1;
	status->state = -1;
	status->song = 0;
	status->songid = 0;
	status->elapsedTime = 0;
	status->totalTime = 0;
	status->bitRate = 0;
	status->sampleRate = 0;
	status->bits = 0;
	status->channels = 0;
	status->crossfade = -1;
	status->error = NULL;
	status->updatingDb = 0;

	while (connection->returnElement) {
		struct mpd_return_element * re = connection->returnElement;
		if (strcmp(re->name,"volume")==0) {
			status->volume = atoi(re->value);
		}
		else if (strcmp(re->name,"repeat")==0) {
			status->repeat = atoi(re->value);
		}
		else if (strcmp(re->name,"random")==0) {
			status->random = atoi(re->value);
		}
		else if (strcmp(re->name,"playlist")==0) {
			status->playlist = strtol(re->value,NULL,10);
		}
		else if (strcmp(re->name,"playlistlength")==0) {
			status->playlistLength = atoi(re->value);
		}
		else if (strcmp(re->name,"bitrate")==0) {
			status->bitRate = atoi(re->value);
		}
		else if (strcmp(re->name,"state")==0) {
			if (strcmp(re->value,"play")==0) {
				status->state = MPD_STATUS_STATE_PLAY;
			}
			else if (strcmp(re->value,"stop")==0) {
				status->state = MPD_STATUS_STATE_STOP;
			}
			else if (strcmp(re->value,"pause")==0) {
				status->state = MPD_STATUS_STATE_PAUSE;
			}
			else {
				status->state = MPD_STATUS_STATE_UNKNOWN;
			}
		}
		else if (strcmp(re->name,"song")==0) {
			status->song = atoi(re->value);
		}
		else if (strcmp(re->name,"songid")==0) {
			status->songid = atoi(re->value);
		}
		else if (strcmp(re->name,"time")==0) {
			char * tok = strchr(re->value,':');
			/* the second strchr below is a safety check */
			if (tok && (strchr(tok,0) > (tok+1))) {
				/* atoi stops at the first non-[0-9] char: */
				status->elapsedTime = atoi(re->value);
				status->totalTime = atoi(tok+1);
			}
		}
		else if (strcmp(re->name,"error")==0) {
			status->error = strdup(re->value);
		}
		else if (strcmp(re->name,"xfade")==0) {
			status->crossfade = atoi(re->value);
		}
		else if (strcmp(re->name,"updating_db")==0) {
			status->updatingDb = atoi(re->value);
		}
		else if (strcmp(re->name,"audio")==0) {
			char * tok = strchr(re->value,':');
			if (tok && (strchr(tok,0) > (tok+1))) {
				status->sampleRate = atoi(re->value);
				status->bits = atoi(++tok);
				tok = strchr(tok,':');
				if (tok && (strchr(tok,0) > (tok+1)))
					status->channels = atoi(tok+1);
			}
		}

		mpd_getNextReturnElement(connection);
		if (connection->error) {
			free(status);
			return NULL;
		}
	}

	if (connection->error) {
		free(status);
		return NULL;
	}
	else if (status->state<0) {
		strcpy(connection->errorStr,"state not found");
		connection->error = 1;
		free(status);
		return NULL;
	}

	return status;
}

void mpd_freeStatus(struct mpd_status * status) {
	if (status->error) free(status->error);
	free(status);
}

