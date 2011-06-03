/**
 * \file: url.h
 *
 * SF-Lib - URL Dispatch System.
 *
 * (C) Stephen Fryatt, 2003-2011
 */

#ifndef SFLIB_URL
#define SFLIB_URL

/* Requires message token URLFailed: to be present, with launch failed message.
*/


typedef union {
	char				*ptr;
	int				offset;
} string_value;


typedef struct {
	wimp_MESSAGE_HEADER_MEMBERS
	union {
		char			url[236];
		struct {
			int		tag;
			string_value	url;
			int		flags;
			string_value	body_file;
			string_value	target;
			string_value	body_mimetype;
		} indirect;
	} data;
} url_message;


#define message_ANT_OPEN_URL  0x4af80        /* ANT url broadcast wimp message number. */


/**
 * Initialise the URL library, registering the necessary message handlers.
 *
 * \return 		TRUE if initialisation is successful; else FALSE.
 */

osbool url_initialise(void);


/**
 * Attempt to launch a URL, using first the Acorn URI Protocol and falling back
 * to the ANT system if that fails.
 *
 * \param *url		The URL to launch.
 */

void launch_url(const char *url);

#endif
