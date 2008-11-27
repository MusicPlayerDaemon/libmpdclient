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

#include "stats.h"
#include <stdlib.h>
#include <string.h>

void mpd_sendStatsCommand(mpd_Connection * connection) {
	mpd_executeCommand(connection,"stats\n");
}

mpd_Stats * mpd_getStats(mpd_Connection * connection) {
	mpd_Stats * stats;

	/*mpd_executeCommand(connection,"stats\n");

	if(connection->error) return NULL;*/

	if(connection->doneProcessing || (connection->listOks &&
	   connection->doneListOk))
	{
		return NULL;
	}

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	stats = malloc(sizeof(mpd_Stats));
	stats->numberOfArtists = 0;
	stats->numberOfAlbums = 0;
	stats->numberOfSongs = 0;
	stats->uptime = 0;
	stats->dbUpdateTime = 0;
	stats->playTime = 0;
	stats->dbPlayTime = 0;

	if(connection->error) {
		free(stats);
		return NULL;
	}
	while(connection->returnElement) {
		mpd_ReturnElement * re = connection->returnElement;
		if(strcmp(re->name,"artists")==0) {
			stats->numberOfArtists = atoi(re->value);
		}
		else if(strcmp(re->name,"albums")==0) {
			stats->numberOfAlbums = atoi(re->value);
		}
		else if(strcmp(re->name,"songs")==0) {
			stats->numberOfSongs = atoi(re->value);
		}
		else if(strcmp(re->name,"uptime")==0) {
			stats->uptime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"db_update")==0) {
			stats->dbUpdateTime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"playtime")==0) {
			stats->playTime = strtol(re->value,NULL,10);
		}
		else if(strcmp(re->name,"db_playtime")==0) {
			stats->dbPlayTime = strtol(re->value,NULL,10);
		}

		mpd_getNextReturnElement(connection);
		if(connection->error) {
			free(stats);
			return NULL;
		}
	}

	if(connection->error) {
		free(stats);
		return NULL;
	}

	return stats;
}

void mpd_freeStats(mpd_Stats * stats) {
	free(stats);
}

mpd_SearchStats * mpd_getSearchStats(mpd_Connection * connection)
{
	mpd_SearchStats * stats;
	mpd_ReturnElement * re;

	if (connection->doneProcessing ||
	    (connection->listOks && connection->doneListOk)) {
		return NULL;
	}

	if (!connection->returnElement) mpd_getNextReturnElement(connection);

	if (connection->error)
		return NULL;

	stats = malloc(sizeof(mpd_SearchStats));
	stats->numberOfSongs = 0;
	stats->playTime = 0;

	while (connection->returnElement) {
		re = connection->returnElement;

		if (strcmp(re->name, "songs") == 0) {
			stats->numberOfSongs = atoi(re->value);
		} else if (strcmp(re->name, "playtime") == 0) {
			stats->playTime = strtol(re->value, NULL, 10);
		}

		mpd_getNextReturnElement(connection);
		if (connection->error) {
			free(stats);
			return NULL;
		}
	}

	if (connection->error) {
		free(stats);
		return NULL;
	}

	return stats;
}

void mpd_freeSearchStats(mpd_SearchStats * stats)
{
	free(stats);
}

