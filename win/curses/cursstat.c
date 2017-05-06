/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursstat.h"

/* Status window functions for curses interface */

/* Private declarations */

typedef struct nhs {
    long value;
    char *txt;
    aligntyp alignment;
    boolean display;
    int highlight_turns;
    int highlight_color;
    int stat_color;
    int stat_attr;
    int x;
    int y;
    char *label;
    const char *id;
} nhstat;

#ifdef STATUS_COLORS
extern const struct text_color_option *text_colors;
extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;

extern struct color_option text_color_of(const char *text,
                                         const struct text_color_option
                                         *color_options);
struct color_option percentage_color_of(int value, int max,
                                        const struct percent_color_option
                                        *color_options);
static boolean stat_colored(const char *id);
#endif

static void handle_status_problem(nhstat *, int, const char *, int *, int *,
                                  int, boolean);
static void handle_stat_change(nhstat *, int, int, int *, int *, int, boolean);
static int decrement_highlight(nhstat *);
static void init_stats(void);
static void set_labels(int label_width);
static void set_stat_color(nhstat *stat);
static void color_stat(nhstat stat, int onoff);

static nhstat prevname;
static nhstat prevdepth;
static nhstat prevstr;
static nhstat prevint;
static nhstat prevwis;
static nhstat prevdex;
static nhstat prevcon;
static nhstat prevcha;
static nhstat prevalign;
static nhstat prevau;
static nhstat prevhp;
static nhstat prevmhp;
static nhstat prevlevel;
static nhstat prevpow;
static nhstat prevmpow;
static nhstat prevac;
static nhstat prevexp;
static nhstat prevtime;

#ifdef SCORE_ON_BOTL
static nhstat prevscore;
#endif
static nhstat prevhunger;
static nhstat prevconf;
static nhstat prevblind;
static nhstat prevstun;
static nhstat prevhallu;
static nhstat prevsick;
static nhstat prevslime;
static nhstat prevencumb;

#define COMPACT_LABELS  1
#define NORMAL_LABELS   2
#define WIDE_LABELS     3

extern const char *hu_stat[];   /* from eat.c */
extern const char *enc_stat[];  /* from botl.c */

/* Handles numerical stat changes of various kinds.
   type is generally STAT_OTHER (generic "do nothing special"),
   but is used if the stat needs to be handled in a special way. */

static void
handle_stat_change(nhstat *stat, int new, int type,
                   int *sx, int *sy, int sx_start, boolean horiz)
{
    char buf[BUFSZ];
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    /* Turncount isn't highlighted, or it would be highlighted constantly.
       Also note that these colors can be ignored if statuscolors is enabled
       in color_stat() */
    if (new != stat->value && type != STAT_TIME) {
        /* Less AC is better */
        if ((type == STAT_AC && new < stat->value) ||
            (type != STAT_AC && new > stat->value)) {
            if (type == STAT_GOLD) {
                stat->highlight_color = HI_GOLD;
            } else {
                stat->highlight_color = STAT_UP_COLOR;
            }
        } else {
            stat->highlight_color = STAT_DOWN_COLOR;
        }
        stat->value = new;

        /* Strength might be displayed differently */
        if (type == STAT_STR && new > 18) {
            if (new > 118) {
                sprintf(buf, "%d", new - 100);
            } else if (new == 118) {
                sprintf(buf, "%d/**");
            } else {
                sprintf(buf, "18/%02d", new - 18);
            }
        } else {
            sprintf(buf, "%d", new);
        }

        free(stat->txt);
        stat->txt = curses_copy_of(buf);
        stat->highlight_turns = 5;
        if (type == STAT_HPEN) {
            stat->highlight_turns = 3;
        }
    }

    if (stat->label) {
        mvwaddstr(win, *sy, *sx, stat->label);
        *sx += strlen(stat->label);
    }

    color_stat(*stat, ON);
    mvwaddstr(win, *sy, *sx, stat->txt);
    color_stat(*stat, OFF);

    if (type == STAT_HPEN) {
        *sx += strlen(stat->txt);
    } else if (horiz) {
        *sx += strlen(stat->txt) + 1;
    } else {
        *sx = sx_start;
        *sy += 1;
    }
}

static void
handle_status_problem(nhstat *stat, int new, const char *str,
                      int *sx, int *sy, int sx_start, boolean horiz)
{
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    if (new != stat->value) {
        stat->highlight_color = STAT_DOWN_COLOR;
        if (stat->txt != NULL) {
            free(stat->txt);
        }
        if (new) {
            stat->txt = curses_copy_of(str);
        } else {
            stat->txt = NULL;
        }
        if (stat->value == 0) {
            stat->highlight_turns = 5;
        }
        stat->value = new;
    }

    if (stat->label != NULL) {
        mvwaddstr(win, *sy, *sx, stat->label);
        *sx += strlen(stat->label);
    }

    if (stat->txt != NULL) {
        color_stat(*stat, ON);
        mvwaddstr(win, *sy, *sx, stat->txt);
        color_stat(*stat, OFF);

        if (horiz) {
            *sx += strlen(stat->txt) + 1;
        } else {
            sx = sx_start;
            *sy += 1;           /* ++ would increase the pointer addr */
        }
    }
}

/* Update the status win - this is called when NetHack would normally
write to the status window, so we know somwthing has changed.  We
override the write and update what needs to be updated ourselves. */

