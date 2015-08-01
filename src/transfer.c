/* Copyright 2003-2015, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: transfer.c
 *
 * Structured support for the RISC OS Data Transfer Protocol.
 */

/* Acorn C Header files. */

#include "flex.h"

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/os.h"
#include "oslib/osfile.h"

/* SF-Lib header files. */

#include "transfer.h"
#include "general.h"
#include "string.h"
#include "errors.h"

/* ANSII C header files. */

#include <string.h>


#define SAVED_FILENAME_LENGTH 256						/**< The maximum length of a full pathname. */

static char				filename[SAVED_FILENAME_LENGTH];
static char				**data;

static int				(*external_tx_function) (char *);
static int				data_length;
static int				data_type;
static int				bytes_xfered;
static int				buffer_offered;
static int				ram_xfer_started;
static int				delete_after;

static wimp_full_message_data_xfer	saved_message;


/* Initialise a Data Save process.
 *
 * Copy w, i, pos and ref from a Message_DataRequest for clipboard transfers.
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_save_start_block(wimp_w w, wimp_i i, os_coord pos, int ref, char **block,
		int length, int type, char *name)
{
	wimp_full_message_data_xfer	datasave;
	os_error			*error;

	/* Copy data across. */

	data = block;
	(external_tx_function) = NULL;
	data_length = length;
	strncpy(filename, name, SAVED_FILENAME_LENGTH);
	data_type = type;
	bytes_xfered = 0;

	/* Set up and send Message_DataSave. */

	datasave.size = WORDALIGN(45 + strlen (name));
	datasave.your_ref = ref;
	datasave.action = message_DATA_SAVE;

	datasave.w = w;
	datasave.i = i;
	datasave.pos = pos;
	datasave.est_size = length;
	datasave.file_type = type;
	strcpy(datasave.file_name, name);

	error = xwimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &datasave, w, i, NULL);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Start a datasave process using a function to save the data once the pathname
 * has been found.
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_save_start_callback(wimp_w w, wimp_i i, os_coord pos, int ref, int (*save_function) (char *filename),
		int length, int type, char *name)
{
	wimp_full_message_data_xfer	datasave;
	os_error			*error;

	/* Copy data across. */

	data = NULL;
	external_tx_function = save_function;
	data_length = length;
	strncpy(filename, name, SAVED_FILENAME_LENGTH);
	data_type = type;
	bytes_xfered = 0;

	/* Set up and send Message_DataSave. */

	datasave.size = WORDALIGN(45 + strlen (name));
	datasave.your_ref = ref;
	datasave.action = message_DATA_SAVE;

	datasave.w = w;
	datasave.i = i;
	datasave.pos = pos;
	datasave.est_size = length;
	datasave.file_type = type;
	strcpy(datasave.file_name, name);

	error = xwimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &datasave, w, i, NULL);
	if (error != NULL) {
		error_report_os_error (error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Reply to a Message_DataSaveAck, received following a call to
 * transfer_save_start_block() or transfer_save_start_callback().
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_save_reply_datasaveack(wimp_message *message)
{
	wimp_full_message_data_xfer	*datasaveack = (wimp_full_message_data_xfer *) message;
	os_error			*error;

	if (data != NULL) {
		error = xosfile_save_stamped(datasaveack->file_name, data_type, (byte const *) *data, (byte const *) *data+data_length);
		if (error != NULL) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			return -1;
		}
	} else {
		if (external_tx_function(datasaveack->file_name))
			return -1;
	}

	datasaveack->your_ref = datasaveack->my_ref;
	datasaveack->action = message_DATA_LOAD;

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) datasaveack, datasaveack->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Reply to a Message_RamFetch, received following a call to send_start_data_save().
 * transfer_save_start_block() or transfer_save_start_callback().
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_save_reply_ramfetch(wimp_message *message, wimp_t task_handle)
{
	wimp_full_message_ram_xfer	*ramfetch = (wimp_full_message_ram_xfer *) message;
	os_error			*error;
	int				bytes_to_send, bytes_sent_this_time;
	wimp_event_no			message_type;


	/* If we're not using memory blocks, the transfer will have to be via
	 * file and not RAM Transfer.
	 */

	if (data == NULL)
		return 0;

	bytes_to_send = data_length - bytes_xfered;
	bytes_sent_this_time = (bytes_to_send > ramfetch->xfer_size) ? ramfetch->xfer_size : bytes_to_send;

	error = xwimp_transfer_block(task_handle, (byte *) *data+bytes_xfered,
			ramfetch->sender, ramfetch->addr, bytes_sent_this_time);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	bytes_xfered += bytes_sent_this_time;

	if (bytes_sent_this_time < ramfetch->xfer_size)
		message_type = wimp_USER_MESSAGE; /* \TODO -- Some means of freeing data blocks is required here... */
	else
		message_type = wimp_USER_MESSAGE_RECORDED;

	ramfetch->your_ref = ramfetch->my_ref;
	ramfetch->action = message_RAM_TRANSMIT;
	ramfetch->xfer_size = bytes_sent_this_time;

	error = xwimp_send_message(message_type, (wimp_message *) ramfetch, ramfetch->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Reply to a Message_DataSave initiating a file load, supplying a pointer to
 * a flex block to take whatever data is offered.
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_load_reply_datasave_block(wimp_message *message, char **data_ptr)
{
	wimp_full_message_data_xfer	*datasave = (wimp_full_message_data_xfer *) message;
	wimp_full_message_ram_xfer	ramfetch;
	os_error			*error;

	/* Initialise global variables. */

	data_length = datasave->est_size;
	string_ctrl_strncpy(filename, datasave->file_name, SAVED_FILENAME_LENGTH - 1);
	data_type = datasave->file_type;
	bytes_xfered = 0;
	buffer_offered = 5; //datasave->est_size + 1; /* +1 to allow a RAM xfer to complete in one go if possible. \TODO -- What on earth??!!?? */
	data = data_ptr;
	external_tx_function = NULL;
	delete_after = 1;

	memcpy(&saved_message, datasave, sizeof(saved_message));

	if (flex_alloc((flex_ptr) data, buffer_offered) == 0) {
		error_msgs_report_error("NoRAMforXFer");
		return -1;
	}

	/* Reply to the message to get a RAM transfer going. */

	ramfetch.size = 28;
	ramfetch.your_ref = datasave->my_ref;
	ramfetch.action = message_RAM_FETCH;

	ramfetch.addr = (byte *) *data;
	ramfetch.xfer_size = buffer_offered;

	ram_xfer_started = 0;

	/* flex_budge_setting = flex_budge (0); */

	error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) &ramfetch, datasave->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		flex_free((flex_ptr) data);
		return -1;
	}

	return 0;
}


/* Reply to a Message_DataSave initiating a file load, supplying a callback
 * function to be supplied with the filename.
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_load_reply_datasave_callback(wimp_message *message, int (*load_function) (char *filename))
{
	wimp_full_message_data_xfer	*datasave = (wimp_full_message_data_xfer *) message;
	os_error			*error;

	/* Initialise global variables. */

	data_length = datasave->est_size;
	string_ctrl_strncpy(filename, datasave->file_name, SAVED_FILENAME_LENGTH - 1);
	data_type = datasave->file_type;
	bytes_xfered = 0;
	buffer_offered = 5; //datasave->est_size + 1; /* +1 to allow a RAM xfer to complete in one go if possible. \TODO -- What on earth??!!?? */
	data = NULL;
	external_tx_function = load_function;
	delete_after = 1;

	/* Reply to the message. */

	datasave->your_ref = datasave->my_ref;
	datasave->action = message_DATA_SAVE_ACK;

	strcpy(datasave->file_name, "<Wimp$Scrap>\r");
	datasave->est_size = -1;

	datasave->size = WORDALIGN(45 + strlen(datasave->file_name));

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) datasave, datasave->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Reply to a Message_RAMTransmit, received following a call to
 * transfer_load_reply_datasave_block() or transfer_load_reply_datasave_callback().
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_load_reply_ramtransmit(wimp_message *message, char *name)
{
	wimp_full_message_ram_xfer	*ramtransmit = (wimp_full_message_ram_xfer *) message;
	os_error			*error;

	if (ramtransmit->xfer_size == buffer_offered) {
		if (flex_extend((flex_ptr) data, flex_size((flex_ptr) data) + buffer_offered) == 0) {
			error_msgs_report_error("NoRAMforXFer");
			flex_free((flex_ptr) data);
			return -1;
		}

		ramtransmit->your_ref = ramtransmit->my_ref;
		ramtransmit->action = message_RAM_FETCH;
		ramtransmit->addr += buffer_offered;

		ram_xfer_started = 1;

		error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) ramtransmit, ramtransmit->sender);
		if (error != NULL) {
			error_report_os_error (error, wimp_ERROR_BOX_CANCEL_ICON);
			flex_free((flex_ptr) data);
			return -1;
		}

		return 0;
	} else {
		flex_extend((flex_ptr) data, flex_size((flex_ptr) data) - (buffer_offered - ramtransmit->xfer_size));

		if (name != NULL)
			strcpy(name, filename);

		return flex_size((flex_ptr) data);
	}
}


