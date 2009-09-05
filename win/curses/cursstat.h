#ifndef CURSSTAT_H
#define CURSSTAT_H


/* Global declarations */

void curses_update_stats(void);

void curses_decrement_highlight(void);


/* Private declarations */

typedef struct nhs
{
    long value;
    char *txt;
    aligntyp alignment;
    boolean display;
    int highlight_turns;
    int highlight_color;
    int x;
    int y;
    char *label;
} nhstat;

static void init_stats(void);

#endif  /* CURSSTAT_H */
