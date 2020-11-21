/* Copyright 2003-2020, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: string.h
 *
 * Generic and RISC OS-specific string handling functions.
 */

#ifndef SFLIB_STRING
#define SFLIB_STRING

#include <stddef.h>
#include "oslib/types.h"


/**
 * Perform a strncpy(), sanity-checking the supplied pointer details and
 * ensuring that the copy is zero-terminated even if the source string
 * is longer than the supplied buffer.
 *
 * \param *dest		A buffer to hold the copied string.
 * \param *src		The string to be copied.
 * \param len		The maximum number of characters to copy.
 * \return		A pointer to the copy of the string, or NULL
 *			if the supplied pointers were invalid.
 */

char *string_copy(char *dest, char *src, size_t len);


/**
 * Perform a string_control_strncpy(), sanity-checking the supplied
 * pointer details and ensuring that the copy is zero-terminated even if
 * the source string is longer than the supplied buffer.
 *
 * \param *dest		A buffer to hold the copied string.
 * \param *src		The string to be copied.
 * \param len		The maximum number of characters to copy.
 * \return		A pointer to the copy of the string, or NULL
 *			if the supplied pointers were invalid.
 */

char *string_ctrl_copy(char *dest, char *src, size_t len);


/**
 * Perform an snprintf(), sanity-checking the supplied pointer details
 * and ensuring that the resulting string is zero-terminated even if the
 * buffer is not long enough to accommodate it.
 *
 * \param *str			A buffer to hold the resulting string.
 * \param len			The length of the supplied buffer.
 * \param *cntrl_string		A standard printf() formatting string.
 * \param ...			Additional printf() parameters as required.
 * \return			The number of characters written, or <0 for error.
 */

int string_printf(char *str, size_t len, char *cntrl_string, ...);


/**
 * Zero-terminate a ctrl-terminated string, overwriting the terminator
 * in the supplied buffer.
 *
 * \param *s1		The string to reterminate.
 * \param len		The maximum number of characters to include.
 * \return		Pointer to the reterminated string.
 */

char *string_ctrl_zero_terminate(char *s1, size_t len);


/**
 * Perform a strncpy() on a source string that is ctrl-terminated.
 *
 * \param *s1		A buffer to hold the copied string.
 * \param *s2		The string to copy.
 * \param len		The maximum number of characters to copy from s2.
 * \return		Pointer to the copy of the string.
 */

char *string_ctrl_strncpy(char *s1, const char *s2, size_t len);


/**
 * Perform a strncat() on two strings that are ctrl-terminated.
 *
 * \param *s1		The destination string.
 * \param *s2		The string to add to the destination string.
 * \param len		The maximum number of characters to copy from s2.
 * \return		Pointer to the resulting string.
 */

char *string_ctrl_strncat(char *s1, const char *s2, size_t len);


/**
 * Perform a strlen() on a string that is ctrl-terminated.
 *
 * \param *s		The string to count.
 * \return		The length of the string, in characters.
 */

size_t string_ctrl_strlen(char *s);


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
 * Compare two strings to see if they match. One string can contain
 * the wildards # for any single character and * for any zero or
 * more characters.  The comparison can be case-insensitive if required.
 *
 * \param *s1		The string to search for, with wildcards.
 * \param *s2		The string to test.
 * \param any_case	TRUE for a case-insensitive search; else FALSE.
 * \return		TRUE if the strings match; else FALSE.
 */

osbool string_wildcard_compare(char *s1, char *s2, osbool any_case);


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

