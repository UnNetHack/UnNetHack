#ifndef CURSMESG_H
#define CURSMESG_H


/* Global declarations */

void curses_message_win_puts(const char *message, boolean recursed);

void curses_more(void);

void curses_clear_unhighlight_message_window(void);

void curses_last_messages(void);

void curses_init_mesg_history(void);

void curses_prev_mesg(void);


/* Private declatations */

typedef struct nhpm
{
    char *str;  /* Message text */
    long turn;  /* Turn number for message */
    struct nhpm *prev_mesg;    /* Pointer to previous message */
    struct nhpm *next_mesg;    /* Pointer to next message */
} nhprev_mesg;

static void scroll_window(winid wid);

static void mesg_add_line(char *mline);

static nhprev_mesg *get_msg_line(boolean reverse, int mindex);


#endif  /* CURSMESG_H */
