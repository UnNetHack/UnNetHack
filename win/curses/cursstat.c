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

static attr_t get_trouble_color(const char *);
static void statusproblem(int, const char *);
static void print_statdiff(const char *append, nhstat *, int, int);
static void get_playerrank(char *);
static attr_t curses_color_attr(int nh_color, int bg_color);
static int hpen_color(boolean, int, int);
static void draw_bar(boolean, int, int, const char *);
static void draw_horizontal(void);
static void draw_vertical(void);
static int decrement_highlight(nhstat *, boolean);
static void decrement_highlights(boolean);
static void init_stats(void);

#ifdef STATUS_COLORS
static attr_t hpen_color_attr(boolean, int, int);

extern struct color_option text_color_of(const char *text,
                                         const struct text_color_option *color_options);
struct color_option percentage_color_of(int value, int max,
                                        const struct percent_color_option *color_options);

extern const struct text_color_option *text_colors;
extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
#endif

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

/* If the statuscolors patch isn't enabled, have some default colors for status problems
   anyway */

struct statcolor {
    const char *id; /* Identifier for statuscolors */
    const char *txt; /* For status problems */
    int color; /* Default color assuming STATUS_COLORS isn't enabled */
};

static const struct statcolor default_colors[] = {
    {"satiated", "Satiated", CLR_YELLOW},
    {"hungry", "Hungry", CLR_YELLOW},
    {"weak", "Weak", CLR_ORANGE},
    {"fainted", "Fainted", CLR_BRIGHT_MAGENTA},
    {"fainting", "Fainting", CLR_BRIGHT_MAGENTA},
    {"burdened", "Burdened", CLR_RED},
    {"stressed", "Stressed", CLR_RED},
    {"strained", "Strained", CLR_ORANGE},
    {"overtaxed", "Overtaxed", CLR_ORANGE},
    {"overloaded", "Overloaded", CLR_BRIGHT_MAGENTA},
    {"conf", "Conf", CLR_BRIGHT_BLUE},
    {"blind", "Blind", CLR_BRIGHT_BLUE},
    {"stun", "Stun", CLR_BRIGHT_BLUE},
    {"hallu", "Hallu", CLR_BRIGHT_BLUE},
    {"ill", "Ill", CLR_BRIGHT_MAGENTA},
    {"foodpois", "FoodPois", CLR_BRIGHT_MAGENTA},
    {"slime", "Slime", CLR_BRIGHT_MAGENTA},
    {NULL, NULL, NO_COLOR},
};

static attr_t
get_trouble_color(const char *stat)
{
    attr_t res = curses_color_attr(CLR_GRAY, 0);
    const struct statcolor *clr;
    for (clr = default_colors; clr->txt; clr++) {
        if (stat && !strcmp(clr->txt, stat)) {
#ifdef STATUS_COLORS
            /* Check if we have a color enabled with statuscolors */
            if (!iflags.use_status_colors)
                return CLR_GRAY; /* no color configured */

            struct text_color_option *cur_option;
            for (cur_option = text_colors; cur_option; cur_option = cur_option->next) {
                if (strcmpi(cur_option->text, clr->id) == 0) {
                    /* We found the option, now make a curses attribute out of it */
                    struct color_option status_color;
                    status_color = text_color_of(clr->id, cur_option);

                    int count;
                    for (count = 0; (1 << count) <= stat_color.attr_bits; count++) {
                        if (count != ATR_NONE && (status_color.attr_bits & (1 << count)))
                            attr |= curses_convert_attr(count);
                    }

                    return attr;
                }
            }

            /* No color configured, use gray */
            return curses_color_attr(CLR_GRAY, 0);
#else
            return curses_color_attr(clr->color, 0);
#endif
        }
    }

    return res;
}

/* TODO: This is in the wrong place. */
void
get_playerrank(char *rank)
{
    char buf[BUFSZ];
    if (Upolyd) {
        int k = 0;

        Strcpy(buf, mons[u.umonnum].mname);
        while(buf[k] != 0) {
            if ((k == 0 || (k > 0 && buf[k-1] == ' ')) &&
                'a' <= buf[k] && buf[k] <= 'z')
                buf[k] += 'A' - 'a';
            k++;
        }
        Strcpy(rank, buf);
    } else
        Strcpy(rank, rank_of(u.ulevel, Role_switch, flags.female));
}

