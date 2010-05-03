/* SF-Lib - Menus.h
 *
 * Version 0.10 (5 May 2003)
 */

#ifndef _SFLIB_MENUS
#define _SFLIB_MENUS

/* ================================================================================================================== */

void load_menus (char *filename, wimp_w *dbox_list, wimp_menu *menus[]);

wimp_menu *create_standard_menu (wimp_menu *menu, wimp_pointer *pointer);
wimp_menu *create_iconbar_menu (wimp_menu *menu, wimp_pointer *pointer, int entries, int lines);
wimp_menu *create_popup_menu (wimp_menu *menu, wimp_pointer *pointer);

void tick_menu_item (wimp_menu *menu, int item, int tick);
void shade_menu_item (wimp_menu *menu, int item, int shade);

char *menu_text (wimp_menu *menu, int item);
char *indirected_menu_text (wimp_menu *menu, int item);

#endif
