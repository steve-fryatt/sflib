/* Copyright 2003-2017, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: icons.c
 *
 * RISC OS Wimp Icon Support.  Support for manipulating icons and their
 * contents.
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "icons.h"
#include "string.h"
#include "debug.h"
#include "msgs.h"
#include "string.h"

/* ANSII C header files. */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


/* Copy the text of an icon (indirected or otherwise) into the supplied buffer.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_copy_text(wimp_w w, wimp_i i, char *buffer, size_t length)
{
	wimp_icon_state		icon;

	if (buffer == NULL || length == 0)
		return NULL;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	if (icon.icon.flags & wimp_ICON_INDIRECTED) {
		string_ctrl_strncpy(buffer, icon.icon.data.indirected_text.text, length - 1);
		buffer[length - 1] = '\0';
	} else if (length > 12) {
		string_ctrl_strncpy(buffer, icon.icon.data.text, 12);
		buffer[12] = '\0';
	} else {
		string_ctrl_strncpy(buffer, icon.icon.data.text, length - 1);
		buffer[length - 1] = '\0';
	}

	return buffer;
}


/* Return a pointer to the indirected text buffer of an icon.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_get_indirected_text_addr(wimp_w w, wimp_i i)
{
	wimp_icon_state		icon;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	if (icon.icon.flags & wimp_ICON_INDIRECTED)
		return icon.icon.data.indirected_text.text;
	else
		return NULL;
}


/* Return a pointer to the validation string buffer of an icon.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_get_validation_addr(wimp_w w, wimp_i i)
{
	wimp_icon_state		icon;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	if ((icon.icon.flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) == (wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return icon.icon.data.indirected_text.validation;
	else
		return NULL;
}


/* Return the given length of the indirected text buffer of an icon.
 *
 * This is an external interface, documented in icons.h
 */

