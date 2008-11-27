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

#include <limits.h>
#include <string.h>
#include <stdio.h>

enum {
	INTLEN =  ((sizeof(int) * CHAR_BIT + 1) / 3 + 1),
};

void
mpd_sendOutputsCommand(struct mpd_connection *connection)
{
	mpd_executeCommand(connection,"outputs\n");
}

struct mpd_output_entity *
mpd_getNextOutput(struct mpd_connection *connection)
{
	struct mpd_output_entity *output = NULL;

	if(connection->doneProcessing || (connection->listOks &&
				connection->doneListOk))
	{
		return NULL;
	}

	if(connection->error) return NULL;

	output = malloc(sizeof(*output));
	output->id = -10;
	output->name = NULL;
	output->enabled = 0;

	if(!connection->returnElement) mpd_getNextReturnElement(connection);

	while(connection->returnElement) {
		struct mpd_return_element *re = connection->returnElement;
		if(strcmp(re->name,"outputid")==0) {
			if(output!=NULL && output->id>=0) return output;
			output->id = atoi(re->value);
		}
		else if(strcmp(re->name,"outputname")==0) {
			output->name = strdup(re->value);
		}
		else if(strcmp(re->name,"outputenabled")==0) {
			output->enabled = atoi(re->value);
		}

		mpd_getNextReturnElement(connection);
		if(connection->error) {
			free(output);
			return NULL;
		}

	}

	return output;
}

void
mpd_freeOutputElement(struct mpd_output_entity *output)
{
	free(output->name);
	free(output);
}

void
mpd_sendEnableOutputCommand(struct mpd_connection *connection, int outputId)
{
	int len = strlen("enableoutput")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "enableoutput \"%i\"\n", outputId);
	mpd_executeCommand(connection,string);
	free(string);
}

void
mpd_sendDisableOutputCommand(struct mpd_connection *connection, int outputId)
{
	int len = strlen("disableoutput")+2+INTLEN+3;
	char *string = malloc(len);
	snprintf(string, len, "disableoutput \"%i\"\n", outputId);
	mpd_executeCommand(connection,string);
	free(string);
}
