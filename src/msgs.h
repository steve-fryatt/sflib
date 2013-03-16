/* Copyright 2003-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: msgs.h
 *
 * RISC OS Message File support.  Open and close Message files, and look
 * up tokens.
 */

#ifndef SFLIB_MSGS
#define SFLIB_MSGS

#include <stdlib.h>
#include "oslib/messagetrans.h"


/**
 * Iniitialise the Msgs module, loading the specified file and preparing the
 * system to handle message lookups.
 *
 * \param *messages_file	The file to open.
 * \return			TRUE if successful; else FALSE.
 */

osbool msgs_initialise(char *messages_file);


/**
 * Initialise the Msgs module using an already prepared MessageTrans
 * Control Block.
 *
 * \param *block		The MessageTrans Control Block to use.
 * \return			TRUE if successful; else FALSE.
 */

osbool msgs_initialise_external(messagetrans_control_block *block);


/**
 * Terminate the Msgs module, closing any Messages file that was opened
 * via msgs_initialise().
 *
 * \return			TRUE if successful; else FALSE.
 */

osbool msgs_terminate(void);


/**
 * Look up a message token without parameters, storing the result in the
 * supplied buffer.
 *
 * \param *token		The message token to look up.
 * \param *buffer		The buffer to hold the result.
 * \param buffer_size		The size of the result buffer.
 * \return			A pointer to the result.
 */

char *msgs_lookup(char *token, char *buffer, size_t buffer_size);


/**
 * Look up a message token, substituting the supplied parameters and storing
 * the result in the supplied buffer.
 *
 * \param *token		The message token to look up.
 * \param *buffer		The buffer to hold the result.
 * \param buffer_size		The size of the result buffer.
 * \param *a			Parameter for %0.
 * \param *b			Parameter for %1.
 * \param *c			Parameter for %2.
 * \param *d			Parameter for %3.
 * \return			A pointer to the result.
 */

char *msgs_param_lookup(char *token, char *buffer, size_t buffer_size, char *a, char *b, char *c, char *d);


/**
 * Look up a message token without parameters, storing the result in the
 * supplied buffer.
 *
 * \param *token		The message token to look up.
 * \param *buffer		The buffer to hold the result.
 * \param buffer_size		The size of the result buffer.
 * \return			TRUE if the token was found; else FALSE.
 */

osbool msgs_lookup_result(char *token, char *buffer, size_t buffer_size);


/**
 * Look up a message token, substituting the supplied parameters and storing
 * the result in the supplied buffer.
 *
 * \param *token		The message token to look up.
 * \param *buffer		The buffer to hold the result.
 * \param buffer_size		The size of the result buffer.
 * \param *a			Parameter for %0.
 * \param *b			Parameter for %1.
 * \param *c			Parameter for %2.
 * \param *d			Parameter for %3.
 * \return			TRUE if the token was found; else FALSE.
 */

osbool msgs_param_lookup_result(char *token, char *buffer, size_t buffer_size, char *a, char *b, char *c, char *d);

#endif

