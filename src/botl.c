/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern const char *const hu_stat[]; /* defined in eat.c */

/* also used in insight.c */
const char *const enc_stat[] = {
    "",
    "Burdened",
    "Stressed",
    "Strained",
    "Overtaxed",
    "Overloaded"
};

static void bot1(void);
static void bot2(void);
static void bot3(void);
static void bot_via_windowport(void);
static void stat_update_time(void);

char *
get_strength_str(void)
{
    static char buf[32];
    int st = ACURR(A_STR);

    if (st > 18) {
        if (st > STR18(100)) {
            Sprintf(buf, "%2d", st - 100);
        } else if (st < STR18(100)) {
            Sprintf(buf, "18/%02d", st - 18);
        } else {
            Sprintf(buf, "18/**");
        }
    } else {
        Sprintf(buf, "%-1d", st);
    }

    return buf;
}

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)

extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
extern const struct text_color_option *text_colors;

struct color_option
text_color_of(const char *text, const struct text_color_option *color_options)
{
    if (color_options == NULL) {
        struct color_option result = {NO_COLOR, 0};
        return result;
    }
    if (strstri(color_options->text, text)
        || strstri(text, color_options->text))
        return color_options->color_option;
    return text_color_of(text, color_options->next);
}

struct color_option
percentage_color_of(int value, int max, const struct percent_color_option *color_options)
{
    if (color_options == NULL) {
        struct color_option result = {NO_COLOR, 0};
        return result;
    }
    switch (color_options->statclrtype) {
    default:
    case STATCLR_TYPE_PERCENT:
        if (100 * value <= color_options->percentage * max) {
            return color_options->color_option;
        }
        break;
    case STATCLR_TYPE_NUMBER_EQ:
        if (value == color_options->percentage) {
            return color_options->color_option;
        }
        break;
    case STATCLR_TYPE_NUMBER_LT:
        if (value < color_options->percentage) {
            return color_options->color_option;
        }
        break;
    case STATCLR_TYPE_NUMBER_GT:
        if (value > color_options->percentage) {
            return color_options->color_option;
        }
        break;
    }
    return percentage_color_of(value, max, color_options->next);
}

void
start_color_option(struct color_option color_option)
{
#ifdef TTY_GRAPHICS
    int i;
    if (color_option.color != NO_COLOR) {
        term_start_color(color_option.color);
    }
    for (i = 0; (1 << i) <= color_option.attr_bits; ++i) {
        if (i != ATR_NONE && color_option.attr_bits & (1 << i)) {
            term_start_attr(i);
        }
    }
#endif  /* TTY_GRAPHICS */
}

void
end_color_option(struct color_option color_option)
{
#ifdef TTY_GRAPHICS
    int i;
    if (color_option.color != NO_COLOR) {
        term_end_color();
    }
    for (i = 0; (1 << i) <= color_option.attr_bits; ++i) {
        if (i != ATR_NONE && color_option.attr_bits & (1 << i)) {
            term_end_attr(i);
        }
    }
#endif  /* TTY_GRAPHICS */
}

static
void
apply_color_option(
    struct color_option color_option,
    const char *newbot2,
    int statusline) /**< apply color on this statusline: 1 or 2 */
{
    if (!iflags.use_status_colors || !iflags.use_color) {
        return;
    }
    curs(WIN_STATUS, 1, statusline-1);
    start_color_option(color_option);
    putstr(WIN_STATUS, 0, newbot2);
    end_color_option(color_option);
}

#ifdef STATUS_HILITES
/* Append a labeled field value to the status line buffer and render it
   with hilite_delta coloring. Uses start_color_option/end_color_option
   which require STATUS_COLORS && TEXTCOLOR (always true when we get here
   since botl_hilite_field lives inside that #if block). */
static void
botl_hilite_field(char *buf, const char *label, const char *value,
                  enum statusfields fld, int statusline)
{
    struct color_option co;

    if (!iflags.hilite_delta || !iflags.use_color) {
        Strcat(buf, label);
        Strcat(buf, value);
        return;
    }

    co = get_hilite_color(fld);
    if (co.color == NO_COLOR && !co.attr_bits) {
        Strcat(buf, label);
        Strcat(buf, value);
        return;
    }

    /* Output everything so far (including label) plain */
    Strcat(buf, label);
    curs(WIN_STATUS, 1, statusline - 1);
    putstr(WIN_STATUS, 0, buf);
    flags.botlx = 0;

    /* Append the value and output with color */
    Strcat(buf, value);
    curs(WIN_STATUS, 1, statusline - 1);
    start_color_option(co);
    putstr(WIN_STATUS, 0, buf);
    end_color_option(co);
}
#endif /* STATUS_HILITES */

void
add_colored_text(const char *text, char *newbot2, int statusline)
{
    char *nb;
    struct color_option color_option;

    if (*text == '\0') {
        return;
    }

    if ((min(MAXCO, CO)-1) < 0) {
        return;
    }
    /* don't add anything if it can't be displayed.
     * Otherwise the color of invisible text may bleed into
     * the statusline. */
    if (strlen(newbot2) >= (unsigned)min(MAXCO, CO)-1) {
        return;
    }

    if (!iflags.use_status_colors) {
        Sprintf(nb = eos(newbot2), " %s", text);
        return;
    }

    Strcat(nb = eos(newbot2), " ");
    curs(WIN_STATUS, 1, statusline);
    putstr(WIN_STATUS, 0, newbot2);

    Strcat(nb = eos(nb), text);
    curs(WIN_STATUS, 1, statusline);
#ifdef STATUS_HILITES
    color_option = get_condition_hilite_color(text);
    if (color_option.color == NO_COLOR && !color_option.attr_bits)
#endif
    color_option = text_color_of(text, text_colors);
    start_color_option(color_option);
    /* Trim the statusline to always have the end color
     * to have effect. */
    newbot2[min(MAXCO, CO)-1] = '\0';
    putstr(WIN_STATUS, 0, newbot2);
    end_color_option(color_option);
}

#endif

static NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */

/* special purpose status update: move counter ('time' status) only */
void
timebot(void)
{
    if (gb.bot_disabled) {
        return;
    }
    /* we're called when disp.time_botl is set and general disp.botl
       is clear; disp.time_botl gets set whenever svm.moves changes value
       so there's no benefit in tracking previous value to decide whether
       to skip update; suppress_map_output() handles program_state.restoring
       and program_state.done_hup (tty hangup => no further output at all)
       and we use it for maybe skipping status as well as for the map */
    if (flags.time && iflags.status_updates && !suppress_map_output()) {
        if (VIA_WINDOWPORT()) {
            stat_update_time();
        } else {
            /* old status display updates everything */
            bot();
        }
    }
    disp.time_botl = FALSE;
}

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(int xlev)
{
    /*
     *   1..2  => 0
     *   3..5  => 1
     *   6..9  => 2
     *  10..13 => 3
     *      ...
     *  26..29 => 7
     *    30   => 8
     * Conversion is precise but only partially reversible.
     */
    return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(int rank)
{
    /*
     *  0 =>  1..2
     *  1 =>  3..5
     *  2 =>  6..9
     *  3 => 10..13
     *      ...
     *  7 => 26..29
     *  8 =>   30
     * We return the low end of each range.
     */
    return (rank < 1) ? 1 : (rank < 2) ? 3
           : (rank < 8) ? ((rank * 4) - 2) : 30;
}

const char *
rank_of(int lev, short int monnum, boolean female)
{
    const struct Role *role;
    int i;

    /* Find the role */
    for (role = roles; role->name.m; role++) {
        if (monnum == role->malenum || monnum == role->femalenum) {
            break;
        }
    }
    if (!role->name.m) {
        role = &urole;
    }

    /* Find the rank */
    for (i = xlev_to_rank((int)lev); i >= 0; i--) {
        if (female && role->rank[i].f) {
            return (role->rank[i].f);
        }
        if (role->rank[i].m) {
            return (role->rank[i].m);
        }
    }

    /* Try the role name, instead */
    if (female && role->name.f) {
        return (role->name.f);
    } else if (role->name.m) {
        return (role->name.m);
    }
    return ("Player");
}

const char *
rank(void)
{
    return rank_of(u.ulevel, Role_switch, flags.female);
}

int
title_to_mon(const char *str, int *rank_indx, int *title_length)
{
    int i, j;

    /* Loop through each of the roles */
    for (i = 0; roles[i].name.m; i++) {
        /* loop through each of the rank titles for role #i */
        for (j = 0; j < 9; j++) {
            if (roles[i].rank[j].m && str_start_is(str, roles[i].rank[j].m, TRUE)) {
                if (rank_indx) {
                    *rank_indx = j;
                }
                if (title_length) {
                    *title_length = Strlen(roles[i].rank[j].m);
                }
                return roles[i].malenum;
            }
            if (roles[i].rank[j].f && str_start_is(str, roles[i].rank[j].f, TRUE)) {
                if (rank_indx) {
                    *rank_indx = j;
                }
                if (title_length) {
                    *title_length = Strlen(roles[i].rank[j].f);
                }
                return ((roles[i].femalenum != NON_PM) ?
                        roles[i].femalenum : roles[i].malenum);
            }
        }
    }

    if (title_length) {
        *title_length = 0;
    }

    return NON_PM;
}

void
max_rank_sz(void)
{
    int i;
    size_t r, maxr = 0;

    for (i = 0; i < 9; i++) {
        if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) {
            maxr = r;
        }
        if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) {
            maxr = r;
        }
    }
    mrank_sz = (int) maxr;
    return;
}

#ifdef SCORE_ON_BOTL
long
botl_score(void)
{
    long deepest = (long) deepest_lev_reached(FALSE);
    long umoney = money_cnt(invent) + hidden_gold();

    if ((umoney -= u.umoney0) < 0L) {
        umoney = 0L;
    }
    return umoney + u.urscore + (long)(50 * (deepest - 1))
           + (long)(deepest > 30 ? 10000 :
                    deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#ifdef DUMP_LOG
void bot1str(char *newbot1)
#else
static void
bot1()
#endif
{
#ifndef DUMP_LOG
    char newbot1[MAXCO];
#endif
    char *nb;
    int i=0, j;
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    int save_botlx = flags.botlx;
#endif

    if (suppress_map_output()) {
        return;
    }

    Strcpy(newbot1, "");
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    if (flags.hitpointbar) {
        flags.botlx = 0;
        curs(WIN_STATUS, 1, 0);
        putstr(WIN_STATUS, 0, newbot1);
        Strcat(newbot1, "[");
        i = 1; /* don't overwrite the string in front */
        curs(WIN_STATUS, 1, 0);
        putstr(WIN_STATUS, 0, newbot1);
    }
#endif
    Strcat(newbot1, plname);
    if ('a' <= newbot1[i] && newbot1[i] <= 'z') {
        newbot1[i] += 'A'-'a';
    }
    newbot1[10] = '\0';
    Sprintf(nb = eos(newbot1), " the ");

    if (Upolyd) {
        char mbot[BUFSZ];
        int k = 0;

        Strcpy(mbot, mons[u.umonnum].mname);
        while (mbot[k] != 0) {
            if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
                'a' <= mbot[k] && mbot[k] <= 'z')
                mbot[k] += 'A' - 'a';
            k++;
        }
        Sprintf(nb = eos(nb), "%s", mbot);
    } else {
        Sprintf(nb = eos(nb), "%s", rank());
    }

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    if (flags.hitpointbar) {
        int bar_length = strlen(newbot1);
        char tmp[MAXCO];
        /* filledbar >= 0 and < MAXCO */
        int hp = (uhp() < 0) ? 0 : uhp();
        int filledbar = (uhpmax() > 0) ? (hp * bar_length / uhpmax()) : 0;
        if (filledbar > MAXCO) {
            filledbar = MAXCO;
        }
        Strcpy(tmp, newbot1);

        /* draw hp bar */
        if (iflags.use_inverse) {
            term_start_attr(ATR_INVERSE);
        }
        tmp[filledbar] = '\0';
        if (iflags.use_color) {
            /* draw in color mode */
            struct color_option bar_co;
#ifdef STATUS_HILITES
            bar_co = get_hilite_color(BL_HP);
            if (bar_co.color == NO_COLOR && !bar_co.attr_bits)
#endif
            bar_co = percentage_color_of(uhp(), uhpmax(), hp_colors);
            apply_color_option(bar_co, tmp, 1);
        } else {
            /* draw in inverse mode */
            curs(WIN_STATUS, 1, 0);
            putstr(WIN_STATUS, 0, tmp);
        }
        term_end_color();
        if (iflags.use_inverse) {
            term_end_attr(ATR_INVERSE);
        }

        Strcat(newbot1, "]");
    }
#endif

    Sprintf(nb = eos(nb), "  ");
    i = mrank_sz + 15;
    j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
    if ((i - j) > 0) {
        Sprintf(nb = eos(nb), "%*s", i-j, " "); /* pad with spaces */
    }
#ifdef STATUS_HILITES
    if (iflags.hilite_delta) {
        char vbuf[BUFSZ];
        botl_hilite_field(newbot1, "St:", get_strength_str(), BL_STR, 1);
        Sprintf(vbuf, "%-1d", ACURR(A_DEX));
        botl_hilite_field(newbot1, " Dx:", vbuf, BL_DX, 1);
        Sprintf(vbuf, "%-1d", ACURR(A_CON));
        botl_hilite_field(newbot1, " Co:", vbuf, BL_CO, 1);
        Sprintf(vbuf, "%-1d", ACURR(A_INT));
        botl_hilite_field(newbot1, " In:", vbuf, BL_IN, 1);
        Sprintf(vbuf, "%-1d", ACURR(A_WIS));
        botl_hilite_field(newbot1, " Wi:", vbuf, BL_WI, 1);
        Sprintf(vbuf, "%-1d", ACURR(A_CHA));
        botl_hilite_field(newbot1, " Ch:", vbuf, BL_CH, 1);
    } else
#endif
    {
        Sprintf(nb = eos(newbot1),
                "St:%s Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
                get_strength_str(),
                ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
    }
    Sprintf(eos(newbot1), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
            (u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        char vbuf[BUFSZ];
        Sprintf(vbuf, "%ld", botl_score());
#ifdef STATUS_HILITES
        botl_hilite_field(newbot1, " S:", vbuf, BL_SCORE, 1);
#else
        Sprintf(eos(newbot1), " S:%s", vbuf);
#endif
    }
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    flags.botlx = save_botlx;
#endif
#ifdef DUMP_LOG
}
static void
bot1(void)
{
    char newbot1[MAXCO];

    int save_botlx = flags.botlx;
    bot1str(newbot1);
#endif
    curs(WIN_STATUS, 1, 0);
    /* bot1str has already updated cw->data to match newbot1, so force
       botlx=0 to make this trailing putstr a no-op (when nothing differs)
       or diff-only (when something does). */
    flags.botlx = 0;
    putstr(WIN_STATUS, 0, newbot1);
    flags.botlx = save_botlx;
}

/* provide the name of the current level for display by various ports */
int
describe_level(
    char *buf, /* output buffer */
    int dflgs) /* 1: append trailing space; 2: include dungeon branch name */
{
    boolean addspace = (dflgs & 1) != 0,  /* (used to be unconditional) */
            addbranch = (dflgs & 2) != 0; /* False: status, True: livelog */
    int ret = 1;

    if (Is_knox(&u.uz)) {
        Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
        addbranch = FALSE;
    } else if (In_quest(&u.uz)) {
        Sprintf(buf, "Home %d ", dunlev(&u.uz));
    } else if (Is_blackmarket(&u.uz)) {
        Sprintf(buf, "Blackmarket ");
        addbranch = FALSE;
    } else if (Is_town_level(&u.uz)) {
        Sprintf(buf, "Town ");
        addbranch = FALSE;
    } else if (Is_minetown_level(&u.uz)) {
        Sprintf(buf, "Mine Town:%-2d ", depth(&u.uz));
    } else if (In_endgame(&u.uz)) {
        /* [3.6.2: this used to be "Astral Plane" or generic "End Game"] */
        (void) endgame_level_name(buf, depth(&u.uz));
        if (!addbranch) {
            (void) strsubst(buf, "Plane of ", ""); /* just keep <element> */
        }
        addbranch = FALSE;
    } else {
        /* ports with more room may expand this one */
        if (!addbranch) {
            char *dgn_name = dungeons[u.uz.dnum].dname;
            Sprintf(buf, "%s:%-2d", /* "Dlvl:n" (grep fodder) */
                    iflags.show_dgn_name ? dgn_name : "Dlvl",
                    depth(&u.uz));
        } else {
            Sprintf(buf, "level %d", depth(&u.uz));
        }
        ret = 0;
    }
    if (addbranch) {
        Sprintf(eos(buf), ", %s", dungeons[u.uz.dnum].dname);
        (void) strsubst(buf, "The ", "the ");
    }
    if (addspace) {
        Strcat(buf, " ");
    }
    return ret;
}

static void
botl_text(int condition, const char *text, char *botl, int statusline)
{
    if (condition) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
        add_colored_text(text, botl, statusline);
#else
        Sprintf(botl = eos(botl), " %s", text);
#endif
    }
}

static void
botl_text_or_blanks(int condition, const char *text, char *botl, int statusline)
{
    if (condition) {
        botl_text(condition, text, botl, statusline);
    } else {
        char *nb = eos(botl);
        int i = strlen(text) + 1;
        while (i-- > 0) { strcat(nb, " "); }
    }
}

const char*
botl_realtime(void)
{
    time_t currenttime;

    if (iflags.showrealtime == REALTIME_PLAYTIME) {
        currenttime = get_realtime();
    } else {
        /* REALTIME_WALLTIME implied */
        currenttime = current_epoch() - u.ubirthday;
    }

    static char buf[BUFSZ] = { 0 };
    switch (iflags.realtime_format) {
        case REALTIME_FORMAT_SECONDS:
            Sprintf(buf, "%ld", currenttime);
            break;

        case REALTIME_FORMAT_CONDENSED:
            Sprintf(buf, "%ld:%2.2ld", currenttime / 3600, (currenttime % 3600) / 60);
            break;

        case REALTIME_FORMAT_UNITS:
        default: {
            char *duration = format_duration(currenttime);
            /* only show 2 time units */
            *(strchr(duration, ':')+4) = '\0';
            Sprintf(buf, "%s", duration);
        }
    }
    return buf;
}

#ifdef DUMP_LOG
void bot2str(char *newbot2)

#else

#endif
{
#ifndef DUMP_LOG
    char newbot2[MAXCO];
#endif
    char *nb;
    int hp, hpmax;
    int cap = near_capacity();
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    int save_botlx = flags.botlx;
#endif

    hp = Upolyd ? u.mh : u.uhp;
    hpmax = Upolyd ? u.mhmax : u.uhpmax;

    if (hp < 0) {
        hp = 0;
    }
    (void) describe_level(newbot2, 1);
#ifdef STATUS_HILITES
    if (iflags.hilite_delta) {
        char label[4], vbuf[BUFSZ];
        Sprintf(label, "%c:", oc_syms[COIN_CLASS]);
        Sprintf(vbuf, "%-2ld", money_cnt(invent));
        botl_hilite_field(newbot2, label, vbuf, BL_GOLD, 2);
    } else
#endif
    Sprintf(nb = eos(newbot2), "%c:%-2ld", oc_syms[COIN_CLASS],
            money_cnt(invent)
            );

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    Strcat(nb = eos(newbot2), " HP:");
    curs(WIN_STATUS, 1, 1);
    putstr(WIN_STATUS, 0, newbot2);
    flags.botlx = 0;

    if (marathon_mode) {
        Sprintf(nb = eos(nb), "%d", hp);
    } else {
        Sprintf(nb = eos(nb), "%d(%d)", hp, hpmax);
    }
    {
        struct color_option hp_co;
#ifdef STATUS_HILITES
        hp_co = get_hilite_color(BL_HP);
        if (hp_co.color == NO_COLOR && !hp_co.attr_bits)
#endif
        hp_co = percentage_color_of(hp, hpmax, hp_colors);
        apply_color_option(hp_co, newbot2, 2);
    }
#else
    if (marathon_mode) {
        Sprintf(nb = eos(nb), " HP:%d", hp);
    } else {
        Sprintf(nb = eos(nb), " HP:%d(%d)", hp, hpmax);
    }
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    Strcat(nb = eos(nb), " Pw:");
    curs(WIN_STATUS, 1, 1);
    putstr(WIN_STATUS, 0, newbot2);

    Sprintf(nb = eos(nb), "%d(%d)", u.uen, u.uenmax);
    {
        struct color_option pw_co;
#ifdef STATUS_HILITES
        pw_co = get_hilite_color(BL_ENE);
        if (pw_co.color == NO_COLOR && !pw_co.attr_bits)
#endif
        pw_co = percentage_color_of(u.uen, u.uenmax, pw_colors);
        apply_color_option(pw_co, newbot2, 2);
    }
#else
    Sprintf(nb = eos(nb), " Pw:%d(%d)", u.uen, u.uenmax);
#endif
#ifdef STATUS_HILITES
    if (iflags.hilite_delta) {
        char vbuf[BUFSZ];
        Sprintf(vbuf, "%-2d", u.uac);
        botl_hilite_field(newbot2, " AC:", vbuf, BL_AC, 2);
        if (Upolyd) {
            Sprintf(vbuf, "%d", mons[u.umonnum].mlevel);
            botl_hilite_field(newbot2, " HD:", vbuf, BL_HD, 2);
        }
#ifdef EXP_ON_BOTL
        else if (flags.showexp) {
            Sprintf(vbuf, "%u", u.ulevel);
            botl_hilite_field(newbot2, " Xp:", vbuf, BL_XP, 2);
            Sprintf(vbuf, "/%-1ld", u.uexp);
            botl_hilite_field(newbot2, "", vbuf, BL_EXP, 2);
        }
#endif
        else {
            Sprintf(vbuf, "%u", u.ulevel);
            botl_hilite_field(newbot2, " Exp:", vbuf, BL_XP, 2);
        }
    } else
#endif
    {
        Sprintf(nb = eos(newbot2), " AC:%-2d", u.uac);
        if (Upolyd) {
            Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
        }
#ifdef EXP_ON_BOTL
        else if (flags.showexp) {
            Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel, u.uexp);
        }
#endif
        else
            Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);
    }

#ifdef SHOW_WEIGHT
    if (flags.showweight) {
        Sprintf(eos(newbot2), " Wt:%ld/%ld", (long)(inv_weight()+weight_cap()),
                (long)weight_cap());
    }
#endif

    if (flags.time) {
#ifdef STATUS_HILITES
        if (iflags.hilite_delta) {
            char vbuf[BUFSZ];
            Sprintf(vbuf, "%ld", moves);
            botl_hilite_field(newbot2, " T:", vbuf, BL_TIME, 2);
        } else
#endif
        Sprintf(eos(newbot2), " T:%ld", moves);
    }

#ifdef REALTIME_ON_BOTL
    if (iflags.showrealtime) {
        Sprintf(eos(newbot2), " %s", botl_realtime());
    }
#endif
    if (iflags.statuslines < 3) {
        botl_text(Stoned,    "Stone",  newbot2, 1);
        botl_text(Slimed,    "Slime",  newbot2, 1);
        botl_text(Strangled, "Strngl", newbot2, 1);
        botl_text(Sick && (u.usick_type & SICK_VOMITABLE),    "FoodPois", newbot2, 1);
        botl_text(Sick && (u.usick_type & SICK_NONVOMITABLE), "Ill",      newbot2, 1);
        botl_text(u.uhs != NOT_HUNGRY, hu_stat[u.uhs], newbot2, 1);
        botl_text(cap > UNENCUMBERED, enc_stat[cap], newbot2, 1);
        botl_text(Blind,         "Blind", newbot2, 1);
        botl_text(Stunned,       "Stun",  newbot2, 1);
        botl_text(Confusion,     "Conf",  newbot2, 1);
        botl_text(Hallucination, "Hallu", newbot2, 1);

        /* levitation and flying are mutually exclusive; riding is not */
        if (Levitation) {
            botl_text(Levitation, "Lev", newbot2, 1);
        } else {
            botl_text(Flying, "Fly", newbot2, 1);
        }

        botl_text(u.usteed != NULL,  "Ride",   newbot2, 1);
        botl_text(u.ufeetfrozen > 0, "Frozen", newbot2, 1);
    }

    int engr_type;
    if ((engr_type = sengr_at("Elbereth", u.ux, u.uy))) {
        boolean feelable_engraving = (engr_type == ENGRAVE || engr_type == BURN) && can_reach_floor(FALSE);
        if (!Blind || feelable_engraving) {
            botl_text(1, "Elbereth", newbot2, 1);
        }
    }
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    flags.botlx = save_botlx;
#endif
#ifdef DUMP_LOG
}
static void
bot2(void)
{
    char newbot2[MAXCO];

    if (suppress_map_output()) {
        return;
    }

    bot2str(newbot2);
    int save_botlx = flags.botlx;
#endif
    curs(WIN_STATUS, 1, 1);
    /* See bot1(): botlx = 0 */
    flags.botlx = 0;
    putstr(WIN_STATUS, 0, newbot2);
    flags.botlx = save_botlx;
}

