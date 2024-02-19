/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: menus.c
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
#include "string.h"

/* ANSII C header files. */

#include <stdlib.h>
#include <string.h>

/* Offsets into the menu template header */

#define MENU_DIALOGUE_LIST_OFFSET (0)
#define MENU_INDIRECTED_LIST_OFFSET (1)
#define MENU_VALIDATION_LIST_OFFSET (2)
#define MENU_EXTENDED_HEADER_OFFSET (3)
#define MENU_FLAGS_OFFSET (4)
#define MENU_NAMES_LIST_OFFSET (5)
#define MENU_HEADER_END_OFFSET (6)
#define MENU_LEGACY_HEADER_END_OFFSET (2)

/* Offset from last header word to first menu block. */

#define MENU_FIRST_BLOCK_OFFSET (3)

/* Offsets back from the menu block */

#define MENU_BLOCK_SUBMENU_LIST (-1)
#define MENU_BLOCK_NEXT_MENU (-2)

/* Offsets into an indirected data block. */

#define MENU_INDIRECTION_BLOCK (0)
#define MENU_INDIRECTION_DATA (1)

/* Offsets into a validation data block. */

#define MENU_VALIDATION_BLOCK (0)
#define MENU_VALIDATION_SIZE (1)
#define MENU_VALIDATION_DATA (2)

/* Offsets into a menu name block. */

#define MENU_NAME_OFFSET (0)
#define MENU_NAME_TEXT (1)

/* Offsets into a dialogue name block. */

#define MENU_DIALOGUE_OFFSET (0)
#define MENU_DIALOGUE_TEXT (1)

/* Menu template constants. */

#define MENU_ZERO_WORD (0)
#define MENU_END_OF_LIST (-1)
#define MENU_WORD_LENGTH (4)


/* Load a menu template block into memory, optionally linking in dialogue
 * boxes and returning the menu block addresses in the supplied array.
 *
 * This is an external interface, documented in menus.h
 */