/* Reply to a bounced Message_RAMFetch, returned following a call to
 * transfer_load_reply_datasave_block() or transfer_load_reply_datasave_callback().
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_load_bounced_ramfetch(wimp_message *message)
{
	os_error			*error;

	/* If subsequent messages bounce, they should be ignored. */

	if (ram_xfer_started)
		return 0;

	/* Free the memory claimed for the RAM transfer. */

	flex_free((flex_ptr) data);

	/* Create and send a Message_DataSaveAck. */

	saved_message.your_ref = saved_message.my_ref;
	saved_message.action = message_DATA_SAVE_ACK;

	strcpy(saved_message.file_name, "<Wimp$Scrap>\r");
	saved_message.est_size = -1;

	saved_message.size = WORDALIGN(45 + strlen(saved_message.file_name));

	error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &saved_message, saved_message.sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	return 0;
}


/* Initialise the data load process following the receipt of a direct
 * Message_DataLoad (such as a drag direct from the Filer).
 *
 * This is an external interface, documented in transfer.h
 */

void transfer_load_start_direct_callback(wimp_message *message, int (*load_function) (char *filename))
{
	wimp_full_message_data_xfer	*dataload = (wimp_full_message_data_xfer *) message;

	/* Initialise global variables. */

	data_length = dataload->est_size;
	string_ctrl_strncpy(filename, dataload->file_name, SAVED_FILENAME_LENGTH - 1);
	data_type = dataload->file_type;
	bytes_xfered = 0;
	buffer_offered = 5; //datasave->est_size + 1; /* +1 to allow a RAM xfer to complete in one go if possible. */
	data = NULL;
	external_tx_function = load_function;
	delete_after = 0;
}


