/* Copyright (c) David Cohrs, 1991,1992				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINTTY_H
#define WINTTY_H

#define E extern

#ifndef WINDOW_STRUCTS
#define WINDOW_STRUCTS

/* menu structure */
typedef struct tty_mi {
    struct tty_mi *next;
    anything identifier;	/* user identifier */
    long count;			/* user count */
    long maxcount;		/* maximum user count */
    char *str;			/* description string (including accelerator) */
    int attr;			/* string attribute */
    boolean selected;		/* TRUE if selected by user */
    char selector;		/* keyboard accelerator */
    char gselector;		/* group accelerator */
    glyph_t glyph;		/* glyph display in menu */
} tty_menu_item;

/* descriptor for tty-based windows */
struct WinDesc {
    int flags;			/* window flags */
    xchar type;			/* type of window */
    boolean active;		/* true if window is active */
    short offx, offy;		/* offset from topleft of display */
    long rows, cols;		/* dimensions */
    long curx, cury;		/* current cursor position */
    long maxrow, maxcol;	/* the maximum size used -- for MENU wins */
				/* maxcol is also used by WIN_MESSAGE for */
				/* tracking the ^P command */
    short *datlen;		/* allocation size for *data */
    char **data;		/* window data [row][column] */
    char *morestr;		/* string to display instead of default */
    tty_menu_item *mlist;	/* menu information (MENU) */
    tty_menu_item **plist;	/* menu page pointers (MENU) */
    long plist_size;		/* size of allocated plist (MENU) */
    long npages;		/* number of pages in menu (MENU) */
    long nitems;		/* total number of items (MENU) */
    short how;			/* menu mode - pick 1 or N (MENU) */
    char menu_ch;		/* menu char (MENU) */
};

/* window flags */
#define WIN_CANCELLED 1
#define WIN_STOP 1		/* for NHW_MESSAGE; stops output */

/* descriptor for tty-based displays -- all the per-display data */
struct DisplayDesc {
    short rows, cols;		/* width and height of tty display */
    short curx, cury;		/* current cursor position on the screen */
    int color;			/* current color */
    int attrs;			/* attributes in effect */
    int toplin;			/* flag for topl stuff */
    int rawprint;		/* number of raw_printed lines since synch */
    int inmore;			/* non-zero if more() is active */
    int inread;			/* non-zero if reading a character */
    int intr;			/* non-zero if inread was interrupted */
    winid lastwin;		/* last window used for I/O */
    char dismiss_more;		/* extra character accepted at --More-- */
};

#endif /* WINDOW_STRUCTS */

#define MAXWIN 20		/* maximum number of windows, cop-out */

/* tty dependent window types */
#ifdef NHW_BASE
#undef NHW_BASE
#endif
#define NHW_BASE    6

extern struct window_procs tty_procs;

/* port specific variable declarations */
extern winid BASE_WINDOW;

extern struct WinDesc *wins[MAXWIN];

extern struct DisplayDesc *ttyDisplay;	/* the tty display descriptor */

extern char morc;		/* last character typed to xwaitforspace */
extern char defmorestr[];	/* default --more-- prompt */

/* port specific external function references */

/* ### getline.c ### */
E void FDECL(xwaitforspace, (const char *));

/* ### termcap.c, video.c ### */

E void FDECL(tty_startup,(int*, int*));
#ifndef NO_TERMS
E void NDECL(tty_shutdown);
#endif
#if defined(apollo)
/* Apollos don't widen old-style function definitions properly -- they try to
 * be smart and use the prototype, or some such strangeness.  So we have to
 * define UNWIDENDED_PROTOTYPES (in tradstdc.h), which makes CHAR_P below a
 * char.  But the tputs termcap call was compiled as if xputc's argument
 * actually would be expanded.	So here, we have to make an exception. */
E void FDECL(xputc, (int));
#else
E void FDECL(xputc, (CHAR_P));
#endif
E void FDECL(xputs, (const char *));
#if defined(SCREEN_VGA) || defined(SCREEN_8514)
E void FDECL(xputg, (int, int, unsigned));
#endif
E void NDECL(cl_end);
E void NDECL(clear_screen);
E void NDECL(home);
E void NDECL(standoutbeg);
E void NDECL(standoutend);
# if 0
E void NDECL(revbeg);
E void NDECL(boldbeg);
E void NDECL(blinkbeg);
E void NDECL(dimbeg);
E void NDECL(m_end);
# endif
E void NDECL(backsp);
E void NDECL(graph_on);
E void NDECL(graph_off);
E void NDECL(cl_eos);

