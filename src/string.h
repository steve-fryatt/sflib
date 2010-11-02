/* SF-Lib - String.h
 *
 * Version 0.10 (5 May 2003)
 */

#ifndef _SFLIB_STRING
#define _SFLIB_STRING

/* ================================================================================================================== */

char *terminate_ctrl_str (char *s1);

char *ctrl_strcpy (char *s1, const char *s2);
char *ctrl_strcat (char *s1, const char *s2);
int  ctrl_strlen (char *s);

char *strip_surrounding_whitespace (char *string);

char *convert_string_toupper (char *string);
char *convert_string_tolower (char *string);

int strcmp_no_case (char *s1, char *s2);
char *strstr_no_case (char *s1, char *s2);

char *find_leafname (char *string);
char *find_pathname (char *string);
char *find_extension (char *string);

/* Takes a filename, and returns a pointer to the start of any extension,
 * if present.  If no extension of the form .../ext is found, then a
 * pointer to the filename's null terminator is returned.
 */

char *lose_extension (char *string);

/* Takes a filename, and returns a pointer to the start of the leafname.
 * If an extension of the form .../ext is present, the / is replaced by
 * a NULL terminator.
 */

int string_convert_version_number(char *string);

#endif
