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


enum event_icon_type {
	EVENT_ICON_NONE,
	EVENT_ICON_RADIO
};

struct event_radio_icon {

};


struct event_icon_action {
	enum event_icon_type		type;

	union {
		struct event_radio_icon		radio;
	} data;

	struct event_icon_action	*next_icon_action;
	struct event_icon_action	*next_window_action;
};

struct event_icon {
	wimp_i				i;

	struct event_icon_action	*actions;

	struct event_icon		*next_icon;
};

struct event_window {
	wimp_w				w;
	void				(*redraw)(wimp_draw *draw);
	void				(*open)(wimp_open *open);
	void				(*close)(wimp_close *close);
	void				(*leaving)(wimp_leaving *leaving);
	void				(*entering)(wimp_entering *entering);
	void				(*pointer)(wimp_pointer *pointer);
	void				(*key)(wimp_key *key);
	void				(*scroll)(wimp_scroll *scroll);
	void				(*lose_caret)(wimp_caret *caret);
	void				(*gain_caret)(wimp_caret *caret);

	wimp_menu			*menu;
	int				menu_ibar;
	void				(*menu_prepare)(wimp_w w, wimp_menu *m, wimp_pointer *pointer);
	void				(*menu_selection)(wimp_w w, wimp_menu *m, wimp_selection *selection);
	void				(*menu_close)(wimp_w w, wimp_menu *m);
	void				(*menu_warning)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning);

	struct event_icon_action	*actions;
	struct event_icon		*icons;

	void				*data;
	struct event_window		*next;
};

struct event_message_action {
	enum event_message_type		type;
	osbool				(*action)(wimp_message *message);

	struct event_message_action	*next;
};

struct event_message {
	unsigned int			message;
	struct event_message_action	*actions;

	struct event_message		*next;
};

/**
 * Global Variables for the module.
 */

static struct event_message	*event_message_list = NULL;
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
static struct event_message *event_find_message(int message);

