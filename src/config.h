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
 * \file: config.h
 *
 * RISC OS Application Configuration.  Support for storing named integer,
 * string and boolean parameters: each having a default value and the ability
 * to be updated and read as required.  Changed values can be saved to and
 * loaded from a textual choices file in the standard locations.
 */

#ifndef SFLIB_CONFIG
#define SFLIB_CONFIG

#include <stdio.h>

/* ================================================================================================================== */

#define sf_MAX_CONFIG_NAME 32							/**< The maximum length of a config value name.		*/
#define sf_MAX_CONFIG_STR  1024							/**< The maximum length of a textual config value.	*/

enum config_read_status {
	sf_CONFIG_READ_EOF = 0,							/**< Nothing returned; End Of File.			*/
	sf_CONFIG_READ_VALUE_RETURNED = 1,					/**< Value returned.					*/
	sf_CONFIG_READ_NEW_SECTION = 2						/**< Start of section; section and value returned.	*/
};

/**
 * Initialise the config module for the given application.
 *
 * \param *app_name		The name of the application.
 * \param *c_dir		The name of the application's Choices: folder.
 * \param *l_dir		The application's directory path.
 * \return			TRUE if the system was initialised OK; else FALSE.
 */

osbool config_initialise(char *app_name, char *c_dir, char *l_dir);


/**
 * Load the currently saved configuration into memory, overriding any settings
 * currently stored in memory.
 *
 * \return		TRUE if successful; else FALSE.
 */

osbool config_load(void);


/**
 * Save the current configuration from memory into the applicable Choices
 * file, recording only those values which differ from the defaults.
 *
 * \return		TRUE if successful; else FALSE.
 */

osbool config_save(void);


/**
 * Restore the default configuration settings.
 *
 * ** Currently Not Implemented **
 *
 * \return			TRUE if successful; else FALSE.
 */

int config_restore_default(void);


/**
 * Create and initialise a boolean config value.
 *
 * \param *name		The name of the config value.
 * \param value		The initial value to assign
 * \return		TRUE if successful; else FALSE.
 */

int config_opt_init(char *name, osbool value);


/**
 * Set a boolean config value.
 *
 * \param *name		The name of the config value to set.
 * \param value		The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_opt_set(char *name, osbool value);


/**
 * Read a boolean config value.
 *
 * \param *name		The name of the config value to read.
 * \return		The value, or FALSE if not found.
 */

osbool config_opt_read(char *name);


/**
 * Create and initialise an integer config value.
 *
 * \param *name		The name of the config value.
 * \param value		The initial value to assign
 * \return		TRUE if successful; else FALSE.
 */

osbool config_int_init(char *name, int value);


/**
 * Set an integer config value.
 *
 * \param *name		The name of the config value to set.
 * \param value		The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_int_set(char *name, int value);


/**
 * Read an integer config value.
 *
 * \param *name		The name of the config value to read.
 * \return		Pointer the value, or 0 if not found.
 */

int config_int_read(char *name);


/**
 * Create and initialise a string config value.
 *
 * \param *name
 * \param *value
 * \return		TRUE if successful; else FALSE.
 */

osbool config_str_init(char *name, char *value);


/**
 * Set a string config value.
 *
 * \param *name		The name of the config value to set.
 * \param *value	The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_str_set(char *name, char *value);


/**
 * Read a string config value.
 *
 * \param *name		The name of the config value to read.
 * \return		Pointer to the value, or to "" if not found.
 */

char *config_str_read(char *name);


/**
 * Process lines from a file, until a valid token/value pair is found or EOF is
 * reached.  Return values for the token and value in *token and *value; if a new
 * section is encountered, return that in *section.
 *
 * \param *file		The file to read from.
 * \param *token	Pointer to buffer to hold a token name.
 * \param *value	Pointer to buffer to hold a token value.
 * \param *section	Pointer to buffer to hold a section name.
 * \return		Result code.
 */

enum config_read_status config_read_token_pair(FILE *file, char *token, char *value, char *section);


/**
 * Write a token/value pair to file, enclosing parameters that contain
 * leading or trailing whitespace in quotes.
 *
 * \param *file			The file handle to write to.
 * \param *token		Pointer to a string containing the token.
 * \param *value		Pointer to a string containing a representation of the value.
 * \return			The number of bytes written, or negative for an error.
 */

int config_write_token_pair(FILE *file, char *token, char *value);


/**
 * Return a pointer to a string representing the supplied boolean value.
 *
 * \param opt		The boolean value to return a string for.
 * \return		Pointer to a string representing the value ("Yes" or "No").
 */

char *config_return_opt_string(osbool opt);


/**
 * Test a string and return TRUE or FALSE depending on whether it is "Yes"
 * or "No".
 *
 * \param *str		The string to be tested.
 * \return		TRUE if the string is "Yes" or "True"; else FALSE.
 */

osbool config_read_opt_string(char *str);


/**
 * Get a filename for the file to load the choices settings from.  The global
 * Choices: paths are tried first; fall back to the application folder.
 *
 * \param *file			A buffer to hold a full pathname.
 * \param len			The size of the buffer.
 * \param *leaf			The leaf file name to use.
 */

void config_find_load_file(char *file, size_t len, char *leaf);


/**
 * Get a filename for the file to save the choices settings to.  <Choices$Write>
 * is tried first; if this fails the application folder is used.
 *
 * \param *file			A buffer to hold a full pathname.
 * \param len			The size of the buffer.
 * \param *leaf			The leaf file name to use.
 */

void config_find_save_file(char *file, size_t len, char *leaf);

#endif

