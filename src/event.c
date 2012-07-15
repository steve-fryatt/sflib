/**
 * \file event.c
 *
 * (C) Stephen Fryatt, 2010-2011
 *
 * SFLib - Simple event-based Wimp_Poll dispatch system.
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"

/* SFLib Header Files. */

#include "event.h"
#include "icons.h"
#include "menus.h"

/* ANSII C header files. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define EVENT_TOKEN_INDEX_LEN 12											/**< The number of digits in a message token index.			*/

/**
 * Menu types, to identify which type of menu handler needs to be called
 * to process incoming menu events.
 */

enum event_menu_type {
	EVENT_MENU_NONE = 0,												/**< No menu: should not be used.					*/
	EVENT_MENU_WINDOW,												/**< Window menu: the menu opened when clicking Menu over a window.	*/
	EVENT_MENU_POPUP_AUTO,												/**< Popup Auto menu: popup menus handled automatically.		*/
	EVENT_MENU_POPUP_MANUAL												/**< Popup Manual menu: popup menus handled via client events.		*/
};

/**
 * Icon types, to identify which type of icon handler needs to be called
 * to process incoming icon events.
 */

enum event_icon_type {
	EVENT_ICON_NONE = 0,												/**< No action: should not be used.					*/
	EVENT_ICON_CLICK,												/**< Click: pass a click event back to the client's handler.		*/
	EVENT_ICON_RADIO,												/**< Radio: reselct the icon after Adjust clicks.			*/
	EVENT_ICON_POPUP_AUTO,												/**< Popup Auto: open a Popup Auto menu.				*/
	EVENT_ICON_POPUP_MANUAL												/**< Popup Manual: open a Popup Manual menu.				*/
};

/**
 * Specific details of the EVENT_ICON_CLICK icon action type.
 */

struct event_icon_click {
	osbool				(*callback)(wimp_pointer *pointer);						/**< Callback function for the icon click.				*/
};

/**
 * Specific details of the EVENT_ICON_RADIO icon action type.
 */

struct event_icon_radio {
	osbool				complete;									/**< TRUE if no further processing is required by the task.		*/
};

/**
 * Specific details of the EVENT_ICON_POPUP_AUTO and EVENT_ICON_POPUP_MANUAL
 * icon action types.
 */

struct event_icon_popup {
	wimp_menu			*menu;										/**< The menu associated with the popup.				*/
	wimp_i				field;										/**< The display field icon attached to the popup menu.			*/
	char				*token;										/**< The message token group to be used for the field entries.		*/
	char				*token_number;									/**< Pointer to the end of the token name, to place the index.		*/
	unsigned			selection;									/**< The currently selected item in the menu.				*/
};

/**
 * Details of an icon click action: one or more of these can be chained to
 * a struct event_icon, and each will be actioned when a click event
 * is received.
 */

struct event_icon_action {
	enum event_icon_type		type;										/**< The event type, which determines what data is in the union.	*/

	union {
		struct event_icon_click		click;									/**< Data for an EVENT_ICON_CLICK.					*/
		struct event_icon_radio		radio;									/**< Data for an EVENT_ICON_RADIO.					*/
		struct event_icon_popup		popup;									/**< Data for an EVENT_ICON_POPUP_AUTO or EVENT_ICON_POPUP_MANUAL.	*/
	} data;														/**< The data for the icon event.					*/

	struct event_icon_action	*next;										/**< Pointer to the next action for the icon, or NULL.			*/
};

/**
 * Details of an icon in a window.
 */

struct event_icon {
	wimp_i				i;										/**< The Wimp icon handle for the icon.					*/

	struct event_icon_action	*actions;									/**< Pointer to chain of actions which relate to the icon.		*/

	struct event_icon		*next;										/**< Pointer to the next icon in the window, or NULL.			*/
};

/**
 * Details of a window, with all of the event handlers relating to it.
 */

struct event_window {
	wimp_w				w;										/**< The Wimp window handle for the window.				*/

