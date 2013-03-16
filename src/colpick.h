/* Copyright 2004-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: colpick.h
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

