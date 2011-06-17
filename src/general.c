/**
 * \file: general.c
 *
 * SF-Lib - General.c
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * General library routines.
 */

/* OS-Lib header files. */

#include "oslib/os.h"

/* SF-Lib header files. */

#include "general.h"

/* ANSII C header files. */

#include <ctype.h>


/* Return the width in OS units of the current screen mode.
 *
 * This is an external interface, documented in general.h
 */

int general_mode_width(void)
{
	int	width, shift;

	os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XWIND_LIMIT, &width);
	os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XEIG_FACTOR, &shift);

	return (width << shift);
}

/* Return the height in OS units of the current screen mode.
 *
 * This is an external interface, documented in general.h
 */

int general_mode_height(void)
{
	int	height, shift;

	os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YWIND_LIMIT, &height);
	os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YEIG_FACTOR, &shift);

	return (height << shift);
}

