/* SF-Lib - Heap.c
 *
 * Version 0.10 (20 September 2003)
 */

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

/* ANSII C header files. */

/* ================================================================================================================== */

static byte *heap_anchor = NULL;
static int  heap_block_size;

#define HEAP_GRANULARITY 1024
#define HEAP_BLOCK_OHEAD 16

/* ================================================================================================================== */

int heap_init (void)
{
  os_error *error;

  heap_block_size = HEAP_GRANULARITY;

  flex_alloc ((flex_ptr) &heap_anchor, heap_block_size);

  error = xosheap_initialise (heap_anchor, heap_block_size);
  if (error != NULL);
  {
    wimp_program_report (error);
  }

  return 1;
}

/* ================================================================================================================== */

void * heap_alloc (int size)
{
  void     *block;
  os_error *error;

  size += sizeof (int);

  error = xosheap_alloc (heap_anchor, size, &block);
  if (error != NULL)
  {
    if (flex_extend ((flex_ptr) &heap_anchor, heap_block_size + size + HEAP_BLOCK_OHEAD))
    {
      osheap_resize (heap_anchor, size + HEAP_BLOCK_OHEAD);
      heap_block_size += (size + HEAP_BLOCK_OHEAD);

      error = xosheap_alloc (heap_anchor, size, &block);
      if (error != NULL)
      {
        wimp_program_report (error);
      }
    }
    else
    {
      block = NULL;
    }
  }

  // debug_printf ("\\O*** Alloc ***");
  // debug_printf ("Claimed memory at %x (%d bytes)", block, size);

  if (block != NULL)
  {
    *(int *)block = size - sizeof (int);
    block = (int *) block + 1;
  }

  // debug_printf ("Returned pointer to %x", block);

  return (block);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void heap_free (void *ptr)
{
  int shrink;

  // debug_printf ("\\O*** Free ***");
  // debug_printf ("Taken pointer to %x", ptr);

  ptr = (int *) ptr - 1;

  // debug_printf ("Freed memory from %x (%d bytes)", ptr, *(int *) ptr);

  osheap_free (heap_anchor, ptr);

  shrink = osheap_resize_no_fail (heap_anchor, 0x80000000);

  if (shrink < 0)
  {
    heap_block_size += shrink;
    flex_extend ((flex_ptr) &heap_anchor, heap_block_size);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void *heap_extend (void *ptr, int new_size)
{
  int      change;
  void     *block;
  os_error *error;

  // debug_printf ("\\O*** Extend ***");
  // debug_printf ("Taken pointer to %x", ptr);

  new_size += sizeof (int);

  ptr = (int *) ptr - 1;

  change = new_size - *(int *)ptr;

  // debug_printf ("Resized memory at %x (%d bytes)", ptr, *(int *)ptr);

  error = xosheap_realloc (heap_anchor, ptr, change, &block);
  if (error != NULL)
  {
    if (flex_extend ((flex_ptr) &heap_anchor, heap_block_size + new_size + HEAP_BLOCK_OHEAD))
    {
      osheap_resize (heap_anchor, new_size + HEAP_BLOCK_OHEAD);
      heap_block_size += (new_size + HEAP_BLOCK_OHEAD);

      error = xosheap_realloc (heap_anchor, ptr, change, &block);
      if (error != NULL)
      {
        wimp_program_report (error);
      }
    }
    else
    {
      block = NULL;
    }
  }

  // debug_printf ("Resized memory to %x (%d bytes)", block, new_size);

  if (block != NULL)
  {
    *(int *)block = new_size - sizeof (int);
    block = (int *) block + 1;
  }

  // debug_printf ("Returned pointer to %x", block);

  return (block);
}

/* ================================================================================================================== */

int heap_size (void *ptr)
{
  return *((int *)ptr - 1);
}
