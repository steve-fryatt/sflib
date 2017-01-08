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
 * \file: ihelp.h
 *
 * Interactive help interface.
 */

#ifndef SFLIB_IHELP
#define SFLIB_IHELP

/**
 * The amount of space allocated for interactive help messages.
 */

#define IHELP_LENGTH 236

/**
 * The size of buffer supplied to decode functions for creating icon names.
 */

#define IHELP_INAME_LEN 64


/**
 * Initialise the interactive help system.
 */

void ihelp_initialise(void);


/**
 * Add a new interactive help window definition.
 *
 * \param window		The window handle to attach help to.
 * \param *name			The token name to associate with the window.
 * \param *decode		A function to use to help decode clicks in the window.
 */

void ihelp_add_window(wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state));


/**
 * Remove an interactive help definition from the window list.
 *
 * \param window		The window handle to remove from the list.
 */

void ihelp_remove_window(wimp_w window);


/**
 * Change the window token modifier for a window's interactive help definition.
 *
 * \param window		The window handle to update.
 * \param *modifier		The new window token modifier text.
 */

void ihelp_set_modifier(wimp_w window, char *modifier);


/**
 * Add a new interactive help menu definition.
 *
 * \param *menu			The menu handle to attach help to.
 * \param *name			The token name to associate with the menu.
 */

void ihelp_add_menu(wimp_menu *menu, char* name);


/**
 * Remove an interactive help definition from the menu list.
 *
 * \param *menu			The menu handle to remove from the list.
 */

void ihelp_remove_menu(wimp_menu *menu);


/**
 * Update the interactive help token to be supplied for undefined
 * menus.
 *
 * \param *token	The token to use, or NULL to unset.
 */

void ihelp_set_default_menu_token(char *token);

#endif

