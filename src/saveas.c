/* Copyright 2012-2016, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: saveas.c
 *
 * Save dialogue implementation.
 */

/* ANSI C header files */

#include <string.h>
#include <stdio.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/dragasprite.h"
#include "oslib/osbyte.h"
#include "oslib/osfscontrol.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/dataxfer.h"
#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/general.h"
#include "sflib/icons.h"
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "saveas.h"

#define SAVEAS_ICON_SAVE 0
#define SAVEAS_ICON_CANCEL 1
#define SAVEAS_ICON_FILENAME 2
#define SAVEAS_ICON_FILE 3
#define SAVEAS_ICON_SELECTION 4


#define SAVEAS_MAX_FILENAME 256
#define SAVEAS_MAX_SPRNAME 16

/* ==================================================================================================================
 * Global variables.
 */

/**
 * Data associated with the Save As window.
 */

struct saveas_block {
	char				full_filename[SAVEAS_MAX_FILENAME];		/**< The full filename to be used in the savebox.			*/
	char				selection_filename[SAVEAS_MAX_FILENAME];	/**< The selection filename to be used in the savebox.			*/
	char				sprite[SAVEAS_MAX_SPRNAME];			/**< The sprite to be used in the savebox.				*/

	wimp_w				window;						/**< The window handle of the savebox to be used.			*/
	osbool				selection;					/**< TRUE if the selection icon is enabled; else FALSE.			*/
	osbool				selected;					/**< TRUE if the selection icon is ticked; else FALSE.			*/

	osbool				(*callback)
			(char *filename, osbool selection, void *data);			/**< The callback function to be used if a save is required.		*/
	void				*callback_data;					/**< Data to be passed to the callback function.			*/
};


static wimp_w				saveas_window = NULL;				/**< The Save As window handle.						*/
static wimp_w				saveas_sel_window = NULL;			/**< The Save As window with selection icon.				*/


/**
 * Function prototypes.
 */

static void				saveas_click_handler(wimp_pointer *pointer);
static osbool				saveas_keypress_handler(wimp_key *key);

static void				saveas_drag_end_handler(wimp_pointer *pointer, void *data);
static osbool				saveas_save_handler(char *filename, void *data);
static void				saveas_immediate_save(struct saveas_block *handle);



/**
 * Initialise the SaveAs dialogue box system.
 *
 * \param *dialogue		Pointer to the template name for the standard SaveAs
 *				dialogue, or NULL for none.
 * \param *select_dialogue	Pointer to the template name for the SaveAs dialogue
 *				with 'Selection' option, or NULL for none.
 */

void saveas_initialise(char *dialogue, char *select_dialogue)
{
	if (dialogue != NULL) {
		saveas_window = templates_create_window(dialogue);
		ihelp_add_window(saveas_window, "SaveAs", NULL);
		event_add_window_mouse_event(saveas_window, saveas_click_handler);
		event_add_window_key_event(saveas_window, saveas_keypress_handler);
		templates_link_menu_dialogue("SaveAs", saveas_window);
	} else {
		saveas_window = NULL;
	}

	if (select_dialogue != NULL) {
		saveas_sel_window = templates_create_window(select_dialogue);
		ihelp_add_window(saveas_sel_window, "SaveAs", NULL);
		event_add_window_mouse_event(saveas_sel_window, saveas_click_handler);
		event_add_window_key_event(saveas_sel_window, saveas_keypress_handler);
		templates_link_menu_dialogue("SaveAsSel", saveas_sel_window);
	} else {
		saveas_sel_window = NULL;
	}
}


/**
 * Create a new save dialogue definition.
 *
 * \param: selection		TRUE if the dialogue has a Selection switch; FALSE if not.
 * \param: *sprite		Pointer to the sprite name for the dialogue.
 * \param: *save_callback	The callback function for saving data.
 * \return			The handle to use for the new save dialogue, or NULL
 *				on failure.
 */

struct saveas_block *saveas_create_dialogue(osbool selection, char *sprite, osbool (*save_callback)(char *filename, osbool selection, void *data))
{
	struct saveas_block		*new;

	/* Check that the requested dialogue exists first. */

	if ((selection == FALSE && saveas_window == NULL) || (selection == TRUE && saveas_sel_window == NULL))
		return NULL;

	/* Claim the required memory for the dialogue definition. */

	new = malloc(sizeof(struct saveas_block));
	if (new == NULL)
		return NULL;

	/* Set up the dialogue. */