/*
 * termcap.c (or facsimiles in other ports) is the right place for doing
 * strange and arcane things such as outputting escape sequences to select
 * a color or whatever.  wintty.c should concern itself with WHERE to put
 * stuff in a window.
 */
E void FDECL(term_start_attr,(int attr));
E void FDECL(term_end_attr,(int attr));
E void NDECL(term_start_raw_bold);
E void NDECL(term_end_raw_bold);

E void NDECL(term_end_color);
E void FDECL(term_start_color,(int color));
E int FDECL(has_color,(int color));

E boolean FDECL(parse_status_color_options, (char *));

/* ### topl.c ### */

E void FDECL(addtopl, (const char *));
E void NDECL(more);
E void FDECL(update_topl, (const char *));
E void FDECL(putsyms, (const char*));

/* ### wintty.c ### */
E void NDECL(setclipped);
E void FDECL(docorner, (int, int));
E void NDECL(end_glyphout);
E void FDECL(g_putch, (int));
E void NDECL(win_tty_init);

/* external declarations */
E void FDECL(tty_init_nhwindows, (int *, char **));
E void NDECL(tty_player_selection);
E void NDECL(tty_askname);
E void NDECL(tty_get_nh_event) ;
E void FDECL(tty_exit_nhwindows, (const char *));
E void FDECL(tty_suspend_nhwindows, (const char *));
E void NDECL(tty_resume_nhwindows);
E winid FDECL(tty_create_nhwindow, (int));
E void FDECL(tty_clear_nhwindow, (winid));
E void FDECL(tty_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(tty_dismiss_nhwindow, (winid));
E void FDECL(tty_destroy_nhwindow, (winid));
E void FDECL(tty_curs, (winid,int,int));
E void FDECL(tty_putstr, (winid, int, const char *));
#ifdef FILE_AREAS
E void FDECL(tty_display_file, (const char *, const char *, BOOLEAN_P));
#else
E void FDECL(tty_display_file, (const char *, BOOLEAN_P));
#endif
E void FDECL(tty_start_menu, (winid));
E void FDECL(tty_add_menu, (winid,int,int,const ANY_P *,
			CHAR_P,CHAR_P,int,const char *, BOOLEAN_P));
E void FDECL(tty_end_menu, (winid, const char *));
E int FDECL(tty_select_menu, (winid, int, MENU_ITEM_P **));
E char FDECL(tty_message_menu, (CHAR_P,int,const char *));
E void NDECL(tty_update_inventory);
E void NDECL(tty_mark_synch);
E void NDECL(tty_wait_synch);
E void FDECL(tty_cliparound, (int, int));
#ifdef POSITIONBAR
E void FDECL(tty_update_positionbar, (char *));
#endif
E void FDECL(tty_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(tty_raw_print, (const char *));
E void FDECL(tty_raw_print_bold, (const char *));
E int NDECL(tty_nhgetch);
E int FDECL(tty_nh_poskey, (int *, int *, int *));
E void NDECL(tty_nhbell);
E int NDECL(tty_doprev_message);
E char FDECL(tty_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(tty_getlin, (const char *,char *));
E int NDECL(tty_get_ext_cmd);
E void FDECL(tty_number_pad, (int));
E void NDECL(tty_delay_output);
#ifdef CHANGE_COLOR
E void FDECL(tty_change_color,(int color,long rgb,int reverse));
E char * NDECL(tty_get_color_string);
#endif

/* other defs that really should go away (they're tty specific) */
E void NDECL(tty_start_screen);
E void NDECL(tty_end_screen);

E void FDECL(genl_outrip, (winid,int));

#ifdef NO_TERMS
# if defined(WIN32CON)
#  if defined(SCREEN_BIOS) || defined(WIN32CON)
#   undef putchar
#   undef putc
#   undef puts
#   define putchar(x) xputc(x)	/* these are in video.c, nttty.c */
#   define putc(x) xputc(x)
#   define puts(x) xputs(x)
#  endif/*SCREEN_BIOS || WIN32CON */
#  ifdef POSITIONBAR
E void FDECL(video_update_positionbar, (char *));
#  endif
# endif
#endif/*NO_TERMS*/

#undef E

#endif /* WINTTY_H */
