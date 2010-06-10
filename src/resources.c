/* SF-Lib - Resources.c
 *
 * Version 0.10 (5 May 2003)
 */

/* OS-Lib header files. */

#include "oslib/territory.h"
#include "oslib/osfile.h"
#include "oslib/fileswitch.h"
#include "oslib/osspriteop.h"
#include "oslib/wimpreadsysinfo.h"

/* SF-Lib header files. */

#include "sflib/resources.h"

/* ANSII C header files. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ================================================================================================================== */

 void find_resource_path (char *path, int size)
{
  int len;
  char *new_path;

  new_path = (char *) malloc (size);

  if (new_path != NULL)
  {
    strcat (path, ".");
    strcpy (new_path, path);

    len = strlen (new_path);
    territory_number_to_name (territory_number (), new_path + len, size - len);

    if (osfile_read_stamped_no_path (new_path, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_DIR)
      strcpy (path, new_path);
    else
      if ((size - len) >= 2)
        strcat (path, "UK");
  }
}

/* ================================================================================================================== */

osspriteop_area *load_user_sprite_area (char *file)
{
  int                    size, type;
  fileswitch_object_type object;
  osspriteop_area        *area;
  char                   *suffix, fullfile[1024];


  suffix = wimpreadsysinfo_sprite_suffix ();
  snprintf (fullfile, sizeof(fullfile), "%s%s", file, suffix);

  object = osfile_read_stamped_no_path (fullfile, NULL, NULL, &size, NULL, (bits *) &type);

  if (!(object == fileswitch_IS_FILE && type == 0xff9))
  {
    strcpy (fullfile, file);
    object = osfile_read_stamped_no_path (fullfile, NULL, NULL, &size, NULL, (bits *) &type);
  }

  if (object == fileswitch_IS_FILE && type == 0xff9)
  {
    size += 4;
    area = (osspriteop_area *) malloc (size);
    area->size = size;
    area->first = 16;

    osspriteop_load_sprite_file (osspriteop_NAME, area, fullfile);
  }
  else
  {
    area = NULL;
  }

  return (area);
}
