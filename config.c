/* SF-Lib - Config.c
 *
 * Version 0.10 (17 August 2003)
 */

/* Acorn C Header files. */


/* OS-Lib header files. */

#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/fileswitch.h"

/* #include "oslib/wimp.h"
 * #include "oslib/messagetrans.h" */

/* ANSII C header files. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/general.h"
#include "sflib/string.h"

/* #include "errors.h" */

/* ================================================================================================================== */

/* Global variables. */

static config_opt *opt_list = NULL;
static config_int *int_list = NULL;
static config_str *str_list = NULL;

static char *choices_dir, *local_dir, *application_name;

/* ================================================================================================================== */

int initialise_configuration (char *app_name, char *c_dir, char *l_dir)
{
  application_name = (char *) malloc (strlen (app_name) + 1);
  strcpy (application_name, app_name);

  local_dir = (char *) malloc (strlen (l_dir) + 1);
  strcpy (local_dir, l_dir);

  choices_dir = (char *) malloc (strlen (c_dir) + 1);
  strcpy (choices_dir, c_dir);

  return 0;
}

/* ================================================================================================================== */

static config_opt *find_config_opt (char *name)
{
  config_opt *block = opt_list;


  while (block != NULL && (strcmp (block->name, name) != 0))
  {
    block = block->next;
  }

  return block;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int init_config_opt (char *name, int value)
{
  config_opt *new;


  if ((new = (config_opt *) malloc (sizeof (config_opt))) == NULL)
  {
    return -1;
  }

  strcpy (new->name, name);
  new->initial = value;
  new->value = value;

  new->next = opt_list;
  opt_list = new;

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int set_config_opt (char *name, int value)
{
  config_opt *option;


  if ((option = find_config_opt (name)) == NULL)
  {
    return -1;
  }

  option->value = value;

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_config_opt (char *name)
{
  config_opt *option;


  if ((option = find_config_opt (name)) == NULL)
  {
    return 0;
  }

  return option->value;
}

/* ================================================================================================================== */

static config_int *find_config_int (char *name)
{
  config_int *block = int_list;


  while (block != NULL && (strcmp (block->name, name) != 0))
  {
    block = block->next;
  }

  return block;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int init_config_int (char *name, int value)
{
  config_int *new;


  if ((new = (config_int *) malloc (sizeof (config_int))) == NULL)
  {
    return -1;
  }

  strcpy (new->name, name);
  new->initial = value;
  new->value = value;

  new->next = int_list;
  int_list = new;

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int set_config_int (char *name, int value)
{
  config_int *option;


  if ((option = find_config_int (name)) == NULL)
  {
    return -1;
  }

  option->value = value;

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_config_int (char *name)
{
  config_int *option;


  if ((option = find_config_int (name)) == NULL)
  {
    return 0;
  }

  return option->value;
}

/* ================================================================================================================== */

static config_str *find_config_str (char *name)
{
  config_str *block = str_list;


  while (block != NULL && (strcmp (block->name, name) != 0))
  {
    block = block->next;
  }

  return block;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int init_config_str (char *name, char *value)
{
  config_str *new;


  if ((new = (config_str *) malloc (sizeof (config_str))) == NULL)
  {
    return -1;
  }

  strcpy (new->name, name);
  strcpy (new->initial, value);
  strcpy (new->value, value);

  new->next = str_list;
  str_list = new;

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int set_config_str (char *name, char *value)
{
  config_str *option;


  if ((option = find_config_str (name)) == NULL)
  {
    return -1;
  }

  strcpy (option->value, value);

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *read_config_str (char *name)
{
  config_str *option;


  if ((option = find_config_str (name)) == NULL)
  {
    return "";
  }

  return option->value;
}

/* ================================================================================================================== */

void find_config_load_file (char *file, size_t len, char *leaf)
{
  snprintf (file, len, "Choices:%s.%s", choices_dir, leaf);

  if (osfile_read_no_path (file, NULL, NULL, NULL, NULL) != fileswitch_IS_FILE)
  {
    snprintf (file, len, "%s.%s", local_dir, leaf);
    if (osfile_read_no_path (file, NULL, NULL, NULL, NULL) != fileswitch_IS_FILE)
    {
      *file = '\0';
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void find_config_save_file (char *file, size_t len, char *leaf)
{
  int        var_len;


  *file = '\0';

  os_read_var_val_size ("Choices$Write", 0, os_VARTYPE_STRING, &var_len, NULL);
  if (var_len == 0)
  {
    snprintf (file, len, "%s.%s", local_dir, leaf);
  }
  else
  {
    snprintf (file, len, "<Choices$Write>.%s", choices_dir);
    if (osfile_read_no_path (file, NULL, NULL, NULL, NULL) == fileswitch_NOT_FOUND)
    {
      osfile_create_dir (file, 0);
    }

    snprintf (file, len, "<Choices$Write>.%s.%s", choices_dir, leaf);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_config_token_pair (FILE *file, char *token, char *value, char *section)
{
  char line[1024], *stripped_line, *a, *b;
  int  result = sf_READ_CONFIG_EOF,
       read = 0;


  while (!read && (fgets (line, sizeof (line), file) != NULL))
  {
    if (*line != '#')
    {
      stripped_line = strip_surrounding_whitespace (line);

      if (wildcard_strcmp ("[*]", stripped_line, 1))
      {
        *strrchr (stripped_line, ']') = '\0';
        if (section != NULL)
        {
          strcpy (section, stripped_line + 1);
        }
        result = sf_READ_CONFIG_NEW_SECTION;
      }
      else
      {
        a = NULL;
        b = strchr (stripped_line, ':');

        if (b != NULL)
        {
          a = stripped_line;

          *b = '\0';
          b += 1;

          if (token != NULL)
          {
            strcpy (token, a);
          }

          if (value != NULL)
          {
            /* Remove external whitespace and enclosing quotes if presnt. */

            b = strip_surrounding_whitespace (b);

            if (*b == '"' && *(strchr(b, '\0')-1) == '"')
            {
              b++;
              *(strchr(b, '\0')-1) = '\0';
            }

            strcpy (value, b);
          }

          if (result != sf_READ_CONFIG_NEW_SECTION)
          {
            result = sf_READ_CONFIG_VALUE_RETURNED;
          }

          read = 1;
        }
        else
        {
          if (token != NULL)
          {
            *token = '\0';
          }

          if (value != NULL)
          {
            *value = '\0';
          }
        }
      }
    }
  }


  return (result);
}

/* ------------------------------------------------------------------------------------------------------------------ */

int write_config_token_pair (FILE *file, char *token, char *value)
{
  int result;

  if (isspace (*value) || isspace (*(strchr(value, '\0')-1)))
  {
    result = fprintf (file, "%s: \"%s\"\n", token, value);
  }
  else
  {
    result = fprintf (file, "%s: %s\n", token, value);
  }

  return (result);
}

/* ================================================================================================================== */

int load_configuration (void)
{
  char       file[1024], token[1024], contents[1024];
  FILE       *in;


  /* Find the options.  First try the Choices: file then the one in the application. */

  find_config_load_file (file, sizeof(file), "Choices");

  /* If a config file was found, use it. */

  if (*file != '\0')
  {
    in = fopen (file, "r");

    if (in != NULL)
    {
      while (read_config_token_pair (in, token, contents, NULL) != sf_READ_CONFIG_EOF)
      {
        /* Check if it's a option config... */

        if (find_config_opt (token) != NULL)
        {
          set_config_opt (token, read_opt_string (contents));
        }

        /* Check if it's an int config... */

        else if (find_config_int (token) != NULL)
        {
          set_config_int (token, atoi (contents));
        }

        /* Check if it's an str config... */

        else if (find_config_str (token) != NULL)
        {
          set_config_str (token, contents);
        }
      }

      fclose (in);
    }
  }

  return 0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

int save_configuration (void)
{
  char       file[1024];
  FILE       *out;

  config_opt *opt_block;
  config_int *int_block;
  config_str *str_block;


  find_config_save_file (file, sizeof(file), "Choices");

  if (*file != '\0')
  {
    out = fopen (file, "w");

    if (out != NULL)
    {
      fprintf (out, "# >Choices for %s\n\n", application_name);

      /* Do the opt configs */

      opt_block = opt_list;

      while (opt_block != NULL)
      {
        if (opt_block->value != opt_block->initial)
        {
          fprintf (out, "%s: %s\n", opt_block->name, return_opt_string (opt_block->value));
        }

        opt_block = opt_block->next;
      }

      /* Do the int configs */

      int_block = int_list;

      while (int_block != NULL)
      {
        if (int_block->value != int_block->initial)
        {
          fprintf (out, "%s: %d\n", int_block->name, int_block->value);
        }

        int_block = int_block->next;
      }

      /* Do the str configs */

      str_block = str_list;

      while (str_block != NULL)
      {
        if (strcmp (str_block->value, str_block->initial) != 0)
        {
          fprintf (out, "%s: \"%s\"\n", str_block->name, str_block->value);
        }

        str_block = str_block->next;
      }

      /* Do the rest... */

      fclose (out);
    }
  }

  return 0;
}

/* ================================================================================================================== */

int restore_default_configuration (void)
{
  return 0;
}

/* ================================================================================================================== */

char *return_opt_string (int opt)
{
  if (opt)
  {
    return ("Yes");
  }
  else
  {
    return ("No");
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_opt_string (char *str)
{
  char line[256];

  strcpy (line, str);
  convert_string_tolower (line);

  return (strcmp (line, "yes") == 0);
}