size_t icons_get_indirected_text_length(wimp_w w, wimp_i i)
{
	wimp_icon_state		icon;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	if ((icon.icon.flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) == (wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return icon.icon.data.indirected_text.size;
	else
		return -1;
}


/* Return the part of an icon's validation string corresponding to the
 * supplied 'command' character.
 *
 * This is an external interface, documented in icons.h
 */

osbool icons_get_validation_command(char *buffer, size_t length, wimp_w w, wimp_i i, char command)
{
	char		*validation;

	validation = icons_get_validation_addr(w, i);

	if (validation == NULL)
		return FALSE;

	return icons_extract_validation_command(buffer, length, validation, command);
}


/* Extract a 'command' from an icon validation string.
 * 
 * This is an external interface, documented in icons.h
 */ 

osbool icons_extract_validation_command(char *buffer, size_t length, char *validation, char command)
{
	size_t		len;
	char		*copy, *part;
	osbool		found = FALSE;

	if (buffer == NULL || length == 0)
		return FALSE;

	*buffer = '\0';
	command = toupper(command);

	len = strlen(validation) + 1;
	copy = malloc(len);

	if (copy == NULL)
		return FALSE;

	string_ctrl_strncpy(copy, validation, len);
	part = strtok(copy, ";");

	while (part != NULL) {
		if (toupper(*part) == command) {
			string_ctrl_strncpy(buffer, part + 1, length);
			found = TRUE;
		}

		part = strtok(NULL, ";");
	}

	free(copy);

	buffer[length - 1] = '\0';

	return found;
}


/* Perform an sprintf() into an icon, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * This is an external interface, documented in icons.h
 */

int icons_printf(wimp_w w, wimp_i i, char *cntrl_string, ...)
{
	int			ret = 0;
	va_list			ap;
	wimp_icon_state		icon;
	os_error		*error;

	icon.w = w;
	icon.i = i;
	error = xwimp_get_icon_state(&icon);
	if (error != NULL)
		return 0;

	if ((icon.icon.flags & wimp_ICON_INDIRECTED) != wimp_ICON_INDIRECTED)
		return 0;

	if (((icon.icon.flags & (wimp_ICON_TEXT | wimp_ICON_SPRITE)) == wimp_ICON_SPRITE) &&
			(icon.icon.data.indirected_sprite.size == 0))
		return 0;

	va_start(ap, cntrl_string);
	ret = vsnprintf(icon.icon.data.indirected_text.text,
				icon.icon.data.indirected_text.size,
				cntrl_string, ap);

	icon.icon.data.indirected_text.text[icon.icon.data.indirected_text.size - 1] = '\0';

	return ret;
}


/* Perform a strncpy() into an icon, assuming that it is indirected.  The
 * icon details are trusted, including buffer length.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_strncpy(wimp_w w, wimp_i i, char *s)
{
	wimp_icon_state		icon;
	os_error		*error;

	icon.w = w;
	icon.i = i;
	error = xwimp_get_icon_state(&icon);
	if (error != NULL)
		return NULL;

	if ((icon.icon.flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) !=
			(wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return NULL;

	strncpy(icon.icon.data.indirected_text.text, s,
				icon.icon.data.indirected_text.size);

	return icon.icon.data.indirected_text.text;
}



/**
 * Perform a MessageTrans lookup into an icon, assuming that it is indirected.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_msgs_lookup(wimp_w w, wimp_i i, char *token)
{
	return icons_msgs_param_lookup(w, i, token, NULL, NULL, NULL, NULL);
}


/**
 * Perform a MessageTrans lookup into an icon, assuming that it is indirected,
 * substituting the supplied parameters.
 *
 * This is an external interface, documented in icons.h
 */

char *icons_msgs_param_lookup(wimp_w w, wimp_i i, char *token, char *a, char *b, char *c, char *d)
{
	wimp_icon_state		icon;
	os_error		*error;

	icon.w = w;
	icon.i = i;
	error = xwimp_get_icon_state(&icon);
	if (error != NULL)
		return NULL;

	if ((icon.icon.flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) !=
			(wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
		return NULL;

	return msgs_param_lookup(token, icon.icon.data.indirected_text.text, icon.icon.data.indirected_text.size, a, b, c, d);
}


/* Change the selected state of an icon.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_selected(wimp_w w, wimp_i i, osbool selected)
{
	wimp_set_icon_state(w, i, (selected) ? wimp_ICON_SELECTED : 0, wimp_ICON_SELECTED);
}


/* Change the shaded state of an icon.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_shaded(wimp_w w, wimp_i i, osbool shaded)
{
	wimp_set_icon_state(w, i, (shaded) ? wimp_ICON_SHADED : 0, wimp_ICON_SHADED);
}


/* Change the deleted state of an icon.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_deleted(wimp_w w, wimp_i i, osbool deleted)
{
	wimp_set_icon_state(w, i, (deleted) ? wimp_ICON_DELETED : 0, wimp_ICON_DELETED);
}


/* Read the selected state of an icon.
 *
 * This is an external interface, documented in icons.h
 */

osbool icons_get_selected(wimp_w w, wimp_i i)
{
	wimp_icon_state		icon;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	return ((icon.icon.flags & wimp_ICON_SELECTED) != 0) ? TRUE : FALSE;
}


/* Read the shaded state of an icon.
 *
 * This is an external interface, documented in icons.h
 */

osbool icons_get_shaded(wimp_w w, wimp_i i)
{
	wimp_icon_state		icon;

	icon.w = w;
	icon.i = i;
	wimp_get_icon_state(&icon);

	return ((icon.icon.flags & wimp_ICON_SHADED) != 0) ? TRUE : FALSE;
}


/* Change the shaded state of a group of icons.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_group_shaded(wimp_w window, osbool shaded, int icons, ...)
{
	int		i;
	va_list		ap;

	va_start(ap, icons);

	for (i=0; i<icons; i++)
		icons_set_shaded(window, va_arg(ap, wimp_i), shaded);

	va_end(ap);
}


/* Change the deleted state of a group of icons, depending upon the state
 * of another icon.  If the icon is deselected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_group_deleted_when_off(wimp_w window, wimp_i icon, int icons, ...)
{
	int		i, state;
	va_list		ap;

	va_start(ap, icons);

	state = !icons_get_selected(window, icon);

	for (i=0; i<icons; i++)
		icons_set_deleted(window, va_arg(ap, wimp_i), state);

	va_end(ap);
}


/* Change the deleted state of a group of icons, depending upon the state
 * of another icon.  If the icon is selected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_group_deleted_when_on(wimp_w window, wimp_i icon, int icons, ...)
{
	int		i, state;
	va_list	ap;

	va_start(ap, icons);

	state = icons_get_selected(window, icon);

	for (i=0; i<icons; i++)
		icons_set_deleted(window, va_arg(ap, wimp_i), state);

	va_end(ap);
}



/* Change the shaded state of a group of icons, depending upon the state
 * of another icon.  If the icon is deselected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_group_shaded_when_off(wimp_w window, wimp_i icon, int icons, ...)
{
	int		i, state;
	va_list		ap;

	va_start(ap, icons);

	state = !icons_get_selected(window, icon);

	for (i=0; i<icons; i++)
		icons_set_shaded(window, va_arg(ap, wimp_i), state);

	va_end(ap);
}


/* Change the shaded state of a group of icons, depending upon the state
 * of another icon.  If the icon is selected, then the icons in the
 * group will be shaded, and vice-versa.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_group_shaded_when_on(wimp_w window, wimp_i icon, int icons, ...)
{
	int		i, state;
	va_list	ap;

	va_start(ap, icons);

	state = icons_get_selected(window, icon);

	for (i=0; i<icons; i++)
		icons_set_shaded(window, va_arg(ap, wimp_i), state);

	va_end(ap);
}


/* Change the selected state of a group of icons (usually radio icons in the
 * same ESG) so that one icon in the group is selected and the rest are not.
 *
 * This is an external interface, documented in icons.h
 */

void icons_set_radio_group_selected(wimp_w window, int selected, int icons, ...)
{
	int		i;
	va_list		ap;

	va_start(ap, icons);

	for (i=0; i<icons; i++)
		icons_set_selected(window, va_arg(ap, wimp_i), selected == i);

	va_end(ap);
}


/* Read the selected state of a group of icons (usually radio icons in the
 * same ESG) returning the index into the group of the selected icon, or
 * -1 if none were selected.
 *
 * This is an external interface, documented in icons.h
 */

int icons_get_radio_group_selected(wimp_w window, int icons, ...)
{
	int		i, selected = -1;
	va_list	ap;

	va_start(ap, icons);

	for (i=0; i<icons; i++)
		if (icons_get_selected(window, va_arg(ap, wimp_i)))
			selected = i;

	va_end(ap);

	return selected;
}


/* Force the redraw of a group of icons.
 *
 * This is an external interface, documented in icons.h
 */

void icons_redraw_group(wimp_w w, int icons, ...)
{
	int		i;
	va_list		ap;

	va_start(ap, icons);

	for (i=0; i<icons; i++)
		wimp_set_icon_state(w, va_arg(ap, wimp_i), 0, 0);

	va_end(ap);
}


/**
 * Place the caret at the end of the specified icon (including the window
 * background if icon is wimp_ICON_WINDOW).
 *
 * This is an external interface, documented in icons.h
 */

void icons_put_caret_at_end(wimp_w window, wimp_i icon)
{
	int			index = 0;
	wimp_icon_state		icon_state;

	if (icon != wimp_ICON_WINDOW) {
		icon_state.w = window;
		icon_state.i = icon;
		wimp_get_icon_state(&icon_state);

		index = string_ctrl_strlen(icon_state.icon.data.indirected_text.text);
	}

	wimp_set_caret_position(window, icon, 0, 0, -1, index);
}


/**
 * Try to place the caret at the end of a sequence of writable icons, using the
 * first not to be shaded or deleted.  If all are shaded, place the caret into
 * the work area of the parent window.
 *
 * This is an external interface, documented in icons.h
 */

void icons_put_caret_in_group(wimp_w window, int icons, ...)
{
	int			i = 0;
	wimp_i			icon;
	wimp_icon_state		state;
	va_list			ap;


	va_start(ap, icons);

	state.w = window;

	while (i < icons && i != -1) {
		i++;

		icon = va_arg(ap, wimp_i);

		state.i = icon;
		wimp_get_icon_state(&state);

		if ((state.icon.flags & (wimp_ICON_SHADED | wimp_ICON_DELETED)) == 0) {
			icons_put_caret_at_end(window, icon);
			i = -1;
		}
	}

	if (i != -1)
		icons_put_caret_at_end(window, wimp_ICON_WINDOW);

	va_end(ap);
}


/**
 * If the caret is in the given window, update its location to take into
 * account any changes in icon contents and shaded or deleted status.
 *
 * This is an external interface, documented in icons.h
 */

void icons_replace_caret_in_window(wimp_w window)
{
	int			i, old_icon, new_icon, type;
	wimp_caret		caret;
	wimp_icon_state		state;
	wimp_window_info	*info, *new;

	wimp_get_caret_position(&caret);

	if (caret.w != window || caret.i == wimp_ICON_WINDOW)
		return;

	state.w = window;
	state.i = caret.i;
	wimp_get_icon_state(&state);

	if ((state.icon.flags & (wimp_ICON_SHADED | wimp_ICON_DELETED)) != 0) {
		/* If the icon where the caret is located is now shaded or deleted, it needs to be moved.  To do this,
		 * we need to know where the other writable icons are.  This is done by getting the window definition
		 * and scanning through the icons looking for a writable one to place the caret into.  The numerically
		 * nearest icon to the original is used.
		 */

		/* First, claim some memory to get the definition into.  This is big enough for the header only info
		 * (to find out how many icons there are) and may also do for the full definition.
		 */

		info = (wimp_window_info *) malloc(4096);

		if (info != NULL) {
			/* We got some memory, so get the window definitin header and check the number of icons. */

			info->w = window;
			wimp_get_window_info_header_only(info);

			if ((88 + 32 * info->icon_count) > sizeof (info)) {
				/* If there are too many icons to fit out block, expand it.  If that fails, the original block is freed
				 * and the pointer set to NULL.
				 */

				new = (wimp_window_info *) realloc(info, (88 + 32 * info->icon_count));

				if (new == NULL)
					free(info);
				info = new;
			}

			if (info != NULL) {
				/* we've got enough memory to take the full window definition, so get that. */

				info->w = window;
				wimp_get_window_info(info);

				old_icon = caret.i;
				new_icon = -1;

				/* For each icon, see if it's writable and not shaded or deleted.  If so, check if it's closer to
				 * the original icon than the current 'new_icon'; if so, update the new icon.
				 */

				for (i=0; i<info->icon_count; i++) {
					type = ((info->icons[i].flags & wimp_ICON_BUTTON_TYPE) >> wimp_ICON_BUTTON_TYPE_SHIFT);

					if ((type == wimp_BUTTON_WRITE_CLICK_DRAG || type == wimp_BUTTON_WRITABLE) &&
							((info->icons[i].flags & (wimp_ICON_SHADED | wimp_ICON_DELETED)) == 0)) {
						if ((abs (old_icon - i) < abs (old_icon - new_icon)) || new_icon == -1)
							new_icon = i;
					}
				}

				/* If we found a suitable new icon, put the caret at the end; if not, dump it into the work area. */

				if (new_icon != -1)
					icons_put_caret_at_end(caret.w, new_icon);
				else
					icons_put_caret_at_end(caret.w, wimp_ICON_WINDOW);

				free(info);
			} else {
				/* If the block expansion failed, fall back to dumping the caret into the workspace. */

				icons_put_caret_at_end(caret.w, wimp_ICON_WINDOW);
			}
		} else {
			/* If the initial memory allocation failed, fall back to dumping the caret into the workspace. */

			icons_put_caret_at_end(caret.w, wimp_ICON_WINDOW);
		}
	} else {
		icons_put_caret_at_end(caret.w, caret.i);
	}
}


/* Insert text into an icon, updating the caret position if applicable.
 *
 * This is an external interface, documented in icons.h
 */

void icons_insert_text(wimp_w w, wimp_i i, int index, char *text, int n)
{
	wimp_icon_state		icon_state;
	wimp_caret		caret;
	int			len, copy, j;

	wimp_get_caret_position(&caret);

	icon_state.w = w;
	icon_state.i = i;
	wimp_get_icon_state(&icon_state);

	/* Calculate the length of the current icon text (including terminator), the amount of free space and hence the
	 * amount of the new text to be copied in.
	 */

	len = string_ctrl_strlen(icon_state.icon.data.indirected_text.text) + 1;
	copy = icon_state.icon.data.indirected_text.size - len;
	if (copy > n)
		copy = n;

	/* Move up the text following the insertion point. */

	if (index < len)
		memmove(icon_state.icon.data.indirected_text.text+index+copy, icon_state.icon.data.indirected_text.text+index, len - index);

	/* Copy in the new text, until all the characters have been copied or a null is reached.  Set the caret to be
	 * after the copied text and redraw the icon.
	 */

	for (j = 0; text[j] != '\0' && j < copy; j++)
		icon_state.icon.data.indirected_text.text[index + j] = text[j];

	if (caret.w == w && caret.i == i)
		wimp_set_caret_position(w, i, 0, 0, -1, index+copy);

	wimp_set_icon_state(w, i, 0, 0);
}

