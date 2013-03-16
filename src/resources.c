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


/* Locate a regional resource directory on the specified path, updating the
 * path in the process.
 *
 * This is an external interface, documented in resources.h
 */

void resources_find_path(char *path, size_t size)
{
	int	len;
	char	*new_path;

	new_path = malloc(size);

	if (new_path == NULL)
		return;

	strcat(path, ".");
	strcpy(new_path, path);

	len = strlen(new_path);
	territory_number_to_name(territory_number(), new_path + len, size - len);

	if (osfile_read_stamped_no_path (new_path, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_DIR)
		strcpy(path, new_path);
	else if ((size - len) >= 2)
		strcat(path, "UK");
}


/* Load a spritefile into a user sprite area, claiming the necessary memory
 * and returning its pointer.
 *
 * This is an external interface, documented in resources.h
 */

osspriteop_area *resources_load_user_sprite_area(char *file)
{
	int			size, type;
	fileswitch_object_type	object;
	osspriteop_area		*area;
	char			*suffix, fullfile[1024];

	suffix = wimpreadsysinfo_sprite_suffix();
	snprintf(fullfile, sizeof(fullfile), "%s%s", file, suffix);

	object = osfile_read_stamped_no_path(fullfile, NULL, NULL, &size, NULL, (bits *) &type);

	if (object != fileswitch_IS_FILE || type != 0xff9) {
		strcpy(fullfile, file);
		object = osfile_read_stamped_no_path(fullfile, NULL, NULL, &size, NULL, (bits *) &type);
	}

	if (object != fileswitch_IS_FILE || type != 0xff9)
		return NULL;

	size += 4;
	area = malloc(size);
	area->size = size;
	area->first = 16;

	osspriteop_load_sprite_file(osspriteop_NAME, area, fullfile);

	return area;
}

