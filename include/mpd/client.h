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

#ifndef MPD_CLIENT_H
#define MPD_CLIENT_H

#include <mpd/connection.h>
#include <mpd/song.h>
#include <mpd/directory.h>

#ifdef WIN32
#  define __W32API_USE_DLLIMPORT__ 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mpd_TagItems
{
	MPD_TAG_ITEM_ARTIST,
	MPD_TAG_ITEM_ALBUM,
	MPD_TAG_ITEM_TITLE,
	MPD_TAG_ITEM_TRACK,
	MPD_TAG_ITEM_NAME,
	MPD_TAG_ITEM_GENRE,
	MPD_TAG_ITEM_DATE,
	MPD_TAG_ITEM_COMPOSER,
	MPD_TAG_ITEM_PERFORMER,
	MPD_TAG_ITEM_COMMENT,
	MPD_TAG_ITEM_DISC,
	MPD_TAG_ITEM_FILENAME,
	MPD_TAG_ITEM_ANY,
	MPD_TAG_NUM_OF_ITEM_TYPES
} mpd_TagItems;

extern const char *const mpdTagItemKeys[MPD_TAG_NUM_OF_ITEM_TYPES];

/* PLAYLISTFILE STUFF */

/* mpd_PlaylistFile
 * stores info about playlist file returned by lsinfo
 */
typedef struct _mpd_PlaylistFile {
	char * path;
} mpd_PlaylistFile;

/* mpd_newPlaylistFile
 * allocates memory for new mpd_PlaylistFile, path is set to NULL
 * free this memory with mpd_freePlaylistFile
 */
mpd_PlaylistFile * mpd_newPlaylistFile(void);

/* mpd_freePlaylist
 * free memory allocated for freePlaylistFile, will also free
 * path, so be careful
 */
void mpd_freePlaylistFile(mpd_PlaylistFile * playlist);

/* mpd_playlistFileDup
 * works like strdup, but for mpd_PlaylistFile
 */
mpd_PlaylistFile * mpd_playlistFileDup(const mpd_PlaylistFile * playlist);

/* INFO ENTITY STUFF */

/* the type of entity returned from one of the commands that generates info
 * use in conjunction with mpd_InfoEntity.type
 */
#define MPD_INFO_ENTITY_TYPE_DIRECTORY		0
#define MPD_INFO_ENTITY_TYPE_SONG		1
#define MPD_INFO_ENTITY_TYPE_PLAYLISTFILE	2

/* mpd_InfoEntity
 * stores info on stuff returned info commands
 */
typedef struct mpd_InfoEntity {
	/* the type of entity, use with MPD_INFO_ENTITY_TYPE_* to determine
	 * what this entity is (song, directory, etc...)
	 */
	int type;
	/* the actual data you want, mpd_song, mpd_directory, etc */
	union {
		struct mpd_directory *directory;
		struct mpd_song *song;
		mpd_PlaylistFile * playlistFile;
	} info;
} mpd_InfoEntity;

mpd_InfoEntity * mpd_newInfoEntity(void);

void mpd_freeInfoEntity(mpd_InfoEntity * entity);

/* INFO COMMANDS AND STUFF */

/* use this function to loop over after calling Info/Listall functions */
mpd_InfoEntity * mpd_getNextInfoEntity(struct mpd_connection *connection);

/* fetches the currently seeletect song (the song referenced by status->song
 * and status->songid*/
void mpd_sendCurrentSongCommand(struct mpd_connection *connection);

/* songNum of -1, means to display the whole list */
void mpd_sendPlaylistInfoCommand(struct mpd_connection *connection, int songNum);

/* songId of -1, means to display the whole list */
void mpd_sendPlaylistIdCommand(struct mpd_connection *connection, int songId);

/* use this to get the changes in the playlist since version _playlist_ */
void mpd_sendPlChangesCommand(struct mpd_connection *connection, long long playlist);

/**
 * @param connection: A valid and connected mpd_Connection.
 * @param playlist: The playlist version you want the diff with.
 * A more bandwidth efficient version of the mpd_sendPlChangesCommand.
 * It only returns the pos+id of the changes song.
 */