void
bot3str(char *newbot3)
{
    newbot3[0] = '\0';

    int cap = near_capacity();
    botl_text(cap > UNENCUMBERED, enc_stat[cap], newbot3, 2);
    unsigned len = strlen(newbot3);
    while (len < strlen(enc_stat[OVERLOADED]) + 1 ) {
        strcat(newbot3, " ");
        len++;
    }

    botl_text_or_blanks(u.uhs != NOT_HUNGRY, hu_stat[u.uhs], newbot3, 2);
    botl_text_or_blanks(Stoned,    "Stone",  newbot3, 2);
    botl_text_or_blanks(Slimed,    "Slime",  newbot3, 2);
    botl_text_or_blanks(Strangled, "Strngl", newbot3, 2);
    botl_text_or_blanks(Sick && (u.usick_type & SICK_VOMITABLE),    "FoodPois", newbot3, 2);
    botl_text_or_blanks(Sick && (u.usick_type & SICK_NONVOMITABLE), "Ill",      newbot3, 2);
    botl_text_or_blanks(Blind,         "Blind", newbot3, 2);
    botl_text_or_blanks(Stunned,       "Stun",  newbot3, 2);
    botl_text_or_blanks(Confusion,     "Conf",  newbot3, 2);
    botl_text_or_blanks(Hallucination, "Hallu", newbot3, 2);

    /* levitation and flying are mutually exclusive; riding is not */
    if (Levitation) {
        botl_text_or_blanks(Levitation, "Lev", newbot3, 2);
    } else {
        botl_text_or_blanks(Flying, "Fly", newbot3, 2);
    }

    if (CO > 90) {
        botl_text_or_blanks(u.usteed != NULL,  "Ride",   newbot3, 2);
        botl_text_or_blanks(u.ufeetfrozen > 0, "Frozen", newbot3, 2);
    }
}

static void
bot3(void)
{
    char newbot3[MAXCO];

    if (suppress_map_output()) {
        return;
    }

    if (iflags.statuslines >= 3) {
        int save_botlx = flags.botlx;
        bot3str(newbot3);

        curs(WIN_STATUS, 1, 2);
        /* See bot1(): botlx = 0 */
        flags.botlx = 0;
        putstr(WIN_STATUS, 0, newbot3);
        flags.botlx = save_botlx;
    }
}

#ifdef STATUS_HILITES
static struct istat_s initblstats[MAXBLSTATS]; /* forward decl; defined below */
static int compare_blstats(struct istat_s *, struct istat_s *);

/* Per-field temporary-hilite bookkeeping for the simplified tty/curses
   render path (upstream's .time/.chg mechanism in struct istat_s lives
   behind the VIA_WINDOWPORT() gate which neither tty nor curses use).
   hilite_until_moves[fld] is the move count up to (and including) which
   the last up/down/changed transition should keep being shown; 0 means
   no active temporary hilite.  hilite_sign[fld] encodes the direction:
   <0 = down, >0 = up, 0 = no change. */
static long hilite_until_moves[MAXBLSTATS];
static int  hilite_sign[MAXBLSTATS];

/* Populate the "current" half of the dual blstats buffer with live game
   state and toggle gn.now_or_before_idx so that get_hilite_color() can
   compute chg = compare_blstats(prev, curr) correctly. This is a minimal
   subset of bot_via_windowport() covering the fields we hilite on. */
void
update_blstats(void)
{
    static boolean blstats_initialized = FALSE;
    static long last_update_seq = -1L;
    static boolean have_run_once = FALSE;
    int idx, i;
    long money;

    /* Gate: run at most once per hero sub-move.  curses/tty redraw the
       status bar several times in a row (bot1/2/3 each trigger a
       putstr which re-enters curses_update_stats, plus curses_putch
       and curses_decrement_highlights); toggling on every call would
       flip prev/curr unpredictably and leave compare_blstats() with
       zero deltas. */
    if (have_run_once && hero_seq == last_update_seq) {
        return;
    }
    last_update_seq = hero_seq;
    have_run_once = TRUE;

    /* init_blstats() is normally gated behind VIA_WINDOWPORT(); for the
       tty/curses render path we trigger it lazily on first use so both
       buffers inherit proper .anytype/.fld from initblstats[]. */
    if (!blstats_initialized) {
        for (i = 0; i <= 1; i++) {
            int j;
            for (j = 0; j < MAXBLSTATS; j++) {
                struct hilite_s *keep = gb.blstats[i][j].thresholds;
                gb.blstats[i][j] = initblstats[j];
                gb.blstats[i][j].a = zeroany;
                gb.blstats[i][j].rawval = zeroany;
                gb.blstats[i][j].thresholds = keep;
                if (gb.blstats[i][j].valwidth && !gb.blstats[i][j].val) {
                    gb.blstats[i][j].val = (char *) alloc(gb.blstats[i][j].valwidth);
                    gb.blstats[i][j].val[0] = '\0';
                }
            }
        }
        blstats_initialized = TRUE;
    }

    /* toggle */
    idx = 1 - gn.now_or_before_idx;
    gn.now_or_before_idx = idx;

    gb.blstats[idx][BL_STR].a.a_int = ACURR(A_STR);
    gb.blstats[idx][BL_DX].a.a_int = ACURR(A_DEX);
    gb.blstats[idx][BL_CO].a.a_int = ACURR(A_CON);
    gb.blstats[idx][BL_IN].a.a_int = ACURR(A_INT);
    gb.blstats[idx][BL_WI].a.a_int = ACURR(A_WIS);
    gb.blstats[idx][BL_CH].a.a_int = ACURR(A_CHA);

#ifdef SCORE_ON_BOTL
    gb.blstats[idx][BL_SCORE].a.a_long = flags.showscore ? botl_score() : 0L;
#endif

    i = Upolyd ? u.mh : u.uhp;
    if (i < 0) i = 0;
    gb.blstats[idx][BL_HP].rawval.a_int = i;
    gb.blstats[idx][BL_HP].a.a_int = min(i, 9999);
    i = Upolyd ? u.mhmax : u.uhpmax;
    gb.blstats[idx][BL_HPMAX].rawval.a_int = i;
    gb.blstats[idx][BL_HPMAX].a.a_int = min(i, 9999);

    if ((money = money_cnt(invent)) < 0L) money = 0L;
    gb.blstats[idx][BL_GOLD].rawval.a_long = money;
    gb.blstats[idx][BL_GOLD].a.a_long = min(money, 999999L);

    gb.blstats[idx][BL_ENE].rawval.a_int = u.uen;
    gb.blstats[idx][BL_ENE].a.a_int = min(u.uen, 9999);
    gb.blstats[idx][BL_ENEMAX].rawval.a_int = u.uenmax;
    gb.blstats[idx][BL_ENEMAX].a.a_int = min(u.uenmax, 9999);

    gb.blstats[idx][BL_AC].a.a_int = u.uac;
    gb.blstats[idx][BL_HD].a.a_int = Upolyd ? (int) mons[u.umonnum].mlevel : 0;
    gb.blstats[idx][BL_XP].a.a_int = u.ulevel;
    gb.blstats[idx][BL_EXP].a.a_long = u.uexp;
    gb.blstats[idx][BL_TIME].a.a_long = moves;
    gb.blstats[idx][BL_HUNGER].a.a_int = (int) u.uhs;
    gb.blstats[idx][BL_CAP].a.a_int = near_capacity();

    /* Record per-field temporary-hilite windows.  For each watched
       field, if the value changed this turn, remember the direction
       and the move at which the temporary hilite should expire. */
    {
        int f;
        long until = moves + (iflags.hilite_delta > 0 ? iflags.hilite_delta : 0);
        for (f = 0; f < MAXBLSTATS; f++) {
            int d = compare_blstats(&gb.blstats[1 - idx][f],
                                    &gb.blstats[idx][f]);
            if (d != 0) {
                hilite_sign[f] = (d > 0) ? 1 : -1;
                hilite_until_moves[f] = until;
            }
        }
    }
}
#endif /* STATUS_HILITES */

void
bot(void)
{
#ifdef STATUS_HILITES
    update_blstats();
#endif
    bot1();
    bot2();
    bot3();
    flags.botl = flags.botlx = 0;
}

/* =======================================================================*/
/*  statusnew routines                                                    */
/* =======================================================================*/

/* structure that tracks the status details in the core */

#ifdef STATUS_HILITES
#endif /* STATUS_HILITES */

static boolean eval_notify_windowport_field(int, boolean *, int);
static void evaluate_and_notify_windowport(boolean *, int);
static void init_blstats(void);
static int compare_blstats(struct istat_s *, struct istat_s *);
static char *anything_to_s(char *, anything *, int);
static int percentage(struct istat_s *, struct istat_s *);
static int exp_percentage(void);
static int QSORTCALLBACK cond_cmp(const genericptr, const genericptr);
static int QSORTCALLBACK menualpha_cmp(const genericptr, const genericptr);

#ifdef STATUS_HILITES
static void s_to_anything(anything *, char *, int);
static enum statusfields fldname_to_bl_indx(const char *);
static boolean hilite_reset_needed(struct istat_s *, long);
static boolean noneoftheabove(const char *);
static struct hilite_s *get_hilite(int, int, genericptr_t, int, int, int *);
static void split_clridx(int, int *, int *);
static boolean is_ltgt_percentnumber(const char *);
static boolean has_ltgt_percentnumber(const char *);
static int splitsubfields(char *, char ***, int);
static boolean is_fld_arrayvalues(const char *, const char *const *,
                                  int, int, int *);
static int query_arrayvalue(const char *, const char *const *, int, int);
static void status_hilite_add_threshold(int, struct hilite_s *);
static boolean parse_status_hl2(char (*)[QBUFSZ], boolean);
static unsigned long query_conditions(void);
static char *conditionbitmask2str(unsigned long);
static unsigned long match_str2conditionbitmask(const char *);
static unsigned long str2conditionbitmask(char *);
static boolean parse_condition(char (*)[QBUFSZ], int);
static char *hlattr2attrname(int, char *, size_t);


static void status_hilite_linestr_add(int, struct hilite_s *,
                                      unsigned long, const char *);
static void status_hilite_linestr_done(void);
static int status_hilite_linestr_countfield(int);
static void status_hilite_linestr_gather_conditions(void);
static void status_hilite_linestr_gather(void);
static char *status_hilite2str(struct hilite_s *);
static int status_hilite_menu_choose_field(void);
static int status_hilite_menu_choose_behavior(int);
static int status_hilite_menu_choose_updownboth(int, const char *,
                                                boolean, boolean);
static boolean status_hilite_menu_add(int);
static boolean status_hilite_remove(int);
static boolean status_hilite_menu_fld(int);
static void status_hilites_viewall(void);

#define has_hilite(i) (gb.blstats[0][(i)].thresholds)
/* TH_UPDOWN encompasses specific 'up' and 'down' also general 'changed' */
#define Is_Temp_Hilite(rule) ((rule) && (rule)->behavior == BL_TH_UPDOWN)

/* pointers to current hilite rule and list of this field's defined rules */
#define INIT_THRESH  , (struct hilite_s *) 0, (struct hilite_s *) 0
#else /* !STATUS_HILITES */
#define INIT_THRESH /*empty*/
#endif

#define INIT_BLSTAT(name, fmtstr, anytyp, wid, fld) \
    { name, fmtstr, 0L, FALSE, FALSE, 0, anytyp,                        \
      { (genericptr_t) 0 }, { (genericptr_t) 0 }, (char *) 0,           \
      wid, -1, fld  INIT_THRESH }
#define INIT_BLSTATP(name, fmtstr, anytyp, wid, maxfld, fld) \
    { name, fmtstr, 0L, FALSE, TRUE, 0, anytyp,                         \
      { (genericptr_t) 0 }, { (genericptr_t) 0 }, (char *) 0,           \
      wid, maxfld, fld  INIT_THRESH }

/* If entries are added to this, botl.h will require updating too.
   'max' value of BL_EXP gets special handling since the percentage
   involved isn't a direct 100*current/maximum calculation. */
static struct istat_s initblstats[MAXBLSTATS] = {
    INIT_BLSTAT("title", "%s", ANY_STR, MAXVALWIDTH, BL_TITLE),
    INIT_BLSTAT("strength", " St:%s", ANY_INT, 10, BL_STR),
    INIT_BLSTAT("dexterity", " Dx:%s", ANY_INT, 10, BL_DX),
    INIT_BLSTAT("constitution", " Co:%s", ANY_INT, 10, BL_CO),
    INIT_BLSTAT("intelligence", " In:%s", ANY_INT, 10, BL_IN),
    INIT_BLSTAT("wisdom", " Wi:%s", ANY_INT, 10, BL_WI),
    INIT_BLSTAT("charisma", " Ch:%s", ANY_INT, 10, BL_CH),
    INIT_BLSTAT("alignment", " %s", ANY_STR, 40, BL_ALIGN),
    INIT_BLSTAT("score", " S:%s", ANY_LONG, 20, BL_SCORE),
    INIT_BLSTAT("carrying-capacity", " %s", ANY_INT, 20, BL_CAP),
    INIT_BLSTAT("gold", " %s", ANY_LONG, 30, BL_GOLD),
    INIT_BLSTATP("power", " Pw:%s", ANY_INT, 10, BL_ENEMAX, BL_ENE),
    INIT_BLSTAT("power-max", "(%s)", ANY_INT, 10, BL_ENEMAX),
    INIT_BLSTATP("experience-level", " Xp:%s", ANY_INT, 10, BL_EXP, BL_XP),
    INIT_BLSTAT("armor-class", " AC:%s", ANY_INT, 10, BL_AC),
    INIT_BLSTAT("HD", " HD:%s", ANY_INT, 10, BL_HD),
    INIT_BLSTAT("time", " T:%s", ANY_LONG, 20, BL_TIME),
    /* hunger used to be 'ANY_UINT'; see note below in bot_via_windowport() */
    INIT_BLSTAT("hunger", " %s", ANY_INT, 40, BL_HUNGER),
    INIT_BLSTATP("hitpoints", " HP:%s", ANY_INT, 10, BL_HPMAX, BL_HP),
    INIT_BLSTAT("hitpoints-max", "(%s)", ANY_INT, 10, BL_HPMAX),
    INIT_BLSTAT("dungeon-level", "%s", ANY_STR, MAXVALWIDTH, BL_LEVELDESC),
    INIT_BLSTATP("experience", "/%s", ANY_LONG, 20, BL_EXP, BL_EXP),
    INIT_BLSTAT("condition", "%s", ANY_MASK32, 0, BL_CONDITION),
    /* optional; once set it doesn't change unless 'showvers' option is
       toggled or player modifies the 'versinfo' option;
       available mostly for screenshots or someone looking over shoulder;
       blstat[][BL_VERS] is actually an int copy of flags.versinfo (0...7) */
    INIT_BLSTAT("version", " %s", ANY_STR, MAXVALWIDTH, BL_VERS),
};

#undef INIT_BLSTATP
#undef INIT_BLSTAT
#undef INIT_THRESH

#ifdef STATUS_HILITES
static const struct condmap condition_aliases[] = {
    { "strangled",      BL_MASK_STRNGL },
    { "all",            BL_MASK_BAREH | BL_MASK_BLIND | BL_MASK_BUSY
                        | BL_MASK_CONF | BL_MASK_DEAF | BL_MASK_ELF_IRON
                        | BL_MASK_FLY | BL_MASK_FOODPOIS | BL_MASK_GLOWHANDS
                        | BL_MASK_GRAB | BL_MASK_HALLU | BL_MASK_HELD
                        | BL_MASK_ICY | BL_MASK_INLAVA | BL_MASK_LEV
                        | BL_MASK_PARLYZ | BL_MASK_RIDE | BL_MASK_SLEEPING
                        | BL_MASK_SLIME | BL_MASK_SLIPPERY | BL_MASK_STONE
                        | BL_MASK_STRNGL | BL_MASK_STUN | BL_MASK_SUBMERGED
                        | BL_MASK_TERMILL | BL_MASK_TETHERED
                        | BL_MASK_TRAPPED | BL_MASK_UNCONSC
                        | BL_MASK_WOUNDEDL | BL_MASK_HOLDING },
    { "major_troubles", BL_MASK_FOODPOIS | BL_MASK_GRAB | BL_MASK_INLAVA
                        | BL_MASK_SLIME | BL_MASK_STONE | BL_MASK_STRNGL
                        | BL_MASK_TERMILL },
    { "minor_troubles", BL_MASK_BLIND | BL_MASK_CONF | BL_MASK_DEAF
                        | BL_MASK_HALLU | BL_MASK_PARLYZ | BL_MASK_SUBMERGED
                        | BL_MASK_STUN },
    { "movement",       BL_MASK_LEV | BL_MASK_FLY | BL_MASK_RIDE },
    { "opt_in",         BL_MASK_BAREH | BL_MASK_BUSY | BL_MASK_GLOWHANDS
                        | BL_MASK_HELD | BL_MASK_ICY | BL_MASK_PARLYZ
                        | BL_MASK_SLEEPING | BL_MASK_SLIPPERY
                        | BL_MASK_SUBMERGED | BL_MASK_TETHERED
                        | BL_MASK_TRAPPED
                        | BL_MASK_UNCONSC | BL_MASK_WOUNDEDL
                        | BL_MASK_HOLDING },
};
#endif /* STATUS_HILITES */