	void				(*redraw)(wimp_draw *draw);							/**< Callback handler for Window Redraw events, or NULL.		*/
	void				(*open)(wimp_open *open);							/**< Callback handler for Window Open events, or NULL.			*/
	void				(*close)(wimp_close *close);							/**< Callback handler for Window Close events, or NULL.			*/
	void				(*leaving)(wimp_leaving *leaving);						/**< Callback handler for Pointer Leaving events, or NULL.		*/
	void				(*entering)(wimp_entering *entering);						/**< Callback handler for Pointer Entering events, or NULL.		*/
	void				(*pointer)(wimp_pointer *pointer);						/**< Callback handler for Pointer events, or NULL.			*/
	osbool				(*key)(wimp_key *key);								/**< Callback handler for Keypress events, or NULL.			*/
	void				(*scroll)(wimp_scroll *scroll);							/**< Callback handler for Scroll events, or NULL.			*/
	void				(*lose_caret)(wimp_caret *caret);						/**< Callback handler for Lose Caret events, or NULL.			*/
	void				(*gain_caret)(wimp_caret *caret);						/**< Callback handler for Gain Caret events, or NULL.			*/

	wimp_menu			*menu;										/**< Wimp window block for the Window Menu, or NULL.			*/
	void				(*menu_prepare)(wimp_w w, wimp_menu *m, wimp_pointer *pointer);			/**< Callback handler for Menu Prepare events, or NULL.			*/
	void				(*menu_selection)(wimp_w w, wimp_menu *m, wimp_selection *selection);		/**< Callback handler for Menu Selection events, or NULL.		*/
	void				(*menu_close)(wimp_w w, wimp_menu *m);						/**< Callback handler for Menu Close events, or NULL.			*/
	void				(*menu_warning)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning);	/**< Callback handler for Menu Warning events, or NULL.			*/

	struct event_icon		*icons;										/**< Pointer to the chain of icons in the window, or NULL.		*/

	void				*data;										/**< Client data pointer.						*/
	struct event_window		*next;										/**< Pointer to the next window in the chain, or NULL.			*/
};

/**
 * Details of an action to be carried out on receipt of a Wimp Message.
 * One or more of these structures can be chained to a
 * struct event_message, and each will be called in sequence until
 * one of the handlers claims the event.
 */

struct event_message_action {
	enum event_message_type		type;										/**< The type of action: Standard, Recorded, Acknowledge, etc.		*/
	osbool				(*action)(wimp_message *message);						/**< Callback handler to be called when the requirements are met.	*/

	struct event_message_action	*next;										/**< Pointer to the next handler in the chain, or NULL.			*/
};

/**
 * Details of a Wimp Message, with a list of actions to be carried out
 * on receipt.
 */

struct event_message {
	unsigned int			message;									/**< The Wimp Message number.						*/
	struct event_message_action	*actions;									/**< Pointer to a chain of actions for the message, or NULL.		*/

	struct event_message		*next;										/**< Pointer to the next message in the chain, or NULL.			*/
};

/**
 * Global Variables for the module.
 */

static struct event_message	*event_message_list = NULL;
static struct event_window	*event_window_list = NULL;

static struct event_window	*current_menu = NULL;
static enum event_menu_type	current_menu_type = EVENT_MENU_NONE;
static struct event_icon	*current_menu_icon = NULL;
static struct event_icon_action	*current_menu_action = NULL;

static wimp_menu		**menu_handle = NULL;
static wimp_menu		*new_client_menu = NULL;			/**< Used for returning menu updates from callbacks. */


/* User Drag Event Data */

static void (*event_drag_end)(wimp_dragged *dragged, void *data) = NULL;
static osbool (*event_drag_null_poll)(void *data) = NULL;
static void *event_drag_data = NULL;

/**
 * Function prototypes for internal functions.
 */