void mpd_sendPlChangesPosIdCommand(struct mpd_connection *connection, long long playlist);

/* recursivel fetches all songs/dir/playlists in "dir* (no metadata is
 * returned) */
void mpd_sendListallCommand(struct mpd_connection *connection, const char * dir);

/* same as sendListallCommand, but also metadata is returned */
void mpd_sendListallInfoCommand(struct mpd_connection *connection, const char * dir);

/* non-recursive version of ListallInfo */
void mpd_sendLsInfoCommand(struct mpd_connection *connection, const char * dir);

#define MPD_TABLE_ARTIST	MPD_TAG_ITEM_ARTIST
#define MPD_TABLE_ALBUM		MPD_TAG_ITEM_ALBUM
#define MPD_TABLE_TITLE		MPD_TAG_ITEM_TITLE
#define MPD_TABLE_FILENAME	MPD_TAG_ITEM_FILENAME

void mpd_sendSearchCommand(struct mpd_connection *connection, int table,
		const char * str);

void mpd_sendFindCommand(struct mpd_connection *connection, int table,
		const char * str);

/* LIST TAG COMMANDS */

/* use this function fetch next artist entry, be sure to free the returned
 * string.  NULL means there are no more.  Best used with sendListArtists
 */
char * mpd_getNextArtist(struct mpd_connection *connection);

char * mpd_getNextAlbum(struct mpd_connection *connection);

char * mpd_getNextTag(struct mpd_connection *connection, int type);

/* list artist or albums by artist, arg1 should be set to the artist if
 * listing albums by a artist, otherwise NULL for listing all artists or albums
 */
void mpd_sendListCommand(struct mpd_connection *connection, int table,
		const char * arg1);

/* SIMPLE COMMANDS */

void mpd_sendAddCommand(struct mpd_connection *connection, const char * file);

int mpd_sendAddIdCommand(struct mpd_connection *connection, const char *file);

void mpd_sendDeleteCommand(struct mpd_connection *connection, int songNum);

void mpd_sendDeleteIdCommand(struct mpd_connection *connection, int songNum);

void mpd_sendSaveCommand(struct mpd_connection *connection, const char * name);

void mpd_sendLoadCommand(struct mpd_connection *connection, const char * name);

void mpd_sendRmCommand(struct mpd_connection *connection, const char * name);

void mpd_sendRenameCommand(struct mpd_connection *connection, const char *from,
                           const char *to);

void mpd_sendShuffleCommand(struct mpd_connection *connection);

void mpd_sendClearCommand(struct mpd_connection *connection);

/* use this to start playing at the beginning, useful when in random mode */
#define MPD_PLAY_AT_BEGINNING	-1

void mpd_sendPlayCommand(struct mpd_connection *connection, int songNum);

void mpd_sendPlayIdCommand(struct mpd_connection *connection, int songNum);

void mpd_sendStopCommand(struct mpd_connection *connection);

void mpd_sendPauseCommand(struct mpd_connection *connection, int pauseMode);

void mpd_sendNextCommand(struct mpd_connection *connection);

void mpd_sendPrevCommand(struct mpd_connection *connection);

void mpd_sendMoveCommand(struct mpd_connection *connection, int from, int to);

void mpd_sendMoveIdCommand(struct mpd_connection *connection, int from, int to);

void mpd_sendSwapCommand(struct mpd_connection *connection, int song1, int song2);

void mpd_sendSwapIdCommand(struct mpd_connection *connection, int song1, int song2);

void mpd_sendSeekCommand(struct mpd_connection *connection, int song, int time);

void mpd_sendSeekIdCommand(struct mpd_connection *connection, int song, int time);

void mpd_sendRepeatCommand(struct mpd_connection *connection, int repeatMode);

void mpd_sendRandomCommand(struct mpd_connection *connection, int randomMode);

void mpd_sendSetvolCommand(struct mpd_connection *connection, int volumeChange);

/* WARNING: don't use volume command, its depreacted */
void mpd_sendVolumeCommand(struct mpd_connection *connection, int volumeChange);

void mpd_sendCrossfadeCommand(struct mpd_connection *connection, int seconds);

