/* SF-Lib - Windows.h
 *
 * Version 0.10 (5 May 2003)
 */

#ifndef _SFLIB_WINDOWS
#define _SFLIB_WINDOWS

/* ================================================================================================================== */

#define sf_ICONBAR_HEIGHT 124          /* The height used for avoiding the iconbar on window opens. */
#define sf_WINDOW_GADGET_HEIGHT 40
#define sf_PANE_ICON_OFFSET 8          /* The inset used for placing a pane in a window icon. */

/* ================================================================================================================== */

void open_window (wimp_w window);
void open_window_nested (wimp_w window, wimp_w parent);
void open_window_nested_as_toolbar (wimp_w w, wimp_w parent, int height);
void open_window_nested_as_footer (wimp_w w, wimp_w parent, int height);

void open_window_centred_at_pointer (wimp_w w, wimp_pointer *p);
void open_window_centred_on_screen (wimp_w w);

void open_transient_window_centred_at_pointer (wimp_w w, wimp_pointer *p);

void open_pane_dialogue (wimp_w parent, wimp_w pane);
/* void open_pane_dialogue_centred_at_pointer (wimp_w parent, wimp_w pane, wimp_pointer *p); */
void open_pane_dialogue_centred_at_pointer (wimp_w parent, wimp_w pane, wimp_i i, int right_margin, wimp_pointer *p);
void open_pane_centred_in_icon (wimp_w parent_handle, wimp_w pane_handle, wimp_i i, int right_margin, wimp_w behind);

int window_is_open (wimp_w window);

char *indirected_window_title (wimp_w w);

void place_window_as_toolbar (wimp_window *parent, wimp_window *pane, int height);
void place_window_as_footer (wimp_window *parent, wimp_window *pane, int height);
void place_window_in_icon (wimp_window *parent, wimp_window *pane, int icon, int right_margin);

void force_visible_window_redraw (wimp_w w);

wimp_window *load_window_template (char *name);

#endif
