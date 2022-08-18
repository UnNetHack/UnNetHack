#ifndef WINCURS_H
#define WINCURS_H

/* Global declarations for curses interface */

extern int term_rows, term_cols;                /* size of underlying terminal */
extern WINDOW *base_term;                       /* underlying terminal window */
extern WINDOW *mapwin, *statuswin, *messagewin; /* Main windows */
extern int orig_cursor;                         /* Preserve initial cursor state */
extern boolean counting;                        /* Count window is active */

#define TEXTCOLOR   /* Allow color */
#define NHW_END 19
#define OFF 0
#define ON 1
#define NONE -1
#define KEY_ESC 0x1b
#define DIALOG_BORDER_COLOR CLR_MAGENTA
#define ALERT_BORDER_COLOR CLR_RED
#define SCROLLBAR_COLOR CLR_MAGENTA
#define SCROLLBAR_BACK_COLOR CLR_BLACK
#define HIGHLIGHT_COLOR CLR_WHITE
#define MORECOLOR CLR_ORANGE
#define STAT_UP_COLOR CLR_GREEN
#define STAT_DOWN_COLOR CLR_RED
#define MESSAGE_WIN 1
#define STATUS_WIN  2
#define MAP_WIN     3
#define INV_WIN     4
#define NHWIN_MAX   5
#define MESG_HISTORY_MAX   200
#if !defined(__APPLE__) || !defined(NCURSES_VERSION)
# define USE_DARKGRAY /* Allow "bright" black; delete if not visible */
#endif  /* !__APPLE__ && !PDCURSES */
#define CURSES_DARK_GRAY    17
#define MAP_SCROLLBARS
#ifdef PDCURSES
# define getmouse nc_getmouse
# ifndef NCURSES_MOUSE_VERSION
#  define NCURSES_MOUSE_VERSION
# endif
#endif


typedef enum orient_type
{
    CENTER,
    UP,
    DOWN,
    RIGHT,
    LEFT,
    UNDEFINED
} orient;


/* cursmain.c */

extern struct window_procs curses_procs;

extern void curses_init_nhwindows(int* argcp, char** argv);

extern void curses_player_selection(void);

extern void curses_askname(void);

extern void curses_get_nh_event(void);

extern void curses_exit_nhwindows(const char *str);

extern void curses_suspend_nhwindows(const char *str);

extern void curses_resume_nhwindows(void);

extern winid curses_create_nhwindow(int type);

extern void curses_clear_nhwindow(winid wid);

extern void curses_display_nhwindow(winid wid, boolean block);

extern void curses_destroy_nhwindow(winid wid);

extern void curses_curs(winid wid, int x, int y);

extern void curses_putstr(winid wid, int attr, const char *text);

#ifdef FILE_AREAS
extern void curses_display_file(const char *filearea, const char *filename, boolean must_exist);
#else
extern void curses_display_file(const char *filename, boolean must_exist);
#endif

extern void curses_start_menu(winid wid);

extern void curses_add_menu(winid wid, int glyph, int cnt, const ANY_P * identifier,
                            char accelerator, char group_accel, int attr,
                            const char *str, unsigned int presel);

extern void curses_end_menu(winid wid, const char *prompt);

extern int curses_select_menu(winid wid, int how, MENU_ITEM_P **selected);

extern void curses_update_inventory(void);

extern void curses_mark_synch(void);

extern void curses_wait_synch(void);

extern void curses_cliparound(int x, int y);

extern void curses_print_glyph(winid wid, coordxy x, coordxy y, int glyph, int bg_glyph);

extern void curses_raw_print(const char *str);

extern void curses_raw_print_bold(const char *str);

extern int curses_nhgetch(void);

extern int curses_nh_poskey(coordxy *x, coordxy *y, int *mod);

extern void curses_nhbell(void);

extern int curses_doprev_message(void);

extern char curses_yn_function(const char *question, const char *choices, char def);

extern void curses_getlin(const char *question, char *input);

extern int curses_get_ext_cmd(void);

extern void curses_number_pad(int state);

extern void curses_delay_output(void);

extern void curses_start_screen(void);

extern void curses_end_screen(void);

extern void curses_outrip(winid wid, int how);

extern void genl_outrip(winid tmpwin, int how);

extern void curses_preference_update(const char *pref);


/* curswins.c */

extern WINDOW *curses_create_window(int width, int height, orient orientation);

extern void curses_destroy_win(WINDOW *win);

extern WINDOW *curses_get_nhwin(winid wid);

