#include "iso8601.h"
#include <mpd/compiler.h>

#include <assert.h>
#include <time.h>

int main(mpd_unused int argc, mpd_unused char **argv)
{
	char buffer[64];
	time_t t, now;
	bool success;

	now = time(NULL);
	success = iso8601_datetime_format(buffer, sizeof(buffer), now);
	assert(success);

	t = iso8601_datetime_parse(buffer);
	assert(t == now);
}