/* Handles numerical stat changes of various kinds.
   type is generally STAT_OTHER (generic "do nothing special"),
   but is used if the stat needs to be handled in a special way. */

static void
print_statdiff(const char *append, nhstat *stat, int new, int type)
{
    char buf[BUFSZ];
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    int color = CLR_GRAY;

    /* Turncount isn't highlighted, or it would be highlighted constantly. */
    if (type != STAT_TIME && new != stat->value) {
        /* Less AC is better */
        if ((type == STAT_AC && new < stat->value) ||
            (type != STAT_AC && new > stat->value)) {
            color = STAT_UP_COLOR;
            if (type == STAT_GOLD)
                color = HI_GOLD;
        } else
            color = STAT_DOWN_COLOR;

        stat->value = new;
        stat->highlight_color = color;
        stat->highlight_turns = 5;
    } else if (stat->highlight_turns)
        color = stat->highlight_color;

    attr_t attr = curses_color_attr(color, 0);
    wattron(win, attr);
    wprintw(win, "%s", append);
    if (type == STAT_STR && new > 18) {
        if (new > 118)
            wprintw(win, "%d", new - 100);
        else if (new == 118)
            wprintw(win, "18/**");
        else
            wprintw(win, "18/%d", new - 18);
    } else
        wprintw(win, "%d", new);

    wattroff(win, attr);
}

static void
statusproblem(int trouble, const char *str)
{
    if (!trouble)
        return;

    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    /* For whatever reason, hunger states have trailing spaces. Get rid of them. */
    char buf[BUFSZ];
    Strcpy(buf, str);
    int i;
    for (i = 0; (buf[i] != ' ' && buf[i] != '\0'); i++) ;

    buf[i] = '\0';
    attr_t attr = get_trouble_color(buf);
    wattron(win, attr);
    wprintw(win, "%s", buf);
    wattroff(win, attr);
}

/* Returns a ncurses attribute for foreground and background.
   This should probably be in cursinit.c or something. */
static attr_t
curses_color_attr(int nh_color, int bg_color)
{
    int color = nh_color + 1;
    attr_t cattr = A_NORMAL;

    if (!nh_color) {
#ifdef USE_DARKGRAY
        if (iflags.wc2_darkgray) {
            if (!can_change_color() || COLORS <= 16)
                cattr |= A_BOLD;
        } else
#endif
            color = COLOR_BLUE;
    }

    if (COLORS < 16 && color > 8) {
        color -= 8;
        cattr = A_BOLD;
    }

    /* Can we do background colors? We can if we have more than
       16*7 colors (more than 8*7 for terminals with bold) */
    if (COLOR_PAIRS > (COLORS >= 16 ? 16 : 8) * 7) {
        /* NH3 has a rather overcomplicated way of defining
           its colors past the first 16:
           Pair    Foreground  Background
           17      Black       Red
           18      Black       Blue
           19      Red         Red
           20      Red         Blue
           21      Green       Red
           ...
           (Foreground order: Black, Red, Green, Yellow, Blue,
           Magenta, Cyan, Gray/White)

           To work around these oddities, we define backgrounds
           by the following pairs:

           16 COLORS
           49-64: Green
           65-80: Yellow
           81-96: Magenta
           97-112: Cyan
           113-128: Gray/White

           8 COLORS
           9-16: Green
           33-40: Yellow
           41-48: Magenta
           49-56: Cyan
           57-64: Gray/White */

        if (bg_color == nh_color)
            color = 1; /* Make foreground black if fg==bg */

        if (bg_color == CLR_RED || bg_color == CLR_BLUE) {
            /* already defined before extension */
            color *= 2;
            color += 16;
            if (bg_color == CLR_RED)
                color--;
        } else {
            boolean hicolor = FALSE;
            if (COLORS >= 16)
                hicolor = TRUE;

            switch (bg_color) {
            case CLR_GREEN:
                color = (hicolor ? 48 : 8) + color;
                break;
            case CLR_BROWN:
                color = (hicolor ? 64 : 32) + color;
                break;
            case CLR_MAGENTA:
                color = (hicolor ? 80 : 40) + color;
                break;
            case CLR_CYAN:
                color = (hicolor ? 96 : 48) + color;
                break;
            case CLR_GRAY:
                color = (hicolor ? 112 : 56) + color;
                break;
            default:
                break;
            }
        }
    }
    cattr |= COLOR_PAIR(color);

    return cattr;
}

