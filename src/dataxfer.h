/* Copyright 2003-2016, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: dataxfer.h
 *
 * Save dialogues and data transfer implementation.
 */

#ifndef SFLIB_DATAXFER
#define SFLIB_DATAXFER

#include <stddef.h>
#include "oslib/wimp.h"

/* ==================================================================================================================
 * Static constants
 */

/**
 * Filetypes which aren't defined in OSLib.
 */

#define dataxfer_TYPE_LOCATE	0x1a1u
#define dataxfer_TYPE_CASHBOOK	0x1cau
#define dataxfer_TYPE_PRINTPDF	0x1d8u
#define dataxfer_TYPE_PUZZLE	0x1f2u
#define dataxfer_TYPE_PDF	0xadfu
#define dataxfer_TYPE_FANCYTEXT	0xaf8u
#define dataxfer_TYPE_CSV	0xdfeu
#define dataxfer_TYPE_HTML	0xfafu
#define dataxfer_TYPE_TSV	0xfffu

/**
 * Datatransfer memory handlers.
 */

struct dataxfer_memory {
	void *(*alloc)(size_t size);			/**< eg. malloc().	*/
	void *(*realloc)(void *ptr, size_t size);	/**< eg. realloc().	*/
	void (*free)(void *ptr);			/**< eg. free().	*/
};


/**
 * Initialise the data transfer system.
 *
 * \param task_handle	The task handle of the client task.
 * \param *handlers	Pointer to memory allocation functions, if RAM Transfers are to be used.
 */

void dataxfer_initialise(wimp_t task_handle, struct dataxfer_memory *handlers);


/**
 * Start dragging from a window work area, creating a sprite to drag and starting
 * a drag action.  When the action completes, a callback will be made to the
 * supplied function.
 *
 * \param w		The window where the drag is starting.
 * \param *pointer	The current pointer state.
 * \param *extent	The extent of the drag box, relative to the window work area.
 * \param *sprite	Pointer to the name of the sprite to use for the drag, or NULL.
 * \param callback	A callback function
 */

void dataxfer_work_area_drag(wimp_w w, wimp_pointer *pointer, os_box *extent, char *sprite, void (* drag_end_callback)(wimp_pointer *pointer, void *data), void *drag_end_data);


/**
 * Start dragging an icon from a dialogue, creating a sprite to drag and starting
 * a drag action.  When the action completes, a callback will be made to the
 * supplied function.
 *
 * \param w		The window where the drag is starting.
 * \param i		The icon to be dragged.
 * \param callback	A callback function
 */

void dataxfer_save_window_drag(wimp_w w, wimp_i i, void (* drag_end_callback)(wimp_pointer *pointer, void *data), void *drag_end_data);


/**
 * Start a clipboard data request operation: the data transfer protocol will
 * be started and, if data is received, the callback will be called with details
 * of where it can be found.
 *
 * \param w			The window to which the data will be targetted.
 * \param i			The icon to which the data will be targetted.
 * \param pos			The position of the caret.
 * \param types[]		A list of acceptable filetypes, terminated by -1.
 * \param *receive_callback	The function to be called when the data has
 *				been received.
 * \param *data			Data to be passed to the callback function.
 * \return			TRUE on success; FALSE on failure.
 */

osbool dataxfer_request_clipboard(wimp_w w, wimp_i i, os_coord pos, bits types[], osbool (*receive_callback)(void *content, size_t size, bits type, void *data), void *data);


/**
 * Start a data save action by sending a message to another task.  The data
 * transfer protocol will be started, and at an appropriate time a callback
 * will be made to save the data.
 *
 * \param *pointer		The Wimp pointer details of the save target.
 * \param *name			The proposed file leafname.
 * \param size			The estimated file size.
 * \param type			The proposed file type.
 * \param your_ref		The "your ref" to use for the opening message, or 0.
 * \param *save_callback	The function to be called with the full pathname
 *				to save the file.
 * \param *data			Data to be passed to the callback function.
 * \return			TRUE on success; FALSE on failure.
 */

osbool dataxfer_start_save(wimp_pointer *pointer, char *name, int size, bits type, int your_ref, osbool (*save_callback)(char *filename, void *data), void *data);


/**
 * Start a data load action for another task by sending it a message containing
 * the name of the file that it should take.
 *
 * \param *pointer		The Wimp pointer details of the save target.
 * \param *name			The full pathname of the file.
 * \param size			The estimated file size.
 * \param type			The proposed file type.
 * \param your_ref		The "your ref" to use for the opening message, or 0.
 * \return			TRUE on success; FALSE on failure.
 */

osbool dataxfer_start_load(wimp_pointer *pointer, char *name, int size, bits type, int your_ref);


/**
 * Register a function to provide clipboard data on request. When called,
 * if the clipboard is currently held by the client and one of the types listed
 * in types[] is an acceptable format, the contents should be copied to a block
 * in the provided heap and the details passed back (pointer to the data in the
 * supplied pointer, type updated to the chosen type, and return the size of the
 * data). If no data is available, it should leave the pointer as NULL and
 * return a size of zero.
 *
 * \param callback		The clipboard data request callback, or NULL
 *				to unset.
 */

void dataxfer_register_clipboard_provider(size_t callback(bits types[], bits *type, void **data));


/**
 * Specify a handler for files which are dragged into a window. Files which match
 * on type, window handle and icon are passed to the appropriate handler for
 * attention.
 *
 * \param filetype		The filetype to register as a target.
 * \param w			The target window, or NULL.
 * \param i			The target icon, or -1.
 * \param *intermediate		Pointer to the intermediate filename to use for the Data
 *				Transfer Protocol, or NULL for default <Wimp$Scrap>.
 * \param *callback		The load callback function.
 * \param *data			Data to be passed to load functions, or NULL.
 * \return			TRUE if successfully registered; else FALSE.
 */

osbool dataxfer_set_drop_target(unsigned filetype, wimp_w w, wimp_i i, char *intermediate,
		osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data);


/**
 * Specify a handler for files which are double-clicked. Files which match
 * on type, are passed to the appropriate handler for attention.
 *
 * \param filetype		The filetype to register as a target.
 * \param *callback		The load callback function.
 * \param *data			Data to be passed to load functions, or NULL.
 * \return			TRUE if successfully registered; else FALSE.
 */

osbool dataxfer_set_load_type(unsigned filetype,
		osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data);


/**
 * Remove a handler for files which are dragged into a window.
 *
 * \param filetype		The filetype to delete as a target.
 * \param w			The target window to delete, or NULL.
 * \param i			The target icon to delete, or -1.
 */

void dataxfer_delete_drop_target(unsigned filetype, wimp_w w, wimp_i i);


/**
 * Remove a handler for files which are double-clicked.
 *
 * \param filetype		The filetype to delete as a target.
 */

void dataxfer_delete_load_type(unsigned filetype);

#endif
