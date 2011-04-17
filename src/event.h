/* SF-Lib - Event.h
 *
 * Version 0.10 (3 May 2010)
 */

int event_process_event(wimp_event_no event, wimp_block *block, int pollword);

int event_add_window_redraw_event(wimp_w w, void (*callback)(wimp_draw *draw));
int event_add_window_open_event(wimp_w w, void (*callback)(wimp_open *open));
int event_add_window_close_event(wimp_w w, void (*callback)(wimp_close *close));
int event_add_window_pointer_leaving_event(wimp_w w, void (*callback)(wimp_leaving *leaving));
int event_add_window_pointer_entering_event(wimp_w w, void (*callback)(wimp_entering *entering));
int event_add_window_mouse_event(wimp_w w, void (*callback)(wimp_pointer *pointer));
int event_add_window_key_event(wimp_w w, void (*callback)(wimp_key *key));
int event_add_window_scroll_event(wimp_w w, void (*callback)(wimp_scroll *scroll));
int event_add_window_lose_caret_event(wimp_w w, void (*callback)(wimp_caret *caret));
int event_add_window_gain_caret_event(wimp_w w, void (*callback)(wimp_caret *caret));

int event_add_window_menu(wimp_w w, wimp_menu *menu, int iconbar);
int event_add_window_menu_prepare(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_pointer *pointer));
int event_add_window_menu_selection(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_selection *selection));
int event_add_window_menu_close(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m));
int event_add_window_menu_warning(wimp_w w, void (*callback)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning));

int event_add_window_user_data(wimp_w w, void *data);
void *event_get_window_user_data(wimp_w w);

void event_delete_window(wimp_w w);

int event_set_drag_handler(void (*drag_end)(wimp_dragged *dragged, void *data), int (*drag_null_poll)(void *data), void *data);

int event_set_menu_pointer(wimp_menu **menu_handle);

