/* SF-Lib - Menus.c
 *
 * Version 0.10 (5 May 2003)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"

/* SF-Lib header files. */

#include "sflib/menus.h"
#include "sflib/windows.h"

/* ANSII C header files. */

#include <stdlib.h>

/* None */

/* ================================================================================================================== */

int *load_menus (char *filename, wimp_w *dbox_list, wimp_menu *menus[])
{

  int *current, *data, dbox, menu, *menu_block;
  int *z, *t;

  int size;

  /* Load the menu data into memory. */

  osfile_read_stamped_no_path (filename, NULL, NULL, &size, NULL, NULL);
  data = (int *) malloc (size);

  if (data == NULL)
  	return data;

  osfile_load_stamped_no_path (filename, (byte *) data, NULL, NULL, NULL, NULL);

  /* Insert the dialogue box pointers.
   *
   * These are linked through the structure with each pointer containing an offset to the next pointer
   * to be filled in.
   *
   * If the dbox pointer points to a zero word, then this is a new format file with embedded
   * dialogue boxes -- in which case we don't do anything.
   */


  dbox=0;

  if (*data != -1 && dbox_list != NULL)
  {
    int *next;

    current = (int *) ((int) data + (int) *data);

    if (*current != 0)
    {
      do
      {
        next = (int *) *current;
        *current = (int) dbox_list[dbox++];
        if ((int) next != -1)
        {
          current = (int *) ((int) data + (int) next);
        }
      }
      while ((int) next != -1);
    }
  }

  /* Insert the indirection pointers. */

  if (*(data+1) != -1)
  {
    int *pointer, offset;

    current = (int *) ((int) data + (int) *(data+1));

    while (*current != -1)
    {
      pointer = (int *) ((int) data + (int) *current);
      *pointer = (int) current + 4;

      offset = (*(pointer + 2) + 7) & ~3;
      current += offset / 4;
    }
  }

  /* Insert the validation string pointers. */

  if (*(data+2) != -1)
  {
    int *pointer, offset;

    current = (int *) ((int) data + (int) *(data+2));

    while (*current != -1)
    {
      pointer = (int *) ((int) data + (int) *current);
      *pointer = (int) current + 8;

      offset = *(current + 1);
      current += offset / 4;
    }
  }


  /* collect together pointers to the menus and link the submenus together. */

  menu_block = data + 5;
  menu = 0;

  while ((int) menu_block != -1)
  {
    menus[menu++] = (wimp_menu *) menu_block;

    z = menu_block - 1;

    if (*z != -1)
    {

      t = (int *) (*z + (int) data);

      while ((int) t != -1)
      {
        z = (int *) *t;
        if ((int) z != -1)
          z = (int*) ((int) z + (int) data);

        *t = (int) menu_block;
        t = z;
      }
    }

    menu_block = (int *) *(menu_block - 2);
    if ((int) menu_block != -1)
      menu_block = (int*) ((int) menu_block + (int) data);
  }

  return data;
}

/* -------------------------------------------------------------------------------------------------------------------*/

int load_menus_dbox(

/* ================================================================================================================== */

wimp_menu *create_standard_menu (wimp_menu *menu, wimp_pointer *pointer)
{
  wimp_create_menu (menu, pointer->pos.x - 64, pointer->pos.y);

  return (menu);
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_menu *create_iconbar_menu (wimp_menu *menu, wimp_pointer *pointer, int entries, int lines)
{
  wimp_create_menu (menu, pointer->pos.x - 64, 96 + (entries * 44) + (lines * 24));

  return (menu);
}

/* ------------------------------------------------------------------------------------------------------------------ */

wimp_menu *create_popup_menu (wimp_menu *menu, wimp_pointer *pointer)
{
  wimp_window_state window;
  wimp_icon_state   icon;

  window.w = pointer->w;
  wimp_get_window_state (&window);

  icon.w = pointer->w;
  icon.i = pointer->i;
  wimp_get_icon_state (&icon);

  wimp_create_menu (menu, window.visible.x0+icon.icon.extent.x1-window.xscroll,
                          window.visible.y1+icon.icon.extent.y1-window.yscroll);

  return (menu);
}

/* ================================================================================================================== */

void tick_menu_item (wimp_menu *menu, int item, int tick)
{
  if (tick)
  {
    menu->entries[item].menu_flags |= wimp_MENU_TICKED;
  }
  else
  {
    menu->entries[item].menu_flags &= ~wimp_MENU_TICKED;
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void shade_menu_item (wimp_menu *menu, int item, int shade)
{
  if (shade)
  {
    menu->entries[item].icon_flags |= wimp_ICON_SHADED;
  }
  else
  {
    menu->entries[item].icon_flags &= ~wimp_ICON_SHADED;
  }
}

/* ================================================================================================================== */

/* Return a pointer to a menu text, indirected or not. */

char *menu_text (wimp_menu *menu, int item)
{
  if (menu->entries[item].icon_flags & wimp_ICON_INDIRECTED)
  {
    return (menu->entries[item].data.indirected_text.text);
  }
  else
  {
    return (menu->entries[item].data.text);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Return pointer to an indirected menu text, or NULL if it isn't indirected. */

char *indirected_menu_text (wimp_menu *menu, int item)
{
  if (menu->entries[item].icon_flags & wimp_ICON_INDIRECTED)
  {
    return (menu->entries[item].data.indirected_text.text);
  }
  else
  {
    return (NULL);
  }
}
