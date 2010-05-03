/* SF-Lib - Icons.h
 *
 * Version 0.10 (5 May 2003)
 */

#ifndef _SFLIB_ICONS
#define _SFLIB_ICONS

/* ================================================================================================================== */

char *icon_text (wimp_w w, wimp_i i, char *buffer);
char *indirected_icon_text (wimp_w w, wimp_i i);
char *indirected_icon_validation (wimp_w w, wimp_i i);

int get_validation_command (char *buffer, wimp_w w, wimp_i i, char command);

void set_icon_selected (wimp_w w, wimp_i i, int selected);
void set_icon_shaded (wimp_w w, wimp_i i, int shaded);
void set_icon_deleted (wimp_w w, wimp_i i, int deleted);

int read_icon_selected (wimp_w w, wimp_i i);
int read_icon_shaded (wimp_w w, wimp_i i);

void set_icons_shaded (wimp_w window, int shaded, int icons, ...);

void set_radio_icon_group_selected (wimp_w window, int selected, int icons, ...);
int read_radio_icon_group_selected (wimp_w window, int icons, ...);

void set_icons_shaded_when_radio_off (wimp_w window, wimp_i icon, int icons, ...);
void set_icons_shaded_when_radio_on (wimp_w window, wimp_i icon, int icons, ...);

void put_caret_at_end (wimp_w window, wimp_i icon);
void replace_caret_in_window (wimp_w window);

void redraw_icons_in_window (wimp_w window, int icons, ...);

void insert_text_into_icon (wimp_w w, wimp_i i, int index, char *text, int n);

#endif
