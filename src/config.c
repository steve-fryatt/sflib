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
 * \file: config.c
 *
 * RISC OS Application Configuration.  Support for storing named integer,
 * string and boolean parameters: each having a default value and the ability
 * to be updated and read as required.  Changed values can be saved to and
 * loaded from a textual choices file in the standard locations.
 */

/* Acorn C Header files. */


/* OS-Lib header files. */

#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/fileswitch.h"

/* ANSII C header files. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* SF-Lib header files. */

#include "config.h"
#include "string.h"


/**
 * Structure for storing boolean config settings.
 */

typedef struct config_opt {
	char			name[sf_MAX_CONFIG_NAME];			/**< The name of the config value.				*/
	osbool			value;						/**< The current value.						*/
	osbool			initial;					/**< The initial, or default, value.				*/

	struct config_opt	*next;						/**< Pointer to the next boolean config value, or NULL.		*/
} config_opt;

/**
 * Structure for storing integer config settings.
 */

typedef struct config_int {
	char			name[sf_MAX_CONFIG_NAME];			/**< The name of the config value.				*/
	int			value;						/**< The current value.						*/
	int			initial;					/**< The initial, or default, value.				*/

	struct config_int	*next;						/**< Pointer to the next integer config value, or NULL.		*/
} config_int;


/**
 * Structure for storing text config settings.
 */

typedef struct config_str {
	char			name[sf_MAX_CONFIG_NAME];			/**< The name of the config value.				*/
	char			value[sf_MAX_CONFIG_STR];			/**< The current value.						*/
	char			initial[sf_MAX_CONFIG_STR];			/**< The initial, or default, value.				*/

	struct config_str	*next;						/**< Pointer to the next text config value, or NULL.		*/
} config_str;


/* Global variables. */

static config_opt		*opt_list = NULL;				/**< The chain of boolean config values.			*/
static config_int		*int_list = NULL;				/**< The chain of integer config values.			*/
static config_str		*str_list = NULL;				/**< The chain of textual config values.			*/

static char			*choices_dir = NULL;				/**< The name of the application's folder in Choices:.		*/
static char			*local_dir = NULL;				/**< The full path to the application's own folder.		*/
static char			*application_name = NULL;			/**< The application name as registered with the Wimp.		*/



/**
 * Initialise the config module for the given application.
 *
 * \param *app_name		The name of the application.
 * \param *c_dir		The name of the application's Choices: folder.
 * \param *l_dir		The application's directory path.
 * \return			TRUE if the system was initialised OK; else FALSE.
 */

osbool config_initialise(char *app_name, char *c_dir, char *l_dir)
{
	application_name = (char *) malloc (strlen (app_name) + 1);
	if (application_name == NULL)
		return FALSE;

	strcpy (application_name, app_name);

	local_dir = (char *) malloc (strlen (l_dir) + 1);
	if (local_dir == NULL)
		return FALSE;

	strcpy (local_dir, l_dir);

	choices_dir = (char *) malloc (strlen (c_dir) + 1);
	if (choices_dir == NULL)
		return FALSE;

	strcpy (choices_dir, c_dir);

	return TRUE;
}


/**
 * Find an opt-config block based on its name.
 *
 * \param *name		The name of the block to find.
 * \return		Pointer to the block, or NULL if not found.
 */

static config_opt *config_find_opt(char *name)
{
	config_opt	*block = opt_list;


	while (block != NULL && (strcmp(block->name, name) != 0))
		block = block->next;

	return block;
}


/**
 * Create and initialise a boolean config value.
 *
 * \param *name		The name of the config value.
 * \param value		The initial value to assign
 * \return		TRUE if successful; else FALSE.
 */

int config_opt_init(char *name, osbool value)
{
	config_opt	*new;

	if ((new = (config_opt *) malloc(sizeof (config_opt))) == NULL)
		return FALSE;

	strcpy(new->name, name);
	new->initial = value;
	new->value = value;

	new->next = opt_list;
	opt_list = new;

	return TRUE;
}


