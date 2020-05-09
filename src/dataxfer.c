/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: dataxfer.c
 *
 * Save dialogue and data transfer implementation.
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/dragasprite.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

/* SF-Lib header files. */

#include "dataxfer.h"
#include "debug.h"
#include "errors.h"
#include "event.h"
#include "general.h"
#include "string.h"

#ifdef __CC_NORCROFT
#include "strdup.h"
#endif


/* ==================================================================================================================
 * Global variables.
 */

#define DATAXFER_CLIPBOARD_NAME "Clipboard"

/**
 * The purpose of a transfer.
 */

enum dataxfer_purpose {
	DATAXFER_UNKNOWN = 0,										/**< The transfer has no defined purpose.				*/
	DATAXFER_FILE_SAVE,										/**< The transfer is for saving a file.					*/
	DATAXFER_FILE_LOAD,										/**< The transfer is for loading a file.				*/
	DATAXFER_CLIPBOARD_SEND,									/**< The transfer is for sending the clipboard content.			*/
	DATAXFER_CLIPBOARD_RECEIVE									/**< The transfer is for receiving the clipboard content.		*/
};

/**
 * Data associated with a message exchange.
 */

enum dataxfer_message_type {
	DATAXFER_MESSAGE_NONE = 0,									/**< An unset message block.						*/
	DATAXFER_MESSAGE_SAVE = 1,									/**< A message block associated with a save operation.			*/
	DATAXFER_MESSAGE_LOAD = 2,									/**< A message block associated with a load operation.			*/
	DATAXFER_MESSAGE_REQUEST = 4,									/**< A message block associated with a datarequest operation.		*/
	DATAXFER_MESSAGE_RAMRX = 8,									/**< A message block associated with a RAM receive operation.		*/
	DATAXFER_MESSAGE_RAMTX = 16,									/**< A message block associated with a RAM send operation.		*/
	DATAXFER_MESSAGE_ALL = 31									/**< All message blocks.						*/
};

struct dataxfer_descriptor {
	enum dataxfer_message_type	type;								/**< The type of message that the block describes.			*/
	enum dataxfer_purpose		purpose;							/**< The purpose of the described transfer.				*/

	int				my_ref;								/**< The MyRef of the sent message.					*/
	wimp_t				task;								/**< The task handle of the recipient task.				*/
	osbool				(*save_callback)(char *filename, void *data);			/**< The callback function to be used if a save is required.		*/
	osbool				(*receive_callback)(void *content, size_t size, bits type,
							void *data);					/**< The callback function to be used if clipboard data is received.	*/
	void				*callback_data;							/**< Data to be passed to the callback function.			*/
	char				*intermediate_filename;						/**< The filename to use for the disc-based Data Transfer Protocol.	*/

	byte				*ram_data;							/**< The buffer for RAM transfers.					*/
	size_t				ram_allocation;							/**< The amount of memory being allocated to the RAM buffer.		*/
	size_t				ram_size;							/**< The size of the buffer for RAM transfers.				*/
	size_t				ram_used;							/**< The amount of RAM buffer used.					*/

	wimp_full_message_data_xfer	*saved_message;							/**< A saved data transfer message block.				*/


	struct dataxfer_descriptor	*next;								/**< The next message block in the chain, or NULL.			*/
};

static struct dataxfer_descriptor	*dataxfer_descriptors = NULL;					/**< List of currently active message operations.			*/

/**
 * Data associated with incoming transfer targets.
 */

/**
 * Types of incoming data.
 */

enum dataxfer_target_type {
	DATAXFER_TARGET_NONE = 0,
	DATAXFER_TARGET_LOAD = 1,	/**< Transfers initiated by a Message_DataLoad.				*/
	DATAXFER_TARGET_SAVE = 2,	/**< Transfers initiated by a Message_DataSave.				*/
	DATAXFER_TARGET_DRAG = 3,	/**< Transfers initiated by Message_DataLoad or MessageDataSave.	*/
	DATAXFER_TARGET_OPEN = 4,	/**< Transfers initiated by a Message_DataOpen.				*/
	DATAXFER_TARGET_ALL  = 7	/**< Transfers initialed by all of the above.				*/
};

struct dataxfer_incoming_target {
	enum dataxfer_target_type	target;								/**< The type of initiation to which this target applies.		*/

	unsigned			filetype;							/**< The target filetype.						*/
	wimp_w				window;								/**< The target window (used in window and icon lists).			*/
	wimp_i				icon;								/**< The target icon (used in icon lists).				*/

	osbool				(*callback)(wimp_w w, wimp_i i,
			unsigned filetype, char *filename, void *data);					/**< The callback function to be used if a load is required.		*/
	void				*callback_data;							/**< Data to be passed to the callback function.			*/

	char				*intermediate_filename;						/**< Filename to be used for disc-based transfers.			*/

	struct dataxfer_incoming_target	*children;							/**< Pointer to a list of child targets (window or icon lists).		*/

	struct dataxfer_incoming_target	*next;								/**< The next target in the chain, or NULL.				*/
};

struct dataxfer_incoming_target		*dataxfer_incoming_targets = NULL;				/**< List of defined incoming targets.					*/

/**
 * Data asscoiated with drag box handling.
 */

static osbool	dataxfer_dragging_sprite = FALSE;							/**< TRUE if we're dragging a sprite, FALSE if dragging a dotted-box.	*/
static void	(*dataxfer_drag_end_callback)(wimp_pointer *, void *) = NULL;				/**< The callback function to be used by the icon drag code.		*/

/**
 * Memory allocation handlers.
 */

static struct dataxfer_memory		*dataxfer_memory_handlers = NULL;				/**< Pointer to client functions for handling memory.			*/

/**
 * Clipboard content locator.
 */

static size_t	(*dataxfer_find_clipboard_content)(bits *, bits *, void **) = NULL;			/**< The callback function to ask the client for the clipboard contents	*/

/**
 * Task handle of the client.
 */

static wimp_t	dataxfer_task_handle;

/**
 * Function prototypes.
 */

static void				dataxfer_terminate_user_drag(wimp_dragged *drag, void *data);

static osbool				dataxfer_message_ram_fetch(wimp_message *message);
static osbool				dataxfer_message_data_save_ack(wimp_message *message);
static osbool				dataxfer_message_data_load_ack(wimp_message *message);