extern void curses_add_nhwin(winid wid, int height, int width, int y,
                             int x, orient orientation, boolean border);

extern void curses_add_wid(winid wid);

extern void curses_refresh_nhwin(winid wid);

extern void curses_refresh_nethack_windows(void);

extern void curses_del_nhwin(winid wid);

extern void curses_del_wid(winid wid);

extern void curses_putch(winid wid, int x, int y, int ch, int color, int attrs);

extern void curses_get_window_size(winid wid, int *height, int *width);

extern boolean curses_window_has_border(winid wid);

extern boolean curses_window_exists(winid wid);

extern int curses_get_window_orientation(winid wid);

extern void curses_get_window_xy(winid wid, int *x, int *y);

extern void curses_puts(winid wid, int attr, const char *text);

extern void curses_clear_nhwin(winid wid);

extern void curses_alert_win_border(winid wid, boolean onoff);

extern void curses_alert_main_borders(boolean onoff);

extern void curses_draw_map(int sx, int sy, int ex, int ey);

extern boolean curses_map_borders(int *sx, int *sy, int *ex, int *ey,
                                  int ux, int uy);


/* cursmisc.c */

extern int curses_read_char(void);
extern void curses_toggle_color_attr(WINDOW *win, int color, int attr, int onoff);
extern void curses_menu_color_attr(WINDOW *, int, int, int);
extern void curses_bail(const char *mesg);
extern winid curses_get_wid(int type);
extern char *curses_copy_of(const char *s);
extern int curses_num_lines(const char *str, int width);
extern char *curses_break_str(const char *str, int width, int line_num);
extern char *curses_str_remainder(const char *str, int width, int line_num);
extern boolean curses_is_menu(winid wid);
extern boolean curses_is_text(winid wid);
extern int curses_convert_glyph(int ch, int glyph);
extern void curses_move_cursor(winid wid, int x, int y);
extern void curses_prehousekeeping(void);
extern void curses_posthousekeeping(void);

#ifdef FILE_AREAS
extern void curses_view_file(const char *filearea, const char *filename, boolean must_exist);
#else
extern void curses_view_file(const char *filename, boolean must_exist);
#endif

extern void curses_rtrim(char *str);

extern int curses_get_count(int first_digit);

extern int curses_convert_attr(int attr);

extern int curses_read_attrs(char *attrs);

extern int curses_convert_keys(int key);

extern int curses_get_mouse(coordxy *mousex, coordxy *mousey, int *mod);

/* cursdial.c */

extern void curses_line_input_dialog(const char *prompt, char *answer, int buffer);

extern int curses_character_input_dialog(const char *prompt, const char *choices, char def);

extern int curses_ext_cmd(void);

extern void curses_create_nhmenu(winid wid);
#ifdef MENU_COLOR
extern boolean get_menu_coloring(char *, int *, int *);
#endif
extern void curses_add_nhmenu_item(winid wid, int glyph, const ANY_P *identifier,
                                   char accelerator, char group_accel, int attr, const char *str,
                                   boolean presel);

extern void curses_finalize_nhmenu(winid wid, const char *prompt);

extern int curses_display_nhmenu(winid wid, int how, MENU_ITEM_P **_selected);

extern boolean curses_menu_exists(winid wid);

extern void curses_del_menu(winid wid);


/* cursstat.c */

extern attr_t curses_color_attr(int nh_color, int bg_color);
extern void curses_update_stats(void);
extern void curses_decrement_highlight(void);

/* cursinvt.c */

extern void curses_update_inv(void);
extern void curses_add_inv(int, int, char, attr_t, const char *);

/* cursinit.c */

extern void curses_create_main_windows(void);

extern void curses_init_nhcolors(void);

extern void curses_choose_character(void);

extern int curses_character_dialog(const char** choices, const char *prompt);

extern void curses_init_options(void);

extern void curses_display_splash_window(void);

extern void curses_cleanup(void);

extern int curses_debug_show_colors(void);

/* cursmesg.c */

extern void curses_message_win_puts(const char *message, boolean recursed);

extern int curses_block(boolean require_tab); /* for MSGTYPE=STOP */

extern int curses_more(void);

extern void curses_clear_unhighlight_message_window(void);

extern void curses_message_win_getline(const char *prompt, char *answer, int buffer);

extern void curses_last_messages(void);

extern void curses_init_mesg_history(void);

extern void curses_prev_mesg(void);

extern void curses_count_window(const char *count_text);

#endif  /* WINCURS_H */
