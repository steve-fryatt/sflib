/**
 * \file: windows.h
 *
 * SF-Lib - Windows.h
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * RISC OS Window support.  Support for handling windows, panes and
 * dialogue boxes.
 */

#ifndef SFLIB_WINDOWS
#define SFLIB_WINDOWS

#include <stdlib.h>


#define sf_ICONBAR_HEIGHT 124							/**< The height in OS units used for avoiding the iconbar on window opens.	*/
#define sf_WINDOW_GADGET_HEIGHT 40						/**< The height in OS units of standard window furniture.			*/
#define sf_PANE_ICON_OFFSET 8							/**< The inset in OS units used when placing a pane in a window icon.		*/


/**
 * Open a window at the top of the window stack.
 *
 * \param window	The window to open.
 */

void windows_open(wimp_w window);


/**
 * Open a window nested inside a parent window, positioned as already set
 * by the respective window states.
 *
 * \param w		The child, pane window to be opened.
 * \param p		The parent window, which should already be open.
 */

void windows_open_nested(wimp_w w, wimp_w p);


/**
 * Open a window nested inside a parent window, positioned as a footer
 * bar.
 *
 * \param w		The child, pane window to be opened.
 * \param p		The parent window, which should already be open.
 * \param height	The height of the pane window.
 */

void windows_open_nested_as_toolbar(wimp_w w, wimp_w p, int height);


/**
 * Open a window nested inside a parent window, positioned as a footer
 * bar.
 *
 * \param w		The child, pane window to be opened.
 * \param p		The parent window, which should already be open.
 * \param height	The height of the pane window.
 */

void windows_open_nested_as_footer(wimp_w w, wimp_w p, int height);


/**
 * Open a window centred on the given pointer position.
 *
 * \param w		The window to open.
 * \param *p		The current pointer position.
 */

void windows_open_centred_at_pointer(wimp_w w, wimp_pointer *p);


/**
 * Open a window centred in the current desktop screen mode.
 *
 * \param w		The window to open.
 */

void windows_open_centred_on_screen(wimp_w w);


/**
 * Open a window transiently centred on the given pointer position.
 *
 * \param w		The window to open.
 * \param *p		The current pointer position.
 */

void windows_open_transient_centred_at_pointer(wimp_w w, wimp_pointer *p);


/**
 * Open a window and pane with the parent window centred at the pointer.
 *
 * \param parent_handle		The parent window to open.
 * \param pane_handle		The pane window to open.
 * \param i			The icon handle in the parent to locate the pane within.
 * \param right_margin		The number of OS units to inset the right-hand margin.
 * \param *p			The current pointer details.
 */

void windows_open_with_pane_centred_at_pointer(wimp_w parent_handle, wimp_w pane_handle, wimp_i i, int right_margin, wimp_pointer *p);


/**
 * Open a pane centred on the specified icon in a parent window.
 *
 * \param parent_handle		The (already open) parent window.
 * \param pane_handle		The pane window to open.
 * \param i			The icon handle in the parent to locate the pane within.
 * \param right_margin		The number of OS units to inset the right-hand margin.
 * \param behind		The window to open the window pair behind.
 */

void windows_open_pane_centred_in_icon(wimp_w parent_handle, wimp_w pane_handle, wimp_i i, int right_margin, wimp_w behind);


/**
 * Discover whether a window is currently open on the desktop.
 *
 * \param w		The window to test.
 * \return 		TRUE if the window is open; else FALSE.
 */

osbool windows_get_open(wimp_w w);


/**
 * Return a pointer to a window's indirected title, or NULL if the title
 * isn't indirected.
 *
 * \param w		The window to process.
 * \return		Pointer to the indirected data, or NULL.
 */

char *windows_get_indirected_title_addr(wimp_w w);


/**
 * Return the length of a window's indirected title buffer, or -1 if the title
 * isn't indirected.
 *
 * \param w		The window to process.
 * \return		The length of the title buffer, or -1.
 */

size_t windows_get_indirected_title_length(wimp_w w);


/**
 * Place an window pane definition over a parent such that it is positioned
 * correctly for a toolbar.
 *
 * \param *parent	The parent window definition.
 * \param *pane		The pane window definition
 * \param height	The height of the toolbar, in OS units.
 */

void windows_place_as_toolbar(wimp_window *parent, wimp_window *pane, int height);


/**
 * Place an window pane definition over a parent such that it is positioned
 * correctly for a window footer bar.
 *
 * \param *parent	The parent window definition.
 * \param *pane		The pane window definition
 * \param height	The height of the footer bar, in OS units.
 */

void windows_place_as_footer(wimp_window *parent, wimp_window *pane, int height);


/**
 * Place an window pane definition over a parent window and icon such that it
 * is positioned correctly inside the icon.
 *
 * \param *parent	The parent window definition.
 * \param *pane		The pane window definition
 * \param icon		The icon to use as a guide.
 * \param right_margin	The right margin (for scrollbars), in OS units.
 */

void windows_place_in_icon(wimp_window *parent, wimp_window *pane, int icon, int right_margin);


/**
 * Force the redraw of the visible parts of a window on the next poll.
 *
 * \param w		The window to redraw.
 */

void windows_redraw(wimp_w w);


/**
 * Load a window template into memory from the currently open template file,
 * storing the details in a newly malloc()'d block.  The block should be released
 * after use with free() if no longer required.
 *
 * \param *name		The name of the template to load.
 * \return		Pointer to the window definition, or NULL on failure.
 */

wimp_window *windows_load_template(char *name);

#endif

