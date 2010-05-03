/* SF-Lib - Heap.h
 *
 * Version 0.10 (20 September 2003)
 */

#ifndef _SFLIB_HEAP
#define _SFLIB_HEAP

/* ================================================================================================================== */

int heap_init (void);

void * heap_alloc (int size);
void heap_free (void *ptr);

void *heap_extend (void *ptr, int new_size);

int heap_size (void *ptr);


#endif