#ifdef STATUS_COLORS
static attr_t
hpen_color_attr(boolean is_hp, int cur, int max)
{
    if (!iflags.use_status_colors)
        return curses_color_attr(CLR_GRAY);

    struct color_option status_color;
    status_color = percentage_color_of(cur, max, is_hp ? hp_colors : pw_colors);

    int count;
    for (count = 0; (1 << count) <= stat_color.attr_bits; count++) {
        if (count != ATR_NONE && (status_color.attr_bits & (1 << count)))
            attr |= curses_convert_attr(count);
    }

    return attr;
}
#endif

static int
hpen_color(boolean is_hp, int cur, int max)
{
    int color = CLR_GRAY;
    if (cur == max)
        color = CLR_GRAY;
    else if (cur * 3 > max * 2) /* >2/3 */
        color = is_hp ? CLR_GREEN : CLR_CYAN;
    else if (cur * 3 > max) /* >1/3 */
        color = is_hp ? CLR_YELLOW : CLR_BLUE;
    else if (cur * 7 > max) /* >1/7 */
        color = is_hp ? CLR_RED : CLR_MAGENTA;
    else
        color = is_hp ? CLR_ORANGE : CLR_BRIGHT_MAGENTA;

    return color;
}

/* Draws a bar
   is_hp: TRUE if we're drawing HP, Pw otherwise (determines colors)
   cur/max: Current/max HP/Pw
   title: Not NULL if we are drawing as part of an existing title.
   Otherwise, the format is as follows: [   11 / 11   ] */
static void
draw_bar(boolean is_hp, int cur, int max, const char *title)
{
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    char buf[BUFSZ];
    if (title)
        Strcpy(buf, title);
    else {
        int len = 5;
        sprintf(buf, "%*d / %-*d", len, cur, len, max);
    }

    /* Colors */
    attr_t fillattr, attr;
#ifdef STATUS_COLORS
    attr = hpen_color_attr(is_hp, cur, max);
    attr &= ~A_REVERSE;
    fillattr = (attr | A_REVERSE);
#else
    int color = hpen_color(is_hp, cur, max);
    int invcolor = color & 7;

    fillattr = curses_color_attr(color, invcolor);
    attr = curses_color_attr(color, 0);
#endif

    /* Figure out how much of the bar to fill */
    int fill = 0;
    int len = strlen(buf);
    if (cur > 0 && max > 0)
        fill = len * cur / max;
    if (fill > len)
        fill = len;

    waddch(win, '[');
    wattron(win, fillattr);
    wprintw(win, "%.*s", fill, buf);
    wattroff(win, fillattr);
    wattron(win, attr);
    wprintw(win, "%.*s", len - fill, &buf[fill]);
    wattroff(win, attr);
    waddch(win, ']');
}