static osbool				dataxfer_message_datarequest(wimp_message *message);

static osbool				dataxfer_message_data_save(wimp_message *message);
static osbool				dataxfer_message_ram_fetch_bounced(wimp_message *message);
static osbool				dataxfer_message_ram_transmit(wimp_message *message);
static osbool				dataxfer_message_data_load(wimp_message *message);
static osbool				dataxfer_message_data_open(wimp_message *message);

static osbool				dataxfer_message_bounced(wimp_message *message);

static osbool				dataxfer_set_load_target(enum dataxfer_target_type target, unsigned filetype, wimp_w w, wimp_i i, char *intermediate,
							osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data);
static void				dataxfer_delete_load_target(enum dataxfer_target_type target, unsigned filetype, wimp_w w, wimp_i i);
static struct dataxfer_incoming_target	*dataxfer_find_incoming_target(enum dataxfer_target_type target, wimp_w w, wimp_i i, unsigned filetype);

static struct dataxfer_descriptor	*dataxfer_new_descriptor(void);
static struct dataxfer_descriptor	*dataxfer_find_descriptor(int ref, enum dataxfer_message_type type);
static void				dataxfer_delete_descriptor(struct dataxfer_descriptor *message);


/**
 * Initialise the data transfer system.
 *
 * \param task_handle	The task handle of the client task.
 * \param *handlers	Pointer to memory allocation functions, if RAM Transfers are to be used.
 */

void dataxfer_initialise(wimp_t task_handle, struct dataxfer_memory *handlers)
{
	event_add_message_handler(message_DATA_REQUEST, EVENT_MESSAGE_INCOMING, dataxfer_message_datarequest);
	event_add_message_handler(message_DATA_SAVE, EVENT_MESSAGE_INCOMING, dataxfer_message_data_save);
	event_add_message_handler(message_DATA_SAVE_ACK, EVENT_MESSAGE_INCOMING, dataxfer_message_data_save_ack);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_INCOMING, dataxfer_message_data_load);
	event_add_message_handler(message_DATA_LOAD_ACK, EVENT_MESSAGE_INCOMING, dataxfer_message_data_load_ack);
	event_add_message_handler(message_DATA_OPEN, EVENT_MESSAGE_INCOMING, dataxfer_message_data_open);

	event_add_message_handler(message_DATA_SAVE, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_bounced);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_bounced);
	event_add_message_handler(message_DATA_SAVE_ACK, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_bounced);
	event_add_message_handler(message_DATA_REQUEST, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_bounced);
	
	dataxfer_memory_handlers = handlers;

	if (handlers != NULL) {
		event_add_message_handler(message_RAM_FETCH, EVENT_MESSAGE_INCOMING, dataxfer_message_ram_fetch);
		event_add_message_handler(message_RAM_TRANSMIT, EVENT_MESSAGE_INCOMING, dataxfer_message_ram_transmit);
		event_add_message_handler(message_RAM_FETCH, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_ram_fetch_bounced);
		event_add_message_handler(message_RAM_TRANSMIT, EVENT_MESSAGE_ACKNOWLEDGE, dataxfer_message_bounced);
	}

	dataxfer_task_handle = task_handle;
}


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

