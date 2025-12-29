/*  SCCS Id: @(#)windows.c  3.4 1996/05/19  */
/* Copyright (c) D. Cohrs, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif
#ifdef CURSES_GRAPHICS
extern struct window_procs curses_procs;
#endif
#ifdef X11_GRAPHICS
/* cannot just blindly include winX.h without including all of X11 stuff */
/* and must get the order of include files right.  Don't bother */
extern struct window_procs X11_procs;
extern void win_X11_init();
#endif
#ifdef QT_GRAPHICS
extern struct window_procs Qt_procs;
#endif
#ifdef GEM_GRAPHICS
#include "wingem.h"
#endif
#ifdef MAC
extern struct window_procs mac_procs;
#endif
#ifdef BEOS_GRAPHICS
extern struct window_procs beos_procs;
extern void be_win_init();
#endif
#ifdef AMIGA_INTUITION
extern struct window_procs amii_procs;
extern struct window_procs amiv_procs;
extern void ami_wininit_data();
#endif
#ifdef WIN32_GRAPHICS
extern struct window_procs win32_procs;
#endif
#ifdef GNOME_GRAPHICS
#include "winGnome.h"
extern struct window_procs Gnome_procs;
#endif
#ifdef MSWIN_GRAPHICS
extern struct window_procs mswin_procs;
#endif
#ifdef DUMMY_GRAPHICS
extern struct window_procs dummy_procs;
#endif
#ifdef LISP_GRAPHICS
#include "winlisp.h"
extern struct window_procs lisp_procs;
#endif

static void def_raw_print(const char *s);

NEARDATA struct window_procs windowprocs;

static
struct win_choices {
    struct window_procs *procs;
    void (*ini_routine)(void);     /* optional (can be 0) */
} winchoices[] = {
#ifdef TTY_GRAPHICS
    { &tty_procs, win_tty_init },
#endif
#ifdef CURSES_GRAPHICS
    { &curses_procs, 0 },
#endif
#ifdef X11_GRAPHICS
    { &X11_procs, win_X11_init },
#endif
#ifdef QT_GRAPHICS
    { &Qt_procs, 0 },
#endif
#ifdef GEM_GRAPHICS
    { &Gem_procs, win_Gem_init },
#endif
#ifdef MAC
    { &mac_procs, 0 },
#endif
#ifdef BEOS_GRAPHICS
    { &beos_procs, be_win_init },
#endif
#ifdef AMIGA_INTUITION
    { &amii_procs, ami_wininit_data },      /* Old font version of the game */
    { &amiv_procs, ami_wininit_data },      /* Tile version of the game */
#endif
#ifdef WIN32_GRAPHICS
    { &win32_procs, 0 },
#endif
#ifdef GNOME_GRAPHICS
    { &Gnome_procs, 0 },
#endif
#ifdef MSWIN_GRAPHICS
    { &mswin_procs, 0 },
#endif
#ifdef DUMMY_GRAPHICS
    { &dummy_procs, 0 },
#endif
#ifdef LISP_GRAPHICS
    { &lisp_procs, win_lisp_init },
#endif
    { 0, 0 }        /* must be last */
};

static void
def_raw_print(const char *s)
{
    puts(s);
}

void
choose_windows(const char *s)
{
    int i;

    for (i=0; winchoices[i].procs; i++) {
        if (!strcmpi(s, winchoices[i].procs->name)) {
            windowprocs = *winchoices[i].procs;
            if (winchoices[i].ini_routine) {
                (*winchoices[i].ini_routine)();
            }
            return;
        }
    }

    if (!windowprocs.win_raw_print) {
        windowprocs.win_raw_print = def_raw_print;
    }

    raw_printf("Window type %s not recognized.  Choices are:", s);
    for (i=0; winchoices[i].procs; i++) {
        raw_printf("        %s", winchoices[i].procs->name);
    }

    if (windowprocs.win_raw_print == def_raw_print) {
        nh_terminate(EXIT_SUCCESS);
    }
    wait_synch();
}

/*
 * tty_message_menu() provides a means to get feedback from the
 * --More-- prompt; other interfaces generally don't need that.
 */
/*ARGSUSED*/
char
genl_message_menu(char let UNUSED, int how UNUSED, const char *mesg)
{
    pline("%s", mesg);
    return 0;
}

/*ARGSUSED*/
void
genl_preference_update(const char *pref UNUSED)
{
    /* window ports are expected to provide
       their own preference update routine
       for the preference capabilities that
       they support.
       Just return in this genl one. */
}

/* insert a non-selectable, possibly highlighted line of text into a menu */
void
add_menu_heading(winid tmpwin, const char *buf)
{
    anything any = cg.zeroany;
#if NEXT_VERSION
    int attr = iflags.menu_headings.attr,
        color = iflags.menu_headings.color;

    /* suppress highlighting during end-of-game disclosure */
    if (program_state.gameover) {
        attr = ATR_NONE, color = NO_COLOR;
    }

#else
    int attr = ATR_NONE;
#endif

    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, '\0', '\0', attr,
             buf, MENU_UNSELECTED);
}

