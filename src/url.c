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

#include "sflib/url.h"
#include "sflib/general.h"
#include "sflib/errors.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>

/* ================================================================================================================== */

static int url_antbroadcast(const char *url)
{
  url_message  urlblock;
  os_error     *error;


  urlblock.size = WORDALIGN(20 + strlen( url ) + 1 );
  urlblock.your_ref = 0;
  urlblock.action = message_ANT_OPEN_URL;

  *urlblock.data.url = 0;
  strncat(urlblock.data.url, url, sizeof(urlblock.data.url) - 1);

  error = xwimp_send_message (wimp_USER_MESSAGE_RECORDED, (wimp_message *) &urlblock, wimp_BROADCAST);
  if (error != NULL)
  {
    wimp_os_error_report (error, wimp_ERROR_BOX_CANCEL_ICON);
    return -1;
  }

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

static int url_antload(const char *url)
{
  char buf[512];
  char *off;


  off = strchr(url, ':');

  /* our url request wasn't answered :-( */
  /* try an Alias$URLOpen, instead */
  if (off == NULL)
  {
    return -1;
  }

  strcpy(buf, "Alias$URLOpen_");
  strncat(buf, url, off - url);
  if (getenv(buf) == NULL)
  {
    return -1;
  }

  strcat(buf, " ");
  strncat(buf, url, sizeof(buf) - strlen(buf) - 1 );

  if (xwimp_start_task(buf + sizeof("Alias$") - 1, NULL) != NULL)
  {
    return -1;
  }

  return 0;
}


/* ================================================================================================================== */

void launch_url (const char *url)
{
  if (url_antbroadcast(url) == -1)
  {
    wimp_msgtrans_info_report ("URLFailed");
  }
}

/* ================================================================================================================== */

static int url_acornlaunch(const char *url)
{
  wimp_t taskhan = 0;
  int success = 1;
  int flags;


  if (xwimpreadsysinfo_task (&taskhan, NULL) != NULL)
  {
    success = 0;
  }

  if (success)
  {
    if (xuri_dispatch (uri_DISPATCH_INFORM_CALLER, url, taskhan, &flags, NULL, NULL) != NULL || flags & 1)
    {
      success = 0;
    }
  }

  if (!success)
  {
    url_antload(url);
  }

  return 0;
}

/* ================================================================================================================== */

void url_bounce(wimp_message *mess)
{
  char      buf[512];
  os_error  *error;


  if (mess->action == message_URI_RETURN_RESULT)
  {
    uri_full_message_return_result *result = (uri_full_message_return_result *) mess;

    if ((result->flags & uri_RETURN_RESULT_NOT_CLAIMED) == 0)
    {
      return; /* url was claimed */
    }

    /* failed AcornURI. Try ANT launch */
    error = xuri_request_uri (0, buf, sizeof buf, result->handle, NULL);

    if (error)
    {
      wimp_os_error_report (error, wimp_ERROR_BOX_CANCEL_ICON);
      return;
    }

    if (url_antload(buf) == -1)
    {
      wimp_msgtrans_info_report ("URLFailed");
    }

    /* we haven't acknowledged the returnresult message, so the URI
     * handler task will automatically free the URI */

  }
  else if (mess->action == message_ANT_OPEN_URL)
  {
    url_message *failed = (url_message *) mess;

    /* otherwise, it was an ANT url broadcast that failed - try Acorn broadcast/launch */

    url_acornlaunch(failed->data.url);
  }
}
