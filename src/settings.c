/* libmpdclient
   (c) 2003-2019 The Music Player Daemon Project
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

#include <mpd/settings.h>
#include "internal.h"
#include "config.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/**
 * Decide where to get host and password from.
 * If the host is unspecified (NULL), use the environment variable MPD_HOST.
 *
 * Try to extract an embedded password from the host.
 * Host strings with an embedded password have the form:
 *     "password@hostname"
 *  where "password" is one or more characters.
 * Embedded passwords are preferred over the password argument.
 *
 * The host/password fields are left as NULL if not specified via argument,
 *  environment (host) or by being embedded in another string (password).
 *
 * @param settings a settings object. both settings->host and
 * settings->password may be modified by this function
 * @param host the explicitly passed host specification or NULL.
 * @param password the explicitly passed password or NULL.
 * @return true on success, false on out of memory
 */
static bool
mpd_choose_host_and_password(struct mpd_settings *settings,
		const char *host, const char *password)
{
	char *at;
	size_t host_len, at_pos;

	assert(settings->host == NULL);
	assert(settings->password == NULL);

	if (host == NULL) {
		host = getenv("MPD_HOST");
	}

	if (host == NULL || *host == 0) {
		/* treat the empty string as NULL */
		host = NULL;
	}

	if (host != NULL) {
		/* find the @, if any. */
		at = strchr(host, '@');

		/* if host begins with a '@' then it's not an empty password,
		 * but an abstract socket address, see unix(7) */
		if (*host == '@' || at == NULL) {
			/* No '@', or host is an abstract socket? copy the whole thing */
			settings->host = strdup(host);
			if (settings->host == NULL) {
				return false;
			}
		} else {
			/* We need to extract the password... */
			at_pos = at - host;
			settings->password = malloc(at_pos + 1);
			if (settings->password == NULL) {
				return false;
			}

			memcpy(settings->password, host, at_pos);
			(settings->password)[at_pos] = 0;

			/* ... and the host on it's own. */
			host_len = strlen(host) - at_pos;
			settings->host = malloc(host_len);
			if (settings->host == NULL) {
				return false;
			}

			memcpy(settings->host, &host[at_pos + 1], host_len - 1);
			(settings->host)[host_len - 1] = 0;
		}
	}

	/* Use the password argument if no embedded password was present. */
	if (password != NULL && settings->password == NULL) {
		settings->password = strdup(password);
		if (settings->password == NULL) {
			return false;
		}
	}

	return true;
}

/**
 * Load port from the environment variable MPD_PORT if unspecified (0).
 *
 * @param settings a settings object. settings->port will be modified.
 * @param port the explicitly passed port number.
 */
static void
mpd_choose_port(struct mpd_settings *settings, unsigned port)
{
	if (port == 0) {
		const char *env_port = getenv("MPD_PORT");
		if (env_port != NULL) {
			port = strtoul(env_port, NULL, 10);
		}
	}

	settings->port = port;
}

/**
 * Load timeout from the environment variable MPD_TIMEOUT if unspecified (0).
 *
 * @param settings a settings object. settings->timeout_ms will be modified.
 * @param timeout_ms the explicitly passed timeout, in milliseconds.
 */
static void
mpd_choose_timeout_ms(struct mpd_settings *settings, unsigned timeout_ms)
{
	if (timeout_ms == 0) {
		const char *env_timeout = getenv("MPD_TIMEOUT");
		if (env_timeout != NULL) {
			/* Note: envvar is specified in seconds */
			timeout_ms = strtoul(env_timeout, NULL, 10) * 1000;
		}
	}

	settings->timeout_ms = timeout_ms;
}

/**
 * Create a new settings struct, using either supplied settings
 * or environment variables. Note that 'host' may also contain a password,
 * and this password will be used in preference to the password argument.
 *
 *  @param host a host specification, NULL indicates unspecified.
 *  @param port the port to use, 0 indicates unspecified.
 *  @param timeout_ms timeout in milliseconds, 0 indicates unspecified.
 *  @param reserved reserved for future use, currently must be NULL.
 *  @param password the password to use when connecting, NULL indicates no password.
 */

struct mpd_settings *
mpd_settings_new(const char *host, unsigned port, unsigned timeout_ms,
		 const char *reserved, const char *password)
{
	(void)reserved;

	struct mpd_settings *settings = malloc(sizeof(*settings));
	if (settings == NULL) {
		return NULL;
	}

	settings->host = NULL;
	settings->password = NULL;

	if (!mpd_choose_host_and_password(settings, host, password)) {
		mpd_settings_free(settings);
		return NULL;
	}

	mpd_choose_port(settings, port);
	mpd_choose_timeout_ms(settings, timeout_ms);

	return settings;
}

void
mpd_settings_free(struct mpd_settings *settings)
{
	free(settings->host);
	free(settings->password);
	free(settings);
}

const char *
mpd_settings_get_host(const struct mpd_settings *settings)
{
	return settings->host;
}

unsigned
mpd_settings_get_port(const struct mpd_settings *settings)
{
	return settings->port;
}

unsigned
mpd_settings_get_timeout_ms(const struct mpd_settings *settings)
{
	return settings->timeout_ms;
}

const char *
mpd_settings_get_password(const struct mpd_settings *settings)
{
	return settings->password;
}
