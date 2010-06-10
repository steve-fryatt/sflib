/* SF-Lib - Tasks.c
 *
 * Version 0.10 (4 July 2009)
 */

/* OS-Lib header files. */

#include "oslib/taskmanager.h"
#include "oslib/wimp.h"

/* SFLib header files. */

#include "errors.h"
#include "msgs.h"

#include "tasks.h"

/* ANSII C header files. */

#include <string.h>

/* ================================================================================================================== */

osbool test_for_running_task (char *task_name, wimp_t ignore_task)
{
  taskmanager_task task_data;
  int              next;
  osbool           found;
  char             *end;


  next = 0;
  found = FALSE;

  while (next >= 0)
  {
    next = taskmanager_enumerate_tasks (next, &task_data, sizeof (taskmanager_task), &end);

    if (end > (char *) &task_data)
    {
      if (strcmp (task_data.name, task_name) == 0 && task_data.task != ignore_task)
      {
        found = TRUE;
      }
    }
  }

  return (found);
}

/* ================================================================================================================== */

osbool test_for_duplicate_task (char *task_name, wimp_t handle, char *message, char *buttons)
{
  osbool                   running, exit;
  char                     mbuf[1024], bbuf[128];
  wimp_error_box_selection result;

  /* Test to see if a task of the given name is running. */

  running = test_for_running_task (task_name, handle);
  exit = FALSE;

  /* If one is found, offer the user the chance to cancel loading the current version. */

  if (running)
  {
    msgs_param_lookup (message, mbuf, sizeof(mbuf), task_name, NULL, NULL, NULL);
    msgs_lookup (buttons, bbuf, sizeof(bbuf));

    result = wimp_question_report (mbuf, bbuf);

    exit = (result == 1);
  }

  return (exit);
}

/* ================================================================================================================== */
