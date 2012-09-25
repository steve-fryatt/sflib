/* Copyright 2009-2012, Stephen Fryatt
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
 * \file: tasks.h
 *
 * Wimp Task detection and Task Manager support.
 */

#ifndef SFLIB_TASKS
#define SFLIB_TASKS

#include "oslib/wimp.h"
#include "oslib/types.h"


/**
 * Check if a named task is running.  If ignore_task is a valid task handle, then
 * that task will be ignored in the comparison.
 *
 * \param *task_name		The name to test against.
 * \param ignore_task		A task handle to ignore, even if the name matches.
 * \return			TRUE if a match was found; else FALSE.
 */

osbool tasks_get_running(char *task_name, wimp_t ignore_task);


/**
 * Test for a duplicate copy of the named task, ignoring the task with the
 * specified handle.  If one is found, ask the user whether to quit.
 *
 * \param *task_name		The name of the task to test for.
 * \param handle		A task handle to ignore, even if the name matches.
 * \param *message		A MessageTrans token for the question to ask if
 *				a match is found.
 * \param *buttons		A MessageTrans token for the button list to use
 *				in the question.  The first button must be
 *				the one to quit.
 * \return			TRUE if the task should quit (ie. the first
 *				option was selected by the user.
 */

osbool tasks_test_for_duplicate(char *task_name, wimp_t handle, char *message, char *buttons);

#endif

