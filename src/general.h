/* Copyright 2003-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: general.h
 *
 * General library routines.
 */

#ifndef SFLIB_GENERAL
#define SFLIB_GENERAL


/**
 * Round the given block size to the next full word.
 */

#define WORDALIGN(x) ( (x+3) & ~3 )


/**
 * Return the width in OS units of the current screen mode.
 *
 * \return		The current screen width in OS units.
 */

int general_mode_width(void);


/**
 * Return the height in OS units of the current screen mode.
 *
 * \return		The current screen height in OS units.
 */

int general_mode_height(void);

#endif

