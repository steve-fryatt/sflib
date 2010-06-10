/* SF-Lib - colpick.c
 *
 * Version 0.10 (19 October 2004)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "colpick.h"
#include "icons.h"

/* ANSII C header files. */

#include <stdio.h>

/* ================================================================================================================== */

/* Simple colour picker static data. */

static wimp_w simple_window;
static wimp_i simple_icon;

/* ================================================================================================================== */

/* Register the opening of a simple colour picker window, by recording the window and icon number of the
 * parent icon.
 */

void open_simple_colour_window (wimp_w w, wimp_i i)
{
  simple_window = w;
  simple_icon   = i;
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Deal with a selection made from a simple colour window, by setting the parent icon to show the colour passed in.
 */

void select_simple_colour_window (int colour)
{
  set_colour_icon (simple_window, simple_icon, colour);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Set an icon up to show a colour using number and background.
 */

void set_colour_icon (wimp_w w, wimp_i i, int colour)
{
  int foreground;

  sprintf (indirected_icon_text (w, i), "%d", colour);

  if (colour < wimp_COLOUR_MID_DARK_GREY || colour > wimp_COLOUR_DARK_BLUE)
  {
    foreground = wimp_COLOUR_BLACK;
  }
  else
  {
    foreground = wimp_COLOUR_WHITE;
  }

  wimp_set_icon_state (w, i,
                       (foreground << wimp_ICON_FG_COLOUR_SHIFT) | (colour << wimp_ICON_BG_COLOUR_SHIFT),
                       wimp_ICON_FG_COLOUR | wimp_ICON_BG_COLOUR);
}