void
curses_update_stats(boolean redraw)
{
    char buf[BUFSZ];
    int count, enc, orient, sx_start, hp, hpmax, labels, swidth,
        sheight, sx_end, sy_end;
    WINDOW *win = curses_get_nhwin(STATUS_WIN);
    static int prev_labels = -1;
    static boolean first = TRUE;
    static boolean horiz;
    int sx = 0;
    int sy = 0;
    boolean border = curses_window_has_border(STATUS_WIN);

    curses_get_window_size(STATUS_WIN, &sheight, &swidth);

    if (border) {
        sx++;
        sy++;
        swidth--;
        sheight--;
    }

    sx_end = swidth - 1;
    sy_end = sheight - 1;
    sx_start = sx;

    if (first) {
        init_stats();
        first = FALSE;
        redraw = TRUE;
    }

    if (redraw) {
        orient = curses_get_window_orientation(STATUS_WIN);

        if ((orient == ALIGN_RIGHT) || (orient == ALIGN_LEFT)) {
            horiz = FALSE;
        } else {
            horiz = TRUE;
        }
    }

    if (horiz) {
        if (term_cols >= 80) {
            labels = NORMAL_LABELS;
        } else {
            labels = COMPACT_LABELS;
        }
    } else {
        labels = WIDE_LABELS;
    }

    if (labels != prev_labels) {
        set_labels(labels);
        prev_labels = labels;
    }

    curses_clear_nhwin(STATUS_WIN);

    /* Line 1 */

    /* Improve when this code is workable, this is a bit awkward at the moment */

#define statchange(stat,new,type) handle_stat_change(stat, new, type, &sx, &sy, \
                                                     sx_start, horiz)

    /* Player name and title */
    strcpy(buf, plname);
    if ('a' <= buf[0] && buf[0] <= 'z')
        buf[0] += 'A' - 'a';
    strcat(buf, " the ");
    if (u.mtimedone) {
        char mname[BUFSZ];
        int k = 0;

        strcpy(mname, mons[u.umonnum].mname);
        while (mname[k] != 0) {
            if ((k == 0 || (k > 0 && mname[k - 1] == ' '))
                && 'a' <= mname[k] && mname[k] <= 'z') {
                mname[k] += 'A' - 'a';
            }
            k++;
        }
        strcat(buf, mname);
    } else {
        strcat(buf, rank_of(u.ulevel, pl_character[0], flags.female));
    }

    if (strcmp(buf, prevname.txt) != 0) {       /* Title changed */
        prevname.highlight_turns = 5;
        prevname.highlight_color = HIGHLIGHT_COLOR;
        free(prevname.txt);
        prevname.txt = curses_copy_of(buf);
        if ((labels == COMPACT_LABELS) && (u.ulevel > 1)) {
            curses_puts(MESSAGE_WIN, A_NORMAL, "You are now known as");
            curses_puts(MESSAGE_WIN, A_NORMAL, prevname.txt);
        }
    }

    if (prevname.label != NULL) {
        mvwaddstr(win, sy, sx, prevname.label);
        sx += strlen(prevname.label);
    }

    if (labels != COMPACT_LABELS) {
        color_stat(prevname, ON);
        mvwaddstr(win, sy, sx, prevname.txt);
        color_stat(prevname, OFF);
    }

    if (horiz) {
        if (labels != COMPACT_LABELS) {
            sx += strlen(prevname.txt) + 1;
        }


    } else {
        sx = sx_start;
        sy++;
    }

    /* Add dungeon name and level if status window is vertical */
    if (!horiz) {
        sprintf(buf, "%s", dungeons[u.uz.dnum].dname);
        mvwaddstr(win, sy, sx, buf);
        sy += 2;
    }

    /* Strength */
    statchange(&prevstr, ACURR(A_STR), STAT_STR);
    statchange(&prevint, ACURR(A_INT), STAT_OTHER);
    statchange(&prevwis, ACURR(A_WIS), STAT_OTHER);
    statchange(&prevdex, ACURR(A_DEX), STAT_OTHER);
    statchange(&prevcon, ACURR(A_CON), STAT_OTHER);
    statchange(&prevcha, ACURR(A_CHA), STAT_OTHER);

    /* Alignment */
    if (prevalign.alignment != u.ualign.type) { /* Alignment changed */
        prevalign.highlight_color = HIGHLIGHT_COLOR;
        prevalign.highlight_turns = 10; /* This is a major change! */
        prevalign.alignment = u.ualign.type;
        free(prevalign.txt);
        switch (u.ualign.type) {
        case A_LAWFUL:
            prevalign.txt = curses_copy_of("Lawful");
            break;
        case A_NEUTRAL:
            prevalign.txt = curses_copy_of("Neutral");
            break;
        case A_CHAOTIC:
            prevalign.txt = curses_copy_of("Chaotic");
            break;
        }
    }

    if (prevalign.label != NULL) {
        mvwaddstr(win, sy, sx, prevalign.label);
        sx += strlen(prevalign.label);
    }

    color_stat(prevalign, ON);
    mvwaddstr(win, sy, sx, prevalign.txt);
    color_stat(prevalign, OFF);

    /* Line 2 */

    sx = sx_start;
    sy++;

    /* Dungeon Level */
    if (depth(&u.uz) != prevdepth.value) {      /* Dungeon level changed */
        prevdepth.highlight_color = HIGHLIGHT_COLOR;
        prevdepth.highlight_turns = 5;
        prevdepth.value = depth(&u.uz);
        free(prevdepth.txt);
        if (In_endgame(&u.uz)) {
            strcpy(buf, (Is_astralevel(&u.uz) ? "Astral Plane" : "End Game"));
        } else {
            sprintf(buf, "%d", depth(&u.uz));
        }
        prevdepth.txt = curses_copy_of(buf);
    }

    if (prevdepth.label != NULL) {
        mvwaddstr(win, sy, sx, prevdepth.label);
        sx += strlen(prevdepth.label);
    }

    color_stat(prevdepth, ON);
    mvwaddstr(win, sy, sx, prevdepth.txt);
    color_stat(prevdepth, OFF);

    if (horiz) {
        sx += strlen(prevdepth.txt) + 1;
    } else {
        sx = sx_start;
        sy++;
    }

    /* Gold */
#ifndef GOLDOBJ
    statchange(&prevau, u.ugold, STAT_GOLD);
#else
    statchange(&prevau, money_cnt(invent), STAT_GOLD);
#endif

    /* Hit Points */

    if (u.mtimedone) {          /* Currently polymorphed - show monster HP */
        hp = u.mh;
        hpmax = u.mhmax;
    } else {                    /* Not polymorphed */

        hp = u.uhp;
        hpmax = u.uhpmax;
    }

    statchange(&prevhp, hp, STAT_HPEN);
    statchange(&prevmhp, hpmax, STAT_OTHER);
    statchange(&prevpow, u.uen, STAT_HPEN);
    statchange(&prevmpow, u.uenmax, STAT_OTHER);
    statchange(&prevac, u.uac, STAT_AC);

    /* Experience */
