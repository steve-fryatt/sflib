/**
 * \file: colpick.c
 *
 * SF-Lib - Colpick.c
 *
 * (C) Stephen Fryatt, 2004-2011
 *
 * Simple Wimp colour selection support.
 */


/* OS-Lib header files. */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "colpick.h"
#include "icons.h"

/* ANSII C header files. */

#include <stdio.h>


static wimp_w		colpick_window;						/**< The window containing the icon to be updated.	*/
static wimp_i		colpick_icon;						/**< The icon to be updated.				*/


/* Initialise a colour picker window, supplying details of the parent window
 * and icon which display the selected colour.  These will be updated if
 * colpick_select_colour() is subsequently called.
 *
 * The specified icon *must* be indirected, with space for at least two
 * digits plus terminator.
 *
 * This function is an external interface, documented in colpick.h.
 */

void colpick_open_window(wimp_w w, wimp_i i)
{
	colpick_window = w;
	colpick_icon   = i;
}


/* Update the parent icon previously selected by colpick_open_window to
 * display the given Wimp colour.
 *
 * This function is an external interface, documented in colpick.h.
 */

void colpick_select_colour(wimp_colour colour)
{
	colpick_set_icon_colour(colpick_window, colpick_icon, colour);
}


/* Set the given icon to display the specified Wimp colour, using colour
 * and colour number.
 *
 * The specified icon must be indirected, with space for at least two
 * digits plus terminator.
 *
 * This function is an external interface, documented in colpick.h.
 */

void colpick_set_icon_colour(wimp_w w, wimp_i i, wimp_colour colour)
{
	wimp_colour	foreground;

	icons_printf(w, i, "%d", colour);

	if (colour < wimp_COLOUR_MID_DARK_GREY || colour > wimp_COLOUR_DARK_BLUE)
		foreground = wimp_COLOUR_BLACK;
	else
		foreground = wimp_COLOUR_WHITE;

	wimp_set_icon_state(w, i, (foreground << wimp_ICON_FG_COLOUR_SHIFT) | (colour << wimp_ICON_BG_COLOUR_SHIFT),
			wimp_ICON_FG_COLOUR | wimp_ICON_BG_COLOUR);
}