/* Reply to a Message_DataLoad, received following a call to
 * transfer_load_reply_datasave_block(), transfer_load_reply_datasave_callback(),
 * or transfer_load_start_direct_callback.
 *
 * This is an external interface, documented in transfer.h
 */

int transfer_load_reply_dataload(wimp_message *message, char *name)
{
	wimp_full_message_data_xfer	*dataload = (wimp_full_message_data_xfer *) message;
	os_error			*error;
	int				size;
	bits				type;


	/* Reply with a Message_DataLoadAck. */

	dataload->your_ref = dataload->my_ref;
	dataload->action = message_DATA_LOAD_ACK;

	error = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, (wimp_message *) dataload, dataload->sender);
	if (error != NULL) {
		error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
		return -1;
	}

	/* Start to load the file. */

	if (data != NULL) {
		error = xosfile_read_stamped_no_path(dataload->file_name, NULL, NULL, NULL, &size, NULL, &type);
		if (error != NULL) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			return -1;
		}

		if (type != dataload->file_type)
			return -1;

		if (flex_alloc((flex_ptr) data, size) == 0) {
			error_msgs_report_error("NoRAMforXFer");
			return -1;
		}

		error = xosfile_load_stamped_no_path(dataload->file_name, (byte *) *data, NULL, NULL, NULL, NULL, NULL);
		if (error != NULL) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			flex_free((flex_ptr) data);
			return -1;
		}
	} else {
		external_tx_function(dataload->file_name);
		size = 0;
	}

	if (delete_after) {
		error = xosfile_delete(dataload->file_name, NULL, NULL, NULL, NULL, NULL);
		if (error != NULL) {
			error_report_os_error(error, wimp_ERROR_BOX_CANCEL_ICON);
			flex_free((flex_ptr) data);
			return -1;
		}
	}

	if (name != NULL)
		strcpy(name, filename);

	return size;
}