void dataxfer_work_area_drag(wimp_w w, wimp_pointer *pointer, os_box *extent, char *sprite, void (* drag_end_callback)(wimp_pointer *pointer, void *data), void *drag_end_data)
{
	wimp_window_state	window;
	wimp_drag		drag;
	int			ox, oy, width, height;

	/* If there's no callback, there's no point bothering. */

	if (drag_end_callback == NULL)
		return;

	/* Get the basic information about the window and icon. */

	window.w = w;
	wimp_get_window_state(&window);

	ox = window.visible.x0 - window.xscroll;
	oy = window.visible.y1 - window.yscroll;

	/* Read CMOS RAM to see if solid drags are required; this can only
	 * happen if a sprite name is supplied.
	 */

	dataxfer_dragging_sprite = ((sprite != NULL) && ((osbyte2(osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
			osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0)) ? TRUE : FALSE;

	/* Set up the drag parameters. */

	drag.w = window.w;
	drag.type = wimp_DRAG_USER_FIXED;

	/* If the drag is a sprite, it is centred on the pointer; if a drag box,
	 * the supplied box extent is used relative to the window work area.
	 */

	if (dataxfer_dragging_sprite) {
		if (xwimpspriteop_read_sprite_size(sprite, &width, &height, NULL, NULL) != NULL) {
			width = 32;
			height = 32;
		}

		// \TODO -- This assumes a square pixel mode!

		drag.initial.x0 = pointer->pos.x - width;
		drag.initial.y0 = pointer->pos.y - height;
		drag.initial.x1 = pointer->pos.x + width;
		drag.initial.y1 = pointer->pos.y + height;
	} else {
		drag.initial.x0 = ox + extent->x0;
		drag.initial.y0 = oy + extent->y0;
		drag.initial.x1 = ox + extent->x1;
		drag.initial.y1 = oy + extent->y1;
	}

	drag.bbox.x0 = 0x80000000;
	drag.bbox.y0 = 0x80000000;
	drag.bbox.x1 = 0x7fffffff;
	drag.bbox.y1 = 0x7fffffff;

	dataxfer_drag_end_callback = drag_end_callback;

	/* Start the drag and set an eventlib callback. */

	if (dataxfer_dragging_sprite)
		dragasprite_start(dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE |
			dragasprite_NO_BOUND | dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW,
			wimpspriteop_AREA, sprite, &(drag.initial), &(drag.bbox));
	else
		wimp_drag_box(&drag);

	event_set_drag_handler(dataxfer_terminate_user_drag, NULL, drag_end_data);
}


/**
 * Start dragging an icon from a dialogue, creating a sprite to drag and starting
 * a drag action.  When the action completes, a callback will be made to the
 * supplied function.
 *
 * \param w		The window where the drag is starting.
 * \param i		The icon to be dragged.
 * \param callback	A callback function
 */

void dataxfer_save_window_drag(wimp_w w, wimp_i i, void (* drag_end_callback)(wimp_pointer *pointer, void *data), void *drag_end_data)
{
	wimp_window_state	window;
	wimp_icon_state		icon;
	wimp_drag		drag;
	int			ox, oy;


	/* If there's no callback, there's no point bothering. */

	if (drag_end_callback == NULL)
		return;

	/* Get the basic information about the window and icon. */

	window.w = w;
	wimp_get_window_state(&window);

	ox = window.visible.x0 - window.xscroll;
	oy = window.visible.y1 - window.yscroll;

	icon.w = window.w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	/* Set up the drag parameters. */

	drag.w = window.w;
	drag.type = wimp_DRAG_USER_FIXED;

	drag.initial.x0 = ox + icon.icon.extent.x0;
	drag.initial.y0 = oy + icon.icon.extent.y0;
	drag.initial.x1 = ox + icon.icon.extent.x1;
	drag.initial.y1 = oy + icon.icon.extent.y1;

	drag.bbox.x0 = 0x80000000;
	drag.bbox.y0 = 0x80000000;
	drag.bbox.x1 = 0x7fffffff;
	drag.bbox.y1 = 0x7fffffff;

	/* Read CMOS RAM to see if solid drags are required. */

	dataxfer_dragging_sprite = ((osbyte2(osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
			osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0) ? TRUE : FALSE;

	dataxfer_drag_end_callback = drag_end_callback;

	/* Start the drag and set an eventlib callback. */

	if (dataxfer_dragging_sprite)
		dragasprite_start(dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE |
			dragasprite_NO_BOUND | dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW,
			wimpspriteop_AREA, icon.icon.data.indirected_text.text, &(drag.initial), &(drag.bbox));
	else
		wimp_drag_box(&drag);

	event_set_drag_handler(dataxfer_terminate_user_drag, NULL, drag_end_data);
}


/**
 * Callback handler for queue window drag termination.
 *
 * Start a data-save dialogue with the application at the other end.
 *
 * \param  *drag		The Wimp poll block from termination.
 * \param  *data		NULL (unused).
 */

static void dataxfer_terminate_user_drag(wimp_dragged *drag, void *data)
{
	wimp_pointer		pointer;

	if (dataxfer_dragging_sprite)
		dragasprite_stop();

	wimp_get_pointer_info(&pointer);

	if (dataxfer_drag_end_callback != NULL)
		dataxfer_drag_end_callback(&pointer, data);

	dataxfer_drag_end_callback = NULL;
}


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

osbool dataxfer_request_clipboard(wimp_w w, wimp_i i, os_coord pos, bits types[], osbool (*receive_callback)(void *content, size_t size, bits type, void *data), void *data)
{
	struct dataxfer_descriptor	*descriptor;
	wimp_full_message_data_request	datarequest;
	os_error			*error;
	int				j;


	if (receive_callback == NULL)
		return FALSE;

	/* Allocate a block to store details of the message. */

	descriptor = dataxfer_new_descriptor();
	if (descriptor == NULL)
		return FALSE;

	descriptor->purpose = DATAXFER_CLIPBOARD_RECEIVE;
	
	descriptor->save_callback = NULL;
	descriptor->receive_callback = receive_callback;
	descriptor->callback_data = data;

	/* Set up and send the datasave message. If it fails, give an error
	 * and delete the message details as we won't need them again.
	 */

	datarequest.size = 48;
	datarequest.your_ref = 0;
	datarequest.action = message_DATA_REQUEST;

	datarequest.w = w;
	datarequest.i = i;
	datarequest.pos.x = pos.x;
	datarequest.pos.y = pos.y;
	datarequest.flags = wimp_DATA_REQUEST_CLIPBOARD;
	for (j = 0; types[j] != -1; j++)
		datarequest.file_types[j] = types[j];
	datarequest.file_types[j] = -1;

	error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &datarequest, wimp_BROADCAST);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return FALSE;
	}

	/* Complete the message descriptor information. */

	descriptor->type = DATAXFER_MESSAGE_REQUEST;
	descriptor->my_ref = datarequest.my_ref;

	return TRUE;
}


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

osbool dataxfer_start_save(wimp_pointer *pointer, char *name, int size, bits type, int your_ref, osbool (*save_callback)(char *filename, void *data), void *data)
{
	struct dataxfer_descriptor	*descriptor;
	wimp_full_message_data_xfer	message;
	os_error			*error;


	if (save_callback == NULL)
		return FALSE;

	/* Allocate a block to store details of the message. */

	descriptor = dataxfer_new_descriptor();
	if (descriptor == NULL)
		return FALSE;

	descriptor->purpose = DATAXFER_FILE_SAVE;

	descriptor->save_callback = save_callback;
	descriptor->receive_callback = NULL;
	descriptor->callback_data = data;

	/* Set up and send the datasave message. If it fails, give an error
	 * and delete the message details as we won't need them again.
	 */

	message.size = WORDALIGN(45 + strlen(name));
	message.your_ref = your_ref;
	message.action = message_DATA_SAVE;
	message.w = pointer->w;
	message.i = pointer->i;
	message.pos = pointer->pos;
	message.est_size = size;
	message.file_type = type;

	string_copy(message.file_name, name, 212);

	error = xwimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &message, pointer->w, pointer->i, &(descriptor->task));
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return FALSE;
	}

	/* Complete the message descriptor information. */

	descriptor->type = DATAXFER_MESSAGE_SAVE;
	descriptor->my_ref = message.my_ref;

	return TRUE;
}


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

osbool dataxfer_start_load(wimp_pointer *pointer, char *name, int size, bits type, int your_ref)
{
	struct dataxfer_descriptor	*descriptor;
	wimp_full_message_data_xfer	message;
	os_error			*error;


	/* Allocate a block to store details of the message. */

	descriptor = dataxfer_new_descriptor();
	if (descriptor == NULL)
		return FALSE;

	descriptor->purpose = DATAXFER_FILE_LOAD;

	descriptor->save_callback = NULL;
	descriptor->receive_callback = NULL;
	descriptor->callback_data = NULL;

	/* Set up and send the datasave message. If it fails, give an error
	 * and delete the message details as we won't need them again.
	 */

	message.size = WORDALIGN(45 + strlen(name));
	message.your_ref = your_ref;
	message.action = message_DATA_LOAD;
	message.w = pointer->w;
	message.i = pointer->i;
	message.pos = pointer->pos;
	message.est_size = size;
	message.file_type = type;

	string_copy(message.file_name, name, 212);

	error = xwimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &message, pointer->w, pointer->i, &(descriptor->task));
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	/* Complete the message descriptor information. */

	descriptor->type = DATAXFER_MESSAGE_SAVE;
	descriptor->my_ref = message.my_ref;

	return TRUE;
}


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