static osbool event_process_icon(struct event_window *window, struct event_icon *icon, wimp_pointer *pointer);
static void event_prepare_auto_menu(struct event_window *window, struct event_icon_action *action);
static void event_set_auto_menu_selection(struct event_window *window, struct event_icon_action *action, unsigned selection);
static struct event_window *event_find_window(wimp_w w);
static struct event_window *event_create_window(wimp_w w);
static void event_delete_icon_block(struct event_window *window, struct event_icon *icon);
static struct event_icon *event_find_icon(struct event_window *window, wimp_i i);
static struct event_icon *event_create_icon(struct event_window *window, wimp_i i);
static struct event_icon_action *event_find_action(struct event_icon *icon, enum event_icon_type type);
static struct event_icon_action *event_create_action(struct event_icon *icon, enum event_icon_type type);
static struct event_message *event_find_message(int message);

/* Accept and process a wimp event.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_process_event(wimp_event_no event, wimp_block *block, int pollword)
{
	struct event_window			*win = NULL;
	struct event_icon			*icon = NULL;
	struct event_message			*message = NULL;
	struct event_message_action		*action = NULL;
	enum event_message_type			type;
	wimp_pointer				pointer;
	wimp_menu				*menu;
	osbool					handled, special;
	wimp_full_message_menus_deleted		*menus_deleted;

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

				new_client_menu = NULL;
				if (win->menu_prepare != NULL)
					(win->menu_prepare)(win->w, win->menu, (wimp_pointer *) block);
				if (new_client_menu != NULL)
					win->menu = new_client_menu;
				if (win->w == wimp_ICON_BAR)
					menu = menus_create_iconbar_menu(win->menu, (wimp_pointer *) block);
				else
					menu = menus_create_standard_menu(win->menu, (wimp_pointer *) block);
				current_menu = win;
				current_menu_type = EVENT_MENU_WINDOW;
				current_menu_icon = NULL;
				current_menu_action = NULL;
				if (menu_handle != NULL)
					*menu_handle = menu;
				return TRUE;
			} else if (win != NULL) {
				/* Try to process an icon handler. */
				icon = event_find_icon(win, block->pointer.i);

				if (icon != NULL && event_process_icon(win, icon, (wimp_pointer *) block))
					return TRUE;

				/* Process generic click handlers. */

				if (win->pointer != NULL) {
					(win->pointer)((wimp_pointer *) block);
					return TRUE;
				}
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
				if (!(win->key)((wimp_key *) block))
					wimp_process_key(block->key.c);
				return TRUE;
			}
		}
		break;

	case wimp_MENU_SELECTION:
		if (current_menu != NULL) {
			wimp_get_pointer_info(&pointer);

			menu = NULL;

			switch (current_menu_type) {
			case EVENT_MENU_WINDOW:
				menu = current_menu->menu;
				break;
			case EVENT_MENU_POPUP_AUTO:
			case EVENT_MENU_POPUP_MANUAL:
				menu = current_menu_action->data.popup.menu;
				break;
			default:
				/* Something's wrong: tidy up and get out. */
				current_menu = NULL;
				current_menu_type = EVENT_MENU_NONE;
				current_menu_icon = NULL;
				current_menu_action = NULL;
				if (menu_handle != NULL)
					*menu_handle = NULL;
				return TRUE;
				break;
			}

			if (current_menu_type == EVENT_MENU_POPUP_AUTO && block->selection.items[0] != -1)
				event_set_auto_menu_selection(current_menu, current_menu_action, block->selection.items[0]);

			if (current_menu->menu_selection != NULL)
				(current_menu->menu_selection)(current_menu->w, menu, (wimp_selection *) block);

			if (pointer.buttons == wimp_CLICK_ADJUST) {
				new_client_menu = NULL;
				if (current_menu->menu_prepare != NULL && current_menu_type != EVENT_MENU_POPUP_AUTO)
					(current_menu->menu_prepare)(current_menu->w, menu, NULL);
				if (new_client_menu != NULL) {
					switch (current_menu_type) {
					case EVENT_MENU_WINDOW:
						current_menu->menu = new_client_menu;
						break;
					case EVENT_MENU_POPUP_MANUAL:
					case EVENT_MENU_POPUP_AUTO:
						current_menu_action->data.popup.menu = new_client_menu;
						break;
					default:
						new_client_menu = NULL;
						break;
					}
					if (menu_handle != NULL && new_client_menu != NULL)
						*menu_handle = new_client_menu;
				}
				if (new_client_menu == NULL || menu == new_client_menu) {
					if (new_client_menu != NULL)
						menu = new_client_menu;
					if (current_menu_type == EVENT_MENU_POPUP_AUTO)
						event_prepare_auto_menu(current_menu, current_menu_action);
					wimp_create_menu(menu, 0, 0);
				}
			} else {
				if (current_menu->menu_close != NULL && current_menu_type != EVENT_MENU_POPUP_AUTO)
					(current_menu->menu_close)(current_menu->w, menu);
				current_menu = NULL;
				current_menu_type = EVENT_MENU_NONE;
				current_menu_icon = NULL;
				current_menu_action = NULL;
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
		/* If the message is one that we need, we then process it first before
		 * anything else can get at it.
		 */

		special = FALSE;

		if (event != wimp_USER_MESSAGE_ACKNOWLEDGE) {
			switch (block->message.action) {
			case message_MENUS_DELETED:
				menus_deleted = (wimp_full_message_menus_deleted *) block;
				if (current_menu != NULL && ((current_menu_type == EVENT_MENU_WINDOW && current_menu->menu == menus_deleted->menu) ||
						((current_menu_type == EVENT_MENU_POPUP_MANUAL || current_menu_type == EVENT_MENU_POPUP_MANUAL) &&
								current_menu_action->data.popup.menu == menus_deleted->menu))  ) {
					if (current_menu->menu_close != NULL && current_menu_type != EVENT_MENU_POPUP_AUTO)
						(current_menu->menu_close)(current_menu->w, current_menu->menu);
					current_menu = NULL;
					current_menu_type = EVENT_MENU_NONE;
					current_menu_icon = NULL;
					current_menu_action = NULL;
					if (menu_handle != NULL)
						*menu_handle = NULL;
					special = TRUE;
				}
				break;

			case message_MENU_WARNING:
				if (current_menu != NULL) {
					if (current_menu->menu_warning != NULL && current_menu_type != EVENT_MENU_POPUP_AUTO)
						(current_menu->menu_warning)(current_menu->w, current_menu->menu, (wimp_message_menu_warning *) &(block->message.data));
					special = TRUE;
				}
				break;
			}
		}

		/* Then pass the message on to any registered handlers, in
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

		/* Always report as handled if this is a message that we processed
		 * at the start.
		 */

		if (handled || special)
			return TRUE;
		break;
	}

	return FALSE;
}


