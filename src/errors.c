/**
 * \file: errors.c
 *
 * SF-Lib - Errors.c
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * Wimp error handling and message box support.
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


#define ERROR_BUTTON_LENGTH 255							/**< The size of the buffer for expanding custom button message tokens.		*/


static char			*app_name = NULL;				/**< The application name, as used in error messages.				*/
static char			*app_sprite = NULL;				/**< The application sprite, as used in error messages.				*/
static void			*close_down_function;				/**< Unused.									*/

static wimp_error_box_selection		error_wimp_os_report(os_error *error,
		wimp_error_box_flags type, wimp_error_box_flags buttons, char *custom_buttons);



/* Initialise the error message module.
 *
 * This is an external interface, documented in errors.h
 */

void error_initialise(char *name, char *sprite, void (*closedown)(void))
{
	char	lookup_buffer[256];

	close_down_function = closedown;

	if (name != NULL) {
		if (app_name != NULL)
			free(app_name);

		msgs_lookup(name, lookup_buffer, sizeof(lookup_buffer));
		app_name = strdup(lookup_buffer);
	}

	if (sprite != NULL) {
		if (app_sprite != NULL)
			free(app_sprite);

		msgs_lookup(sprite, lookup_buffer, sizeof(lookup_buffer));
		app_sprite = strdup(lookup_buffer);
	}
}


/**
 * Display a Wimp error box on the screen, using the specified type.  Either use
 * the specified standard buttons, or a set of custom buttons.
 *
 * \param *error		An Error Block defining the error and the message
 *				to display.
 * \param type			The error box type, from wimp_ERROR_BOX_CATEGORY_INFO,
 *				wimp_ERROR_BOX_CATEGORY_ERROR, wimp_ERROR_BOX_CATEGORY_PROGRAM
 *				or wimp_ERROR_BOX_CATEGORY_QUESTION.
 * \param buttons		The buttons to use, if *custom_buttons is NULL, from
 *				wimp_ERROR_BOX_OK_ICON and wimp_ERROR_BOX_CANCEL_ICON .
 * \param *custom_buttons	A comma-separated list of custom button texts.
 * \return			The selected button, counting from 1.
 */

static wimp_error_box_selection error_wimp_os_report(os_error *error, wimp_error_box_flags type,
		wimp_error_box_flags buttons, char *custom_buttons)
{
	wimp_error_box_selection	click;
	wimp_error_box_flags		flags;

	if (custom_buttons != NULL && *custom_buttons != '\0') {
		flags = wimp_ERROR_BOX_GIVEN_CATEGORY | (type << wimp_ERROR_BOX_CATEGORY_SHIFT);
		click = wimp_report_error_by_category(error, flags, app_name, app_sprite, wimpspriteop_AREA, custom_buttons) - 2;
	} else {
		flags = wimp_ERROR_BOX_GIVEN_CATEGORY | buttons | (type << wimp_ERROR_BOX_CATEGORY_SHIFT);
		click = wimp_report_error_by_category(error, flags, app_name, app_sprite, wimpspriteop_AREA, NULL);
	}

	return click;
}


/* Display a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containing
 * details of the error as contained in an Error Block.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_report_os_error(os_error *error, wimp_error_box_flags buttons)
{
	if (error != NULL)
		return error_wimp_os_report(error, wimp_ERROR_BOX_CATEGORY_ERROR, buttons, NULL);
	else
		return wimp_ERROR_BOX_SELECTED_NOTHING;
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_INFO, containg the
 * message looked up via the given MessageTrans token and an OK button.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_msgs_report_info(char *token)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	msgs_lookup(token, error.errmess, os_ERROR_LIMIT);

	return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_INFO, wimp_ERROR_BOX_OK_ICON, NULL);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_INFO, containg the
 * given message and an OK button.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_report_info(char *message)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	strncpy(error.errmess, message, os_ERROR_LIMIT);

	return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_INFO, wimp_ERROR_BOX_OK_ICON, NULL);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containg the
 * message looked up via the given MessageTrans token and an OK button.
 *
 * This is an external interface, documented in errors.h
 */


wimp_error_box_selection error_msgs_report_error(char *token)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	msgs_lookup(token, error.errmess, os_ERROR_LIMIT);

	return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_ERROR, wimp_ERROR_BOX_OK_ICON, NULL);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containg the
 * given message and an OK button.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_report_error(char *message)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	strncpy(error.errmess, message, os_ERROR_LIMIT);

	return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_ERROR, wimp_ERROR_BOX_OK_ICON, NULL);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_QUESTION, containg the
 * message looked up via the given MessageTrans token and either OK and
 * Cancel buttons or buttons as specified in the comma-separated list
 * contained in the buttons token.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_msgs_report_question(char *token, char *buttons)
{
	os_error	error;
	char		button_text[ERROR_BUTTON_LENGTH];

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	msgs_lookup(token, error.errmess, os_ERROR_LIMIT);

	if (buttons == NULL) {
		return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_QUESTION,
				wimp_ERROR_BOX_OK_ICON | wimp_ERROR_BOX_CANCEL_ICON, NULL);
	} else {
		msgs_lookup(buttons, button_text, ERROR_BUTTON_LENGTH);
		return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_QUESTION, 0, button_text);
	}
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_QUESTION, containg the
 * given message and either OK and Cancel buttons or buttons as specified in
 * the comma-separated list.
 *
 * This is an external interface, documented in errors.h
 */

wimp_error_box_selection error_report_question(char *message, char *buttons)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	strncpy(error.errmess, message, os_ERROR_LIMIT);

	if (buttons == NULL)
		return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_QUESTION,
				wimp_ERROR_BOX_OK_ICON | wimp_ERROR_BOX_CANCEL_ICON, NULL);
	else
		return error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_QUESTION, 0, buttons);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg the
 * message looked up via the given MessageTrans token and a Cancel button.
 *
 * This function never returns.
 *
 * This is an external interface, documented in errors.h
 */

void error_msgs_report_fatal(char *token)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	msgs_lookup(token, error.errmess, os_ERROR_LIMIT);

	error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
	exit(1);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg the
 * given message and a Cancel button.
 *
 * This function never returns.
 *
 * This is an external interface, documented in errors.h
 */

void error_report_fatal(char *message)
{
	os_error	error;

	error.errnum = 255; /* A dummy error number, which should probably be checked. */
	strncpy(error.errmess, message, os_ERROR_LIMIT);

	error_wimp_os_report(&error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
	exit(1);
}


/* Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg
 * details of the error in an Error Block and a Cancel button.
 *
 * This function never returns.
 */

void error_report_program(os_error *error)
{
	if (error != NULL) {
		error_wimp_os_report(error, wimp_ERROR_BOX_CATEGORY_PROGRAM, wimp_ERROR_BOX_CANCEL_ICON, NULL);
		exit(1);
	}
}

