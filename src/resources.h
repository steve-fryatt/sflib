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
 * Locate a regional resource directory on the specified path, updating the
 * path in the process.
 *
 * \param *path		Buffer containing base resource path and space for
 *			regional folder name.
 * \param size		The size of the buffer.
 */

void resources_find_path(char *path, size_t size);


/**
 * Load a spritefile into a user sprite area, claiming the necessary memory
 * and returning its pointer.
 *
 * \param *file		The sprite file to load.
 * \return		Pointer to the sprite area, or NULL on failure.
 */

osspriteop_area *resources_load_user_sprite_area(char *file);

#endif

