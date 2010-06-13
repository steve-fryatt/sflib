/* SF-Lib - Event.h
 *
 * Version 0.10 (3 May 2010)
 */

int event_process_event(wimp_event_no event, wimp_block *block, int pollword);

int event_add_window_redraw_event(wimp_w w, void (*callback)(wimp_draw *draw));
int event_add_window_open_event(wimp_w w, void (*callback)(wimp_open *open));
int event_add_window_close_event(wimp_w w, void (*callback)(wimp_close *close));
int event_add_window_mouse_event(wimp_w w, void (*callback)(wimp_pointer *pointer));
int event_add_window_key_event(wimp_w w, void (*callback)(wimp_key *key));

int event_add_window_menu(wimp_w w, wimp_menu *menu,
		void (*prepare)(wimp_pointer *pointer, wimp_menu *m),
		void (*selection)(wimp_w w, wimp_menu *m, wimp_selection *selection),
		void (*close)(wimp_w w, wimp_menu *m),
		void (*warning)(wimp_w w, wimp_menu *m, wimp_message_menu_warning *warning));

int event_add_window_user_data(wimp_w w, void *data);
void *event_get_window_user_data(wimp_w w);

void event_delete_window(wimp_w w);
