#ifndef CURSWIN_H
#define CURSWIN_H


/* Global declarations */

WINDOW *curses_create_window(int width, int height, orient orientation);

void curses_destroy_win(WINDOW *win);

void curses_refresh_nethack_windows(void);

WINDOW *curses_get_nhwin(winid wid);

void curses_add_nhwin(winid wid, int height, int width, int y, int x,
 orient orientation, boolean border);

void curses_refresh_nhwin(winid wid);

void curses_del_nhwin(winid wid);

void curses_putch(winid wid, int x, int y, int ch, int color, int attrs);

void curses_get_window_xy(winid wid, int *x, int *y);

boolean curses_window_has_border(winid wid);

boolean curses_window_exists(winid wid);

int curses_get_window_orientation(winid wid);

void curses_puts(winid wid, int attr, const char *text);

void curses_clear_nhwin(winid wid);


#endif  /* CURSWIN_H */
