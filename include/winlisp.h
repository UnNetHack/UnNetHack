#ifndef WINLISP_H
#define WINLISP_H

#ifndef E
#define E extern
#endif

#if defined(BOS) || defined(NHSTDC)
#define DIMENSION_P int
#else
# ifdef WIDENED_PROTOTYPES
#define DIMENSION_P unsigned int
# else
#define DIMENSION_P Dimension
# endif
#endif

extern struct window_procs tty_procs;

/* ### winlisp.c ### */
E void win_lisp_init();
E void lisp_init_nhwindows(int *, char **);
E void lisp_player_selection();
E void lisp_askname();
E void lisp_get_nh_event();
E void lisp_exit_nhwindows(const char *);
E void lisp_suspend_nhwindows(const char *);
E void lisp_resume_nhwindows();
E winid lisp_create_nhwindow(int);
E void lisp_clear_nhwindow(winid);
E void lisp_display_nhwindow(winid, BOOLEAN_P);
E void lisp_destroy_nhwindow(winid);
E void lisp_curs(winid, int, int);
E void lisp_putstr(winid, int, const char *);
#ifdef FILE_AREAS
E void lisp_display_file(const char *, const char *, BOOLEAN_P);
#else
E void lisp_display_file(const char *, BOOLEAN_P);
#endif
E void lisp_start_menu(winid);
E void lisp_add_menu(winid, int, int, const ANY_P *,
                     CHAR_P, CHAR_P, int, const char *, unsigned int);
E void lisp_end_menu(winid, const char *);
E int lisp_select_menu(winid, int, MENU_ITEM_P **);
E char lisp_message_menu(char, int, const char *mesg);
E void lisp_update_inventory();
E void lisp_mark_synch();
E void lisp_wait_synch();
#ifdef CLIPPING
E void lisp_cliparound(int, int);
#endif
#ifdef POSITIONBAR
E void lisp_update_positionbar(char *);
#endif
E void lisp_print_glyph(winid, XCHAR_P, XCHAR_P, int, int);
E void lisp_raw_print(const char *);
E void lisp_raw_print_bold(const char *);
E int lisp_nhgetch();
E int lisp_nh_poskey(int *, int *, int *);
E void lisp_nhbell();
E int lisp_doprev_message();
E char lisp_yn_function(const char *, const char *, CHAR_P);
E void lisp_getlin(const char *, char *);
E int lisp_get_ext_cmd();
E void lisp_number_pad(int);
E void lisp_delay_output();

/* other defs that really should go away (they're tty specific) */
E void lisp_start_screen();
E void lisp_end_screen();

E void lisp_outrip(winid, int);

#endif /* WINLISP_H */
