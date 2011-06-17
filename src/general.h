/**
 * \file: general.h
 *
 * SF-Lib - General.h
 *
 * (C) Stephen Fryatt, 2003-2011
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

