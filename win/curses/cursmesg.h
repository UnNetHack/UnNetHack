#ifndef CURSMESG_H
#define CURSMESG_H


/* Global declarations */

void curses_message_win_puts(const char *message, boolean recursed);

int curses_more(void);

void curses_clear_unhighlight_message_window(void);

void curses_last_messages(void);

void curses_init_mesg_history(void);

void curses_prev_mesg(void);

void curses_count_window(const char *count_text);

#endif  /* CURSMESG_H */
