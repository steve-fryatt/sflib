/**
 * \file event.c
 *
 * SFLib - Simple event-based Wimp_Poll dispatch system.
 *
 * Version 0.10 (3 May 2010)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"

/* SFLib Header Files. */

#include "event.h"
#include "menus.h"

/* ANSII C header files. */

#include <assert.h>
#include <stdlib.h>


struct event_window {
	wimp_w			w;
	void			(*redraw)(wimp_draw *draw);
	void			(*open)(wimp_open *open);
	void			(*close)(wimp_close *close);
	void			(*leaving)(wimp_leaving *leaving);
	void			(*entering)(wimp_entering *entering);
	void			(*pointer)(wimp_pointer *pointer);
	void			(*key)(wimp_key *key);
	void			(*scroll)(wimp_scroll *scroll);
	void			(*lose_caret)(wimp_caret *caret);
	void			(*gain_caret)(wimp_caret *caret);

	wimp_menu		*menu;
	int			menu_ibar;
	void			(*menu_prepare)(wimp_w w, wimp_menu *m, wimp_pointer *pointer);
	void			(*menu_selection)(wimp_w w, wimp_menu *m, wimp_selection *selection);
	void			(*menu_close)(wimp_w w, wimp_menu *m);
	void			(*menu_warning)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning);



	void			*data;
	struct event_window	*next;
};

/**
 * Global Variables for the module.
 */

static struct event_window	*event_window_list = NULL;
static struct event_window	*current_menu = NULL;

static wimp_menu		**menu_handle = NULL;

/* User Drag Event Data */

static void (*event_drag_end)(wimp_dragged *dragged, void *data) = NULL;
static int (*event_drag_null_poll)(void *data) = NULL;
static void *event_drag_data = NULL;

/**
 * Function prototypes for internal functions.
 */

static struct event_window *event_find_window(wimp_w w);
static struct event_window *event_create_window(wimp_w w);



/**
 * Accept and process a wimp event.
 *
 * \param  event	The Wimp event code to be handled.
 * \param  block	The Wimp poll block.
 * \param  pollword	The Wimp pollword.
 * \return		Zero if the event was handled; else non-zero.
 */