void mpd_sendUpdateCommand(struct mpd_connection *connection, const char *path);

/* returns the update job id, call this after a update command*/
int mpd_getUpdateId(struct mpd_connection *connection);

void mpd_sendPasswordCommand(struct mpd_connection *connection, const char * pass);

/* after executing a command, when your done with it to get its status
 * (you want to check connection->error for an error)
 */
void mpd_finishCommand(struct mpd_connection *connection);

/* command list stuff, use this to do things like add files very quickly */
void mpd_sendCommandListBegin(struct mpd_connection *connection);

void mpd_sendCommandListOkBegin(struct mpd_connection *connection);

void mpd_sendCommandListEnd(struct mpd_connection *connection);

/* advance to the next listOk
 * returns 0 if advanced to the next list_OK,
 * returns -1 if it advanced to an OK or ACK */
int mpd_nextListOkCommand(struct mpd_connection *connection);

/**
 * @param connection a #mpd_Connection
 *
 * Queries mpd for the allowed commands
 */
void mpd_sendCommandsCommand(struct mpd_connection *connection);

/**
 * @param connection a #mpd_Connection
 *
 * Queries mpd for the not allowed commands
 */
void mpd_sendNotCommandsCommand(struct mpd_connection *connection);

/**
 * @param connection a #mpd_Connection
 *
 * returns the next supported command.
 *
 * @returns a string, needs to be free'ed
 */
char *mpd_getNextCommand(struct mpd_connection *connection);

void mpd_sendUrlHandlersCommand(struct mpd_connection *connection);

char *mpd_getNextHandler(struct mpd_connection *connection);

void mpd_sendTagTypesCommand(struct mpd_connection *connection);

char *mpd_getNextTagType(struct mpd_connection *connection);

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content, with full metadata, of a stored playlist.
 *
 */
void mpd_sendListPlaylistInfoCommand(struct mpd_connection *connection, char *path);

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content of a stored playlist.
 *
 */
void mpd_sendListPlaylistCommand(struct mpd_connection *connection, char *path);

/**
 * @param connection a #mpd_Connection
 * @param exact if to match exact
 *
 * starts a search, use mpd_addConstraintSearch to add
 * a constraint to the search, and mpd_commitSearch to do the actual search
 */
void mpd_startSearch(struct mpd_connection *connection, int exact);

/**
 * @param connection a #mpd_Connection
 * @param type
 * @param name
 */
void mpd_addConstraintSearch(struct mpd_connection *connection, int type, const char *name);

/**
 * @param connection a #mpd_Connection
 */
void mpd_commitSearch(struct mpd_connection *connection);

/**
 * @param connection a #mpd_Connection
 * @param type The type to search for
 *
 * starts a search for fields... f.e. get a list of artists would be:
 * @code
 * mpd_startFieldSearch(connection, MPD_TAG_ITEM_ARTIST);
 * mpd_commitSearch(connection);
 * @endcode
 *
 * or get a list of artist in genre "jazz" would be:
 * @code
 * mpd_startFieldSearch(connection, MPD_TAG_ITEM_ARTIST);
 * mpd_addConstraintSearch(connection, MPD_TAG_ITEM_GENRE, "jazz")
 * mpd_commitSearch(connection);
 * @endcode
 *
 * mpd_startSearch will return  a list of songs (and you need mpd_getNextInfoEntity)
 * this one will return a list of only one field (the one specified with type) and you need
 * mpd_getNextTag to get the results
 */
void mpd_startFieldSearch(struct mpd_connection *connection, int type);

void mpd_startPlaylistSearch(struct mpd_connection *connection, int exact);

void mpd_startStatsSearch(struct mpd_connection *connection);

void mpd_sendPlaylistClearCommand(struct mpd_connection *connection, char *path);

void mpd_sendPlaylistAddCommand(struct mpd_connection *connection,
                                char *playlist, char *path);

void mpd_sendPlaylistMoveCommand(struct mpd_connection *connection,
                                 char *playlist, int from, int to);

void mpd_sendPlaylistDeleteCommand(struct mpd_connection *connection,
                                   char *playlist, int pos);

#ifdef __cplusplus
}
#endif

#endif
