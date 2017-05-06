/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef CURSSTAT_H
# define CURSSTAT_H

/* Used by handle_stat_change to handle some stats differently. Not an enum
   because this is how NetHack code generally handles them. */
# define STAT_OTHER  0
# define STAT_STR    1
# define STAT_GOLD   2
# define STAT_HPEN   3          /* HP or Pw */
# define STAT_AC     4
# define STAT_TIME   5

/* Global declarations */

void curses_update_stats(boolean redraw);

void curses_decrement_highlight(void);


#endif /* CURSSTAT_H */