int event_process_event(wimp_event_no event, wimp_block *block, int pollword)
{
	struct event_window	*win = NULL;
	wimp_pointer		pointer;
	wimp_menu		*menu;

	switch (event) {
	case wimp_NULL_REASON_CODE:
		if (event_drag_null_poll != NULL && (event_drag_null_poll)(event_drag_data) == 0)
			return 0;
		break;

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

	case wimp_POINTER_LEAVING_WINDOW:
		if (block->leaving.w != NULL) {
			win = event_find_window(block->leaving.w);

			if (win != NULL && win->leaving != NULL) {
				(win->leaving)((wimp_leaving *) block);
				return 0;
			}
		}
		break;

	case wimp_POINTER_ENTERING_WINDOW:
		if (block->entering.w != NULL) {
			win = event_find_window(block->entering.w);

			if (win != NULL && win->entering != NULL) {
				(win->entering)((wimp_entering *) block);
				return 0;
			}
		}
		break;

	case wimp_MOUSE_CLICK:
		if (block->pointer.w != NULL) {
			win = event_find_window(block->pointer.w);

			if (win != NULL && block->pointer.buttons == wimp_CLICK_MENU
					&& win->menu != NULL) {
				/* Process window menus on Menu clicks. */

				if (win->menu_prepare != NULL)
					(win->menu_prepare)(win->w, win->menu, (wimp_pointer *) block);
				menu = create_standard_menu(win->menu, (wimp_pointer *) block);
				current_menu = win;
				if (menu_handle != NULL)
					*menu_handle = menu;
				return 0;
			} else if (win != NULL && win->pointer != NULL) {
				/* Process generic click handlers. */

				(win->pointer)((wimp_pointer *) block);
				return 0;
			}
		}
		break;

	case wimp_USER_DRAG_BOX:
		if (event_drag_end != NULL) {
			(event_drag_end)((wimp_dragged *) block, event_drag_data);

			/* One-shot, so clear the function pointer. */

			event_drag_end = NULL;
			event_drag_null_poll = NULL;
			event_drag_data = NULL;
			return 0;
		}
		break;

	case wimp_KEY_PRESSED:
		if (block->key.w != NULL) {
			win = event_find_window(block->key.w);

			if (win != NULL && win->key != NULL) {
				(win->key)((wimp_key *) block);
				return 0;
			}
		}
		break;

	case wimp_MENU_SELECTION:
		if (current_menu != NULL) {
			wimp_get_pointer_info(&pointer);

			if (current_menu->menu_selection != NULL)
				(current_menu->menu_selection)(current_menu->w, current_menu->menu, (wimp_selection *) block);

			if (pointer.buttons == wimp_CLICK_ADJUST) {
				if (current_menu->menu_prepare != NULL)
					(current_menu->menu_prepare)(current_menu->w, current_menu->menu, &pointer);
				wimp_create_menu(current_menu->menu, 0, 0);
			} else {
				if (current_menu->menu_close != NULL)
					(current_menu->menu_close)(current_menu->w, current_menu->menu);
				current_menu = NULL;
				if (menu_handle != NULL)
					*menu_handle = NULL;
			}
			return 0;
		}
		break;

	case wimp_SCROLL_REQUEST:
		if (block->scroll.w != NULL) {
			win = event_find_window(block->scroll.w);

			if (win != NULL && win->scroll != NULL) {
				(win->scroll)((wimp_scroll *) block);
				return 0;
			}
		}
		break;

	case wimp_LOSE_CARET:
		if (block->caret.w != NULL) {
			win = event_find_window(block->caret.w);

			if (win != NULL && win->lose_caret != NULL) {
				(win->lose_caret)((wimp_caret *) block);
				return 0;
			}
		}
		break;

	case wimp_GAIN_CARET:
		if (block->caret.w != NULL) {
			win = event_find_window(block->caret.w);

			if (win != NULL && win->gain_caret != NULL) {
				(win->gain_caret)((wimp_caret *) block);
				return 0;
			}
		}
		break;

	case wimp_USER_MESSAGE:
	case wimp_USER_MESSAGE_RECORDED:
		switch (block->message.action) {
		case message_MENUS_DELETED:
			if (current_menu != NULL) {
				if (current_menu->menu_close != NULL)
					(current_menu->menu_close)(current_menu->w, current_menu->menu);
				current_menu = NULL;
				if (menu_handle != NULL)
					*menu_handle = NULL;
				return 0;
			}
			break;

		case message_MENU_WARNING:
			if (current_menu != NULL) {
				if (current_menu->menu_warning != NULL)
					(current_menu->menu_warning)(current_menu->w, current_menu->menu, (wimp_message_menu_warning *) &(block->message.data));
				return 0;
			}
			break;
		}
		break;
	}

	return 1;
}


/**
 * Add a window redraw event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback function to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
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
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback function to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
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
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback function to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
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
 * Add a pointer leaving event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_pointer_leaving_event(wimp_w w, void (*callback)(wimp_leaving *leaving))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->leaving = callback;

	return (block == NULL);
}


/**
 * Add a pointer entering event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_pointer_entering_event(wimp_w w, void (*callback)(wimp_entering *entering))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->entering = callback;

	return (block == NULL);
}


/**
 * Add a mouse click (pointer) event handler for the specified window.
 *
 * If the window has a window menu attached, this handler is not called for
 * Menu clicks over the work area.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_mouse_event(wimp_w w, void (*callback)(wimp_pointer *pointer))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->pointer = callback;

	return (block == NULL);
}


/**
 * Add a keypress event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_key_event(wimp_w w, void (*callback)(wimp_key *key))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->key = callback;

	return (block == NULL);
}


/**
 * Add a scroll event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_scroll_event(wimp_w w, void (*callback)(wimp_scroll *scroll))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->scroll = callback;

	return (block == NULL);
}


/**
 * Add a lose caret event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_lose_caret_event(wimp_w w, void (*callback)(wimp_caret *caret))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->lose_caret = callback;

	return (block == NULL);
}


/**
 * Add a gain caret event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_gain_caret_event(wimp_w w, void (*callback)(wimp_caret *caret))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->gain_caret = callback;

	return (block == NULL);
}

/**
 * Register a menu to the specified window: this will then be opened whenever
 * there is a menu click within the work area (even over icons).
 *
 * If a menu is registered, no events related to it will be passed back from
 * event_process_event() -- even if specific handlers are registed as NULL.
 *
 * \param  w		The window handle to attach the menu to.
 * \param  *menu	The menu handle.
 * \param  iconbar	1 if the menu is an iconbar menu; else 0.
 * \return		0 if the handler was registered; else 1.
 */

