/* NetHack SDL2 window port -- header */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINSDL_H
#define WINSDL_H

/*
 * Deliberately no <SDL2/SDL.h> include here: nothing declared in this
 * header needs SDL types (those live only in win/sdl/sdlmain.c's
 * static state), and hack.h's yn(query) macro collides with the
 * yn(int,double) Bessel function that SDL_stdinc.h drags in via
 * <math.h> on Apple's SDK. sdlmain.c includes SDL2's headers itself,
 * ahead of hack.h, to keep that collision from ever happening.
 */

extern struct window_procs sdl_procs;

/*
 * This port keeps a small fixed table of window "slots" rather than a
 * real allocator -- UnNetHack never has more than a handful of windows
 * open at once (message, map, status, plus one or two menu/text windows).
 */
#define SDL_MAXWIN 40
#define SDL_MAXTEXTLINES 2000  /* generous cap for menu/text/message scrollback */

typedef struct sdl_menu_item {
    anything id;
    long count;
    char accelerator;
    char group_accel;
    int attr;
    char *str;
    int glyph;
    unsigned presel;
    boolean selected;
} sdl_menu_item;

typedef struct sdl_window {
    boolean inuse;
    int type;             /* NHW_MESSAGE, NHW_STATUS, NHW_MAP, NHW_MENU, NHW_TEXT */

    /* NHW_MESSAGE / NHW_TEXT / NHW_STATUS: plain scrolling text lines */
    char **lines;
    int numlines;
    int linecap;

    /* NHW_MAP: glyph grid, one entry per dungeon cell */
    int *glyphs;
    int *bg_glyphs;
    coordxy cursx, cursy;

    /* NHW_MENU: selectable entries built via start/add/end_menu */
    sdl_menu_item *items;
    int numitems;
    int itemcap;
    char *menuprompt;
    boolean menu_in_progress; /* true between start_menu() and end_menu() */
} sdl_window;

extern sdl_window sdl_windows[SDL_MAXWIN];

/* window_procs entry points -- "sdl_" prefixed to match the struct table */
void sdl_init_nhwindows(int *argc, char **argv);
void sdl_player_selection(void);
void sdl_askname(void);
void sdl_get_nh_event(void);
void sdl_exit_nhwindows(const char *);
void sdl_suspend_nhwindows(const char *);
void sdl_resume_nhwindows(void);
winid sdl_create_nhwindow(int type);
void sdl_clear_nhwindow(winid wid);
void sdl_display_nhwindow(winid wid, boolean block);
void sdl_destroy_nhwindow(winid wid);
void sdl_curs(winid wid, int x, int y);
void sdl_putstr(winid wid, int attr, const char *text);
#ifdef FILE_AREAS
void sdl_display_file(const char *farea, const char *filename,
                       boolean must_exist);
#else
void sdl_display_file(const char *filename, boolean must_exist);
#endif
void sdl_start_menu(winid wid);
void sdl_add_menu(winid wid, int glyph, int cnt, const ANY_P *identifier,
                   char accelerator, char group_accel, int attr,
                   const char *str, unsigned int presel);
void sdl_end_menu(winid wid, const char *prompt);
int sdl_select_menu(winid wid, int how, MENU_ITEM_P **selected);
void sdl_update_inventory(void);
void sdl_mark_synch(void);
void sdl_wait_synch(void);
#ifdef CLIPPING
void sdl_cliparound(int x, int y);
#endif
void sdl_print_glyph(winid wid, coordxy x, coordxy y, int glyph, int bg_glyph);
void sdl_raw_print(const char *str);
void sdl_raw_print_bold(const char *str);
int sdl_nhgetch(void);
int sdl_nh_poskey(coordxy *x, coordxy *y, int *mod);
void sdl_nhbell(void);
int sdl_doprev_message(void);
char sdl_yn_function(const char *question, const char *choices, char def);
void sdl_getlin(const char *question, char *input);
int sdl_get_ext_cmd(void);
void sdl_number_pad(int state);
void sdl_delay_output(void);
void sdl_start_screen(void);
void sdl_end_screen(void);
void sdl_outrip(winid wid, int how);

#endif /* WINSDL_H */