/**
 * Set a boolean config value.
 *
 * \param *name		The name of the config value to set.
 * \param value		The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_opt_set(char *name, osbool value)
{
	config_opt	*option;

	if ((option = config_find_opt(name)) == NULL)
		return FALSE;

	option->value = value;

	return TRUE;
}


/**
 * Read a boolean config value.
 *
 * \param *name		The name of the config value to read.
 * \return		The value, or FALSE if not found.
 */

osbool config_opt_read(char *name)
{
	config_opt	*option;

	if ((option = config_find_opt(name)) == NULL)
		return FALSE;

	return option->value;
}


/**
 * Find an int-config block based on its name.
 *
 * \param *name		The name of the block to find.
 * \return		Pointer to the block, or NULL if not found.
 */

static config_int *config_find_int(char *name)
{
	config_int	*block = int_list;


	while (block != NULL && (strcmp(block->name, name) != 0))
		block = block->next;

	return block;
}


/**
 * Create and initialise an integer config value.
 *
 * \param *name		The name of the config value.
 * \param value		The initial value to assign
 * \return		TRUE if successful; else FALSE.
 */

osbool config_int_init(char *name, int value)
{
	config_int	*new;

	if ((new = (config_int *) malloc(sizeof (config_int))) == NULL)
		return FALSE;

	strcpy(new->name, name);
	new->initial = value;
	new->value = value;

	new->next = int_list;
	int_list = new;

	return TRUE;
}


/**
 * Set an integer config value.
 *
 * \param *name		The name of the config value to set.
 * \param value		The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_int_set(char *name, int value)
{
	config_int	*option;

	if ((option = config_find_int(name)) == NULL)
		return FALSE;

	option->value = value;

	return TRUE;
}


/**
 * Read an integer config value.
 *
 * \param *name		The name of the config value to read.
 * \return		The value, or 0 if not found.
 */

int config_int_read(char *name)
{
	config_int	*option;

	if ((option = config_find_int(name)) == NULL)
		return 0;

	return option->value;
}


/**
 * Find an str-config block based on its name.
 *
 * \param *name		The name of the block to find.
 * \return		Pointer to the block, or NULL if not found.
 */

static config_str *config_find_str(char *name)
{
	config_str	*block = str_list;


	while (block != NULL && (strcmp(block->name, name) != 0))
		block = block->next;

	return block;
}


/**
 * Create and initialise a string config value.
 *
 * \param *name		The name of the config value.
 * \param *value	The initial value to assign
 * \return		TRUE if successful; else FALSE.
 */

osbool config_str_init(char *name, char *value)
{
	config_str	*new;

	if ((new = (config_str *) malloc(sizeof (config_str))) == NULL)
		return FALSE;

	strcpy(new->name, name);
	strcpy(new->initial, value);
	strcpy(new->value, value);

	new->next = str_list;
	str_list = new;

	return TRUE;
}


/**
 * Set a string config value.
 *
 * \param *name		The name of the config value to set.
 * \param *value	The new value to assign.
 * \return		TRUE if successful; else FALSE.
 */

osbool config_str_set(char *name, char *value)
{
	config_str *option;

	if ((option = config_find_str(name)) == NULL)
		return FALSE;

	strcpy(option->value, value);

	return TRUE;
}


/**
 * Read a string config value.
 *
 * \param *name		The name of the config value to read.
 * \return		Pointer to the value, or to "" if not found.
 */

char *config_str_read(char *name)
{
	config_str *option;


	if ((option = config_find_str(name)) == NULL)
		return "";

	return option->value;
}


/**
 * Get a filename for the file to load the choices settings from.  The global
 * Choices: paths are tried first; fall back to the application folder.
 *
 * \param *file			A buffer to hold a full pathname.
 * \param len			The size of the buffer.
 * \param *leaf			The leaf file name to use.
 */

