/* SF-Lib - Errors.h
 *
 * Version 0.15 (15 June 2003)
 */

#ifndef _SFLIB_ERRORS
#define _SFLIB_ERRORS

#include "oslib/messagetrans.h"

/* ================================================================================================================== */

#define sf_errors_BUTTONS_MAX 255

/* ================================================================================================================== */

void error_initialise (char *name, char *sprite, void closedown(void));

/* wimp_ERROR_BOX_OK_ICON
 * wimp_ERROR_BOX_CANCEL_ICON
 */

wimp_error_box_selection wimp_os_error_report (os_error *error, wimp_error_box_flags buttons);
wimp_error_box_selection wimp_msgtrans_error_report (char *token);
wimp_error_box_selection wimp_error_report (char *message);
wimp_error_box_selection wimp_msgtrans_info_report (char *token);
wimp_error_box_selection wimp_info_report (char *message);
wimp_error_box_selection wimp_msgtrans_question_report (char *token, char *buttons);
wimp_error_box_selection wimp_question_report (char *message, char *buttons);
void wimp_msgtrans_fatal_report (char *token); /* Never returns... */
void wimp_fatal_report (char *message); /* Never returns... */
void wimp_program_report (os_error *error); /* Never returns... */

wimp_error_box_selection wimp_report (os_error *error, wimp_error_box_flags type, wimp_error_box_flags buttons,
                                      char *custom_buttons);

#endif
