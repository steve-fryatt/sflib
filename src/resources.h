/**
 * \file: resources.h
 *
 * SF-Lib - Resources.h
 *
 * (C) Stephen Fryatt, 2003-2011
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

