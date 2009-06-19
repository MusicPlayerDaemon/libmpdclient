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

#ifndef MPD_ENTITY_H
#define MPD_ENTITY_H

#include <mpd/song.h>
#include <mpd/directory.h>
#include <mpd/client.h>

/* the type of entity returned from one of the commands that generates info
 * use in conjunction with mpd_entity.type
 */
enum mpd_entity_type {
	MPD_ENTITY_TYPE_DIRECTORY,
	MPD_ENTITY_TYPE_SONG,
	MPD_ENTITY_TYPE_PLAYLISTFILE
};

/* mpd_entity
 * stores info on stuff returned info commands
 */
typedef struct mpd_entity {
	/* the type of entity, use with MPD_ENTITY_TYPE_* to determine
	 * what this entity is (song, directory, etc...)
	 */
	enum mpd_entity_type type;
	/* the actual data you want, mpd_song, mpd_directory, etc */
	union {
		struct mpd_directory *directory;
		struct mpd_song *song;
		struct mpd_stored_playlist *playlistFile;
	} info;
} mpd_entity;

#ifdef __cplusplus
extern "C" {
#endif

void mpd_entity_free(mpd_entity * entity);

/* use this function to loop over after calling Info/Listall functions */
mpd_entity * mpd_get_next_entity(struct mpd_connection *connection);

#ifdef __cplusplus
}
#endif

#endif