int event_add_window_menu(wimp_w w, wimp_menu *menu, int iconbar)
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL) {
		block->menu = menu;
		block->menu_ibar = iconbar;
	}

	return (block == NULL);
}


/**
 * Add a menu prepare event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_menu_prepare(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_pointer *pointer))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_prepare = callback;

	return (block == NULL);
}


/**
 * Add a menu selection event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_menu_selection(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_selection *selection))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_selection = callback;

	return (block == NULL);
}


/**
 * Add a menu close event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_menu_close(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_close = callback;

	return (block == NULL);
}


/**
 * Add a menu warning event handler for the specified window.
 *
 * \param  w		The window handle to attach the action to.
 * \param  *callback()	The callback to use on the event.
 * \return		Zero if the handler was registered; else non-zero.
 */

int event_add_window_menu_warning(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_warning = callback;

	return (block == NULL);
}

/**
 * Add a user data pointer for the specified window.
 *
 * \param  w		The window handle to attach the data to.
 * \param  *data	The data to attach.
 * \return		Zero if the handler was registered; else non-zero.
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
 * \param  w		The window to locate the data for.
 * \return		A pointer to the user data, or NULL.
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
 * \param  w		The window to remove the data for.
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

		if (block == current_menu)
			current_menu = NULL;
		free(block);
	}
}

/**
 * Find the window data block for the given window.
 *
 * \param  w		The window handle to find the structure for.
 * \return		A pointer to the window structure, or NULL.
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
 * \param  w		The window handle to create a new structure for.
 * \return		A pointer to the window structure, or NULL.
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
		block->leaving = NULL;
		block->entering = NULL;
		block->pointer = NULL;
		block->key = NULL;
		block->scroll = NULL;
		block->lose_caret = NULL;
		block->gain_caret = NULL;

		block->menu = NULL;
		block->menu_ibar = 0;

		block->menu_prepare = NULL;
		block->menu_selection = NULL;
		block->menu_close = NULL;
		block->menu_warning = NULL;

		block->data = NULL;

		block->next = event_window_list;
		event_window_list = block;
	}

	return block;
}


/**
 * Set a handler for the next drag box event and any Null Polls in between.
 * If either handler is NULL it will not be called; both will be cancelled on
 * the next User_Drag_Box event to be received.
 *
 * Null Polls can be passed on to the application by returning 1 from
 * (drag_null_poll)(); returning 0 causes event_process_event() to also return 0.
 *
 * \param  *drag_end		A callback function for the drag end event.
 * \param  *drag_null_poll	A callback function for Null Polls during the drag.
 * \param  *data		Private data to be passed to the callback routines.
 * \return			Zero if the handler was registerd; else non-Zero.
 */

int event_set_drag_handler(void (*drag_end)(wimp_dragged *dragged, void *data), int (*drag_null_poll)(void *data), void *data)
{
	event_drag_end = drag_end;
	event_drag_null_poll = drag_null_poll;
	event_drag_data = data;

	return 0;
}


/**
 * Set a variable to store a pointer to the currently open menu block.
 *
 * \param  **menu		Pointer to a wimp_menu * to take the pointers.
 * \return			Zero if the variable was registerd; else non-Zero.Ze
 */

int event_set_menu_pointer(wimp_menu **menu)
{
	menu_handle = menu;

	return 1;
}

