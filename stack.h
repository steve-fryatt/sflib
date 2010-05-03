/* SF-Lib - Stack.h
 *
 * Version 0.10 (3 Februaty 2005)
 */

#ifndef _SFLIB_STACK
#define _SFLIB_STACK

/* ================================================================================================================== */

#define sf_stack_STACK_SIZE 200

/* ================================================================================================================== */

void stack_push (int val);
int  stack_pull (void);
int  stack_pop (void);

#endif
