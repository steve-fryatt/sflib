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
 * \file: resources.c
 *
 * Generic RISC OS resource handling for internationalisation and other
 * shared data.
 */

/* OS-Lib header files. */

#include "oslib/territory.h"
#include "oslib/osfile.h"
#include "oslib/fileswitch.h"
#include "oslib/osspriteop.h"
#include "oslib/wimpreadsysinfo.h"

/* SF-Lib header files. */

#include "resources.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * The maximum length of an expanded filename.
 */

#define RESOURCES_MAX_FILENAME 1024

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
	strncpy(new_path, path, size);
	new_path[size - 1] = '\0';

	len = strlen(new_path);
	territory_number_to_name(territory_number(), new_path + len, size - len);

	if (osfile_read_stamped_no_path (new_path, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_DIR) {
		strncpy(path, new_path, size);
		path[size - 1] = '\0';
	} else if ((size - len) >= strlen(uk) + 2) {
		strncat(path, uk, strlen(uk));
	}

	free(new_path);
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
	fileswitch_object_type	object;
	osspriteop_area		*area;
	char			*suffix, full_file[RESOURCES_MAX_FILENAME];
	os_error		*error;

	if (file == NULL)
		return NULL;

	/* Identify the current mode sprite suffix. */

	suffix = wimpreadsysinfo_sprite_suffix();
	snprintf(full_file, RESOURCES_MAX_FILENAME, "%s%s", file, suffix);
	full_file[RESOURCES_MAX_FILENAME - 1] = '\0';

	/* Check for a suffixed sprite file. */

	object = osfile_read_stamped_no_path(full_file, NULL, NULL, &size, NULL, &type);

	/* If not found, check for an un-suffixed sprite file. */

	if (object != fileswitch_IS_FILE || type != osfile_TYPE_SPRITE) {
		strncpy(full_file, file, RESOURCES_MAX_FILENAME);
		full_file[RESOURCES_MAX_FILENAME - 1] = '\0';
		object = osfile_read_stamped_no_path(full_file, NULL, NULL, &size, NULL, &type);
	}

	/* If neither found, exit. */

	if (object != fileswitch_IS_FILE || type != osfile_TYPE_SPRITE)
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
