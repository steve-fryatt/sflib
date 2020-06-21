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
 * \file: resources.c
 *
 * Generic RISC OS resource handling for internationalisation and other
 * shared data.
 */

/* OS-Lib header files. */

#include "oslib/territory.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/fileswitch.h"
#include "oslib/osspriteop.h"
#include "oslib/serviceinternational.h"
#include "oslib/wimpreadsysinfo.h"

/* SF-Lib header files. */

#include "resources.h"

#include "string.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * The maximum length of an expanded filename.
 */

#define RESOURCES_MAX_FILENAME 1024

/**
 * The number of bytes to allocate to storing country names.
 */

#define RESOURCES_MAX_PATH_SET_LEN 256

/* Function Prototypes. */

static osbool resources_read_variable(char *varname, char *buffer, size_t length);
static osbool resources_read_country(char *buffer, size_t length);
static void resources_add_path_set(char *path, char *set, char **out, char *end);
static void resources_copy_and_separate(char *buffer, size_t length, int *tail, char **string);


/* Locate a regional resource directory on the specified path, updating the
 * path in the process.
 *
 * This is an external interface, documented in resources.h
 */

void resources_find_path(char *path, size_t size)
{
	int	len;
	char	*new_path;
	char	*uk = "UK";

	new_path = malloc(size);

	if (new_path == NULL)
		return;

	len = strlen(path);
	strncat(path, ".", size - (len + 2));
	string_copy(new_path, path, size);

	len = strlen(new_path);
	territory_number_to_name(territory_number(), new_path + len, size - len);

	if (osfile_read_stamped_no_path (new_path, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_DIR)
		string_copy(path, new_path, size);
	else if ((size - len) >= strlen(uk) + 2)
		strncat(path, uk, strlen(uk));

	free(new_path);
}


/**
 * Initialise the resources path set, ready for looking up resource file names.
 * If successful, on exit the *path_set buffer will contain the original
 * path, followed by one or more country folder names, each terminated by
 * a '\0'. The final entry will be followed by a second '\0' to indicate
 * the end of the list.
 *
 * \param *path_set		Pointer to the buffer to hold the path set, which
 *				should contain the path of the resources folder
 *				terminated by '\0'.
 * \param length		The length of the path set buffer.
 * \param *appvar		Pointer to the application language variable name.
 * \return			TRUE if successful; FALSE on failure.
 */

osbool resources_initialise_paths(char *path_set, size_t length, char *appvar)
{
	char *out = NULL, *end = NULL;
	char variable[RESOURCES_MAX_PATH_SET_LEN];
	osbool success = TRUE;

	if (path_set == NULL)
		return FALSE;

	/* Find the end of the initial path, and the end of the buffer. */

	out = path_set + strlen(path_set) + 1;
	end = path_set + length;

	if (out > end)
		return FALSE;

	/* Add any countries stored in AppName$Language. */

	if (resources_read_variable(appvar, variable, RESOURCES_MAX_PATH_SET_LEN))
		resources_add_path_set(path_set, variable, &out, end);

	/* Add any countries stored in ResFind$LanguagesPref. */

	if (resources_read_variable("ResFind$LanguagePref", variable, RESOURCES_MAX_PATH_SET_LEN))
		resources_add_path_set(path_set, variable, &out, end);

	/* Add the configured country. */

	if (resources_read_country(variable, RESOURCES_MAX_PATH_SET_LEN))
		resources_add_path_set(path_set, variable, &out, end);

	/* Add any countries stored in ResFind$LanguagesSuff. */

	if (resources_read_variable("ResFind$LanguageSuff", variable, RESOURCES_MAX_PATH_SET_LEN))
		resources_add_path_set(path_set, variable, &out, end);

	/* Add in the fallback of "UK". */

	resources_add_path_set(path_set, "UK", &out, end);

	/* If there was a buffer overrun, pull the pointer back and terminate
	 * the final string to be written.
	 */

	if (out >= end) {
		out = end - 2;
		*out++ = '\0';
		success = FALSE;
	}

	/* Terminate the list of strings. */

	*out = '\0';

	return success;
}


/**
 * Read the the value of a system variable into a supplied buffer,
 * terminating it before returning. No attempt is made to check
 * the length of the value before reading, so it will be truncated
 * if too long.
 *
 * \param *name			Pointer to the name of the variable to be read.
 * \param *buffer		Pointer to the buffer to take the variable value.
 * \param length		The length of the supplied buffer.
 * \return			TRUE if successful; FALSE on failure.
 */

static osbool resources_read_variable(char *varname, char *buffer, size_t length)
{
	int used;
	os_var_type type;
	os_error *error;

	if (varname == NULL || buffer == NULL || length == 0)
		return FALSE;

	error = xos_read_var_val(varname, buffer, length, 0, os_VARTYPE_STRING, &used, NULL, &type);
	if (error != NULL || type != os_VARTYPE_STRING)
		return FALSE;

	if (used >= length)
		used = length - 1;

	buffer[used] = '\0';

	return TRUE;
}


/**
 * Read the name of the configured country into a supplied buffer,
 * terminating it before returning.
 *
 * \param *buffer		Pointer to the buffer to take the country name.
 * \param length		The length of the supplied buffer.
 * \return			TRUE if successful; FALSE on failure.
 */

static osbool resources_read_country(char *buffer, size_t length)
{
	int used;
	territory_t country;
	osbool unclaimed;
	os_error *error;

	if (buffer == NULL || length == 0)
		return FALSE;

	error = xosbyte1(osbyte_COUNTRY_NUMBER, 127, 0, &country);
	if (error != NULL)
		return FALSE;

	error = xserviceinternational_country_number_to_country_name(country, buffer, length, &unclaimed, &used);
	if (error != NULL || unclaimed == TRUE)
		return FALSE;

	if (used >= length)
		used = length - 1;

	buffer[used] = '\0';

	return TRUE;
}


/**
 * Take a comma-separated list of country names, and test each
 * one against the available resource folders. For each that exists,
 * add the name to our list.
 * 
 * \param *path			Pointer to the path name for the resources folder.
 * \param *set			Pointer to a comma-separated list of country
 *				folder names to be tested.
 * \param **out			Pointer to a pointer to the end of the current
 *				path set list, which is updated on exit if any
 *				folders are added.
 * \param *end			Pointer to the first position after the end of
 *				the path set list buffer.
 */

static void resources_add_path_set(char *path, char *set, char **out, char *end)
{
	char dirname[RESOURCES_MAX_FILENAME];
	char *in = set, *pos = set;
	int tail = 0;
	fileswitch_object_type object_type;
	os_error *error;

	if (path == NULL || set == NULL || out == NULL || *out == NULL)
		return;

	/* Copy the path into the filename buffer, with no terminator. */

	resources_copy_and_separate(dirname, RESOURCES_MAX_FILENAME, &tail, &path);

	/* Scan the supplied set of resource folder names, testing each as we go
	 * and adding any that exist to the end of our ordered search list.
	 */

	while (*pos != '\0') {
		while (*pos != '\0' && *pos != ',')
			pos++;

		/* Replace commas with '\0' terminators. */

		if (*pos == ',')
			*pos++ = '\0';

		/* If there's a non-empty name, add the name to the supplied
		 * path and check whether or not the resource folder exists.
		 */

		if (*in != '\0') {
			string_copy(dirname + tail, in, RESOURCES_MAX_FILENAME - tail);

			error = xosfile_read_no_path(dirname, &object_type, NULL, NULL, NULL, NULL);

			/* Add the name to the list if it exists. */

			if (error == NULL && object_type == fileswitch_IS_DIR)
				while ((*out < end) && ((*(*out)++ = *in++) != '\0'));
		}

		in = pos;
	}
}


/**
 * Given a pre-configured resource path set from resources_initialise_paths(),
 * search for a file of a given name and filetype by order of preference. If
 * one is found, copy the full name into the supplied buffer. If no file is
 * found, the buffer will be returned containing a terminated, empty string.
 * 
 * \param *paths		Pointer to the path set created by
 *				resources_initialise_paths().
 * \param *buffer		Pointer to a buffer to take a returned filename.
 * \param length		The length of the supplied buffer.
 * \param *file			Pointer to the name of the required file.
 * \param type			The RISC OS filetype of the required file.
 * \return			TRUE if a file was found; otherwise FALSE.
 */

osbool resources_find_file(char *paths, char *buffer, size_t length, char *file, bits type)
{
	char filename[RESOURCES_MAX_FILENAME];
	int tail = 0, ptr = 0;
	fileswitch_object_type object_type;
	bits file_type;
	os_error *error;

	if (paths == NULL || file == NULL)
		return FALSE;

	if (buffer != NULL && length > 0)
		*buffer = '\0';

	/* Copy the path into the filename buffer. */

	resources_copy_and_separate(filename, RESOURCES_MAX_FILENAME, &tail, &paths);

	/* Run through the list of resource folders, testing for the file in each. */

	while (*(++paths) != '\0') {
		ptr = tail;

		/* Add the folder and filenames to the buffer. */

		resources_copy_and_separate(filename, RESOURCES_MAX_FILENAME, &ptr, &paths);
		string_copy(filename + ptr, file, RESOURCES_MAX_FILENAME - ptr);

		/* Test for the presence of the file. */

		error = xosfile_read_stamped_no_path(filename, &object_type, NULL, NULL, NULL, NULL, &file_type);

		/* If the file exists, and it is of the correct type, copy its
		 * name into the supplied buffer and return.
		 */

		if (error == NULL && object_type == fileswitch_IS_FILE && file_type == type) {
			if (buffer != NULL && length > 0)
				string_copy(buffer, filename, length);
			return TRUE;
		}
	}

	return FALSE;
}


/**
 * Copy a string from one buffer to another, updating pointers as we
 * go. The copy is NOT terminated, but will have a RISC OS directory
 * separator added to the end if one is not present.
 * 
 * \param *buffer		Pointer to the buffer to hold the copy.
 * \param length		The length of the buffer.
 * \param *tail			Pointer to a variable indicating the point
 *				in the buffer to start copying the data.
 *				This will be updated on exit to indicate the
 *				first position after the terminator.
 * \param **string		Pointer to a pointer to the string to copy
 *				in to the buffer. This will be updated on
 *				exit to point to the zero terminator of
 *				the copied string.
 */

static void resources_copy_and_separate(char *buffer, size_t length, int *tail, char **string)
{
	if (buffer == NULL || tail == NULL || string == NULL || *string == NULL)
		return;

	/* Copy the path into the filename buffer, with no terminator. */

	for (; (**string != '\0') && (*tail < length); (*tail)++)
		buffer[*tail] = *(*string)++;

	/* Add a directory separator if required. */

	if ((*tail > 0) && (*tail < length) && (buffer[*tail - 1] != '.'))
		buffer[(*tail)++] = '.';
}


/* Load a spritefile into a user sprite area, claiming the necessary memory
 * and returning its pointer.
 *
 * This is an external interface, documented in resources.h
 */

osspriteop_area *resources_load_user_sprite_area(char *file)
{
	int			size;
	bits			type;
	fileswitch_object_type	object_type;
	osspriteop_area		*area;
	char			*suffix, full_file[RESOURCES_MAX_FILENAME];
	os_error		*error;

	if (file == NULL)
		return NULL;

	/* Identify the current mode sprite suffix. */

	suffix = wimpreadsysinfo_sprite_suffix();
	string_printf(full_file, RESOURCES_MAX_FILENAME, "%s%s", file, suffix);

	/* Check for a suffixed sprite file. */

	object_type = osfile_read_stamped_no_path(full_file, NULL, NULL, &size, NULL, &type);

	/* If not found, check for an un-suffixed sprite file. */

	if (object_type != fileswitch_IS_FILE || type != osfile_TYPE_SPRITE) {
		string_copy(full_file, file, RESOURCES_MAX_FILENAME);
		object_type = osfile_read_stamped_no_path(full_file, NULL, NULL, &size, NULL, &type);
	}

	/* If neither found, exit. */

	if (object_type != fileswitch_IS_FILE || type != osfile_TYPE_SPRITE)
		return NULL;

	/* Allocate the sprite area memory. */

	size += sizeof(int);
	area = malloc(size);
	if (area == NULL)
		return NULL;

	/* Initialise the sprite area. */

	area->size = size;
	area->first = 16;

	/* Load the sprite file into the area. */

	error = xosspriteop_load_sprite_file(osspriteop_USER_AREA, area, full_file);
	if (error != NULL) {
		free(area);
		return NULL;
	}

	return area;
}
