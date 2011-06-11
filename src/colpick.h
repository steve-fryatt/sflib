/**
 * \file: colpick.h
 *
 * SF-Lib - Colpick.h
 *
 * (C) Stephen Fryatt, 2004-2011
 *
 * Simple Wimp colour selection support.
 */

#ifndef SFLIB_COLPICK
#define SFLIB_COLPICK

#include "oslib/wimp.h"


/**
 * Initialise a colour picker window, supplying details of the parent window
 * and icon which display the selected colour.  These will be updated if
 * colpick_select_colour() is subsequently called.
 *
 * The specified icon *must* be indirected, with space for at least two
 * digits plus terminator.
 *
 * \param w		The parent window.
 * \param i		The parent icon, which will be updated on completion.
 */

void colpick_open_window(wimp_w w, wimp_i i);


/**
 * Update the parent icon previously selected by colpick_open_window to
 * display the given Wimp colour.
 *
 * \param colour	The colour to display.
 */

void colpick_select_colour(wimp_colour colour);


/**
 * Set the given icon to display the specified Wimp colour, using colour
 * and colour number.
 *
 * The specified icon must be indirected, with space for at least two
 * digits plus terminator.
 *
 * \param w		The handle of the window containing the icon.
 * \param i		The icon to update.
 * \param colour	The colour to display.
 */

void colpick_set_icon_colour(wimp_w w, wimp_i i, wimp_colour colour);

#endif

