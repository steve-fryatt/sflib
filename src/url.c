/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file: url.c
 *
 * URL dispatch code, which takes the following steps to broadcast a URL to other
 * applications on the machine:
 *
 *  1) Try ANT broadcast
 *  2) If that bounces, try Acorn URI dispatch [broadcast + load]
 *  3) If that succeeds, wait for success or failure wimp message
 *  4) If either fails, try ANT URL_Open
 *  5) If that fails, give error message
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"
#include "oslib/os.h"
#include "oslib/uri.h"

/* SF-Lib header files. */

#include "url.h"

#include "errors.h"
#include "event.h"
#include "general.h"
#include "string.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>

#define URL_BUFFER_LENGTH 512

static osbool		url_antbroadcast(const char *url);
static osbool		url_antload(const char *url);
static osbool		url_acornlaunch(const char *url);
static osbool		url_bounce(wimp_message *mess);


/* Initialise the URL library, registering the necessary message handlers.
 *
 * This function is an external interface, documented in url.h.
 */

osbool url_initialise(void)
{
	osbool		ok = TRUE;

	if (ok)
		ok = event_add_message_handler(message_URI_RETURN_RESULT, EVENT_MESSAGE_INCOMING, url_bounce);

	if (ok)
		ok = event_add_message_handler(message_ANT_OPEN_URL, EVENT_MESSAGE_ACKNOWLEDGE, url_bounce);

	return ok;
}


/* Attempt to launch a URL.
 *
 * This function is an external interface, documented in url.h.
 */

void url_launch(const char *url)
{
	if (!url_antbroadcast(url))
		error_msgs_report_info("URLFailed:Failed to launch URL.");
}


/**
 * Launch a URL via the ANT broadcast protocol.
 *
 * \param *url		The URL to broadcast.
 * \return		TRUE if the broadcast was sent; else FALSE.
 */

static osbool url_antbroadcast(const char *url)
{
	url_message  urlblock;
	os_error     *error;


	urlblock.size = WORDALIGN(20 + strlen(url) + 1);
	urlblock.your_ref = 0;
	urlblock.action = message_ANT_OPEN_URL;

	*urlblock.data.url = 0;
	strncat(urlblock.data.url, url, sizeof(urlblock.data.url) - 1);

	error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &urlblock, wimp_BROADCAST);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return FALSE;
	}

	return TRUE;
}


/**
 * Launch a URL via the ANT Load protocol.
 *
 * \param *url		The URL to broadcast.
 * \return		TRUE if the broadcast was sent; else FALSE.
 */

static osbool url_antload(const char *url)
{
	char	buf[URL_BUFFER_LENGTH];
	char	*protocol_offset;
	char	*separator = " ";
	int	prefix_length, suffix_length, separator_length;

	/* Find the offset of the protocol separator in the URL. */

	protocol_offset = strchr(url, ':');

	if (protocol_offset == NULL)
		return FALSE;

	/* Extract the protocol from the URL (the bit before the colon, so http, file, etc),
	 * and tack it on to the URLOpen_ Alias prefix to make the name of a possible
	 * system variable.
	 *
	 * If the variable we need doesn't exist, give up.
	 */

	string_copy(buf, "Alias$URLOpen_", URL_BUFFER_LENGTH);

	prefix_length = strlen(buf);
	suffix_length = protocol_offset - url;
	separator_length = strlen(separator);

	if (prefix_length + suffix_length + separator_length + 1 > URL_BUFFER_LENGTH) /* +1 for terminator. */
		return FALSE;

	strncat(buf, url, suffix_length);
	if (getenv(buf) == NULL)
		return FALSE;

	/* Append the URL to the end of the Alias name, and then call the command. */

	strncat(buf, separator, separator_length);
	strncat(buf, url, URL_BUFFER_LENGTH - (prefix_length + suffix_length + separator_length + 1));

	if (xwimp_start_task(buf + strlen("Alias$"), NULL) != NULL)
		return FALSE;

	return TRUE;
}


/**
 * Launch a URL via the Acorn URI protocol.
 *
 * \param *url		The URL to broadcast.
 * \return		TRUE if successful; else FALSE.
 */

static osbool url_acornlaunch(const char *url)
{
	wimp_t			taskhan = 0;
	osbool			success = TRUE;
	uri_dispatch_flags	flags;


	if (xwimpreadsysinfo_task (&taskhan, NULL) != NULL)
		success = FALSE;

	if (success)
		if (xuri_dispatch (uri_DISPATCH_INFORM_CALLER, url, taskhan, &flags, NULL, NULL) != NULL || flags & 1)
			success = FALSE;

	if (!success)
		url_antload(url);

	return TRUE;
}


/**
 * Message handler for user messages and bounces.
 *
 * \param *mess		The incoming message block to be handled.
 * \return		TRUE if the message should be claimed; else FALSE.
 */

static osbool url_bounce(wimp_message *mess)
{
	char		buf[URL_BUFFER_LENGTH];
	os_error	*error;


	if (mess->action == message_URI_RETURN_RESULT) {
		uri_full_message_return_result *result = (uri_full_message_return_result *) mess;

		if ((result->flags & uri_RETURN_RESULT_NOT_CLAIMED) == 0)
			return TRUE; /* url was claimed */

		/* failed AcornURI. Try ANT launch */
		error = xuri_request_uri(0, buf, URL_BUFFER_LENGTH, result->handle, NULL);

		if (error) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			return TRUE;
		}

		if (!url_antload(buf))
			error_msgs_report_info("URLFailed:Failed to launch URL.");

		/* we haven't acknowledged the returnresult message, so the URI
		 * handler task will automatically free the URI */

	} else if (mess->action == message_ANT_OPEN_URL) {
		url_message *failed = (url_message *) mess;

		/* otherwise, it was an ANT url broadcast that failed - try Acorn broadcast/launch */

		url_acornlaunch(failed->data.url);
	}

	return TRUE;
}
