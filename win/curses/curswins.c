#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "curswins.h"

/* Window handling for curses interface */

/* Private declarations */

typedef struct nhw
{
    winid nhwin;  /* NetHack window id */
    WINDOW *curwin; /* Curses window pointer */
    int width;  /* Usable width not counting border */
    int height; /* Usable height not counting border */
    int x;  /* start of window on terminal (left) */
    int y;  /* start of window on termial (top) */
    int orientation;    /* Placement of window relative to map */
    boolean border; /* Whether window has a visible border */
    struct nhw *prev_window;    /* Pointer to previous entry */
    struct nhw *next_window;    /* Pointer to next entry */
} nethack_window;

static void refresh_map_window(void);

static nethack_window *nhwins = NULL;  /* NetHack window array */



/* Create a window with the specified size and orientation */

WINDOW *curses_create_window(int width, int height, orient orientation)
{
    int mapx, mapy, maph, mapw = 0;
    int startx = 0;
    int starty = 0;
    WINDOW *win;
    boolean map_border = FALSE;
        
    if ((orientation == UP) || (orientation == DOWN) ||
     (orientation == LEFT))
    {
        map_border = curses_window_has_border(MAP_WIN);
        curses_get_window_xy(MAP_WIN, &mapx, &mapy);
        curses_get_window_size(MAP_WIN, &maph, &mapw);
    }
    width += 2;    /* leave room for bounding box */
    height += 2;
    if ((width > term_cols) || (height > term_rows))
        panic("curses_create_window: Terminal too small for dialog window");
    switch (orientation)
    {
        case CENTER:
        {
            startx = (term_cols / 2) - (width / 2);
            starty = (term_rows / 2) - (height / 2);
            break;
        }
        case UP:
        {
            startx = (mapw / 2) - (width / 2) + mapx;
            starty = mapy;
            break;
        }
        case DOWN:
        {
            startx = (mapw / 2) - (width / 2) + mapx;
            starty = height - mapy - 1;
            break;
        }
        case LEFT:
        {
            if (map_border && (width < term_cols))
                startx = 1;
            else
                startx = 0;
            starty = term_rows - height;
            break;
        }
        case RIGHT:
        {
            startx = term_cols - width;
            starty = 0;
            break;
        }
        default:
        {
            panic("curses_create_window: Bad orientation");
        }
    }
    win = newwin(height, width, starty, startx);
    curses_toggle_color_attr(win, DIALOG_BORDER_COLOR, NONE, ON);
    box(win, 0, 0);
    curses_toggle_color_attr(win, DIALOG_BORDER_COLOR, NONE, OFF);
    return win;
}


/* Erase and delete curses window, and refresh standard windows */

void curses_destroy_win(WINDOW *win)
{
    werase(win);
    wrefresh(win);
    delwin(win);
    curses_refresh_nethack_windows();
}


/* Refresh nethack windows if they exist, or base window if not */

void curses_refresh_nethack_windows()
{
    WINDOW *status_window, *message_window;
    
    status_window = curses_get_nhwin(STATUS_WIN);
    message_window = curses_get_nhwin(MESSAGE_WIN);
    
    if (!iflags.window_inited)
    {
        /* Main windows not yet displayed; refresh base window instead */
        touchwin(stdscr);
        refresh();
    }
    else
    {
        touchwin(status_window);
        wnoutrefresh(status_window);
        box(mapborderwin, 0, 0);
        wrefresh(mapborderwin);
        touchwin(message_window);
        wnoutrefresh(message_window);
        doupdate();
        refresh_map_window();
    }
}


/* Return curses window pointer for given NetHack winid */

WINDOW *curses_get_nhwin(winid wid)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
            return winptr->curwin;
        winptr = winptr->next_window;
    }
    
    return NULL;    /* Not found */
}


/* Add curses window pointer and window info to list for given NetHack winid */

void curses_add_nhwin(winid wid, int height, int width, int y, int x,
  orient orientation, boolean border)
{
    WINDOW *win;
    nethack_window *new_win;
    nethack_window *winptr = nhwins;
    int real_width = width;
    int real_height = height;
    
    new_win = malloc(sizeof(nethack_window));
    new_win->nhwin = wid;
    new_win->border = border;
    new_win->width = width;
    new_win->height = height;
    new_win->x = x;
    new_win->y = y;
    new_win->orientation = orientation;
    if (border)
    {
        real_width += 2;    /* leave room for bounding box */
        real_height += 2;
    }
    new_win->next_window = NULL;
    if (winptr == NULL)
    {
        new_win->prev_window = NULL;
        nhwins = new_win;
    }
    else
    {
        while (winptr->next_window != NULL)
            winptr = winptr->next_window;
        new_win->prev_window = winptr;
        winptr->next_window = new_win;
    }
    if (wid != MAP_WIN)
    {
        win = newwin(real_height, real_width, y, x);
        if (border)
        {
            box(win, 0, 0);
        }
    }
    else
    {
        if (border)
        {
            box(mapborderwin, 0, 0);
        }
        win = newpad(ROWNO, COLNO);
        mapwin = win;
    }
    new_win->curwin = win;
}


/* refresh a curses window via given nethack winid */

void curses_refresh_nhwin(winid wid)
{
    if (wid != MAP_WIN)
        wrefresh(curses_get_nhwin(wid));
    else
        refresh_map_window();
}