#ifdef EXP_ON_BOTL
    if (prevexp.display != flags.showexp) {     /* Setting has changed */
        prevexp.display = flags.showexp;
        free(prevlevel.label);
        if (prevexp.display) {
            prevlevel.label = curses_copy_of("/");
        } else {
            if (horiz) {
                if (labels == COMPACT_LABELS) {
                    prevlevel.label = curses_copy_of("Lv:");
                } else {
                    prevlevel.label = curses_copy_of("Lvl:");
                }
            } else {
                prevlevel.label = curses_copy_of("Level:         ");
            }
        }
    }

    if (prevexp.display && !u.mtimedone) {
        if (u.uexp != prevexp.value) {
            if (u.uexp > prevexp.value) {
                prevexp.highlight_color = STAT_UP_COLOR;
            } else {
                prevexp.highlight_color = STAT_DOWN_COLOR;
            }
            sprintf(buf, "%ld", u.uexp);
            free(prevexp.txt);
            prevexp.txt = curses_copy_of(buf);
            prevexp.highlight_turns = 3;
        }

        if (prevexp.label != NULL) {
            mvwaddstr(win, sy, sx, prevexp.label);
            sx += strlen(prevexp.label);
        }

        color_stat(prevexp, ON);
        mvwaddstr(win, sy, sx, prevexp.txt);
        color_stat(prevexp, OFF);

        sx += strlen(prevexp.txt);
    }

    prevexp.value = u.uexp;     /* Track it even when it's not displayed */
#endif /* EXP_ON_BOTL */

    /* Level */
    if (u.mtimedone) {          /* Currently polymorphed - show monster HD */
        if ((strncmp(prevlevel.label, "HP:", 3) != 0) ||
            (strncmp(prevlevel.label, "Hit Points:", 11) != 0)) {
            free(prevlevel.label);
            if (horiz) {
                prevlevel.label = curses_copy_of("HD:");
            } else {
                prevlevel.label = curses_copy_of("Hit Dice:      ");
            }
        }
        if (mons[u.umonnum].mlevel != prevlevel.value) {
            if (mons[u.umonnum].mlevel > prevlevel.value) {
                prevlevel.highlight_color = STAT_UP_COLOR;
            } else {
                prevlevel.highlight_color = STAT_DOWN_COLOR;
            }
            prevlevel.highlight_turns = 5;
        }
        prevlevel.value = mons[u.umonnum].mlevel;
        sprintf(buf, "%d", mons[u.umonnum].mlevel);
        free(prevlevel.txt);
        prevlevel.txt = curses_copy_of(buf);
    } else {                    /* Not polymorphed */

        if ((strncmp(prevlevel.label, "HD:", 3) != 0) ||
            (strncmp(prevlevel.label, "Hit Dice:", 9) != 0)) {
            free(prevlevel.label);
            if (prevexp.display) {
                prevlevel.label = curses_copy_of("/");
            } else {
                if (horiz) {
                    if (labels == COMPACT_LABELS) {
                        prevlevel.label = curses_copy_of("Lv:");
                    } else {
                        prevlevel.label = curses_copy_of("Lvl:");
                    }
                } else {
                    prevlevel.label = curses_copy_of("Level:         ");
                }
            }
        }
        if (u.ulevel > prevlevel.value) {
            prevlevel.highlight_color = STAT_UP_COLOR;
            prevlevel.highlight_turns = 5;
        } else if (u.ulevel < prevlevel.value) {
            prevlevel.highlight_color = STAT_DOWN_COLOR;
            prevlevel.highlight_turns = 5;
        }
        prevlevel.value = u.ulevel;
        sprintf(buf, "%d", u.ulevel);
        free(prevlevel.txt);
        prevlevel.txt = curses_copy_of(buf);
    }

    if (prevlevel.label != NULL) {
        mvwaddstr(win, sy, sx, prevlevel.label);
        sx += strlen(prevlevel.label);
    }

    color_stat(prevlevel, ON);
    mvwaddstr(win, sy, sx, prevlevel.txt);
    color_stat(prevlevel, OFF);

    if (horiz) {
        sx += strlen(prevlevel.txt) + 1;
    } else {
        sx = sx_start;
        sy++;
    }

    /* Time */
    if (prevtime.display != flags.time) {       /* Setting has changed */
        prevtime.display = flags.time;
    }
    if (prevtime.display) {
        statchange(&prevtime, moves, STAT_TIME);
    }

    /* Score */
#ifdef SCORE_ON_BOTL
    if (prevscore.display != flags.showscore) { /* Setting has changed */
        prevscore.display = flags.showscore;
    }
    if (prevscore.display) {
        statchange(&prevscore, botl_score(), STAT_OTHER);
    }

    prevscore.value = botl_score();     /* Track it even when it's not displayed */
#endif /* SCORE_ON_BOTL */

    /* Hunger */
    if (u.uhs != prevhunger.value) {
        if ((u.uhs > prevhunger.value) || (u.uhs > 3)) {
            prevhunger.highlight_color = STAT_DOWN_COLOR;
        } else {
            prevhunger.highlight_color = STAT_UP_COLOR;
        }
        prevhunger.value = u.uhs;
        for (count = 0; count < strlen(hu_stat[u.uhs]); count++) {
            if ((hu_stat[u.uhs][count]) == ' ') {
                break;
            }
            buf[count] = hu_stat[u.uhs][count];
        }

        buf[count] = '\0';
        free(prevhunger.txt);
        prevhunger.txt = curses_copy_of(buf);
        prevhunger.highlight_turns = 5;
    }

    if (prevhunger.label != NULL) {
        mvwaddstr(win, sy, sx, prevhunger.label);
        sx += strlen(prevhunger.label);
    }

    color_stat(prevhunger, ON);
    mvwaddstr(win, sy, sx, prevhunger.txt);
    color_stat(prevhunger, OFF);

    if (strlen(prevhunger.txt) > 0) {
        if (horiz) {
            sx += strlen(prevhunger.txt) + 1;
        } else {
            sx = sx_start;
            sy++;
        }
    }