/* Update the status win - this is called when NetHack would normally
write to the status window, so we know somwthing has changed.  We
override the write and update what needs to be updated ourselves. */
static void
draw_horizontal(void)
{
    char buf[BUFSZ];
    char rank[BUFSZ];
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    /* The area we're allowed to print on. Excludes borders */
    int x, y, h, w;

    /* Starting x/y */
    x = 0;
    y = 0;

    /* Starting height/width */
    curses_get_window_size(STATUS_WIN, &h, &w);

    boolean border = FALSE;
    if (curses_window_has_border(STATUS_WIN)) {
        x++;
        y++;
        h--;
        w--;
        border = TRUE;
    }

    int hp = u.uhp;
    int hpmax = u.uhpmax;
    if (Upolyd) {
        hp = u.mh;
        hpmax = u.mhmax;
    }

    /* Line 1 */
    wmove(win, y, x);

    get_playerrank(rank);
    sprintf(buf, "%s the %s", plname, rank);

    /* Use the title as HP bar (similar to hitpointbar) */
    draw_bar(TRUE, hp, hpmax, buf);

    /* Attributes */
    print_statdiff(" St:", &prevstr, ACURR(A_STR), STAT_STR);
    print_statdiff(" Dx:", &prevdex, ACURR(A_DEX), STAT_OTHER);
    print_statdiff(" Co:", &prevcon, ACURR(A_CON), STAT_OTHER);
    print_statdiff(" In:", &prevint, ACURR(A_INT), STAT_OTHER);
    print_statdiff(" Wi:", &prevwis, ACURR(A_WIS), STAT_OTHER);
    print_statdiff(" Ch:", &prevcha, ACURR(A_CHA), STAT_OTHER);

    wprintw(win, (u.ualign.type == A_CHAOTIC ? " Chaotic" :
                  u.ualign.type == A_NEUTRAL ? " Neutral" : " Lawful"));

#ifdef SCORE_ON_BOTL
    if (flags.showscore)
        print_statdiff(" S:", &prevscore, botl_score(), STAT_OTHER);
#endif /* SCORE_ON_BOTL */

    wclrtoeol(win);


    /* Line 2 */
    y++;
    wmove(win, y, x);

    describe_level(buf);

    wprintw(win, "%s", buf);

#ifndef GOLDOBJ
    print_statdiff("$", &prevau, u.ugold, STAT_GOLD);
#else
    print_statdiff("$", &prevau, money_cnt(invent), STAT_GOLD);
#endif

    /* HP/Pw use special coloring rules */
    int hpcolor, pwcolor;
    attr_t attr;
    hpcolor = hpen_color(TRUE, hp, hpmax);
    pwcolor = hpen_color(FALSE, u.uen, u.uenmax);

    wprintw(win, " HP:");
    attr = curses_color_attr(hpcolor, 0);
    wattron(win, attr);
    wprintw(win, "%d(%d)", hp, hpmax);
    wattroff(win, attr);

    wprintw(win, " Pw:");
    attr = curses_color_attr(pwcolor, 0);
    wattron(win, attr);
    wprintw(win, "%d(%d)", u.uen, u.uenmax);
    wattroff(win, attr);

    print_statdiff(" AC:", &prevac, u.uac, STAT_AC);

    if (Upolyd)
        print_statdiff(" HD:", &prevlevel, mons[u.umonnum].mlevel, STAT_OTHER);
#ifdef EXP_ON_BOTL
    else if (flags.showexp) {
        print_statdiff(" Xp:", &prevlevel, u.ulevel, STAT_OTHER);
        /* use waddch, we don't want to highlight the '/' */
        waddch(win, '/');
        print_statdiff("", &prevexp, u.uexp, STAT_OTHER);
#endif
    } else
        print_statdiff(" Exp:", &prevlevel, u.ulevel, STAT_OTHER);

    if (flags.time)
        print_statdiff(" T:", &prevtime, moves, STAT_TIME);
#define statprob(stat, str)                   \
    if (stat) {                               \
        waddch(win, ' ');                     \
        statusproblem(stat, str);             \
    }

    /* Hunger */
    statprob(u.uhs != 1, hu_stat[u.uhs]);

    /* General troubles */
    statprob(Confusion, "Conf");
    statprob(Blind, "Blind");
    statprob(Stunned, "Stun");
    statprob(Hallucination, "Hallu");
    statprob((u.usick_type & SICK_VOMITABLE), "FoodPois");
    statprob((u.usick_type & (SICK_NONVOMITABLE|SICK_ZOMBIE)), "Ill");
    statprob(Slimed, "Slime");

    /* Encumbrance */
    int enc = near_capacity();
    statprob(enc > UNENCUMBERED, enc_stat[enc]);
#undef statprob

    wclrtoeol(win);
}


/* Personally I never understood the point of a vertical status bar. But removing the
   option would be silly, so keep the functionality. */
