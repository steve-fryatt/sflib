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
 * \file: transfer.h
 *
 * Structured support for the RISC OS Data Transfer Protocol.
 */

#ifndef SFLIB_TRANSFER
#define SFLIB_TRANSFER


/* Requires the following MessageTrans tokens to be defined in the application's Messages file.
 *
 * NoRAMforXFer: Shown if a data-load process fails due to lack of RAM.
 */


/**
 * Initialise a Data Save process.
 *
 * Copy w, i, pos and ref from a Message_DataRequest for clipboard transfers.
 *
 * \param w		The destination window handle.
 * \param i		The destination icon handle.
 * \param pos		The destination pointer position. \TODO -- Not a pointer ?!?
 * \param ref		MyRef from Message_DataRequest, or zero.
 * \param **data	Pointer to data pointer, to allow for flex blocks.
 * \param length	Length of data to be transferred.
 * \param type		Filetype of data.
 * \param *name		Pointer to proposed leafname for data.
 * \return		0 if the transfer is started OK; -1 for an error.
 */

int transfer_save_start_block(wimp_w w, wimp_i i, os_coord pos, int ref, char **data,
		int length, int type, char *name);


/**
 * Start a datasave process using a function to save the data once the pathname
 * has been found.
 *
 * \param w		The destination window handle.
 * \param i		The destination icon handle.
 * \param pos		The destination pointer position. \TODO -- Not a pointer ?!?
 * \param ref		MyRef from Message_DataRequest, or zero.
 * \param save_function	Callback function to handle the datasave process.
 * \param length	Length of data to be transferred.
 * \param type		Filetype of data.
 * \param *name		Pointer to proposed leafname for data.
 * \return		0 if the transfer is started OK; -1 for an error.
 */

int transfer_save_start_callback(wimp_w w, wimp_i i, os_coord pos, int ref, int (*save_function) (char *filename),
		int length, int type, char *name);


/**
 * Reply to a Message_DataSaveAck, received following a call to send_start_data_save().
 *
 * \param *message		The DataSave_Ack message block.
 * \return			0 if successful; -1 if an error occurred.
 */

int transfer_save_reply_datasaveack(wimp_message *message);


/**
 * Reply to a Message_RamFetch, received following a call to send_start_data_save().
 *
 * \param *message		The Message_RAMFetch data block.
 * \param task_handle		The task handle of the owning task.
 * \return			0 on successful conculsion; -1 for an error.
 */

int transfer_save_reply_ramfetch(wimp_message *message, wimp_t task_handle);


/**
 * Reply to a Message_DataSave initiating a file load, supplying a pointer to
 * a flex block to take whatever data is offered.
 *
 * \param *message		The Message_DataSave block.
 * \param **data_ptr		Pointer to an unused flex block handle, to
 *				be updated with details of a block.
 * \return			0 on success; -1 if an error occurred.
 */

int transfer_load_reply_datasave_block(wimp_message *message, char **data_ptr);


/**
 * Reply to a Message_DataSave initiating a file load, supplying a callback
 * function to be supplied with the filename.
 *
 * \param *message		The Message_DataSave block.
 * \param *load_function	Callback function to be called with the filename.
 * \return			0 on success; -1 if an error occurred.
 */

int transfer_load_reply_datasave_callback(wimp_message *message, int (*load_function) (char *filename));


/**
 * Reply to a Message_RAMTransmit, received following a call to
 * transfer_load_reply_datasave_block() or transfer_load_reply_datasave_callback().
 *
 * \param *message		The Message_RAMFetch data block.
 * \param name			Pointer to a buffer to take the leafname of the
 *				file -- no buffer length checks are made.
 * \return			0 if transfer is still in progress; >0 indicates
 *				completion and size of data; -1 indicates error.
 */

int transfer_load_reply_ramtransmit(wimp_message *message, char *name);


/**
 * Reply to a bounced Message_RAMFetch, returned following a call to
 * transfer_load_reply_datasave_block() or transfer_load_reply_datasave_callback().
 *
 * \param *message		The returned MessageRAMFetch message block.
 * \return			0 if successful; -1 if an error occurred.
 */

int transfer_load_bounced_ramfetch(wimp_message *message);


/**
 * Initialise the data load process following the receipt of a direct
 * Message_DataLoad (such as a drag direct from the Filer).
 *
 * \param *message		The Message_DataLoad block.
 * \param *load_function	Callback function to be called with the filename.
 */

void transfer_load_start_direct_callback(wimp_message *message, int (*load_function) (char *filename));


/**
 * Reply to a Message_DataLoad, received following a call to
 * transfer_load_reply_datasave_block(), transfer_load_reply_datasave_callback(),
 * or transfer_load_start_direct_callback.
 *
 * \param *message		The Message_RAMFetch data block.
 * \param name			Pointer to a buffer to take the leafname of the
 *				file -- no buffer length checks are made.
 * \return			0 if transfer is still in progress; >0 indicates
 *				completion and size of data; -1 indicates error.
 */

int transfer_load_reply_dataload(wimp_message *message, char *name);

#endif