#define statusproblem(stat,new,str) handle_status_problem(stat, new, str, &sx, &sy, \
                                                          sx_start, horiz)

    /* Confusion */
    statusproblem(&prevconf, Confusion, "Conf");
    statusproblem(&prevblind, Blind, "Blind");
    statusproblem(&prevstun, Stunned, "Stun");
    statusproblem(&prevhallu, Hallucination, "Hallu");

    /* TODO: allow all 3 kinds of sickness seperately: FoodPois, Ill, Zombie */
    statusproblem(&prevsick, Sick,
                  (u.usick_type & SICK_VOMITABLE) ? "FoodPois" : "Ill");
    statusproblem(&prevslime, Slimed, "Slime");

    /* Encumberance */
    enc = near_capacity();

    if (enc != prevencumb.value) {
        if (enc < prevencumb.value) {
            prevencumb.highlight_color = STAT_UP_COLOR;
        } else {
            prevencumb.highlight_color = STAT_DOWN_COLOR;
        }
        if (prevencumb.txt != NULL) {
            free(prevencumb.txt);
        }
        if (enc > UNENCUMBERED) {
            sprintf(buf, "%s", enc_stat[enc]);
            prevencumb.txt = curses_copy_of(buf);
            prevencumb.highlight_turns = 5;
        } else {
            prevencumb.txt = NULL;
        }
        prevencumb.value = enc;
    }

    if (prevencumb.label != NULL) {
        mvwaddstr(win, sy, sx, prevencumb.label);
        sx += strlen(prevencumb.label);
    }

    if (prevencumb.txt != NULL) {
        color_stat(prevencumb, ON);
        mvwaddstr(win, sy, sx, prevencumb.txt);
        color_stat(prevencumb, OFF);
    }

    if (prevencumb.txt != NULL) {
        if (horiz) {
            sx += strlen(prevencumb.txt) + 1;
        } else {
            sx = sx_start;
            sy++;
        }
    }

    wrefresh(win);
}

/* Decrement a single highlight, return 1 if decremented to zero */

static int
decrement_highlight(nhstat *stat)
{
    if (stat->highlight_turns > 0) {
        stat->highlight_turns--;
        if (stat->highlight_turns == 0) {
            return 1;
        }
    }
    return 0;
}

/* Decrement the highlight_turns for all stats.  Call curses_update_stats
if needed to unhighlight a stat */

void
curses_decrement_highlight()
{
    int unhighlight = 0;

    unhighlight |= decrement_highlight(&prevname);
    unhighlight |= decrement_highlight(&prevdepth);
    unhighlight |= decrement_highlight(&prevstr);
    unhighlight |= decrement_highlight(&prevint);
    unhighlight |= decrement_highlight(&prevwis);
    unhighlight |= decrement_highlight(&prevdex);
    unhighlight |= decrement_highlight(&prevcon);
    unhighlight |= decrement_highlight(&prevcha);
    unhighlight |= decrement_highlight(&prevalign);
    unhighlight |= decrement_highlight(&prevau);
    unhighlight |= decrement_highlight(&prevhp);
    unhighlight |= decrement_highlight(&prevmhp);
    unhighlight |= decrement_highlight(&prevlevel);
    unhighlight |= decrement_highlight(&prevpow);
    unhighlight |= decrement_highlight(&prevmpow);
    unhighlight |= decrement_highlight(&prevac);
#ifdef EXP_ON_BOTL
    unhighlight |= decrement_highlight(&prevexp);
#endif
    unhighlight |= decrement_highlight(&prevtime);
#ifdef SCORE_ON_BOTL
    unhighlight |= decrement_highlight(&prevscore);
#endif
    unhighlight |= decrement_highlight(&prevhunger);
    unhighlight |= decrement_highlight(&prevconf);
    unhighlight |= decrement_highlight(&prevblind);
    unhighlight |= decrement_highlight(&prevstun);
    unhighlight |= decrement_highlight(&prevhallu);
    unhighlight |= decrement_highlight(&prevsick);
    unhighlight |= decrement_highlight(&prevslime);
    unhighlight |= decrement_highlight(&prevencumb);

    if (unhighlight) {
        curses_update_stats(FALSE);
    }
}


/* Initialize the stats with beginning values. */