menu_template menus_load_templates(char *filename, wimp_w dbox_list[], wimp_menu *menus[], size_t len)
{
	int			*current, *data, dbox, menu, *menu_block;
	int			*submenu, *next_submenu;
	int			size;
	fileswitch_object_type	type;
	os_error		*error;

	/* Load the menu data into memory. */

	error = xosfile_read_stamped_no_path(filename, &type, NULL, NULL, &size, NULL, NULL);
	if (error != NULL)
		return NULL;

	if (type != fileswitch_IS_FILE)
		return NULL;

	data = malloc(size);

	if (data == NULL)
		return data;

	error = xosfile_load_stamped_no_path(filename, (byte *) data, NULL, NULL, NULL, NULL, NULL);
	if (error != NULL) {
		free(data);
		return NULL;
	}

	/* Insert the dialogue box pointers.
	 *
	 * These are linked through the structure with each pointer containing an offset to the next pointer
	 * to be filled in.
	 *
	 * If the dbox pointer points to a zero word, then this is a new format file with embedded
	 * dialogue boxes -- in which case we don't do anything.
	 */

	dbox = 0;

	if (*(data + MENU_DIALOGUE_LIST_OFFSET) != MENU_END_OF_LIST && dbox_list != NULL) {
		int *next;

		current = (int *) ((int) data + (int) *(data + MENU_DIALOGUE_LIST_OFFSET));

		if (*current != MENU_ZERO_WORD) {
			do {
				next = (int *) *current;
				*current = (int) dbox_list[dbox++];
				if ((int) next != MENU_END_OF_LIST)
					current = (int *) ((int) data + (int) next);
			} while ((int) next != MENU_END_OF_LIST);
		}
	}

	/* Insert the indirection pointers. */

	if (*(data + MENU_INDIRECTED_LIST_OFFSET) != MENU_END_OF_LIST) {
		int *pointer, offset;

		current = (int *) ((int) data + (int) *(data + MENU_INDIRECTED_LIST_OFFSET));

		while (*current != MENU_END_OF_LIST) {
			pointer = (int *) ((int) data + (int) *(current + MENU_INDIRECTION_BLOCK));
			*pointer = (int) current + (MENU_INDIRECTION_DATA * MENU_WORD_LENGTH);

			offset = (*(pointer + 2) + 7) & ~3;
			current += offset / MENU_WORD_LENGTH;
		}
	}

	/* Insert the validation string pointers. */

	if (*(data + MENU_VALIDATION_LIST_OFFSET) != MENU_END_OF_LIST) {
		int *pointer, offset;

		current = (int *) ((int) data + (int) *(data + MENU_VALIDATION_LIST_OFFSET));

		while (*current != MENU_END_OF_LIST) {
			pointer = (int *) ((int) data + (int) *(current + MENU_VALIDATION_BLOCK));
			*pointer = (int) current + (MENU_VALIDATION_DATA * MENU_WORD_LENGTH);

			offset = *(current + MENU_VALIDATION_SIZE);
			current += offset / MENU_WORD_LENGTH;
		}
	}

	/* collect together pointers to the menus and link the submenus together. */

	if (*(data + MENU_EXTENDED_HEADER_OFFSET) == MENU_ZERO_WORD) {
		/* There's an extended header, so point to where we think
		 * the terminating zero word should be.
		 */

		menu_block = data + MENU_HEADER_END_OFFSET;

		/* Step forward until we find the zero word. */

		while (*menu_block != MENU_ZERO_WORD)
			menu_block++;
	} else {
		/* There's no extended header, so point to the end of
		 * the standard 'legacy' header.
		 */

		menu_block = data + MENU_LEGACY_HEADER_END_OFFSET;
	}

	/* Step on from the last header byte to the first menu definition. */

	menu_block += MENU_FIRST_BLOCK_OFFSET;

	/* Count the menus that we find. */

	menu = 0;

	while ((int) menu_block != MENU_END_OF_LIST) {
		if (menus != NULL && menu < len)
			menus[menu++] = (wimp_menu *) menu_block;

		submenu = menu_block + MENU_BLOCK_SUBMENU_LIST;

		if (*submenu != MENU_END_OF_LIST) {
			next_submenu = (int *) (*submenu + (int) data);

			while ((int) next_submenu != MENU_END_OF_LIST) {
				submenu = (int *) *next_submenu;
				if ((int) submenu != MENU_END_OF_LIST)
					submenu = (int*) ((int) submenu + (int) data);

				*next_submenu = (int) menu_block;
				next_submenu = submenu;
			}
		}

		menu_block = (int *) *(menu_block + MENU_BLOCK_NEXT_MENU);
		if ((int) menu_block != MENU_END_OF_LIST)
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

	if (data == NULL || tag == NULL)
		return FALSE;

	if (*(data + MENU_DIALOGUE_LIST_OFFSET) == MENU_END_OF_LIST)
		return FALSE;

	current = (int *) ((int) data + (int) *(data + MENU_DIALOGUE_LIST_OFFSET));

	if (*current != MENU_ZERO_WORD)
		return FALSE;

	current++; /* Find the first tag block */

	/* Find the correct dbox list by string matching the tags. */

	while (*current != -1 && strcmp((char *) (current + MENU_DIALOGUE_TEXT), tag) != 0) {
		current = (int *) ((int) current +
				((strlen((char *) (current + MENU_DIALOGUE_TEXT)) + 8) & (~3)));
	}

	if (*current == MENU_END_OF_LIST)
		return FALSE;

	current = (int *) ((int) data + (int) *(current + MENU_DIALOGUE_OFFSET));

	do {
		next = *current;
		*current = (int) dbox;
		if (next != MENU_END_OF_LIST) {
			current = (int *) ((int) data + next);
		}
	} while (next != MENU_END_OF_LIST);

	return TRUE;
}


/* Return a menu block pointer from a menu template block, given
 * a textual menu tag.
 *
 * This is an external interface, documented in menus.h
 */

wimp_menu *menus_get_menu(menu_template data, char *tag)
{
	int *current;

	if (data == NULL || tag == NULL)
		return NULL;

	if (*(data + MENU_EXTENDED_HEADER_OFFSET) != MENU_ZERO_WORD || *(data + MENU_NAMES_LIST_OFFSET) == MENU_END_OF_LIST)
		return NULL;

	current = (int *) ((int) data + (int) *(data + MENU_NAMES_LIST_OFFSET));

	/* Find the correct menu offset by string matching the tags. */

	while (*current != -1 && strcmp((char *) (current + MENU_NAME_TEXT), tag) != 0) {
		current = (int *) ((int) current +
				((strlen((char *) (current + MENU_NAME_TEXT)) + 8) & (~3)));
	}

	if (*current == MENU_END_OF_LIST)
		return NULL;

	return (wimp_menu *) ((int) data + (int) *(current + MENU_NAME_OFFSET));
}

/**
 * Create a new menu in memory, setting the title and blanking the
 * entries ready for them to be filled in by menu_build_entry().
 *
 * The memory is allocated using malloc(), and must be freed after
 * use if no longer required.
 * 
 * This is an external interface, documented in menus.h
 */

wimp_menu *menus_build_menu(char *title, osbool external_title, size_t entries)
{
	wimp_menu	*menu = NULL;
	wimp_menu_entry	*entry = NULL;
	char		*buffer = NULL;
	size_t		len, size;

	/* A menu must have one entry. */

	if (entries < 1)
		return NULL;

	/* Allocate the menu definition block. */

	size = wimp_SIZEOF_MENU(entries);

	menu = malloc(size);
	if (menu == NULL)
		return NULL;

	/* Set up the menu title. */

	if (external_title == FALSE) {
		len = strlen(title);
		if (len > 12)
			buffer = malloc(len + 1);
	} else {
		buffer = title;
	}

	if (buffer != NULL) {
		if (external_title == FALSE)
			string_copy(buffer, title, len);
		menu->title_data.indirected_text.text = buffer;
	} else {
		strncpy(menu->title_data.text, title, 12);
	}

	/* Set up the remainder of the menu header. */

	menu->title_fg = wimp_COLOUR_BLACK;
	menu->title_bg = wimp_COLOUR_LIGHT_GREY;
	menu->work_fg = wimp_COLOUR_BLACK;
	menu->work_bg = wimp_COLOUR_WHITE;
	menu->width = (16 * strlen(title)) + 16;
	menu->height = wimp_MENU_ITEM_HEIGHT;
	menu->gap = wimp_MENU_ITEM_GAP;

	/* Initialise the menu entries. */

	entry = menu->entries;

	while (entries-- > 0) {
		entry->menu_flags = (entries > 0) ? 0 : wimp_MENU_LAST;
		entry->sub_menu = NULL;
		entry->icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
				(wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
				(wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);
		strncpy(entry->data.text, "", 12);

		entry++;
	}

	/* Set the title indirection flag. */

	if (buffer != NULL)
		menu->entries[0].menu_flags |= wimp_MENU_TITLE_INDIRECTED;

	return menu;
}


/**
 * Add a menu entry to an existing menu created by menus_build_menu().
 *
 * The memory is allocated using malloc(), and must be freed after
 * use if no longer required.
 * 
 * This is an external interface, documented in menus.h
 */

void menus_build_entry(wimp_menu *menu, int entry, char *text, size_t text_length, enum menus_separator separator, wimp_menu *sub_menu)
{
	wimp_menu_entry	*definition = NULL, *previous = NULL;
	char		*buffer = NULL;
	int		width;
	size_t		len;

	if (menu == NULL || text == NULL)
		return;

	/* Find the menu definition that we're to update. */

	definition = menu->entries;

	while (entry > 0) {
		if (definition->menu_flags & wimp_MENU_LAST)
			return;

		entry--;
		previous = definition++;
	}

	/* Set up the menu text. */

	if (text_length > 0) {
		buffer = text;
		len = text_length;
	} else {
		len = strlen(text);
		if (len > 12)
			buffer = malloc(len + 1);
	}

	if (buffer != NULL) {
		if (text_length == 0)
			string_copy(buffer, text, len);
		definition->data.indirected_text.text = text;
		definition->data.indirected_text.validation = "";
		definition->data.indirected_text.size = len + 1;
		definition->icon_flags |= wimp_ICON_INDIRECTED;
	} else {
		strncpy(definition->data.text, text, 12);
	}

	definition->sub_menu = sub_menu;

	if (separator == MENUS_SEPARATOR_THIS)
		definition->menu_flags |= wimp_MENU_SEPARATE;
	else if (separator == MENUS_SEPARATOR_PREVIOUS && previous != NULL)
		previous->menu_flags |= wimp_MENU_SEPARATE;

	/* Recalculate the menu width. */

	width = (16 * strlen(text)) + 16;
	if (width > menu->width)
		menu->width = width;
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

	if (xwimp_create_menu(menu, pointer->pos.x - 64, pointer->pos.y) != NULL)
		return NULL;

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

	if (xwimp_create_menu(menu, pointer->pos.x - 64,
			96 + (entries * (menu->height + menu->gap)) + (lines * wimp_MENU_ITEM_SEPARATION)) != NULL)
		return NULL;

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
	if (xwimp_get_window_state(&window) != NULL)
		return NULL;

	icon.w = pointer->w;
	icon.i = pointer->i;
	if (xwimp_get_icon_state(&icon) != NULL)
		return NULL;

	if (xwimp_create_menu(menu, window.visible.x0 + icon.icon.extent.x1 - window.xscroll,
			window.visible.y1 + icon.icon.extent.y1 - window.yscroll) != NULL)
		return NULL;

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