/* condition names and their abbreviations are used by windowport code */
const struct conditions_t conditions[] = {
    /* ranking, mask, identifier, txt1, txt2, txt3 */
    { 20, BL_MASK_BAREH,     bl_bareh,     { "Bare",     "Bar",   "Bh"  } },
    { 10, BL_MASK_BLIND,     bl_blind,     { "Blind",    "Blnd",  "Bl"  } },
    { 20, BL_MASK_BUSY,      bl_busy,      { "Busy",     "Bsy",   "By"  } },
    { 10, BL_MASK_CONF,      bl_conf,      { "Conf",     "Cnf",   "Cf"  } },
    { 10, BL_MASK_DEAF,      bl_deaf,      { "Deaf",     "Def",   "Df"  } },
    { 15, BL_MASK_ELF_IRON,  bl_elf_iron,  { "Iron",     "Irn",   "Fe"  } },
    { 10, BL_MASK_FLY,       bl_fly,       { "Fly",      "Fly",   "Fl"  } },
    {  6, BL_MASK_FOODPOIS,  bl_foodpois,  { "FoodPois", "Fpois", "Poi" } },
    { 20, BL_MASK_GLOWHANDS, bl_glowhands, { "Glow",     "Glo",   "Gl"  } },
    {  2, BL_MASK_GRAB,      bl_grab,      { "Grab",     "Grb",   "Gr"  } },
    { 10, BL_MASK_HALLU,     bl_hallu,     { "Hallu",    "Hal",   "Hl"  } },
    { 20, BL_MASK_HELD,      bl_held,      { "Held",     "Hld",   "Hd"  } },
    { 20, BL_MASK_ICY,       bl_icy,       { "Icy",      "Icy",   "Ic"  } },
    {  8, BL_MASK_INLAVA,    bl_inlava,    { "Lava",     "Lav",   "La"  } },
    { 10, BL_MASK_LEV,       bl_lev,       { "Lev",      "Lev",   "Lv"  } },
    { 20, BL_MASK_PARLYZ,    bl_parlyz,    { "Parlyz",   "Para",  "Par" } },
    { 10, BL_MASK_RIDE,      bl_ride,      { "Ride",     "Rid",   "Rd"  } },
    { 20, BL_MASK_SLEEPING,  bl_sleeping,  { "Zzz",      "Zzz",   "Zz"  } },
    {  6, BL_MASK_SLIME,     bl_slime,     { "Slime",    "Slim",  "Slm" } },
    { 20, BL_MASK_SLIPPERY,  bl_slippery,  { "Slip",     "Sli",   "Sl"  } },
    {  6, BL_MASK_STONE,     bl_stone,     { "Stone",    "Ston",  "Sto" } },
    {  4, BL_MASK_STRNGL,    bl_strngl,    { "Strngl",   "Stngl", "Str" } },
    { 10, BL_MASK_STUN,      bl_stun,      { "Stun",     "Stun",  "St"  } },
    { 15, BL_MASK_SUBMERGED, bl_submerged, { "Sub",      "Sub",   "Sw"  } },
    {  6, BL_MASK_TERMILL,   bl_termill,   { "TermIll",  "Ill",   "Ill" } },
    { 20, BL_MASK_TETHERED,  bl_tethered,  { "Teth",     "Tth",   "Te"  } },
    { 20, BL_MASK_TRAPPED,   bl_trapped,   { "Trap",     "Trp",   "Tr"  } },
    { 20, BL_MASK_UNCONSC,   bl_unconsc,   { "Out",      "Out",   "KO"  } },
    { 20, BL_MASK_WOUNDEDL,  bl_woundedl,  { "Legs",     "Leg",   "Lg"  } },
    { 20, BL_MASK_HOLDING,   bl_holding,   { "UHold",    "UHld",  "UHd" } },
};

struct condtests_t condtests[CONDITION_COUNT] = {
    /* id, useropt, opt_in or out, enabled, configchoice, testresult;
       default value for enabled is !opt_in but can get changed via options */
    { bl_bareh,     "barehanded",  opt_in,  FALSE, FALSE, FALSE },
    { bl_blind,     "blind",       opt_out, TRUE,  FALSE, FALSE },
    { bl_busy,      "busy",        opt_in,  FALSE, FALSE, FALSE },
    { bl_conf,      "conf",        opt_out, TRUE,  FALSE, FALSE },
    { bl_deaf,      "deaf",        opt_out, TRUE,  FALSE, FALSE },
    { bl_elf_iron,  "iron",        opt_out, TRUE,  FALSE, FALSE },
    { bl_fly,       "fly",         opt_out, TRUE,  FALSE, FALSE },
    { bl_foodpois,  "foodPois",    opt_out, TRUE,  FALSE, FALSE },
    { bl_glowhands, "glowhands",   opt_in,  FALSE, FALSE, FALSE },
    { bl_grab,      "grab",        opt_out, TRUE,  FALSE, FALSE },
    { bl_hallu,     "hallucinat",  opt_out, TRUE,  FALSE, FALSE },
    { bl_held,      "held",        opt_in,  FALSE, FALSE, FALSE },
    { bl_icy,       "ice",         opt_in,  FALSE, FALSE, FALSE },
    { bl_inlava,    "lava",        opt_out, TRUE,  FALSE, FALSE },
    { bl_lev,       "levitate",    opt_out, TRUE,  FALSE, FALSE },
    { bl_parlyz,    "paralyzed",   opt_in,  FALSE, FALSE, FALSE },
    { bl_ride,      "ride",        opt_out, TRUE,  FALSE, FALSE },
    { bl_sleeping,  "sleep",       opt_in,  FALSE, FALSE, FALSE },
    { bl_slime,     "slime",       opt_out, TRUE,  FALSE, FALSE },
    { bl_slippery,  "slip",        opt_in,  FALSE, FALSE, FALSE },
    { bl_stone,     "stone",       opt_out, TRUE,  FALSE, FALSE },
    { bl_strngl,    "strngl",      opt_out, TRUE,  FALSE, FALSE },
    { bl_stun,      "stun",        opt_out, TRUE,  FALSE, FALSE },
    { bl_submerged, "submerged",   opt_in,  FALSE, FALSE, FALSE },
    { bl_termill,   "termIll",     opt_out, TRUE,  FALSE, FALSE },
    { bl_tethered,  "tethered",    opt_in,  FALSE, FALSE, FALSE },
    { bl_trapped,   "trap",        opt_in,  FALSE, FALSE, FALSE },
    { bl_unconsc,   "unconscious", opt_in,  FALSE, FALSE, FALSE },
    { bl_woundedl,  "woundedlegs", opt_in,  FALSE, FALSE, FALSE },
    { bl_holding,   "holding",     opt_in,  FALSE, FALSE, FALSE },
};

/* condition indexing */
int cond_idx[CONDITION_COUNT] = { 0 };

/* cache-related */
static boolean cache_avail[3] = { FALSE, FALSE, FALSE };
static boolean cache_reslt[3] = { FALSE, FALSE, FALSE };
static const char *cache_nomovemsg = NULL, *cache_multi_reason = NULL;

#define cond_cache_prepA() \
do {                                                        \
    boolean clear_cache = FALSE, refresh_cache = FALSE;     \
                                                            \
    if (multi < 0) {                                        \
        if (nomovemsg || multi_reason) {                    \
            if (cache_nomovemsg != nomovemsg)               \
                refresh_cache = TRUE;                       \
            if (cache_multi_reason != multi_reason)         \
                refresh_cache = TRUE;                       \
        } else {                                            \
            clear_cache = TRUE;                             \
        }                                                   \
    } else {                                                \
        clear_cache = TRUE;                                 \
    }                                                       \
    if (clear_cache) {                                      \
        cache_nomovemsg = (const char *) 0;                 \
        cache_multi_reason = (const char *) 0;              \
    }                                                       \
    if (refresh_cache) {                                    \
        cache_nomovemsg = nomovemsg;                        \
        cache_multi_reason = multi_reason;                  \
    }                                                       \
    if (clear_cache || refresh_cache) {                     \
        cache_reslt[0] = cache_avail[0] = FALSE;            \
        cache_reslt[1] = cache_avail[1] = FALSE;            \
    }                                                       \
} while (0)

/* we don't put this next declaration in #ifdef STATUS_HILITES.
 * In the absence of STATUS_HILITES, each array
 * element will be 0 however, and quite meaningless,
 * but we need to pass the first array element as
 * the final argument of status_update, with or
 * without STATUS_HILITES.
 */

static void
bot_via_windowport(void)
{
    char buf[BUFSZ];
    const char *titl;
    char *nb;
    int i, idx, cap;
    long money;

    if (!gb.blinit) {
        panic("bot before init.");
    }

    /* toggle from previous iteration */
    idx = 1 - gn.now_or_before_idx; /* 0 -> 1, 1 -> 0 */
    gn.now_or_before_idx = idx;

    /* clear the "value set" indicators */
    (void) memset((genericptr_t) gv.valset, 0, MAXBLSTATS * sizeof (boolean));

    /*
     * Note: min(x,9999) - we enforce the same maximum on hp, maxhp,
     * pw, maxpw, and gold as basic status formatting so that the two
     * modes of status display don't produce different information.
     */

    /*
     *  Player name and title.
     */
    Strcpy(nb = buf, plname);
    nb[0] = highc(nb[0]);
    titl = !Upolyd ? rank() : mons[u.umonnum].mname;
    i = (int) (strlen(buf) + sizeof " the " + strlen(titl) - sizeof "");
    /* if "Name the Rank/monster" is too long, we truncate the name but
       always keep at least BOTL_NSIZ characters of it; when hitpointbar is
       enabled, anything beyond 30 (long monster name) will be truncated */
    if (i > 30) {
        i = 30 - (int) (sizeof " the " + strlen(titl) - sizeof "");
        nb[max(i, BOTL_NSIZ)] = '\0';
    }
    Strcpy(nb = eos(nb), " the ");
    Strcpy(nb = eos(nb), titl);
    if (Upolyd) { /* when poly'd, capitalize monster name */
        for (i = 0; nb[i]; i++)
            if (i == 0 || nb[i - 1] == ' ')
                nb[i] = highc(nb[i]);
    }
    Sprintf(gb.blstats[idx][BL_TITLE].val, "%-30s", buf);
    gv.valset[BL_TITLE] = TRUE; /* indicate val already set */

    /* Strength */
    gb.blstats[idx][BL_STR].a.a_int = ACURR(A_STR);
    Strcpy(gb.blstats[idx][BL_STR].val, get_strength_str());
    gv.valset[BL_STR] = TRUE; /* indicate val already set */

    /*  Dexterity, constitution, intelligence, wisdom, charisma. */
    gb.blstats[idx][BL_DX].a.a_int = ACURR(A_DEX);
    gb.blstats[idx][BL_CO].a.a_int = ACURR(A_CON);
    gb.blstats[idx][BL_IN].a.a_int = ACURR(A_INT);
    gb.blstats[idx][BL_WI].a.a_int = ACURR(A_WIS);
    gb.blstats[idx][BL_CH].a.a_int = ACURR(A_CHA);

    /* Alignment */
    Strcpy(gb.blstats[idx][BL_ALIGN].val, (u.ualign.type == A_CHAOTIC) ? "Chaotic"
                                        : (u.ualign.type == A_NEUTRAL) ? "Neutral"
                                                                       : "Lawful");

    /* Score */
    gb.blstats[idx][BL_SCORE].a.a_long =
#ifdef SCORE_ON_BOTL
        flags.showscore ? botl_score() :
#endif
        0L;

    /*  Hit points  */
    i = Upolyd ? u.mh : u.uhp;
    if (i < 0) { /* gameover sets u.uhp to -1 */
        i = 0;
    }
    gb.blstats[idx][BL_HP].rawval.a_int = i;
    gb.blstats[idx][BL_HP].a.a_int = min(i, 9999);
    i = Upolyd ? u.mhmax : u.uhpmax;
    gb.blstats[idx][BL_HPMAX].rawval.a_int = i;
    gb.blstats[idx][BL_HPMAX].a.a_int = min(i, 9999);

    /*  Dungeon level. */
    (void) describe_level(gb.blstats[idx][BL_LEVELDESC].val, 1);
    gv.valset[BL_LEVELDESC] = TRUE; /* indicate val already set */

    /* Gold */
    if ((money = money_cnt(invent)) < 0L) {
        money = 0L; /* ought to issue impossible() and then discard gold */
    }
    gb.blstats[idx][BL_GOLD].rawval.a_long = money;
    gb.blstats[idx][BL_GOLD].a.a_long = min(money, 999999L);
    /*
     * The tty port needs to display the current symbol for gold
     * as a field header, so to accommodate that we pass gold with
     * that already included. If a window port needs to use the text
     * gold amount without the leading "$:" the port will have to
     * skip past ':' to the value pointer it was passed in status_update()
     * for the BL_GOLD case.
     *
     * Another quirk of BL_GOLD is that the field display may have
     * changed if a new symbol set was loaded, or we entered or left
     * the rogue level.
     *
     * The currency prefix is encoded as ten character \GXXXXNNNN
     * sequence.
     */
    Sprintf(gb.blstats[idx][BL_GOLD].val, "%s:%ld",
            (iflags.in_dumplog || iflags.invis_goldsym) ? "$"
              : encglyph(objnum_to_glyph(GOLD_PIECE)),
            gb.blstats[idx][BL_GOLD].a.a_long);
    gv.valset[BL_GOLD] = TRUE; /* indicate val already set */

    /* Power (magical energy) */
    gb.blstats[idx][BL_ENE].rawval.a_int = u.uen;
    gb.blstats[idx][BL_ENE].a.a_int = min(u.uen, 9999);
    gb.blstats[idx][BL_ENEMAX].rawval.a_int = u.uenmax;
    gb.blstats[idx][BL_ENEMAX].a.a_int = min(u.uenmax, 9999);

    /* Armor class */
    gb.blstats[idx][BL_AC].a.a_int = u.uac;

    /* Monster level (if Upolyd) */
    gb.blstats[idx][BL_HD].a.a_int = Upolyd ? (int) mons[u.umonnum].mlevel : 0;

    /* Experience */
    gb.blstats[idx][BL_XP].a.a_int = u.ulevel;
    gb.blstats[idx][BL_EXP].a.a_long = u.uexp;

    /* Time (moves) */
    gb.blstats[idx][BL_TIME].a.a_long = moves;

    /* Hunger */
    /* note: u.uhs is unsigned, and 3.6.1's STATUS_HILITE defined
       BL_HUNGER to be ANY_UINT, but that was the only non-int/non-long
       numeric field so it's far simpler to treat it as plain int and
       not need ANY_UINT handling at all */
    gb.blstats[idx][BL_HUNGER].a.a_int = (int) u.uhs;
    Strcpy(gb.blstats[idx][BL_HUNGER].val,
           (u.uhs != NOT_HUNGRY) ? hu_stat[u.uhs] : "");
    gv.valset[BL_HUNGER] = TRUE;

    /* Carrying capacity */
    cap = near_capacity();
    gb.blstats[idx][BL_CAP].a.a_int = cap;
    Strcpy(gb.blstats[idx][BL_CAP].val,
           (cap > UNENCUMBERED) ? enc_stat[cap] : "");
    gv.valset[BL_CAP] = TRUE;

#ifdef NEXT_VERSION
    /* Version; unchanging unless player toggles 'showvers' option or
       modifies 'versinfo' option; toggling showvers off will clear it */
    if (gb.blstats[idx][BL_VERS].a.a_int != (int) flags.versinfo) {
        gb.blstats[idx][BL_VERS].a.a_int = (int) flags.versinfo;
        gv.valset[BL_VERS] = FALSE;
    }
    if (!gv.valset[BL_VERS]) {
        (void) status_version(gb.blstats[idx][BL_VERS].val,
                              gb.blstats[idx][BL_VERS].valwidth, FALSE);
        gv.valset[BL_VERS] = TRUE;
    }
#endif

    /* Conditions */

    gb.blstats[idx][BL_CONDITION].a.a_ulong = 0L;

    /*
     * Avoid anything that does string comparisons in here because this
     * is called *extremely* often, for every screen update and the same
     * string comparisons would be repeated, thus contributing toward
     * performance degradation.  If it is essential that string comparisons
     * are needed for a particular condition, consider adding a caching
     * mechanism to limit the string comparisons to the first occurrence
     * for that cache lifetime.  There is caching of that nature done for
     * unconsc (1) and parlyz (2) because the suggested way of being able
     * to distinguish unconsc, parlyz, sleeping, and busy involves multiple
     * string comparisons.
     *
     * [Rebuttal:  it's called a lot for Windows and MS-DOS because their
     * sample run-time configuration file enables 'time' (move counter).
     * The optimization to bypass full status update when only 'time'
     * has changed (via timebot(), only effective for VIA_WINDOWPORT()
     * configurations) should ameliorate that.]
     */

#define test_if_enabled(c) if (condtests[(c)].enabled) condtests[(c)].test

    condtests[bl_foodpois].test = condtests[bl_termill].test = FALSE;
    if (Sick) {
        test_if_enabled(bl_foodpois) = (u.usick_type & SICK_VOMITABLE) != 0;
        test_if_enabled(bl_termill) = (u.usick_type & SICK_NONVOMITABLE) != 0;
    }
    condtests[bl_inlava].test = condtests[bl_tethered].test
        = condtests[bl_trapped].test = FALSE;
    if (u.utrap) {
        test_if_enabled(bl_inlava) = (u.utraptype == TT_LAVA);
        test_if_enabled(bl_tethered) = (u.utraptype == TT_BURIEDBALL);
        /* if in-lava or tethered is disabled and the condition applies,
           lump it in with trapped */
        test_if_enabled(bl_trapped) = (!condtests[bl_inlava].test
                                       && !condtests[bl_tethered].test);
    }
    condtests[bl_grab].test = condtests[bl_held].test
#if 0
        = condtests[bl_engulfed].test
#endif
        = condtests[bl_holding].test = FALSE;
    if (u.ustuck) {
        /* it is possible for a hero in sticks() form to be swallowed,
           so swallowed needs to be checked first; it is not possible for
           a hero in sticks() form to be held--sticky hero does the holding
           even if u.ustuck is also a holder */
        if (u.uswallow) {
            /* engulfed/swallowed isn't currently a tracked status condition;
               "held" might look odd for it but seems better than blank */
#if 0
            test_if_enabled(bl_engulfed) = TRUE;
#else
            test_if_enabled(bl_held) = TRUE;
#endif
        } else if (Upolyd && sticks(youmonst.data)) {
            test_if_enabled(bl_holding) = TRUE;
        } else {
            /* grab == hero is held by sea monster and about to be drowned;
               held == hero is held by something else and can't move away */
            test_if_enabled(bl_grab) = (u.ustuck->data->mlet == S_EEL);
            test_if_enabled(bl_held) = !condtests[bl_grab].test;
        }
    }
    condtests[bl_blind].test     = (Blind) ? TRUE : FALSE;
    condtests[bl_conf].test      = (Confusion) ? TRUE : FALSE;
    condtests[bl_deaf].test      = (Deaf) ? TRUE : FALSE;
    condtests[bl_fly].test       = (Flying) ? TRUE : FALSE;
    condtests[bl_glowhands].test = (u.umconf) ? TRUE : FALSE;
    condtests[bl_hallu].test     = (Hallucination) ? TRUE : FALSE;
    condtests[bl_lev].test       = (Levitation) ? TRUE : FALSE;
    condtests[bl_ride].test      = (u.usteed) ? TRUE : FALSE;
    condtests[bl_slime].test     = (Slimed) ? TRUE : FALSE;
    condtests[bl_stone].test     = (Stoned) ? TRUE : FALSE;
    condtests[bl_strngl].test    = (Strangled) ? TRUE : FALSE;
    condtests[bl_stun].test      = (Stunned) ? TRUE : FALSE;
    condtests[bl_submerged].test = (Underwater) ? TRUE : FALSE;
    test_if_enabled(bl_elf_iron) = (FALSE);
    test_if_enabled(bl_bareh)    = (!uarmg && !uwep);
    test_if_enabled(bl_icy)      = (levl[u.ux][u.uy].typ == ICE);
    test_if_enabled(bl_slippery) = (Glib) ? TRUE : FALSE;
    test_if_enabled(bl_woundedl) = (Wounded_legs) ? TRUE : FALSE;

    if (multi < 0) {
        cond_cache_prepA();
        if (condtests[bl_unconsc].enabled
            && cache_nomovemsg && !cache_avail[0]) {
                cache_reslt[0] = (!u.usleep && unconscious());
                cache_avail[0] = TRUE;
        }
        if (condtests[bl_parlyz].enabled
            && cache_multi_reason && !cache_avail[1]) {
                cache_reslt[1] = (!strncmp(cache_multi_reason, "paralyzed", 9)
                                 || !strncmp(cache_multi_reason, "frozen", 6));
                cache_avail[1] = TRUE;
        }
        if (cache_avail[0] && cache_reslt[0]) {
            condtests[bl_unconsc].test = cache_reslt[0];
        } else if (cache_avail[1] && cache_reslt[1]) {
            condtests[bl_parlyz].test = cache_reslt[1];
        } else if (condtests[bl_sleeping].enabled && u.usleep) {
            condtests[bl_sleeping].test = TRUE;
        } else if (condtests[bl_busy].enabled) {
            condtests[bl_busy].test = TRUE;
        }
    } else {
        condtests[bl_unconsc].test = condtests[bl_parlyz].test =
            condtests[bl_sleeping].test = condtests[bl_busy].test = FALSE;
    }

#define cond_setbit(c) \
        gb.blstats[idx][BL_CONDITION].a.a_ulong |= conditions[(c)].mask

    for (i = 0; i < CONDITION_COUNT; ++i) {
        if (condtests[i].enabled
             /* && i != bl_holding  */ /* uncomment to suppress UHold */
                && condtests[i].test) {
            cond_setbit(i);
        }
    }
#undef cond_bitset

    evaluate_and_notify_windowport(gv.valset, idx);
#undef test_if_enabled
}

#undef cond_cache_prepA

/* functions from other files used by STATUS_HILITES */
#define config_error_add raw_printf

/* update just the status lines' 'time' field */
static void
stat_update_time(void)
{
    int idx = gn.now_or_before_idx; /* no 0/1 toggle */
    int fld = BL_TIME;

    /* Time (moves) */
    gb.blstats[idx][fld].a.a_long = moves;
    gv.valset[fld] = FALSE;

    eval_notify_windowport_field(fld, gv.valset, idx);
#ifdef NEXT_VERSION
    if ((windowprocs.wincap2 & WC2_FLUSH_STATUS) != 0L) {
        status_update(BL_FLUSH, (genericptr_t) 0, 0, 0,
                      NO_COLOR, (unsigned long *) 0);
    }
#endif
    return;
}