/**
 * Handle mouse click events on an icon in a window.
 *
 * \param *window		The window block to handle.
 * \param *icon			The icon block to handle.
 * \param *pointer		The Wimp Event block.
 * \return 			TRUE if the event was handled; else FALSE.
 */

static osbool event_process_icon(struct event_window *window, struct event_icon *icon, wimp_pointer *pointer)
{
	struct event_icon_action	*action;
	osbool				handled = TRUE;
	wimp_menu			*menu;

	action = icon->actions;

	while (action != NULL) {
		switch (action->type) {
		case EVENT_ICON_CLICK:
			if (action->data.click.callback != NULL)
				handled = action->data.click.callback(pointer);
			break;

		case EVENT_ICON_RADIO:
			if (pointer->buttons == wimp_CLICK_ADJUST)
				icons_set_selected(pointer->w, pointer->i, TRUE);
			handled = action->data.radio.complete;
			break;

		case EVENT_ICON_POPUP_AUTO:
		case EVENT_ICON_POPUP_MANUAL:
			new_client_menu = NULL;
			if (window->menu_prepare != NULL)
				(window->menu_prepare)(window->w, action->data.popup.menu, pointer);
			if (new_client_menu != NULL)
				action->data.popup.menu = new_client_menu;
			if (action->type == EVENT_ICON_POPUP_AUTO)
				event_prepare_auto_menu(window, action);
			menu = menus_create_popup_menu(action->data.popup.menu, pointer);

			current_menu = window;
			current_menu_icon = icon;
			current_menu_action = action;
			current_menu_type = (action->type == EVENT_ICON_POPUP_AUTO) ? EVENT_MENU_POPUP_AUTO : EVENT_MENU_POPUP_MANUAL;
			if (menu_handle != NULL)
				*menu_handle = menu;
			handled = TRUE;

		default:
			break;
		}

		action = action->next;
	}

	return handled;
}


