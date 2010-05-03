/* SF-Lib - Debug.c
 *
 * Version 0.10 (16 November 2003)
 */

/* Acorn C Header files. */

#include "kernel.h"

/* OS-Lib header files. */

#include "oslib/report.h"

/* ANSII C header files. */

#include <stdarg.h>
#include <stdio.h>

/* SF-Lib header files. */

#include "sflib/debug.h"

/* ==================================================================================================================
 * Stand-alone functions
 */

int debug_printf (char *cntrl_string, ...)
{
  char    s[256];
  int     ret;
  va_list ap;

  va_start(ap, cntrl_string);
  ret = vsnprintf (s, sizeof(s), cntrl_string, ap);
  report_text0 (s);

  return (ret);
}
