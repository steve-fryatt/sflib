/**
 * \file: stack.c
 *
 * SF-Lib - Stack.c
 *
 * (C) Stephen Fryatt, 2005-2011
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

