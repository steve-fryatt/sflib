/* Copyright 2005-2012, Stephen Fryatt
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
 * \file: stack.h
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