void dataxfer_register_clipboard_provider(size_t callback(bits types[], bits *type, void **data))
{
	dataxfer_find_clipboard_content = callback;
}


/**
 * Handle the receipt of a Message_DataRequest, by finding out if the client
 * owns the clipboard and starting a transfer if it does.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to claim the message; FALSE to pass it on.
 */

static osbool dataxfer_message_datarequest(wimp_message *message)
{
	struct dataxfer_descriptor	*descriptor;
	wimp_full_message_data_request	*requestblock = (wimp_full_message_data_request *) message;
	wimp_full_message_data_xfer	*xferblock = (wimp_full_message_data_xfer *) message;
	void				*clipboard_data = NULL;
	size_t				clipboard_size = 0;
	bits				clipboard_type = -1;
	os_error			*error;

	if (dataxfer_find_clipboard_content != NULL)
		clipboard_size = dataxfer_find_clipboard_content(requestblock->file_types, &clipboard_type, &clipboard_data);

	/* Just return if the client does not own the clipboard at present. */

	if (clipboard_data == NULL || clipboard_size == 0 || clipboard_type == -1)
		return FALSE;

	/* Check that the message flags are correct. */

	if ((requestblock->flags & wimp_DATA_REQUEST_CLIPBOARD) == 0)
		return FALSE;

	/* Allocate a block to store details of the message. */

	descriptor = dataxfer_new_descriptor();
	if (descriptor == NULL)
		return FALSE;

	descriptor->purpose = DATAXFER_CLIPBOARD_SEND;

	descriptor->save_callback = NULL;
	descriptor->receive_callback = NULL;
	descriptor->callback_data = NULL;

	descriptor->ram_data = clipboard_data;
	descriptor->ram_size = clipboard_size;
	descriptor->ram_allocation = 0;
	descriptor->ram_used = 0;

	/* Set up and send the datasave message. If it fails, give an error
	 * and delete the message details as we won't need them again.
	 */

	xferblock->size = WORDALIGN(45 + strlen(DATAXFER_CLIPBOARD_NAME));
	xferblock->your_ref = requestblock->my_ref;
	xferblock->action = message_DATA_SAVE;
	xferblock->est_size = clipboard_size;
	xferblock->file_type = clipboard_type;

	string_copy(xferblock->file_name, DATAXFER_CLIPBOARD_NAME, 212);

	error = xwimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED, (wimp_message *) xferblock, xferblock->w, xferblock->i, &(descriptor->task));
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	/* Complete the message descriptor information. */

	descriptor->type = DATAXFER_MESSAGE_SAVE;
	descriptor->my_ref = xferblock->my_ref;

	return TRUE;
}


/**
 * Handle the receipt of a Message_RAMFetch to either kick off or continue a
 * RAM transfer of data out to another application.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_ram_fetch(wimp_message *message)
{
	wimp_full_message_ram_xfer	*ramfetch = (wimp_full_message_ram_xfer *) message;
	struct dataxfer_descriptor	*descriptor;
	os_error			*error;
	int				bytes_to_send, send_this_time;
	wimp_event_no			message_type;


	/* See if we recognise the message, and if we're sending the clipboard.
	 * If either are false, let the message go.
	 */

	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_SAVE | DATAXFER_MESSAGE_RAMRX);
	if (descriptor == NULL || descriptor->purpose != DATAXFER_CLIPBOARD_SEND)
		return FALSE;

	/* See how many bytes are left to go, and how many we can send in this
	 * transfer -- then send the data and update the sent byte count.
	 */

	bytes_to_send = descriptor->ram_size - descriptor->ram_used;
	send_this_time = (bytes_to_send > ramfetch->xfer_size) ? ramfetch->xfer_size : bytes_to_send;

	error = xwimp_transfer_block(dataxfer_task_handle, (byte *) descriptor->ram_data + descriptor->ram_used,
			ramfetch->sender, ramfetch->addr, send_this_time);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	descriptor->ram_used += send_this_time;

	/* Update the message block and send the reply. If there's still data
	 * to go, it must be sent Recorded.
	 */

	if (send_this_time < ramfetch->xfer_size)
		message_type = wimp_USER_MESSAGE;
	else
		message_type = wimp_USER_MESSAGE_RECORDED;

	ramfetch->your_ref = ramfetch->my_ref;
	ramfetch->action = message_RAM_TRANSMIT;
	ramfetch->xfer_size = send_this_time;

	error = xwimp_send_message(message_type, (wimp_message *) ramfetch, ramfetch->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	/* Complete the message descriptor information. */

	descriptor->type = DATAXFER_MESSAGE_RAMRX;
	descriptor->my_ref = ramfetch->my_ref;

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataSaveAck in response our starting a
 * save action.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_data_save_ack(wimp_message *message)
{
	struct dataxfer_descriptor	*descriptor;
	wimp_full_message_data_xfer	*datasaveack = (wimp_full_message_data_xfer *) message;
	os_error			*error;


	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_SAVE);
	if (descriptor == NULL)
		return FALSE;

	/* We know the message in question, so try and do something with it. This
	 * will depend on the purpose of the message.
	 */

	switch (descriptor->purpose) {
	case DATAXFER_FILE_SAVE:
		/* If the client's supplied a callback, call it.  If it returns FALSE
		 * then we don't want to send a Message_DataLoad so free the information
		 * block and quit marking the incoming message as handled.
		 */

		if (descriptor->save_callback != NULL &&
				!descriptor->save_callback(datasaveack->file_name, descriptor->callback_data)) {
			dataxfer_delete_descriptor(descriptor);
			return TRUE;
		}
		break;

	case DATAXFER_CLIPBOARD_SEND:
		/* We're transferring some clipboard data, so save the data to the supplied
		 * file and return.
		 */

		error = xosfile_save_stamped(datasaveack->file_name, datasaveack->file_type, descriptor->ram_data, descriptor->ram_data + descriptor->ram_size);
		if (error != NULL) {
			dataxfer_delete_descriptor(descriptor);
			return TRUE;
		}
		break;

	default:
		break;
	}

	/* The client saved something, so finish off the data transfer. */

	datasaveack->your_ref = datasaveack->my_ref;
	datasaveack->action = message_DATA_LOAD;

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) datasaveack, datasaveack->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);

		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	descriptor->my_ref = datasaveack->my_ref;

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataLoadAck in response our starting a
 * save action.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_data_load_ack(wimp_message *message)
{
	struct dataxfer_descriptor	*descriptor;

	/* This is the tail end of a data save, so just clean up and claim the
	 * message.
	 */

	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_SAVE);
	if (descriptor == NULL)
		return FALSE;

	dataxfer_delete_descriptor(descriptor);
	return TRUE;
}


