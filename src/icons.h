/**
 * \file: icons.h
 *
 * SF-Lib - Icons.h
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * RISC OS Wimp Icon Support.  Support for manipulating icons and their
 * contents.
 */

#ifndef SFLIB_ICONS
#define SFLIB_ICONS

#include <stddef.h>


/**
 * Copy the text of an icon (indirected or otherwise) into the supplied buffer.
 *
 * No attempt is made to spot buffer overruns.
 *
 * \param w		The handle of the window containing the icon.
 * \param i		The handle of the icon to be read.
 * \param *buffer	A buffer to contain the icon text.
 * \return		A pointer to the start of the returned text.
 */

char *icons_copy_text(wimp_w w, wimp_i i, char *buffer);


/**
 * Return a pointer to the indirected text buffer of an icon.
 *
 * \param w		The handle of the window containing the icon.
 * \param i		The handle of the icon to read.
 * \return		The start of the icon's indirected buffer.
 */

char *icons_get_indirected_text_addr(wimp_w w, wimp_i i);


/**
 * Return a pointer to the validation string buffer of an icon.
 *
 * \param w		The handle of the window containing the icon.
 * \param i		The handle of the icon to read.
 * \return		The start of the icon's validation string buffer.
 */

char *icons_get_validation_addr(wimp_w w, wimp_i i);


/**
 * Return the given length of the indirected text buffer of an icon.
 *
 * \param w		The handle of the window containing the icon.
 * \param i		The handle of the icon to read.
 * \return		The size of the icon's validation string buffer, or -1.
 */

size_t icons_get_indirected_text_length(wimp_w w, wimp_i i);


/**
 * Return the part of an icon's validation string corresponding to the
 * supplied 'command' character.
 *
 * No attempt is made to spot and prevent buffer overruns.
 *
 * \param *buffer	Pointer to a buffer to take the returned text.
 * \param w		The handle of the window containing the icon.
 * \param i		The handle of the icon to read.
 * \param command	The single character validation command to return.
 * \return		TRUE if the command code was found; else FALSE.
 */

osbool icons_get_validation_command(char *buffer, wimp_w w, wimp_i i, char command);


/**
 * Perform an sprintf() into an icon, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param *cntrl_string	The format string for printf.
 * \param ...		Any remaining parameters.
 * \return		The number of characters printed.
 */

int icons_printf(wimp_w w, wimp_i i, char *cntrl_string, ...);


/**
 * Perform a strncpy() into an icon, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param *s		The string to be copied.
 * \return		A pointer to the copied string.
 */

char *icons_strncpy(wimp_w w, wimp_i i, char *s);


/**
 * Perform a MessageTrans lookup into an icon, assuming that it is indirected.
 *
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param *token	The message token to look up.
 * \return		A pointer to the result.
 */

char *icons_msgs_lookup(wimp_w w, wimp_i i, char *token);

/**
 * Perform a MessageTrans lookup into an icon, assuming that it is indirected,
 * substituting the supplied parameters.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param *token	The message token to look up.
 * \param *a		Parameter for %0.
 * \param *b		Parameter for %1.
 * \param *c		Parameter for %2.
 * \param *d		Parameter for %3.
 * \return		A pointer to the result.
 */

char *icons_msgs_param_lookup(wimp_w w, wimp_i i, char *token, char *a, char *b, char *c, char *d);


/**
 * Change the selected state of an icon.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param selected	TRUE to select the icon; FALSE to clear it.
 */

void icons_set_selected(wimp_w w, wimp_i i, osbool selected);


/**
 * Change the shaded state of an icon.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param shaded	TRUE to shade the icon; FALSE to clear it.
 */

void icons_set_shaded(wimp_w w, wimp_i i, osbool shaded);


/**
 * Change the deleted state of an icon.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \param deleted	TRUE to delete the icon; FALSE to restore it.
 */

void icons_set_deleted(wimp_w w, wimp_i i, osbool deleted);


/**
 * Read the selected state of an icon.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \return		TRUE if the icon is selected; else FALSE.
 */

