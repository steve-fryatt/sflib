/* SF-Lib - General.c
 *
 * Version 0.10 (5 May 2003)
 */

/* OS-Lib header files. */

#include "oslib/os.h"

/* SF-Lib header files. */

#include "general.h"

/* ANSII C header files. */

#include <ctype.h>

/* ================================================================================================================== */

int wildcard_strcmp (char *s1, char *s2, int any_case)
{
  char c1 = *s1, c2 = *s2;

  if (any_case)
  {
    c1 = tolower (c1);
    c2 = tolower (c2);
  }

  if (c2 == 0) {
    while (*s1 == '*')
      s1++;
    return (*s1 == 0);
  }

  if (c1 == c2 || c1 == '#')
    return (wildcard_strcmp (&s1[1], &s2[1], any_case));

  if (c1 == '*') {
    int ok = FALSE;

    if (s1[1] == 0)
      return (TRUE);

    while (!ok && *s2 != 0) {
      ok = ok || wildcard_strcmp (&s1[1], s2, any_case);
      s2++;
    }

    return (ok);
  }

  return (FALSE);
}

/* ==================================================================================================================
 * Mode details
 */

/* Return the width in OS units of the current screen mode. */

int mode_width (void)
{
  int width, shift;

  os_read_mode_variable (os_CURRENT_MODE, os_MODEVAR_XWIND_LIMIT, &width);
  os_read_mode_variable (os_CURRENT_MODE, os_MODEVAR_XEIG_FACTOR, &shift);

  return (width << shift);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Return the height in OS units of the current screen mode. */

int mode_height (void)
{
  int height, shift;

  os_read_mode_variable (os_CURRENT_MODE, os_MODEVAR_YWIND_LIMIT, &height);
  os_read_mode_variable (os_CURRENT_MODE, os_MODEVAR_YEIG_FACTOR, &shift);

  return (height << shift);
}
