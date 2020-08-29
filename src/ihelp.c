/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: ihelp.c
 *
 * Interactive help implementation.
 */

/* ANSI C header files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/help.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "ihelp.h"
#include "debug.h"
#include "errors.h"
#include "event.h"
#include "general.h"
#include "icons.h"
#include "string.h"
#include "msgs.h"


#define OBJECT_NAME_LENGTH 13
#define OBJECT_MODIFIER_LENGTH 13
#define MENU_TOKEN_LENGTH 64
#define TOKEN_LENGTH 128

/* ==================================================================================================================
 * Data structures
 */

struct ihelp_window {
	wimp_w			window;
	char			name[OBJECT_NAME_LENGTH];
	char			modifier[OBJECT_MODIFIER_LENGTH];
	void			(*pointer_location) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state);

	struct ihelp_window	*next;
};


struct ihelp_menu {
	wimp_menu		*menu;
	char			name[OBJECT_NAME_LENGTH];

	struct ihelp_menu	*next;
};

/* ==================================================================================================================
 * Global variables.
 */

static struct ihelp_window	*windows = NULL;
static struct ihelp_menu	*menus = NULL;
static char			default_menu_help_token[MENU_TOKEN_LENGTH];


/* Function prototypes */

static struct ihelp_window	*ihelp_find_window(wimp_w window);
static struct ihelp_menu	*ihelp_find_menu(wimp_menu *menu);
static osbool			ihelp_send_reply_help_request(wimp_message *message);
static char			*ihelp_get_text(char *buffer, size_t length, wimp_w window, wimp_i icon, os_coord pos, wimp_mouse_state buttons);


/**
 * Initialise the interactive help system.
 */

void ihelp_initialise(void)
{
	event_add_message_handler(message_HELP_REQUEST, EVENT_MESSAGE_INCOMING, ihelp_send_reply_help_request);
}


/**
 * Add a new interactive help window definition.
 *
 * This is an external interface, documented in ihelp.h.
 */

void ihelp_add_window(wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state))
{
	struct ihelp_window	*new;

	if (ihelp_find_window(window) != NULL)
		return;

	new = malloc(sizeof(struct ihelp_window));

	if (new == NULL)
		return;

	new->window = window;
	string_copy(new->name, name, OBJECT_NAME_LENGTH);
	*(new->modifier) = '\0';
	new->pointer_location = decode;

	new->next = windows;
	windows = new;
}


/**
 * Remove an interactive help definition from the window list.
 *
 * This is an external interface, documented in ihelp.h.
 */

void ihelp_remove_window(wimp_w window)
{
	struct ihelp_window	**list, *del;

	/* Delink the block and delete it. */

	list = &windows;

	while (*list != NULL && (*list)->window != window)
		list = &((*list)->next);

	if (*list == NULL)
		return;

	del = *list;

	*list = del->next;
	free(del);
}


/**
 * Change the window token modifier for a window's interactive help definition.
 *
 * This is an external interface, documented in ihelp.h.
 */

void ihelp_set_modifier(wimp_w window, char *modifier)
{
	struct ihelp_window	*window_data;

	window_data = ihelp_find_window(window);

	if (window_data == NULL)
		return;

	string_copy(window_data->modifier, (modifier != NULL) ? modifier : "", OBJECT_MODIFIER_LENGTH);
}


/**
 * Find an interactive help block based on the given window handle.
 *
 * \param window		The window handle to find.
 * \return			Pointer to the ihelp block, or NULL.
 */

static struct ihelp_window *ihelp_find_window(wimp_w window)
{
	struct ihelp_window	*list;

	list = windows;

	while (list != NULL && list->window != window)
		list = list->next;

	return list;
}


/**
 * Add a new interactive help menu definition.
 *
 * This is an external interface, documented in ihelp.h.
 */

void ihelp_add_menu(wimp_menu *menu, char* name)
{
	struct ihelp_menu	*new;

	if (ihelp_find_menu(menu) != NULL)
		return;

	new = malloc(sizeof(struct ihelp_menu));

	if (new == NULL)
		return;

	new->menu = menu;
	string_copy(new->name, name, OBJECT_NAME_LENGTH);

	new->next = menus;
	menus = new;
}


/**
 * Remove an interactive help definition from the menu list.
 *
 * This is an external interface, documented in ihelp.h.
 */

void ihelp_remove_menu(wimp_menu *menu)
{
	struct ihelp_menu	**list, *del;

	/* Delink the block and delete it. */

	list = &menus;

	while (*list != NULL && (*list)->menu != menu)
		list = &((*list)->next);

	if (*list == NULL)
		return;

	del = *list;

	*list = del->next;
	free(del);
}


/**
 * Update the interactive help token to be supplied for undefined
 * menus.
 *
 * \param *token	The token to use, or NULL to unset.
 */

void ihelp_set_default_menu_token(char *token)
{
	string_copy(default_menu_help_token, (token != NULL) ? token : "", MENU_TOKEN_LENGTH);
}


