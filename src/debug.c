/**
 * \file: debug.c
 *
 * SF-Lib - Debug.c
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * Debug support for writing data to Reporter.
 */

/* Acorn C Header files. */

#include "kernel.h"

/* OS-Lib header files. */

#include "oslib/report.h"

/* ANSII C header files. */

#include <stdarg.h>
#include <stdio.h>

/* SF-Lib header files. */

#include "debug.h"


/* Print a string to Reporter, using the standard printf() syntax and
 * functionality.  Expanded text is limited to 256 characters including
 * a null terminator.
 *
 * This function is an external interface, documented in debug.c.
 */

int debug_printf(char *cntrl_string, ...)
{
	char		s[256];
	int		ret;
	va_list		ap;

	va_start(ap, cntrl_string);
	ret = vsnprintf(s, sizeof(s), cntrl_string, ap);
	report_text0(s);

	return ret;
}