static void
draw_vertical(void)
{
    char buf[BUFSZ];
    char rank[BUFSZ];
    WINDOW *win = curses_get_nhwin(STATUS_WIN);

    /* The area we're allowed to print on. Excludes borders */
    int x, y, h, w;

    /* Starting x/y */
    x = 0;
    y = 0;

    /* Starting height/width */
    curses_get_window_size(STATUS_WIN, &h, &w);

    boolean border = FALSE;
    if (curses_window_has_border(STATUS_WIN)) {
        x++;
        y++;
        h--;
        w--;
        border = TRUE;
    }

    int hp = u.uhp;
    int hpmax = u.uhpmax;
    if (Upolyd) {
        hp = u.mh;
        hpmax = u.mhmax;
    }

    /* Clear the window */
    werase(win);

    /* Print title and dungeon branch */
    wmove(win, y++, x);

    get_playerrank(rank);
    int ranklen = strlen(rank);
    int namelen = strlen(plname);
    if ((ranklen + namelen) > 19) {
        /* The result doesn't fit. Strip name if >10 characters, then strip title */
        if (namelen > 10) {
            while (namelen > 10 && (ranklen + namelen) > 19)
                namelen--;
        }

        while ((ranklen + namelen) > 19)
            ranklen--; /* Still doesn't fit, strip rank */
    }
    sprintf(buf, "%-*s the %-*s", namelen, plname, ranklen, rank);
    draw_bar(TRUE, hp, hpmax, buf);
    wmove(win, y++, x);
    wprintw(win, "%s", dungeons[u.uz.dnum].dname);

    y++; /* Blank line inbetween */
    wmove(win, y++, x);

    /* Attributes. Old  vertical order is preserved */
    print_statdiff("Strength:      ", &prevstr, ACURR(A_STR), STAT_STR);
    wmove(win, y++, x);
    print_statdiff("Intelligence:  ", &prevint, ACURR(A_INT), STAT_OTHER);
    wmove(win, y++, x);
    print_statdiff("Wisdom:        ", &prevwis, ACURR(A_WIS), STAT_OTHER);
    wmove(win, y++, x);
    print_statdiff("Dexterity:     ", &prevdex, ACURR(A_DEX), STAT_OTHER);
    wmove(win, y++, x);
    print_statdiff("Constitution:  ", &prevcon, ACURR(A_CON), STAT_OTHER);
    wmove(win, y++, x);
    print_statdiff("Charisma:      ", &prevcha, ACURR(A_CHA), STAT_OTHER);
    wmove(win, y++, x);
    wprintw(win,   "Alignment:     ");
    wprintw(win, (u.ualign.type == A_CHAOTIC ? "Chaotic" :
                  u.ualign.type == A_NEUTRAL ? "Neutral" : "Lawful"));
    wmove(win, y++, x);
    wprintw(win,   "Dungeon Level: ");

    /* Astral Plane doesn't fit */
    if (In_endgame(&u.uz))
        wprintw(win, "%s", Is_astralevel(&u.uz) ? "Astral" : "End Game");
    else
        wprintw(win, "%d", depth(&u.uz));
    wmove(win, y++, x);

#ifndef GOLDOBJ
    print_statdiff("Gold:          ", &prevau, u.ugold, STAT_GOLD);
#else
    print_statdiff("Gold:          ", &prevau, money_cnt(invent), STAT_GOLD);
#endif
    wmove(win, y++, x);

    /* HP/Pw use special coloring rules */
    int hpcolor, pwcolor;
    attr_t attr;
    hpcolor = hpen_color(TRUE, hp, hpmax);
    pwcolor = hpen_color(FALSE, u.uen, u.uenmax);

    wprintw(win,   "Hit Points:    ");
    attr = curses_color_attr(hpcolor, 0);
    wattron(win, attr);
    wprintw(win, "%d/%d", hp, hpmax);
    wattroff(win, attr);
    wmove(win, y++, x);

    wprintw(win,   "Magic Power:   ");
    attr = curses_color_attr(pwcolor, 0);
    wattron(win, attr);
    wprintw(win, "%d/%d", u.uen, u.uenmax);
    wattroff(win, attr);
    wmove(win, y++, x);

    print_statdiff("Armor Class:   ", &prevac, u.uac, STAT_AC);
    wmove(win, y++, x);

    if (Upolyd)
        print_statdiff("Hit Dice:      ", &prevlevel, mons[u.umonnum].mlevel, STAT_OTHER);
#ifdef EXP_ON_BOTL
    else if (flags.showexp) {
        print_statdiff("Experience:    ", &prevlevel, u.ulevel, STAT_OTHER);
        /* use waddch, we don't want to highlight the '/' */
        waddch(win, '/');
        print_statdiff("", &prevexp, u.uexp, STAT_OTHER);
#endif
    } else
        print_statdiff("Level:         ", &prevlevel, u.ulevel, STAT_OTHER);
    wmove(win, y++, x);

    if (flags.time) {
        print_statdiff("Time:          ", &prevtime, moves, STAT_TIME);
        wmove(win, y++, x);
    }

#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        print_statdiff("Score:         ", &prevscore, botl_score(), STAT_OTHER);
        wmove(win, y++, x);
    }
#endif /* SCORE_ON_BOTL */

    /* Troubles. Uses a macro to avoid major repetition */

