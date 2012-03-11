/**
 * \file: menus.c
 *
 * SF-Lib - Menus.c
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * RISC OS Wimp Menu Support.  Support for loading menu definitions from
 * file, creating menus on screen, and manipulating menu contents.
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"

/* SF-Lib header files. */

#include "menus.h"
#include "windows.h"

/* ANSII C header files. */

#include <stdlib.h>
#include <string.h>


/* Load a menu template block into memory, optionally linking in dialogue
 * boxes and returning the menu block addresses in the supplied array.
 *
 * This is an external interface, documented in menus.h
 */

menu_template menus_load_templates(char *filename, wimp_w dbox_list[], wimp_menu *menus[], size_t len)
{
	int	*current, *data, dbox, menu, *menu_block;
	int	*z, *t;
	int	size;

	/* Load the menu data into memory. */

	osfile_read_stamped_no_path(filename, NULL, NULL, &size, NULL, NULL);
	data = malloc(size);

	if (data == NULL)
		return data;

	osfile_load_stamped_no_path(filename, (byte *) data, NULL, NULL, NULL, NULL);

	/* Insert the dialogue box pointers.
	 *
	 * These are linked through the structure with each pointer containing an offset to the next pointer
	 * to be filled in.
	 *
	 * If the dbox pointer points to a zero word, then this is a new format file with embedded
	 * dialogue boxes -- in which case we don't do anything.
	 */

	dbox=0;

	if (*data != -1 && dbox_list != NULL) {
		int *next;

		current = (int *) ((int) data + (int) *data);

		if (*current != 0) {
			do {
				next = (int *) *current;
				*current = (int) dbox_list[dbox++];
				if ((int) next != -1)
					current = (int *) ((int) data + (int) next);
			} while ((int) next != -1);
		}
	}

	/* Insert the indirection pointers. */

	if (*(data+1) != -1) {
		int *pointer, offset;

		current = (int *) ((int) data + (int) *(data+1));

		while (*current != -1) {
			pointer = (int *) ((int) data + (int) *current);
			*pointer = (int) current + 4;

			offset = (*(pointer + 2) + 7) & ~3;
			current += offset / 4;
		}
	}

	/* Insert the validation string pointers. */

	if (*(data+2) != -1) {
		int *pointer, offset;

		current = (int *) ((int) data + (int) *(data+2));

		while (*current != -1) {
			pointer = (int *) ((int) data + (int) *current);
			*pointer = (int) current + 8;

			offset = *(current + 1);
			current += offset / 4;
		}
	}

	/* collect together pointers to the menus and link the submenus together. */

	menu_block = data + 5;
	menu = 0;

	while ((int) menu_block != -1) {
		menus[menu++] = (wimp_menu *) menu_block;

		z = menu_block - 1;

		if (*z != -1) {
			t = (int *) (*z + (int) data);

			while ((int) t != -1) {
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


/* Link a window handle into a menu template block as a dialogue box.
 *
 * This is an external interface, documented in menus.h
 */

osbool menus_link_dbox(menu_template data, char *tag, wimp_w dbox)
{
	int next, *current;

	if (data == NULL)
		return FALSE;

	if (*data == -1)
		return FALSE;

	current = (int *) ((int) data + (int) *data);

	if (*current != 0)
		return FALSE;

	current++; /* Find the first tag block */

	/* Find the correct dbox list by string matching the tags. */

	while (*current != -1 && strcmp((char *) (current + 1), tag) != 0) {
		current = (int *) ((int) current +
				((strlen((char *) (current + 1)) + 8) & (~3)));
	}

	if (*current == -1)
		return FALSE;

	current = (int *) ((int) data + (int) *current);

	do {
		next = *current;
		*current = (int) dbox;
		if (next != -1) {
			current = (int *) ((int) data + next);
		}
	} while (next != -1);

	return TRUE;
}


/* Open a menu at the specified pointer position, located according to
 * Style Guide requirements.
 *
 * This is an external interface, documented in menus.h
 */

wimp_menu *menus_create_standard_menu(wimp_menu *menu, wimp_pointer *pointer)
{
	if (menu == NULL || pointer == NULL)
		return NULL;

	wimp_create_menu(menu, pointer->pos.x - 64, pointer->pos.y);

	return menu;
}


/* Open an iconbar menu at the specified pointer position, located according to
 * Style Guide requirements and positioned correctly in the Y direction
 * based on the number of entries in the menu.
 *
 * This is an external interface, documented in menus.h
 */

wimp_menu *menus_create_iconbar_menu(wimp_menu *menu, wimp_pointer *pointer)
{
	int entry = 0, entries = 0, lines = 0;

	if (menu == NULL || pointer == NULL)
		return NULL;

	do {
		entries ++;
		if ((menu->entries[entry].menu_flags & wimp_MENU_SEPARATE) != 0)
			lines++;
	} while ((menu->entries[entry++].menu_flags & wimp_MENU_LAST) == 0);

	wimp_create_menu(menu, pointer->pos.x - 64, 96 + (entries * (menu->height + menu->gap)) + (lines * 24));

	return menu;
}


/* Open a popup menu at the specified pointer position, located according to
 * Style Guide requirements and assuming that the icon under the pointer is
 * the popup meny icon which launched the menu.
 *
 * This is an external interface, documented in menus.h
 */

wimp_menu *menus_create_popup_menu(wimp_menu *menu, wimp_pointer *pointer)
{
	wimp_window_state	window;
	wimp_icon_state		icon;

	if (menu == NULL || pointer == NULL)
		return NULL;

	window.w = pointer->w;
	wimp_get_window_state (&window);

	icon.w = pointer->w;
	icon.i = pointer->i;
	wimp_get_icon_state(&icon);

	wimp_create_menu(menu, window.visible.x0 + icon.icon.extent.x1 - window.xscroll,
			window.visible.y1 + icon.icon.extent.y1 - window.yscroll);

	return menu;
}

/* Set the ticked status of an entry in a menu block.
 *
 * This is an external interface, documented in menus.h
 */

void menus_tick_entry(wimp_menu *menu, int entry, osbool tick)
{
	if (menu == NULL)
		return;

	if (tick)
		menu->entries[entry].menu_flags |= wimp_MENU_TICKED;
	else
		menu->entries[entry].menu_flags &= ~wimp_MENU_TICKED;
}

/* Set the shaded status of an entry in a menu block.
 *
 * This is an external interface, documented in menus.h
 */

void menus_shade_entry(wimp_menu *menu, int entry, osbool shade)
{
	if (menu == NULL)
		return;

	if (shade)
		menu->entries[entry].icon_flags |= wimp_ICON_SHADED;
	else
		menu->entries[entry].icon_flags &= ~wimp_ICON_SHADED;
}


/* Return a pointer to the text of a menu entry, ignoring its indirected
 * status.
 *
 * This is an external interface, documented in menus.h
 */

char *menus_get_text_addr(wimp_menu *menu, int entry)
{
	if (menu == NULL)
		return NULL;

	if (menu->entries[entry].icon_flags & wimp_ICON_INDIRECTED)
		return menu->entries[entry].data.indirected_text.text;
	else
		return menu->entries[entry].data.text;
}


/* Return a pointer to the text of a menu entry.
 *
 * This is an external interface, documented in menus.h
 */

char *menus_get_indirected_text_addr(wimp_menu *menu, int entry)
{
	if (menu == NULL || (menu->entries[entry].icon_flags & wimp_ICON_INDIRECTED) == 0)
		return NULL;

	return menu->entries[entry].data.indirected_text.text;
}


/*
 * Return the number of entries in a menu.
 *
 * This is an external interface, documented in menus.h
 */

unsigned menus_get_entries(wimp_menu *menu)
{
	unsigned entries = 0;

	if (menu == NULL)
		return entries;

	while ((menu->entries[entries++].menu_flags & wimp_MENU_LAST) == 0);

	return entries;
}


