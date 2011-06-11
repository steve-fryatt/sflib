/**
 * \file: debug.h
 *
 * SF-Lib - Debug.h
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * Debug support for writing data to Reporter.
 */

#ifndef SFLIB_DEBUG
#define SFLIB_DEBUG


/**
 * Print a string to Reporter, using the standard printf() syntax and
 * functionality.  Expanded text is limited to 256 characters including
 * a null terminator.
 *
 * \param *cntrl_string		A standard printf() formatting string.
 * \param ...			Additional printf() parameters as required.
 * \return			The number of characters written, or <0 for error.
 */

int debug_printf(char *cntrl_string, ...);

#endif