void config_find_load_file(char *file, size_t len, char *leaf)
{
	snprintf(file, len, "Choices:%s.%s", choices_dir, leaf);

	if (osfile_read_no_path(file, NULL, NULL, NULL, NULL) != fileswitch_IS_FILE) {
		snprintf(file, len, "%s.%s", local_dir, leaf);
		if (osfile_read_no_path (file, NULL, NULL, NULL, NULL) != fileswitch_IS_FILE)
			*file = '\0';
	}
}


/**
 * Get a filename for the file to save the choices settings to.  <Choices$Write>
 * is tried first; if this fails the application folder is used.
 *
 * \param *file			A buffer to hold a full pathname.
 * \param len			The size of the buffer.
 * \param *leaf			The leaf file name to use.
 */

void config_find_save_file(char *file, size_t len, char *leaf)
{
	int		var_len;

	*file = '\0';

	os_read_var_val_size("Choices$Write", 0, os_VARTYPE_STRING, &var_len, NULL);

	if (var_len == 0) {
		snprintf(file, len, "%s.%s", local_dir, leaf);
	} else {
		snprintf(file, len, "<Choices$Write>.%s", choices_dir);
		if (osfile_read_no_path(file, NULL, NULL, NULL, NULL) == fileswitch_NOT_FOUND)
			osfile_create_dir(file, 0);

		snprintf(file, len, "<Choices$Write>.%s.%s", choices_dir, leaf);
	}
}


/**
 * Load the currently saved configuration into memory, overriding any settings
 * currently stored in memory.
 *
 * \return		TRUE if successful; else FALSE.
 */

osbool config_load(void)
{
	char		file[1024], token[1024], contents[1024];
	FILE		*in;


	/* Find the options.  First try the Choices: file then the one in the application. */

	config_find_load_file(file, sizeof(file), "Choices");

	if (file == NULL || *file == '\0')
		return FALSE;

	/* If a config file was found, use it. */

	in = fopen(file, "r");

	if (in == NULL)
		return FALSE;

	while (config_read_token_pair(in, token, contents, NULL) != sf_READ_CONFIG_EOF) {
		/* If the token can be matched to a current setting, save it. */

		if (config_find_opt(token) != NULL)
			config_opt_set(token, config_read_opt_string(contents));
		else if (config_find_int(token) != NULL)
			config_int_set(token, atoi (contents));
		else if (config_find_str(token) != NULL)
			config_str_set(token, contents);
	}

	fclose(in);

	return TRUE;
}


/**
 * Save the current configuration from memory into the applicable Choices
 * file, recording only those values which differ from the defaults.
 *
 * \return		TRUE if successful; else FALSE.
 */

osbool config_save(void)
{
	char		file[1024];
	FILE		*out;
	config_opt	*opt_block;
	config_int	*int_block;
	config_str	*str_block;


	config_find_save_file(file, sizeof(file), "Choices");

	if (file == NULL || *file == '\0')
		return FALSE;

	out = fopen(file, "w");

	if (out == NULL)
		return FALSE;

	fprintf(out, "# >Choices for %s\n\n", application_name);

	/* Do the opt configs */

	opt_block = opt_list;

	while (opt_block != NULL) {
		if (opt_block->value != opt_block->initial)
			fprintf(out, "%s: %s\n", opt_block->name, config_return_opt_string(opt_block->value));

		opt_block = opt_block->next;
	}

	/* Do the int configs */

	int_block = int_list;

	while (int_block != NULL) {
		if (int_block->value != int_block->initial)
			fprintf(out, "%s: %d\n", int_block->name, int_block->value);

		int_block = int_block->next;
	}

	/* Do the str configs */

	str_block = str_list;

	while (str_block != NULL) {
		if (strcmp(str_block->value, str_block->initial) != 0)
			fprintf(out, "%s: \"%s\"\n", str_block->name, str_block->value);

		str_block = str_block->next;
	}

	fclose(out);

	return TRUE;
}


