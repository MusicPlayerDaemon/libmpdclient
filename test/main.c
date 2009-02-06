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

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/entity.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BRIGHT 1
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BG_BLACK 40
#define COLOR_CODE 0x1B

#define LOG_INFO(x, ...) {printf("    [info]" x "\n", __VA_ARGS__);}
#define LOG_WARNING(x, ...) \
{\
	fprintf(stderr, "%c[%d;%d;%dm[WARNING](%s:%d) %s : " x "\n", COLOR_CODE, BRIGHT, YELLOW, BG_BLACK, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);\
	printf("%c[%dm", 0x1B, 0);\
}

#define LOG_ERROR(x, ...) \
{\
	fprintf(stderr, "%c[%d;%d;%dm[ERROR](%s:%d) %s : " x "\n", COLOR_CODE, BRIGHT, RED, BG_BLACK, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);\
	printf("%c[%dm", 0x1B, 0);\
}

#define START_TEST(description, method, ...) \
{\
	printf("[Start Test] " description "\n");\
	if (method(__VA_ARGS__) < 0)\
		printf("%c[%d;%d;%dm[End Test: ERROR]\n", COLOR_CODE, BRIGHT, RED, BG_BLACK);\
	else\
		printf("%c[%d;%d;%dm[End Test: OK]\n", COLOR_CODE, BRIGHT, GREEN, BG_BLACK);\
	printf("%c[%dm", 0x1B, 0);\
}

#define CHECK_CONNECTION(conn) \
	if (mpd_get_error(conn) != MPD_ERROR_SUCCESS) { \
		LOG_ERROR("%s", mpd_get_error_string(conn)); \
		return -1; \
	}

static int
test_new_connection(struct mpd_connection **conn)
{
	const char *hostname = getenv("MPD_HOST");
	const char *port = getenv("MPD_PORT");

	if (hostname == NULL)
		hostname = "localhost";
	if (port == NULL)
		port = "6600";

	*conn = mpd_newConnection(hostname, atoi(port), 10);

	if (!*conn || mpd_get_error(*conn) != MPD_ERROR_SUCCESS) {
		LOG_ERROR("%s", mpd_get_error_string(*conn));
		mpd_closeConnection(*conn);
		*conn = NULL;
		return -1;
	}
	return 0;
}

static int
test_version(struct mpd_connection *conn)
{
	int i, total = -1;
	for (i=0; i<3; ++i) {
		LOG_INFO("version[%i]: %i", i, mpd_get_server_version(conn)[i]);
		total += mpd_get_server_version(conn)[i];
	}
	/* Check if at least one of the three number is positive */
	return total;
}

static void
print_status(struct mpd_status *status)
{
	LOG_INFO("volume: %i", mpd_status_get_volume(status));
	LOG_INFO("repeat: %i", mpd_status_get_repeat(status));
	LOG_INFO("random: %i", mpd_status_get_random(status));
	LOG_INFO("playlist: %lli", mpd_status_get_playlist(status));
	LOG_INFO("playlistLength: %i", mpd_status_get_playlist_length(status));

	if (mpd_status_get_state(status) == MPD_STATUS_STATE_PLAY ||
	    mpd_status_get_state(status) == MPD_STATUS_STATE_PAUSE) {
		LOG_INFO("song: %i", mpd_status_get_song(status));
		LOG_INFO("elaspedTime: %i", mpd_status_get_elapsed_time(status));
		LOG_INFO("totalTime: %i", mpd_status_get_total_time(status));
		LOG_INFO("bitRate: %i", mpd_status_get_bit_rate(status));
		LOG_INFO("sampleRate: %i", mpd_status_get_sample_rate(status));
		LOG_INFO("bits: %i", mpd_status_get_bits(status));
		LOG_INFO("channels: %i", mpd_status_get_channels(status));
	}
}

static void
print_song(struct mpd_song *song)
{
	LOG_INFO("file: %s", song->file);
	if (song->artist)
		LOG_INFO("artist: %s", song->artist);
	if (song->album)
		LOG_INFO("album: %s", song->album);
	if (song->title)
		LOG_INFO("title: %s", song->title);
	if (song->track)
		LOG_INFO("track: %s", song->track);
	if (song->name)
		LOG_INFO("name: %s", song->name);
	if (song->date)
		LOG_INFO("date: %s", song->date);
	if (song->time!=MPD_SONG_NO_TIME)
		LOG_INFO("time: %i", song->time);
	if (song->pos!=MPD_SONG_NO_NUM)
		LOG_INFO("pos: %i", song->pos);
}

static int
test_status(struct mpd_connection *conn)
{
	struct mpd_status *status;

	mpd_send_status(conn);

	CHECK_CONNECTION(conn);

	status = mpd_get_status(conn);
	if (!status) {
		LOG_ERROR("%s", mpd_get_error_string(conn));
		return -1;
	}
	if (mpd_status_get_error(status)) {
		LOG_WARNING("status error: %s", mpd_status_get_error(status));
	}

	print_status(status);
	mpd_free_status(status);

	mpd_finishCommand(conn);
	CHECK_CONNECTION(conn);

	return 0;
}