static void
init_stats()
{
    char buf[BUFSZ];
    int count;

    /* Player name and title */
    strcpy(buf, plname);
    if ('a' <= buf[0] && buf[0] <= 'z')
        buf[0] += 'A' - 'a';
    strcat(buf, " the ");
    if (u.mtimedone) {
        char mname[BUFSZ];
        int k = 0;

        strcpy(mname, mons[u.umonnum].mname);
        while (mname[k] != 0) {
            if ((k == 0 || (k > 0 && mname[k - 1] == ' '))
                && 'a' <= mname[k] && mname[k] <= 'z') {
                mname[k] += 'A' - 'a';
            }
            k++;
        }
        strcat(buf, mname);
    } else {
        strcat(buf, rank_of(u.ulevel, pl_character[0], flags.female));
    }

    prevname.txt = curses_copy_of(buf);
    prevname.display = TRUE;
    prevname.highlight_turns = 0;
    prevname.label = NULL;
    prevname.id = "name";
    set_stat_color(&prevname);

    /* Strength */
    if (ACURR(A_STR) > 118) {
        sprintf(buf, "%d", ACURR(A_STR) - 100);
    } else if (ACURR(A_STR) == 118) {
        sprintf(buf, "18/**");
    } else if (ACURR(A_STR) > 18) {
        sprintf(buf, "18/%02d", ACURR(A_STR) - 18);
    } else {
        sprintf(buf, "%d", ACURR(A_STR));
    }

    prevstr.value = ACURR(A_STR);
    prevstr.txt = curses_copy_of(buf);
    prevstr.display = TRUE;
    prevstr.highlight_turns = 0;
    prevstr.label = NULL;
    prevstr.id = "str";
    set_stat_color(&prevstr);

    /* Intelligence */
    sprintf(buf, "%d", ACURR(A_INT));
    prevint.value = ACURR(A_INT);
    prevint.txt = curses_copy_of(buf);
    prevint.display = TRUE;
    prevint.highlight_turns = 0;
    prevint.label = NULL;
    prevint.id = "int";
    set_stat_color(&prevint);

    /* Wisdom */
    sprintf(buf, "%d", ACURR(A_WIS));
    prevwis.value = ACURR(A_WIS);
    prevwis.txt = curses_copy_of(buf);
    prevwis.display = TRUE;
    prevwis.highlight_turns = 0;
    prevwis.label = NULL;
    prevwis.id = "wis";
    set_stat_color(&prevwis);

    /* Dexterity */
    sprintf(buf, "%d", ACURR(A_DEX));
    prevdex.value = ACURR(A_DEX);
    prevdex.txt = curses_copy_of(buf);
    prevdex.display = TRUE;
    prevdex.highlight_turns = 0;
    prevdex.label = NULL;
    prevdex.id = "dex";
    set_stat_color(&prevdex);

    /* Constitution */
    sprintf(buf, "%d", ACURR(A_CON));
    prevcon.value = ACURR(A_CON);
    prevcon.txt = curses_copy_of(buf);
    prevcon.display = TRUE;
    prevcon.highlight_turns = 0;
    prevcon.label = NULL;
    prevcon.id = "con";
    set_stat_color(&prevcon);

    /* Charisma */
    sprintf(buf, "%d", ACURR(A_CHA));
    prevcha.value = ACURR(A_CHA);
    prevcha.txt = curses_copy_of(buf);
    prevcha.display = TRUE;
    prevcha.highlight_turns = 0;
    prevcha.label = NULL;
    prevcha.id = "cha";
    set_stat_color(&prevcha);

    /* Alignment */
    switch (u.ualign.type) {
    case A_LAWFUL:
        prevalign.txt = curses_copy_of("Lawful");
        break;
    case A_NEUTRAL:
        prevalign.txt = curses_copy_of("Neutral");
        break;
    case A_CHAOTIC:
        prevalign.txt = curses_copy_of("Chaotic");
        break;
    }

    prevalign.alignment = u.ualign.type;
    prevalign.display = TRUE;
    prevalign.highlight_turns = 0;
    prevalign.label = NULL;
    prevalign.id = "align";
    set_stat_color(&prevalign);

    /* Dungeon level */
    if (In_endgame(&u.uz)) {
        strcpy(buf, (Is_astralevel(&u.uz) ? "Astral Plane" : "End Game"));
    } else {
        sprintf(buf, "%d", depth(&u.uz));
    }

    prevdepth.value = depth(&u.uz);
    prevdepth.txt = curses_copy_of(buf);
    prevdepth.display = TRUE;
    prevdepth.highlight_turns = 0;
    prevdepth.label = NULL;
    prevdepth.id = "dlvl";
    set_stat_color(&prevdepth);

    /* Gold */
#ifndef GOLDOBJ
    sprintf(buf, "%ld", u.ugold);
    prevau.value = u.ugold;
#else
    sprintf(buf, "%ld", money_cnt(invent));
    prevau.value = money_cnt(invent);
#endif
    prevau.txt = curses_copy_of(buf);
    prevau.display = TRUE;
    prevau.highlight_turns = 0;
    prevau.label = NULL;
    prevau.id = "gold";
    set_stat_color(&prevau);

    /* Hit Points */
    if (u.mtimedone) {          /* Currently polymorphed - show monster HP */
        prevhp.value = u.mh;
        sprintf(buf, "%d", u.mh);
        prevhp.txt = curses_copy_of(buf);
    } else if (u.uhp != prevhp.value) { /* Not polymorphed */
        prevhp.value = u.uhp;
        sprintf(buf, "%d", u.uhp);
        prevhp.txt = curses_copy_of(buf);
    }
    prevhp.display = TRUE;
    prevhp.highlight_turns = 0;
    prevhp.label = NULL;
    prevhp.id = "hp";
    set_stat_color(&prevhp);

    /* Max Hit Points */
    if (u.mtimedone) {          /* Currently polymorphed - show monster HP */
        prevmhp.value = u.mhmax;
        sprintf(buf, "%d", u.mhmax);
        prevmhp.txt = curses_copy_of(buf);
    } else {                    /* Not polymorphed */

        prevmhp.value = u.uhpmax;
        sprintf(buf, "%d", u.uhpmax);
        prevmhp.txt = curses_copy_of(buf);
    }
    prevmhp.display = TRUE;
    prevmhp.highlight_turns = 0;
    prevmhp.label = curses_copy_of("/");
    prevmhp.id = "mhp";
    set_stat_color(&prevmhp);

    /* Power */
    prevpow.value = u.uen;
    sprintf(buf, "%d", u.uen);
    prevpow.txt = curses_copy_of(buf);
    prevpow.display = TRUE;
    prevpow.highlight_turns = 0;
    prevpow.label = NULL;
    prevpow.id = "pw";
    set_stat_color(&prevpow);

    /* Max Power */
    prevmpow.value = u.uenmax;
    sprintf(buf, "%d", u.uenmax);
    prevmpow.txt = curses_copy_of(buf);
    prevmpow.display = TRUE;
    prevmpow.highlight_turns = 0;
    prevmpow.label = curses_copy_of("/");
    prevmpow.id = "mpw";
    set_stat_color(&prevmpow);

    /* Armor Class */
    prevac.value = u.uac;
    sprintf(buf, "%d", u.uac);
    prevac.txt = curses_copy_of(buf);
    prevac.display = TRUE;
    prevac.highlight_turns = 0;
    prevac.label = NULL;
    prevac.id = "ac";
    set_stat_color(&prevac);

    /* Experience */
#ifdef EXP_ON_BOTL
    prevexp.value = u.uexp;
    sprintf(buf, "%ld", u.uexp);
    prevexp.txt = curses_copy_of(buf);
    prevexp.display = flags.showexp;
    prevexp.highlight_turns = 0;
    prevexp.label = NULL;
    prevexp.id = "xp";
    set_stat_color(&prevexp);
#endif

    /* Level */
    prevlevel.label = NULL;
    if (u.mtimedone) {          /* Currently polymorphed - show monster HP */
        prevlevel.value = mons[u.umonnum].mlevel;
        sprintf(buf, "%d", mons[u.umonnum].mlevel);
        prevlevel.txt = curses_copy_of(buf);
    } else if (u.ulevel != prevlevel.value) {   /* Not polymorphed */
        prevlevel.value = u.ulevel;
        sprintf(buf, "%d", u.ulevel);
        prevlevel.txt = curses_copy_of(buf);
    }
    prevlevel.display = TRUE;
    prevlevel.highlight_turns = 0;
    prevlevel.id = "lvl";
    set_stat_color(&prevlevel);

    /* Time */
    prevtime.value = moves;
    sprintf(buf, "%ld", moves);
    prevtime.txt = curses_copy_of(buf);
    prevtime.display = flags.time;
    prevtime.highlight_turns = 0;
    prevtime.label = NULL;
    prevtime.id = "time";
    set_stat_color(&prevtime);

    /* Score */
#ifdef SCORE_ON_BOTL
    prevscore.value = botl_score();
    sprintf(buf, "%ld", botl_score());
    prevscore.txt = curses_copy_of(buf);
    prevscore.display = flags.showscore;
    prevscore.highlight_turns = 0;
    prevscore.label = NULL;
    prevscore.id = "score";
    set_stat_color(&prevscore);
#endif

    /* Hunger */
    prevhunger.value = u.uhs;
    for (count = 0; count < strlen(hu_stat[u.uhs]); count++) {
        if ((hu_stat[u.uhs][count]) == ' ') {
            break;
        }
        buf[count] = hu_stat[u.uhs][count];
    }

    buf[count] = '\0';
    prevhunger.txt = curses_copy_of(buf);
    prevhunger.display = TRUE;
    prevhunger.highlight_turns = 0;
    prevhunger.label = NULL;
    prevhunger.id = "hunger";
    set_stat_color(&prevhunger);

    /* Confusion */
    prevconf.value = Confusion;
    if (Confusion) {
        prevconf.txt = curses_copy_of("Conf");
    } else {
        prevconf.txt = NULL;
    }
    prevconf.display = TRUE;
    prevconf.highlight_turns = 0;
    prevconf.label = NULL;
    prevconf.id = "conf";
    set_stat_color(&prevconf);

    /* Blindness */
    prevblind.value = Blind;
    if (Blind) {
        prevblind.txt = curses_copy_of("Blind");
    } else {
        prevblind.txt = NULL;
    }
    prevblind.display = TRUE;
    prevblind.highlight_turns = 0;
    prevblind.label = NULL;
    prevblind.id = "blind";
    set_stat_color(&prevblind);

    /* Stun */
    prevstun.value = Stunned;
    if (Stunned) {
        prevstun.txt = curses_copy_of("Stun");
    } else {
        prevstun.txt = NULL;
    }
    prevstun.display = TRUE;
    prevstun.highlight_turns = 0;
    prevstun.label = NULL;
    prevstun.id = "stun";
    set_stat_color(&prevstun);

    /* Hallucination */
    prevhallu.value = Hallucination;
    if (Hallucination) {
        prevhallu.txt = curses_copy_of("Hallu");
    } else {
        prevhallu.txt = NULL;
    }
    prevhallu.display = TRUE;
    prevhallu.highlight_turns = 0;
    prevhallu.label = NULL;
    prevhallu.id = "hallu";
    set_stat_color(&prevhallu);

    /* Sick */
    prevsick.value = Sick;
    if (Sick) {
        if (u.usick_type & SICK_VOMITABLE) {
            prevsick.txt = curses_copy_of("FoodPois");
        } else {
            prevsick.txt = curses_copy_of("Ill");
        }
    } else {
        prevsick.txt = NULL;
    }
    prevsick.display = TRUE;
    prevsick.highlight_turns = 0;
    prevsick.label = NULL;
    prevsick.id = "sick";
    set_stat_color(&prevsick);

    /* Slimed */
    prevslime.value = Slimed;
    if (Slimed) {
        prevslime.txt = curses_copy_of("Slime");
    } else {
        prevslime.txt = NULL;
    }
    prevslime.display = TRUE;
    prevslime.highlight_turns = 0;
    prevslime.label = NULL;
    prevslime.id = "slime";
    set_stat_color(&prevslime);

    /* Encumberance */
    prevencumb.value = near_capacity();
    if (prevencumb.value > UNENCUMBERED) {
        sprintf(buf, "%s", enc_stat[prevencumb.value]);
        prevencumb.txt = curses_copy_of(buf);
    } else {
        prevencumb.txt = NULL;
    }
    prevencumb.display = TRUE;
    prevencumb.highlight_turns = 0;
    prevencumb.label = NULL;
    prevencumb.id = "encumberance";
    set_stat_color(&prevencumb);
}