	new->full_filename[0] = '\0';
	new->selection_filename[0] = '\0';
	strncpy(new->sprite, (sprite != NULL) ? sprite : "", SAVEAS_MAX_SPRNAME);

	new->window = (selection) ? saveas_sel_window : saveas_window;
	new->selection = FALSE;
	new->selected = FALSE;
	new->callback = save_callback;
	new->callback_data = NULL;

	return new;
}


/**
 * Initialise a save dialogue definition with two filenames, and an indication of
 * the selection icon status.  This is called when opening a menu or creating a
 * dialogue from a toolbar.
 *
 * \param *handle		The handle of the save dialogue to be initialised.
 * \param *filename		Pointer to a filename to use for the full save; else NULL.
 * \param *fullname		Pointer to the filename token for a full save.
 * \param *selectname		Pointer to the filename token for a selection save.
 * \param selection		TRUE if the Selection option is enabled; else FALSE.
 * \param selected		TRUE if the Selection option is selected; else FALSE.
 * \param *data			Data to pass to any save callbacks, or NULL.
 */

void saveas_initialise_dialogue(struct saveas_block *handle, char *filename, char *fullname, char *selectname, osbool selection, osbool selected, void *data)
{
	if (handle == NULL)
		return;

	if (filename != NULL && *filename != '\0') {
		strncpy(handle->full_filename, filename, SAVEAS_MAX_FILENAME);
		handle->full_filename[SAVEAS_MAX_FILENAME - 1] = '\0';
	} else if (fullname != NULL) {
		msgs_lookup(fullname, handle->full_filename, SAVEAS_MAX_FILENAME);
	} else {
		handle->full_filename[0] = '\0';
	}

	if (selectname != NULL)
		msgs_lookup(selectname, handle->selection_filename, SAVEAS_MAX_FILENAME);
	else
		handle->selection_filename[0] = '\0';

	handle->selection = selection;
	handle->selected = (selection) ? selected : FALSE;
	handle->callback_data = data;

	event_add_window_user_data(handle->window, NULL);
}


/**
 * Prepare the physical save dialogue based on the current state of the given
 * dialogue data.  Any existing data will be saved into the appropriate
 * dialogue definition first.
 *
 * \param *handle		The handle of the save dialogue to prepare.
 */

void saveas_prepare_dialogue(struct saveas_block *handle)
{
	struct saveas_block		*old_handle;

	if (handle == NULL)
		return;

	old_handle = event_get_window_user_data(handle->window);
	if (old_handle != NULL) {
		if (old_handle->window == saveas_window) {
			icons_copy_text(old_handle->window, SAVEAS_ICON_FILENAME, old_handle->full_filename, SAVEAS_MAX_FILENAME);
			old_handle->selected = FALSE;
		} else {
			old_handle->selected = icons_get_selected(old_handle->window, SAVEAS_ICON_SELECTION);
			icons_copy_text(old_handle->window, SAVEAS_ICON_FILENAME,
					(old_handle->selected) ? old_handle->selection_filename : old_handle->full_filename, SAVEAS_MAX_FILENAME);
		}
	}

	event_add_window_user_data(handle->window, handle);

	icons_printf(handle->window, SAVEAS_ICON_FILE, handle->sprite);

	if (handle->window == saveas_window) {
		icons_printf(handle->window, SAVEAS_ICON_FILENAME, handle->full_filename);
	} else {
		icons_set_shaded(handle->window, SAVEAS_ICON_SELECTION, !handle->selection);
		icons_set_selected(handle->window, SAVEAS_ICON_SELECTION, handle->selected);
		icons_printf(handle->window, SAVEAS_ICON_FILENAME, (handle->selected) ? handle->selection_filename : handle->full_filename);
	}
}


/**
 * Open a Save As dialogue at the pointer.
 *
 * \param *handle		The handle of the save dialogue to be opened.
 * \param *pointer		The pointer location to open the dialogue.
 */

void saveas_open_dialogue(struct saveas_block *handle, wimp_pointer *pointer)
{
	if (handle == NULL || pointer == NULL)
		return;

	menus_create_standard_menu((wimp_menu *) handle->window, pointer);
}


