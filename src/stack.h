/**
 * \file: stack.h
 *
 * SF-Lib - Stack.h
 *
 * (C) Stephen Fryatt, 2005-2011
 *
 * Simplistic integer stack implementation.
 */

#ifndef SFLIB_STACK
#define SFLIB_STACK

/**
 * Push a value on to the stack.
 *
 * \param val		The value to push on to the stack.
 */

void stack_push(int val);


/**
 * Pull (remove completely) a value from the top of the stack.
 *
 * \return		The top value from the stack, or 0 if empty.
 */

int stack_pull(void);


/**
 * Pop (read without removing) a value from the top of the stack.
 *
 * \return		The top value from the stack, or 0 if empty.
 */

int stack_pop(void);

#endif

