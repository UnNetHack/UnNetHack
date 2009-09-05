#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursmisc.h"
#include "func_tab.h"
#include "dlb.h"

/* Misc. curses interface functions */

static int curs_x = -1;
static int curs_y = -1;
static winid curs_win = 0;


/* Read a character of input from the user */

int curses_read_char()
{
    int ch;

    ch = getch();
    if (!ch)
    {
        ch = '\033'; /* map NUL to ESC since nethack doesn't expect NUL */
    }
    
    if ((ch >= 0x197) && (ch <= 0x1ba)) /* Alt/Meta key handling */
    {
        ch = (ch - 0x140) | 0x80;
    }

#ifdef KEY_RESIZE
    /* Handle resize events via get_nh_event, not this code */
    if (ch == KEY_RESIZE)
    {
        ch = '\033'; /* NetHack doesn't know what to do with KEY_RESIZE */
    }
#endif

    /* Handle arrow keys */
    
    switch (ch)
    {
        case KEY_LEFT:
        {
            if (iflags.num_pad)
            {
                ch = '4';
            }
            else
            {
                ch = 'h';
            }
            break;
        }
        case KEY_RIGHT:
        {
            if (iflags.num_pad)
            {
                ch = '6';
            }
            else
            {
                ch = 'l';
            }
            break;
        }
        case KEY_UP:
        {
            if (iflags.num_pad)
            {
                ch = '8';
            }
            else
            {
                ch = 'k';
            }
            break;
        }
        case KEY_DOWN:
        {
            if (iflags.num_pad)
            {
                ch = '2';
            }
            else
            {
                ch = 'j';
            }
            break;
        }
    }

    return ch;
}

/* Turn on or off the specified color and / or attribute */

void curses_toggle_color_attr(WINDOW *win, int color, int attr, int onoff)
{
#ifdef TEXTCOLOR
    int curses_color, curses_attr;

    switch (attr)
    {
        case ATR_NONE:
        {
            curses_attr = A_NORMAL;
            break;
        }
        case ATR_ULINE:
        {
            curses_attr = A_UNDERLINE;
            break;
        }
        case ATR_BOLD:
        {
            curses_attr = A_BOLD;
            break;
        }
        case ATR_BLINK:
        {
            curses_attr = A_BLINK;
            break;
        }
        case ATR_INVERSE:
        {
            curses_attr = A_REVERSE;
            break;
        }
        default:
        {
            curses_attr = A_NORMAL;
        }
    }
    if (color == 0) /* make black fg visible */
#ifdef CURSES_VERSION
        color = 16;
#else
        color = 8;
#endif
    curses_color = color + 1;
    if (curses_color > 8)
        curses_color -= 8;
    if (onoff == ON)    /* Turn on color/attributes */
    {
        if ((attr != NONE) && iflags.use_color)
            wattron(win, curses_attr);
        if ((color != NONE) && iflags.use_color)
        {
            if ((color > 7) && iflags.use_color)  /* high-intensity color */
                wattron(win, A_BOLD);
            if (iflags.use_color)
                wattron(win, COLOR_PAIR(curses_color));
        }
    }
    else                /* Turn off color/attributes */
    {
        if ((attr != NONE) && iflags.use_color)
            wattroff(win, curses_attr);
        if ((color != NONE) && iflags.use_color)
        {
            if ((color > 7) && iflags.use_color)  /* high-intensity color */
                wattroff(win, A_BOLD);
            if (iflags.use_color)
                wattroff(win, COLOR_PAIR(curses_color));
        }
    }
#endif
}


/* clean up and quit - taken from tty port */

void curses_bail(const char *mesg)
{
    clearlocks();
    curses_exit_nhwindows(mesg);
    terminate(EXIT_SUCCESS);
}


/* Return a winid for a new window of the given type */

winid curses_get_wid(int type)
{
	winid ret;
    static winid menu_wid = 20; /* Always even */
    static winid text_wid = 21; /* Always odd */

	switch (type)
	{
		case NHW_MESSAGE:
		{
			return MESSAGE_WIN;
			break;
		}
		case NHW_MAP:
		{
			return MAP_WIN;
			break;
		}
		case NHW_STATUS:
		{
			return STATUS_WIN;
			break;
		}
		case NHW_MENU:
		{
			ret = menu_wid;
			break;
		}
		case NHW_TEXT:
		{
			ret = text_wid;
			break;
		}
		default:
		{
			panic("curses_get_wid: unsupported window type");
		}
	}

	while (curses_window_exists(ret))
	{
	    ret += 2;
	    if ((ret + 2) > 10000)    /* Avoid "wid2k" problem */
	    {
	        ret -= 9900;
	    }
	}
	
	if (type == NHW_MENU)
	{
	    menu_wid += 2;
	}
	else
	{
	    text_wid += 2;
	}

	return ret;
}


