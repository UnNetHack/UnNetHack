/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursmesg.h"
#include <ctype.h>

/* player can type ESC at More>> prompt to avoid seeing more messages
   for the current move; but hero might get more than one move per turn,
   so the input routines need to be able to cancel this */
long curs_mesg_suppress_seq = -1L;
/* if a message is marked urgent, existing suppression will be overridden
   so that messages resume being shown; this is used in case the urgent
   message triggers More>> for the previous message and the player responds
   with ESC; we need to avoid initiating suppression in that situation */
boolean curs_mesg_no_suppress = FALSE;

/* Message window routines for curses interface */

/* Private declatations */

typedef struct nhpm {
    char *str;                  /* Message text */
    long turn;                  /* Turn number for message */
    struct nhpm *prev_mesg;     /* Pointer to previous message */
    struct nhpm *next_mesg;     /* Pointer to next message */
} nhprev_mesg;

static void scroll_window(winid wid);
static void unscroll_window(winid wid);
static void directional_scroll(winid wid, int nlines);
static void mesg_add_line(char *mline);
static nhprev_mesg *get_msg_line(boolean reverse, int mindex);

static int turn_lines = 1;
static int mx = 0;
static int my = 0;              /* message window text location */
static nhprev_mesg *first_mesg = NULL;
static nhprev_mesg *last_mesg = NULL;
static int max_messages;
static int num_messages = 0;



/* Write a string to the message window.  Attributes set by calling function. */

void
curses_message_win_puts(const char *message, boolean recursed)
{
    int height, width, linespace;
    char *tmpstr;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    boolean border = curses_window_has_border(MESSAGE_WIN);
    int message_length = strlen(message);
    int border_space = 0;
    static long suppress_turn = -1;

    if (strncmp("Count:", message, 6) == 0) {
        curses_count_window(message);
        return;
    }

    if (curs_mesg_suppress_seq == hero_seq) {
        return; /* user has typed ESC to avoid seeing remaining messages. */
    }

    curses_get_window_size(MESSAGE_WIN, &height, &width);
    if (border) {
        border_space = 1;
        if (mx < 1) {
            mx = 1;
        }
        if (my < 1) {
            my = 1;
        }
    }

    linespace = ((width + border_space) - 3) - mx;

    if (strcmp(message, "#") == 0) {    /* Extended command or Count: */
        if ((strcmp(toplines, "#") != 0) && (my >= (height - 1 + border_space)) && (height != 1)) {     /* Bottom of message window */
            scroll_window(MESSAGE_WIN);
            mx = width;
            my--;
            strcpy(toplines, message);
        }

        return;
    }

    if (!recursed) {
        strcpy(toplines, message);
        mesg_add_line((char *) message);
    }

    /* -2: room for trailing ">>" (if More>> is needed) or leading "  "
       (if combining this message with preceding one) */
    linespace = (width - 1) - 2 - (mx - border_space);

    if (linespace < message_length) {
        if (my - border_space >= height - 1) {
            /* bottom of message win */
            if (++turn_lines > height || (turn_lines == height && mx > border_space)) {
                 /* pause until key is hit - ESC suppresses further messages
                    this turn unless an urgent message is being delivered */
                if (curses_more() == '\033' && !curs_mesg_no_suppress) {
                    curs_mesg_suppress_seq = hero_seq;
                    return;
                }
                /* turn_lines reset to 0 by more()->block()->got_input() */
            } else {
                scroll_window(MESSAGE_WIN);
            }
        } else {
            if (mx != border_space) {
                my++;
                mx = border_space;
                ++turn_lines;
            }
        }
    } else { /* don't need to move to next line */
        /* if we aren't at the start of the line, we're combining multiple
           messages on one line; use 2-space separation */
        if (mx > border_space) {
            waddstr(win, "  ");
            mx += 2;
        }
    }

    if (height > 1) {
        curses_toggle_color_attr(win, NONE, A_BOLD, ON);
    }

    if ((mx == border_space) && ((message_length + 2) > width)) {
        tmpstr = curses_break_str(message, (width - 2), 1);
        mvwprintw(win, my, mx, "%s", tmpstr);
        mx += strlen(tmpstr);
        if (strlen(tmpstr) < (width - 2)) {
            mx++;
        }
        free(tmpstr);
        if (height > 1) {
            curses_toggle_color_attr(win, NONE, A_BOLD, OFF);
        }
        curses_message_win_puts(tmpstr = curses_str_remainder(message, (width - 2), 1),
                                TRUE);
        free(tmpstr);
    } else {
        mvwprintw(win, my, mx, "%s", message);
        curses_toggle_color_attr(win, NONE, A_BOLD, OFF);
        mx += message_length + 1;
    }
    wrefresh(win);
}

