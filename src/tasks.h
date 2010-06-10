/* SF-Lib - Tasks.h
 *
 * Version 0.10 (4 July 2009)
 */

#ifndef _SFLIB_TASKS
#define _SFLIB_TASKS

#include "oslib/wimp.h"
#include "oslib/types.h"

/* ================================================================================================================== */

/* ================================================================================================================== */

osbool test_for_running_task (char *task_name, wimp_t ignore_task);

/* Returns TRUE if a task with the given name is already running.  If ignore_task is a valid task handle, then
 * that task will be ignored in the comparison.
 */

osbool test_for_duplicate_task (char *task_name, wimp_t handle, char *message, char *buttons);

/* Test for a duplicate copy of the task, and return TRUE if the current task should quit as a result.
 */

#endif
