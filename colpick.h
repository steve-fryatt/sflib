/* SF-Lib - colpick.h
 *
 * Version 0.10 (19 October 2004)
 */

#ifndef _SFLIB_COLPICK
#define _SFLIB_COLPICK

/* ================================================================================================================== */

void open_simple_colour_window (wimp_w w, wimp_i i);

/* Register the opening of a simple colour picker window, by recording the window and icon number of the
 * parent icon.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


void select_simple_colour_window (int colour);

/* Deal with a selection made from a simple colour window, by setting the parent icon to show the colour passed in.
 *
 * ------------------------------------------------------------------------------------------------------------------ */


void set_colour_icon (wimp_w w, wimp_i i, int colour);

/* Set an icon up to show a colour using number and background.
 *
 * ------------------------------------------------------------------------------------------------------------------ */

#endif
