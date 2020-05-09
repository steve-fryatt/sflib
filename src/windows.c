/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: windows.c
 *
 * RISC OS Window support.  Support for handling windows, panes and
 * dialogue boxes.
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "general.h"
#include "msgs.h"
#include "string.h"
#include "windows.h"

/* ANSII C header files. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Open a window at the top of the window stack.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open(wimp_w w)
{
	wimp_window_state	window;

	window.w = w;
	wimp_get_window_state(&window);
	window.next = wimp_TOP;
	wimp_open_window((wimp_open *) &window);
}


/* Open a window nested inside a parent window, positioned as already set
 * by the respective window states.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_nested(wimp_w w, wimp_w p)
{
	wimp_window_state	window;

	window.w = w;
	wimp_get_window_state(&window);
	window.next = wimp_TOP;
	wimp_open_window_nested((wimp_open *) &window, p,
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_LS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_RS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_TS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_BS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_YORIGIN_SHIFT);
}


/* Open a window nested inside a parent window, positioned as a footer
 * bar.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_nested_as_toolbar(wimp_w w, wimp_w p, int height, osbool fixed)
{
	wimp_window_state		window, parent;
	wimp_window_nesting_flags	flags;

	parent.w = p;
	wimp_get_window_state(&parent);

	window.w = w;
	wimp_get_window_state(&window);

	window.visible.x0 = parent.visible.x0;
	window.visible.x1 = parent.visible.x1;
	window.visible.y0 = parent.visible.y1 - height;
	window.visible.y1 = parent.visible.y1;

	flags = wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_LS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT << wimp_CHILD_RS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT << wimp_CHILD_TS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT << wimp_CHILD_BS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT << wimp_CHILD_YORIGIN_SHIFT;

	if (fixed)
		flags |= (wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_XORIGIN_SHIFT);
	else
		flags |= (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT);

	window.next = wimp_TOP;
	wimp_open_window_nested((wimp_open *) &window, p, flags);
}


/* Open a window nested inside a parent window, positioned as a footer
 * bar.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_nested_as_footer(wimp_w w, wimp_w p, int height, osbool fixed)
{
	wimp_window_state	window, parent;
	wimp_window_nesting_flags	flags;

	parent.w = p;
	wimp_get_window_state(&parent);

	window.w = w;
	wimp_get_window_state(&window);

	window.visible.x0 = parent.visible.x0;
	window.visible.x1 = parent.visible.x1;
	window.visible.y0 = parent.visible.y0;
	window.visible.y1 = parent.visible.y0 + height;

	flags = wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_LS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT << wimp_CHILD_RS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_TS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_BS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_YORIGIN_SHIFT;

	if (fixed)
		flags |= (wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_XORIGIN_SHIFT);
	else
		flags |= (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT);

	window.next = wimp_TOP;
	wimp_open_window_nested((wimp_open *) &window, p, flags);
}


/* Open a window centred on the given pointer position.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_centred_at_pointer(wimp_w w, wimp_pointer *p)
{
	wimp_window_state	window;
	int			width, height;

	window.w = w;
	wimp_get_window_state(&window);

	if ((window.flags & wimp_WINDOW_OPEN) == 0) {
		width = window.visible.x1 - window.visible.x0;
		height = window.visible.y1 - window.visible.y0;

		window.visible.x0 = p->pos.x - (width / 2);
		window.visible.y0 = p->pos.y - (height / 2);

		if (window.visible.y0 < sf_ICONBAR_HEIGHT)
			window.visible.y0 = sf_ICONBAR_HEIGHT;

		window.visible.x1 = window.visible.x0 + width;
		window.visible.y1 = window.visible.y0 + height;
	}

	window.next = wimp_TOP;
	wimp_open_window((wimp_open *) &window);
}


/* Open a window centred in the current desktop screen mode.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_centred_on_screen(wimp_w w)
{
	wimp_window_state	window;
	int			width, height;

	window.w = w;
	wimp_get_window_state(&window);

	if ((window.flags & wimp_WINDOW_OPEN) == 0) {
		width = window.visible.x1 - window.visible.x0;
		height = window.visible.y1 - window.visible.y0;

		window.visible.x0 = (general_mode_width() - width) / 2;
		window.visible.y0 = (general_mode_height() - height) / 2;

		if (window.visible.y0 < sf_ICONBAR_HEIGHT)
			window.visible.y0 = sf_ICONBAR_HEIGHT;

		window.visible.x1 = window.visible.x0 + width;
		window.visible.y1 = window.visible.y0 + height;
	}

	window.next = wimp_TOP;
	wimp_open_window((wimp_open *) &window);
}

/* Open a window transiently centred on the given pointer position.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_transient_centred_at_pointer(wimp_w w, wimp_pointer *p)
{
	wimp_window_state	window;
	int			width, height;

	window.w = w;
	wimp_get_window_state(&window);
	width = window.visible.x1 - window.visible.x0;
	height = window.visible.y1 - window.visible.y0;

	wimp_create_menu((wimp_menu *) w, p->pos.x - width/2, p->pos.y + height/2);
}


/* Open a window and pane with the parent window centred at the pointer.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_with_pane_centred_at_pointer(wimp_w parent_handle, wimp_w pane_handle, wimp_i i,
		int right_margin, wimp_pointer *p)
{
	wimp_window_state	parent;
	int			width, height;

	/* Get the size of the parent window and centre it at the pointer. */

	parent.w = parent_handle;
	wimp_get_window_state(&parent);

	if ((parent.flags & wimp_WINDOW_OPEN) == 0) {
		width = parent.visible.x1 - parent.visible.x0;
		height = parent.visible.y1 - parent.visible.y0;

		parent.visible.x0 = p->pos.x - (width / 2);
		parent.visible.y0 = p->pos.y - (height / 2);

		if (parent.visible.y0 < sf_ICONBAR_HEIGHT)
			parent.visible.y0 = sf_ICONBAR_HEIGHT;

		parent.visible.x1 = parent.visible.x0 + width;
		parent.visible.y1 = parent.visible.y0 + height;
	}

	parent.next = wimp_TOP;
	wimp_open_window((wimp_open *) &parent);

	/* Open the pane in the icon. */

	windows_open_pane_centred_in_icon(parent_handle, pane_handle, i, right_margin, wimp_TOP);
}


