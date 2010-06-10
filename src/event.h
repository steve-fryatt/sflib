/* SF-Lib - Event.h
 *
 * Version 0.10 (3 May 2010)
 */

int event_process_event(int event, wimp_block *block, int pollword);

int event_add_window_open_event(wimp_w w, void (*callback)(wimp_open *open));
int event_add_window_close_event(wimp_w w, void (*callback)(wimp_close *close));