/* insert a non-selectable, unhighlighted line of text into a menu */
void
add_menu_str(winid tmpwin, const char *buf)
{
    anything any = zeroany;

    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, '\0', '\0', ATR_NONE,
             buf, MENU_ITEMFLAGS_NONE);
}

/****************************************************************************/
/* genl backward compat stuff                                               */
/****************************************************************************/

const char *status_fieldnm[MAXBLSTATS];
const char *status_fieldfmt[MAXBLSTATS];
char *status_vals[MAXBLSTATS];
boolean status_activefields[MAXBLSTATS];

void
genl_status_init(void)
{
    int i;

    for (i = 0; i < MAXBLSTATS; ++i) {
        status_vals[i] = (char *) alloc(MAXCO);
        *status_vals[i] = '\0';
        status_activefields[i] = FALSE;
        status_fieldfmt[i] = (const char *) 0;
    }
    /* Use a window for the genl version; backward port compatibility */
    WIN_STATUS = create_nhwindow(NHW_STATUS);
    display_nhwindow(WIN_STATUS, FALSE);
}

void
genl_status_finish(void)
{
    /* tear down routine */
    int i;

    /* free alloc'd memory here */
    for (i = 0; i < MAXBLSTATS; ++i) {
        if (status_vals[i]) {
            free((genericptr_t) status_vals[i]), status_vals[i] = (char *) 0;
        }
    }
}

void
genl_status_enablefield(
    int fieldidx,
    const char *nm,
    const char *fmt,
    boolean enable)
{
    status_fieldfmt[fieldidx] = fmt;
    status_fieldnm[fieldidx] = nm;
    status_activefields[fieldidx] = enable;
}

