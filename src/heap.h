/* Copyright 2003-2012, Stephen Fryatt
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
 * \file: heap.h
 *
 * Flexlib-based static heap implementation, providing malloc-like calls
 * on an OS_Heap managed heap inside the first block of a flex heap.
 */

#ifndef SFLIB_HEAP
#define SFLIB_HEAP

#include <stdlib.h>
#include "oslib/types.h"

/**
 * Initialise the heap.  Flex must have been initialised via flex_init() before
 * this is called.
 *
 * \return		TRUE if the heap was initialised; else FALSE.
 */

osbool heap_initialise(void);


/**
 * Allocate a block of memory from the heap.
 *
 * \param size		The amount of memory to claim, in bytes.
 * \return		Pointer to the claimed memory, or NULL on failure.
 */

void * heap_alloc(size_t size);


/**
 * Free a block of memory previously claimed from the heap.
 *
 * \param *ptr		Pointer to the block of memory to be freed.
 */

void heap_free(void *ptr);


/**
 * Change the size of a block of memory previously claimed from the heap.
 * This may result in the block moving.
 *
 * \param *ptr		Pointer to the block of memory to change.
 * \param new_size	The new size for the block.
 * \return		Pointer to the block of memory after update.
 */

void *heap_extend(void *ptr, size_t new_size);


/**
 * Find the size of a block of memory previously claimed from the heap.
 *
 * \param *ptr		Pointer to the block of memory of interest.
 * \return		The size of the block.
 */

size_t heap_size(void *ptr);


/**
 * Perform a strdup() on a string, using memory cliamed by heap_alloc().
 *
 * \param *string	Pointer to the string to be duplicated.
 * \return		Pointer to the duplicate string, or NULL on failure.
 */

char *heap_strdup(char *string);

#endif