/* deal with player's choice to change processing of a condition */
void
condopt(int idx, boolean *addr, boolean negated)
{
    int i;

    /* sanity check */
    if ((idx < 0 || idx >= CONDITION_COUNT)
        || (addr && addr != &condtests[idx].choice)) {
        return;
    }

    if (!addr) {
        /* special: indicates a request to init so
           set the choice values to match the defaults */
        gc.condmenu_sortorder = 0;
        for (i = 0; i < CONDITION_COUNT; ++i) {
            cond_idx[i] = i;
            condtests[i].choice = condtests[i].enabled;
        }
        qsort((genericptr_t) cond_idx, CONDITION_COUNT,
              sizeof cond_idx[0], cond_cmp);
    } else {
        /* (addr == &condtests[idx].choice) */
        condtests[idx].enabled = negated ? FALSE : TRUE;
        condtests[idx].choice = condtests[idx].enabled;
        /* avoid lingering false positives if test is no longer run */
        condtests[idx].test = FALSE;
    }
}

/* qsort callback routine for sorting the condition index */
static int QSORTCALLBACK
cond_cmp(const genericptr vptr1, const genericptr vptr2)
{
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2,
        c1 = conditions[indx1].ranking, c2 = conditions[indx2].ranking;

    if (c1 != c2) {
        return c1 - c2;
    }

    /* tie-breaker - visible alpha by name */
    return strcmpi(condtests[indx1].useroption, condtests[indx2].useroption);
}

/* qsort callback for alphabetical sorting of condition index */
static int QSORTCALLBACK
menualpha_cmp(const genericptr vptr1, const genericptr vptr2)
{
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2;

    return strcmpi(condtests[indx1].useroption, condtests[indx2].useroption);
}

int
parse_cond_option(boolean negated, char *opts)
{
    int i, sl;
    const char *compareto, *uniqpart, prefix[] = "cond_";

    if (!opts || strlen(opts) <= sizeof prefix - 1) {
        return 2;
    }
    uniqpart = opts + (sizeof prefix - 1);
    for (i = 0; i < CONDITION_COUNT; ++i) {
        compareto = condtests[i].useroption;
        sl = Strlen(compareto);
        if (match_optname(uniqpart, compareto, (sl >= 4) ? 4 : sl, FALSE)) {
            condopt(i, &condtests[i].choice, negated);
            return 0;
        }
    }
    return 1; /* !0 indicates error */
}

/* display a menu of all available status condition options and let player
   toggled them on or off; returns True iff any changes are made */
boolean
cond_menu(void)
{
#ifdef NEXT_VERSION
    static const char *const menutitle[2] = {
        "alphabetically", "by ranking"
    };
#endif
    int i, res, idx = 0;
    int sequence[CONDITION_COUNT];
    winid tmpwin;
    anything any;
    menu_item *picks = (menu_item *) 0;
    char mbuf[QBUFSZ];
    boolean showmenu = TRUE;
    boolean changed = FALSE;

    do {
        for (i = 0; i < CONDITION_COUNT; ++i)
            sequence[i] = i;
        qsort((genericptr_t) sequence, CONDITION_COUNT,
              sizeof sequence[0],
              (gc.condmenu_sortorder) ? cond_cmp : menualpha_cmp);

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);

        any = zeroany;
        any.a_int = 1;
        Sprintf(mbuf, "change sort order from \"%s\" to \"%s\"",
                menutitle[gc.condmenu_sortorder],
                menutitle[1 - gc.condmenu_sortorder]);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'S', 0, ATR_NONE,
                 mbuf, FALSE);
        any = zeroany;
        Sprintf(mbuf, "sorted %s", menutitle[gc.condmenu_sortorder]);
        add_menu_heading(tmpwin, mbuf);
        for (i = 0; i < SIZE(condtests); i++) {
            idx = sequence[i];
            Sprintf(mbuf, "cond_%-14s", condtests[idx].useroption);
            any = zeroany;
            any.a_int = idx + 2; /* avoid zero and the sort change pick */
            condtests[idx].choice = FALSE;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     mbuf, condtests[idx].enabled);
        }

        end_menu(tmpwin, "Choose status conditions to toggle");

        res = select_menu(tmpwin, PICK_ANY, &picks);
        destroy_nhwindow(tmpwin);
        showmenu = FALSE;
        if (res > 0) {
            for (i = 0; i < res; i++) {
                idx = picks[i].item.a_int;
                if (idx == 1) {
                   /* sort change requested */
                   gc.condmenu_sortorder = 1 - gc.condmenu_sortorder;
                   showmenu = TRUE;
                   break; /* for loop */
                } else {
                    idx -= 2;
                    condtests[idx].choice = TRUE;
                }
            }
            free((genericptr_t) picks);
        }
    } while (showmenu);

    if (res >= 0) {
        for (i = 0; i < CONDITION_COUNT; ++i)
            if (condtests[i].enabled != condtests[i].choice) {
                condtests[i].enabled = condtests[i].choice;
                condtests[idx].test = FALSE;
                flags.botl = changed = TRUE;
            }
    }
    return changed;
}

/* called by all_options_conds() to get value for next cond_xyz option
   so that #saveoptions can collect it and write the set into new RC file.
   returns zero-length string if the option is the default value. */
boolean
opt_next_cond(int indx, char *outbuf)
{
    *outbuf = '\0';
    if (indx >= CONDITION_COUNT) {
        return FALSE;
    }

    /*
     * The entries are returned in internal order which requires the
     * least code.  It would be easy to sort them into alphabetic order
     * (just sort all over again for every requested entry:
     *  int i, sequence[CONDITION_COUNT]
     *  for (i = 0; i < CONDITION_COUNT; ++i) sequence[i] = i;
     *  qsort(sequence, ..., menualpha_cmp);
     *  indx = sequence[indx];
     *  Sprintf(outbuf, ...);
     * with no need to hang on to 'sequence[]' between calls).
     *
     * But using 'severity order' isn't feasible unless the player has
     * used 'mO' on conditions in this session.  Even then, they would
     * revert to the default order (whether internal or alphabetical)
     * if #saveoptions got used in some later session where doset()
     * wasn't used to choose their preferred order.
     */

    if ((condtests[indx].opt == opt_in && condtests[indx].enabled)
        || (condtests[indx].opt == opt_out && !condtests[indx].enabled)) {
        Sprintf(outbuf, "%scond_%s", condtests[indx].enabled ? "" : "!",
                condtests[indx].useroption);
    }
    return TRUE;
}

static boolean
eval_notify_windowport_field(
    int fld,
    boolean *valsetlist,
    int idx)
{
    static int oldrndencode = 0;
#ifdef NEXT_VERSION
    static nhsym oldgoldsym = 0;
#endif
    int pc, chg, color = NO_COLOR;
    unsigned anytype;
    boolean updated = FALSE, reset;
    struct istat_s *curr, *prev;
    enum statusfields fldmax;

    /*
     *  Now pass the changed values to window port.
     */
    anytype = gb.blstats[idx][fld].anytype;
    curr = &gb.blstats[idx][fld];
    prev = &gb.blstats[1 - idx][fld];
    color = NO_COLOR;

    chg = gu.update_all ? 0 : compare_blstats(prev, curr);
    /*
     * TODO:
     *  Dynamically update 'percent_matters' as rules are added or
     *  removed to track whether any of them are percentage rules.
     *  Then there'll be no need to assume that non-Null 'thresholds'
     *  means that percentages need to be kept up to date.
     *  [Affects exp_percent_changing() too.]
     */
    if (((chg || gu.update_all || fld == BL_XP)
         && curr->percent_matters
#ifdef STATUS_HILITES
         && curr->thresholds
#endif
        )
        /* when 'hitpointbar' is On, percent matters even if HP
           hasn't changed and has no percentage rules (in case HPmax
           has changed when HP hasn't, where we ordinarily wouldn't
           update HP so would miss an update of the hitpoint bar) */
        || (fld == BL_HP && flags.hitpointbar)) {
        fldmax = curr->idxmax;
        pc = (fldmax == BL_EXP) ? exp_percentage()
              : (fldmax >= 0 && fldmax < MAXBLSTATS)
                 ? percentage(curr, &gb.blstats[idx][fldmax])
                 : 0; /* bullet proofing; can't get here */
        if (pc != prev->percent_value)
            chg = (pc < prev->percent_value) ? -1 : 1;
        curr->percent_value = pc;
    } else {
        pc = 0;
    }

    /* Temporary? hack: moveloop()'s prolog for a new game sets
     * svc.context.rndencode after the status window has been init'd,
     * so $:0 has already been encoded and cached by the window
     * port.  Without this hack, gold's \G sequence won't be
     * recognized and ends up being displayed as-is for 'gu.update_all'.
     *
     * Also, even if svc.context.rndencode hasn't changed and the
     * gold amount itself hasn't changed, the glyph portion of the
     * encoding may have changed if a new symset was put into effect.
     *
     *  \GXXXXNNNN:25
     *  XXXX = the svc.context.rndencode portion
     *  NNNN = the glyph portion
     *  25   = the gold amount
     *
     * Setting 'chg = 2' is enough to render the field properly, but
     * not to honor an initial highlight, so force 'gu.update_all = TRUE'.
     */
    if (fld == BL_GOLD
        && (flags.rndencode != oldrndencode
#ifdef NEXT_VERSION
            || gs.showsyms[COIN_CLASS + SYM_OFF_O] != oldgoldsym
#endif
            )) {
        gu.update_all = TRUE; /* chg = 2; */
        oldrndencode = flags.rndencode;
#ifdef NEXT_VERSION
        oldgoldsym = gs.showsyms[COIN_CLASS + SYM_OFF_O];
#endif
    }

    reset = FALSE;
#ifdef STATUS_HILITES
    if (gu.update_all) {
        chg = 0;
        curr->time = prev->time = 0L;
    } else if (!chg && curr->time) {
        reset = hilite_reset_needed(prev, gb.bl_hilite_moves);
        if (reset) {
            curr->time = prev->time = 0L;
        }
    }
#endif

   if (gu.update_all || chg || reset) {
        if (!valsetlist[fld]) {
            (void) anything_to_s(curr->val, &curr->a, anytype);
        }

        if (anytype != ANY_MASK32) {
#ifdef STATUS_HILITES
            if (chg || *curr->val) {
                /* if Xp percentage changed, we set 'chg' to 1 above;
                   reset that if the Xp value hasn't actually changed
                   or possibly went down rather than up (level loss) */
                if (chg == 1 && fld == BL_XP)
                    chg = compare_blstats(prev, curr);

                curr->hilite_rule = get_hilite(idx, fld,
                                               (genericptr_t) &curr->a,
                                               chg, pc, &color);
                prev->hilite_rule = curr->hilite_rule;
                if (chg == 2) {
                    color = NO_COLOR;
                    chg = 0;
                }
            }
#endif /* STATUS_HILITES */
#ifdef NEXT_VERSION
            status_update(fld, (genericptr_t) curr->val,
                          chg, pc, color, (unsigned long *) 0);
        } else {
            /* Color for conditions is done through gc.cond_hilites[] */
            status_update(fld, (genericptr_t) &curr->a.a_ulong,
                          chg, pc, color, gc.cond_hilites);
#endif
        }
        curr->chg = prev->chg = TRUE;
        updated = TRUE;
    }
    return updated;
}

static void
evaluate_and_notify_windowport(
    boolean *valsetlist,
    int idx)
{
    int i, fld, updated = 0;

    /*
     *  Now pass the changed values to window port.
     */
    for (i = 0; i < MAXBLSTATS; i++) {
        fld = initblstats[i].fld;
        if (((fld == BL_SCORE) && !flags.showscore)
            || ((fld == BL_EXP) && !flags.showexp)
            || ((fld == BL_TIME) && !flags.time)
            || ((fld == BL_HD) && !Upolyd)
            || ((fld == BL_XP || i == BL_EXP) && Upolyd)
#ifdef NEXT_VERSION
            || ((fld == BL_VERS) && !flags.showvers)
#endif
            ) {
            continue;
        }
        if (eval_notify_windowport_field(fld, valsetlist, idx)) {
            updated++;
        }
    }
    /*
     * Notes:
     *  1. It is possible to get here, with nothing having been pushed
     *     to the window port, when none of the info has changed.
     *
     *  2. Some window ports are also known to optimize by only drawing
     *     fields that have changed since the previous update.
     *
     * In both of those situations, we need to force updates to
     * all of the fields when disp.botlx is set. The tty port in
     * particular has a problem if that isn't done, since the core sets
     * disp.botlx when a menu or text display obliterates the status
     * line.
     *
     * For those situations, to trigger the full update of every field
     * whether changed or not, call status_update() with BL_RESET.
     *
     * For regular processing and to notify the window port that a
     * bot() round has finished and it's time to trigger a flush of
     * all buffered changes received thus far but not reflected in
     * the display, call status_update() with BL_FLUSH.
     *
     */
#ifdef NEXT_VERSION
    if (disp.botlx && (windowprocs.wincap2 & WC2_RESET_STATUS) != 0L) {
        status_update(BL_RESET, (genericptr_t) 0, 0, 0,
                      NO_COLOR, (unsigned long *) 0);
    } else if ((updated || disp.botlx)
             && (windowprocs.wincap2 & WC2_FLUSH_STATUS) != 0L) {
        status_update(BL_FLUSH, (genericptr_t) 0, 0, 0,
                      NO_COLOR, (unsigned long *) 0);
    }
#endif

    flags.botl = flags.botlx = disp.time_botl = FALSE;
    gu.update_all = FALSE;
}

void
status_initialize(
    boolean reassessment) /* True: just recheck fields without other init */
{
#ifdef NEXT_VERSION
    enum statusfields fld;
    boolean fldenabl;
    int i;
    const char *fieldfmt, *fieldname;
#endif

    if (!reassessment) {
        if (gb.blinit) {
            impossible("2nd status_initialize with full init.");
        }
        init_blstats();
#ifdef NEXT_VERSION
        (*windowprocs.win_status_init)();
#endif
        gb.blinit = TRUE;
    } else if (!gb.blinit) {
        panic("status 'reassess' before init");
    }
#ifdef NEXT_VERSION
    for (i = 0; i < MAXBLSTATS; ++i) {
        fld = initblstats[i].fld;
        fldenabl = (fld == BL_SCORE) ? flags.showscore
                 : (fld == BL_TIME) ? flags.time
                 : (fld == BL_EXP) ? (boolean) (flags.showexp && !Upolyd)
                 : (fld == BL_XP) ? (boolean) !Upolyd
                 : (fld == BL_HD) ? (boolean) Upolyd
                 : (fld == BL_VERS) ? flags.showvers
                 : TRUE;

        fieldname = initblstats[i].fldname;
        fieldfmt = (fld == BL_TITLE && iflags.wc2_hitpointbar) ? "%-30.30s"
                   : initblstats[i].fldfmt;
        status_enablefield(fld, fieldname, fieldfmt, fldenabl);
    }
#endif
    gu.update_all = TRUE;
    flags.botlx = TRUE;
}

void
status_finish(void)
{
    int i;

#ifdef NEXT_VERSION
    /* call the window port cleanup routine first */
    if (windowprocs.win_status_finish) {
        (*windowprocs.win_status_finish)();
    }
#endif

    /* free memory that we alloc'd now */
    for (i = 0; i < MAXBLSTATS; ++i) {
        if (gb.blstats[0][i].val) {
            free((genericptr_t) gb.blstats[0][i].val),
                gb.blstats[0][i].val = (char *) NULL;
        }
        if (gb.blstats[1][i].val) {
            free((genericptr_t) gb.blstats[1][i].val),
                gb.blstats[1][i].val = (char *) NULL;
        }
#ifdef STATUS_HILITES
        /* pointer to an entry in thresholds list; Null it out since
           that list is about to go away */
        gb.blstats[0][i].hilite_rule = gb.blstats[1][i].hilite_rule = 0;
        if (gb.blstats[0][i].thresholds) {
            struct hilite_s *temp, *next;

            for (temp = gb.blstats[0][i].thresholds; temp; temp = next) {
                next = temp->next;
                free((genericptr_t) temp);
            }
            gb.blstats[0][i].thresholds
                = gb.blstats[1][i].thresholds
                    = (struct hilite_s *) NULL;
        }
#endif /* STATUS_HILITES */
    }
}

static void
init_blstats(void)
{
    static boolean initalready = FALSE;
    int i, j;

    if (initalready) {
        impossible("init_blstats called more than once.");
        return;
    }
    for (i = 0; i <= 1; ++i) {
        for (j = 0; j < MAXBLSTATS; ++j) {
#ifdef STATUS_HILITES
            struct hilite_s *keep_hilite_chain = gb.blstats[i][j].thresholds;
#endif
            gb.blstats[i][j] = initblstats[j];
            gb.blstats[i][j].a = zeroany;
            if (gb.blstats[i][j].valwidth) {
                gb.blstats[i][j].val = (char *) alloc(gb.blstats[i][j].valwidth);
                gb.blstats[i][j].val[0] = '\0';
            } else {
                gb.blstats[i][j].val = (char *) 0;
            }
#ifdef STATUS_HILITES
            gb.blstats[i][j].thresholds = keep_hilite_chain;
#endif
        }
    }
    initalready = TRUE;
}

/*
 * This compares the previous stat with the current stat,
 * and returns one of the following results based on that:
 *
 *   if prev_value < new_value (stat went up, increased)
 *      return 1
 *
 *   if prev_value > new_value (stat went down, decreased)
 *      return  -1
 *
 *   if prev_value == new_value (stat stayed the same)
 *      return 0
 *
 *   Special cases:
 *     - for bitmasks, 0 = stayed the same, 1 = changed
 *     - for strings,  0 = stayed the same, 1 = changed
 *
 */
static int
compare_blstats(struct istat_s *bl1, struct istat_s *bl2)
{
    anything *a1, *a2;
    boolean use_rawval;
    int anytype, fld, result = 0;

    if (!bl1 || !bl2) {
        panic("compare_blstat: bad istat pointer %s, %s",
              fmt_ptr((genericptr_t) bl1), fmt_ptr((genericptr_t) bl2));
    }

    anytype = bl1->anytype;
    if ((!bl1->a.a_void || !bl2->a.a_void)
        && (anytype == ANY_IPTR || anytype == ANY_UPTR
            || anytype == ANY_LPTR || anytype == ANY_ULPTR)) {
        panic("compare_blstat: invalid pointer %s, %s",
              fmt_ptr((genericptr_t) bl1->a.a_void),
              fmt_ptr((genericptr_t) bl2->a.a_void));
    }

    fld = bl1->fld;
    use_rawval = (fld == BL_HP || fld == BL_HPMAX
                  || fld == BL_ENE || fld == BL_ENEMAX
                  || fld == BL_GOLD);
    a1 = use_rawval ? &bl1->rawval : &bl1->a;
    a2 = use_rawval ? &bl2->rawval : &bl2->a;

    switch (anytype) {
    case ANY_INT:
        result = (a1->a_int < a2->a_int) ? 1
               : (a1->a_int > a2->a_int) ? -1 : 0;
        break;
    case ANY_IPTR:
        result = (*a1->a_iptr < *a2->a_iptr) ? 1
               : (*a1->a_iptr > *a2->a_iptr) ? -1 : 0;
        break;
    case ANY_LONG:
        result = (a1->a_long < a2->a_long) ? 1
               : (a1->a_long > a2->a_long) ? -1 : 0;
        break;
    case ANY_LPTR:
        result = (*a1->a_lptr < *a2->a_lptr) ? 1
               : (*a1->a_lptr > *a2->a_lptr) ? -1 : 0;
        break;
    case ANY_UINT:
        result = (a1->a_uint < a2->a_uint) ? 1
               : (a1->a_uint > a2->a_uint) ? -1 : 0;
        break;
    case ANY_UPTR:
        result = (*a1->a_uptr < *a2->a_uptr) ? 1
               : (*a1->a_uptr > *a2->a_uptr) ? -1 : 0;
        break;
    case ANY_ULONG:
        result = (a1->a_ulong < a2->a_ulong) ? 1
               : (a1->a_ulong > a2->a_ulong) ? -1 : 0;
        break;
    case ANY_ULPTR:
        result = (*a1->a_ulptr < *a2->a_ulptr) ? 1
               : (*a1->a_ulptr > *a2->a_ulptr) ? -1 : 0;
        break;
    case ANY_STR:
        result = sgn(strcmp(bl1->val, bl2->val));
        break;
    case ANY_MASK32:
        result = (a1->a_ulong != a2->a_ulong);
        break;
    default:
        result = 1;
    }
    return result;
}

__attribute__((unused))
static char *
anything_to_s(char *buf, anything *a, int anytype)
{
    if (!buf) {
        return (char *) 0;
    }

    switch (anytype) {
    case ANY_ULONG:
        Sprintf(buf, "%lu", a->a_ulong);
        break;
    case ANY_MASK32:
        Sprintf(buf, "%lx", a->a_ulong);
        break;
    case ANY_LONG:
        Sprintf(buf, "%ld", a->a_long);
        break;
    case ANY_INT:
        Sprintf(buf, "%d", a->a_int);
        break;
    case ANY_UINT:
        Sprintf(buf, "%u", a->a_uint);
        break;
    case ANY_IPTR:
        Sprintf(buf, "%d", *a->a_iptr);
        break;
    case ANY_LPTR:
        Sprintf(buf, "%ld", *a->a_lptr);
        break;
    case ANY_ULPTR:
        Sprintf(buf, "%lu", *a->a_ulptr);
        break;
    case ANY_UPTR:
        Sprintf(buf, "%u", *a->a_uptr);
        break;
    case ANY_STR: /* do nothing */
        ;
        break;
    default:
        buf[0] = '\0';
    }
    return buf;
}