/**
 * Handle the receipt of a Message_DataSave due to another application trying
 * to transfer data in to us.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_data_save(wimp_message *message)
{
	wimp_full_message_data_xfer	*datasave = (wimp_full_message_data_xfer *) message;
	wimp_full_message_ram_xfer	ramfetch;
	struct dataxfer_descriptor	*descriptor;
	os_error			*error;
	struct dataxfer_incoming_target	*target;
	osbool				data_unsafe = TRUE;


	/* We don't want to respond to our own save requests. */

	if (message->sender == dataxfer_task_handle)
		return FALSE;

	/* Check to see if this message is a reply to one of our
	 * Message_DataRequests.
	 */

	if (message->your_ref != 0) {
		/* See if this is a reply to a message we think we've sent. */
	
		descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_REQUEST);
		if (descriptor == NULL || descriptor->purpose != DATAXFER_CLIPBOARD_RECEIVE)
			return FALSE;

		if (dataxfer_memory_handlers != NULL) {
			descriptor->ram_allocation = datasave->est_size + 1;
			descriptor->ram_data = dataxfer_memory_handlers->alloc(descriptor->ram_allocation);
			descriptor->saved_message = malloc(sizeof(wimp_full_message_data_xfer));

			/* Both the above blocks are freed when the descriptor is deleted, so while
			 * this is a short-term memory leak it isn't fatal.
			 */

			if (descriptor->ram_data != NULL && descriptor->saved_message != NULL) {
				descriptor->ram_size = descriptor->ram_allocation;

				memcpy(descriptor->saved_message, message, sizeof(wimp_full_message_data_xfer));

				ramfetch.size = 28;
				ramfetch.your_ref = datasave->my_ref;
				ramfetch.action = message_RAM_FETCH;

				ramfetch.addr = (byte *) descriptor->ram_data;
				ramfetch.xfer_size = descriptor->ram_size;

				error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &ramfetch, datasave->sender);
				if (error != NULL) {
					error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
					dataxfer_delete_descriptor(descriptor);
					return TRUE;
				}

				descriptor->type = DATAXFER_MESSAGE_RAMRX;
				descriptor->my_ref = ramfetch.my_ref;

				return TRUE;
			}
		}
	} else {
		/* See if the window is one of the registered targets. */

		target = dataxfer_find_incoming_target(DATAXFER_TARGET_SAVE, datasave->w, datasave->i, datasave->file_type);
		if (target == NULL || target->callback == NULL)
			return FALSE;

		/* If we've got a target, get a descriptor to track the message exchange. */

		descriptor = dataxfer_new_descriptor();
		if (descriptor == NULL)
			return FALSE;

		descriptor->purpose = DATAXFER_FILE_LOAD;

		descriptor->save_callback = NULL;
		descriptor->receive_callback = NULL;
		descriptor->callback_data = target;
		if (target->intermediate_filename != NULL) {
			descriptor->intermediate_filename = target->intermediate_filename;
			/* If an intermediate file has been supplied, assume that the client
			 * will be ensuring that the data is maintained as 'safe'.
			 */
			data_unsafe = FALSE;
		}

		/* Update the message block and send an acknowledgement. */
	}

	datasave->your_ref = datasave->my_ref;
	datasave->action = message_DATA_SAVE_ACK;
	if (data_unsafe)
		datasave->est_size = -1;
	string_copy(datasave->file_name, descriptor->intermediate_filename, 212);
	datasave->size = WORDALIGN(45 + strlen(datasave->file_name));

	error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) datasave, datasave->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);

		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	descriptor->type = DATAXFER_MESSAGE_LOAD;
	descriptor->my_ref = datasave->my_ref;

	return TRUE;
}


/**
 * Handle bounces of Message_RAMFetch, which tells us that the application
 * trying to send us data can't handle RAM transfers.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_ram_fetch_bounced(wimp_message *message)
{
	struct dataxfer_descriptor	*descriptor;
	os_error			*error;


	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_RAMRX);
	if (descriptor == NULL)
		return FALSE;

	// \TODO -- Bounces of the non-first message should be handled differently.

	/* Free any memory that's claimed for the transfer. */

	if (descriptor->ram_data != NULL) {
		if (dataxfer_memory_handlers != NULL)
			dataxfer_memory_handlers->free(descriptor->ram_data);
		descriptor->ram_data = NULL;
		descriptor->ram_size = 0;
		descriptor->ram_allocation = 0;
	}

	/* Send a Message_DataSaveAck to start a disc-based transfer. */

	descriptor->saved_message->your_ref = descriptor->saved_message->my_ref;
	descriptor->saved_message->action = message_DATA_SAVE_ACK;
	descriptor->saved_message->est_size = -1;
	string_copy(descriptor->saved_message->file_name, descriptor->intermediate_filename, 212);
	descriptor->saved_message->size = WORDALIGN(45 + strlen(descriptor->saved_message->file_name));

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) descriptor->saved_message, descriptor->saved_message->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);

		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	descriptor->type = DATAXFER_MESSAGE_LOAD;
	descriptor->my_ref = descriptor->saved_message->my_ref;

	/* We don't need the saved message any more. */

	free(descriptor->saved_message);
	descriptor->saved_message = NULL;

	return TRUE;
}

