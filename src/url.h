/* Copyright 2003-2012, Stephen Fryatt
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
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
 * \file: url.h
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

#ifndef SFLIB_URL
#define SFLIB_URL

/* Requires message token URLFailed: to be present, with launch failed message.
*/


/**
 * The string_value data type used in the ANT Open URL message block.
 */

typedef union {
	char				*ptr;					/**< String pointer to shared memory if >= 236.					*/
	int				offset;					/**< Offset into message block if <236 (0 indicates data not present).		*/
} string_value;


/**
 * The ANT Open URL ("MOpenURL") message block, for use in the ANT Internet
 * Suite's protocol.
 */

typedef struct {
	wimp_MESSAGE_HEADER_MEMBERS						/**< The standard Wimp message headers.						*/
	union {
		char			url[236];				/**< The URL to open if it is <236 characters long.				*/
		struct {
			int		tag;					/**< Zero tag to identify the use of the extended message format.		*/
			string_value	url;					/**< The URL to open.								*/
			int		flags;					/**< Flag word (bit 0 defined; all others reserved).				*/
			string_value	body_file;				/**< Body file name, if applicable (offset 0 if not present).			*/
			string_value	target;					/**< Frame target, if applicable (offset 0 if not present).			*/
			string_value	body_mimetype;				/**< Document body mimetype, if applicable (offset 0 if not present).		*/
		} indirect;							/**< The extended message body if not a standard URL of <236 characters.	*/
	} data;
} url_message;


#define message_ANT_OPEN_URL  0x4af80						/**< ANT url broadcast wimp message number. */


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

void url_launch(const char *url);

#endif