#ifdef STATUS_HILITES
static void
s_to_anything(anything *a, char *buf, int anytype)
{
    if (!buf || !a) {
        return;
    }

    switch (anytype) {
    case ANY_LONG:
        a->a_long = atol(buf);
        break;
    case ANY_INT:
        a->a_int = atoi(buf);
        break;
    case ANY_UINT:
        a->a_uint = (unsigned) atoi(buf);
        break;
    case ANY_ULONG:
        a->a_ulong = (unsigned long) atol(buf);
        break;
    case ANY_IPTR:
        if (a->a_iptr)
            *a->a_iptr = atoi(buf);
        break;
    case ANY_UPTR:
        if (a->a_uptr)
            *a->a_uptr = (unsigned) atoi(buf);
        break;
    case ANY_LPTR:
        if (a->a_lptr)
            *a->a_lptr = atol(buf);
        break;
    case ANY_ULPTR:
        if (a->a_ulptr)
            *a->a_ulptr = (unsigned long) atol(buf);
        break;
    case ANY_MASK32:
        a->a_ulong = (unsigned long) atol(buf);
        break;
    default:
        a->a_void = 0;
        break;
    }
    return;
}
#endif /* STATUS_HILITES */

/* integer percentage is 100 * bl->a / maxbl->a */
static int
percentage(struct istat_s *bl, struct istat_s *maxbl)
{
    int result = 0;
    int anytype;
    int ival, mval;
    long lval;
    unsigned uval;
    unsigned long ulval;
    int fld;
    boolean use_rawval;

    if (!bl || !maxbl) {
        impossible("percentage: bad istat pointer %s, %s",
                   fmt_ptr((genericptr_t) bl), fmt_ptr((genericptr_t) maxbl));
        return 0;
    }

    fld = bl->fld;
    use_rawval = (fld == BL_HP || fld == BL_ENE);
    ival = 0, lval = 0L, uval = 0U, ulval = 0UL;
    anytype = bl->anytype;
    if (maxbl->a.a_void) {
        switch (anytype) {
        case ANY_INT:
            /* HP and energy are int so this is the only case that cares
               about 'rawval'; for them, we use that rather than their
               potentially truncated (to 9999) display value */
            ival = use_rawval ? bl->rawval.a_int : bl->a.a_int;
            mval = use_rawval ? maxbl->rawval.a_int : maxbl->a.a_int;
            result = ((100 * ival) / mval);
            break;
        case ANY_LONG:
            lval  = bl->a.a_long;
            result = (int) ((100L * lval) / maxbl->a.a_long);
            break;
        case ANY_UINT:
            uval = bl->a.a_uint;
            result = (int) ((100U * uval) / maxbl->a.a_uint);
            break;
        case ANY_ULONG:
            ulval = bl->a.a_ulong;
            result = (int) ((100UL * ulval) / maxbl->a.a_ulong);
            break;
        case ANY_IPTR:
            ival = *bl->a.a_iptr;
            result = ((100 * ival) / (*maxbl->a.a_iptr));
            break;
        case ANY_LPTR:
            lval = *bl->a.a_lptr;
            result = (int) ((100L * lval) / (*maxbl->a.a_lptr));
            break;
        case ANY_UPTR:
            uval = *bl->a.a_uptr;
            result = (int) ((100U * uval) / (*maxbl->a.a_uptr));
            break;
        case ANY_ULPTR:
            ulval = *bl->a.a_ulptr;
            result = (int) ((100UL * ulval) / (*maxbl->a.a_ulptr));
            break;
        }
    }
    /* don't let truncation from integer division produce a zero result
       from a non-zero input; note: if we ever change to something like
       ((((1000 * val) / max) + 5) / 10) for a rounded result, we'll
       also need to check for and convert false 100 to 99 */
    if (result == 0
        && (ival != 0 || lval != 0L || uval != 0U || ulval != 0UL)) {
        result = 1;
    }

    return result;
}

/* percentage for both xp (level) and exp (points) is the percentage for
   (curr_exp - this_level_start) in (next_level_start - this_level_start) */
static int
exp_percentage(void)
{
    int res = 0;

    if (u.ulevel < 30) {
        long exp_val, nxt_exp_val, curlvlstart;

        curlvlstart = newuexp(u.ulevel - 1);
        exp_val = u.uexp - curlvlstart;
        nxt_exp_val = newuexp(u.ulevel) - curlvlstart;
        if (exp_val == nxt_exp_val - 1L) {
            /*
             * Full 100% is unattainable since hero gains a level
             * and the threshold for next level increases, but treat
             * (next_level_start - 1 point) as a special case.  It's a
             * key value after being level drained so is something that
             * some players would like to be able to highlight distinctly.
             */
            res = 100;
        } else {
            struct istat_s curval, maxval;

            curval.anytype = maxval.anytype = ANY_LONG;
            curval.a = maxval.a = cg.zeroany;
            curval.a.a_long = exp_val;
            maxval.a.a_long = nxt_exp_val;
            curval.fld = maxval.fld = BL_EXP; /* (neither BL_HP nor BL_ENE) */
            /* maximum delta between levels is 10000000; calculation of
               100 * (10000000 - N) / 10000000 fits within 32-bit long */
            res = percentage(&curval, &maxval);
        }
    }
    return res;
}

#ifdef STATUS_HILITES

/****************************************************************************/
/* Core status hiliting support */
/****************************************************************************/

static const struct fieldid_t {
    const char *fieldname;
    enum statusfields fldid;
} fieldids_alias[] = {
    { "characteristics",   BL_CHARACTERISTICS },
    { "encumbrance",       BL_CAP },
    { "experience-points", BL_EXP },
    { "dx",       BL_DX },
    { "co",       BL_CO },
    { "con",      BL_CO },
    { "points",   BL_SCORE },
    { "cap",      BL_CAP },
    { "pw",       BL_ENE },
    { "pw-max",   BL_ENEMAX },
    { "xl",       BL_XP },
    { "xplvl",    BL_XP },
    { "ac",       BL_AC },
    { "hit-dice", BL_HD },
    { "turns",    BL_TIME },
    { "hp",       BL_HP },
    { "hp-max",   BL_HPMAX },
    { "dgn",      BL_LEVELDESC },
    { "xp",       BL_EXP },
    { "exp",      BL_EXP },
    { "flags",    BL_CONDITION },
    { NULL,       BL_FLUSH }
};

/* format arguments */
static const char threshold_value[] = "hilite_status threshold ",
                  is_out_of_range[] = " is out of range";

/* functions from other files used by STATUS_HILITES */
#define config_error_add raw_printf
extern int match_str2clr(char *);
extern int match_str2attr(const char *, boolean);
extern boolean critically_low_hp(boolean);

/* field name to bottom line index */
static enum statusfields
fldname_to_bl_indx(const char *name)
{
    int i, nmatches = 0, fld = 0;

    if (name && *name) {
        /* check matches to canonical names */
        for (i = 0; i < SIZE(initblstats); i++)
            if (fuzzymatch(initblstats[i].fldname, name, " -_", TRUE)) {
                fld = initblstats[i].fld;
                nmatches++;
            }
        if (!nmatches) {
            /* check aliases */
            for (i = 0; fieldids_alias[i].fieldname; i++)
                if (fuzzymatch(fieldids_alias[i].fieldname, name,
                               " -_", TRUE)) {
                    fld = fieldids_alias[i].fldid;
                    nmatches++;
                }
        }
        if (!nmatches) {
            /* check partial matches to canonical names */
            int len = (int) strlen(name);
            for (i = 0; i < SIZE(initblstats); i++)
                if (!strncmpi(name, initblstats[i].fldname, len)) {
                    fld = initblstats[i].fld;
                    nmatches++;
                }
        }
    }
    return (nmatches == 1) ? fld : BL_FLUSH;
}

static boolean
hilite_reset_needed(
    struct istat_s *bl_p,
    long augmented_time) /* no longer augmented; it once encoded fractional
                          * amounts for multiple moves within same turn */
{
    /*
     * This 'multi' handling may need some tuning...
     */
    if (multi) {
        return FALSE;
    }

    if (!Is_Temp_Hilite(bl_p->hilite_rule)) {
        return FALSE;
    }

    if (bl_p->time == 0 || bl_p->time >= augmented_time) {
        return FALSE;
    }

    return TRUE;
}

/* called by options handling when 'statushilites' value is changed */
void
reset_status_hilites(void)
{
    if (iflags.hilite_delta) {
        int i;
        for (i = 0; i < MAXBLSTATS; ++i) {
            gb.blstats[0][i].time = gb.blstats[1][i].time = 0L;
        }
    }
    flags.botlx = TRUE;
}

/* test whether the text from a title rule matches the string for
   title-while-polymorphed in the 'textmatch' menu */
static boolean
noneoftheabove(const char *hl_text)
{
    if (fuzzymatch(hl_text, "none of the above", "\" -_", TRUE)
        || fuzzymatch(hl_text, "(polymorphed)", "\"()", TRUE)
        || fuzzymatch(hl_text, "none of the above (polymorphed)",
                      "\" -_()", TRUE)) {
        return TRUE;
    }

    return FALSE;
}

/*
 * get_hilite
 *
 * Returns, based on the value and the direction it is moving,
 * the highlight rule that applies to the specified field.
 *
 * Provide get_hilite() with the following to work with:
 *     actual value vp
 *          useful for BL_TH_VAL_ABSOLUTE
 *     indicator of down, up, or the same (-1, 1, 0) chg
 *          useful for BL_TH_UPDOWN or change detection
 *     percentage (current value percentage of max value) pc
 *          useful for BL_TH_VAL_PERCENTAGE
 *
 * Get back:
 *     pointer to rule that applies; Null if no rule does.
 */
static struct hilite_s *
get_hilite(
    int idx, int fldidx,
    genericptr_t vp,
    int chg, int pc,
    int *colorptr)
{
    struct hilite_s *hl, *rule = 0;
    anything *value = (anything *) vp;
    char *txtstr;

    if (fldidx < 0 || fldidx >= MAXBLSTATS) {
        return (struct hilite_s *) 0;
    }

    if (has_hilite(fldidx)) {
        int dt;
        /* there are hilites set here */
        int max_pc = -1, min_pc = 101;
        /* LARGEST_INT isn't INT_MAX; it fits within 16 bits, but that
           value is big enough to handle all 'int' status fields */
        int max_ival = -LARGEST_INT, min_ival = LARGEST_INT;
        /* LONG_MAX comes from <limits.h> which might not be available for
           ancient configurations; we don't need LONG_MIN */
        long max_lval = -LONG_MAX, min_lval = LONG_MAX;
        boolean exactmatch = FALSE, updown = FALSE, changed = FALSE,
                perc_or_abs = FALSE, crit_hp = FALSE;

        /* min_/max_ are used to track best fit */
        for (hl = gb.blstats[0][fldidx].thresholds; hl; hl = hl->next) {
            dt = initblstats[fldidx].anytype; /* only needed for 'absolute' */
            /* for HP, if we already have a critical-hp rule then we ignore
               other HP rules unless we hit another critical-hp one (last
               one found wins); critical-hp takes precedence over temporary
               HP highlights, otherwise a hero with regeneration and an up
               or changed rule for HP would always show that up or changed
               highlight even when within the critical-hp threshold because
               the value will go up by at least one on every move */
            if (crit_hp && hl->behavior != BL_TH_CRITICALHP) {
                continue;
            }
            /* if we've already matched a temporary highlight, it takes
               precedence over all persistent ones; we still process
               updown rules to get the last one which qualifies */
            if ((updown || changed) && hl->behavior != BL_TH_UPDOWN) {
                continue;
            }
            /* among persistent highlights, if a 'percentage' or 'absolute'
               rule has been matched, it takes precedence over 'always' */
            if (perc_or_abs && hl->behavior == BL_TH_ALWAYS_HILITE) {
                continue;
            }

            switch (hl->behavior) {
            case BL_TH_VAL_PERCENTAGE: /* percent values are always ANY_INT */
                if (hl->rel == EQ_VALUE && pc == hl->value.a_int) {
                    rule = hl;
                    min_pc = max_pc = hl->value.a_int;
                    exactmatch = perc_or_abs = TRUE;
                } else if (exactmatch) {
                    ; /* already found best fit, skip lt,ge,&c */
                } else if (hl->rel == LT_VALUE
                           && (pc < hl->value.a_int)
                           && (hl->value.a_int <= min_pc)) {
                    rule = hl;
                    min_pc = hl->value.a_int;
                    perc_or_abs = TRUE;
                } else if (hl->rel == LE_VALUE
                           && (pc <= hl->value.a_int)
                           && (hl->value.a_int <= min_pc)) {
                    rule = hl;
                    min_pc = hl->value.a_int;
                    perc_or_abs = TRUE;
                } else if (hl->rel == GT_VALUE
                           && (pc > hl->value.a_int)
                           && (hl->value.a_int >= max_pc)) {
                    rule = hl;
                    max_pc = hl->value.a_int;
                    perc_or_abs = TRUE;
                } else if (hl->rel == GE_VALUE
                           && (pc >= hl->value.a_int)
                           && (hl->value.a_int >= max_pc)) {
                    rule = hl;
                    max_pc = hl->value.a_int;
                    perc_or_abs = TRUE;
                }
                break;
            case BL_TH_UPDOWN: /* uses 'chg' (set by caller), not 'dt' */
                /* specific 'up' or 'down' takes precedence over general
                   'changed' regardless of their order in the rule set */
                if (chg < 0 && hl->rel == LT_VALUE) {
                    rule = hl;
                    updown = TRUE;
                } else if (chg > 0 && hl->rel == GT_VALUE) {
                    rule = hl;
                    updown = TRUE;
                } else if (chg != 0 && hl->rel == EQ_VALUE && !updown) {
                    rule = hl;
                    changed = TRUE;
                }
                break;
            case BL_TH_VAL_ABSOLUTE: /* either ANY_INT or ANY_LONG */
                /*
                 * The int and long variations here are identical aside from
                 * union field and min_/max_ variable names.  If you change
                 * one, be sure to make a corresponding change in the other.
                 */
                if (dt == ANY_INT) {
                    if (hl->rel == EQ_VALUE
                        && hl->value.a_int == value->a_int) {
                        rule = hl;
                        min_ival = max_ival = hl->value.a_int;
                        exactmatch = perc_or_abs = TRUE;
                    } else if (exactmatch) {
                        ; /* already found best fit, skip lt,ge,&c */
                    } else if (hl->rel == LT_VALUE
                               && (value->a_int < hl->value.a_int)
                               && (hl->value.a_int <= min_ival)) {
                        rule = hl;
                        min_ival = hl->value.a_int;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == LE_VALUE
                               && (value->a_int <= hl->value.a_int)
                               && (hl->value.a_int <= min_ival)) {
                        rule = hl;
                        min_ival = hl->value.a_int;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == GT_VALUE
                               && (value->a_int > hl->value.a_int)
                               && (hl->value.a_int >= max_ival)) {
                        rule = hl;
                        max_ival = hl->value.a_int;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == GE_VALUE
                               && (value->a_int >= hl->value.a_int)
                               && (hl->value.a_int >= max_ival)) {
                        rule = hl;
                        max_ival = hl->value.a_int;
                        perc_or_abs = TRUE;
                    }
                } else { /* ANY_LONG */
                    if (hl->rel == EQ_VALUE
                        && hl->value.a_long == value->a_long) {
                        rule = hl;
                        min_lval = max_lval = hl->value.a_long;
                        exactmatch = perc_or_abs = TRUE;
                    } else if (exactmatch) {
                        ; /* already found best fit, skip lt,ge,&c */
                    } else if (hl->rel == LT_VALUE
                               && (value->a_long < hl->value.a_long)
                               && (hl->value.a_long <= min_lval)) {
                        rule = hl;
                        min_lval = hl->value.a_long;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == LE_VALUE
                               && (value->a_long <= hl->value.a_long)
                               && (hl->value.a_long <= min_lval)) {
                        rule = hl;
                        min_lval = hl->value.a_long;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == GT_VALUE
                               && (value->a_long > hl->value.a_long)
                               && (hl->value.a_long >= max_lval)) {
                        rule = hl;
                        max_lval = hl->value.a_long;
                        perc_or_abs = TRUE;
                    } else if (hl->rel == GE_VALUE
                               && (value->a_long >= hl->value.a_long)
                               && (hl->value.a_long >= max_lval)) {
                        rule = hl;
                        max_lval = hl->value.a_long;
                        perc_or_abs = TRUE;
                    }
                }
                break;
            case BL_TH_TEXTMATCH: /* ANY_STR */
                txtstr = gb.blstats[idx][fldidx].val;
                if (fldidx == BL_TITLE) {
                    /* "<name> the <rank-title>", skip past "<name> the " */
                    txtstr += strlen(plname) + sizeof " the " - sizeof "";
                }
                if (hl->rel == TXT_VALUE && hl->textmatch[0]) {
                    if (fuzzymatch(hl->textmatch, txtstr, "\" -_", TRUE)) {
                        rule = hl;
                        exactmatch = TRUE;
                    } else if (exactmatch) {
                        ; /* already found best fit, skip "noneoftheabove" */
                    } else if (fldidx == BL_TITLE
                               && Upolyd && noneoftheabove(hl->textmatch)) {
                        rule = hl;
                    }
                }
                break;
            case BL_TH_ALWAYS_HILITE:
                rule = hl;
                break;
            case BL_TH_CRITICALHP:
                if (fldidx == BL_HP && critically_low_hp(FALSE)) {
                    rule = hl;
                    crit_hp = TRUE;
                    updown = changed = perc_or_abs = FALSE;
                }
                break;
            case BL_TH_NONE:
            default:
                break;
            }
        }
    }
    *colorptr = rule ? rule->coloridx : NO_COLOR;
    return rule;
}

/* ======================================================================= */
/*  STATUS_HILITES helper functions for unnethack                          */
/* ======================================================================= */

/*
 * get_hilite_color - evaluate hilite rules for a field and return
 * a color_option suitable for use with start_color_option/end_color_option.
 * Returns {NO_COLOR, 0} if no hilite applies.
 */
struct color_option
get_hilite_color(enum statusfields fld)
{
    struct color_option result = {NO_COLOR, 0};
    int coloridx = NO_COLOR;
    int idx = gn.now_or_before_idx;
    int prev_idx = 1 - idx;
    int chg, pc;

    if (!iflags.hilite_delta || fld < 0 || fld >= MAXBLSTATS) {
        return result;
    }

    if (!has_hilite(fld)) {
        return result;
    }

    chg = compare_blstats(&gb.blstats[prev_idx][fld], &gb.blstats[idx][fld]);

    /* Extend temporary (up/down/changed) hilites for 'statushilites'
       turns past the transition: while the remembered expiry window
       is still active for this field, keep reporting the change so
       the matching BL_TH_UPDOWN rule stays selected. */
    if (hilite_until_moves[fld] != 0L && moves <= hilite_until_moves[fld]) {
        if (chg == 0 && hilite_sign[fld] != 0) {
            chg = hilite_sign[fld];
        }
    } else if (hilite_until_moves[fld] != 0L) {
        hilite_until_moves[fld] = 0L;
        hilite_sign[fld] = 0;
    }

    /* compute percentage if applicable */
    pc = 0;
    if (gb.blstats[idx][fld].percent_matters && initblstats[fld].idxmax >= 0) {
        int fldmax = initblstats[fld].idxmax;
        if (fldmax == BL_EXP) {
            pc = exp_percentage();
        } else if (fldmax >= 0 && fldmax < MAXBLSTATS) {
            pc = percentage(&gb.blstats[idx][fld], &gb.blstats[idx][fldmax]);
        }
    }

    (void) get_hilite(idx, fld, (genericptr_t) &gb.blstats[idx][fld].a,
                      chg, pc, &coloridx);

    if (coloridx != NO_COLOR) {
        int clr, attr;
        split_clridx(coloridx, &clr, &attr);
        result.color = clr;
        /* convert HL_* attributes to attr_bits */
        if (attr & HL_BOLD) {
            result.attr_bits |= (1 << ATR_BOLD);
        }
        if (attr & HL_DIM) {
            result.attr_bits |= (1 << ATR_DIM);
        }
        if (attr & HL_ITALIC) {
            result.attr_bits |= (1 << ATR_ITALIC);
        }
        if (attr & HL_INVERSE) {
            result.attr_bits |= (1 << ATR_INVERSE);
        }
        if (attr & HL_BLINK) {
            result.attr_bits |= (1 << ATR_BLINK);
        }
#ifdef ATR_ULINE
        if (attr & HL_ULINE) {
            result.attr_bits |= (1 << ATR_ULINE);
        }
#endif
    }
    return result;
}

/*
 * get_condition_hilite_color - check condition hilites for a given
 * condition text. Returns a color_option.
 */
struct color_option
get_condition_hilite_color(const char *condtext)
{
    struct color_option result = {NO_COLOR, 0};
    int i;

    if (!iflags.hilite_delta) {
        return result;
    }

    for (i = 0; i < SIZE(conditions); i++) {
        if (fuzzymatch(conditions[i].text[0], condtext, " -_", TRUE)
            || fuzzymatch(conditions[i].text[1], condtext, " -_", TRUE)) {
            unsigned long mask = conditions[i].mask;
            int j;

            /* check color */
            for (j = 0; j < CLR_MAX; j++) {
                if (gc.cond_hilites[j] & mask) {
                    result.color = j;
                    break;
                }
            }
            /* check attributes */
            if (gc.cond_hilites[HL_ATTCLR_BOLD] & mask) {
                result.attr_bits |= (1 << ATR_BOLD);
            }
            if (gc.cond_hilites[HL_ATTCLR_DIM] & mask) {
                result.attr_bits |= (1 << ATR_DIM);
            }
            if (gc.cond_hilites[HL_ATTCLR_ITALIC] & mask) {
                result.attr_bits |= (1 << ATR_ITALIC);
            }
            if (gc.cond_hilites[HL_ATTCLR_INVERSE] & mask) {
                result.attr_bits |= (1 << ATR_INVERSE);
            }
            if (gc.cond_hilites[HL_ATTCLR_BLINK] & mask) {
                result.attr_bits |= (1 << ATR_BLINK);
            }
#ifdef ATR_ULINE
            if (gc.cond_hilites[HL_ATTCLR_ULINE] & mask) {
                result.attr_bits |= (1 << ATR_ULINE);
            }
#endif
            break;
        }
    }
    return result;
}

