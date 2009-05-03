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

#include <mpd/output.h>
#include <mpd/connection.h>
#include <mpd/pair.h>
#include "internal.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>

struct mpd_output_entity *
mpd_output_get_next(struct mpd_connection *connection)
{
	struct mpd_output_entity *output = NULL;

	if (connection->doneProcessing || (connection->listOks &&
				connection->doneListOk))
	{
		return NULL;
	}

	if (mpd_error_is_defined(&connection->error))
		return NULL;

	output = malloc(sizeof(*output));
	output->id = -10;
	output->name = NULL;
	output->enabled = 0;

	if (connection->pair == NULL)
		mpd_get_next_return_element(connection);

	while (connection->pair != NULL) {
		const struct mpd_pair *pair = connection->pair;

		if (strcmp(pair->name, "outputid") == 0) {
			if (output!=NULL && output->id>=0) return output;
			output->id = atoi(pair->value);
		}
		else if (strcmp(pair->name, "outputname") == 0) {
			output->name = strdup(pair->value);
		}
		else if (strcmp(pair->name, "outputenabled") == 0) {
			output->enabled = atoi(pair->value);
		}

		mpd_get_next_return_element(connection);
		if (mpd_error_is_defined(&connection->error)) {
			if (output->name != NULL)
				free(output->name);
			free(output);
			return NULL;
		}

	}

	return output;
}

void
mpd_output_free(struct mpd_output_entity *output)
{
	free(output->name);
	free(output);
}
