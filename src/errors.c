/* SF-Lib - Errors.c
 *
 * Version 0.15 (10 August 2003)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"
#include "oslib/messagetrans.h"

/* SF-Lib header files. */

#include "errors.h"
#include "msgs.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>

/* ================================================================================================================== */

#define sf_errors_NO_CUSTOM_BUTTONS ""

/* ================================================================================================================== */

static char                       *app_name = NULL, *app_sprite = NULL;
static void                       *close_down_function;

/* ================================================================================================================== */

void error_initialise (char *name, char *sprite, void (*closedown)(void))
{
  char     lookup_buffer[256];

  close_down_function = closedown;

  if (name != NULL)
  {
    if (app_name != NULL)
    {
      free (app_name);
    }

    msgs_lookup (name, lookup_buffer, sizeof (lookup_buffer));

    app_name = (char *) malloc (strlen (lookup_buffer) + 1);
    strcpy (app_name, lookup_buffer);
  }

  if (sprite != NULL)
  {
    if (app_sprite != NULL)
    {
      free (app_sprite);
    }

    msgs_lookup (sprite, lookup_buffer, sizeof (lookup_buffer));

    app_sprite = (char *) malloc (strlen (lookup_buffer) + 1);
    strcpy (app_sprite, lookup_buffer);
  }
}

/* ================================================================================================================== */

wimp_error_box_selection wimp_os_error_report (os_error *error, wimp_error_box_flags buttons)
{
  wimp_error_box_selection result;

  if (error != NULL)
  {
    result = wimp_report (error, wimp_ERROR_BOX_CATEGORY_ERROR, buttons, NULL);
  }
  else
  {
    result = wimp_ERROR_BOX_SELECTED_NOTHING;
  }

  return (result);
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_msgtrans_error_report (char *token)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  msgs_lookup (token, error.errmess, os_ERROR_LIMIT);

  return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_ERROR, wimp_ERROR_BOX_OK_ICON, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_error_report (char *message)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  strcpy (error.errmess, message);

  return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_ERROR, wimp_ERROR_BOX_OK_ICON, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_msgtrans_info_report (char *token)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  msgs_lookup (token, error.errmess, os_ERROR_LIMIT);

  return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_INFO, wimp_ERROR_BOX_OK_ICON, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_info_report (char *message)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  strcpy (error.errmess, message);

  return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_INFO, wimp_ERROR_BOX_OK_ICON, NULL));
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_msgtrans_question_report (char *token, char *buttons)
{
  os_error error;
  char     button_text[sf_errors_BUTTONS_MAX];

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  msgs_lookup (token, error.errmess, os_ERROR_LIMIT);

  if (buttons == NULL)
  {
    return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_QUESTION,
                         wimp_ERROR_BOX_OK_ICON | wimp_ERROR_BOX_CANCEL_ICON, NULL));
  }
  else
  {
    msgs_lookup (buttons, button_text, sf_errors_BUTTONS_MAX);
    return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_QUESTION, 0, button_text));
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_error_box_selection wimp_question_report (char *message, char *buttons)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  strcpy (error.errmess, message);

  if (buttons == NULL)
  {
    return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_QUESTION,
                         wimp_ERROR_BOX_OK_ICON | wimp_ERROR_BOX_CANCEL_ICON, NULL));
  }
  else
  {
    return (wimp_report (&error, wimp_ERROR_BOX_CATEGORY_QUESTION, 0, buttons));
  }
}

/* ================================================================================================================== */

void wimp_msgtrans_fatal_report (char *token)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  msgs_lookup (token, error.errmess, os_ERROR_LIMIT);

  wimp_report (&error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
  exit (1);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void wimp_fatal_report (char *message)
{
  os_error error;

  error.errnum = 255; /* A dummy error number, which should probably be checked. */
  strcpy (error.errmess, message);

  wimp_report (&error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
  exit (1);
}


/* ================================================================================================================== */

void wimp_program_report (os_error *error)
{
  if (error != NULL)
  {
    wimp_report (error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
    exit (1);
  }
}

/* ================================================================================================================== */

wimp_error_box_selection wimp_report (os_error *error, wimp_error_box_flags type, wimp_error_box_flags buttons,
                                      char *custom_buttons)
{
  wimp_error_box_selection click;
  wimp_error_box_flags     flags;

  if (custom_buttons != NULL && *custom_buttons != '\0')
  {
    flags = wimp_ERROR_BOX_GIVEN_CATEGORY | (type << wimp_ERROR_BOX_CATEGORY_SHIFT);
    click = wimp_report_error_by_category (error, flags, app_name, app_sprite, wimpspriteop_AREA, custom_buttons) - 2;
  }
  else
  {
    flags = wimp_ERROR_BOX_GIVEN_CATEGORY | buttons | (type << wimp_ERROR_BOX_CATEGORY_SHIFT);
    click = wimp_report_error_by_category (error, flags, app_name, app_sprite, wimpspriteop_AREA, NULL);
  }

  return (click);
}
