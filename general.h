/* SF-Lib - General.h
 *
 * Version 0.10 (5 May 2003)
 */

#ifndef _SFLIB_GENERAL
#define _SFLIB_GENERAL

/* ================================================================================================================== */

#define WORDALIGN(x) ( (x+3) & ~3 )

/* ================================================================================================================== */

int wildcard_strcmp (char *wildcard, char *compared, int any_case);

/* Mode details */

int mode_width (void);
int mode_height (void);

#endif
