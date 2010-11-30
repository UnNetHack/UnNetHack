#ifndef CURSINIT_H
#define CURSINIT_H

/* Global declarations */

void curses_create_main_windows(void);

void curses_init_nhcolors(void);

void curses_choose_character(void);

int curses_character_dialog(const char** choices, const char *prompt);

void curses_init_options(void);

void curses_display_splash_window(void);


#endif  /* CURSINIT_H */
