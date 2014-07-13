#ifndef CURSMISC_H
#define CURSMISC_H

/* Global declarations */

int curses_read_char(void);

void curses_toggle_color_attr(WINDOW *win, int color, int attr, int onoff);

void curses_bail(const char *mesg);

winid curses_get_wid(int type);

char *curses_copy_of(const char *s);

int curses_num_lines(const char *str, int width);

char *curses_break_str(const char *str, int width, int line_num);

char *curses_str_remainder(const char *str, int width, int line_num);

boolean curses_is_menu(winid wid);

boolean curses_is_text(winid wid);

int curses_convert_glyph(int ch, int glyph);

void curses_move_cursor(winid wid, int x, int y);

void curses_prehousekeeping(void);

void curses_posthousekeeping(void);

void curses_view_file(const char *filearea, const char *filename, boolean must_exist);

void curses_rtrim(char *str);

int curses_get_count(int first_digit);

int curses_convert_attr(int attr);

int curses_read_attrs(char *attrs);

int curses_convert_keys(int key);

int curses_get_mouse(int *mousex, int *mousey, int *mod);

#endif  /* CURSMISC_H */
