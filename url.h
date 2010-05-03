/* SF-Lib - URL.h
 *
 * Version 0.15 (15 June 2003)
 */

#ifndef _SFLIB_URL
#define _SFLIB_URL

/* Requires message token URLFailed: to be present, with launch failed message.
*/

/* ================================================================================================================== */

typedef union
{
  char *ptr;
  int offset;
}
string_value;

/* ------------------------------------------------------------------------------------------------------------------ */

typedef struct
{
  wimp_MESSAGE_HEADER_MEMBERS
  union
  {
    char url[236];
    struct
    {
      int tag;
      string_value url;
      int flags;
      string_value body_file;
      string_value target;
      string_value body_mimetype;
    }
    indirect;
  }
  data;
}
url_message;

/* ================================================================================================================== */

#define message_ANT_OPEN_URL  0x4af80        /* ANT url broadcast wimp message number. */

/* ================================================================================================================== */

void launch_url (const char *url);

/* Launch a url using the acorn system first, then the ANT system if that fails.
 *
 * *url - URL to launch.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


void url_bounce(wimp_message *message);

/* Handle a bounced url message; called when UserMessageRecorded of URI_MReturnResult or a UserMessageAck of type
 * of type Wimp_MOpenUrl is received.
 *
 * *message - wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */

#endif