/**
 * Accept and process a wimp event.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_process_event(wimp_event_no event, wimp_block *block, int pollword)
{
	struct event_window		*win = NULL;
	struct event_message		*message = NULL;
	struct event_message_action	*action = NULL;
	enum event_message_type		type;
	wimp_pointer			pointer;
	wimp_menu			*menu;
	osbool				handled;

	switch (event) {
	case wimp_NULL_REASON_CODE:
		if (event_drag_null_poll != NULL && (event_drag_null_poll)(event_drag_data) == TRUE)
			return TRUE;
		break;

	case wimp_REDRAW_WINDOW_REQUEST:
		if (block->redraw.w != NULL) {
			win = event_find_window(block->redraw.w);

			if (win != NULL && win->redraw != NULL) {
				(win->redraw)((wimp_draw *) block);
				return TRUE;
			}
		}
		break;

	case wimp_OPEN_WINDOW_REQUEST:
		if (block->open.w != NULL) {
			win = event_find_window(block->open.w);

			if (win != NULL && win->open != NULL) {
				(win->open)((wimp_open *) block);
				return TRUE;
			}
		}
		break;

	case wimp_CLOSE_WINDOW_REQUEST:
		if (block->close.w != NULL) {
			win = event_find_window(block->close.w);

			if (win != NULL && win->close != NULL) {
				(win->close)((wimp_close *) block);
				return TRUE;
			}
		}
		break;

	case wimp_POINTER_LEAVING_WINDOW:
		if (block->leaving.w != NULL) {
			win = event_find_window(block->leaving.w);

			if (win != NULL && win->leaving != NULL) {
				(win->leaving)((wimp_leaving *) block);
				return TRUE;
			}
		}
		break;

	case wimp_POINTER_ENTERING_WINDOW:
		if (block->entering.w != NULL) {
			win = event_find_window(block->entering.w);

			if (win != NULL && win->entering != NULL) {
				(win->entering)((wimp_entering *) block);
				return TRUE;
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
				if (win->menu_ibar) {
					int entry = 0, entries = 0, lines = 0;

					do {
						entries ++;
						if ((win->menu->entries[entry].menu_flags & wimp_MENU_SEPARATE) != 0)
							lines++;
					} while ((win->menu->entries[entry++].menu_flags & wimp_MENU_LAST) == 0);

					menu = create_iconbar_menu (win->menu, (wimp_pointer *) block, entries, lines);
				} else {
					menu = create_standard_menu(win->menu, (wimp_pointer *) block);
				}
				current_menu = win;
				if (menu_handle != NULL)
					*menu_handle = menu;
				return TRUE;
			} else if (win != NULL && win->pointer != NULL) {
				/* Process generic click handlers. */

				(win->pointer)((wimp_pointer *) block);
				return TRUE;
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
			return TRUE;
		}
		break;

	case wimp_KEY_PRESSED:
		if (block->key.w != NULL) {
			win = event_find_window(block->key.w);

			if (win != NULL && win->key != NULL) {
				(win->key)((wimp_key *) block);
				return TRUE;
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
					(current_menu->menu_prepare)(current_menu->w, current_menu->menu, NULL);
				wimp_create_menu(current_menu->menu, 0, 0);
			} else {
				if (current_menu->menu_close != NULL)
					(current_menu->menu_close)(current_menu->w, current_menu->menu);
				current_menu = NULL;
				if (menu_handle != NULL)
					*menu_handle = NULL;
			}
			return TRUE;
		}
		break;

	case wimp_SCROLL_REQUEST:
		if (block->scroll.w != NULL) {
			win = event_find_window(block->scroll.w);

			if (win != NULL && win->scroll != NULL) {
				(win->scroll)((wimp_scroll *) block);
				return TRUE;
			}
		}
		break;

	case wimp_LOSE_CARET:
		if (block->caret.w != NULL) {
			win = event_find_window(block->caret.w);

			if (win != NULL && win->lose_caret != NULL) {
				(win->lose_caret)((wimp_caret *) block);
				return TRUE;
			}
		}
		break;

	case wimp_GAIN_CARET:
		if (block->caret.w != NULL) {
			win = event_find_window(block->caret.w);

			if (win != NULL && win->gain_caret != NULL) {
				(win->gain_caret)((wimp_caret *) block);
				return TRUE;
			}
		}
		break;

	case wimp_USER_MESSAGE:
	case wimp_USER_MESSAGE_RECORDED:
	case wimp_USER_MESSAGE_ACKNOWLEDGE:
		/* First pass the message on to any registered handlers, in
		 * reverse order of registration, until we run out or one of them
		 * returns TRUE to indicate that it has claimed the message.
		 */

		switch (event) {
		case wimp_USER_MESSAGE:
			type = EVENT_MESSAGE;
			break;
		case wimp_USER_MESSAGE_RECORDED:
			type = EVENT_MESSAGE_RECORDED;
			break;
		case wimp_USER_MESSAGE_ACKNOWLEDGE:
			type = EVENT_MESSAGE_ACKNOWLEDGE;
			break;
		default:
			type = EVENT_MESSAGE_NONE;
			break;
		}

		message = event_find_message(block->message.action);
		handled = FALSE;

		if (message != NULL && message->actions != NULL) {
			action = message->actions;

			while (action != NULL && handled == FALSE) {
				if ((action->type & type) != 0 && action->action != NULL)
					handled = action->action((wimp_message *) block);

				action = action->next;
			}
		}

		/* If the message is one that we need, we then process it regardless
		 * of whether any external code has claimed it.
		 */

		if (event != wimp_USER_MESSAGE_ACKNOWLEDGE) {
			switch (block->message.action) {
			case message_MENUS_DELETED:
				if (current_menu != NULL) {
					if (current_menu->menu_close != NULL)
						(current_menu->menu_close)(current_menu->w, current_menu->menu);
					current_menu = NULL;
					if (menu_handle != NULL)
						*menu_handle = NULL;
					handled = TRUE;
				}
				break;

			case message_MENU_WARNING:
				if (current_menu != NULL) {
					if (current_menu->menu_warning != NULL)
						(current_menu->menu_warning)(current_menu->w, current_menu->menu, (wimp_message_menu_warning *) &(block->message.data));
					handled = TRUE;
				}
				break;
			}
		}

		if (handled)
			return TRUE;
		break;
	}

	return FALSE;
}