/* call once for each field, then call with BL_FLUSH to output the result */
void
genl_status_update(
    int idx,
    genericptr_t ptr,
    int chg UNUSED, int percent UNUSED,
    int color UNUSED, unsigned long *colormasks UNUSED)
{
    char newbot1[MAXCO], newbot2[MAXCO];
    long cond, *condptr = (long *) ptr;
    int i;
    unsigned pass, lndelta;
    enum statusfields idx1, idx2, *fieldlist;
    char *nb, *text = (char *) ptr;

    static enum statusfields fieldorder[][15] = {
        /* line one */
        { BL_TITLE, BL_STR, BL_DX, BL_CO, BL_IN, BL_WI, BL_CH, BL_ALIGN,
          BL_SCORE, BL_FLUSH, BL_FLUSH, BL_FLUSH, BL_FLUSH, BL_FLUSH,
          BL_FLUSH },
        /* line two, default order */
        { BL_LEVELDESC, BL_GOLD,
          BL_HP, BL_HPMAX, BL_ENE, BL_ENEMAX, BL_AC,
          BL_XP, BL_EXP, BL_HD,
          BL_TIME,
          BL_HUNGER, BL_CAP, BL_CONDITION,
          BL_FLUSH },
        /* move time to the end */
        { BL_LEVELDESC, BL_GOLD,
          BL_HP, BL_HPMAX, BL_ENE, BL_ENEMAX, BL_AC,
          BL_XP, BL_EXP, BL_HD,
          BL_HUNGER, BL_CAP, BL_CONDITION,
          BL_TIME, BL_FLUSH },
        /* move experience and time to the end */
        { BL_LEVELDESC, BL_GOLD,
          BL_HP, BL_HPMAX, BL_ENE, BL_ENEMAX, BL_AC,
          BL_HUNGER, BL_CAP, BL_CONDITION,
          BL_XP, BL_EXP, BL_HD, BL_TIME, BL_FLUSH },
        /* move level description plus gold and experience and time to end */
        { BL_HP, BL_HPMAX, BL_ENE, BL_ENEMAX, BL_AC,
          BL_HUNGER, BL_CAP, BL_CONDITION,
          BL_LEVELDESC, BL_GOLD, BL_XP, BL_EXP, BL_HD, BL_TIME, BL_FLUSH },
    };

    /* in case interface is using genl_status_update() but has not
       specified WC2_FLUSH_STATUS (status_update() for field values
       is buffered so final BL_FLUSH is needed to produce output) */
    windowprocs.wincap2 |= WC2_FLUSH_STATUS;

    if (idx >= 0) {
        if (!status_activefields[idx]) {
            return;
        }
        switch (idx) {
        case BL_CONDITION:
            cond = condptr ? *condptr : 0L;
            nb = status_vals[idx];
            *nb = '\0';
            if (cond & BL_MASK_STONE) {
                Strcpy(nb = eos(nb), " Stone");
            }
            if (cond & BL_MASK_SLIME) {
                Strcpy(nb = eos(nb), " Slime");
            }
            if (cond & BL_MASK_STRNGL) {
                Strcpy(nb = eos(nb), " Strngl");
            }
            if (cond & BL_MASK_FOODPOIS) {
                Strcpy(nb = eos(nb), " FoodPois");
            }
            if (cond & BL_MASK_TERMILL) {
                Strcpy(nb = eos(nb), " TermIll");
            }
            if (cond & BL_MASK_BLIND) {
                Strcpy(nb = eos(nb), " Blind");
            }
            if (cond & BL_MASK_DEAF) {
                Strcpy(nb = eos(nb), " Deaf");
            }
            if (cond & BL_MASK_STUN) {
                Strcpy(nb = eos(nb), " Stun");
            }
            if (cond & BL_MASK_CONF) {
                Strcpy(nb = eos(nb), " Conf");
            }
            if (cond & BL_MASK_HALLU) {
                Strcpy(nb = eos(nb), " Hallu");
            }
            if (cond & BL_MASK_LEV) {
                Strcpy(nb = eos(nb), " Lev");
            }
            if (cond & BL_MASK_FLY) {
                Strcpy(nb = eos(nb), " Fly");
            }
            if (cond & BL_MASK_RIDE) {
                Strcpy(nb = eos(nb), " Ride");
            }
            break;
        default:
            Sprintf(status_vals[idx],
                    status_fieldfmt[idx] ? status_fieldfmt[idx] : "%s",
                    text ? text : "");
            break;
        }
        return; /* processed one field other than BL_FLUSH */
    } /* (idx >= 0, thus not BL_FLUSH, BL_RESET, BL_CHARACTERISTICS) */

    /* does BL_RESET require any specific code to ensure all fields ? */

    if (!(idx == BL_FLUSH || idx == BL_RESET)) {
        return;
    }

    /* We've received BL_FLUSH; time to output the gathered data */
    nb = newbot1;
    *nb = '\0';
    /* BL_FLUSH is the only pseudo-index value we need to check for
       in the loop below because it is the only entry used to pad the
       end of the fieldorder array. We could stop on any
       negative (illegal) index, but this should be fine */
    for (i = 0; (idx1 = fieldorder[0][i]) != BL_FLUSH; ++i) {
        if (status_activefields[idx1]) {
            Strcpy(nb = eos(nb), status_vals[idx1]);
        }
    }
    /* if '$' is encoded, buffer length of \GXXXXNNNN is 9 greater than
       single char; we want to subtract that 9 when checking display length */
    lndelta = (status_activefields[BL_GOLD]
               && strstr(status_vals[BL_GOLD], "\\G")) ? 9 : 0;
    /* basic bot2 formats groups of second line fields into five buffers,
       then decides how to order those buffers based on comparing lengths
       of [sub]sets of them to the width of the map; we have more control
       here but currently emulate that behavior */
    for (pass = 1; pass <= 4; pass++) {
        fieldlist = fieldorder[pass];
        nb = newbot2;
        *nb = '\0';
        for (i = 0; (idx2 = fieldlist[i]) != BL_FLUSH; ++i) {
            if (status_activefields[idx2]) {
                const char *val = status_vals[idx2];

                switch (idx2) {
                case BL_HP: /* for pass 4, Hp comes first; mungspaces()
                               will strip the unwanted leading spaces */
                case BL_XP:
                case BL_HD:
                case BL_TIME:
                    Strcpy(nb = eos(nb), " ");
                    break;

                case BL_LEVELDESC:
                    /* leveldesc has no leading space, so if we've moved
                       it past the first position, provide one */
                    if (i != 0) {
                        Strcpy(nb = eos(nb), " ");
                    }
                    break;
                /*
                 * We want "  hunger encumbrance conditions"
                 *   or    "  encumbrance conditions"
                 *   or    "  hunger conditions"
                 *   or    "  conditions"
                 * 'hunger'      is either " " or " hunger_text";
                 * 'encumbrance' is either " " or " encumbrance_text";
                 * 'conditions'  is either ""  or " cond1 cond2...".
                 */
                case BL_HUNGER:
                    /* hunger==" " - keep it, end up with " ";
                       hunger!=" " - insert space and get "  hunger" */
                    if (strcmp(val, " ")) {
                        Strcpy(nb = eos(nb), " ");
                    }
                    break;

                case BL_CAP:
                    /* cap==" " - suppress it, retain "  hunger" or " ";
                       cap!=" " - use it, get "  hunger cap" or "  cap" */
                    if (!strcmp(val, " ")) {
                        ++val;
                    }
                    break;

                default:
                    break;
                }
                Strcpy(nb = eos(nb), val); /* status_vals[idx2] */
            } /* status_activefields[idx2] */

            if (idx2 == BL_CONDITION && pass < 4
                && strlen(newbot2) - lndelta > COLNO) {
                break; /* switch to next order */
            }
        } /* i */

        if (idx2 == BL_FLUSH) { /* made it past BL_CONDITION */
            if (pass > 1) {
                mungspaces(newbot2);
            }
            break;
        }
    } /* pass */
    curs(WIN_STATUS, 1, 0);
    putstr(WIN_STATUS, 0, newbot1);
    curs(WIN_STATUS, 1, 1);
    putmixed(WIN_STATUS, 0, newbot2); /* putmixed() due to GOLD glyph */
}

/*windows.c*/