/* Set labels based on orientation of status window.  If horizontal,
we want to compress this info; otherwise we know we have a width of at
least 26 characters. */

static void
set_labels(int label_width)
{
    char buf[BUFSZ];

    switch (label_width) {
    case COMPACT_LABELS:
        /* Strength */
        if (prevstr.label) {
            free(prevstr.label);
        }
        prevstr.label = curses_copy_of("S:");
        /* Intelligence */
        if (prevint.label) {
            free(prevint.label);
        }
        prevint.label = curses_copy_of("I:");

        /* Wisdom */
        if (prevwis.label) {
            free(prevwis.label);
        }
        prevwis.label = curses_copy_of("W:");

        /* Dexterity */
        if (prevdex.label) {
            free(prevdex.label);
        }
        prevdex.label = curses_copy_of("D:");

        /* Constitution */
        if (prevcon.label) {
            free(prevcon.label);
        }
        prevcon.label = curses_copy_of("C:");

        /* Charisma */
        if (prevcha.label) {
            free(prevcha.label);
        }
        prevcha.label = curses_copy_of("Ch:");

        /* Alignment */
        if (prevalign.label) {
            free(prevalign.label);
        }
        prevalign.label = NULL;

        /* Dungeon level */
        if (prevdepth.label) {
            free(prevdepth.label);
        }
        prevdepth.label = curses_copy_of("Dl:");

        /* Gold */
        if (prevau.label) {
            free(prevau.label);
        }
        sprintf(buf, "%c:", GOLD_SYM);
        prevau.label = curses_copy_of(buf);

        /* Hit points */
        if (prevhp.label) {
            free(prevhp.label);
        }
        prevhp.label = curses_copy_of("HP:");

        /* Power */
        if (prevpow.label) {
            free(prevpow.label);
        }
        prevpow.label = curses_copy_of("Pw:");

        /* Armor Class */
        if (prevac.label) {
            free(prevac.label);
        }
        prevac.label = curses_copy_of("AC:");

#ifdef EXP_ON_BOTL
        /* Experience */
        if (prevexp.label) {
            free(prevexp.label);
        }
        prevexp.label = curses_copy_of("XP:");
#endif

        /* Level */
        if (prevlevel.label) {
            free(prevlevel.label);
            prevlevel.label = NULL;
        }
        if (u.mtimedone) {      /* Currently polymorphed - show monster HP */
            prevlevel.label = curses_copy_of("HD:");
        } else {                /* Not polymorphed */

            if (prevexp.display) {
                prevlevel.label = curses_copy_of("/");
            } else {
                prevlevel.label = curses_copy_of("Lv:");
            }
        }

        /* Time */
        if (prevtime.label) {
            free(prevtime.label);
        }
        prevtime.label = curses_copy_of("T:");

#ifdef SCORE_ON_BOTL
        /* Score */
        if (prevscore.label) {
            free(prevscore.label);
        }
        prevscore.label = curses_copy_of("S:");
#endif
        break;

    case NORMAL_LABELS:
        /* Strength */
        if (prevstr.label) {
            free(prevstr.label);
        }
        prevstr.label = curses_copy_of("Str:");
        /* Intelligence */
        if (prevint.label) {
            free(prevint.label);
        }
        prevint.label = curses_copy_of("Int:");

        /* Wisdom */
        if (prevwis.label) {
            free(prevwis.label);
        }
        prevwis.label = curses_copy_of("Wis:");

        /* Dexterity */
        if (prevdex.label) {
            free(prevdex.label);
        }
        prevdex.label = curses_copy_of("Dex:");

        /* Constitution */
        if (prevcon.label) {
            free(prevcon.label);
        }
        prevcon.label = curses_copy_of("Con:");

        /* Charisma */
        if (prevcha.label) {
            free(prevcha.label);
        }
        prevcha.label = curses_copy_of("Cha:");

        /* Alignment */
        if (prevalign.label) {
            free(prevalign.label);
        }
        prevalign.label = NULL;

        /* Dungeon level */
        if (prevdepth.label) {
            free(prevdepth.label);
        }
        prevdepth.label = curses_copy_of("Dlvl:");

        /* Gold */
        if (prevau.label) {
            free(prevau.label);
        }
        sprintf(buf, "%c:", GOLD_SYM);
        prevau.label = curses_copy_of(buf);

        /* Hit points */
        if (prevhp.label) {
            free(prevhp.label);
        }
        prevhp.label = curses_copy_of("HP:");

        /* Power */
        if (prevpow.label) {
            free(prevpow.label);
        }
        prevpow.label = curses_copy_of("Pw:");

        /* Armor Class */
        if (prevac.label) {
            free(prevac.label);
        }
        prevac.label = curses_copy_of("AC:");

#ifdef EXP_ON_BOTL
        /* Experience */
        if (prevexp.label) {
            free(prevexp.label);
        }
        prevexp.label = curses_copy_of("XP:");
#endif

        /* Level */
        if (prevlevel.label) {
            free(prevlevel.label);
            prevlevel.label = NULL;
        }
        if (u.mtimedone) {      /* Currently polymorphed - show monster HP */
            prevlevel.label = curses_copy_of("HD:");
        } else {                /* Not polymorphed */

            if (prevexp.display) {
                prevlevel.label = curses_copy_of("/");
            } else {
                prevlevel.label = curses_copy_of("Lvl:");
            }
        }

        /* Time */
        if (prevtime.label) {
            free(prevtime.label);
        }
        prevtime.label = curses_copy_of("T:");

#ifdef SCORE_ON_BOTL
        /* Score */
        if (prevscore.label) {
            free(prevscore.label);
        }
        prevscore.label = curses_copy_of("S:");
#endif
        break;

    case WIDE_LABELS:
        /* Strength */
        if (prevstr.label) {
            free(prevstr.label);
        }
        prevstr.label = curses_copy_of("Strength:      ");

        /* Intelligence */
        if (prevint.label) {
            free(prevint.label);
        }
        prevint.label = curses_copy_of("Intelligence:  ");

        /* Wisdom */
        if (prevwis.label) {
            free(prevwis.label);
        }
        prevwis.label = curses_copy_of("Wisdom:        ");

        /* Dexterity */
        if (prevdex.label) {
            free(prevdex.label);
        }
        prevdex.label = curses_copy_of("Dexterity:     ");

        /* Constitution */
        if (prevcon.label) {
            free(prevcon.label);
        }
        prevcon.label = curses_copy_of("Constitution:  ");

        /* Charisma */
        if (prevcha.label) {
            free(prevcha.label);
        }
        prevcha.label = curses_copy_of("Charisma:      ");

        /* Alignment */
        if (prevalign.label) {
            free(prevalign.label);
        }
        prevalign.label = curses_copy_of("Alignment:     ");

        /* Dungeon level */
        if (prevdepth.label) {
            free(prevdepth.label);
        }
        prevdepth.label = curses_copy_of("Dungeon Level: ");

        /* Gold */
        if (prevau.label) {
            free(prevau.label);
        }
        prevau.label = curses_copy_of("Gold:          ");

        /* Hit points */
        if (prevhp.label) {
            free(prevhp.label);
        }
        prevhp.label = curses_copy_of("Hit Points:    ");

        /* Power */
        if (prevpow.label) {
            free(prevpow.label);
        }
        prevpow.label = curses_copy_of("Magic Power:   ");

        /* Armor Class */
        if (prevac.label) {
            free(prevac.label);
        }
        prevac.label = curses_copy_of("Armor Class:   ");

#ifdef EXP_ON_BOTL
        /* Experience */
        if (prevexp.label) {
            free(prevexp.label);
        }
        prevexp.label = curses_copy_of("Experience:    ");
#endif

        /* Level */
        if (prevlevel.label) {
            free(prevlevel.label);
        }
        if (u.mtimedone) {      /* Currently polymorphed - show monster HP */
            prevlevel.label = curses_copy_of("Hit Dice:      ");
        } else {                /* Not polymorphed */

            if (prevexp.display) {
                prevlevel.label = curses_copy_of(" / ");
            } else {
                prevlevel.label = curses_copy_of("Level:         ");
            }
        }

        /* Time */
        if (prevtime.label) {
            free(prevtime.label);
        }
        prevtime.label = curses_copy_of("Time:          ");

#ifdef SCORE_ON_BOTL
        /* Score */
        if (prevscore.label) {
            free(prevscore.label);
        }
        prevscore.label = curses_copy_of("Score:         ");
#endif
        break;

    default:
        panic("set_labels(): Invalid label_width %d\n", label_width);
        break;
    }
}