/**
 * Handle the receipt of a Message_RAMTransmit due to ongoing RAM transfer
 * of data in to us.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_ram_transmit(wimp_message *message)
{
	wimp_full_message_ram_xfer	*ramtransmit = (wimp_full_message_ram_xfer *) message;
	struct dataxfer_descriptor	*descriptor;
	os_error			*error;
	byte				*block;


	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_RAMRX);
	if (descriptor == NULL || descriptor->purpose != DATAXFER_CLIPBOARD_RECEIVE)
		return FALSE;

	if (ramtransmit->xfer_size == descriptor->ram_allocation) {
		block = dataxfer_memory_handlers->realloc(descriptor->ram_data, descriptor->ram_size + descriptor->ram_allocation);
		if (block == NULL) {
			error_msgs_report_error("NoRAMforXFer:No RAM for data transfer.");
			dataxfer_delete_descriptor(descriptor);
			return TRUE;
		}
		descriptor->ram_data = block;
		descriptor->ram_used = descriptor->ram_size;
		descriptor->ram_size += descriptor->ram_allocation;

		ramtransmit->your_ref = ramtransmit->my_ref;
		ramtransmit->action = message_RAM_FETCH;
		ramtransmit->addr = descriptor->ram_data + descriptor->ram_used;

		error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) ramtransmit, ramtransmit->sender);
		if (error != NULL) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			dataxfer_delete_descriptor(descriptor);
			return TRUE;
		}

		descriptor->my_ref = ramtransmit->my_ref;
	} else {
		/* That's it; so return the data to the client. */

		if (descriptor->receive_callback != NULL)
			descriptor->receive_callback(descriptor->ram_data, descriptor->ram_used + ramtransmit->xfer_size, 0xfff, descriptor->callback_data);

		/* We don't want the descriptor deletion to free the data block,
		 * as the client might want to keep it.
		 */

		descriptor->ram_data = NULL;

		dataxfer_delete_descriptor(descriptor);
	}

	return TRUE;
}

/**
 * Handle the receipt of a Message_DataLoad due to a filer load or an ongoing
 * data transfer process.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_data_load(wimp_message *message)
{
	wimp_full_message_data_xfer	*dataload = (wimp_full_message_data_xfer *) message;
	struct dataxfer_descriptor	*descriptor = NULL;
	os_error			*error;
	struct dataxfer_incoming_target	*target = NULL;


	/* We don't want to respond to our own save requests. */

	if (message->sender == dataxfer_task_handle)
		return TRUE;

	/* See if we know about this transfer already.  If we do, this is the
	 * closing stage of a full transfer; if not, we must ask the client
	 * if we need to proceed.
	 */

	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_LOAD);
	if (descriptor == NULL) {
		/* See if the window is one of the registered targets. */

		target = dataxfer_find_incoming_target(DATAXFER_TARGET_LOAD, dataload->w, dataload->i, dataload->file_type);

		if (target == NULL || target->callback == NULL)
			return FALSE;
	} else if (descriptor->purpose == DATAXFER_CLIPBOARD_RECEIVE && descriptor->receive_callback != NULL) {
		/* This is the end of a clipboard data request, so we need to
		 * load the file contents and present it to the client as a
		 * block of memory.
		 */

		fileswitch_object_type	type;
		int			size;
		byte			*data;

		error = xosfile_read_no_path(dataload->file_name, &type, NULL, NULL, &size, NULL);
		if (error != NULL)
			return FALSE;

		data = dataxfer_memory_handlers->alloc(size * sizeof(byte));
		if (data == NULL)
			return FALSE;

		error = xosfile_load_stamped_no_path(dataload->file_name, data, NULL, NULL, NULL, NULL, NULL);
		if (error == NULL)
			descriptor->receive_callback(data, size, dataload->file_type, descriptor->callback_data);

		xosfscontrol_wipe(dataload->file_name, NONE, 0, 0, 0, 0);
	} else {
		/* This is someone saving data to us. */

		target = descriptor->callback_data;
	}

	/* If this wasn't a clipboard transfer, we just pass the filename to the
	 * client and let them load it.
	 */

	if (target != NULL && (descriptor == NULL || (descriptor != NULL && descriptor->purpose == DATAXFER_FILE_LOAD))) {
		/* If there's no load callback function, abandon the transfer here. */

		if (target->callback == NULL)
			return TRUE;

		/* If the load failed, abandon the transfer here. */

		if (target->callback(dataload->w, dataload->i, dataload->file_type, dataload->file_name, target->callback_data) == FALSE)
			return TRUE;

		/* If this was an inter-application transfer, tidy up. */

		if (descriptor != NULL) {
			xosfscontrol_wipe(dataload->file_name, NONE, 0, 0, 0, 0);
			dataxfer_delete_descriptor(descriptor);
		}
	}

	/* Update the message block and send an acknowledgement. */

	dataload->your_ref = dataload->my_ref;
	dataload->action = message_DATA_LOAD_ACK;

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) dataload, dataload->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return TRUE;
	}

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataOpen due to a double-click in the Filer.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_data_open(wimp_message *message)
{
	wimp_full_message_data_xfer	*dataopen = (wimp_full_message_data_xfer *) message;
	os_error			*error;
	struct dataxfer_incoming_target	*target;


	target = dataxfer_find_incoming_target(DATAXFER_TARGET_OPEN, NULL, -1, dataopen->file_type);

	if (target == NULL)
		return FALSE;

	/* If there's no load callback function, abandon the transfer here. */

	if (target->callback == NULL)
		return TRUE;

	/* Update the message block and send an acknowledgement. Do this before
	 * calling the load callback, to meet the requirements of the PRM.
	 */

	dataopen->your_ref = dataopen->my_ref;
	dataopen->action = message_DATA_LOAD_ACK;

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) dataopen, dataopen->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return TRUE;
	}

	/* Call the load callback. */

	target->callback(NULL, -1, dataopen->file_type, dataopen->file_name, target->callback_data);

	return TRUE;
}


