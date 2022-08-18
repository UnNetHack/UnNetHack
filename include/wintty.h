/*  SCCS Id: @(#)wintty.h   3.4 1996/02/18  */
/* Copyright (c) David Cohrs, 1991,1992               */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINTTY_H
#define WINTTY_H

#define E extern

#ifndef WINDOW_STRUCTS
#define WINDOW_STRUCTS

/* menu structure */
typedef struct tty_mi {
    struct tty_mi *next;
    anything identifier;    /* user identifier */
    long count;         /* user count */
    long maxcount;      /* maximum user count */
    char *str;          /* description string (including accelerator) */
    int attr;           /* string attribute */
    boolean selected;       /* TRUE if selected by user */
    char selector;      /* keyboard accelerator */
    char gselector;     /* group accelerator */
    glyph_t glyph;      /* glyph display in menu */
} tty_menu_item;

/* descriptor for tty-based windows */
struct WinDesc {
    int flags;          /* window flags */
    xchar type;         /* type of window */
    boolean active;     /* true if window is active */
    short offx, offy;       /* offset from topleft of display */
    long rows, cols;        /* dimensions */
    long curx, cury;        /* current cursor position */
    long maxrow, maxcol;    /* the maximum size used -- for MENU wins */
    /* maxcol is also used by WIN_MESSAGE for */
    /* tracking the ^P command */
    short *datlen;      /* allocation size for *data */
    char **data;        /* window data [row][column] */
    int **attributes;       /* window data [row][column] */
    char *morestr;      /* string to display instead of default */
    tty_menu_item *mlist;   /* menu information (MENU) */
    tty_menu_item **plist;  /* menu page pointers (MENU) */
    long plist_size;        /* size of allocated plist (MENU) */
    long npages;        /* number of pages in menu (MENU) */
    long nitems;        /* total number of items (MENU) */
    short how;          /* menu mode - pick 1 or N (MENU) */
    char menu_ch;       /* menu char (MENU) */
};

/* window flags */
#define WIN_CANCELLED 1
#define WIN_STOP 1        /* for NHW_MESSAGE; stops output */
#define WIN_LOCKHISTORY 2 /* for NHW_MESSAGE; suppress history updates */

/* topline states */
#define TOPLINE_EMPTY          0 /* empty */
#define TOPLINE_NEED_MORE      1 /* non-empty, need --More-- */
#define TOPLINE_NON_EMPTY      2 /* non-empty, no --More-- required */
#define TOPLINE_SPECIAL_PROMPT 3 /* special prompt state */

/* descriptor for tty-based displays -- all the per-display data */
struct DisplayDesc {
    short rows, cols;       /* width and height of tty display */
    short curx, cury;       /* current cursor position on the screen */
#ifdef TEXTCOLOR
    int color;          /* current color */
#endif
    int attrs;          /* attributes in effect */
    int toplin;         /* flag for topl stuff */
    int rawprint;       /* number of raw_printed lines since synch */
    int inmore;         /* non-zero if more() is active */
    int inread;         /* non-zero if reading a character */
    int intr;           /* non-zero if inread was interrupted */
    winid lastwin;      /* last window used for I/O */
    char dismiss_more;      /* extra character accepted at --More-- */
};

#endif /* WINDOW_STRUCTS */

#define MAXWIN 20       /* maximum number of windows, cop-out */

/* tty dependent window types */
#ifdef NHW_BASE
#undef NHW_BASE
#endif
#define NHW_BASE    6

extern struct window_procs tty_procs;

/* port specific variable declarations */
extern winid BASE_WINDOW;

extern struct WinDesc *wins[MAXWIN];

extern struct DisplayDesc *ttyDisplay;  /* the tty display descriptor */

extern char morc;       /* last character typed to xwaitforspace */
extern char defmorestr[];   /* default --more-- prompt */

/* port specific external function references */

/* ### getline.c ### */
E void xwaitforspace(const char *);

/* ### termcap.c, video.c ### */

E void tty_startup(int*, int*);
#ifndef NO_TERMS
E void tty_shutdown();
#endif
extern int xputc(int);
E void xputs(const char *);
#if defined(SCREEN_VGA) || defined(SCREEN_8514)
E void xputg(int, int, unsigned);
#endif
E void cl_end();
E void clear_screen();
E void home();
E void standoutbeg();
E void standoutend();
# if 0
E void revbeg();
E void boldbeg();
E void blinkbeg();
E void dimbeg();
E void m_end();
# endif
E void backsp();
E void graph_on();
E void graph_off();
E void cl_eos();