/* Open a pane centred on the specified icon in a parent window.
 *
 * This is an external interface, documented in windows.h
 */

void windows_open_pane_centred_in_icon(wimp_w parent_handle, wimp_w pane_handle, wimp_i i, int right_margin, wimp_w behind)
{
	wimp_window_state	parent, pane;
	wimp_icon_state		icon;

	/* Get the size of the parent window and the position of the icon. */

	parent.w = parent_handle;
	wimp_get_window_state(&parent);

	icon.w = parent_handle;
	icon.i = i;
	wimp_get_icon_state(&icon);

	/* Get the pane details and place it in the icon. */

	pane.w = pane_handle;
	wimp_get_window_state(&pane);

	pane.visible.x0 = parent.visible.x0 - parent.xscroll + icon.icon.extent.x0 + sf_PANE_ICON_OFFSET;
	pane.visible.x1 = parent.visible.x0 - parent.xscroll + icon.icon.extent.x1 - sf_PANE_ICON_OFFSET - right_margin;
	pane.visible.y0 = parent.visible.y1 - parent.yscroll + icon.icon.extent.y0 + sf_PANE_ICON_OFFSET;
	pane.visible.y1 = parent.visible.y1 - parent.yscroll + icon.icon.extent.y1 - sf_PANE_ICON_OFFSET;

	pane.next = behind;
	wimp_open_window_nested((wimp_open *) &pane, parent_handle,
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_LS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_RS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_TS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_BS_EDGE_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT |
			wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_YORIGIN_SHIFT);
}