/**
 * Handle the bounce of a Message during a load or save operation.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool dataxfer_message_bounced(wimp_message *message)
{
	struct dataxfer_descriptor	*descriptor;

	/* The message has bounced, so just clean up. */

	descriptor = dataxfer_find_descriptor(message->your_ref, DATAXFER_MESSAGE_ALL);
	if (descriptor != NULL) {
		if (message->action == message_DATA_LOAD) {
			wimp_full_message_data_xfer *dataload = (wimp_full_message_data_xfer *) message;

			xosfscontrol_wipe(dataload->file_name, NONE, 0, 0, 0, 0);
			error_msgs_report_error("XferFail:Data transfer failed.");
		}

		dataxfer_delete_descriptor(descriptor);
		return TRUE;
	}

	return FALSE;
}


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

osbool dataxfer_set_drop_target(unsigned filetype, wimp_w w, wimp_i i, char *intermediate, osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data)
{
	return dataxfer_set_load_target(DATAXFER_TARGET_DRAG, filetype, w, i, intermediate, callback, data);
}


/**
 * Specify a handler for files which are double-clicked. Files which match
 * on type, are passed to the appropriate handler for attention.
 *
 * \param filetype		The filetype to register as a target.
 * \param *callback		The load callback function.
 * \param *data			Data to be passed to load functions, or NULL.
 * \return			TRUE if successfully registered; else FALSE.
 */

osbool dataxfer_set_load_type(unsigned filetype, osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data)
{
	return dataxfer_set_load_target(DATAXFER_TARGET_OPEN, filetype, NULL, -1, NULL, callback, data);
}


/**
 * Specify a handler for files which are double-clicked or dragged into a window.
 * Files which match on type, target window and target icon are passed to the
 * appropriate handler for attention.
 *
 * To specify a generic handler for a type, set window to NULL and icon to -1.
 * To specify a generic handler for all the icons in a window, set icon to -1.
 *
 * Double-clicked files (Message_DataOpen) will be passed to a generic type
 * handler or a type handler for a window with the handle wimp_ICON_BAR.
 *
 * \param target		The target type(s) which are to be used.
 * \param filetype		The filetype to register as a target.
 * \param w			The target window, or NULL.
 * \param i			The target icon, or -1.
 * \param *intermediate		Pointer to the intermediate filename to use, or NULL for default.
 * \param *callback		The load callback function.
 * \param *data			Data to be passed to load functions, or NULL.
 * \return			TRUE if successfully registered; else FALSE.
 */

static osbool dataxfer_set_load_target(enum dataxfer_target_type target, unsigned filetype, wimp_w w, wimp_i i, char *intermediate,
		osbool (*callback)(wimp_w w, wimp_i i, unsigned filetype, char *filename, void *data), void *data)
{
	struct dataxfer_incoming_target		*type, *window, *icon;

	/* Validate the input: if there's an icon, there must be a window. */

	if (w == NULL && i != -1)
		return FALSE;

	/* Set up the top-level filetype target. */

	type = dataxfer_incoming_targets;

	while (type != NULL && (type->filetype != filetype || (type->target & target) != target))
		type = type->next;

	if (type == NULL) {
		type = malloc(sizeof(struct dataxfer_incoming_target));
		if (type == NULL)
			return FALSE;

		type->target = target;
		type->filetype = filetype;
		type->window = 0;
		type->icon = 0;

		type->callback = NULL;
		type->callback_data = NULL;

		type->intermediate_filename = (intermediate != NULL) ? strdup(intermediate) : NULL;

		type->children = NULL;

		type->next = dataxfer_incoming_targets;
		dataxfer_incoming_targets = type;
	}

	if (w == NULL) {
		type->callback = callback;
		type->callback_data = data;
		return TRUE;
	}

	/* Set up the window target. */

	window = type->children;

	while (window != NULL && window->window != w)
		window = window->next;

	if (window == NULL) {
		window = malloc(sizeof(struct dataxfer_incoming_target));
		if (window == NULL)
			return FALSE;

		window->target = target;
		window->filetype = filetype;
		window->window = w;
		window->icon = 0;

		window->callback = NULL;
		window->callback_data = NULL;

		window->intermediate_filename = (intermediate != NULL) ? strdup(intermediate) : NULL;

		window->children = NULL;

		window->next = type->children;
		type->children = window;
	}

	if (i == -1) {
		window->callback = callback;
		window->callback_data = data;
		return TRUE;
	}

	/* Set up the icon target. */

	icon = window->children;

	while (icon != NULL && icon->icon != i)
		icon = icon->next;

	if (icon == NULL) {
		icon = malloc(sizeof(struct dataxfer_incoming_target));
		if (icon == NULL)
			return FALSE;

		icon->target = target;
		icon->filetype = filetype;
		icon->window = w;
		icon->icon = i;

		icon->callback = NULL;
		icon->callback_data = NULL;

		icon->intermediate_filename = (intermediate != NULL) ? strdup(intermediate) : NULL;

		icon->children = NULL;

		icon->next = window->children;
		window->children = icon;
	}

	icon->callback = callback;
	icon->callback_data = data;

	return TRUE;
}


/**
 * Remove a handler for files which are dragged into a window.
 *
 * \param filetype		The filetype to delete as a target.
 * \param w			The target window to delete, or NULL.
 * \param i			The target icon to delete, or -1.
 */

void dataxfer_delete_drop_target(unsigned filetype, wimp_w w, wimp_i i)
{
	dataxfer_delete_load_target(DATAXFER_TARGET_DRAG, filetype, w, i);
}


/**
 * Remove a handler for files which are double-clicked.
 *
 * \param filetype		The filetype to delete as a target.
 */

void dataxfer_delete_load_type(unsigned filetype)
{
	dataxfer_delete_load_target(DATAXFER_TARGET_OPEN, filetype, NULL, -1);
}


/**
 * Remove a handler for files which are double-clicked or dragged into a window.
 *
 * To specify all of the handlers for a given window (and icon), set filetype to -1.
 * To specify the generic handler for a type, set window to NULL and icon to -1.
 * To specify the generic handler for all the icons in a window, set icon to -1.
 *
 * \param target		The targets type(s) which are to be deleted.
 * \param filetype		The filetype to delete from the target list.
 * \param w			The target window to delete from the target list, or NULL.
 * \param i			The target icon to delete from the target list, or -1.
 */