/* Add a window redraw event handler for the specified window.
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


/* Add a window open event handler for the specified window.
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


/* Add a window close event handler for the specified window.
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


/* Add a pointer leaving event handler for the specified window.
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


/* Add a pointer entering event handler for the specified window.
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


/* Add a mouse click (pointer) event handler for the specified window.
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


/* Add a keypress event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_key_event(wimp_w w, osbool (*callback)(wimp_key *key))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL)
		block->key = callback;

	return (block == NULL) ? FALSE : TRUE;
}


/* Add a scroll event handler for the specified window.
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


/* Add a lose caret event handler for the specified window.
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


/* Add a gain caret event handler for the specified window.
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


/* Register a menu to the specified window: this will then be opened whenever
 * there is a menu click within the work area (even over icons).
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu(wimp_w w, wimp_menu *menu)
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL) {
		block->menu = menu;
		event_add_message_handler(message_MENUS_DELETED, EVENT_MESSAGE_INCOMING, NULL);
	}

	return (block == NULL) ? FALSE: TRUE;
}


/* Add a menu prepare event handler for the specified window.
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


/* Add a menu selection event handler for the specified window.
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


/* Add a menu close event handler for the specified window.
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


/* Add a menu warning event handler for the specified window.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_menu_warning(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning))
{
	struct event_window	*block;

	block = event_create_window(w);

	if (block != NULL) {
		block->menu_warning = callback;
		event_add_message_handler(message_MENU_WARNING, EVENT_MESSAGE_INCOMING, NULL);
	}

	return (block == NULL) ? FALSE : TRUE;
}


/* Add an icon click handler for the specified window and icon.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_icon_click(wimp_w w, wimp_i i, osbool (*callback)(wimp_pointer *pointer))
{
	struct event_window		*window;
	struct event_icon		*icon;
	struct event_icon_action	*action;

	window = event_create_window(w);

	if (window == NULL)
		return FALSE;

	icon = event_create_icon(window, i);

	if (icon == NULL)
		return FALSE;

	action = event_create_action(icon, EVENT_ICON_CLICK);

	if (action == NULL)
		return FALSE;

	action->data.click.callback = callback;

	return TRUE;
}


/* Add a radio icon handler for the specified window and icon.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_icon_radio(wimp_w w, wimp_i i, osbool complete)
{
	struct event_window		*window;
	struct event_icon		*icon;
	struct event_icon_action	*action;

	window = event_create_window(w);

	if (window == NULL)
		return FALSE;

	icon = event_create_icon(window, i);

	if (icon == NULL)
		return FALSE;

	action = event_create_action(icon, EVENT_ICON_RADIO);

	if (action == NULL)
		return FALSE;

	action->data.radio.complete = complete;

	return TRUE;
}


/* Add a pop-up menu handler for the specified window and icon.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_window_icon_popup(wimp_w w, wimp_i i, wimp_menu *menu, wimp_i field, char *token)
{
	struct event_window		*window;
	struct event_icon		*icon;
	struct event_icon_action	*action;

	window = event_create_window(w);

	if (window == NULL)
		return FALSE;

	icon = event_create_icon(window, i);

	if (icon == NULL)
		return FALSE;

	/* An icon can't have both Auto and Manual menus attached at the same time! */

	if (event_find_action(icon, (field == -1) ? EVENT_ICON_POPUP_AUTO : EVENT_ICON_POPUP_MANUAL) != NULL)
		return FALSE;

	action = event_create_action(icon, (field == -1) ? EVENT_ICON_POPUP_MANUAL : EVENT_ICON_POPUP_AUTO);

	if (action == NULL)
		return FALSE;

	if (token != NULL) {
		action->data.popup.token = malloc(strlen(token) + EVENT_TOKEN_INDEX_LEN + 1);
		if (action->data.popup.token != NULL) {
			strcpy(action->data.popup.token, token);
			action->data.popup.token_number = action->data.popup.token + strlen(token);
		} else {
			action->data.popup.token_number = NULL;
		}
	}

	action->data.popup.menu = menu;
	action->data.popup.field = field;

	event_add_message_handler(message_MENUS_DELETED, EVENT_MESSAGE_INCOMING, NULL);

	return TRUE;
}


