#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursmesg.h"

/* Message window routines for curses interface */

static int turn_lines = 1;
static int mx = 0;
static int my = 0;  /* message window text location */
static nhprev_mesg *first_mesg = NULL;
static nhprev_mesg *last_mesg = NULL;
static int max_messages;
static int num_messages = 0;



/* Write a string to the message window.  Attributes set by calling function. */

void curses_message_win_puts(const char *message, boolean recursed)
{
    int height, width, linespace, count;
    char *tmpstr;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    boolean border = curses_window_has_border(MESSAGE_WIN);
    int message_length = strlen(message);
    int border_space = 0;
    
    if (!recursed)
    {
        strcpy(toplines, message);
        mesg_add_line((char *) message);
    }
    
    curses_get_window_size(MESSAGE_WIN, &height, &width);
    if (border)
    {
        border_space = 1;
        if (mx < 1)
        {
            mx = 1;
        }
        if (my < 1)
        {
            my = 1;
        }
    }
    
    linespace = ((width + border_space) - 3) - mx;
    
    if (linespace < message_length)
    {
        if (my >= (height - 1 + border_space)) /* bottom of message win */
        {
            if (turn_lines == height)
            {
                curses_more();
            }
            else
            {
                scroll_window(MESSAGE_WIN);
                turn_lines++;
            }
        }
        else
        {
            if (mx != border_space)
            {
                my++;
                mx = border_space;
            }
        }
    }

    if (height > 1)
    {
        curses_toggle_color_attr(win, NONE, ATR_BOLD, ON);
    }
    
    if ((mx == border_space) && ((message_length + 2) > width))
    {
        tmpstr = curses_break_str(message, (width - 2), 1);
        mvwprintw(win, my, mx, tmpstr);
        mx += strlen(tmpstr);
        free(tmpstr);
        if (height > 1)
        {
            curses_toggle_color_attr(win, NONE, ATR_BOLD, OFF);
        }
        wrefresh(win);
        curses_message_win_puts(curses_str_remainder(message, (width - 2), 1),
         TRUE);
    }
    else
    {
        mvwprintw(win, my, mx, message);
        curses_toggle_color_attr(win, NONE, ATR_BOLD, OFF);
    }
    wrefresh(win);
    mx += message_length + 1;
}


void curses_more()
{
    int height, width;
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);
    
    curses_get_window_size(MESSAGE_WIN, &height, &width);
    curses_toggle_color_attr(win, MORECOLOR, NONE, ON);
    mvwprintw(win, my, mx - 1, ">>");
    curses_toggle_color_attr(win, MORECOLOR, NONE, OFF);
    wrefresh(win);
    wgetch(win);
    if (height == 1)
    {
        curses_clear_unhighlight_message_window();
    }
    else
    {
        mvwprintw(win, my, mx - 1, "  ");
        scroll_window(MESSAGE_WIN);
        turn_lines = 1;
    }
}


/* Clear the message window if one line; otherwise unhighlight old messages */

