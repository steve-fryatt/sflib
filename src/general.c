/* Copyright 2003-2019, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: general.c
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

	return ((width + 1) << shift);
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

	return ((height + 1) << shift);
}