static int
test_currentsong(struct mpd_connection *conn)
{
	struct mpd_song *song;
	mpd_InfoEntity *entity;

	mpd_send_currentsong(conn);

	CHECK_CONNECTION(conn);

	mpd_nextListOkCommand(conn);

	entity = mpd_getNextInfoEntity(conn);
	if (entity) {
		song = entity->info.song;
		if (entity->type != MPD_INFO_ENTITY_TYPE_SONG || !song) {
			LOG_ERROR("entity doesn't have the expected type (song)i :%d", entity->type);
			mpd_freeInfoEntity(entity);
			return -1;
		}

		print_song(song);

		mpd_freeInfoEntity(entity);
	}

	mpd_finishCommand(conn);
	CHECK_CONNECTION(conn);

	return 0;
}


static int
test_list_status_currentsong(struct mpd_connection *conn)
{
	struct mpd_status *status;
	struct mpd_song *song;
	mpd_InfoEntity *entity;

	CHECK_CONNECTION(conn);

	mpd_sendCommandListOkBegin(conn);
	mpd_send_status(conn);
	mpd_send_currentsong(conn);
	mpd_sendCommandListEnd(conn);

	CHECK_CONNECTION(conn);

	status = mpd_get_status(conn);
	if (!status) {
		LOG_ERROR("%s", mpd_get_error_string(conn));
		return -1;
	}
	if (mpd_status_get_error(status)) {
		LOG_WARNING("status error: %s", mpd_status_get_error(status));
	}

	print_status(status);
	mpd_free_status(status);

	CHECK_CONNECTION(conn);

	mpd_nextListOkCommand(conn);

	entity = mpd_getNextInfoEntity(conn);
	if (entity) {
		song = entity->info.song;
		if (entity->type != MPD_INFO_ENTITY_TYPE_SONG || !song) {
			LOG_ERROR("entity doesn't have the expected type (song)i :%d", entity->type);
			mpd_freeInfoEntity(entity);
			return -1;
		}

		print_song(song);

		mpd_freeInfoEntity(entity);
	}

	mpd_finishCommand(conn);
	CHECK_CONNECTION(conn);

	return 0;
}

static int
test_lsinfo(struct mpd_connection *conn, const char *path)
{
	mpd_InfoEntity *entity;

	mpd_send_lsinfo(conn, path);
	CHECK_CONNECTION(conn);

	while ((entity = mpd_getNextInfoEntity(conn))) {
		if (entity->type == MPD_INFO_ENTITY_TYPE_SONG) {
			struct mpd_song *song = entity->info.song;
			print_song (song);
		} else if (entity->type == MPD_INFO_ENTITY_TYPE_DIRECTORY) {
			struct mpd_directory *dir = entity->info.directory;
			LOG_INFO("directory: %s", dir->path);
		} else if (entity->type == MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
			struct mpd_stored_playlist * pl = entity->info.playlistFile;
			LOG_INFO("playlist: %s", pl->path);
		} else {
			LOG_ERROR("Unknown type: %d", entity->type);
			mpd_freeInfoEntity(entity);
			return -1;
		}

		mpd_freeInfoEntity(entity);
	}

	mpd_finishCommand(conn);
	CHECK_CONNECTION(conn);

	return 0;
}

static int
test_list_artists(struct mpd_connection *conn)
{
	char *artist;
	int first = 1;

        mpd_send_list_artist(conn);
	CHECK_CONNECTION(conn);

	LOG_INFO("%s: ", "Artists list");
	while ((artist = mpd_getNextArtist(conn))) {
		if (first) {
			printf("    %s", artist);
			first = 0;
		} else {
			printf(", %s", artist);
		}
		free(artist);
	}
	printf("\n");

	mpd_finishCommand(conn);
	CHECK_CONNECTION(conn);

	return 0;
}

static int
test_close_connection(struct mpd_connection *conn)
{
	mpd_closeConnection(conn);
	return 0;
}

int
main(int argc, char ** argv)
{
	struct mpd_connection *conn = NULL;
	const char *lsinfo_path = "/";

	if (argc==2)
		lsinfo_path = argv[1];

	START_TEST("Test connection to MPD server", test_new_connection, &conn);
	if (!conn)
		return -1;

	START_TEST("Check MPD versions", test_version, conn);
	START_TEST("'status' command", test_status, conn);
	START_TEST("'currentsong' command", test_currentsong, conn);
	START_TEST("List commands: 'status' and 'currentsong'", test_list_status_currentsong, conn);
	START_TEST("'lsinfo' command", test_lsinfo, conn, lsinfo_path);
	START_TEST("'list artist' command", test_list_artists, conn);
	START_TEST("Test connection closing", test_close_connection, conn);

	return 0;
}
