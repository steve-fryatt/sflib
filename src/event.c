/* SF-Lib - Event.c
 *
 * Version 0.10 (3 May 2010)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"

/* ANSII C header files. */

#include <assert.h>
#include <stdlib.h>


struct event_window {
	wimp_w			w;
	void			(*redraw)(wimp_draw *draw);
	void			(*open)(wimp_open *open);
	void			(*close)(wimp_close *close);
	void			(*pointer)(wimp_pointer *pointer);
	void			(*key)(wimp_key *key);
	void			*data;
	struct event_window	*next;
};

/**
 * Global Variables for the module.
 */

static struct event_window	*event_window_list = NULL;

/**
 * Function prototypes for internal functions.
 */

static struct event_window *event_find_window(wimp_w w);
static struct event_window *event_create_window(wimp_w w);



/**
 * Accept and process a wimp event.
 *
 * Param:  event	The Wimp event code to be handled.
 * Param:  block	The Wimp poll block.
 * Param:  pollword	The Wimp pollword.
 * Return:		Zero if the event was handled; else non-zero.
 */

int event_process_event(wimp_event_no event, wimp_block *block, int pollword)
{
	struct event_window	*win = NULL;

	switch (event) {
	case wimp_REDRAW_WINDOW_REQUEST:
		if (block->redraw.w != NULL) {
			win = event_find_window(block->redraw.w);

			if (win != NULL && win->redraw != NULL) {
				(win->redraw)((wimp_draw *) block);
				return 0;
			}
		}
		break;

	case wimp_OPEN_WINDOW_REQUEST:
		if (block->open.w != NULL) {
			win = event_find_window(block->open.w);

			if (win != NULL && win->open != NULL) {
				(win->open)((wimp_open *) block);
				return 0;
			}
		}
		break;

	case wimp_CLOSE_WINDOW_REQUEST:
		if (block->close.w != NULL) {
			win = event_find_window(block->close.w);

			if (win != NULL && win->close != NULL) {
				(win->close)((wimp_close *) block);
				return 0;
			}
		}
		break;
	}

	return 1;
}


/**
 * Add a window redraw event handler for the specified window.
 *
 * Param:  w		The window handle to attach the action to.
 * Param:  *redraw()	The callback function to use on the event.
 * Return:		Zero if the handler was registered; else non-zero.
 */

int event_add_window_redraw_event(wimp_w w, void (*callback)(wimp_draw *draw))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->redraw = callback;

	return (block == NULL);
}


/**
 * Add a window open event handler for the specified window.
 *
 * Param:  w		The window handle to attach the action to.
 * Param:  *open()	The callback function to use on the event.
 * Return:		Zero if the handler was registered; else non-zero.
 */

int event_add_window_open_event(wimp_w w, void (*callback)(wimp_open *open))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->open = callback;

	return (block == NULL);
}


/**
 * Add a window close event handler for the specified window.
 *
 * Param:  w		The window handle to attach the action to.
 * Param:  *close()	The callback function to use on the event.
 * Return:		Zero if the handler was registered; else non-zero.
 */

int event_add_window_close_event(wimp_w w, void (*callback)(wimp_close *close))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->close = callback;

	return (block == NULL);
}


/**
 * Add a user data pointer for the specified window.
 *
 * Param:  w		The window handle to attach the data to.
 * Param:  *data	The data to attach.
 * Return:		Zero if the handler was registered; else non-zero.
 */

int event_add_window_user_data(wimp_w w, void *data)
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->data = data;

	return (block == NULL);
}


/**
 * Return the user data block associated with the specified window.
 *
 * Param:  w		The window to locate the data for.
 * Return:		A pointer to the user data, or NULL.
 */

void *event_get_window_user_data(wimp_w w)
{
	void			*data = NULL;
	struct event_window	*block;

	block = event_find_window(w);
	if (block != NULL)
		data = block->data;

	return data;
}


/**
 * Remove a window and its associated event details from the records.
 *
 * Param:  w		The window to remove the data for.
 */

void event_delete_window(wimp_w w)
{
	struct event_window	*block, **parent;

	block = event_find_window(w);

	if (block != NULL) {
		parent = &event_window_list;

		while (*parent != NULL && *parent != block)
				parent = &((*parent)->next);

		assert(*parent != NULL);

		*parent = block->next;

		free(block);
	}
}

/**
 * Find the window data block for the given window.
 *
 * Param:  w		The window handle to find the structure for.
 * Return:		A pointer to the window structure, or NULL.
 */

struct event_window *event_find_window(wimp_w w)
{
	struct event_window	*block = NULL;

	if (w != NULL) {
		block = event_window_list;

		while (block != NULL && block->w != w)
			block = block->next;
	}

	return block;
}


/**
 * Create a new window data block for the given window.
 *
 * Param:  w		The window handle to create a new structure for.
 * Return:		A pointer to the window structure, or NULL.
 */

struct event_window *event_create_window(wimp_w w)
{
	struct event_window	*block;

	/* Just in case we try and create a new block for an existing window. */

	block = event_find_window(w);

	if (block != NULL)
		return block;

	/* There isn't a block in the list, so create and link a new one. */

	block = (struct event_window *) malloc(sizeof(struct event_window));

	if (block != NULL) {
		block->w = w;

		block->redraw = NULL;
		block->open = NULL;
		block->close = NULL;
		block->pointer = NULL;
		block->key = NULL;

		block->data = NULL;

		block->next = event_window_list;
		event_window_list = block;
	}

	return block;
}