#define statprob(stat, str)                   \
    if (stat) {                               \
        statusproblem(stat, str);             \
        wmove(win, y++, x);                   \
    }

    /* Hunger */
    statprob(u.uhs != 1, hu_stat[u.uhs]);

    /* General troubles */
    statprob(Confusion, "Conf");
    statprob(Blind, "Blind");
    statprob(Stunned, "Stun");
    statprob(Hallucination, "Hallu");
    statprob((u.usick_type & SICK_VOMITABLE), "FoodPois");
    statprob((u.usick_type & (SICK_NONVOMITABLE|SICK_ZOMBIE)), "Ill");
    statprob(Slimed, "Slime");

    /* Encumbrance */
    int enc = near_capacity();
    statprob(enc > UNENCUMBERED, enc_stat[enc]);
#undef statprob
}

void
curses_update_stats(void)
{
    char buf[BUFSZ];
    int count, enc, sx_start, hp, hpmax, labels, swidth, sheight, sx_end, sy_end,
        orient;
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
    }

    orient = curses_get_window_orientation(STATUS_WIN);

    horiz = FALSE;
    if ((orient != ALIGN_RIGHT) && (orient != ALIGN_LEFT))
        horiz = TRUE;

    if (orient != ALIGN_RIGHT && orient != ALIGN_LEFT)
        draw_horizontal();
    else
        draw_vertical();

    if (border)
        box(win, 0, 0);

    wrefresh(win);
}

/* Decrement a single highlight, return 1 if decremented to zero. zero is TRUE if we're
   zeroing the highlight. */
static int
decrement_highlight(nhstat *stat, boolean zero)
{
    if (stat->highlight_turns > 0) {
        if (zero) {
            stat->highlight_turns = 0;
            return 1;
        }

        stat->highlight_turns--;
        if (stat->highlight_turns == 0)
            return 1;
    }
    return 0;
}

/* Decrement the highlight_turns for all stats.  Call curses_update_stats
if needed to unhighlight a stat */

static void
decrement_highlights(boolean zero)
{
    int unhighlight = 0;

    unhighlight |= decrement_highlight(&prevdepth, zero);
    unhighlight |= decrement_highlight(&prevstr, zero);
    unhighlight |= decrement_highlight(&prevdex, zero);
    unhighlight |= decrement_highlight(&prevcon, zero);
    unhighlight |= decrement_highlight(&prevint, zero);
    unhighlight |= decrement_highlight(&prevwis, zero);
    unhighlight |= decrement_highlight(&prevcha, zero);
    unhighlight |= decrement_highlight(&prevau, zero);
    unhighlight |= decrement_highlight(&prevhp, zero);
    unhighlight |= decrement_highlight(&prevmhp, zero);
    unhighlight |= decrement_highlight(&prevlevel, zero);
    unhighlight |= decrement_highlight(&prevpow, zero);
    unhighlight |= decrement_highlight(&prevmpow, zero);
    unhighlight |= decrement_highlight(&prevac, zero);
#ifdef EXP_ON_BOTL
    unhighlight |= decrement_highlight(&prevexp, zero);
#endif
    unhighlight |= decrement_highlight(&prevtime, zero);
#ifdef SCORE_ON_BOTL
    unhighlight |= decrement_highlight(&prevscore, zero);
#endif

    if (unhighlight)
        curses_update_stats();
}


void
curses_decrement_highlight()
{
    decrement_highlights(FALSE);
}


/* Initializes the prev(whatever) values */
static void
init_stats()
{
    prevdepth.value = depth(&u.uz);
    prevstr.value = ACURR(A_STR);
    prevdex.value = ACURR(A_DEX);
    prevcon.value = ACURR(A_CON);
    prevint.value = ACURR(A_INT);
    prevwis.value = ACURR(A_WIS);
    prevcha.value = ACURR(A_CHA);
#ifndef GOLDOBJ
    prevau.value = u.ugold;
#else
    prevau.value = money_cnt(invent);
#endif
    int hp = u.uhp;
    int hpmax = u.uhpmax;
    if (Upolyd) {
        hp = u.mh;
        hpmax = u.mhmax;
    }
    prevhp.value = hp;
    prevmhp.value = hpmax;
    prevlevel.value = (Upolyd ? mons[u.umonnum].mlevel : u.ulevel);
    prevpow.value = u.uen;
    prevmpow.value = u.uenmax;
    prevac.value = u.uac;
#ifdef EXP_ON_BOTL
    prevexp.value = u.uexp;
#endif
    prevtime.value = moves;
#ifdef SCORE_ON_BOTL
    prevscore.value = botl_score();
#endif
}
