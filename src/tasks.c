/* Copyright 2009-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: tasks.c
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

