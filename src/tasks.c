/**
 * \file: tasks.c
 *
 * SF-Lib - Tasks.c
 *
 * (C) Stephen Fryatt, 2009-2011
 *
 * Wimp Task detection and Task Manager support.
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


/* Check if a named task is running.
 *
 * This is an external interface, documented in tasks.h
 */

osbool tasks_get_running(char *task_name, wimp_t ignore_task)
{
	taskmanager_task	task_data;
	int			next = 0;
	osbool			found = FALSE;
	char			*end;

	while (next >= 0) {
		next = taskmanager_enumerate_tasks(next, &task_data, sizeof(taskmanager_task), &end);

		if (end > (char *) &task_data && strcmp(task_data.name, task_name) == 0 && task_data.task != ignore_task)
			found = TRUE;
	}

	return found;
}


/* Test for a duplicate copy of the named task, ignoring the task with the
 * specified handle.  If one is found, ask the user whether to quit.
 *
 * This is an external interface, documented in tasks.h
 */

osbool tasks_test_for_duplicate (char *task_name, wimp_t handle, char *message, char *buttons)
{
	osbool			running, exit = FALSE;
	char			mbuf[1024], bbuf[128];

	/* Test to see if a task of the given name is running. */

	running = tasks_get_running(task_name, handle);

	/* If one is found, offer the user the chance to cancel loading the current version. */

	if (running) {
		msgs_param_lookup(message, mbuf, sizeof(mbuf), task_name, NULL, NULL, NULL);
		msgs_lookup(buttons, bbuf, sizeof(bbuf));

		if (error_report_question(mbuf, bbuf) == 1)
			exit = TRUE;
	}

	return exit;
}

