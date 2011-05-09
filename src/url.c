/* SF-Lib - URL.c
 *
 * Version 0.10 (22 June 2003)
 * Based on Nettle sources, but OSLib-ified.
 *
 * The following steps are taken in a url broadcast:
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

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>

static osbool		url_antbroadcast(const char *url);
static osbool		url_antload(const char *url);
static osbool		url_acornlaunch(const char *url);
static osbool		url_bounce(wimp_message *mess);


/**
 * Initialise the URL library, registering the necessary message handlers.
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


/**
 * Attempt to launch a URL.
 *
 * This function is an external interface, documented in url.h.
 */

void launch_url (const char *url)
{
	if (!url_antbroadcast(url))
		wimp_msgtrans_info_report("URLFailed");
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
		wimp_os_error_report(error, wimp_ERROR_BOX_CANCEL_ICON);
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
	char buf[512];
	char *off;


	off = strchr(url, ':');

	if (off == NULL)
		return FALSE;

	strcpy(buf, "Alias$URLOpen_");
	strncat(buf, url, off - url);
	if (getenv(buf) == NULL)
		return FALSE;

	strcat(buf, " ");
	strncat(buf, url, sizeof(buf) - strlen(buf) - 1 );

	if (xwimp_start_task(buf + sizeof("Alias$") - 1, NULL) != NULL)
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
	char		buf[512];
	os_error	*error;


	if (mess->action == message_URI_RETURN_RESULT) {
		uri_full_message_return_result *result = (uri_full_message_return_result *) mess;

		if ((result->flags & uri_RETURN_RESULT_NOT_CLAIMED) == 0)
			return TRUE; /* url was claimed */

		/* failed AcornURI. Try ANT launch */
		error = xuri_request_uri(0, buf, sizeof buf, result->handle, NULL);

		if (error) {
			wimp_os_error_report(error, wimp_ERROR_BOX_CANCEL_ICON);
			return TRUE;
		}

		if (!url_antload(buf))
			wimp_msgtrans_info_report("URLFailed");

		/* we haven't acknowledged the returnresult message, so the URI
		 * handler task will automatically free the URI */

	} else if (mess->action == message_ANT_OPEN_URL) {
		url_message *failed = (url_message *) mess;

		/* otherwise, it was an ANT url broadcast that failed - try Acorn broadcast/launch */

		url_acornlaunch(failed->data.url);
	}

	return TRUE;
}
