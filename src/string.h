/**
 * \file: string.h
 *
 * SF-Lib - String.h
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * Generic and RISC OS-specific string handling functions.
 */

#ifndef SFLIB_STRING
#define SFLIB_STRING


/**
 * Zero-terminate a ctrl-terminated string, overwriting the terminator
 * in the supplied buffer.
 *
 * \param *s1		The string to reterminate.
 * \return		Pointer to the reterminated string.
 */

char *string_ctrl_zero_terminate(char *s1);


/**
 * Perform a strcpy() on a source string that is ctrl-terminated.
 *
 * \param *s1		A buffer to hold the copied string.
 * \param *s2		The string to copy.
 * \return		Pointer to the copy of the string.
 */

char *string_ctrl_strcpy(char *s1, const char *s2);


/**
 * Perform a strcat() on two strings that are ctrl-terminated.
 *
 * \param *s1		The destination string.
 * \param *s2		The string to add to the destination string.
 * \return		Pointer to the resulting string.
 */

char *string_ctrl_strcat(char *s1, const char *s2);


/**
 * Perform a strlen() on a string that is ctrl-terminated.
 *
 * \param *s		The string to count.
 * \return		The length of the string, in characters.
 */

int string_ctrl_strlen(char *s);


/**
 * Convert a string to upper case.
 *
 * \param *string	The string to convert.
 * \return		Pointer to the converted string.
 */

char *string_toupper(char *string);


/**
 * Convert a string to lower case.
 *
 * \param *string	The string to convert.
 * \return		Pointer to the converted string.
 */

char *string_tolower(char *string);


/**
 * Perform a strcmp() case-insensitively on two strings, returning
 * a value less than, equal to or greater than zero depending on
 * their relative values.
 *
 * \param *s1		The first string to be compared.
 * \param *s2		The second string to be compared.
 * \return		The result of the comparison.
 */

int string_nocase_strcmp(char *s1, char *s2);


/**
 * Perform an strstr() case-insensitively on two strings, searching
 * one string for the other substring and returning a pointer to the
 * found location.
 *
 * \param *s1		The string to search.
 * \param *s2		The substring to search for.
 * \return		Pointer to the first match, or NULL if not found.
 */

char *string_nocase_strstr(char *s1, char *s2);


/**
 * Strip whitespace from the supplied string.  Space at the end is
 * removed by overwiting the first character with zero; the returned
 * pointer is set to the first non-space character in the buffer.
 *
 * \param *string	The string to strip.
 * \return		Pointer to the first non-space character, or NULL.
 */

char *string_strip_surrounding_whitespace(char *string);


/**
 * Given a RISC OS filename, return a pointer to the part that represents the
 * leafname (ie the part after the final '.').
 *
 * \param *filename		The filename to check.
 * \return			Pointer to the leafname part of the buffer.
 */

char *string_find_leafname(char *filename);


/**
 * Given a RISC OS filename, return a pointer to the part that represents the
 * pathname (ie the part before the final '.') and terminate the name at
 * the leafname.  The passed string is modified.
 *
 * \param *filename		The filename to check.
 * \return			Pointer to the pathname part of the buffer.
 */

char *string_find_pathname(char *filename);


/**
 * Given a RISC OS filename, return a pointer to the part that represents the
 * file extension (ie the part of the leafname after the final '/').  If no
 * extension is present, a pointer to an empty string is returned.
 *
 * \param *filename		The filename to check.
 * \return			Pointer to the extension part of the buffer.
 */

char *string_find_extension(char *filename);


/**
 * Given a RISC OS filename, remove any filetype extension (the part including,
 * and following, the final '/'), terminating the name at the location of the
 * '/' and returning the remaining leafname.  The passed string is modified.
 *
 * \param *filename		The filename to check.
 * \return			Pointer to the leafname part of the buffer.
 */

char *string_strip_extension(char *filename);


/**
 * Convert a textual n.nn version number into an integer in the form nnn.
 *
 * \param *string		The string to parse into a version number.
 * \return			The numeric version number, or -1 if failed.
 */

int string_convert_version_number(char *string);

#endif