void
curses_got_input(void)
{
    /* if messages are being suppressed, reenable them */
    curs_mesg_suppress_seq = -1L;

    /* misleadingly named; represents number of lines delivered since
       player was sure to have had a chance to read them; if player
       has just given input then there aren't any such lines right now;
       that includes responding to More>> even though it stays same turn */
    turn_lines = 0;
}

int
curses_block(boolean noscroll) /* noscroll - blocking because of msgtype
                                * = stop/alert else blocking because
                                * window is full, so need to scroll after */
{
    static const char resp[] = " \r\n\033"; /* space, enter, esc */
    static int prev_x = -1, prev_y = -1, blink = 0;
    int height, width, moreattr, oldcrsr, ret = 0,
        brdroffset = curses_window_has_border(MESSAGE_WIN) ? 1 : 0;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);

    curses_get_window_size(MESSAGE_WIN, &height, &width);
    /* -3: room for ">>_" */
    if (mx - brdroffset > width - 3) {
        if (my - brdroffset < height - 1) {
            ++my, mx = brdroffset;
        } else {
            mx = width - 3 + brdroffset;
        }
    }
    /* if ">>" (--More--) is being rendered at the same spot as before,
       toggle attributes so that the first '>' starts blinking if it wasn't
       or stops blinking if it was */
    if (mx == prev_x && my == prev_y) {
        blink = 1 - blink;
    } else {
        prev_x = mx, prev_y = my;
        blink = 0;
    }
    moreattr = !iflags.wc2_guicolor ? (int) A_REVERSE : NONE;
    curses_toggle_color_attr(win, MORECOLOR, moreattr, ON);
    if (blink) {
        wattron(win, A_BLINK);
        mvwprintw(win, my, mx, ">"), mx += 1;
        wattroff(win, A_BLINK);
        waddstr(win, ">"), mx += 1;
    } else {
        mvwprintw(win, my, mx, ">>"), mx += 2;
    }
    curses_toggle_color_attr(win, MORECOLOR, moreattr, OFF);
    wrefresh(win);

    /* cancel mesg suppression; all messages will have had chance to be read */
    curses_got_input();

    oldcrsr = curs_set(1);
    do {
        ret = wgetch(win);
        if (ret == ERR || ret == '\0') {
            ret = '\n';
        }
        /* msgtype=stop should require space/enter rather than any key,
           as we want to prevent YASD from direction keys. */
    } while (!index(resp, (char) ret));
    if (oldcrsr >= 0) {
        (void) curs_set(oldcrsr);
    }

    if (height == 1) {
        curses_clear_unhighlight_message_window();
    } else {
        mx -= 2, mvwprintw(win, my, mx, "  "); /* back up and blank out ">>" */
        if (!noscroll) {
            scroll_window(MESSAGE_WIN);
        }
        wrefresh(win);
    }

    return ret;
}

int
curses_more(void)
{
    return curses_block(FALSE);
}


/* Clear the message window if one line; otherwise unhighlight old messages */

