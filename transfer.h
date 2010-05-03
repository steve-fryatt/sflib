/* SF-Lib - Transfer.h
 *
 * Version 0.20 (20 December 2003)
 */

#ifndef _SFLIB_TRANSFER
#define _SFLIB_TRANSFER

/* ================================================================================================================== */

/* Requires the following MessageTrans tokens to be defined in the application's Messages file.
 *
 * NoRAMforXFer: Shown if a data-load process fails due to lack of RAM.
 */

/* ==================================================================================================================
 * Saving data
 */

int send_start_data_save_block (wimp_w w, wimp_i i, os_coord pos, int ref, char **data,
                                int length, int type, char *filename);

/* Start a datasave process.
 *
 * w, i, pos - Window, icon and pointer position of destination: copy from Message_DataRequest for clipboard xfers.
 * ref       - MyRef from Message_DataRequest, or zero.
 * **data    - Pointer to data pointer, to allow for flex blocks.
 * length    - Length of data to be transferred.
 * type      - Filetype of data.
 * *filename - Pointer to proposed leafname for data.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int send_start_data_save_function (wimp_w w, wimp_i i, os_coord pos, int ref, int (*save_function) (char *filename),
                                   int length, int type, char *filename);

/* Start a datasave process using a function to save the data once the pathname has been found.
 *
 * w, i, pos      - Window, icon and pointer position of destination: copy from Message_DataRequest for clipboard xfers.
 * ref            - MyRef from Message_DataRequest, or zero.
 * save_function()- Function to manage the data saving.
 * length         - Length of data to be transferred.
 * type           - Filetype of data.
 * *filename      - Pointer to proposed leafname for data.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int send_reply_data_save_ack (wimp_message *data_save_ack);

/* Reply to a Message_DataSaveAck, following a call to send_start_data_save ().
 *
 * *data_save_ack - wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int send_reply_ram_fetch (wimp_message *ram_fetch, wimp_t task_handle);

/* Reply to a Message_RamFetch, following a call to send_start_data_save ().
 *
 * *ram_fetch  - wimp message block.
 * task_handle - task handle of calling task, for use by Wimp_TransferBlock.
 *
 * ------------------------------------------------------------------------------------------------------------------ */

/* ==================================================================================================================
 * Loading data
 */

int receive_reply_data_save_block (wimp_message *data_save, char **data);

/* Reply to a Message_DataSave.
 *
 * *data_save - wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int receive_reply_data_save_function (wimp_message *data_save, int (*load_function) (char *filename));

/* Reply to a Message_DataSave.
 *
 * *data_save - wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int recieve_reply_ram_transmit (wimp_message *ram_transmit, char *name);

/* Reply to a Message_RAMTransmit.
 *
 * *ram_transmit - the wimp message block.
 * *name         - pointer to string to take leafname, or NULL.
 *
 * returns int result  (0  = transfer in progress;
 *                      -1 = error occurred;
 *                      >0 = size of recieved data)
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int receive_bounced_ram_fetch (wimp_message *ram_fetch);

/* Deal with a bounce from Message_RAMFetch.
 *
 * *ram_fetch - the wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


void receive_init_quick_data_load_function (wimp_message *data_load, int (*load_function) (char *filename));

/* Initilaise process ready to call receive_reply_data_load() if a transfer is started with a Message_DataLoad
 * (eg. from the Filer, direct).
 *
 * *data_load - the wimp message block.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


int receive_reply_data_load (wimp_message *data_load, char *name);

/* Reply to a Message_DataLoad.
 *
 * *data_load - the wimp message block.
 * *name      - pointer to string to take leafname, or NULL.
 *
 * returns int result  (0  = transfer in progress;
 *                      -1 = error occurred;
 *                      >0 = size of recieved data)
 *
 * ------------------------------------------------------------------------------------------------------------------ */

#endif
