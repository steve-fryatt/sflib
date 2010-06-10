/* SF-Lib - Msgs.c
 *
 * Version 0.10 (22 September 2003)
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

/* ================================================================================================================== */

static messagetrans_control_block *message_block = NULL;
static int                        external_file = FALSE;

/* ================================================================================================================== */

int msgs_init (char *messages_file)
{
  int  message_size;
  char *message_buffer;

  messagetrans_file_info (messages_file, NULL, &message_size);
  message_block = (messagetrans_control_block *) malloc (sizeof (messagetrans_control_block));
  message_buffer = (char *) malloc (message_size);
  messagetrans_open_file (message_block, messages_file, message_buffer);

  return 1;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int msgs_external_init (messagetrans_control_block *block)
{
  message_block = block;
  external_file = TRUE;

  return 1;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int msgs_close_file (void)
{
  if (external_file == FALSE)
  {
    messagetrans_close_file (message_block);
    free (message_block);
  }

  message_block = NULL;
  external_file = FALSE;

  return 1;
}

/* ================================================================================================================== */

char *msgs_lookup (char *token, char *buffer, int buffer_size)
{
  return (msgs_param_lookup (token, buffer, buffer_size, NULL, NULL, NULL, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *msgs_param_lookup (char *token, char *buffer, int buffer_size, char *a, char *b, char *c, char *d)
{
  os_error *error;
  char     *text;


  text = strchr (token, ':');
  if (text != NULL)
  {
    *text++ = '\0';
  }
  else
  {
    text = token;
  }

  error = xmessagetrans_lookup (message_block, token, buffer, buffer_size, a, b, c, d, NULL, NULL);
  if (error != NULL)
  {
    if (strlen (text) < buffer_size)
    {
      strcpy (buffer, text);
    }
    else
    {
      strncpy (buffer, text, buffer_size - 1);
      *(buffer + buffer_size - 1) = '\0';
    }
  }

  return (buffer);
}

/* ================================================================================================================== */

int msgs_lookup_result (char *token, char *buffer, int buffer_size)
{
  return (msgs_param_lookup_result (token, buffer, buffer_size, NULL, NULL, NULL, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

int msgs_param_lookup_result (char *token, char *buffer, int buffer_size, char *a, char *b, char *c, char *d)
{
  os_error *error;
  char     *text;


  text = strchr (token, ':');
  if (text != NULL)
  {
    *text++ = '\0';
  }
  else
  {
    text = token;
  }

  error = xmessagetrans_lookup (message_block, token, buffer, buffer_size, a, b, c, d, NULL, NULL);
  if (error != NULL)
  {
    if (strlen (text) < buffer_size)
    {
      strcpy (buffer, text);
    }
    else
    {
      strncpy (buffer, text, buffer_size - 1);
      *(buffer + buffer_size - 1) = '\0';
    }
  }

  return (error != 0);
}
