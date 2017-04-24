/* Copyright 2003-2017, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: msgs.c
 *
 * RISC OS Message File support.  Open and close Message files, and look
 * up tokens.
 */

/* Acorn C header files. */



/* OS-Lib header files. */

#include "oslib/messagetrans.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "msgs.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>


static messagetrans_control_block	*message_block = NULL;
static osbool				external_file = FALSE;


/* Iniitialise the Msgs module, loading the specified file and preparing the
 * system to handle message lookups.
 *
 * This is an external interface, documented in msgs.h
 */

osbool msgs_initialise(char *messages_file)
{
	int	message_size;
	char	*message_buffer;

	messagetrans_file_info(messages_file, NULL, &message_size);
	message_block = malloc(sizeof (messagetrans_control_block));
	message_buffer = malloc(message_size);
	messagetrans_open_file(message_block, messages_file, message_buffer);

	return TRUE;
}


/* Initialise the Msgs module using an already prepared MessageTrans
 * Control Block.
 *
 * This is an external interface, documented in msgs.h
 */

osbool msgs_initialise_external(messagetrans_control_block *block)
{
	message_block = block;
	external_file = TRUE;

	return TRUE;
}


/* Terminate the Msgs module, closing any Messages file that was opened
 * via msgs_initialise().
 *
 * This is an external interface, documented in msgs.h
 */

osbool msgs_terminate(void)
{
	if (message_block == NULL)
		return FALSE;

	if (external_file == FALSE) {
		messagetrans_close_file(message_block);
		free(message_block);
	}

	message_block = NULL;
	external_file = FALSE;

	return TRUE;
}


/* Look up a message token without parameters, storing the result in the
 * supplied buffer.
 *
 * This is an external interface, documented in msgs.h
 */

char *msgs_lookup(char *token, char *buffer, size_t buffer_size)
{
	msgs_param_lookup_result(token, buffer, buffer_size, NULL, NULL, NULL, NULL);

	return buffer;
}


/* Look up a message token, substituting the supplied parameters and storing
 * the result in the supplied buffer.
 *
 * This is an external interface, documented in msgs.h
 */

char *msgs_param_lookup(char *token, char *buffer, size_t buffer_size, char *a, char *b, char *c, char *d)
{
	msgs_param_lookup_result(token, buffer, buffer_size, a, b, c, d);

	return buffer;
}


/* Look up a message token without parameters, storing the result in the
 * supplied buffer.
 *
 * This is an external interface, documented in msgs.h
 */

osbool msgs_lookup_result(char *token, char *buffer, size_t buffer_size)
{
	return msgs_param_lookup_result(token, buffer, buffer_size, NULL, NULL, NULL, NULL);
}


/* Look up a message token, substituting the supplied parameters and storing
 * the result in the supplied buffer.
 *
 * This is an external interface, documented in msgs.h
 */

osbool msgs_param_lookup_result(char *token, char *buffer, size_t buffer_size, char *a, char *b, char *c, char *d)
{
	os_error	*error;

	if (buffer == NULL || buffer_size <= 0)
		return FALSE;

	/* If there's no token, return an empty buffer. */

	if (token == NULL) {
		*buffer = '\0';
		return FALSE;
	}

	/* If there's no message block, instead of using the Global block, return the supplied token. */

	if (message_block == NULL) {
		strncpy(buffer, token, buffer_size);
		buffer[buffer_size - 1] = '\0';
		return FALSE;
	}

	/* Look up the token. */

	error = xmessagetrans_lookup(message_block, token, buffer, buffer_size, a, b, c, d, NULL, NULL);

	/* If there was an error, return an empty buffer. */

	if (error != NULL) {
		*buffer = '\0';
		return FALSE;
	}

	return TRUE;
}

