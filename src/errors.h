/* Copyright 2003-2012, Stephen Fryatt
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
 * \file: errors.h
 *
 * Wimp error handling and message box support.
 */

#ifndef SFLIB_ERRORS
#define SFLIB_ERRORS

#include "oslib/messagetrans.h"
#include "oslib/wimp.h"


/**
 * Initialise the error message module.
 *
 * \param *name			MessageTrans token for the application name.
 * \param *sprite		MessageTrans token for the application sprite.
 * \param *closedown		Callback handler (unused; set to NULL).
 */

void error_initialise(char *name, char *sprite, void (*closedown)(void));


/**
 * Display a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containing
 * details of the error as contained in an Error Block.
 *
 * \param *error		An Error Block defining the error and the
 *				message to display.
 * \param buttons		The buttons to include in the error message.
 * \return			The selection made.
 */

wimp_error_box_selection error_report_os_error(os_error *error, wimp_error_box_flags buttons);

/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_INFO, containg the
 * message looked up via the given MessageTrans token and an OK button.
 *
 * \param *token		The MessageTrans token for the message.
 * \return			The selection made.
 */

wimp_error_box_selection error_msgs_report_info(char *token);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_INFO, containg the
 * given message and an OK button.
 *
 * \param *message		The text of the message.
 * \return			The selection made.
 */

wimp_error_box_selection error_report_info(char *message);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containg the
 * message looked up via the given MessageTrans token and an OK button.
 *
 * \param *token		The MessageTrans token for the message.
 * \return			The selection made.
 */

wimp_error_box_selection error_msgs_report_error(char *token);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_ERROR, containg the
 * given message and an OK button.
 *
 * \param *message		The text of the message.
 * \return			The selection made.
 */

wimp_error_box_selection error_report_error(char *message);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_QUESTION, containg the
 * message looked up via the given MessageTrans token and either OK and
 * Cancel buttons or buttons as specified in the comma-separated list
 * contained in the buttons token.
 *
 * \param *token		The MessageTrans token for the message.
 * \param *buttons		The MessageTrans token for the buttons, or NULL
 *				to use OK and Cancel.
 * \return			The selection made.
 */

wimp_error_box_selection error_msgs_report_question(char *token, char *buttons);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_QUESTION, containg the
 * given message and either OK and Cancel buttons or buttons as specified in
 * the comma-separated list.
 *
 * \param *message		The text of the message.
 * \param *buttons		A comma-separated list of buttons, or NULL
 *				to use OK and Cancel.
 * \return			The selection made.
 */

wimp_error_box_selection error_report_question(char *message, char *buttons);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg the
 * message looked up via the given MessageTrans token and a Cancel button.
 *
 * This function never returns.
 *
 * \param *token		The MessageTrans token for the message.
 */

void error_msgs_report_fatal(char *token);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg the
 * given message and a Cancel button.
 *
 * This function never returns.
 *
 * \param *message		The text of the message.
 */

void error_report_fatal(char *message);


/**
 * Open a Wimp error box of type wimp_ERROR_BOX_CATEGORY_PROGRAM, containg
 * details of the error in an Error Block and a Cancel button.
 *
 * This function never returns.
 *
 * \param *error		An Error Block defining the error and the
 *				message to display.
 */

void error_report_program(os_error *error);

#endif

