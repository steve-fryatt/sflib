/* SF-Lib - String.c
 *
 * Version 0.10 (5 May 2003)
 */

/* OS-Lib header files. */

#include "oslib/os.h"

/* SF-Lib header files. */

#include "general.h"

/* ANSII C header files. */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* ================================================================================================================== */

char *terminate_ctrl_str (char *s1)
{
  while (*s1 >= os_VDU_SPACE)
  {
    s1++;
  }

  *s1 = '\0';

  return (s1);
}

/* ================================================================================================================== */

char *ctrl_strcpy (char *s1, const char *s2)
{
  char *s = s1;

  while (*s2 >= os_VDU_SPACE)
  {
    *s1++ = *s2++;
  }
  *s1 = '\0';

  return (s);
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *ctrl_strcat (char *s1, const char *s2)
{
  char *s = s1;

  while (*s1 >= os_VDU_SPACE)
  {
    s1++;
  }

  while (*s2 >= os_VDU_SPACE)
  {
    *s1++ = *s2++;
  }
  *s1 = '\0';

  return (s);
}

/* ------------------------------------------------------------------------------------------------------------------ */

size_t ctrl_strlen (char *s)
{
  int len = 0;

  while (*s++ >= os_VDU_SPACE)
  {
    len++;
  }

  return ((size_t) len);
}

/* ================================================================================================================== */

char *strip_surrounding_whitespace (char *string)
{
  char *start, *end;

  if (*string == '\0')
  {
    return (string);
  }

  start = string;
  while (isspace (*start))
  {
    start++;
  }

  end = strrchr (string, '\0') - 1;
  while (isspace (*end))
  {
    *end-- = '\0';
  }

  return (start);
}

/* ================================================================================================================== */

char *convert_string_toupper (char *string)
{
  char *start;

  start = string;

  while (*string != '\0')
  {
    *string = toupper (*string);
    string++;
  }

  return (start);
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *convert_string_tolower (char *string)
{
  char *start;

  start = string;

  while (*string != '\0')
  {
    *string = tolower (*string);
    string++;
  }

  return (start);
}

/* ------------------------------------------------------------------------------------------------------------------ */

int strcmp_no_case (char *s1, char *s2)
{
  while (*s1 != '\0' && *s2 != '\0' && (toupper(*s1) - toupper(*s2)) == 0)
  {
    s1++;
    s2++;
  }

  return (toupper(*s1) - toupper(*s2));
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *strstr_no_case (char *s1, char *s2)
{
  char *s1c, *s2c;

  while (*s1 != '\0')
  {
    s1c = s1;
    s2c = s2;

    while (*s1c != '\0' && *s2c != '\0' && toupper(*s1c) == toupper(*s2c))
    {
      s1c++;
      s2c++;
    }

    if (*s2c == '\0')
    {
      break;
    }
    s1++;
  }

  return (s1);
}

/* ================================================================================================================== */

char *find_leafname (char *string)
{
  char *start;

  start = strrchr (string, '.');

  if (start != NULL)
  {
    start++;
  }
  else
  {
    start = string;
  }

  return (start);
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *find_pathname (char *string)
{
  char *start;

  start = strrchr (string, '.');

  if (start != NULL)
  {
    *start = '\0';
  }

  return (string);
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *find_extension (char *string)
{
  char *leaf, *ext;

  leaf = find_leafname (string);

  ext = strrchr (leaf, '/');

  if (ext != NULL)
  {
    ext++;
  }
  else
  {
    ext = strchr (leaf, '\0');
  }

  return (ext);
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *lose_extension (char *string)
{
  char *leaf, *ext;

  leaf = find_leafname (string);

  ext = strrchr (leaf, '/');

  if (ext != NULL)
  {
    *ext = '\0';
  }

  return (leaf);
}

/**
 * Convert a textual n.nn version number into an integer in the form nnn.
 *
 * \param *string		The string to parse into a version number.
 * \return			The numeric version number, or -1 if failed.
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