/* Get the default (non-highlighted) color for a stat.  For now, this
is NO_COLOR unless the statuscolors patch is in use. */

static void
set_stat_color(nhstat *stat)
{
#ifdef STATUS_COLORS
    struct color_option stat_color;
    int count;
    int attr = A_NORMAL;

    if (iflags.use_status_colors && stat_colored(stat->id)) {
        stat_color = text_color_of(stat->id, text_colors);

        for (count = 0; (1 << count) <= stat_color.attr_bits; ++count) {
            if (count != ATR_NONE && stat_color.attr_bits & (1 << count)) {
                attr += curses_convert_attr(count);
            }
        }

        stat->stat_color = stat_color.color;
        stat->stat_attr = attr;
    } else {
        stat->stat_color = NO_COLOR;
        stat->stat_attr = A_NORMAL;
    }
#else
    stat->stat_color = NO_COLOR;
    stat->stat_attr = A_NORMAL;
#endif /* STATUS_COLORS */
}


/* Set the color to the base color for the given stat, or highlight a
 changed stat. */

static void
color_stat(nhstat stat, int onoff)
{
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

#ifdef STATUS_COLORS
    struct color_option stat_color;
    int color, attr, hp, hpmax, count;
    char buf[BUFSIZ];

    stat_color.color = NO_COLOR;
    stat_color.attr_bits = ATR_NONE;

    if (strcmp(stat.id, "hp") == 0) {
        hp = Upolyd ? u.mh : u.uhp;
        hpmax = Upolyd ? u.mhmax : u.uhpmax;
        stat_color = percentage_color_of(hp, hpmax, hp_colors);
    }

    if (strcmp(stat.id, "pw") == 0) {
        stat_color = percentage_color_of(u.uen, u.uenmax, pw_colors);
    }

    if (strcmp(stat.id, "hunger") == 0) {
        for (count = 0; count < strlen(hu_stat[u.uhs]); count++) {
            if ((hu_stat[u.uhs][count]) == ' ') {
                break;
            }
            buf[count] = hu_stat[u.uhs][count];
        }

        buf[count] = '\0';
        stat_color = text_color_of(buf, text_colors);
    }

    if (strcmp(stat.id, "encumberance") == 0) {
        stat_color = text_color_of(enc_stat[prevencumb.value], text_colors);
    }

    if (strcmp(stat.id, "sick") == 0) {
        if (u.usick_type & SICK_VOMITABLE) {
            stat_color = text_color_of("foodpois", text_colors);
        } else {
            stat_color = text_color_of("ill", text_colors);
        }
    }

    if (strcmp(stat.id, "align") == 0) {
        switch (u.ualign.type) {
        case A_LAWFUL:
            stat_color = text_color_of("lawful", text_colors);
            break;
        case A_NEUTRAL:
            stat_color = text_color_of("neutral", text_colors);
            break;
        case A_CHAOTIC:
            stat_color = text_color_of("chaotic", text_colors);
            break;
        }
    }

    color = stat_color.color;
    attr = A_NORMAL;

    for (count = 0; (1 << count) <= stat_color.attr_bits; ++count) {
        if (count != ATR_NONE && stat_color.attr_bits & (1 << count)) {
            attr += curses_convert_attr(count);
        }
    }

    stat.stat_color = color;
    stat.stat_attr = attr;
#endif /* STATUS_COLORS */

    if ((stat.stat_color == NO_COLOR) && (stat.stat_attr == A_NORMAL)) {
        if (stat.highlight_turns > 0) {
#ifdef STATUS_COLORS
            if (iflags.use_status_colors)
#endif
                curses_toggle_color_attr(win, stat.highlight_color,
                                         A_NORMAL, onoff);
        }

        return;
    }
#ifdef STATUS_COLORS
    if (iflags.use_status_colors)
#endif
        curses_toggle_color_attr(win, stat.stat_color, stat.stat_attr, onoff);
}


/* Determine if a stat is configured via statuscolors. */

#ifdef STATUS_COLORS
static boolean
stat_colored(const char *id)
{
    struct text_color_option *cur_option =
        (struct text_color_option *) text_colors;

    while (cur_option != NULL) {
        if (strcmpi(cur_option->text, id) == 0) {
            return TRUE;
        }

        cur_option = (struct text_color_option *) cur_option->next;
    }

    return FALSE;
}
#endif /* STATUS_COLORS */