#undef has_hilite
#undef Is_Temp_Hilite

static void
split_clridx(int idx, int *coloridx, int *attrib)
{
    if (coloridx) {
        *coloridx = idx & 0x00FF;
    }
    if (attrib) {
        *attrib = (idx >> 8) & 0x00FF;
    }
}

/*
 * This is the parser for the hilite options.
 *
 * parse_status_hl1() separates each hilite entry into
 * a set of field threshold/action component strings,
 * then calls parse_status_hl2() to parse further
 * and configure the hilite.
 */
boolean
parse_status_hl1(char *op, boolean from_configfile)
{
#define MAX_THRESH 21
    char hsbuf[MAX_THRESH][QBUFSZ];
    boolean rslt, badopt = FALSE;
    int i, fldnum, ccount = 0;
    char c;

    fldnum = 0;
    for (i = 0; i < MAX_THRESH; ++i) {
        hsbuf[i][0] = '\0';
    }
    while (*op && fldnum < MAX_THRESH && ccount < (QBUFSZ - 2)) {
        c = lowc(*op);
        if (c == ' ') {
            if (fldnum >= 1) {
                if (fldnum == 1 && strcmpi(hsbuf[0], "title") == 0) {
                    /* spaces are allowed in title */
                    hsbuf[fldnum][ccount++] = c;
                    hsbuf[fldnum][ccount] = '\0';
                    op++;
                    continue;
                }
                rslt = parse_status_hl2(hsbuf, from_configfile);
                if (!rslt) {
                    badopt = TRUE;
                    break;
                }
            }
            for (i = 0; i < MAX_THRESH; ++i) {
                hsbuf[i][0] = '\0';
            }
            fldnum = 0;
            ccount = 0;
        } else if (c == '/') {
            fldnum++;
            ccount = 0;
        } else {
            hsbuf[fldnum][ccount++] = c;
            hsbuf[fldnum][ccount] = '\0';
        }
        op++;
    }
    if (fldnum >= 1 && !badopt) {
        rslt = parse_status_hl2(hsbuf, from_configfile);
        if (!rslt) {
            badopt = TRUE;
        }
    }
    if (badopt) {
        return FALSE;
    }
    /* make sure highlighting is On; use short duration for temp highlights */
    if (!iflags.hilite_delta) {
        iflags.hilite_delta = 3L;
    }
    return TRUE;
#undef MAX_THRESH
}

/* is str in the format of "[<>]?=?[-+]?[0-9]+%?" regex */
static boolean
is_ltgt_percentnumber(const char *str)
{
    const char *s = str;
    if (*s == '<' || *s == '>') {
        s++;
    }
    if (*s == '=') {
        s++;
    }
    if (*s == '-' || *s == '+') {
        s++;
    }
    if (!digit(*s)) {
        return FALSE;
    }
    while (digit(*s)) {
        s++;
    }
    if (*s == '%') {
        s++;
    }
    return (*s == '\0');
}

/* does str only contain "<>=-+0-9%" chars */
static boolean
has_ltgt_percentnumber(const char *str)
{
    const char *s = str;
    while (*s) {
        if (!strchr("<>=-+0123456789%", *s)) {
            return FALSE;
        }
        s++;
    }
    return TRUE;
}

/* splitsubfields(): splits str in place into '+' or '&' separated strings.
   returns number of strings, or -1 if more than maxsf or MAX_SUBFIELDS */
static int
splitsubfields(char *str, char ***sfarr, int maxsf)
{
#define MAX_SUBFIELDS 16
    static char *subfields[MAX_SUBFIELDS];
    char *st = (char *) 0;
    int sf = 0;

    if (!str) {
        return 0;
    }
    for (sf = 0; sf < MAX_SUBFIELDS; ++sf) {
        subfields[sf] = (char *) 0;
    }

    maxsf = (maxsf == 0) ? MAX_SUBFIELDS : min(maxsf, MAX_SUBFIELDS);

    if (strchr(str, '+') || strchr(str, '&')) {
        char *c = str;
        sf = 0;
        st = c;
        while (*c && sf < maxsf) {
            if (*c == '&' || *c == '+') {
                *c = '\0';
                subfields[sf] = st;
                st = c + 1;
                sf++;
            }
            c++;
        }
        if (sf >= maxsf - 1) {
            return -1;
        }
        if (!*c && c != st) {
            subfields[sf++] = st;
        }
    } else {
        sf = 1;
        subfields[0] = str;
    }
    *sfarr = subfields;

    return sf;
#undef MAX_SUBFIELDS
}

static boolean
is_fld_arrayvalues(
    const char *str,
    const char *const *arr,
    int arrmin, int arrmax,
    int *retidx)
{
    int i;
    for (i = arrmin; i < arrmax; i++) {
        if (!strcmpi(str, arr[i])) {
            *retidx = i;
            return TRUE;
        }
    }

    return FALSE;
}

static int
query_arrayvalue(
    const char *querystr,
    const char *const *arr,
    int arrmin, int arrmax)
{
    int i, res, ret = arrmin - 1;
    winid tmpwin;
    anything any;
    menu_item *picks = (menu_item *) 0;
    int adj = (arrmin > 0) ? 1 : arrmax;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    for (i = arrmin; i < arrmax; i++) {
        if (!arr[i]) {
            continue;
        }
        if (!arr[i]) { /* the array of hunger status values has a gap ...*/
            continue;  /*... set to Null between Satiated and Hungry     */
        }
        any = zeroany;
        any.a_int = i + adj;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 arr[i], FALSE);
    }
    end_menu(tmpwin, querystr);
    res = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (res > 0) {
        ret = picks->item.a_int - adj;
        free((genericptr_t) picks);
    }
    return ret;
}

static void
status_hilite_add_threshold(int fld, struct hilite_s *hilite)
{
    struct hilite_s *new_hilite, *old_hilite;

    if (!hilite) {
        return;
    }

    /* alloc and initialize a new hilite_s struct */
    new_hilite = (struct hilite_s *) alloc(sizeof (struct hilite_s));
    *new_hilite = *hilite; /* copy struct */

    new_hilite->set = TRUE;
    new_hilite->fld = fld;
    new_hilite->next = (struct hilite_s *) 0;
    /* insert new entry at the end of the list */
    if (!gb.blstats[0][fld].thresholds) {
        gb.blstats[0][fld].thresholds = new_hilite;
    } else {
        for (old_hilite = gb.blstats[0][fld].thresholds; old_hilite->next;
             old_hilite = old_hilite->next) {
            continue;
        }
        old_hilite->next = new_hilite;
    }
    /* sort_hilites(fld) */

    /* current and prev must both point at the same hilites */
    gb.blstats[1][fld].thresholds = gb.blstats[0][fld].thresholds;
}

static boolean
parse_status_hl2(char (*s)[QBUFSZ], boolean from_configfile)
{
    static const char *const aligntxt[] = { "chaotic", "neutral", "lawful" };
    /* hu_stat[] from eat.c has trailing spaces which foul up comparisons;
       for the "not hungry" case, there's no text hence no way to highlight */
    static const char *const hutxt[] = {
        "Satiated", "", "Hungry", "Weak", "Fainting", "Fainted", "Starved"
    };
    char *tmp, *how;
    int sidx = 0, i = -1, dt = ANY_INVALID;
    int coloridx = -1, successes = 0;
    int disp_attrib = 0;
    boolean percent, changed, numeric, down, up,
            grt, lt, gte, le, eq, txtval, always, criticalhp;
    const char *txt;
    enum statusfields fld = BL_FLUSH;
    struct hilite_s hilite;
    char tmpbuf[BUFSZ];

    /* Examples:
        3.6.1:
      OPTION=hilite_status: hitpoints/<10%/red
      OPTION=hilite_status: hitpoints/<10%/red/<5%/purple/1/red&blink+inverse
      OPTION=hilite_status: experience/down/red/up/green
      OPTION=hilite_status: cap/strained/yellow/overtaxed/orange
      OPTION=hilite_status: title/always/blue
      OPTION=hilite_status: title/blue
    */

    /* field name to statusfield */
    fld = fldname_to_bl_indx(s[sidx]);

    if (fld == BL_CHARACTERISTICS) {
        boolean res = FALSE;

        /* recursively set each of strength, dexterity, constitution, &c */
        for (fld = BL_STR; fld <= BL_CH; fld++) {
            Strcpy(s[sidx], initblstats[fld].fldname);
            res = parse_status_hl2(s, from_configfile);
            if (!res) {
                return FALSE;
            }
        }
        return TRUE;
    }
    if (fld == BL_FLUSH) {
        config_error_add("Unknown status field '%s'", s[sidx]);
        return FALSE;
    }
    if (fld == BL_CONDITION) {
        return parse_condition(s, sidx);
    }

    ++sidx;
    while (s[sidx][0]) {
        char buf[BUFSZ], **subfields;
        int sf = 0; /* subfield count */
        int kidx;

        txt = (const char *) 0;
        percent = numeric = always = FALSE;
        down = up = changed = FALSE;
        criticalhp = FALSE;
        grt = gte = eq = le = lt = txtval = FALSE;
#if 0
        /* threshold value - return on empty string */
        if (!s[sidx][0])
            return TRUE;
#endif

        memset((genericptr_t) &hilite, 0, sizeof (struct hilite_s));
        hilite.set = FALSE; /* mark it "unset" */
        hilite.fld = fld;

        if (*s[sidx + 1] == '\0' || !strcmpi(s[sidx], "always")) {
            /* "field/always/color" OR "field/color" */
            always = TRUE;
            if (*s[sidx + 1] == '\0') {
                sidx--;
            }
        } else if (!strcmpi(s[sidx], "up") || !strcmpi(s[sidx], "down")) {
            if (initblstats[fld].anytype == ANY_STR) {
                /* ordered string comparison is supported but LT/GT for
                   the string fields (title, dungeon-level, alignment)
                   is pointless; treat 'up' or 'down' for string fields
                   as 'changed' rather than rejecting them outright */
                ;
            } else if (!strcmpi(s[sidx], "down")) {
                down = TRUE;
            } else {
                up = TRUE;
            }
            changed = TRUE;
        } else if (fld == BL_CAP
                   && is_fld_arrayvalues(s[sidx], enc_stat,
                                         SLT_ENCUMBER, OVERLOADED + 1,
                                         &kidx)) {
            txt = enc_stat[kidx];
            txtval = TRUE;
        } else if (fld == BL_ALIGN
                   && is_fld_arrayvalues(s[sidx], aligntxt, 0, 3, &kidx)) {
            txt = aligntxt[kidx];
            txtval = TRUE;
        } else if (fld == BL_HUNGER
                   && is_fld_arrayvalues(s[sidx], hutxt,
                                         SATIATED, STARVED + 1, &kidx)) {
            txt = hu_stat[kidx]; /* store hu_stat[] val, not hutxt[] */
            txtval = TRUE;
        } else if (!strcmpi(s[sidx], "changed")) {
            changed = TRUE;
        } else if (fld == BL_HP && !strcmpi(s[sidx], "criticalhp")) {
            criticalhp = TRUE;
        } else if (is_ltgt_percentnumber(s[sidx])) {
            const char *op;

            tmp = s[sidx]; /* is_ltgt_() guarantees [<>]?=?[-+]?[0-9]+%? */
            if (strchr(tmp, '%')) {
               percent = TRUE;
            }
            if (*tmp == '<') {
                if (tmp[1] == '=') {
                    le = TRUE;
                } else {
                    lt = TRUE;
                }
            } else if (*tmp == '>') {
                if (tmp[1] == '=') {
                    gte = TRUE;
                } else {
                    grt = TRUE;
                }
            }
            /* '%', '<', '>' have served their purpose, '=' is either
               part of '<' or '>' or optional for '=N', unary '+' is
               just decorative, so get rid of them, leaving -?[0-9]+ */
            tmp = stripchars(tmpbuf, "%<>=+", tmp);
            numeric = TRUE;
            dt = percent ? ANY_INT : initblstats[fld].anytype;
            (void) s_to_anything(&hilite.value, tmp, dt);

            op = grt ? ">" : gte ? ">=" : lt ? "<" : le ? "<=" : "=";
            if (dt == ANY_INT
                /* AC is the only field where negative values make sense but
                   accept >-1 for other fields; reject <0 for non-AC */
                && (hilite.value.a_int
                    < ((fld == BL_AC) ? -128 : grt ? -1 : lt ? 1 : 0)
                /* percentages have another more comprehensive check below */
                    || hilite.value.a_int > (percent ? (lt ? 101 : 100)
                                                     : LARGEST_INT))) {
                config_error_add("%s'%s%d%s'%s", threshold_value,
                                 op, hilite.value.a_int, percent ? "%" : "",
                                 is_out_of_range);
                return FALSE;
            } else if (dt == ANY_LONG
                       && hilite.value.a_long < (grt ? -1L : lt ? 1L : 0L)) {
                config_error_add("%s'%s%ld'%s", threshold_value,
                                 op, hilite.value.a_long, is_out_of_range);
                return FALSE;
            }
        } else if (initblstats[fld].anytype == ANY_STR) {
            txt = s[sidx];
            txtval = TRUE;
        } else {
            config_error_add(has_ltgt_percentnumber(s[sidx])
                 ? "Wrong format '%s', expected a threshold number or percent"
                 : "Unknown behavior '%s'",
                             s[sidx]);
            return FALSE;
        }

        /* relationships {LT_VALUE, LE_VALUE, EQ_VALUE, GE_VALUE, GT_VALUE} */
        if (grt || up) {
            hilite.rel = GT_VALUE;
        } else if (lt || down) {
            hilite.rel = LT_VALUE;
        } else if (gte) {
            hilite.rel = GE_VALUE;
        } else if (le) {
            hilite.rel = LE_VALUE;
        } else if (eq || percent || numeric || changed) {
            hilite.rel = EQ_VALUE;
        } else if (txtval) {
            hilite.rel = TXT_VALUE;
        } else {
            hilite.rel = LT_VALUE;
        }

        if (initblstats[fld].anytype == ANY_STR && (percent || numeric)) {
            config_error_add("Field '%s' does not support numeric values",
                             initblstats[fld].fldname);
            return FALSE;
        }

        if (percent) {
            if (initblstats[fld].idxmax < 0) {
                config_error_add("Cannot use percent with '%s'",
                                 initblstats[fld].fldname);
                return FALSE;
            } else if ((hilite.value.a_int < -1)
                       || (hilite.value.a_int == -1
                           && hilite.value.a_int != GT_VALUE)
                       || (hilite.value.a_int == 0
                           && hilite.rel == LT_VALUE)
                       || (hilite.value.a_int == 100
                           && hilite.rel == GT_VALUE)
                       || (hilite.value.a_int == 101
                           && hilite.value.a_int != LT_VALUE)
                       || (hilite.value.a_int > 101)) {
                config_error_add(
                           "hilite_status: invalid percentage value '%s%d%%'",
                                 (hilite.rel == LT_VALUE) ? "<"
                               : (hilite.rel == LE_VALUE) ? "<="
                               : (hilite.rel == GT_VALUE) ? ">"
                               : (hilite.rel == GE_VALUE) ? ">="
                               : "=",
                                 hilite.value.a_int);
                return FALSE;
            }
        }

        /* actions */
        sidx++;
        how = s[sidx];
        if (!how) {
            if (!successes) {
                return FALSE;
            }
        }
        coloridx = -1;
        Strcpy(buf, how);
        sf = splitsubfields(buf, &subfields, 0);
        if (sf < 1) {
            return FALSE;
        }

        disp_attrib = HL_UNDEF;
        for (i = 0; i < sf; ++i) {
            int a = match_str2attr(subfields[i], FALSE);

            if (a == ATR_BOLD) {
                disp_attrib |= HL_BOLD;
            } else if (a == ATR_DIM) {
                disp_attrib |= HL_DIM;
            } else if (a == ATR_ITALIC) {
                disp_attrib |= HL_ITALIC;
            } else if (a == ATR_ULINE) {
                disp_attrib |= HL_ULINE;
            } else if (a == ATR_BLINK) {
                disp_attrib |= HL_BLINK;
            } else if (a == ATR_INVERSE) {
                disp_attrib |= HL_INVERSE;
            } else if (a == ATR_NONE) {
                disp_attrib = HL_NONE;
            } else {
                int c = match_str2clr(subfields[i]);
                if (c >= CLR_MAX || coloridx != -1) {
                    config_error_add("bad color '%d %d'", c, coloridx);
                    return FALSE;
                }
                coloridx = c;
            }
        }
        if (coloridx == -1) {
            coloridx = NO_COLOR;
        }

        /* Assign the values */
        hilite.coloridx = coloridx | (disp_attrib << 8);

        if (always) {
            hilite.behavior = BL_TH_ALWAYS_HILITE;
        } else if (percent) {
            hilite.behavior = BL_TH_VAL_PERCENTAGE;
        } else if (changed) {
            hilite.behavior = BL_TH_UPDOWN;
        } else if (numeric) {
            hilite.behavior = BL_TH_VAL_ABSOLUTE;
        } else if (txtval) {
            hilite.behavior = BL_TH_TEXTMATCH;
        } else if (hilite.value.a_void) {
            hilite.behavior = BL_TH_VAL_ABSOLUTE;
        } else if (criticalhp) {
            hilite.behavior = BL_TH_CRITICALHP;
        } else {
            hilite.behavior = BL_TH_NONE;
        }

        hilite.anytype = dt;

        if (hilite.behavior == BL_TH_TEXTMATCH && txt) {
            (void) strncpy(hilite.textmatch, txt, sizeof hilite.textmatch);
            hilite.textmatch[sizeof hilite.textmatch - 1] = '\0';
            (void) trimspaces(hilite.textmatch);
        }

        status_hilite_add_threshold(fld, &hilite);

        successes++;
        sidx++;
    }
    return (successes > 0);
}


static unsigned long
query_conditions(void)
{
    int i, res;
    unsigned long ret = 0UL;
    winid tmpwin;
    anything any;
    menu_item *picks = (menu_item *) 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    for (i = 0; i < SIZE(conditions); i++) {
        any = zeroany;
        any.a_ulong = conditions[i].mask;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 conditions[i].text[0], FALSE);
    }
    end_menu(tmpwin, "Choose status conditions");

    res = select_menu(tmpwin, PICK_ANY, &picks);
    destroy_nhwindow(tmpwin);
    if (res > 0) {
        for (i = 0; i < res; i++) {
            ret |= picks[i].item.a_ulong;
        }
        free((genericptr_t) picks);
    }
    return ret;
}

static char *
conditionbitmask2str(unsigned long ul)
{
    static char buf[BUFSZ];
    int i;
    boolean first = TRUE;
    const char *alias = (char *) 0;

    buf[0] = '\0';
    if (!ul) {
        return buf;
    }

    for (i = 1; i < SIZE(condition_aliases); i++) {
        if (condition_aliases[i].bitmask == ul) {
            alias = condition_aliases[i].id;
        }
    }

    for (i = 0; i < SIZE(conditions); i++) {
        if ((conditions[i].mask & ul) != 0UL) {
            Sprintf(eos(buf), "%s%s", (first) ? "" : "+",
                    conditions[i].text[0]);
            first = FALSE;
        }
    }

    if (!first && alias) {
        Sprintf(buf, "%s", alias);
    }

    return buf;
}


static unsigned long
match_str2conditionbitmask(const char *str)
{
    int i, nmatches = 0;
    unsigned long mask = 0UL;

    if (str && *str) {
        /* check matches to canonical names */
        for (i = 0; i < SIZE(conditions); i++) {
            if (fuzzymatch(conditions[i].text[0], str, " -_", TRUE)) {
                mask |= conditions[i].mask;
                nmatches++;
            }
        }
        if (!nmatches) {
            /* check aliases */
            for (i = 0; i < SIZE(condition_aliases); i++) {
                if (fuzzymatch(condition_aliases[i].id, str, " -_", TRUE)) {
                    mask |= condition_aliases[i].bitmask;
                    nmatches++;
                }
            }
        }
        if (!nmatches) {
            /* check partial matches to aliases */
            int len = (int) strlen(str);

            for (i = 0; i < SIZE(condition_aliases); i++) {
                if (!strncmpi(str, condition_aliases[i].id, len)) {
                    mask |= condition_aliases[i].bitmask;
                    nmatches++;
                }
            }
        }
    }
    return mask;
}

static unsigned long
str2conditionbitmask(char *str)
{
    unsigned long conditions_bitmask = 0UL;
    char **subfields;
    int i, sf;

    sf = splitsubfields(str, &subfields, SIZE(conditions));
    if (sf < 1) {
        return 0UL;
    }

    for (i = 0; i < sf; ++i) {
        unsigned long bm = match_str2conditionbitmask(subfields[i]);

        if (!bm) {
            config_error_add("Unknown condition '%s'", subfields[i]);
            return 0UL;
        }
        conditions_bitmask |= bm;
    }
    return conditions_bitmask;
}

