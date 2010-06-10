/* SF-Lib - Config.h
 *
 * Version 0.10 (17 August 2003)
 */

#ifndef _SFLIB_CONFIG
#define _SFLIB_CONFIG

#include <stdio.h>

/* ================================================================================================================== */

#define sf_MAX_CONFIG_NAME 32
#define sf_MAX_CONFIG_STR  1024

#define sf_READ_CONFIG_EOF 0
#define sf_READ_CONFIG_VALUE_RETURNED 1
#define sf_READ_CONFIG_NEW_SECTION 2

/* ================================================================================================================== */

typedef struct config_opt
{
  char              name[sf_MAX_CONFIG_NAME];
  int               value;
  int               initial;

  struct config_opt *next;
}
config_opt;

typedef struct config_int
{
  char              name[sf_MAX_CONFIG_NAME];
  int               value;
  int               initial;

  struct config_int *next;
}
config_int;

typedef struct config_str
{
  char              name[sf_MAX_CONFIG_NAME];
  char              value[sf_MAX_CONFIG_STR];
  char              initial[sf_MAX_CONFIG_STR];

  struct config_str *next;
}
config_str;

/* ================================================================================================================== */

int initialise_configuration (char *app_name, char *choices_dir, char *local_dir);

int restore_default_configuration (void);

void find_config_load_file (char *file, size_t len, char *leaf);
void find_config_save_file (char *file, size_t len, char *leaf);

int read_config_token_pair (FILE *file, char *token, char *value, char *section);
int write_config_token_pair (FILE *file, char *token, char *value);

int load_configuration (void);
int save_configuration (void);

int init_config_opt (char *name, int value);
int set_config_opt (char *name, int value);
int read_config_opt (char *name);

int init_config_int (char *name, int value);
int set_config_int (char *name, int value);
int read_config_int (char *name);

int init_config_str (char *name, char *value);
int set_config_str (char *name, char *value);
char * read_config_str (char *name);

char *return_opt_string (int opt);
int read_opt_string (char *str);

#endif