/* Delete curses window via given NetHack winid and remove entry from list */

void curses_del_nhwin(winid wid)
{
    nethack_window *tmpwin;
    nethack_window *winptr = nhwins;
    
    if (curses_is_menu(wid) || curses_is_text(wid))
    {
        curses_del_menu(wid);
    }
    
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            if (winptr->prev_window != NULL)
            {
                tmpwin = winptr->prev_window;
                tmpwin->next_window = winptr->next_window;
            }
            else
            {
                nhwins = winptr->next_window;   /* New head mode, or NULL */
            }
            if (winptr->next_window != NULL)
            {
                tmpwin = winptr->next_window;
                tmpwin->prev_window = winptr->prev_window;
            }
            free(winptr);
            break;
        }
        winptr = winptr->next_window;
    }
}


/* Print a single character in the given window at the given coordinates */

void curses_putch(winid wid, int x, int y, int ch, int color, int attr)
{
    WINDOW *win;
    boolean border = curses_window_has_border(wid);
    
    if (border && (wid != MAP_WIN))
    {
        x++;
        y++;
    }
    win = curses_get_nhwin(wid);
    curses_toggle_color_attr(win, color, attr, ON);
#ifdef PDCURSES
    mvwaddrawch(win, y, x, ch);
#else
    mvwaddch(win, y, x, ch);
#endif
    if (wid == MAP_WIN)
    {
        refresh_map_window();
    }
else
    {
        wrefresh(win);
    }
    
    curses_toggle_color_attr(win, color, attr, OFF);
}


/* Get x, y coordinates of curses window on the physical terminal window */

void curses_get_window_xy(winid wid, int *x, int *y)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            *x = winptr->x;
            *y = winptr->y;
            break;
        }
        winptr = winptr->next_window;
    }
}


/* Get usable width and height curses window on the physical terminal window */

void curses_get_window_size(winid wid, int *height, int *width)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            *height = winptr->height;
            *width = winptr->width;
            break;
        }
        winptr = winptr->next_window;
    }
}


/* Determine if given window has a visible border */

boolean curses_window_has_border(winid wid)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            return winptr->border;
        }
        winptr = winptr->next_window;
    }
    
    return FALSE;
}


/* Determine if window for given winid exists */

boolean curses_window_exists(winid wid)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            return TRUE;
        }
        winptr = winptr->next_window;
    }
    
    return FALSE;
}


/* Return the orientation of the specified window */

int curses_get_window_orientation(winid wid)
{
    nethack_window *winptr = nhwins;
 
    while (winptr != NULL)
    {
        if (winptr->nhwin == wid)
        {
            return winptr->orientation;
        }
        winptr = winptr->next_window;
    }
    
    return UNDEFINED;   /* Not found */
}


/* Output a line of text to specified NetHack window with given coordinates
and text attributes */

void curses_puts(winid wid, int attr, const char *text)
{
    anything *identifier;
    WINDOW *win = curses_get_nhwin(wid);
    
    curses_toggle_color_attr(win, NONE, attr, ON);
    if (wid == MESSAGE_WIN)
    {
        curses_message_win_puts(text, FALSE);
        return;
    }
    
    if (wid == STATUS_WIN)
    {
        curses_update_stats();  /* We will do the write ourselves */
        return;
    }
    
    if (curses_is_menu(wid) || curses_is_text(wid))
    {
        if (!curses_menu_exists(wid))
        {
            panic("curses_puts: Attempted write to nonexistant window!"); 
        }
        identifier = malloc(sizeof(anything));
        identifier->a_void = NULL;
        curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, attr, text,
         FALSE);
    }
    else
    {
        waddstr(win, text);
        wrefresh(win);
    }
    curses_toggle_color_attr(win, NONE, attr, OFF);
}


/* Clear the contents of a window via the given NetHack winid */

void curses_clear_nhwin(winid wid)
{
    WINDOW *win = curses_get_nhwin(wid);
    boolean border = curses_window_has_border(wid);
    
    werase(win);
    if (border && (wid != MAP_WIN))
        box(win, 0, 0);
    if (wid != MAP_WIN)
    {
        wrefresh(win);
    }
    else
    {
        refresh_map_window();
    }
}


/* Refresh visible portion of map window */

static void refresh_map_window(void)
{
    int mapwinx, mapwiny, maph, mapw, mapwinw, mapwinh, mapx, mapy;
    WINDOW *map_window = curses_get_nhwin(MAP_WIN);
    
    curses_get_window_xy(MAP_WIN, &mapwinx, &mapwiny);
    curses_get_window_size(MAP_WIN, &mapwinh, &mapwinw);
    maph = ROWNO;
    mapw = COLNO;
    mapx = u.ux - (mapwinw / 2);
    if ((mapx + mapwinw) > mapw)
    {
        mapx = mapw - mapwinw;
    }
    if (mapx < 0)
    {
        mapx = 0;
    }
    mapy = u.uy - (mapwinh / 2);
    if ((mapy + mapwinh) > maph)
    {
        mapy = maph - mapwinh;
    }
    if (mapy < 0)
    {
        mapy = 0;
    }
    prefresh(map_window, mapy, mapx, mapwiny, mapwinx, mapwiny + mapwinh - 1, 
     mapwinx + mapwinw - 1);
}