/**
 * Restore the default configuration settings.
 *
 * ** Currently Not Implemented **
 *
 * \return			TRUE if successful; else FALSE.
 */

osbool config_restore_default(void)
{
	return FALSE;
}


/**
 * Process lines from a file, until a valid token/value pair is found or EOF is
 * reached.  Return values for the token and value in *token and *value; if a new
 * section is encountered, return that in *section.
 *
 * Result code is: sf_READ_CONFIG_EOF, sf_READ_CONFIG_NEW_SECTION or
 * sf_READ_CONFIG_VALUE_RETURNED.  New sections are returned with the first
 * token in the section.
 *
 * No buffer overrun protection is performed.  Ensure *token, *value and *section
 * are large enough.
 *
 * \param *file		The file to read from.
 * \param *token	Pointer to buffer to hold a token name.
 * \param *value	Pointer to buffer to hold a token value.
 * \param *section	Pointer to buffer to hold a section name.
 * \return		Result code.
 */

int config_read_token_pair(FILE *file, char *token, char *value, char *section)
{
	char		line[1024], *stripped_line, *a, *b;
	int		result = sf_READ_CONFIG_EOF;
	osbool		read = FALSE;


	if (file == NULL)
		return result;

	while (!read && (fgets(line, sizeof(line), file) != NULL)) {
		if (*line != '#') {
			stripped_line = string_strip_surrounding_whitespace(line);

			if (string_wildcard_compare("[*]", stripped_line, 1)) {
				*strrchr(stripped_line, ']') = '\0';
				if (section != NULL)
					strcpy(section, stripped_line + 1);
				result = sf_READ_CONFIG_NEW_SECTION;
			} else {
				a = NULL;
				b = strchr(stripped_line, ':');

				if (b != NULL) {
					a = stripped_line;
					*b++ = '\0';

					if (token != NULL)
						strcpy(token, a);

					if (value != NULL) {
						/* Remove external whitespace and enclosing quotes if presnt. */

						b = string_strip_surrounding_whitespace(b);

						if (*b == '"' && *(strchr(b, '\0')-1) == '"') {
							b++;
							*(strchr(b, '\0')-1) = '\0';
						}

						strcpy(value, b);
					}

					if (result != sf_READ_CONFIG_NEW_SECTION)
						result = sf_READ_CONFIG_VALUE_RETURNED;

					read = TRUE;
				} else {
					if (token != NULL)
						*token = '\0';

					if (value != NULL)
						*value = '\0';
				}
			}
		}
	}

	return result;
}


/**
 * Write a token/value pair to file, enclosing parameters that contain
 * leading or trailing whitespace in quotes.
 *
 * \param *file			The file handle to write to.
 * \param *token		Pointer to a string containing the token.
 * \param *value		Pointer to a string containing a representation of the value.
 * \return			The number of bytes written, or negative for an error.
 */

int config_write_token_pair(FILE *file, char *token, char *value)
{
	int	result = -1;

	if (file == NULL)
		return result;

	if (isspace(*value) || isspace(*(strchr(value, '\0')-1)))
		result = fprintf(file, "%s: \"%s\"\n", token, value);
	else
		result = fprintf(file, "%s: %s\n", token, value);

	return result;
}


/**
 * Return a pointer to a string representing the supplied boolean value.
 *
 * \param opt		The boolean value to return a string for.
 * \return		Pointer to a string representing the value ("Yes" or "No").
 */

char *config_return_opt_string(osbool opt)
{
	if (opt == TRUE)
		return ("Yes");
	else
		return ("No");
}


/**
 * Test a string and return TRUE or FALSE depending on whether it is "Yes"
 * or "No".
 *
 * \param *str		The string to be tested.
 * \return		TRUE if the string is "Yes" or "True"; else FALSE.
 */

osbool config_read_opt_string(char *str)
{
	char		line[256];

	strcpy(line, str);
	string_tolower(line);

	return (strcmp(line, "yes") == 0 || strcmp(line, "true") == 0) ? TRUE : FALSE;
}

