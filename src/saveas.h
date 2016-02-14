/* Copyright 2012-2015, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: saveas.h
 *
 * Save dialogue implementation.
 */

#ifndef SFLIB_SAVEAS
#define SFLIB_SAVEAS

/* ==================================================================================================================
 * Static constants
 */


/**
 * A savebox data handle.
 */

struct saveas_block;

/**
 * Initialise the SaveAs dialogue box system.
 *
 * \param *dialogue		Pointer to the template name for the standard SaveAs
 *				dialogue, or NULL for none.
 * \param *select_dialogue	Pointer to the template name for the SaveAs dialogue
 *				with 'Selection' option, or NULL for none.
 */

void saveas_initialise(char *dialogue, char *select_dialogue);


/**
 * Create a new save dialogue definition.
 *
 * \param: selection		TRUE if the dialogue has a Selection switch; FALSE if not.
 * \param: *sprite		Pointer to the sprite name for the dialogue.
 * \param: *save_callback	The callback function for saving data.
 * \return			The handle to use for the new save dialogue, or NULL
 *				on failure.
 */

struct saveas_block *saveas_create_dialogue(osbool selection, char *sprite, osbool (*save_callback)(char *filename, osbool selection, void *data));


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

void saveas_initialise_dialogue(struct saveas_block *handle, char*filename, char *fullname, char *selectname, osbool selection, osbool selected, void *data);


/**
 * Prepare the physical save dialogue based on the current state of the given
 * dialogue data.  Any existing data will be saved into the appropriate
 * dialogue definition first.
 *
 * \param *handle		The handle of the save dialogue to prepare.
 */

void saveas_prepare_dialogue(struct saveas_block *handle);


/**
 * Open a Save As dialogue at the pointer.
 *
 * \param *handle		The handle of the save dialogue to be opened.
 * \param *pointer		The pointer location to open the dialogue.
 */

void saveas_open_dialogue(struct saveas_block *handle, wimp_pointer *pointer);

#endif