/**
 * Find an interactive help block based on the given menu handle.
 *
 * \param menu			The menu handle to find.
 * \return			Pointer to the ihelp block, or NULL.
 */

static struct ihelp_menu *ihelp_find_menu(wimp_menu *menu)
{
	struct ihelp_menu	*list;

	list = menus;

	while (list != NULL && list->menu != menu)
		list = list->next;

	return list;
}


/**
 * Respond to a Message_HelpRequest.
 *
 * \param *message		The message to reply to.
 * \return			TRUE if the message was handled; else FALSE.
 */

static osbool ihelp_send_reply_help_request(wimp_message *message)
{
	os_error			*error;

	help_full_message_request	*help_request = (help_full_message_request *) message;
	help_full_message_reply		help_reply;

	ihelp_get_text(help_reply.reply, 236, help_request->w, help_request->i, help_request->pos, help_request->buttons);

	if (*help_reply.reply != '\0') {
		help_reply.size = WORDALIGN(21 + strlen(help_reply.reply));
		help_reply.your_ref = help_request->my_ref;
		help_reply.action = message_HELP_REPLY;

		error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &help_reply, help_request->sender);
		if (error != NULL)
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
	}

	return TRUE;
}


/**
 * Take window and icon handles, pointer position and button state, and return the
 * required help text into the buffer supplied.
 *
 * \param *buffer		A buffer to take the interactive help text.
 * \param length		The size of the buffer, in bytes.
 * \param window		The applicable window handle.
 * \param icon			The applicable icon handle.
 * \param pos			The applicable mouse position.
 * \param buttons		The applicable mouse button state.
 * \return			A pointer to the buffer.
 */

static char *ihelp_get_text(char *buffer, size_t length, wimp_w window, wimp_i icon, os_coord pos, wimp_mouse_state buttons)
{
	char			help_text[IHELP_LENGTH], token[TOKEN_LENGTH], icon_name[IHELP_INAME_LEN];
	struct ihelp_window	*window_data;
	struct ihelp_menu	*menu_data;
	wimp_menu		*current_menu;
	wimp_selection		menu_selection;
	int			i;
	osbool			found;


	/* Set the buffer to a null string, to return no text if a result isn't found. */

	*buffer = '\0';


	if (window == wimp_ICON_BAR) {
		/* Special case, if the window is the iconbar. */

		if (msgs_lookup_result("Help.IconBar", help_text, IHELP_LENGTH))
			string_copy(buffer, help_text, length);
	} else if ((window_data = ihelp_find_window(window)) != NULL) {
		/* Otherwise, if the window is one of the windows registered for interactive help. */

		found = FALSE;
		*icon_name = '\0';

		/* If the window supplied a decoding function, call that to get an 'icon name'. */

		if (window_data->pointer_location != NULL)
			(window_data->pointer_location)(icon_name, window, icon, pos, buttons);

		/* If there wasn't a decoding function, or it didn't help, try to get the icon name from the validation string.
		 * If the icon isn't validated, make a string of the form IconX where X is the number.
		 */

		if (*icon_name == '\0' && icon >= 0 && !icons_get_validation_command(icon_name, IHELP_INAME_LEN, window, icon, 'N'))
			string_printf(icon_name, IHELP_INAME_LEN, "Icon%d", icon);

		/* If an icon name was found from somewhere, look up a token based on that name. */

		if (*icon_name != '\0') {
			string_printf(token, TOKEN_LENGTH, "Help.%s%s.%s", window_data->name, window_data->modifier, icon_name);
			found = msgs_lookup_result(token, help_text, IHELP_LENGTH);
		}

		/* If the icon did not have a name, or it is the window background, look up a token for the window. */

		if (!found) {
			string_printf(token, TOKEN_LENGTH, "Help.%s%s", window_data->name, window_data->modifier);
			found = msgs_lookup_result(token, help_text, IHELP_LENGTH);
		}

		/* If a message was found, return it. */

		if (found)
			string_copy(buffer, help_text, length);
	} else {
		/* Otherwise, try the window as a menu structure. */

		wimp_get_menu_state(0, &menu_selection, 0, 0);
		current_menu = event_get_current_menu();

		/* The list will be null if this isn't a menu belonging to us (or it isn't a menu at all...). */

		if (menu_selection.items[0] != -1 && current_menu != NULL) {
			menu_data = ihelp_find_menu(current_menu);

			if (menu_data != NULL || *default_menu_help_token != '\0') {
				string_printf(token, TOKEN_LENGTH, "Help.%s.", (menu_data == NULL) ? default_menu_help_token : menu_data->name);

				for (i=0; menu_selection.items[i] != -1; i++) {
					string_printf(icon_name, IHELP_INAME_LEN, "%02x", menu_selection.items[i]);
					strncat(token, icon_name, TOKEN_LENGTH - (strlen(icon_name) + 1));
				}

				if (msgs_lookup_result(token, help_text, IHELP_LENGTH))
					string_copy(buffer, help_text, length);
			}
		}
	}

	return buffer;
}

