/**
 * \file: msgs.c
 *
 * SF-Lib - Msgs.c
 *
 * (C) Stephen Fryatt, 2003-2011
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

	messagetrans_file_info (messages_file, NULL, &message_size);
	message_block = (messagetrans_control_block *) malloc (sizeof (messagetrans_control_block));
	message_buffer = (char *) malloc (message_size);
	messagetrans_open_file (message_block, messages_file, message_buffer);

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
	char		*text;

	text = strchr(token, ':');
	if (text != NULL)
		*text++ = '\0';
	else
		text = token;

	error = xmessagetrans_lookup(message_block, token, buffer, buffer_size, a, b, c, d, NULL, NULL);

	if (error == NULL)
		return TRUE;

	if (strlen(text) < buffer_size) {
		strcpy(buffer, text);
	} else {
		strncpy(buffer, text, buffer_size - 1);
		*(buffer + buffer_size - 1) = '\0';
	}

	return FALSE;
}