osbool icons_get_selected(wimp_w w, wimp_i i);


/**
 * Read the shaded state of an icon.
 *
 * \param w		The handle of the windown containing the icon.
 * \param i		The target icon handle.
 * \return		TRUE if the icon is shaded; else FALSE.
 */

osbool icons_get_shaded(wimp_w w, wimp_i i);


/**
 * Change the shaded state of a group of icons.
 *
 * \param window	The window containing the icons.
 * \param shaded	TRUE to shade the icons; FALSE to unshade them.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons.
 */

void icons_set_group_shaded(wimp_w window, osbool shaded, int icons, ...);


/**
 * Change the deleted state of a group of icons, depending upon the state
 * of another icon.  If the icon is deselected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * \param window	The window containing the icons.
 * \param icon		The icon whose selected state controls the deletion.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_set_group_deleted_when_off(wimp_w window, wimp_i icon, int icons, ...);


/**
 * Change the deleted state of a group of icons, depending upon the state
 * of another icon.  If the icon is selected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * \param window	The window containing the icons.
 * \param icon		The icon whose selected state controls the deletion.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_set_group_deleted_when_on(wimp_w window, wimp_i icon, int icons, ...);


/**
 * Change the shaded state of a group of icons, depending upon the state
 * of another icon.  If the icon is deselected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * \param window	The window containing the icons.
 * \param icon		The icon whose selected state controls the shading.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_set_group_shaded_when_off(wimp_w window, wimp_i icon, int icons, ...);


/**
 * Change the shaded state of a group of icons, depending upon the state
 * of another icon.  If the icon is selected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * \param window	The window containing the icons.
 * \param icon		The icon whose selected state controls the shading.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_set_group_shaded_when_on(wimp_w window, wimp_i icon, int icons, ...);


/**
 * Change the selected state of a group of icons (usually radio icons in the
 * same ESG) so that one icon in the group is selected and the rest are not.
 *
 * \param window	The window containing the icons.
 * \param selected	The index into the group of the icon to be selected.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_set_radio_group_selected(wimp_w window, int selected, int icons, ...);


/**
 * Read the selected state of a group of icons (usually radio icons in the
 * same ESG) returning the index into the group of the selected icon, or
 * -1 if none were selected.
 *
 * \param window	The window containing the icons.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 * \return		The inxed into the group of the selected icon, or -1.
 */

int icons_get_radio_group_selected(wimp_w window, int icons, ...);


/**
 * Force the redraw of a group of icons.
 *
 * \param window	The window containing the icons.
 * \param icons		The number of icons in the group.
 * \param ...		The icon handles for each of the icons in the group.
 */

void icons_redraw_group(wimp_w window, int icons, ...);


/**
 * Place the caret at the end of the specified icon (including the window
 * background if icon is wimp_ICON_WINDOW).
 *
 * \param window	The window to take the caret.
 * \param icon		The icon to take the caret.
 */

void icons_put_caret_at_end(wimp_w window, wimp_i icon);


/**
 * Try to place the caret at the end of a sequence of writable icons, using the
 * first not to be shaded or deleted.  If all are shaded, place the caret into
 * the work area of the parent window.
 *
 * \param window	The window to take the caret.
 * \param icons		The number if icons in the group.
 * \param ...		The handles for the icons to take the caret, in
 *			decreasing order of priority.
 */

void icons_put_caret_in_group(wimp_w window, int icons, ...);


/**
 * If the caret is in the given window, update its location to take into
 * account any changes in icon contents and shaded status.
 *
 * \param window	The window to test.
 */

void icons_replace_caret_in_window(wimp_w window);


/**
 * Insert text into an icon, updating the caret position if applicable.
 *
 * \param w		The window containing the icon.
 * \param i		The icon to insert text into.
 * \param index		The index into the icon text of the insertion point.
 * \param *text		The text to insert.
 * \param n		The number of characters to insert.
 */

void icons_insert_text(wimp_w w, wimp_i i, int index, char *text, int n);

#endif

