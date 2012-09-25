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
 * \file: menus.h
 *
 * RISC OS Wimp Menu Support.  Support for loading menu definitions from
 * file, creating menus on screen, and manipulating menu contents.
 */

#ifndef SFLIB_MENUS
#define SFLIB_MENUS

#include <stdlib.h>
#include "oslib/wimp.h"

/**
 * Pointer to a menu template block, which holds all of the loaded menu
 * definition blocks and any indirected data.
 */

typedef int * menu_template;


/**
 * Load a menu template block into memory, optionally linking in dialogue
 * boxes and returning the menu block addresses in the supplied array.
 *
 * \param *filename	The filename of the menu template block to be loaded.
 * \param dbox_list[]	Pointer to an array of dialogue boxes, or NULL for
 *			a new format file with named dboxes which are
 *			linked via menus_link_dbox().
 * \param *menus[]	Pointer to an array to hold the menu block pointers.
 * \param len		The number of entries in the menu block array.
 * \return		The menu template handle for the file, NULL for failure.
 */

menu_template menus_load_templates(char *filename, wimp_w dbox_list[], wimp_menu *menus[], size_t len);


/**
 * Link a window handle into a menu template block as a dialogue box.
 *
 * \param data		The menu templates to link to.
 * \param *tag		The dialogue box tag in the menu templates.
 * \param dbox		The window handle to link.
 * \return		TRUE if the link was successful; else FALSE.
 */

osbool menus_link_dbox(menu_template data, char *tag, wimp_w dbox);


/**
 * Open a menu at the specified pointer position, located according to
 * Style Guide requirements.
 *
 * \param *menu		The menu to open.
 * \param *pointer	The details of the pointer position.
 * \return		Pointer to the opened menu block, or NULL on failure.
 */

wimp_menu *menus_create_standard_menu(wimp_menu *menu, wimp_pointer *pointer);


/**
 * Open an iconbar menu at the specified pointer position, located according to
 * Style Guide requirements and positioned correctly in the Y direction
 * based on the number of entries in the menu.
 *
 * \param *menu		The menu to open.
 * \param *pointer	The details of the pointer position.
 * \return		Pointer to the opened menu block, or NULL on failure.
 */

wimp_menu *menus_create_iconbar_menu(wimp_menu *menu, wimp_pointer *pointer);


/**
 * Open a popup menu at the specified pointer position, located according to
 * Style Guide requirements and assuming that the icon under the pointer is
 * the popup meny icon which launched the menu.
 *
 * \param *menu		The menu to open.
 * \param *pointer	The details of the pointer position.
 * \return		Pointer to the opened menu block, or NULL on failure.
 */

wimp_menu *menus_create_popup_menu(wimp_menu *menu, wimp_pointer *pointer);


/**
 * Set the ticked status of an entry in a menu block.
 *
 * \param *menu		The menu block to update.
 * \param entry		The entry to update.
 * \param tick		TRUE to tick the entry; FALSE to clear it.
 */

void menus_tick_entry(wimp_menu *menu, int entry, osbool tick);


/**
 * Set the shaded status of an entry in a menu block.
 *
 * \param *menu		The menu block to update.
 * \param entry		The entry to update.
 * \param shade		TRUE to shade the entry; FALSE to clear it.
 */

void menus_shade_entry(wimp_menu *menu, int entry, osbool shade);


/**
 * Return a pointer to the text of a menu entry, ignoring its indirected
 * status.
 *
 * \param *menu		The menu block to read.
 * \param entry		The entry to read.
 * \return		Pointer to the start of the entry text.
 */

char *menus_get_text_addr(wimp_menu *menu, int entry);


/**
 * Return a pointer to the text of a menu entry.
 *
 * \param *menu		The menu block to read.
 * \param entry		The entry to read.
 * \return		Pointer to the start of the entry text, or NULL if not
 *			indirected.
 */

char *menus_get_indirected_text_addr(wimp_menu *menu, int entry);


/**
 * Return the number of entries in a menu.
 *
 * \param *menu		The menu block to read.
 * \return		The number of entries in the menu.
 */

unsigned menus_get_entries(wimp_menu *menu);

#endif

