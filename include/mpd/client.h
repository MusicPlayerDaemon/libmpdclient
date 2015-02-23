/* libmpdclient
   (c) 2003-2015 The Music Player Daemon Project
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

/*! \file
 * \brief MPD client library
 *
 * This is a client library for the Music Player Daemon, written in C.
 *
 * You can choose one of several APIs, depending on your requirements:
 *
 * - struct mpd_async: a very low-level asynchronous API which knows
 *   the protocol syntax, but no specific commands
 *
 * - struct mpd_connection: a basic synchronous API which knows all
 *   MPD commands and parses all responses
 *
 * \author Max Kellermann (max@duempel.org)
 */

#ifndef MPD_CLIENT_H
#define MPD_CLIENT_H

#include <mpd/audio_format.h>
#include <mpd/capabilities.h>
#include <mpd/connection.h>
#include <mpd/database.h>
#include <mpd/directory.h>
#include <mpd/entity.h>
#include <mpd/idle.h>
#include <mpd/list.h>
#include <mpd/message.h>
#include <mpd/mixer.h>
#include <mpd/output.h>
#include <mpd/pair.h>
#include <mpd/password.h>
#include <mpd/player.h>
#include <mpd/playlist.h>
#include <mpd/queue.h>
#include <mpd/recv.h>
#include <mpd/response.h>
#include <mpd/search.h>
#include <mpd/send.h>
#include <mpd/settings.h>
#include <mpd/song.h>
#include <mpd/stats.h>
#include <mpd/status.h>
#include <mpd/sticker.h>
#include <mpd/version.h>

#endif