/**
 * Process mouse clicks in the Save As dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void saveas_click_handler(wimp_pointer *pointer)
{
	struct saveas_block		*handle;

	if (pointer == NULL || (pointer->w != saveas_window && pointer->w != saveas_sel_window))
		return;

	handle = event_get_window_user_data(pointer->w);
	if (handle == NULL)
		return;

	switch (pointer->i) {
	case SAVEAS_ICON_CANCEL:
		if (pointer->buttons == wimp_CLICK_SELECT)
			wimp_create_menu(NULL, 0, 0);
		break;

	case SAVEAS_ICON_SAVE:
		if (pointer->buttons == wimp_CLICK_SELECT)
			saveas_immediate_save(handle);
		break;

	case SAVEAS_ICON_FILE:
		if (pointer->buttons == wimp_DRAG_SELECT)
			dataxfer_save_window_drag(pointer->w, SAVEAS_ICON_FILE, saveas_drag_end_handler, handle);
		break;

	case SAVEAS_ICON_SELECTION:
		handle->selected = icons_get_selected(handle->window, SAVEAS_ICON_SELECTION);
		icons_copy_text(handle->window, SAVEAS_ICON_FILENAME, (handle->selected) ? handle->full_filename : handle->selection_filename, SAVEAS_MAX_FILENAME);
		icons_strncpy(handle->window, SAVEAS_ICON_FILENAME, (handle->selected) ? handle->selection_filename : handle->full_filename);
		wimp_set_icon_state(handle->window, SAVEAS_ICON_FILENAME, 0, 0);
		icons_replace_caret_in_window(handle->window);
		break;
	}
}


/**
 * Process keypresses in the Save As dialogue.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool saveas_keypress_handler(wimp_key *key)
{
	struct saveas_block		*handle;

	handle = event_get_window_user_data(key->w);
	if (handle == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		saveas_immediate_save(handle);
		break;

	case wimp_KEY_ESCAPE:
		wimp_create_menu(NULL, 0, 0);
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Process the termination of icon drags from the Save dialogues.
 *
 * \param *pointer		The pointer location at the end of the drag.
 * \param *data			The saveas_savebox data for the drag.
 */

static void saveas_drag_end_handler(wimp_pointer *pointer, void *data)
{
	struct saveas_block	*handle = data;
	char			*leafname;

	if (handle == NULL)
		return;

	if (handle->window == saveas_window) {
		icons_copy_text(handle->window, SAVEAS_ICON_FILENAME, handle->full_filename, SAVEAS_MAX_FILENAME);
		handle->selected = FALSE;
	} else {
		handle->selected = icons_get_selected(handle->window, SAVEAS_ICON_SELECTION);
		icons_copy_text(handle->window, SAVEAS_ICON_FILENAME, (handle->selected) ? handle->selection_filename : handle->full_filename, SAVEAS_MAX_FILENAME);
	}

	leafname = string_find_leafname((handle->selected) ? handle->selection_filename : handle->full_filename);

	dataxfer_start_save(pointer, leafname, 0, 0xffffffffu, 0, saveas_save_handler, handle);
}


/**
 * Process data transfer results for Save As dialogues.
 *
 * \param *filename		The destination of the dragged file.
 * \param *data			Context data.
 * \return			TRUE if the save succeeded; FALSE if it failed.
 */

static osbool saveas_save_handler(char *filename, void *data)
{
	struct saveas_block		*handle = data;
	osbool				result;

	if (handle == NULL || handle->callback == NULL)
		return FALSE;

	result = handle->callback(filename, handle->selected, handle->callback_data);

	wimp_create_menu(NULL, 0, 0);

	return result;
}


/**
 * Perform an immediate save based on the information in a save
 * dialogue.
 *
 * \param *handle		The handle of the save dialogue.
 */

static void saveas_immediate_save(struct saveas_block *handle)
{
	char	*filename;

	if (handle == NULL)
		return;

	if (handle->window == saveas_window) {
		icons_copy_text(handle->window, SAVEAS_ICON_FILENAME, handle->full_filename, SAVEAS_MAX_FILENAME);
		handle->selected = FALSE;
	} else {
		handle->selected = icons_get_selected(handle->window, SAVEAS_ICON_SELECTION);
		icons_copy_text(handle->window, SAVEAS_ICON_FILENAME, (handle->selected) ? handle->selection_filename : handle->full_filename, SAVEAS_MAX_FILENAME);
	}

	filename = (handle->selected) ? handle->selection_filename : handle->full_filename;

	if (strchr(filename, '.') == NULL) {
		error_msgs_report_info("BadSave");
		return;
	}

	if (handle->callback !=NULL)
		handle->callback(filename, handle->selected, handle->callback_data);

	wimp_create_menu(NULL, 0, 0);
}