/*
 * Allocate a copy of the given string.  If null, return a string of
 * zero length.
 *
 * This is taken from copy_of() in tty/wintty.c.
 */

char *curses_copy_of(const char *s)
{
    if (!s) s = "";
    return strcpy((char *) alloc((unsigned) (strlen(s) + 1)), s);
}


/* Determine the number of lines needed for a string for a dialog window
of the given width */

int curses_num_lines(const char *str, int width)
{
    int last_space, count;
    int curline = 1;
    char substr[BUFSZ];
    char tmpstr[BUFSZ];
    
    strcpy(substr, str);
    
    while (strlen(substr) > width)
    {
        last_space = 0;
        
        for (count = 0; count <= width; count++)
        {
            if (substr[count] == ' ')
            last_space = count;
        }
        if (last_space == 0)    /* No spaces found */
        {
            last_space = count - 1;
        }
        for (count = (last_space + 1); count < strlen(substr); count++)
        {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        strcpy(substr, tmpstr);
        curline++;
    }
    
    return curline;
}


/* Break string into smaller lines to fit into a dialog window of the
given width */

char *curses_break_str(const char *str, int width, int line_num)
{
    int last_space, count;
    char *retstr;
    int curline = 0;
    int strsize = strlen(str);
    char substr[strsize];
    char curstr[strsize];
    char tmpstr[strsize];
    
    strcpy(substr, str);
    
    while (curline < line_num)
    {
        if (strlen(substr) == 0 )
        {
            break;
        }
        curline++;
        last_space = 0;       
        for (count = 0; count <= width; count++)
        {
            if (substr[count] == ' ')
            {
                last_space = count;
            }
            else if (substr[count] == '\0')           
            {
                last_space = count;
                break;
            }
        }
        if (last_space == 0)    /* No spaces found */
        {
            last_space = count - 1;
        }
        for (count = 0; count < last_space; count++)
        {
            curstr[count] = substr[count];
        }
        curstr[count] = '\0';
        if (substr[count] == '\0')
        {
            break;
        }
        for (count = (last_space + 1); count < strlen(substr); count++)
        {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        strcpy(substr, tmpstr);
    }
    
    if (curline < line_num)
    {
        return NULL;
    }
    
    retstr = curses_copy_of(curstr);
    
    return retstr;
}


/* Return the remaining portion of a string after hacking-off line_num lines */

char *curses_str_remainder(const char *str, int width, int line_num)
{
    int last_space, count;
    char *retstr;
    int curline = 0;
    int strsize = strlen(str);
    char substr[strsize];
    char curstr[strsize];
    char tmpstr[strsize];
    
    strcpy(substr, str);
    
    while (curline < line_num)
    {
        if (strlen(substr) == 0 )
        {
            break;
        }
        curline++;
        last_space = 0;       
        for (count = 0; count <= width; count++)
        {
            if (substr[count] == ' ')
            {
                last_space = count;
            }
            else if (substr[count] == '\0')           
            {
                last_space = count;
                break;
            }
        }
        if (last_space == 0)    /* No spaces found */
        {
            last_space = count - 1;
        }
        for (count = 0; count < last_space; count++)
        {
            curstr[count] = substr[count];
        }
        curstr[count] = '\0';
        if (substr[count] == '\0')
        {
            break;
        }
        for (count = (last_space + 1); count < strlen(substr); count++)
        {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        strcpy(substr, tmpstr);
    }
    
    if (curline < line_num)
    {
        return NULL;
    }
    
    retstr = curses_copy_of(substr);
    
    return retstr;
}


/* Determine if the given NetHack winid is a menu window */

boolean curses_is_menu(winid wid)
{
    if ((wid > 19) && !(wid % 2))   /* Even number */
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/* Determine if the given NetHack winid is a text window */

boolean curses_is_text(winid wid)
{
    if ((wid > 19) && (wid % 2))   /* Odd number */
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/* Replace certain characters with portable drawing characters if
cursesgraphics option is enabled */

int curses_convert_glyph(int ch, int glyph)
{
    int symbol;
    
    /* Save some processing time by returning if the glyph represents
    an object that we don't have custom characters for */
    if (!glyph_is_cmap(glyph))
    {
        return ch;
    }
    
    symbol = glyph_to_cmap(glyph);
    
    /* If user selected a custom character for this object, don't
    override this. */
    if (((glyph_is_cmap(glyph)) && (ch != showsyms[symbol])))
    {
        return ch;
    }

    switch (symbol)
    {
        case S_vwall:
            return ACS_VLINE;
        case S_hwall:
            return ACS_HLINE;
        case S_tlcorn:
            return ACS_ULCORNER;
        case S_trcorn:
            return ACS_URCORNER;
        case S_blcorn:
            return ACS_LLCORNER;
        case S_brcorn:
            return ACS_LRCORNER;
        case S_crwall:
            return ACS_PLUS;
        case S_tuwall:
            return ACS_TTEE;
        case S_tdwall:
            return ACS_BTEE;
        case S_tlwall:
            return ACS_LTEE;
        case S_trwall:
            return ACS_RTEE;
        case S_tree:
            return ACS_PLMINUS;
        case S_corr:
            return ACS_CKBOARD;
        case S_litcorr:
            return ACS_CKBOARD;
    }

	return ch;
}


/* Move text cursor to specified coordinates in the given NetHack window */

void curses_move_cursor(winid wid, int x, int y)
{
    WINDOW *win = curses_get_nhwin(curs_win);

    if (wid != MAP_WIN)
    {
        return;
    }

#ifdef PDCURSES
    /* PDCurses seems to not handle wmove correctly, so we use move and
    physical screen coordinates instead */
    curses_get_window_xy(wid, &curs_x, &curs_y);
    curs_x += x;
    curs_y += y;
#else
    curs_x = x;
    curs_y = y;
#endif    
    curs_win = wid;
#ifdef PDCURSES
    move(curs_y, curs_x);
#else
    wmove(win, curs_y, curs_x);
#endif
}


/* Perform actions that should be done every turn before nhgetch() */

void curses_prehousekeeping()
{
    WINDOW *win = curses_get_nhwin(curs_win);

    if ((curs_x > -1) && (curs_y > -1))
    {
        curs_set(1);
#ifdef PDCURSES
        /* PDCurses seems to not handle wmove correctly, so we use move
        and physical screen coordinates instead */
        move(curs_y, curs_x);
#else
        wmove(win, curs_y, curs_x);
#endif
        curses_refresh_nhwin(curs_win);
    }
}


/* Perform actions that should be done every turn after nhgetch() */

void curses_posthousekeeping()
{
    curs_set(0);
    curses_decrement_highlight();
    curses_clear_unhighlight_message_window();
}


void curses_view_file(const char *filename, boolean must_exist)
{
    winid wid;
    anything *identifier;
    char buf[BUFSZ];
    menu_item *selected = NULL;
    dlb *fp = dlb_fopen(filename, "r");
    
    if ((fp == NULL) && (must_exist))
    {
        pline("Cannot open %s for reading!", filename);
    }

    if (fp == NULL)
    {
        return;
    }
    
    wid = curses_get_wid(NHW_MENU);
    curses_create_nhmenu(wid);
    identifier = malloc(sizeof(anything));
    identifier->a_void = NULL;
    
    while (dlb_fgets(buf, BUFSZ, fp) != NULL)
    {
        curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, A_NONE, buf,
         FALSE);
    }
    
    dlb_fclose(fp);
    curses_end_menu(wid, "");
    curses_select_menu(wid, PICK_NONE, &selected);
}


char *curses_rtrim(char *str)
{
    char *s;

    for(s = str; *s != '\0'; ++s);
    for(--s;isspace(*s) && s > str; --s);
    if(s == str) *s = '\0';
    else *(++s) = '\0';
}

/* Use nethack wall symbols for drawing unless cursesgraphics is
defined, in which case we use the standard curses ones.  Not sure if
this is desirable behavior since one may prefer regular lines for
borders but traditional symbols to draw rooms.  Commenting it out for
now. */

/*
 * void curses_border(WINDOW *win)
 * {
 *     if (iflags.cursesgraphics)
 *     {
 *         box(win, 0, 0);
 *     }
 *     else
 *     {
 *         wborder(win, showsyms[S_vwall], showsyms[S_vwall],
 *          showsyms[S_hwall], showsyms[S_hwall], showsyms[S_tlcorn],
 *          showsyms[S_trcorn], showsyms[S_blcorn], showsyms[S_brcorn]);
 *     }
 * }
 */