/*
 * termcap.c (or facsimiles in other ports) is the right place for doing
 * strange and arcane things such as outputting escape sequences to select
 * a color or whatever.  wintty.c should concern itself with WHERE to put
 * stuff in a window.
 */
E void term_start_attr(int attr);
E void term_end_attr(int attr);
E void term_start_raw_bold();
E void term_end_raw_bold();

#ifdef TEXTCOLOR
E void term_end_color();
E void term_start_color(int color);
E int has_color(int color);
#endif /* TEXTCOLOR */

#ifdef STATUS_COLORS
E boolean parse_status_color_options(char *);
#endif /* STATUS_COLOR */

/* ### topl.c ### */

extern void show_topl(const char *);
extern void remember_topl(void);
E void addtopl(const char *);
E void more();
E void update_topl(const char *);
E void putsyms(const char*);

/* ### wintty.c ### */
#ifdef CLIPPING
E void setclipped();
#endif
E void docorner(int, int);
E void end_glyphout();
E void g_putch(int);
E void win_tty_init();

/* external declarations */
E void tty_init_nhwindows(int *, char **);
E void tty_player_selection();
E void tty_askname();
E void tty_get_nh_event();
E void tty_exit_nhwindows(const char *);
E void tty_suspend_nhwindows(const char *);
E void tty_resume_nhwindows();
E winid tty_create_nhwindow(int);
E void tty_clear_nhwindow(winid);
E void tty_display_nhwindow(winid, BOOLEAN_P);
E void tty_dismiss_nhwindow(winid);
E void tty_destroy_nhwindow(winid);
E void tty_curs(winid, int, int);
E void tty_putstr(winid, int, const char *);
#ifdef FILE_AREAS
E void tty_display_file(const char *, const char *, BOOLEAN_P);
#else
E void tty_display_file(const char *, BOOLEAN_P);
#endif
E void tty_start_menu(winid);
E void tty_add_menu(winid, int, int, const ANY_P *,
                    CHAR_P, CHAR_P, int, const char *, unsigned int);
E void tty_end_menu(winid, const char *);
E int tty_select_menu(winid, int, MENU_ITEM_P **);
E char tty_message_menu(CHAR_P, int, const char *);
E void tty_update_inventory();
E void tty_mark_synch();
E void tty_wait_synch();
#ifdef CLIPPING
E void tty_cliparound(int, int);
#endif
#ifdef POSITIONBAR
E void tty_update_positionbar(char *);
#endif
E void tty_print_glyph(winid, XCHAR_P, XCHAR_P, int, int);
E void tty_raw_print(const char *);
E void tty_raw_print_bold(const char *);
E int tty_nhgetch();
E int tty_nh_poskey(int *, int *, int *);
E void tty_nhbell();
E int tty_doprev_message();
E char tty_yn_function(const char *, const char *, CHAR_P);
E void tty_getlin(const char *, char *);
E int tty_get_ext_cmd();
E void tty_number_pad(int);
E void tty_delay_output();
#ifdef CHANGE_COLOR
E void tty_change_color(int color, long rgb, int reverse);
#ifdef MAC
E void tty_change_background(int white_or_black);
E short set_tty_font_name(winid, char *);
#endif
E char * tty_get_color_string();
#endif
E int tty_debug_show_colors();

/* other defs that really should go away (they're tty specific) */
E void tty_start_screen();
E void tty_end_screen();

E void genl_outrip(winid, int);

#ifdef NO_TERMS
# ifdef MAC
#  ifdef putchar
#   undef putchar
#   undef putc
#  endif
#  define putchar term_putc
#  define fflush term_flush
#  define puts term_puts
E int term_putc(int c);
E int term_flush(void *desc);
E int term_puts(const char *str);
# endif /* MAC */
# if defined(MSDOS) || defined(WIN32CON)
#  if defined(SCREEN_BIOS) || defined(SCREEN_DJGPPFAST) || defined(WIN32CON)
#   undef putchar
#   undef putc
#   undef puts
#   define putchar(x) xputc(x)  /* these are in video.c, nttty.c */
#   define putc(x) xputc(x)
#   define puts(x) xputs(x)
#  endif/*SCREEN_BIOS || SCREEN_DJGPPFAST || WIN32CON */
#  ifdef POSITIONBAR
E void video_update_positionbar(char *);
#  endif
# endif/*MSDOS*/
#endif/*NO_TERMS*/

#undef E

#endif /* WINTTY_H */