/* Discover whether a window is currently open on the desktop.
 *
 * This is an external interface, documented in windows.h
 */

osbool windows_get_open(wimp_w w)
{
	wimp_window_state	window;

	window.w = w;
	wimp_get_window_state(&window);

	return ((window.flags & wimp_WINDOW_OPEN) != 0) ? TRUE : FALSE;
}


/* Return a pointer to a window's indirected title, or NULL if the title
 * isn't indirected.
 *
 * This is an external interface, documented in windows.h
 */

char *windows_get_indirected_title_addr(wimp_w w)
{
	wimp_window_info	window;

	window.w = w;
	wimp_get_window_info_header_only(&window);

	if (window.title_flags & wimp_ICON_INDIRECTED)
		return window.title_data.indirected_text.text;
	else
		return NULL;
}


/* Return the length of a window's indirected title buffer, or -1 if the title
 * isn't indirected.
 *
 * This is an external interface, documented in windows.h
 */

size_t windows_get_indirected_title_length(wimp_w w)
{
	wimp_window_info	window;

	window.w = w;
	wimp_get_window_info_header_only(&window);

	if (window.title_flags & wimp_ICON_INDIRECTED)
		return window.title_data.indirected_text.size;
	else
		return -1;
}


/* Perform an sprintf() into a window title, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * This is an external interface, documented in windows.h
 */