/**
 * Prepare an auto menu for opening, by ticking the entries in relation to the
 * current selection.
 *
 * \param *window		The window containing the menu.
 * \param *action		The action block for the popup menu.
 */

static void event_prepare_auto_menu(struct event_window *window, struct event_icon_action *action)
{
	int	line = 0;

	if (window == NULL || action == NULL || action->type != EVENT_ICON_POPUP_AUTO)
		return;

	do {
		if (action->data.popup.selection == line)
			action->data.popup.menu->entries[line].menu_flags |= wimp_MENU_TICKED;
		else
			action->data.popup.menu->entries[line].menu_flags &= ~wimp_MENU_TICKED;
	} while ((action->data.popup.menu->entries[line++].menu_flags & wimp_MENU_LAST) == 0);
}


/* Set the menu to be used for a popup menu, and update its field.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_window_icon_popup_menu(wimp_w w, wimp_i i, wimp_menu *menu)
{
	struct event_window		*window;
	struct event_icon		*icon;
	struct event_icon_action	*action;
	unsigned			entries;

	if ((window = event_find_window(w)) == NULL)
		return FALSE;

	if ((icon = event_find_icon(window, i)) == NULL)
		return FALSE;

	action = event_find_action(icon, EVENT_ICON_POPUP_MANUAL);

	if (action == NULL)
		action = event_find_action(icon, EVENT_ICON_POPUP_AUTO);

	if (action == NULL)
		return FALSE;

	action->data.popup.menu = menu;

	if (action->type == EVENT_ICON_POPUP_AUTO) {
		entries = menus_get_entries(menu);

		if (action->data.popup.selection > (entries - 1))
			event_set_auto_menu_selection(window, action, entries - 1);
	}

	return TRUE;
}


/* Set the currently selected item from a popup menu, and update its field.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_window_icon_popup_selection(wimp_w w, wimp_i i, unsigned selection)
{
	struct event_window *window;
	struct event_icon *icon;
	struct event_icon_action *action;

	if ((window = event_find_window(w)) == NULL)
		return FALSE;

	if ((icon = event_find_icon(window, i)) == NULL)
		return FALSE;

	if ((action = event_find_action(icon, EVENT_ICON_POPUP_AUTO)) == NULL)
		return FALSE;

	event_set_auto_menu_selection(window, action, selection);

	return TRUE;
}


/* Return the currently selected item from a popup menu.
 *
 * This function is an external interface, documented in event.h.
 */

unsigned event_get_window_icon_popup_selection(wimp_w w, wimp_i i)
{
	struct event_window *window;
	struct event_icon *icon;
	struct event_icon_action *action;

	if ((window = event_find_window(w)) == NULL)
		return 0;

	if ((icon = event_find_icon(window, i)) == NULL)
		return 0;

	if ((action = event_find_action(icon, EVENT_ICON_POPUP_AUTO)) == NULL)
		return 0;

	return action->data.popup.selection;
}


/**
 * Select an item in an auto popup menu, updating the internal selection and
 * the associated text field.
 *
 * \param *window		The window containing the menu.
 * \param *action		The action block for the popup menu.
 */