static void dataxfer_delete_load_target(enum dataxfer_target_type target, unsigned filetype, wimp_w w, wimp_i i)
{
	struct dataxfer_incoming_target		*delete, *type, *window, *icon, *parent_type, *parent_window, *parent_icon;

	/* Validate the input: if there's an icon, there must be a window. */

	if (w == NULL && i != -1)
		return;

	/* Locate the top-level filetype targets. */

	type = dataxfer_incoming_targets;
	parent_type = NULL;
	delete = NULL;

	while (type != NULL) {
		if ((filetype == -1 || type->filetype == filetype) && ((type->target & target) != DATAXFER_TARGET_NONE)) {
			/* If the global type matches, clear it. */

			if (w == NULL && i == -1) {
				type->callback = NULL;
				type->callback_data = NULL;
			}

			window = type->children;
			parent_window = NULL;

			while (window != NULL) {
				if ((filetype == -1 || window->filetype == filetype) && (w == NULL || window->window == w) && ((window->target & target) != DATAXFER_TARGET_NONE)) {
					/* If the global window matches, clear it. */

					if (i == -1) {
						window->callback = NULL;
						window->callback_data = NULL;
					}

					icon = window->children;
					parent_icon = NULL;

					while (icon != NULL) {
						if ((filetype == -1 || icon->filetype == filetype) && (w == NULL || icon->window == w) && (i == -1 || icon->icon == i) &&
								((icon->target & target) != DATAXFER_TARGET_NONE)) {
							icon->callback = NULL;
							icon->callback_data = NULL;
						}

						if (icon->callback == NULL && icon->children == NULL) {
							if (parent_icon == NULL)
								window->children = icon->next;
							else
								parent_icon->next = icon->next;
							delete = icon;
							icon = icon->next;
							if (delete->intermediate_filename != NULL)
								free(delete->intermediate_filename);
							free(delete);
						} else {
							parent_icon = icon;
							icon = icon->next;
						}
					}
				}

				if (window->callback == NULL && window->children == NULL) {
					if (parent_window == NULL)
						type->children = window->next;
					else
						parent_window->next = window->next;
					delete = window;
					window = window->next;
					if (delete->intermediate_filename != NULL)
						free(delete->intermediate_filename);
					free(delete);
				} else {
					parent_window = window;
					window = window->next;
				}
			}
		}

		if (type->callback == NULL && type->children == NULL) {
			if (parent_type == NULL)
				dataxfer_incoming_targets = type->next;
			else
				parent_type->next = type->next;
			delete = type;
			type = type->next;
			if (delete->intermediate_filename != NULL)
				free(delete->intermediate_filename);
			free(delete);
		} else {
			parent_type = type;
			type = type->next;
		}
	}
}


/**
 * Find an incoming transfer target based on the filetype and its target
 * window and icon handle.
 *
 * \param target		The target type(s) to match.
 * \param w			The window into which the file was dropped.
 * \param i			The icon onto which the file was dropped.
 * \param filetype		The filetype of the incoming file.
 * \return			The appropriate target, or NULL if none found.
 */

static struct dataxfer_incoming_target *dataxfer_find_incoming_target(enum dataxfer_target_type target, wimp_w w, wimp_i i, unsigned filetype)
{
	struct dataxfer_incoming_target		*type, *window, *icon;

	/* Search for a filetype. */

	type = dataxfer_incoming_targets;

	while (type != NULL && (type->filetype != filetype || (type->target & target) == DATAXFER_TARGET_NONE))
		type = type->next;

	if (type == NULL)
		return NULL;

	/* Now search for a window. */

	window = type->children;

	while (w != NULL && window != NULL && (window->window != w || (window->target & target) == DATAXFER_TARGET_NONE))
		window = window->next;

	if (window == NULL)
		return type;

	/* Now search for an icon. */

	icon = window->children;

	while (i != -1 && icon != NULL && (icon->icon != i || (icon->target & target) == DATAXFER_TARGET_NONE))
		icon = icon->next;

	if (icon == NULL)
		return window;

	return icon;
}


/**
 * Create a new message descriptor with no data and return a pointer.
 *
 * \return			The new block, or NULL on failure.
 */

static struct dataxfer_descriptor *dataxfer_new_descriptor(void)
{
	struct dataxfer_descriptor		*new;

	new = malloc(sizeof(struct dataxfer_descriptor));
	if (new != NULL) {
		new->type = DATAXFER_MESSAGE_NONE;
		new->purpose = DATAXFER_UNKNOWN;

		new->intermediate_filename = "<Wimp$Scrap>";

		new->ram_data = NULL;
		new->ram_allocation = 0;
		new->ram_size = 0;
		new->ram_used = 0;

		new->saved_message = NULL;

		new->next = dataxfer_descriptors;
		dataxfer_descriptors = new;
	}

	return new;
}


/**
 * Find a record for a message, based on type and reference.
 *
 * \param ref			The message reference field to match.
 * \param type			The message type(s) to match.
 * \return			The message descriptor, or NULL if not found.
 */

static struct dataxfer_descriptor *dataxfer_find_descriptor(int ref, enum dataxfer_message_type type)
{
	struct dataxfer_descriptor		*list = dataxfer_descriptors;

	while (list != NULL && ((list->type & type) == 0 || list->my_ref != ref))
		list = list->next;

	return list;
}


/**
 * Delete a message descriptor.
 *
 * \param *message		The message descriptor to be deleted.
 */

static void dataxfer_delete_descriptor(struct dataxfer_descriptor *message)
{
	struct dataxfer_descriptor		*list = dataxfer_descriptors;

	if (message == NULL)
		return;

	/* If there's a saved message block, free it. */

	if (message->saved_message != NULL)
		free(message->saved_message);

	/* If there's any RAM transfer memory, free it. */

	if (message->ram_data != NULL && dataxfer_memory_handlers != NULL)
		dataxfer_memory_handlers->free(message->ram_data);

	/* If the message is at the head of the list, delink and free it. */

	if (dataxfer_descriptors == message) {
		dataxfer_descriptors = message->next;
		free(message);
		return;
	}

	/* Otherwise, find the message in the list, delink and free it. */

	while (list != NULL && list->next != message)
		list = list->next;

	if (list != NULL)
		list->next = message->next;

	free(message);
}

