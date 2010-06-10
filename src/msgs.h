/* SF-Lib - Msgs.h
 *
 * Version 0.10 (22 September 2003)
 */

#ifndef _SFLIB_MSGS
#define _SFLIB_MSGS

#ifndef messagetrans_H
#include "oslib/messagetrans.h"
#endif

/* ================================================================================================================== */

int msgs_init (char *messages_file);
int msgs_external_init (messagetrans_control_block *block);
int msgs_close_file (void);

char *msgs_lookup (char *token, char *buffer, int buffer_size);
char *msgs_param_lookup (char *token, char *buffer, int buffer_size, char *a, char *b, char *c, char *d);

int msgs_lookup_result (char *token, char *buffer, int buffer_size);
int msgs_param_lookup_result (char *token, char *buffer, int buffer_size, char *a, char *b, char *c, char *d);

#endif
