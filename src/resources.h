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
 * \file: resources.h
 *
 * Generic RISC OS resource handling for internationalisation and other
 * shared data.
 */

#ifndef SFLIB_RESOURCES
#define SFLIB_RESOURCES

#include <stdlib.h>
#include "oslib/osspriteop.h"

/**
 * A set of resource folders to be used for looking up internationalised data.
 */

typedef char resources_path_set;

/**
 * Locate a regional resource directory on the specified path, updating the
 * path in the process.
 *
 * \param *path		Buffer containing base resource path and space for
 *			regional folder name.
 * \param size		The size of the buffer.
 */

void resources_find_path(char *path, size_t size);


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

osbool resources_initialise_paths(char *path_set, size_t length, char *appvar);


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

osbool resources_find_file(char *paths, char *buffer, size_t length, char *file, bits type);


/**
 * Load a spritefile into a user sprite area, claiming the necessary memory
 * and returning its pointer.
 *
 * \param *file		The sprite file to load.
 * \return		Pointer to the sprite area, or NULL on failure.
 */

osspriteop_area *resources_load_user_sprite_area(char *file);

#endif