static boolean
parse_condition(char (*s)[QBUFSZ], int sidx)
{
    int i;
    int coloridx = NO_COLOR;
    char *tmp, *how;
    unsigned long conditions_bitmask = 0UL;
    boolean result = FALSE;

    if (!s) {
        return FALSE;
    }

    /*3.6.1:
      OPTION=hilite_status: condition/stone+slime+foodPois/red&inverse */

    /*
     * TODO?
     *  It would be simpler to treat each condition (also hunger state
     *  and encumbrance level) as if it were a separate field.  That
     *  way they could have either or both 'changed' temporary rule and
     *  'always' persistent rule and wouldn't need convoluted access to
     *  the intended color and attributes.
     */

    sidx++;
    if (!s[sidx][0]) {
        config_error_add("Missing condition(s)");
        return FALSE;
    }
    while (s[sidx][0]) {
        int sf = 0; /* subfield count */
        char buf[BUFSZ], **subfields;

        tmp = s[sidx];
        Strcpy(buf, tmp);
        conditions_bitmask = str2conditionbitmask(buf);

        if (!conditions_bitmask) {
            return FALSE;
        }

        /*
         * We have the conditions_bitmask with bits set for
         * each ailment we want in a particular color and/or
         * attribute, but we need to assign it to an array of
         * bitmasks indexed by the color chosen
         *        (0 to (CLR_MAX - 1))
         * and/or attributes chosen
         *        (HL_ATTCLR_NONE to (BL_ATTCLR_MAX - 1))
         * We still have to parse the colors and attributes out.
         */

        /* actions */
        sidx++;
        how = s[sidx];
        if (!how || !*how) {
            config_error_add("Missing color+attribute");
            return FALSE;
        }

        Strcpy(buf, how);
        sf = splitsubfields(buf, &subfields, 0);

        /*
         * conditions_bitmask now has bits set representing
         * the conditions that player wants represented, but
         * now we parse out *how* they will be represented.
         *
         * Only 1 colour is allowed, but potentially multiple
         * attributes are allowed.
         *
         * We have the following additional array offsets to
         * use for storing the attributes beyond the end of
         * the color indexes, all of which are less than CLR_MAX.
         *
         */

        for (i = 0; i < sf; ++i) {
            int a = match_str2attr(subfields[i], FALSE);

            if (a == ATR_BOLD) {
                gc.cond_hilites[HL_ATTCLR_BOLD] |= conditions_bitmask;
            } else if (a == ATR_DIM) {
                gc.cond_hilites[HL_ATTCLR_DIM] |= conditions_bitmask;
            } else if (a == ATR_ITALIC) {
                gc.cond_hilites[HL_ATTCLR_ITALIC] |= conditions_bitmask;
            } else if (a == ATR_ULINE) {
                gc.cond_hilites[HL_ATTCLR_ULINE] |= conditions_bitmask;
            } else if (a == ATR_BLINK) {
                gc.cond_hilites[HL_ATTCLR_BLINK] |= conditions_bitmask;
            } else if (a == ATR_INVERSE) {
                gc.cond_hilites[HL_ATTCLR_INVERSE] |= conditions_bitmask;
            } else if (a == ATR_NONE) {
                gc.cond_hilites[HL_ATTCLR_BOLD] &= ~conditions_bitmask;
                gc.cond_hilites[HL_ATTCLR_DIM] &= ~conditions_bitmask;
                gc.cond_hilites[HL_ATTCLR_ITALIC] &= ~conditions_bitmask;
                gc.cond_hilites[HL_ATTCLR_ULINE] &= ~conditions_bitmask;
                gc.cond_hilites[HL_ATTCLR_BLINK] &= ~conditions_bitmask;
                gc.cond_hilites[HL_ATTCLR_INVERSE] &= ~conditions_bitmask;
            } else {
                int k = match_str2clr(subfields[i]);

                if (k >= CLR_MAX) {
                    config_error_add("bad color %d", k);
                    return FALSE;
                }
                coloridx = k;
            }
        }
        /* set the bits in the appropriate member of the
           condition array according to color chosen as index */

        gc.cond_hilites[coloridx] |= conditions_bitmask;
        result = TRUE;
        sidx++;
    }
    return result;
}

void
clear_status_hilites(void)
{
    int i;

    for (i = 0; i < MAXBLSTATS; ++i) {
        struct hilite_s *temp, *next;

        for (temp = gb.blstats[0][i].thresholds; temp; temp = next) {
            next = temp->next;
            free(temp);
        }
        gb.blstats[0][i].thresholds = gb.blstats[1][i].thresholds = 0;
        /* pointer into thresholds list, now stale */
        gb.blstats[0][i].hilite_rule = gb.blstats[1][i].hilite_rule = 0;
    }
}

static char *
hlattr2attrname(int attrib, char *buf, size_t bufsz)
{
    if (attrib && buf) {
        char attbuf[BUFSZ];
        int first = 0;
        size_t k;

        attbuf[0] = '\0';
        if (attrib == HL_NONE) {
            Strcpy(buf, "normal");
            return buf;
        }

        if (attrib & HL_BOLD) {
            Strcat(attbuf, first++ ? "+bold" : "bold");
        }
        if (attrib & HL_DIM) {
            Strcat(attbuf, first++ ? "+dim" : "dim");
        }
        if (attrib & HL_ITALIC) {
            Strcat(attbuf, first++ ? "+italic" : "italic");
        }
        if (attrib & HL_ULINE) {
            Strcat(attbuf, first++ ? "+underline" : "underline");
        }
        if (attrib & HL_BLINK) {
            Strcat(attbuf, first++ ? "+blink" : "blink");
        }
        if (attrib & HL_INVERSE) {
            Strcat(attbuf, first++ ? "+inverse" : "inverse");
        }

        k = strlen(attbuf);
        if (k < (size_t)(bufsz - 1)) {
            Strcpy(buf, attbuf);
        }
        return buf;
    }
    return (char *) 0;
}

struct _status_hilite_line_str {
    int id;
    int fld;
    struct hilite_s *hl;
    unsigned long mask;
    char str[BUFSZ];
    struct _status_hilite_line_str *next;
};

/* these don't need to be in 'struct g' */
static struct _status_hilite_line_str *status_hilite_str = 0;
static int status_hilite_str_id = 0;

static void
status_hilite_linestr_add(
    int fld,
    struct hilite_s *hl,
    unsigned long mask,
    const char *str)
{
    struct _status_hilite_line_str *tmp, *nxt;

    tmp = (struct _status_hilite_line_str *) alloc(sizeof *tmp);
    (void) memset(tmp, 0, sizeof *tmp);
    tmp->next = (struct _status_hilite_line_str *) 0;

    tmp->id = ++status_hilite_str_id;
    tmp->fld = fld;
    tmp->hl = hl;
    tmp->mask = mask;
    if (fld == BL_TITLE) {
        Strcpy(tmp->str, str);
    } else {
        (void) stripchars(tmp->str, " ", str);
    }

    if ((nxt = status_hilite_str) != 0) {
        while (nxt->next) {
            nxt = nxt->next;
        }
        nxt->next = tmp;
    } else {
        status_hilite_str = tmp;
    }
}

static void
status_hilite_linestr_done(void)
{
    struct _status_hilite_line_str *nxt, *tmp = status_hilite_str;
    while (tmp) {
        nxt = tmp->next;
        free(tmp);
        tmp = nxt;
    }
    status_hilite_str = (struct _status_hilite_line_str *) 0;
    status_hilite_str_id = 0;
}

static int
status_hilite_linestr_countfield(int fld)
{
    struct _status_hilite_line_str *tmp;
    boolean countall = (fld == BL_FLUSH);
    int count = 0;

    for (tmp = status_hilite_str; tmp; tmp = tmp->next) {
        if (countall || tmp->fld == fld) count++;
    }
    return count;
}

/* used by options handling, doset(options.c) */
int
count_status_hilites(void)
{
    int count;

    status_hilite_linestr_gather();
    count = status_hilite_linestr_countfield(BL_FLUSH);
    status_hilite_linestr_done();
    return count;
}

static void
status_hilite_linestr_gather_conditions(void)
{
    int i;
    struct _cond_map {
        unsigned long bm;
        unsigned int clratr;
    } cond_maps[SIZE(conditions)];

    (void) memset(cond_maps, 0,
                  SIZE(conditions) * sizeof (struct _cond_map));

    for (i = 0; i < SIZE(conditions); i++) {
        int clr = NO_COLOR;
        int atr = HL_NONE;
        int j;

        for (j = 0; j < CLR_MAX; j++) {
            if (gc.cond_hilites[j] & conditions[i].mask) {
                clr = j;
                break;
            }
        }
        if (gc.cond_hilites[HL_ATTCLR_BOLD] & conditions[i].mask) {
            atr |= HL_BOLD;
        }
        if (gc.cond_hilites[HL_ATTCLR_DIM] & conditions[i].mask) {
            atr |= HL_DIM;
        }
        if (gc.cond_hilites[HL_ATTCLR_ITALIC] & conditions[i].mask) {
            atr |= HL_ITALIC;
        }
        if (gc.cond_hilites[HL_ATTCLR_ULINE] & conditions[i].mask) {
            atr |= HL_ULINE;
        }
        if (gc.cond_hilites[HL_ATTCLR_BLINK] & conditions[i].mask) {
            atr |= HL_BLINK;
        }
        if (gc.cond_hilites[HL_ATTCLR_INVERSE] & conditions[i].mask) {
            atr |= HL_INVERSE;
        }
        if (atr != HL_NONE) {
            atr &= ~HL_NONE;
        }

        if (clr != NO_COLOR || atr != HL_NONE) {
            unsigned int ca = clr | (atr << 8);
            boolean added_condmap = FALSE;

            for (j = 0; j < SIZE(conditions); j++) {
                if (cond_maps[j].clratr == ca) {
                    cond_maps[j].bm |= conditions[i].mask;
                    added_condmap = TRUE;
                    break;
                }
            }
            if (!added_condmap) {
                for (j = 0; j < SIZE(conditions); j++) {
                    if (!cond_maps[j].bm) {
                        cond_maps[j].bm = conditions[i].mask;
                        cond_maps[j].clratr = ca;
                        break;
                    }
                }
            }
        }
    }

    for (i = 0; i < SIZE(conditions); i++) {
        if (cond_maps[i].bm) {
            int clr = NO_COLOR, atr = HL_NONE;

            split_clridx(cond_maps[i].clratr, &clr, &atr);
            if (clr != NO_COLOR || atr != HL_NONE) {
                char clrbuf[BUFSZ];
                char attrbuf[BUFSZ];
                char condbuf[BUFSZ];
                char *tmpattr;

                (void) strNsubst(strcpy(clrbuf, clr2colorname(clr)),
                                 " ", "-", 0);
                tmpattr = hlattr2attrname(atr, attrbuf, BUFSZ);
                if (tmpattr) {
                    Sprintf(eos(clrbuf), "&%s", tmpattr);
                }
                Snprintf(condbuf, sizeof(condbuf), "condition/%s/%s",
                         conditionbitmask2str(cond_maps[i].bm), clrbuf);
                status_hilite_linestr_add(BL_CONDITION, 0,
                                          cond_maps[i].bm, condbuf);
            }
        }
    }
}

static void
status_hilite_linestr_gather(void)
{
    int i;
    struct hilite_s *hl;

    status_hilite_linestr_done();
    for (i = 0; i < MAXBLSTATS; i++) {
        hl = gb.blstats[0][i].thresholds;
        while (hl) {
            status_hilite_linestr_add(i, hl, 0UL, status_hilite2str(hl));
            hl = hl->next;
        }
    }
    status_hilite_linestr_gather_conditions();
}

static char *
status_hilite2str(struct hilite_s *hl)
{
    static char buf[BUFSZ];
    int clr = NO_COLOR, attr = ATR_NONE;
    char behavebuf[BUFSZ];
    char clrbuf[BUFSZ];
    char attrbuf[BUFSZ];
    char *tmpattr;
    const char *op;

    if (!hl)
        return (char *) 0;

    behavebuf[0] = '\0';
    clrbuf[0] = '\0';
    op = (hl->rel == LT_VALUE) ? "<"
       : (hl->rel == LE_VALUE) ? "<="
       : (hl->rel == GT_VALUE) ? ">"
       : (hl->rel == GE_VALUE) ? ">="
       : (hl->rel == EQ_VALUE) ? "="
       : 0;

    switch (hl->behavior) {
    case BL_TH_VAL_PERCENTAGE:
        if (op) {
            Sprintf(behavebuf, "%s%d%%", op, hl->value.a_int);
        } else {
            impossible("hl->behavior=percentage, rel error");
        }
        break;
    case BL_TH_UPDOWN:
        if (hl->rel == LT_VALUE) {
            Sprintf(behavebuf, "down");
        } else if (hl->rel == GT_VALUE) {
            Sprintf(behavebuf, "up");
        } else if (hl->rel == EQ_VALUE) {
            Sprintf(behavebuf, "changed");
        } else {
            impossible("hl->behavior=updown, rel error");
        }
        break;
    case BL_TH_VAL_ABSOLUTE:
        if (op) {
            Sprintf(behavebuf, "%s%d", op, hl->value.a_int);
        } else {
            impossible("hl->behavior=absolute, rel error");
        }
        break;
    case BL_TH_TEXTMATCH:
        if (hl->rel == TXT_VALUE && hl->textmatch[0]) {
            Sprintf(behavebuf, "%s", hl->textmatch);
        } else {
            impossible("hl->behavior=textmatch, rel or textmatch error");
        }
        break;
    case BL_TH_CONDITION:
        if (hl->rel == EQ_VALUE) {
            Sprintf(behavebuf, "%s", conditionbitmask2str(hl->value.a_ulong));
        } else {
            impossible("hl->behavior=condition, rel error");
        }
        break;
    case BL_TH_ALWAYS_HILITE:
        Sprintf(behavebuf, "always");
        break;
    case BL_TH_CRITICALHP:
        Sprintf(behavebuf, "criticalhp");
        break;
    case BL_TH_NONE:
        break;
    default:
        break;
    }

    split_clridx(hl->coloridx, &clr, &attr);
    (void) strNsubst(strcpy(clrbuf, clr2colorname(clr)), " ", "-", 0);
    if (attr != HL_UNDEF) {
        if ((tmpattr = hlattr2attrname(attr, attrbuf, BUFSZ)) != 0) {
            Sprintf(eos(clrbuf), "&%s", tmpattr);
        }
    }
    Snprintf(buf, sizeof(buf), "%s/%s/%s", initblstats[hl->fld].fldname,
             behavebuf, clrbuf);

    return buf;
}


static int
status_hilite_menu_choose_field(void)
{
    winid tmpwin;
    int i, res, fld = BL_FLUSH;
    anything any;
    menu_item *picks = (menu_item *) 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    for (i = 0; i < MAXBLSTATS; i++) {
#ifndef SCORE_ON_BOTL
        if (initblstats[i].fld == BL_SCORE
            && !gb.blstats[0][BL_SCORE].thresholds) {
            continue;
        }
#endif
        any = zeroany;
        any.a_int = (i + 1);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 initblstats[i].fldname, FALSE);
    }
    end_menu(tmpwin, "Select a hilite field:");
    res = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (res > 0) {
        fld = picks->item.a_int - 1;
        free((genericptr_t) picks);
    }
    return fld;
}

static int
status_hilite_menu_choose_behavior(int fld)
{
    winid tmpwin;
    int res = 0, beh = BL_TH_NONE - 1;
    anything any;
    menu_item *picks = (menu_item *) 0;
    char buf[BUFSZ];
    int at;
    int onlybeh = BL_TH_NONE, nopts = 0;

    if (fld < 0 || fld >= MAXBLSTATS) {
        return BL_TH_NONE;
    }

    at = initblstats[fld].anytype;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    if (fld != BL_CONDITION) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_ALWAYS_HILITE;
        Sprintf(buf, "Always highlight %s", initblstats[fld].fldname);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'a', 0, ATR_NONE,
                 buf, FALSE);
        nopts++;
    }
    if (fld == BL_CONDITION) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_CONDITION;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'b', 0, ATR_NONE,
                 "Bitmask of conditions", FALSE);
        nopts++;
    }
    if (fld != BL_CONDITION && fld != BL_VERS) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_UPDOWN;
        Sprintf(buf, "%s value changes", initblstats[fld].fldname);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'c', 0, ATR_NONE,
                 buf, FALSE);
        nopts++;
    }
    if (fld != BL_CAP && fld != BL_HUNGER
        && (at == ANY_INT || at == ANY_LONG)) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_VAL_ABSOLUTE;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'n', 0, ATR_NONE,
                 "Number threshold", FALSE);
        nopts++;
    }
    if (initblstats[fld].idxmax >= 0) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_VAL_PERCENTAGE;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'p', 0, ATR_NONE,
                 "Percentage threshold", FALSE);
        nopts++;
    }
    if (fld == BL_HP) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_CRITICALHP;
        Sprintf(buf, "Highlight critically low %s", initblstats[fld].fldname);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'C', 0, ATR_NONE,
                 buf, FALSE);
        nopts++;
    }
    if (initblstats[fld].anytype == ANY_STR
        || fld == BL_CAP || fld == BL_HUNGER) {
        any = zeroany;
        any.a_int = onlybeh = BL_TH_TEXTMATCH;
        Sprintf(buf, "%s text match", initblstats[fld].fldname);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 't', 0, ATR_NONE,
                 buf, FALSE);
        nopts++;
    }

    Sprintf(buf, "Select %s field hilite behavior:", initblstats[fld].fldname);
    end_menu(tmpwin, buf);

    if (nopts > 1) {
        res = select_menu(tmpwin, PICK_ONE, &picks);
        if (res == 0) { /* none chosen*/
            beh = BL_TH_NONE;
        } else if (res == -1) { /* menu cancelled */
            beh = (BL_TH_NONE - 1);
        }
    } else if (onlybeh != BL_TH_NONE) {
        beh = onlybeh;
    }
    destroy_nhwindow(tmpwin);
    if (res > 0) {
        beh = picks->item.a_int;
        free((genericptr_t) picks);
    }
    return beh;
}


static int
status_hilite_menu_choose_updownboth(
    int fld,
    const char *str,
    boolean ltok, boolean gtok)
{
    int res, ret = NO_LTEQGT;
    winid tmpwin;
    char buf[BUFSZ];
    anything any;
    menu_item *picks = (menu_item *) 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    if (ltok) {
        if (str) {
            Sprintf(buf, "%s than %s",
                    (fld == BL_AC) ? "Better (lower)" : "Less", str);
        } else {
            Sprintf(buf, "Value goes down");
        }
        any = zeroany;
        any.a_int = 10 + LT_VALUE;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 buf, FALSE);
        if (str) {
            Sprintf(buf, "%s or %s",
                    str, (fld == BL_AC) ? "better (lower)" : "less");
            any = zeroany;
            any.a_int = 10 + LE_VALUE;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     buf, FALSE);
        }
    }

    if (str) {
        Sprintf(buf, "Exactly %s", str);
    } else {
        Sprintf(buf, "Value changes");
    }
    any = zeroany;
    any.a_int = 10 + EQ_VALUE;
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
             buf, FALSE);

    if (gtok) {
        if (str) {
            Sprintf(buf, "%s or %s",
                    str, (fld == BL_AC) ? "worse (higher)" : "more");
            any = zeroany;
            any.a_int = 10 + GE_VALUE;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     buf, FALSE);
        }
        if (str) {
            Sprintf(buf, "%s than %s",
                    (fld == BL_AC) ? "Worse (higher)" : "More", str);
        } else {
            Sprintf(buf, "Value goes up");
        }
        any = zeroany;
        any.a_int = 10 + GT_VALUE;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 buf, FALSE);
    }
    Sprintf(buf, "Select field %s value:", initblstats[fld].fldname);
    end_menu(tmpwin, buf);

    res = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (res > 0) {
        ret = picks->item.a_int - 10;
        free((genericptr_t) picks);
    }
    return ret;
}


