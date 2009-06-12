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

#ifndef MPD_COMMAND_H
#define MPD_COMMAND_H

#include <stdbool.h>

enum {
	/**
	 *  use this to start playing at the beginning, useful when in
	 *  random mode
	 */
	MPD_PLAY_AT_BEGINNING = -1,
};

struct mpd_connection;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Player commands
 *
 */

/**
 * Fetches the currently selected song (the song referenced by
 * status->song and status->songid).
 */
bool
mpd_send_currentsong(struct mpd_connection *connection);

bool
mpd_send_play(struct mpd_connection *connection, int song_pos);

bool
mpd_send_playid(struct mpd_connection *connection, int id);

bool
mpd_send_stop(struct mpd_connection *connection);

bool
mpd_send_pause(struct mpd_connection *connection, int mode);

bool
mpd_send_next(struct mpd_connection *connection);

bool
mpd_send_previous(struct mpd_connection *connection);

bool
mpd_send_seek(struct mpd_connection *connection, int song_pos, int time);

bool
mpd_send_seekid(struct mpd_connection *connection, int id, int time);

bool
mpd_send_repeat(struct mpd_connection *connection, int mode);

bool
mpd_send_random(struct mpd_connection *connection, int mode);

bool
mpd_send_single(struct mpd_connection *connection, int mode);

bool
mpd_send_consume(struct mpd_connection *connection, int mode);

bool
mpd_send_crossfade(struct mpd_connection *connection, int seconds);


/*
 * Playlist commands
 *
 */

/**
 * song_pos of -1, means to display the whole list
 */
bool
mpd_send_playlistinfo(struct mpd_connection *connection, int song_pos);

/**
 * songId of -1, means to display the whole list
 */
bool
mpd_send_playlistid(struct mpd_connection *connection, int id);

/**
 * use this to get the changes in the playlist since version _playlist_
 */
bool
mpd_send_plchanges(struct mpd_connection *connection, long long playlist);

/**
 * A more bandwidth efficient version of the mpd_send_plchanges.
 * It only returns the pos+id of the changes song.
 *
 * @param connection A valid and connected mpd_connection.
 * @param playlist The playlist version you want the diff with.
 */
bool
mpd_send_plchangesposid(struct mpd_connection *connection, long long playlist);

bool
mpd_send_add(struct mpd_connection *connection, const char *file);

bool
mpd_send_addid(struct mpd_connection *connection, const char *file);

bool
mpd_send_delete(struct mpd_connection *connection, int song_pos);

bool
mpd_send_deleteid(struct mpd_connection *connection, int id);

bool
mpd_send_shuffle(struct mpd_connection *connection);

bool
mpd_send_shuffle_range(struct mpd_connection *connection, unsigned start, unsigned end);

bool
mpd_send_clear(struct mpd_connection *connection);

bool
mpd_send_move(struct mpd_connection *connection, int from, int to);

bool
mpd_send_moveid(struct mpd_connection *connection, int from, int to);

bool
mpd_send_swap(struct mpd_connection *connection, int pos1, int pos2);

bool
mpd_send_swapid(struct mpd_connection *connection, int id1, int id2);


/*
 * Stored playlist commands
 *
 */

/**
 * @param connection a #mpd_connection
 * @param path	the path to the playlist.
 *
 * List the content of a stored playlist.
 *
 */
bool
mpd_send_listplaylist(struct mpd_connection *connection, char *name);

/**
 * @param connection a #mpd_connection
 * @param path	the path to the playlist.
 *
 * List the content, with full metadata, of a stored playlist.
 *
 */
bool
mpd_send_listplaylistinfo(struct mpd_connection *connection, char *name);

bool
mpd_send_playlistclear(struct mpd_connection *connection, char *name);

bool
mpd_send_playlistadd(struct mpd_connection *connection, char *name,
		     const char *path);

bool
mpd_send_playlistmove(struct mpd_connection *connection, char *name,
		      int from, int to);

bool
mpd_send_playlistdelete(struct mpd_connection *connection, char *name,
			int pos);

bool
mpd_send_save(struct mpd_connection *connection, const char *name);

bool
mpd_send_load(struct mpd_connection *connection, const char *name);

bool
mpd_send_rename(struct mpd_connection *connection,
		const char *from, const char *to);

bool
mpd_send_rm(struct mpd_connection *connection, const char *name);


/*
 * Music database commands
 *
 */

/**
 * recursively fetches all songs/dir/playlists in "dir" (no metadata
 * is returned)
 */
bool
mpd_send_listall(struct mpd_connection *connection, const char *dir);

/**
 * same as mpd_send_listall(), but also metadata is returned
 */
bool
mpd_send_listallinfo(struct mpd_connection *connection, const char *dir);


/**
 * non-recursive version of mpd_send_listallinfo()
 */
bool
mpd_send_lsinfo(struct mpd_connection *connection, const char *dir);

bool
mpd_send_update(struct mpd_connection *connection, const char *path);


/*
 * Mixer commands
 *
 */

bool
mpd_send_setvol(struct mpd_connection *connection, int change);

/**
 * WARNING: don't use volume command, its depreacted
 */
bool
mpd_send_volume(struct mpd_connection *connection, int change);


/*
 * Connection commands
 *
 */

bool
mpd_send_password(struct mpd_connection *connection, const char *password);

/**
 * @param connection a #mpd_connection
 *
 * Queries mpd for the allowed commands
 */
bool
mpd_send_commands(struct mpd_connection *connection);

/**
 * @param connection a #mpd_connection
 *
 * Queries mpd for the not allowed commands
 */
bool
mpd_send_notcommands(struct mpd_connection *connection);

bool
mpd_send_urlhandlers(struct mpd_connection *connection);

bool
mpd_send_tagtypes(struct mpd_connection *connection);


/*
 * Output commands
 *
 */

bool
mpd_send_outputs(struct mpd_connection *connection);

bool
mpd_send_enable_output(struct mpd_connection *connection, unsigned output_id);

bool
mpd_send_disable_output(struct mpd_connection *connection, unsigned output_id);

#ifdef __cplusplus
}
#endif

#endif