void
curses_clear_unhighlight_message_window(void)
{
    int mh, mw, count;
    boolean border = curses_window_has_border(MESSAGE_WIN);
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);

    turn_lines = 1;

    curses_get_window_size(MESSAGE_WIN, &mh, &mw);

    mx = 0;

    if (border) {
        mx++;
    }

    if (mh == 1) {
        curses_clear_nhwin(MESSAGE_WIN);
    } else {
        mx += mw;               /* Force new line on new turn */

        if (border) {

            for (count = 0; count < mh; count++) {
                mvwchgat(win, count + 1, 1, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        } else {
            for (count = 0; count < mh; count++) {
                mvwchgat(win, count, 0, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        }

        wnoutrefresh(win);
    }
}


/* Reset message window cursor to starting position, and display most
recent messages. */

void
curses_last_messages(void)
{
    boolean border = curses_window_has_border(MESSAGE_WIN);

    if (border) {
        mx = 1;
        my = 1;
    } else {
        mx = 0;
        my = 0;
    }

    nhprev_mesg *mesg;
    int i;
    for (i = (num_messages - 1); i > 0; i--) {
        mesg = get_msg_line(TRUE, i);
        if (mesg && mesg->str && strcmp(mesg->str, ""))
            curses_message_win_puts(mesg->str, TRUE);
    }
    curses_message_win_puts(toplines, TRUE);
}


/* Initialize list for message history */

void
curses_init_mesg_history(void)
{
    max_messages = iflags.msg_history;

    if (max_messages < 1) {
        max_messages = 1;
    }

    if (max_messages > MESG_HISTORY_MAX) {
        max_messages = MESG_HISTORY_MAX;
    }
}


/* Display previous message window messages in reverse chron order */

void
curses_prev_mesg(void)
{
    int count;
    winid wid;
    long turn = 0;
    anything identifier;
    nhprev_mesg *mesg;
    menu_item *selected = NULL;

    wid = curses_get_wid(NHW_MENU);
    curses_create_nhmenu(wid);
    identifier = zeroany;

    for (count = 0; count < num_messages; count++) {
        mesg = get_msg_line(TRUE, count);
        if ((turn != mesg->turn) && (count != 0)) {
            curses_add_menu(wid, NO_GLYPH, MENU_DEFCNT, &identifier, 0, 0, A_NORMAL,
                            "---", FALSE);
        }
        curses_add_menu(wid, NO_GLYPH, MENU_DEFCNT, &identifier, 0, 0, A_NORMAL,
                        mesg->str, FALSE);
        turn = mesg->turn;
    }

    curses_end_menu(wid, "");
    curses_select_menu(wid, PICK_NONE, &selected);
}


/* Shows Count: in a separate window, or at the bottom of the message
window, depending on the user's settings */

void
curses_count_window(const char *count_text)
{
    static WINDOW *countwin = NULL;
    int winx, winy;
    int messageh, messagew, border;

    if (!count_text) {
        if (countwin) {
            delwin(countwin);
            countwin = NULL;
        }
        counting = FALSE;
        return;
    }

    /* position of message window, not current position within message window
       (so <0,0> for align_message:Top but will vary for other alignings) */
    curses_get_window_xy(MESSAGE_WIN, &winx, &winy);
    /* size of message window, with space for borders already subtracted */
    curses_get_window_size(MESSAGE_WIN, &messageh, &messagew);

    /* decide where to put the one-line counting window */
    border = curses_window_has_border(MESSAGE_WIN) ? 1 : 0;
    winx += border; /* first writeable message column */
    winy += border + (messageh - 1); /* last writable message line */

    /* if most recent message (probably prompt leading to this instance of
       counting window) is going to be covered up, scroll mesgs up a line */
    if (!counting && my == border + (messageh - 1) && mx > border) {
        scroll_window(MESSAGE_WIN);
        if (messageh > 1) {
            /* handling for next message will behave as if we're currently
               positioned at the end of next to last line of message window */
            my = border + (messageh - 1) - 1;
            mx = border + (messagew - 1); /* (0 + 80 - 1) or (1 + 78 - 1) */
        } else {
            /* for a one-line window, use beginning of only line instead */
            my = mx = border; /* 0 or 1 */
        }
        /* wmove(curses_get_nhwin(MESSAGE_WIN), my, mx); -- not needed */
    }
    /* in case we're being called from clear_nhwindow(MESSAGE_WIN)
       which gets called for every command keystroke; it sends an
       empty string to get the scroll-up-one-line effect above and
       we want to avoid the curses overhead for the operations below... */
    if (!*count_text) {
        return;
    }

    counting = TRUE;
#ifdef PDCURSES
    if (countwin) {
        curses_destroy_win(countwin), countwin = NULL;
    }
#endif /* PDCURSES */
    /* this used to specify a width of 25; that was adequate for 'Count: 123'
       but not for dolook's autodescribe when it refers to a named monster */
    if (!countwin) {
        countwin = newwin(1, messagew, winy, winx);
    }
    werase(countwin);

    mvwprintw(countwin, 0, 0, "%s", count_text);
    wrefresh(countwin);
}

/* Gets a "line" (buffer) of input. */
void
curses_message_win_getline(const char *prompt, char *answer, int buffer)
{
    int height, width; /* of window */
    char *tmpbuf, *p_answer; /* combined prompt + answer */
    int nlines, maxlines, i; /* prompt + answer */
    int promptline;
    int promptx;
    char **linestarts; /* pointers to start of each line */
    char *tmpstr; /* for free() */
    int maxy, maxx; /* linewrap / scroll */
    int ch;

    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    int border_space = 0;
    int len = 0; /* of answer string */
    boolean border = curses_window_has_border(MESSAGE_WIN);
    int orig_cursor = curs_set(0);

    curses_get_window_size(MESSAGE_WIN, &height, &width);
    if (border) {
        border_space = 1;
        if (mx < 1) mx = 1;
        if (my < 1) my = 1;
    }
    maxy = height - 1 + border_space;
    maxx = width - 1 + border_space;

    tmpbuf = (char *)malloc(strlen(prompt) + buffer + 2);
    maxlines = buffer / width * 2;
    strcpy(tmpbuf, prompt);
    strcat(tmpbuf, " ");
    nlines = curses_num_lines(tmpbuf,width);
    maxlines += nlines * 2;
    linestarts = (char **)malloc(sizeof(char*) * maxlines);
    p_answer = tmpbuf + strlen(tmpbuf);
    linestarts[0] = tmpbuf;

    if (mx > border_space) { /* newline */
        if (my >= maxy) scroll_window(MESSAGE_WIN);
        else my++;
        mx = border_space;
    }

    curses_toggle_color_attr(win, NONE, A_BOLD, ON);

    for (i = 0; i < nlines-1; i++) {
        tmpstr = curses_break_str(linestarts[i],width-1,1);
        linestarts[i+1] = linestarts[i] + strlen(tmpstr);
        if (*linestarts[i+1] == ' ') linestarts[i+1]++;
        mvwaddstr(win,my,mx,tmpstr);
        free(tmpstr);
        if (++my >= maxy) {
            scroll_window(MESSAGE_WIN);
            my--;
        }
    }
    mvwaddstr(win,my,mx,linestarts[nlines-1]);
    mx = promptx = strlen(linestarts[nlines-1]) + border_space;
    promptline = nlines - 1;

    while(1) {
        mx = strlen(linestarts[nlines - 1]) + border_space;
        if (mx > maxx) {
            if (nlines < maxlines) {
                tmpstr = curses_break_str(linestarts[nlines - 1], width - 1, 1);
                mx = strlen(tmpstr) + border_space;
                mvwprintw(win, my, mx, "%*c", maxx - mx + 1, ' ');
                if (++my > maxy) {
                    scroll_window(MESSAGE_WIN);
                    my--;
                }
                mx = border_space;
                linestarts[nlines] = linestarts[nlines - 1] + strlen(tmpstr);
                if (*linestarts[nlines] == ' ') linestarts[nlines]++;
                mvwaddstr(win, my, mx, linestarts[nlines]);
                mx = strlen(linestarts[nlines]) + border_space;
                nlines++;
                free(tmpstr);
            } else {
                p_answer[--len] = '\0';
                mvwaddch(win, my, --mx, ' ');
            }
        }
        wmove(win, my, mx);
        curs_set(1);
        wrefresh(win);
        ch = getch();
        curs_set(0);
        switch(ch) {
        case '\033': /* DOESCAPE */
            /* blank the input but don't exit */
            while(nlines  - 1 > promptline) {
                if (nlines-- > height) {
                    unscroll_window(MESSAGE_WIN);
                    tmpstr = curses_break_str(linestarts[nlines - height], width - 1, 1);
                    mvwaddstr(win, border_space, border_space, tmpstr);
                    free(tmpstr);
                } else {
                    mx = border_space;
                    mvwprintw(win, my, mx, "%*c", maxx - mx, ' ');
                    my--;
                }
            }
            mx = promptx;
            mvwprintw(win, my, mx, "%*c", maxx - mx, ' ');
            *p_answer = '\0';
            len = 0;
            break;
        case ERR: /* should not happen */
            *answer = '\0';
            goto alldone;

        case '\r':
        case '\n':
            (void) strncpy(answer, p_answer, buffer);
            answer[buffer - 1] = '\0';
            Strcpy(toplines, tmpbuf);
            mesg_add_line(tmpbuf);
#if 1
            /* position at end of current line so next message will be
               written on next line regardless of whether it could fit here */
            mx = border_space ? (width + 1) : (width - 1);
            wmove(win, my, mx);
#else       /* after various other changes, this resulted in getline()
             * prompt+answer being following by a blank message line */
            if (++my > maxy) {
                scroll_window(MESSAGE_WIN);
                my--;
            }
            mx = border_space;
#endif /*0*/
            goto alldone;

        case '\b':
        case KEY_BACKSPACE:
            if (len < 1) {
                len = 1;
                mx = promptx;
            }
            p_answer[--len] = '\0';
            mvwaddch(win, my, --mx, ' ');
            /* try to unwrap back to the previous line if there is one */
            if (nlines > 1 && strlen(linestarts[nlines - 2]) < width) {
                mvwaddstr(win, my - 1, border_space, linestarts[nlines - 2]);
                if (nlines-- > height) {
                    unscroll_window(MESSAGE_WIN);
                    tmpstr = curses_break_str(linestarts[nlines - height], width - 1, 1);
                    mvwaddstr(win, border_space, border_space, tmpstr);
                    free(tmpstr);
                } else {
                    /* clean up the leftovers on the next line, if we didn't scroll it away */
                    mvwprintw(win, my--, border_space, "%*c", (int) strlen(linestarts[nlines]), ' ');
                }
            }
            break;
        default:
            p_answer[len++] = ch;
            if (len >= buffer) len = buffer-1;
            else mvwaddch(win, my, mx, ch);
            p_answer[len] = '\0';
        }
    }

 alldone:
    free(linestarts);
    free(tmpbuf);
    curses_toggle_color_attr(win, NONE, A_BOLD, OFF);
    curs_set(orig_cursor);
    return;
}

/* Scroll lines upward in given window, or clear window if only one line. */
static void
scroll_window(winid wid)
{
    directional_scroll(wid,1);
}

static void
unscroll_window(winid wid)
{
    directional_scroll(wid,-1);
}

static void
directional_scroll(winid wid, int nlines)
{
    int wh, ww, s_top, s_bottom;
    boolean border = curses_window_has_border(wid);
    WINDOW *win = curses_get_nhwin(wid);

    curses_get_window_size(wid, &wh, &ww);
    if (wh == 1) {
        curses_clear_nhwin(wid);
        return;
    }
    if (border) {
        s_top = 1;
        s_bottom = wh;
    } else {
        s_top = 0;
        s_bottom = wh - 1;
    }
    scrollok(win, TRUE);
    wsetscrreg(win, s_top, s_bottom);
    wscrl(win, nlines);
    scrollok(win, FALSE);
    if (wid == MESSAGE_WIN) {
        if (border)
            mx = 1;
        else
            mx = 0;
    }
    if (border) {
        box(win, 0, 0);
    }
    wrefresh(win);
}


/* Add given line to message history */

static void
mesg_add_line(char *mline)
{
    nhprev_mesg *tmp_mesg = NULL;
    nhprev_mesg *current_mesg = malloc(sizeof (nhprev_mesg));

    current_mesg->str = curses_copy_of(mline);
    current_mesg->turn = moves;
    current_mesg->next_mesg = NULL;

    if (num_messages == 0) {
        first_mesg = current_mesg;
    }

    if (last_mesg != NULL) {
        last_mesg->next_mesg = current_mesg;
    }
    current_mesg->prev_mesg = last_mesg;
    last_mesg = current_mesg;

    if (num_messages < max_messages) {
        num_messages++;
    } else {
        tmp_mesg = first_mesg->next_mesg;
        free(first_mesg->str);
        free(first_mesg);
        first_mesg = tmp_mesg;
    }
}


/* Returns specified line from message history, or NULL if out of bounds */

static nhprev_mesg *
get_msg_line(boolean reverse, int mindex)
{
    int count;
    nhprev_mesg *current_mesg;

    if (reverse) {
        current_mesg = last_mesg;
        for (count = 0; count < mindex; count++) {
            if (current_mesg == NULL) {
                return NULL;
            }
            current_mesg = current_mesg->prev_mesg;
        }
        return current_mesg;
    } else {
        current_mesg = first_mesg;
        for (count = 0; count < mindex; count++) {
            if (current_mesg == NULL) {
                return NULL;
            }
            current_mesg = current_mesg->next_mesg;
        }
        return current_mesg;
    }
}