static void event_set_auto_menu_selection(struct event_window *window, struct event_icon_action *action, unsigned selection)
{
	if (window == NULL || action == NULL || action->type != EVENT_ICON_POPUP_AUTO || action->data.popup.field == -1)
		return;

	if (action->data.popup.token != NULL) {
		snprintf(action->data.popup.token_number, EVENT_TOKEN_INDEX_LEN, "%d", selection);
		icons_msgs_lookup(window->w, action->data.popup.field, action->data.popup.token);
	} else {
		icons_strncpy(window->w, action->data.popup.field, menus_get_text_addr(action->data.popup.menu, selection));
	}
	wimp_set_icon_state(window->w, action->data.popup.field, 0, 0);
	action->data.popup.selection = selection;
}


/* Add a user data pointer for the specified window.
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


/* Return the user data block associated with the specified window.
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


/* Remove a window and its associated event details from the records.
 *
 * This function is an external interface, documented in event.h.
 */

void event_delete_window(wimp_w w)
{
	struct event_window		*block, *parent;

	block = event_find_window(w);

	if (block != NULL) {
		/* Delete all linked icon and action definitions. */

		while (block->icons != NULL)
			event_delete_icon_block(block, block->icons);

		/* Delete the window itself. */

		if (event_window_list == block) {
			event_window_list = block->next;
		} else {
			for (parent = event_window_list; parent != NULL && parent->next != block; parent = parent->next);

			if (parent != NULL && parent->next == block)
				parent->next = block->next;
		}

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
 * Return the window data block for the given window, creating one first if required.
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

		block->menu_prepare = NULL;
		block->menu_selection = NULL;
		block->menu_close = NULL;
		block->menu_warning = NULL;

		block->data = NULL;

		block->icons = NULL;

		block->next = event_window_list;
		event_window_list = block;
	}

	return block;
}


/* Remove an icon and its associated event details from the records.
 *
 * This function is an external interface, documented in event.h.
 */

void event_delete_icon(wimp_w w, wimp_i i)
{
	struct event_window		*window;
	struct event_icon		*icon;

	window = event_find_window(w);

	if (window == NULL)
		return;

	icon = event_find_icon(window, i);

	if (icon != NULL)
		event_delete_icon_block(window, icon);
}


/**
 * Delete an icon block from a window definition.
 *
 * \param *window	The window containing the icon definition.
 * \param *icon		The icon definition to delete.
 */


static void event_delete_icon_block(struct event_window *window, struct event_icon *icon)
{
	struct event_icon		*parent;
	struct event_icon_action	*action;

	if (window == NULL || icon == NULL)
		return;

	/* Link the icon out of the window's chain. */

	if (window->icons == icon) {
		window->icons = icon->next;
	} else {
		for (parent = window->icons; parent != NULL && parent->next != icon; parent = parent->next);

		if (parent != NULL && parent->next == icon)
			parent->next = icon->next;
	}

	/* Delete all linked action definitions. */

	while (icon->actions != NULL) {
		action = icon->actions;
		icon->actions = action->next;

		if ((action->type == EVENT_ICON_POPUP_AUTO || action->type == EVENT_ICON_POPUP_MANUAL) &&
				action->data.popup.token != NULL)
			free(action->data.popup.token);
		free(action);
	}

	free(icon);
}


/**
 * Find the icon data block for the given icon in the specified window.
 *
 * \param *window	The window structure to find the icon structure for.
 * \param i		The icon to find the structure for.
 * \return		A pointer to the icon structure, or NULL.
 */

static struct event_icon *event_find_icon(struct event_window *window, wimp_i i)
{
	struct event_icon	*block = NULL;

	if (window != NULL && i >= 0) {
		block = window->icons;

		while (block != NULL && block->i != i)
			block = block->next;
	}

	return block;
}


/**
 * Return the icon data block for the given window and icon, creating it first
 * if required.
 *
 * \param *window	The window structure to find the icon structure for.
 * \param i		The icon to find the structure for.
 * \return		A pointer to the icon structure, or NULL.
 */

