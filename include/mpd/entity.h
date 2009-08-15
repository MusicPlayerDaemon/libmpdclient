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

#ifndef MPD_ENTITY_H
#define MPD_ENTITY_H

#include <mpd/song.h>
#include <mpd/directory.h>
#include <mpd/client.h>

/**
 * The type of a #mpd_entity object.
 */
enum mpd_entity_type {
	/**
	 * A directory (#mpd_directory) containing more entities.
	 */
	MPD_ENTITY_TYPE_DIRECTORY,

	/**
	 * A song file (#mpd_song) which can be added to the playlist.
	 */
	MPD_ENTITY_TYPE_SONG,

	/**
	 * A stored playlist (#mpd_stored_playlist).
	 */
	MPD_ENTITY_TYPE_PLAYLISTFILE,
};

/**
 * An "entity" is an object returned by commands like "lsinfo".  It is
 * an object wrapping all possible entity types.
 */
struct mpd_entity {
	/**
	 * The type of this entity.
	 */
	enum mpd_entity_type type;

	/**
	 * This union contains type-safe pointers to the real object.
	 * Check the entity type before attempting to obtain the
	 * object!
	 */
	union {
		/**
		 * Only valid if type==#MPD_ENTITY_TYPE_DIRECTORY.
		 */
		struct mpd_directory *directory;

		/**
		 * Only valid if type==#MPD_ENTITY_TYPE_SONG.
		 */
		struct mpd_song *song;

		/**
		 * Only valid if type==#MPD_ENTITY_TYPE_PLAYLISTFILE.
		 */
		struct mpd_stored_playlist *playlistFile;
	} info;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Releases an entity.  This also frees the wrapped object.
 */
void
mpd_entity_free(struct mpd_entity *entity);

/**
 * Receives the next entity from the MPD server.
 *
 * @return an entity object, or NULL on error or if the entity list is
 * finished
 */
struct mpd_entity *
mpd_recv_entity(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
