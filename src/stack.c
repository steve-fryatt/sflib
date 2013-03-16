/* Copyright 2005-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: stack.c
 *
 * Simplistic integer stack implementation.
 */

/* SFLib header files. */

#include "stack.h"


#define STACK_SIZE 200								/**< The stack size.		*/


static int	stack[STACK_SIZE];						/**< The stack data.		*/
static int	ptr = 0;							/**< The stack pointer.		*/


/* Push a value on to the stack.
 *
 * This is an external interface, documented in stack.h
 */

void stack_push(int val)
{
	if (ptr < STACK_SIZE)
		stack[ptr++] = val;
}


/* Pull (remove completely) a value from the top of the stack.
 *
 * This is an external interface, documented in stack.h
 */

int stack_pull(void)
{
	int	val = 0;

	if (ptr > 0)
		val = stack[--ptr];

	return val;
}


/* Pop (read without removing) a value from the top of the stack.
 *
 * This is an external interface, documented in stack.h
 */

int stack_pop(void)
{
	int	val = 0;

	if (ptr > 0)
		val = stack[ptr-1];

	return val;
}