static boolean
status_hilite_menu_add(int origfld)
{
    int fld;
    int behavior;
    int lt_gt_eq;
    int clr = NO_COLOR, atr = HL_UNDEF;
    struct hilite_s hilite;
    unsigned long cond = 0UL;
    char colorqry[BUFSZ];
    char attrqry[BUFSZ];
    int retry = 0;

 choose_field:
    fld = origfld;
    if (fld == BL_FLUSH) {
        fld = status_hilite_menu_choose_field();
        /* isn't this redundant given what follows? */
        if (fld == BL_FLUSH) {
            return FALSE;
        }
    }

    if (fld == BL_FLUSH) {
        return FALSE;
    }

    colorqry[0] = '\0';
    attrqry[0] = '\0';

    memset((genericptr_t) &hilite, 0, sizeof (struct hilite_s));
    hilite.next = (struct hilite_s *) 0;
    hilite.set = FALSE; /* mark it "unset" */
    hilite.fld = fld;

 choose_behavior:
    behavior = status_hilite_menu_choose_behavior(fld);

    if (behavior == (BL_TH_NONE - 1)) {
        return FALSE;
    } else if (behavior == BL_TH_NONE) {
        if (origfld == BL_FLUSH) {
            goto choose_field;
        }
        return FALSE;
    }

    hilite.behavior = behavior;

 choose_value:
    if (retry++ > 5) {
        pline("That's enough tries.");
        return FALSE;
    }
    if (behavior == BL_TH_VAL_PERCENTAGE
        || behavior == BL_TH_VAL_ABSOLUTE) {
        char inbuf[BUFSZ], buf[BUFSZ];
        anything aval;
        int val, dt;
        boolean gotnum = FALSE, percent = (behavior == BL_TH_VAL_PERCENTAGE);
        char *inp, *numstart;
        const char *op;

        lt_gt_eq = NO_LTEQGT; /* not set up yet */
        inbuf[0] = '\0';
        Sprintf(buf, "Enter %svalue for %s threshold:",
                percent ? "percentage " : "",
                initblstats[fld].fldname);
        getlin(buf, inbuf);
        if (inbuf[0] == '\0' || inbuf[0] == '\033') {
            goto choose_behavior;
        }

        inp = numstart = trimspaces(inbuf);
        if (!*inp) {
            goto choose_behavior;
        }

        /* allow user to enter "<50%" or ">50" or just "50"
           or <=50% or >=50 or =50 */
        if (*inp == '>' || *inp == '<' || *inp == '=') {
            lt_gt_eq = (*inp == '>') ? ((inp[1] == '=') ? GE_VALUE : GT_VALUE)
                     : (*inp == '<') ? ((inp[1] == '=') ? LE_VALUE : LT_VALUE)
                       : EQ_VALUE;
            *inp++ = ' ';
            numstart++;
            if (lt_gt_eq == GE_VALUE || lt_gt_eq == LE_VALUE) {
                *inp++ = ' ';
                numstart++;
            }
        }
        if (*inp == '-') {
            inp++;
        } else if (*inp == '+') {
            *inp++ = ' ';
            numstart++;
        }
        while (digit(*inp)) {
            inp++;
            gotnum = TRUE;
        }
        if (*inp == '%') {
            if (!percent) {
                pline("Not expecting a percentage.");
                goto choose_behavior;
            }
            *inp = '\0'; /* strip '%' [this accepts trailing junk!] */
        } else if (*inp) {
            /* some random characters */
            pline("\"%s\" is not a recognized number.", inp);
            goto choose_value;
        }
        if (!gotnum) {
            pline("Is that an invisible number?");
            goto choose_value;
        }
        op = (lt_gt_eq == LT_VALUE) ? "<"
           : (lt_gt_eq == LE_VALUE) ? "<="
           : (lt_gt_eq == GT_VALUE) ? ">"
           : (lt_gt_eq == GE_VALUE) ? ">="
           : (lt_gt_eq == EQ_VALUE) ? "="
           : ""; /* didn't specify lt_gt_eq with number */

        aval = cg.zeroany;
        dt = percent ? ANY_INT : initblstats[fld].anytype;
        (void) s_to_anything(&aval, numstart, dt);

        if (percent) {
            val = aval.a_int;
            if (initblstats[fld].idxmax == -1) {
                pline("Field '%s' does not support percentage values.",
                      initblstats[fld].fldname);
                behavior = BL_TH_VAL_ABSOLUTE;
                goto choose_value;
            }
            /* if player only specified a number then lt_gt_eq isn't set
               up yet and the >-1 and <101 exceptions can't be honored;
               deliberate use of those should be uncommon enough for
               that to be palatable; for 0 and 100, choose_updown_both()
               will prevent useless operations */
            if ((val < 0 && (val != -1 || lt_gt_eq != GT_VALUE))
                || (val == 0 && lt_gt_eq == LT_VALUE)
                || (val == 100 && lt_gt_eq == GT_VALUE)
                || (val > 100 && (val != 101 || lt_gt_eq != LT_VALUE))) {
                pline("'%s%d%%' is not a valid percent value.", op, val);
                goto choose_value;
            }
            /* restore suffix for use in color and attribute prompts */
            if (!strchr(numstart, '%')) {
                Strcat(numstart, "%");
            }

        /* reject negative values except for AC and >-1; reject 0 for < */
        } else if (dt == ANY_INT
                   && (aval.a_int < ((fld == BL_AC) ? -128
                                     : (lt_gt_eq == GT_VALUE) ? -1
                                       : (lt_gt_eq == LT_VALUE) ? 1 : 0))) {
            pline("%s'%s%d'%s", threshold_value,
                  op, aval.a_int, is_out_of_range);
            goto choose_value;
        } else if (dt == ANY_LONG
                   && (aval.a_long < ((lt_gt_eq == GT_VALUE) ? -1L
                                      : (lt_gt_eq == LT_VALUE) ? 1L : 0L))) {
            pline("%s'%s%ld'%s", threshold_value,
                  op, aval.a_long, is_out_of_range);
            goto choose_value;
        }

        if (lt_gt_eq == NO_LTEQGT) {
            boolean ltok = ((dt == ANY_INT)
                            ? (aval.a_int > 0 || fld == BL_AC)
                            : (aval.a_long > 0L)),
                    gtok = (!percent || aval.a_long < 100);

            lt_gt_eq = status_hilite_menu_choose_updownboth(fld, inbuf,
                                                            ltok, gtok);
            if (lt_gt_eq == NO_LTEQGT) {
                goto choose_value;
            }
        }

        Sprintf(colorqry, "Choose a color for when %s is %s%s%s:",
                initblstats[fld].fldname,
                (lt_gt_eq == LT_VALUE) ? "less than "
                  : (lt_gt_eq == GT_VALUE) ? "more than "
                    : "",
                numstart,
                (lt_gt_eq == LE_VALUE) ? " or less"
                  : (lt_gt_eq == GE_VALUE) ? " or more"
                    : "");
        Sprintf(attrqry, "Choose attribute for when %s is %s%s%s:",
                initblstats[fld].fldname,
                (lt_gt_eq == LT_VALUE) ? "less than "
                  : (lt_gt_eq == GT_VALUE) ? "more than "
                    : "",
                numstart,
                (lt_gt_eq == LE_VALUE) ? " or less"
                  : (lt_gt_eq == GE_VALUE) ? " or more"
                    : "");

        hilite.rel = lt_gt_eq;
        hilite.value = aval;
    } else if (behavior == BL_TH_UPDOWN) {
        if (initblstats[fld].anytype != ANY_STR) {
            boolean ltok = (fld != BL_TIME), gtok = TRUE;

            lt_gt_eq = status_hilite_menu_choose_updownboth(fld, (char *) 0,
                                                            ltok, gtok);
            if (lt_gt_eq == NO_LTEQGT) {
                goto choose_behavior;
            }
        } else { /* ANY_STR */
            /* player picked '<field> value changes' in outer menu;
               ordered string comparison is supported but LT/GT for the
               string status fields (title, dungeon level, alignment)
               is pointless; rather than calling ..._choose_updownboth()
               with ltok==False plus gtok=False and having a menu with a
               single choice, skip it altogether and just use 'changed' */
            lt_gt_eq = EQ_VALUE;
        }
        Sprintf(colorqry, "Choose a color for when %s %s:",
                initblstats[fld].fldname,
                (lt_gt_eq == EQ_VALUE) ? "changes"
                  : (lt_gt_eq == LT_VALUE) ? "decreases"
                    : "increases");
        Sprintf(attrqry, "Choose attribute for when %s %s:",
                initblstats[fld].fldname,
                (lt_gt_eq == EQ_VALUE) ? "changes"
                  : (lt_gt_eq == LT_VALUE) ? "decreases"
                    : "increases");
        hilite.rel = lt_gt_eq;
    } else if (behavior == BL_TH_CONDITION) {
        cond = query_conditions();
        if (!cond) {
            if (origfld == BL_FLUSH) {
                goto choose_field;
            }
            return FALSE;
        }
        Snprintf(colorqry, sizeof(colorqry),
                "Choose a color for conditions %s:",
                conditionbitmask2str(cond));
        Snprintf(attrqry, sizeof(attrqry),
                "Choose attribute for conditions %s:",
                conditionbitmask2str(cond));
    } else if (behavior == BL_TH_TEXTMATCH) {
        char qry_buf[BUFSZ];

        Sprintf(qry_buf, "%s %s text value to match:",
                (fld == BL_CAP
                 || fld == BL_ALIGN
                 || fld == BL_HUNGER
                 || fld == BL_TITLE) ? "Choose" : "Enter",
                initblstats[fld].fldname);
        if (fld == BL_CAP) {
            int rv = query_arrayvalue(qry_buf,
                                      enc_stat,
                                      SLT_ENCUMBER, OVERLOADED + 1);

            if (rv < SLT_ENCUMBER) {
                goto choose_behavior;
            }

            hilite.rel = TXT_VALUE;
            Strcpy(hilite.textmatch, enc_stat[rv]);
        } else if (fld == BL_ALIGN) {
            static const char *const aligntxt[] = {
                "chaotic", "neutral", "lawful"
            };
            int rv = query_arrayvalue(qry_buf,
                                      aligntxt, 0, 2 + 1);

            if (rv < 0) {
                goto choose_behavior;
            }

            hilite.rel = TXT_VALUE;
            Strcpy(hilite.textmatch, aligntxt[rv]);
        } else if (fld == BL_HUNGER) {
            static const char *const hutxt[] = {
                "Satiated", (char *) 0, "Hungry", "Weak",
                "Fainting", "Fainted", "Starved"
            };
            int rv = query_arrayvalue(qry_buf, hutxt, SATIATED, STARVED + 1);

            if (rv < SATIATED) {
                goto choose_behavior;
            }

            hilite.rel = TXT_VALUE;
            Strcpy(hilite.textmatch, hutxt[rv]);
        } else if (fld == BL_TITLE) {
            const char *rolelist[3 * 9 + 1];
            char mbuf[MAXVALWIDTH], fbuf[MAXVALWIDTH], obuf[MAXVALWIDTH];
            int i, j, rv;

            for (i = j = 0; i < 9; i++) {
                Sprintf(mbuf, "\"%s\"", urole.rank[i].m);
                if (urole.rank[i].f) {
                    Sprintf(fbuf, "\"%s\"", urole.rank[i].f);
                    Snprintf(obuf, sizeof obuf, "%s or %s",
                            flags.female ? fbuf : mbuf,
                            flags.female ? mbuf : fbuf);
                } else {
                    fbuf[0] = obuf[0] = '\0';
                }
                if (flags.female) {
                    if (*fbuf) {
                        rolelist[j++] = dupstr(fbuf);
                    }
                    rolelist[j++] = dupstr(mbuf);
                    if (*obuf) {
                        rolelist[j++] = dupstr(obuf);
                    }
                } else {
                    rolelist[j++] = dupstr(mbuf);
                    if (*fbuf) {
                        rolelist[j++] = dupstr(fbuf);
                    }
                    if (*obuf) {
                        rolelist[j++] = dupstr(obuf);
                    }
                }
            }
            rolelist[j++] = dupstr("\"none of the above (polymorphed)\"");

            rv = query_arrayvalue(qry_buf, rolelist, 0, j);
            if (rv >= 0) {
                hilite.rel = TXT_VALUE;
                Strcpy(hilite.textmatch, rolelist[rv]);
            }
            for (i = 0; i < j; i++) {
                free((genericptr_t) rolelist[i]), rolelist[i] = 0;
            }
            if (rv < 0) {
                goto choose_behavior;
            }
        } else {
            char inbuf[BUFSZ];

            inbuf[0] = '\0';
            getlin(qry_buf, inbuf);
            if (inbuf[0] == '\0' || inbuf[0] == '\033') {
                goto choose_behavior;
            }

            hilite.rel = TXT_VALUE;
            if (strlen(inbuf) < sizeof hilite.textmatch) {
                Strcpy(hilite.textmatch, inbuf);
            } else {
                return FALSE;
            }
        }
        Sprintf(colorqry, "Choose a color for when %s is '%s':",
                initblstats[fld].fldname, hilite.textmatch);
        Sprintf(attrqry, "Choose attribute for when %s is '%s':",
                initblstats[fld].fldname, hilite.textmatch);
    } else if (behavior == BL_TH_ALWAYS_HILITE) {
        Sprintf(colorqry, "Choose a color to always hilite %s:",
                initblstats[fld].fldname);
        Sprintf(attrqry, "Choose attribute to always hilite %s:",
                initblstats[fld].fldname);
    }

 choose_color:
    clr = query_color(colorqry);
    if (clr == -1) {
        if (behavior != BL_TH_ALWAYS_HILITE) {
            goto choose_value;
        } else {
            goto choose_behavior;
        }
    }
    atr = query_attr(attrqry);
    if (atr == -1) {
        goto choose_color;
    }

    if (behavior == BL_TH_CONDITION) {
        char clrbuf[BUFSZ];
        char attrbuf[BUFSZ];
        char *tmpattr;

        if (atr & HL_BOLD) {
            gc.cond_hilites[HL_ATTCLR_BOLD] |= cond;
        }
        if (atr & HL_DIM) {
            gc.cond_hilites[HL_ATTCLR_DIM] |= cond;
        }
        if (atr & HL_ITALIC) {
            gc.cond_hilites[HL_ATTCLR_ITALIC] |= cond;
        }
        if (atr & HL_ULINE) {
            gc.cond_hilites[HL_ATTCLR_ULINE] |= cond;
        }
        if (atr & HL_BLINK) {
            gc.cond_hilites[HL_ATTCLR_BLINK] |= cond;
        }
        if (atr & HL_INVERSE) {
            gc.cond_hilites[HL_ATTCLR_INVERSE] |= cond;
        }
        if (atr == HL_NONE) {
            gc.cond_hilites[HL_ATTCLR_BOLD] &= ~cond;
            gc.cond_hilites[HL_ATTCLR_DIM] &= ~cond;
            gc.cond_hilites[HL_ATTCLR_ITALIC] &= ~cond;
            gc.cond_hilites[HL_ATTCLR_ULINE] &= ~cond;
            gc.cond_hilites[HL_ATTCLR_BLINK] &= ~cond;
            gc.cond_hilites[HL_ATTCLR_INVERSE] &= ~cond;
        }
        gc.cond_hilites[clr] |= cond;
        (void) strNsubst(strcpy(clrbuf, clr2colorname(clr)), " ", "-", 0);
        tmpattr = hlattr2attrname(atr, attrbuf, BUFSZ);
        if (tmpattr) {
            Sprintf(eos(clrbuf), "&%s", tmpattr);
        }
        pline("Added hilite condition/%s/%s",
              conditionbitmask2str(cond), clrbuf);
    } else {
        char *p, *q;

        hilite.coloridx = clr | (atr << 8);
        hilite.anytype = initblstats[fld].anytype;

        if (fld == BL_TITLE && (p = strstri(hilite.textmatch, " or ")) != 0) {
            /* split menu choice "male-rank or female-rank" into two distinct
               but otherwise identical rules, "male-rank" and "female-rank" */
            *p = '\0'; /* chop off " or female-rank" */
            /* new rule for male-rank */
            status_hilite_add_threshold(fld, &hilite);
            pline("Added hilite %s", status_hilite2str(&hilite));
            /* transfer female-rank to start of hilite.textmatch buffer */
            p += sizeof " or " - sizeof "";
            q = hilite.textmatch;
            while ((*q++ = *p++) != '\0') {
                continue;
            }
            /* proceed with normal addition of new rule */
        }
        status_hilite_add_threshold(fld, &hilite);
        pline("Added hilite %s", status_hilite2str(&hilite));
    }
    reset_status_hilites();
    return TRUE;
}

static boolean
status_hilite_remove(int id)
{
    struct _status_hilite_line_str *hlstr = status_hilite_str;

    while (hlstr && hlstr->id != id) {
        hlstr = hlstr->next;
    }
    if (!hlstr) {
        return FALSE;
    }

    if (hlstr->fld == BL_CONDITION) {
        int i;
        for (i = 0; i < CLR_MAX; i++) {
            gc.cond_hilites[i] &= ~hlstr->mask;
        }
        gc.cond_hilites[HL_ATTCLR_BOLD] &= ~hlstr->mask;
        gc.cond_hilites[HL_ATTCLR_DIM] &= ~hlstr->mask;
        gc.cond_hilites[HL_ATTCLR_ITALIC] &= ~hlstr->mask;
        gc.cond_hilites[HL_ATTCLR_ULINE] &= ~hlstr->mask;
        gc.cond_hilites[HL_ATTCLR_BLINK] &= ~hlstr->mask;
        gc.cond_hilites[HL_ATTCLR_INVERSE] &= ~hlstr->mask;
        return TRUE;
    } else {
        int fld = hlstr->fld;
        struct hilite_s *hl, *hlprev = (struct hilite_s *) 0;
        for (hl = gb.blstats[0][fld].thresholds; hl; hl = hl->next) {
            if (hlstr->hl == hl) {
                if (hlprev) {
                    hlprev->next = hl->next;
                } else {
                    gb.blstats[0][fld].thresholds = hl->next;
                    gb.blstats[1][fld].thresholds = gb.blstats[0][fld].thresholds;
                }
                if (gb.blstats[0][fld].hilite_rule == hl) {
                    gb.blstats[0][fld].hilite_rule = gb.blstats[1][fld].hilite_rule
                        = (struct hilite_s *) 0;
                    gb.blstats[0][fld].time = gb.blstats[1][fld].time = 0L;
                }
                free((genericptr_t) hl);
                return TRUE;
            }
            hlprev = hl;
        }
    }
    return FALSE;
}


static boolean
status_hilite_menu_fld(int fld)
{
    winid tmpwin;
    int i, res;
    menu_item *picks = (menu_item *) 0;
    anything any;
    int count = status_hilite_linestr_countfield(fld);
    struct _status_hilite_line_str *hlstr;
    char buf[BUFSZ];
    boolean acted;

    if (!count) {
        if (status_hilite_menu_add(fld)) {
            status_hilite_linestr_done();
            status_hilite_linestr_gather();
            count = status_hilite_linestr_countfield(fld);
        } else {
            return FALSE;
        }
    }

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    if (count) {
        hlstr = status_hilite_str;
        while (hlstr) {
            if (hlstr->fld == fld) {
                any = zeroany;
                any.a_int = hlstr->id;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                         hlstr->str, FALSE);
            }
            hlstr = hlstr->next;
        }
    } else {
        Sprintf(buf, "No current hilites for %s", initblstats[fld].fldname);
        add_menu_str(tmpwin, buf);
    }

    /* separator line */
    add_menu_str(tmpwin, "");

    if (count) {
        any = zeroany;
        any.a_int = -1;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'X', 0, ATR_NONE,
                 "Remove selected hilites", FALSE);
    }

#ifndef SCORE_ON_BOTL
    if (fld == BL_SCORE) {
        /* suppress 'Z - Add a new hilite' for 'score' when SCORE_ON_BOTL
           is disabled; we wouldn't be called for 'score' unless it has
           hilite rules from the config file, so count must be positive
           (hence there's no risk that we're putting up an empty menu) */
        ;
    } else
#endif
    {
        any = zeroany;
        any.a_int = -2;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'Z', 0, ATR_NONE,
                "Add new hilites", FALSE);
    }

    Sprintf(buf, "Current %s hilites:", initblstats[fld].fldname);
    end_menu(tmpwin, buf);

    acted = FALSE;
    if ((res = select_menu(tmpwin, PICK_ANY, &picks)) > 0) {
        int idx;
        unsigned mode = 0;

        for (i = 0; i < res; i++) {
            idx = picks[i].item.a_int;
            if (idx == -1) {
                mode |= 1; /* delete selected hilites */
            } else if (idx == -2) {
                mode |= 2; /* create new hilites */
            }
        }
        if (mode & 1) { /* delete selected hilites */
            for (i = 0; i < res; i++) {
                idx = picks[i].item.a_int;
                if (idx > 0 && status_hilite_remove(idx))
                    acted = TRUE;
            }
        }
        if (mode & 2) { /* create new hilites */
            while (status_hilite_menu_add(fld)) {
                acted = TRUE;
            }
        }
        free((genericptr_t) picks), picks = 0;
    }
    destroy_nhwindow(tmpwin);

    return acted;
}


static void
status_hilites_viewall(void)
{
    winid datawin;
    struct _status_hilite_line_str *hlstr = status_hilite_str;
    char buf[BUFSZ];

    datawin = create_nhwindow(NHW_TEXT);
    while (hlstr) {
        Sprintf(buf, "OPTIONS=hilite_status: %.*s",
                (int) (BUFSZ - sizeof "OPTIONS=hilite_status: " - 1),
                hlstr->str);
        putstr(datawin, 0, buf);
        hlstr = hlstr->next;
    }
    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
}


boolean
status_hilite_menu(void)
{
    winid tmpwin;
    int i, fld, res;
    menu_item *picks = (menu_item *) 0;
    anything any;
    boolean redo;
    int countall;

 shlmenu_redo:
    redo = FALSE;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    status_hilite_linestr_gather();
    countall = status_hilite_linestr_countfield(BL_FLUSH);
    if (countall) {
        any = zeroany;
        any.a_int = -1;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 "View all hilites in config format", FALSE);

        add_menu_str(tmpwin, "");
    }

    for (i = 0; i < MAXBLSTATS; i++) {
        int count;
        char buf[BUFSZ];

        fld = initblstats[i].fld;
        count = status_hilite_linestr_countfield(fld);
#ifndef SCORE_ON_BOTL
        /* config file might contain rules for highlighting 'score'
           even when SCORE_ON_BOTL is disabled; if so, 'O' command
           menus will show them and allow deletions but not additions,
           otherwise, it won't show 'score' at all */
        if (fld == BL_SCORE && !count) {
            continue;
        }
#endif
        any = zeroany;
        any.a_int = fld + 1;
        Sprintf(buf, "%-18s", initblstats[i].fldname);
        if (count) {
            Sprintf(eos(buf), " (%d defined)", count);
        }
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 buf, FALSE);
    }

    end_menu(tmpwin, "Status hilites:");
    if ((res = select_menu(tmpwin, PICK_ONE, &picks)) > 0) {
        fld = picks->item.a_int - 1;
        if (fld < 0) {
            status_hilites_viewall();
        } else {
            if (status_hilite_menu_fld(fld)) {
                reset_status_hilites();
            }
        }
        free((genericptr_t) picks), picks = (menu_item *) 0;
        redo = TRUE;
    }

    destroy_nhwindow(tmpwin);
    countall = status_hilite_linestr_countfield(BL_FLUSH);
    status_hilite_linestr_done();

    /* fuzzer is unlikely to pick something useful within nested menus;
       limit it to one try */
    if (redo && !iflags.debug_fuzzer) {
        goto shlmenu_redo;
    }

    /* hilite_delta=='statushilites' does double duty:  it is the
       number of turns for temporary highlights to remain visible
       and also when non-zero it is the flag to enable highlighting */
    if (countall > 0 && !iflags.hilite_delta) {
        iflags.hilite_delta = 3L;
    }

    return TRUE;
}
#endif /* STATUS_HILITES */

/*botl.c*/
