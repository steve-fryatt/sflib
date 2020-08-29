/* Copyright 2009-2016, Stephen Fryatt
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
 * \file: templates.c
 *
 * Window and menu template handling.
 */

/* ANSI C header files */

#include <stdlib.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "templates.h"
#include "errors.h"
#include "menus.h"
#include "msgs.h"
#include "windows.h"


static menu_template	menu_definitions;					/**< The menu definition block handle.				*/


/**
 * Open the window templates file for processing.
 *
 * \param *file		The template file to open.
 */

void templates_open(char *file)
{
	os_error	*error;

	error = xwimp_open_template(file);
	if (error != NULL)
		error_report_program(error);
}


/**
 * Close the window templates file.
 */

void templates_close(void)
{
	wimp_close_template();
}


/**
 * Load a window definition from the current templates file.
 *
 * \param *name		The name of the template to load.
 * \return		Pointer to the loaded definition, or NULL.
 */

wimp_window *templates_load_window(char *name)
{
	wimp_window	*definition;

	definition = windows_load_template(name);

	if (definition == NULL)
		error_msgs_param_report_fatal("BadTemplate:Window template '%0' not found.", name, NULL, NULL, NULL);

	return definition;
}


/**
 * Create a window from the current templates file.
 *
 * \param *name		The name of the window to create.
 * \return		The window handle if the new window.
 */

wimp_w templates_create_window(char *name)
{
	wimp_window	*definition;
	wimp_w		w = NULL;

	definition = windows_load_template(name);

	if (definition != NULL) {
		w = wimp_create_window(definition);
		free(definition);
	} else {
		error_msgs_param_report_fatal("BadTemplate:Window template '%0' not found.", name, NULL, NULL, NULL);
	}

	return w;
}


/**
 * Load the menu file into memory and link in any dialogue boxes.  The
 * templates file must be open when this function is called.
 *
 * \param *file		The filename of the menu file.
 */

void templates_load_menus(char *file)
{
	menu_definitions = menus_load_templates(file, NULL, NULL, 0);

	if (menu_definitions == NULL)
		error_msgs_param_report_fatal("MenuLoad:Failed to open menus file: '%0'.", file, NULL, NULL, NULL);
}


/**
 * Link a dialogue box to the menu block once the template has been
 * loaded into memory.
 *
 * \param *dbox		The dialogue box name in the menu tree.
 * \param w		The window handle to link.
 */

void templates_link_menu_dialogue(char *dbox, wimp_w w)
{
	menus_link_dbox(menu_definitions, dbox, w);
}


/**
 * Return a menu handle based on a menu file tag name.
 *
 * \param *menu		The menu tag of the required menu.
 * \return		The menu block pointer, or NULL for an invalid index.
 */

wimp_menu *templates_get_menu(char *menu)
{
	wimp_menu *handle;

	handle = menus_get_menu(menu_definitions, menu);

	if (handle == NULL)
		error_msgs_param_report_fatal("BadMenu:Menu template '%0' not found.", menu, NULL, NULL, NULL);

	return handle;
}