void curses_clear_unhighlight_message_window()
{
    int mh, mw, count;
    boolean border = curses_window_has_border(MESSAGE_WIN);
    WINDOW *win = curses_get_nhwin(MESSAGE_WIN);

    turn_lines = 1;
    
    curses_get_window_size(MESSAGE_WIN, &mh, &mw); 
    
    mx = 0;
    
    if (border)
    {
        mx++;
    }
       
    if (mh == 1)
    {
        curses_clear_nhwin(MESSAGE_WIN);
    }
    else
    {
        mx += mw;    /* Force new line on new turn */
        
        if (border)
        {

            for (count = 0; count < mh; count++)
            {
                mvwchgat(win, count+1, 1, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        }
        else
        {
            for (count = 0; count < mh; count++)
            {
                mvwchgat(win, count, 0, mw, COLOR_PAIR(8), A_NORMAL, NULL);
            }
        }

        wrefresh(win);
    }
}


/* Reset message window cursor to starting position, and display most
recent messages. */

void curses_last_messages()
{
    boolean border = curses_window_has_border(MESSAGE_WIN);

    if (border)
    {
        mx = 1;
        my = 1;
    }
    else
    {
        mx = 0;
        my = 0;
    }
    
    pline(toplines);
}


/* Initialize list for message history */

void curses_init_mesg_history()
{
    max_messages = iflags.msg_history;
    
    if (max_messages < 1)
    {
        max_messages = 1;
    }

    if (max_messages > MESG_HISTORY_MAX)
    {
        max_messages = MESG_HISTORY_MAX;
    }
}


/* Display previous message window messages in reverse chron order */

void curses_prev_mesg()
{
    int count;
    winid wid;
    long turn = 0;
    anything *identifier;
    nhprev_mesg *mesg;
    menu_item *selected = NULL;

    wid = curses_get_wid(NHW_MENU);
    curses_create_nhmenu(wid);
    identifier = malloc(sizeof(anything));
    identifier->a_void = NULL;
    
    for (count = 0; count < num_messages; count++)
    {
        mesg = get_msg_line(TRUE, count);
        if ((turn != mesg->turn) && (count != 0))
        {
            curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, A_NONE,
             "---", FALSE);
        }
        curses_add_menu(wid, NO_GLYPH, identifier, 0, 0, A_NONE,
         mesg->str, FALSE);
        turn = mesg->turn;
    }
    
    curses_end_menu(wid, "");
    curses_select_menu(wid, PICK_NONE, &selected);
}


/* Scroll lines upward in given window, or clear window if only one line. */

static void scroll_window(winid wid)
{
    int wh, ww, s_top, s_bottom;
    boolean border = curses_window_has_border(wid);
    WINDOW *win = curses_get_nhwin(wid);
    
    curses_get_window_size(wid, &wh, &ww);
    if (wh == 1)
    {
        curses_clear_nhwin(wid);
        return;
    }
    if (border)
    {
        s_top = 1;
        s_bottom = wh;
    }
    else
    {
        s_top = 0;
        s_bottom = wh - 1;
    }
    scrollok(win, TRUE);
    wsetscrreg(win, s_top, s_bottom);
    scroll(win);
    scrollok(win, FALSE);
    if (wid == MESSAGE_WIN)
    {
        if (border)
            mx = 1;
        else
            mx = 0;
    }
    if (border)
    {
        box(win, 0, 0);
    }
    wrefresh(win);
}


/* Add given line to message history */

static void mesg_add_line(char *mline)
{
    nhprev_mesg *tmp_mesg = NULL;
    nhprev_mesg *current_mesg = malloc(sizeof(nhprev_mesg));

    current_mesg->str = curses_copy_of(mline);
    current_mesg->turn = moves;
    current_mesg->next_mesg = NULL;

    if (num_messages == 0)
    {
        first_mesg = current_mesg;
    }
    
    if (last_mesg != NULL)
    {
        last_mesg->next_mesg = current_mesg;
    }
    current_mesg->prev_mesg = last_mesg;
    last_mesg = current_mesg;


    if (num_messages < max_messages)
    {
        num_messages++;
    }
    else
    {
        tmp_mesg = first_mesg->next_mesg;
        free(first_mesg);
        first_mesg = tmp_mesg;
    }
}


/* Returns specified line from message history, or NULL if out of bounds */

static nhprev_mesg *get_msg_line(boolean reverse, int mindex)
{
    int count;
    char *line;
    nhprev_mesg *current_mesg;

    if (reverse)
    {
        current_mesg = last_mesg;
        for (count = 0; count < mindex; count++)
        {
            if (current_mesg == NULL)
            {
                return NULL;
            }
            current_mesg = current_mesg->prev_mesg;
        }
        return current_mesg;
    }
    else
    {
        current_mesg = first_mesg;
        for (count = 0; count < mindex; count++)
        {
            if (current_mesg == NULL)
            {
                return NULL;
            }
            current_mesg = current_mesg->next_mesg;
        }
        return current_mesg;
    }
}