int windows_title_printf(wimp_w w, char *cntrl_string, ...)
{
	int			ret = 0;
	va_list			ap;
	wimp_window_info	window;
	os_error		*error;

	window.w = w;
	error = xwimp_get_window_info_header_only(&window);
	if (error != NULL)
		return 0;

	if ((window.title_flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) !=
			(wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return 0;

	if (window.title_data.indirected_text.text == NULL ||
			window.title_data.indirected_text.size <= 0)
		return 0;

	va_start(ap, cntrl_string);
	ret = vsnprintf(window.title_data.indirected_text.text,
				window.title_data.indirected_text.size,
				cntrl_string, ap);

	window.title_data.indirected_text.text[window.title_data.indirected_text.size - 1] = '\0';

	return ret;
}


/* Perform a strncpy() into a window title, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * This is an external interface, documented in windows.h
 */

char *windows_title_strncpy(wimp_w w, char *s)
{
	wimp_window_info	window;
	os_error		*error;

	window.w = w;
	error = xwimp_get_window_info_header_only(&window);
	if (error != NULL)
		return NULL;

	if ((window.title_flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) !=
			(wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return NULL;

	if (window.title_data.indirected_text.text == NULL ||
			window.title_data.indirected_text.size <= 0)
		return NULL;

	string_copy(window.title_data.indirected_text.text, s,
				window.title_data.indirected_text.size);

	return window.title_data.indirected_text.text;
}


/* Perform a MessageTrans lookup into a window title, assuming that it is indirected.
 *
 * This is an external interface, documented in windows.h
 */

char *windows_title_msgs_lookup(wimp_w w, char *token)
{
	return windows_title_msgs_param_lookup(w, token, NULL, NULL, NULL, NULL);
}


/* Perform a MessageTrans lookup into a window title, assuming that it is indirected,
 * substituting the supplied parameters.
 *
 * This is an external interface, documented in windows.h
 */

char *windows_title_msgs_param_lookup(wimp_w w, char *token, char *a, char *b, char *c, char *d)
{
	wimp_window_info	window;
	os_error		*error;

	window.w = w;
	error = xwimp_get_window_info_header_only(&window);
	if (error != NULL)
		return NULL;

	if ((window.title_flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) !=
			(wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return NULL;

	return msgs_param_lookup(token, window.title_data.indirected_text.text, window.title_data.indirected_text.size, a, b, c, d);
}


/* Place an window pane definition over a parent such that it is positioned
 * correctly for a toolbar.
 *
 * This is an external interface, documented in windows.h
 */

void windows_place_as_toolbar(wimp_window *parent, wimp_window *pane, int height)
{
	pane->visible.x0 = parent->visible.x0;
	pane->visible.x1 = parent->visible.x1;
	pane->visible.y0 = parent->visible.y1 - height;
	pane->visible.y1 = parent->visible.y1;

	pane->extent.x0 = parent->extent.x0;
	pane->extent.x1 = parent->extent.x1;
	pane->extent.y0 = parent->extent.y1 - height;
	pane->extent.y1 = parent->extent.y1;
}


/* Place an window pane definition over a parent such that it is positioned
 * correctly for a window footer bar.
 *
 * This is an external interface, documented in windows.h
 */

void windows_place_as_footer(wimp_window *parent, wimp_window *pane, int height)
{
	pane->visible.x0 = parent->visible.x0;
	pane->visible.x1 = parent->visible.x1;
	pane->visible.y0 = parent->visible.y0;
	pane->visible.y1 = parent->visible.y0 + height;

	pane->extent.x0 = parent->extent.x0;
	pane->extent.x1 = parent->extent.x1;
	pane->extent.y0 = parent->extent.y1 - height;
	pane->extent.y1 = parent->extent.y1;
}


/* Place an window pane definition over a parent window and icon such that it
 * is positioned correctly inside the icon.
 *
 * This is an external interface, documented in windows.h
 */

void windows_place_in_icon(wimp_window *parent, wimp_window *pane, int icon, int right_margin)
{
	pane->visible.x0 = parent->visible.x0 - parent->xscroll + parent->icons[icon].extent.x0 + sf_PANE_ICON_OFFSET;
	pane->visible.x1 = parent->visible.x0 - parent->xscroll + parent->icons[icon].extent.x1 - sf_PANE_ICON_OFFSET
			- right_margin;
	pane->visible.y0 = parent->visible.y1 - parent->yscroll + parent->icons[icon].extent.y0 + sf_PANE_ICON_OFFSET;
	pane->visible.y1 = parent->visible.y1 - parent->yscroll + parent->icons[icon].extent.y1 - sf_PANE_ICON_OFFSET;
}


/* Force the redraw of the visible parts of a window on the next poll.
 *
 * This is an external interface, documented in windows.h
 */

void windows_redraw(wimp_w w)
{
	wimp_window_state	window;

	window.w = w;
	wimp_get_window_state(&window);
	wimp_force_redraw (w, window.xscroll, window.yscroll - (window.visible.y1 - window.visible.y0),
			window.xscroll + (window.visible.x1 - window.visible.x0), window.yscroll);
}


/* Load a window template into memory from the currently open template file,
 * storing the details in a newly malloc()'d block.  The block should be released
 * after use with free() if no longer required.
 * 
 * *name is assumed to point to 12 bytes of word-aligned memory if it contains
 * a wildcarded name. If it does not contain a wildcard, it can simply be a
 * pointer to a string, based on the current Wimp implementation.
 *
 * This is an external interface, documented in windows.h
 */

wimp_window *windows_load_template(char *name)
{
	wimp_window	*window_def = NULL;
	byte		*ind_data = NULL;
	int		def_size, ind_size, context = 0;
	os_error	*error;

	error = xwimp_load_template(wimp_GET_SIZE, 0, 0, wimp_NO_FONTS, name, 0, &def_size, &ind_size, &context);
	if (error != NULL || context == 0)
		return NULL;

	window_def = malloc(def_size);
	ind_data = malloc(ind_size);

	if (window_def == NULL || (ind_size > 0 && ind_data == NULL)) {
		if (window_def != NULL)
			free(window_def);
		if (ind_data != NULL)
			free(ind_data);

		return NULL;
	}

	error = xwimp_load_template(window_def, (char *) ind_data, (char const *) ind_data+ind_size, wimp_NO_FONTS, name, 0, NULL, NULL, &context);
	if (error != NULL || context == 0) {
		if (window_def != NULL)
			free(window_def);
		if (ind_data != NULL)
			free(ind_data);

		return NULL;
	}

	return window_def;
}
