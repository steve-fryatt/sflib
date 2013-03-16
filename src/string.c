/* Copyright 2003-2012, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of SFLib:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
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
 * \file: string.c
 *
 * Generic and RISC OS-specific string handling functions.
 */

/* OS-Lib header files. */

#include "oslib/os.h"

/* SF-Lib header files. */

#include "general.h"

/* ANSII C header files. */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/* Zero-terminate a ctrl-terminated string, overwriting the terminator
 * in the supplied buffer.
 *
 * This is an external interface, documented in string.h
 */

char *string_ctrl_zero_terminate(char *s1)
{
	if (s1 == NULL)
		return NULL;

	while (*s1 >= os_VDU_SPACE)
		s1++;

	*s1 = '\0';

	return s1;
}


/* Perform a strcpy() on a source string that is ctrl-terminated.
 *
 * This is an external interface, documented in string.h
 */

char *string_ctrl_strcpy(char *s1, const char *s2)
{
	char	*s = s1;

	if (s1 == NULL)
		return NULL;

	while (*s2 >= os_VDU_SPACE)
		*s1++ = *s2++;

	*s1 = '\0';

	return s;
}


/* Perform a strcat() on two strings that are ctrl-terminated.
 *
 * This is an external interface, documented in string.h
 */

char *string_ctrl_strcat(char *s1, const char *s2)
{
	char	*s = s1;

	while (*s1 >= os_VDU_SPACE)
		s1++;

	while (*s2 >= os_VDU_SPACE)
		*s1++ = *s2++;

	*s1 = '\0';

	return s;
}


/* Perform a strlen() on a string that is ctrl-terminated.
 *
 * This is an external interface, documented in string.h
 */

size_t string_ctrl_strlen(char *s)
{
	int	len = 0;

	while (*s++ >= os_VDU_SPACE)
		len++;

	return (size_t) len;
}


/* Convert a string to upper case.
 *
 * This is an external interface, documented in string.h
 */

char *string_toupper(char *string)
{
	char	*start;

	start = string;

	while (*string != '\0') {
		*string = toupper(*string);
		string++;
	}

	return start;
}


/* Convert a string to lower case.
 *
 * This is an external interface, documented in string.h
 */

char *string_tolower(char *string)
{
	char	*start;

	start = string;

	while (*string != '\0') {
		*string = tolower (*string);
		string++;
	}

	return start;
}


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

osbool string_wildcard_compare(char *s1, char *s2, osbool any_case)
{
	char c1 = *s1, c2 = *s2;

	if (any_case) {
		c1 = tolower(c1);
		c2 = tolower(c2);
	}

	if (c2 == 0) {
		while (*s1 == '*')
			s1++;
		return (*s1 == 0);
	}

	if (c1 == c2 || c1 == '#')
		return string_wildcard_compare(&s1[1], &s2[1], any_case);

	if (c1 == '*') {
		osbool ok = FALSE;

		if (s1[1] == 0)
			return TRUE;

		while (!ok && *s2 != 0) {
			ok = ok || string_wildcard_compare(&s1[1], s2, any_case);
			s2++;
		}

		return ok;
	}

	return FALSE;
}


/* Perform a strcmp() case-insensitively on two strings, returning
 * a value less than, equal to or greater than zero depending on
 * their relative values.
 *
 * This is an external interface, documented in string.h
 */

int string_nocase_strcmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s2 != '\0' && (toupper(*s1) - toupper(*s2)) == 0) {
		s1++;
		s2++;
	}

	return (toupper(*s1) - toupper(*s2));
}


/* Perform an strstr() case-insensitively on two strings, searching
 * one string for the other substring and returning a pointer to the
 * found location.
 *
 * This is an external interface, documented in string.h
 */

char *string_nocase_strstr(char *s1, char *s2)
{
	char	*s1c, *s2c;

	while (*s1 != '\0') {
		s1c = s1;
		s2c = s2;

		while (*s1c != '\0' && *s2c != '\0' && toupper(*s1c) == toupper(*s2c)) {
			s1c++;
			s2c++;
		}

		if (*s2c == '\0')
			break;

		s1++;
	}

	return s1;
}


/* Strip whitespace from the supplied string.  Space at the end is
 * removed by overwiting the first character with zero; the returned
 * pointer is set to the first non-space character in the buffer.
 *
 * This is an external interface, documented in string.h
 */

char *string_strip_surrounding_whitespace(char *string)
{
	char	*start, *end;

	if (string == NULL || *string == '\0')
		return string;

	start = string;
	while (isspace(*start))
		start++;

	end = strrchr(string, '\0') - 1;
	while (isspace(*end))
		*end-- = '\0';

	return start;
}


/* Given a RISC OS filename, return a pointer to the part that represents the
 * leafname (ie the part after the final '.').
 *
 * This is an external interface, documented in string.h
 */

char *string_find_leafname(char *filename)
{
	char	*leaf;

	leaf = strrchr(filename, '.');

	if (leaf != NULL)
		leaf++;
	else
		leaf = filename;

	return leaf;
}


/* Given a RISC OS filename, return a pointer to the part that represents the
 * pathname (ie the part before the final '.') and terminate the name at
 * the leafname.  The passed string is modified.
 *
 * This is an external interface, documented in string.h
 */

char *string_find_pathname(char *filename)
{
	char	*leaf;

	leaf = strrchr(filename, '.');

	if (leaf != NULL)
		*leaf = '\0';

	return filename;
}


/* Given a RISC OS filename, return a pointer to the part that represents the
 * file extension (ie the part of the leafname after the final '/').  If no
 * extension is present, a pointer to an empty string is returned.
 *
 * This is an external interface, documented in string.h
 */

char *string_find_extension(char *filename)
{
	char	*leaf, *ext;

	leaf = string_find_leafname(filename);
	ext = strrchr(leaf, '/');

	if (ext != NULL)
		ext++;
	else
		ext = strchr (leaf, '\0');

	return ext;
}


/* Given a RISC OS filename, remove any filetype extension (the part including,
 * and following, the final '/'), terminating the name at the location of the
 * '/' and returning the remaining leafname.  The passed string is modified.
 *
 * This is an external interface, documented in string.h
*/

char *string_strip_extension(char *filename)
{
	char	*leaf, *ext;

	leaf = string_find_leafname(filename);
	ext = strrchr(leaf, '/');

	if (ext != NULL)
		*ext = '\0';

	return leaf;
}


/* Convert a textual n.nn version number into an integer in the form nnn.
 *
 * This is an external interface, documented in string.h
 */

int string_convert_version_number(char *string)
{
	char	*start, *end, *p;
	int	count;

	if (*string == '\0')
		return -1;


	/* Clean up the string, removing surplus characters from start and finish. */

	start = string;
	while (!isdigit(*start))
		start++;


	end = strrchr(string, '\0') - 1;

	while (!isdigit(*end))
		*end-- = '\0';

	/* Check that we're all digits and one decimal point. */

	p = start;
	count = 0;

	while (*p != '\0' && count <= 1 && (isdigit(*p) || *p == '.')) {
		if (*p == '.')
			count++;
		p++;
	}

	if (count >= 2 || *p != '\0')
		return -1;

	/* Check that we've the right number of decimal places. */

	p = strchr(start, '.');
	if ((end - p) != 2)
		return -1;

	/* Convert to an integer string and then to an integer. */

	*p = *(p+1);
	p++;
	*p = *(p+1);
	p++;
	*p = '\0';

	return atoi(start);
}