static struct event_icon *event_create_icon(struct event_window *window, wimp_i i)
{
	struct event_icon	*block;

	if (window == NULL)
		return NULL;

	block = event_find_icon(window, i);

	if (block != NULL)
		return block;

	block = (struct event_icon *) malloc(sizeof(struct event_icon));

	if (block != NULL) {
		block->i = i;

		block->actions = NULL;

		block->next = window->icons;
		window->icons = block;
	}

	return block;

}


/**
 * Find the action data block for the given action in the specified icon.
 *
 * \param *icon		The icon structure to find the action structure for.
 * \param type		The action type to find the structure for.
 * \return		A pointer to the action structure, or NULL.
 */

static struct event_icon_action *event_find_action(struct event_icon *icon, enum event_icon_type type)
{
	struct event_icon_action *action = NULL;

	if (icon != NULL) {
		action = icon->actions;

		while (action != NULL && action->type != type)
			action = action->next;
	}

	return action;
}


/**
 * Return the action data block for the given icon and action, creating it first
 * if required.
 *
 * \param *icon		The icon structure to find the action structure for.
 * \param type		The action to find the structure for.
 * \return		A pointer to the action structure, or NULL.
 */

static struct event_icon_action *event_create_action(struct event_icon *icon, enum event_icon_type type)
{
	struct event_icon_action	*block;

	if (icon == NULL)
		return NULL;

	block = event_find_action(icon, type);

	if (block != NULL)
		return block;

	block = malloc(sizeof(struct event_icon_action));

	if (block != NULL) {
		block->type = type;

		switch (type) {
		case EVENT_ICON_CLICK:
			block->data.click.callback = NULL;
			break;

		case EVENT_ICON_RADIO:
			block->data.radio.complete = FALSE;
			break;

		case EVENT_ICON_POPUP_AUTO:
		case EVENT_ICON_POPUP_MANUAL:
			block->data.popup.menu = NULL;
			block->data.popup.field = wimp_ICON_WINDOW;
			block->data.popup.token = NULL;
			block->data.popup.token_number = NULL;
			block->data.popup.selection = 0;
			break;

		case EVENT_ICON_NONE:
			break;
		}

		block->next = icon->actions;
		icon->actions = block;
	}

	return block;

}


/* Add a message handler for the given user message, and add the message to
 * the list of messages required from the Wimp if it isn't already on it.
 *
 * *message_action can be passed as NULL, in which case the message is
 * registered with the Wimp and recorded in the messages list, but not given
 * an action.  This is used internally to ensure that messages required by
 * library are returned.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_add_message_handler(unsigned int message, enum event_message_type type, osbool (*message_action)(wimp_message *message))
{
	wimp_MESSAGE_LIST(2)		message_list;
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

		if (message != message_QUIT) {
			message_list.messages[0]=message;
			message_list.messages[1]=message_QUIT;
			xwimp_add_messages((wimp_message_list *) &message_list);
		}
	}

	if (message_action == NULL)
		return TRUE;

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


/* Set a handler for the next drag box event and any Null Polls in between.
 * If either handler is NULL it will not be called; both will be cancelled on
 * the next User_Drag_Box event to be received.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_drag_handler(void (*drag_end)(wimp_dragged *dragged, void *data), osbool (*drag_null_poll)(void *data), void *data)
{
	event_drag_end = drag_end;
	event_drag_null_poll = drag_null_poll;
	event_drag_data = data;

	return TRUE;
}


/* Set a variable to store a pointer to the currently open menu block.
 *
 * This function is an external interface, documented in event.h.
 */

osbool event_set_menu_pointer(wimp_menu **menu)
{
	menu_handle = menu;

	return TRUE;
}


/**
 * Change the menu block associated with the current _menu_prepare() callback.
 *
 * \param *menu			The new menu block.
 */

void event_set_menu_block(wimp_menu *menu)
{
	new_client_menu = menu;
}

