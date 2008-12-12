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

#include <mpd/idle.h>
#include "internal.h"

#include <stdio.h>
#include <string.h>
#ifdef MPD_GLIB
#include <glib.h>
#endif

static const char *const idle_names[] = {
	"database",
	"stored_playlist",
	"playlist",
	"player",
	"mixer",
	"output",
	"options",
	NULL
};

static void mpd_readChanges(struct mpd_connection *connection)
{
	unsigned i;
	unsigned flags = 0;
	struct mpd_return_element *re;

	if (!connection->returnElement) mpd_getNextReturnElement(connection);

	if (connection->error.code == MPD_ERROR_CONNCLOSED) {
		connection->notify_cb (connection, IDLE_DISCONNECT, connection->userdata);
		return;
	}

	while (connection->returnElement) {
		re = connection->returnElement;
		if (re->name &&!strncmp (re->name, "changed", strlen ("changed"))) {
			for (i = 0; idle_names[i]; ++i) {
				if (!strcmp (re->value, idle_names[i])) {
					flags |= (1 << i);
				}
			}
		}
		mpd_getNextReturnElement(connection);
	}

	/* Notifiy application */
	if (connection->notify_cb && flags)
		connection->notify_cb (connection, flags, connection->userdata);
}

void mpd_startIdle(struct mpd_connection *connection, mpd_NotificationCb notify_cb, void *userdata)
{
	if (connection->idle)
		return;

	if (connection->startIdle)
		connection->startIdle(connection);

	mpd_executeCommand(connection, "idle\n");
	connection->idle = 1;
	connection->notify_cb = notify_cb;
	connection->userdata = userdata;
}

void mpd_stopIdle(struct mpd_connection *connection)
{
	if (connection->stopIdle)
		connection->stopIdle(connection);

	connection->idle = 0;
	connection->notify_cb = NULL;
	connection->doneProcessing = 1;
	mpd_executeCommand(connection, "noidle\n");
	mpd_readChanges(connection);
}

#ifdef MPD_GLIB
static gboolean mpd_glibReadCb (GIOChannel *iochan, GIOCondition cond, gpointer data)
{
	struct mpd_connection *connection = data;

	if (!connection->idle) {
		connection->source_id = 0;
		return FALSE;
	}

	if ((cond & G_IO_IN)) {
		connection->idle = 0;
		if (connection->source_id) {
			g_source_remove (connection->source_id);
			connection->source_id = 0;
		}
		mpd_readChanges(connection);
	}

	return TRUE;
}

static void mpd_glibStartIdle(struct mpd_connection *connection)
{
	static GIOChannel* iochan = NULL;

	if (!iochan) {
#ifdef WIN32
		iochan = g_io_channel_win32_new_socket (connection->sock);
#else
		iochan = g_io_channel_unix_new (connection->sock);
#endif
	}

	connection->source_id = g_io_add_watch (iochan,
						G_IO_IN | G_IO_ERR | G_IO_HUP,
						mpd_glibReadCb,
						connection);
}

static void mpd_glibStopIdle(struct mpd_connection *connection)
{
	if (connection->source_id) {
		g_source_remove (connection->source_id);
		connection->source_id = 0;
	}
}

void mpd_glibInit(struct mpd_connection *connection)
{
	connection->startIdle = mpd_glibStartIdle;
	connection->stopIdle = mpd_glibStopIdle;
}
#endif

