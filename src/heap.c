/**
 * \file: heap.c
 *
 * SF-Lib - Heap.c
 *
 * (C) Stephen Fryatt, 2003-2011
 *
 * Flexlib-based static heap implementation, providing malloc-like calls
 * on an OS_Heap managed heap inside the first block of a flex heap.
 */

#include <stdlib.h>

/* Acorn C header files. */

#include "flex.h"

/* OS-Lib header files. */

#include "oslib/os.h"
#include "oslib/osheap.h"
#include "oslib/messagetrans.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "heap.h"
#include "errors.h"

#include "debug.h"


#define HEAP_GRANULARITY 1024							/**< The size of standard allocations from flex.			*/
#define HEAP_BLOCK_OHEAD 16							/**< The amount of memory requuired by OS_Heap to manage a heap block.	*/


static byte	*heap_anchor = NULL;
static int	heap_block_size = HEAP_GRANULARITY;


/* Initialise the heap.  Flex must have been initialised via flex_init() before
 * this is called.
 *
 * This is an external interface, documented in heap.h
 */

int heap_initialise(void)
{
	os_error	*error;

	flex_alloc((flex_ptr) &heap_anchor, heap_block_size);

	error = xosheap_initialise(heap_anchor, heap_block_size);
	if (error != NULL);
		wimp_program_report(error);

	return 1;
}


/* Allocate a block of memory from the heap.
 *
 * This is an external interface, documented in heap.h
 */

void * heap_alloc(size_t size)
{
	void		*block;
	os_error	*error;

	size += sizeof(int);

	error = xosheap_alloc(heap_anchor, size, &block);
	if (error != NULL) {
		if (flex_extend ((flex_ptr) &heap_anchor, heap_block_size + size + HEAP_BLOCK_OHEAD)) {
			osheap_resize(heap_anchor, size + HEAP_BLOCK_OHEAD);
			heap_block_size += (size + HEAP_BLOCK_OHEAD);

			error = xosheap_alloc(heap_anchor, size, &block);
			if (error != NULL)
				wimp_program_report (error);
		} else {
			block = NULL;
		}
	}

	if (block != NULL) {
		*(int *)block = size - sizeof (int);
		block = (int *) block + 1;
	}

	return block;
}


/* Free a block of memory previously claimed from the heap.
 *
 * This is an external interface, documented in heap.h
 */

void heap_free(void *ptr)
{
	int	shrink;

	ptr = (int *) ptr - 1;

	osheap_free(heap_anchor, ptr);

	shrink = osheap_resize_no_fail(heap_anchor, 0x80000000);

	if (shrink < 0) {
		heap_block_size += shrink;
		flex_extend((flex_ptr) &heap_anchor, heap_block_size);
	}
}


/* Change the size of a block of memory previously claimed from the heap.
 * This may result in the block moving.
 *
 * This is an external interface, documented in heap.h
 */

void *heap_extend(void *ptr, size_t new_size)
{
	int		change;
	void		*block;
	os_error	*error;

	new_size += sizeof(int);
	ptr = (int *) ptr - 1;
	change = new_size - *(int *)ptr;

	error = xosheap_realloc(heap_anchor, ptr, change, &block);
	if (error != NULL) {
		if (flex_extend((flex_ptr) &heap_anchor, heap_block_size + new_size + HEAP_BLOCK_OHEAD)) {
			osheap_resize(heap_anchor, new_size + HEAP_BLOCK_OHEAD);
			heap_block_size += (new_size + HEAP_BLOCK_OHEAD);

			error = xosheap_realloc(heap_anchor, ptr, change, &block);
			if (error != NULL)
				wimp_program_report(error);
		} else {
			block = NULL;
		}
	}

	if (block != NULL) {
		*(int *) block = new_size - sizeof(int);
		block = (int *) block + 1;
	}

	return block;
}


/* Find the size of a block of memory previously claimed from the heap.
 *
 * This is an external interface, documented in heap.h
 */

size_t heap_size(void *ptr)
{
	return *((size_t *) ptr - 1);
}