/**
 * Add a window redraw event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_redraw_event(wimp_w w, void (*callback)(wimp_draw *draw))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->redraw = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a window open event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_open_event(wimp_w w, void (*callback)(wimp_open *open))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->open = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a window close event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_close_event(wimp_w w, void (*callback)(wimp_close *close))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->close = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a pointer leaving event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_pointer_leaving_event(wimp_w w, void (*callback)(wimp_leaving *leaving))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->leaving = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a pointer entering event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_pointer_entering_event(wimp_w w, void (*callback)(wimp_entering *entering))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->entering = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a mouse click (pointer) event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_mouse_event(wimp_w w, void (*callback)(wimp_pointer *pointer))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->pointer = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a keypress event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_key_event(wimp_w w, void (*callback)(wimp_key *key))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->key = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a scroll event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_scroll_event(wimp_w w, void (*callback)(wimp_scroll *scroll))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->scroll = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a lose caret event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_lose_caret_event(wimp_w w, void (*callback)(wimp_caret *caret))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->lose_caret = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a gain caret event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_gain_caret_event(wimp_w w, void (*callback)(wimp_caret *caret))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->gain_caret = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Register a menu to the specified window: this will then be opened whenever
 * there is a menu click within the work area (even over icons).
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu(wimp_w w, wimp_menu *menu, osbool iconbar)
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL) {
		block->menu = menu;
		block->menu_ibar = iconbar;
	}

	return (block == NULL) ? FALSE: TRUE;
}


/**
 * Add a menu prepare event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu_prepare(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_pointer *pointer))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_prepare = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a menu selection event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu_selection(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_selection *selection))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_selection = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a menu close event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu_close(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_close = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a menu warning event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu_warning(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->menu_warning = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Add a user data pointer for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_user_data(wimp_w w, void *data)
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->data = data;

	return (block == NULL) ? FALSE : TRUE;
}


/**
 * Return the user data block associated with the specified window.
 *
 * This function is an external interface, documented in event.h.
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
 * This function is an external interface, documented in event.h.
 */

void event_delete_window(wimp_w w)
{
	struct event_window		*block, **parent;
	struct event_icon		*icon;
	struct event_icon_action	*action;

	block = event_find_window(w);

	if (block != NULL) {
		/* Delete all linked icon and action definitions. */

		while (block->icons != NULL) {
			icon = block->icons;
			block->icons = icon->next_icon;
			free(icon);
		}

		while (block->actions != NULL) {
			action = block->actions;
			block->actions = action->next_window_action;
			free(action);
		}

		/* Delete the window itself. */

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

static struct event_window *event_find_window(wimp_w w)
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

static struct event_window *event_create_window(wimp_w w)
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

		block->actions = NULL;
		block->icons = NULL;

		block->next = event_window_list;
		event_window_list = block;
	}

	return block;
}


/**
 * Add a message handler for the given user message.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_message_handler(unsigned int message, enum event_message_type type, osbool (*message_action)(wimp_message *message))
{
	struct event_message		*block = NULL;
	struct event_message_action	*action = NULL;

	block = event_find_message(message);

	if (block == NULL) {
		block = malloc(sizeof(struct event_message));

		if (block == NULL)
			return FALSE;

		block->message = message;
		block->actions = NULL;
		block->next = event_message_list;
		event_message_list = block;
	}

	/* Create a new action for the message. */

	action = malloc(sizeof(struct event_message_action));

	if (action == NULL)
		return FALSE;

	action->type = type;
	action->action = message_action;
	action->next = block->actions;
	block->actions = action;

	return TRUE;
}

/**
 * Find the message block for the given message.
 *
 * \param message	The message to find the structure for.
 * \return		A pointer to the message structure, or NULL.
 */

static struct event_message *event_find_message(int message)
{
	struct event_message	*block = NULL;

	block = event_message_list;

	while (block != NULL && block->message != message)
		block = block->next;

	return block;
}


/**
 * Set a handler for the next drag box event and any Null Polls in between.
 * If either handler is NULL it will not be called; both will be cancelled on
 * the next User_Drag_Box event to be received.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_drag_handler(void (*drag_end)(wimp_dragged *dragged, void *data), int (*drag_null_poll)(void *data), void *data)
{
	event_drag_end = drag_end;
	event_drag_null_poll = drag_null_poll;
	event_drag_data = data;

	return TRUE;
}


/**
 * Set a variable to store a pointer to the currently open menu block.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_menu_pointer(wimp_menu **menu)
{
	menu_handle = menu;

	return TRUE;
}

