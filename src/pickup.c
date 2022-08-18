/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Contains code for picking objects up, and container use.
 */

#include "hack.h"

#define CONTAINED_SYM '>' /* from invent.c */

static void simple_look(struct obj *, boolean);
static boolean query_classes(char *, boolean *, boolean *,
                             const char *, struct obj *, boolean, int *);
static void check_here(boolean);
static boolean n_or_more(struct obj *);
static boolean all_but_uchain(struct obj *);
#if 0 /* not used */
static boolean allow_cat_no_uchain(struct obj *);
#endif
static int autopick(struct obj*, int, menu_item **);
static int count_categories(struct obj *, int);
static long carry_count(struct obj *, struct obj *, long, boolean, int *, int *);
static int lift_object(struct obj *, struct obj *, long *, boolean);
static boolean mbag_explodes(struct obj *, int);
static int in_container(struct obj *);
static int ck_bag(struct obj *);
static int out_container(struct obj *);
static void removed_from_icebox(struct obj *);
static long mbag_item_gone(int, struct obj *);
static int traditional_loot(boolean);
static int menu_loot(int, boolean);
static char in_or_out_menu(const char *, struct obj *, boolean, boolean, boolean, boolean);
static boolean able_to_loot(int, int, boolean);
static boolean mon_beside(int, int);
static int do_loot_cont(struct obj **, int, int);
static int dump_container(struct obj*, boolean);
static void del_sokoprize(void);

/* define for query_objlist() and autopickup() */
#define FOLLOW(curr, flags) \
    (((flags) & BY_NEXTHERE) ? (curr)->nexthere : (curr)->nobj)

#define GOLD_WT(n)      (((n) + 50L) / 100L)
/* if you can figure this out, give yourself a hearty pat on the back... */
#define GOLD_CAPACITY(w, n)  (((w) * -100L) - ((n) + 50L) - 1L)

/* A variable set in use_container(), to be used by the callback routines
   in_container() and out_container() from askchain() and use_container().
   Also used by menu_loot() and container_gone(). */
static NEARDATA struct obj *current_container;
static NEARDATA boolean abort_looting;
#define Icebox (current_container->otyp == ICE_BOX)

static const char moderateloadmsg[] = "You have a little trouble lifting";
static const char nearloadmsg[] = "You have much trouble lifting";
static const char overloadmsg[] = "You have extreme difficulty lifting";

/* BUG: this lets you look at cockatrice corpses while blind without
   touching them */
/* much simpler version of the look-here code; used by query_classes() */
static void
simple_look(
    struct obj *otmp, /**< list of objects */
    boolean here)     /**< flag for type of obj list linkage */
{
    /* Neither of the first two cases is expected to happen, since
     * we're only called after multiple classes of objects have been
     * detected, hence multiple objects must be present.
     */
    if (!otmp) {
        impossible("simple_look(null)");
    } else if (!(here ? otmp->nexthere : otmp->nobj)) {
        pline("%s", doname(otmp));
    } else {
        winid tmpwin = create_nhwindow(NHW_MENU);
        putstr(tmpwin, 0, "");
        do {
            putstr(tmpwin, 0, doname(otmp));
            otmp = here ? otmp->nexthere : otmp->nobj;
        } while (otmp);
        display_nhwindow(tmpwin, TRUE);
        destroy_nhwindow(tmpwin);
    }
}

int
collect_obj_classes(char *ilets, struct obj *otmp, boolean here, boolean (*filter) (struct obj *), int *itemcount)
{
    int iletct = 0;
    char c;

    *itemcount = 0;
    ilets[iletct] = '\0'; /* terminate ilets so that index() will work */
    while (otmp) {
        c = def_oc_syms[(int)otmp->oclass];
        if (!index(ilets, c) && (!filter || (*filter)(otmp)))
            ilets[iletct++] = c,  ilets[iletct] = '\0';
        *itemcount += 1;
        otmp = here ? otmp->nexthere : otmp->nobj;
    }

    return iletct;
}

/*
 * Suppose some '?' and '!' objects are present, but '/' objects aren't:
 *  "a" picks all items without further prompting;
 *  "A" steps through all items, asking one by one;
 *  "?" steps through '?' items, asking, and ignores '!' ones;
 *  "/" becomes 'A', since no '/' present;
 *  "?a" or "a?" picks all '?' without further prompting;
 *  "/a" or "a/" becomes 'A' since there aren't any '/'
 *      (bug fix:  3.1.0 thru 3.1.3 treated it as "a");
 *  "?/a" or "a?/" or "/a?",&c picks all '?' even though no '/'
 *      (ie, treated as if it had just been "?a").
 */
static boolean
query_classes(
    char oclasses[],
    boolean *one_at_a_time,
    boolean *everything,
    const char *action,
    struct obj *objs,
    boolean here,
    int *menu_on_demand)
{
    char ilets[20], inbuf[BUFSZ];
    int iletct, oclassct;
    boolean not_everything, filtered;
    char qbuf[QBUFSZ];
    boolean m_seen;
    int itemcount;

    oclasses[oclassct = 0] = '\0';
    *one_at_a_time = *everything = m_seen = FALSE;
    if (menu_on_demand) {
        *menu_on_demand = 0;
    }
    iletct = collect_obj_classes(ilets, objs, here, (boolean (*)(OBJ_P)) 0, &itemcount);
    if (iletct == 0) {
        return FALSE;
    } else if (iletct == 1) {
        oclasses[0] = def_char_to_objclass(ilets[0]);
        oclasses[1] = '\0';
    } else  {   /* more than one choice available */
        /* additional choices */
        ilets[iletct++] = ' ';
        ilets[iletct++] = 'a';
        ilets[iletct++] = 'A';
        ilets[iletct++] = (objs == invent ? 'i' : ':');
    }
    if (itemcount && menu_on_demand) {
        ilets[iletct++] = 'm';
    }
    if (count_unpaid(objs)) {
        ilets[iletct++] = 'u';
    }
    int bcnt, ucnt, ccnt, xcnt, ocnt;
    tally_BUCX(objs, here, &bcnt, &ucnt, &ccnt, &xcnt, &ocnt);
    if (bcnt) {
        ilets[iletct++] = 'B';
    }
    if (ucnt) {
        ilets[iletct++] = 'U';
    }
    if (ccnt) {
        ilets[iletct++] = 'C';
    }
    if (xcnt) {
        ilets[iletct++] = 'X';
    }
    ilets[iletct] = '\0';

    if (iletct > 1) {
        const char *where = 0;
        char sym, oc_of_sym, *p;

ask_again:
        oclasses[oclassct = 0] = '\0';
        *one_at_a_time = *everything = FALSE;
        not_everything = filtered = FALSE;
        Sprintf(qbuf, "What kinds of thing do you want to %s? [%s]",
                action, ilets);
        getlin(qbuf, inbuf);
        if (*inbuf == '\033') return FALSE;

        for (p = inbuf; (sym = *p++); ) {
            if (sym == ' ') continue;
            else if (sym == 'A') *one_at_a_time = TRUE;
            else if (sym == 'a') *everything = TRUE;
            else if (sym == ':') {
                simple_look(objs, here); /* dumb if objs==invent */
                /* if we just scanned the contents of a container
                   then mark it as having known contents */
                if (objs->where == OBJ_CONTAINED) {
                    objs->ocontainer->cknown = 1;
                }
                goto ask_again;
            } else if (sym == 'i') {
                (void) display_inventory((char *)0, TRUE);
                goto ask_again;
            } else if (sym == 'm') {
                m_seen = TRUE;
            } else if (index("uBUCX", sym)) {
                add_valid_menu_class(sym); /* 'u' or 'B','U','C',or 'X' */
                filtered = TRUE;
            } else {
                oc_of_sym = def_char_to_objclass(sym);
                if (index(ilets, sym)) {
                    add_valid_menu_class(oc_of_sym);
                    oclasses[oclassct++] = oc_of_sym;
                    oclasses[oclassct] = '\0';
                } else {
                    if (!where)
                        where = !strcmp(action, "pick up")  ? "here" :
                                !strcmp(action, "take out") ?
                                "inside" : "";
                    if (*where)
                        There("are no %c's %s.", sym, where);
                    else
                        You("have no %c's.", sym);
                    not_everything = TRUE;
                }
            }
        } /* for p:sym in inbuf */

        if (m_seen && menu_on_demand) {
            *menu_on_demand = (((*everything || !oclassct) && !filtered) ? -2 : -3);
            return FALSE;
        }
        if (!oclassct && (!*everything || not_everything)) {
            /* didn't pick anything,
               or tried to pick something that's not present */
            *one_at_a_time = TRUE;  /* force 'A' */
            *everything = FALSE;    /* inhibit 'a' */
        }
    }
    return TRUE;
}

/* check whether hero is bare-handedly touching a cockatrice corpse */
static boolean
fatal_corpse_mistake(struct obj *obj, boolean remotely)
{
    if (uarmg || remotely || obj->otyp != CORPSE ||
         !touch_petrifies(&mons[obj->corpsenm]) || Stone_resistance) {
        return FALSE;
    }

    if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM)) {
        display_nhwindow(WIN_MESSAGE, FALSE); /* --More-- */
        return FALSE;
    }

    pline("Touching %s is a fatal mistake.",
          corpse_xname(obj, (const char *) 0, CXN_SINGULAR | CXN_ARTICLE));
    instapetrify(killer_xname(obj));
    return TRUE;
}

/* attempting to manipulate a Rider's corpse triggers its revival */
boolean
rider_corpse_revival(struct obj *obj, boolean remotely)
{
    if (!obj || obj->otyp != CORPSE || !is_rider(&mons[obj->corpsenm])) {
        return FALSE;
    }

    pline("At your %s, the corpse suddenly moves...",
          remotely ? "attempted acquisition" : "touch");

    (void) revive_corpse(obj);
    exercise(A_WIS, FALSE);
    return TRUE;
}

/* look at the objects at our location, unless there are too many of them */
static void
check_here(boolean picked_some)
{
    struct obj *obj;
    int ct = 0;

    /* count the objects here */
    for (obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere) {
        if (obj != uchain)
            ct++;
    }

    /* If there are objects here, take a look. */
    if (ct) {
        if (flags.run) nomul(0, 0);
        flush_screen(1);
        (void) look_here(ct, picked_some);
    } else {
        read_engr_at(u.ux, u.uy);
    }
}

/* Value set by query_objlist() for n_or_more(). */
static long val_for_n_or_more;

/* query_objlist callback: return TRUE if obj's count is >= reference value */
static boolean
n_or_more(struct obj *obj)
{
    if (obj == uchain) return FALSE;
    return (obj->quan >= val_for_n_or_more);
}

/* list of valid menu classes for query_objlist() and allow_category callback
   (with room for all object classes, 'u'npaid, BUCX, and terminator) */
static char valid_menu_classes[MAXOCLASSES + 10];
static boolean class_filter, bucx_filter, shop_filter;
static boolean unidentified_filter, recently_picked_up_filter;

/* check valid_menu_classes[] for an entry; also used by askchain() */
boolean
menu_class_present(int c)
{
    return (c && index(valid_menu_classes, c)) ? TRUE : FALSE;
}

void
add_valid_menu_class(int c)
{
    static int vmc_count = 0;

    if (c == 0) {
        /* reset */
        vmc_count = 0;
        class_filter = bucx_filter = shop_filter = FALSE;
        unidentified_filter = recently_picked_up_filter = FALSE;
    } else if (!menu_class_present(c)) {
        valid_menu_classes[vmc_count++] = (char) c;
        /* categorize the new class */
        switch (c) {
        case 'B':
        case 'U':
        case 'C':
            /* fall through */

        case 'X':
            bucx_filter = TRUE;
            break;

        case 'u':
            shop_filter = TRUE;
            break;

        case 'I':
            unidentified_filter = TRUE;
            break;

        case 'P':
            recently_picked_up_filter = TRUE;
            break;

        default:
            class_filter = TRUE;
            break;
        }
    }
    valid_menu_classes[vmc_count] = '\0';
}

/* query_objlist callback: return TRUE if not uchain */
static boolean
all_but_uchain(struct obj *obj)
{
    return (obj != uchain);
}

/* query_objlist callback: return TRUE */
/*ARGSUSED*/
boolean
allow_all(struct obj *obj UNUSED)
{
    return TRUE;
}

boolean
allow_category(struct obj *obj)
{
    /* For coins, if any class filter is specified, accept if coins
     * are included regardless of whether either unpaid or BUC-status
     * is also specified since player has explicitly requested coins.
     * If no class filtering is specified but bless/curse state is,
     * coins are always ignored.
     */
    if (obj->oclass == COIN_CLASS && !recently_picked_up_filter) {
        return class_filter ? (index(valid_menu_classes, COIN_CLASS) ? TRUE : FALSE) :
               /* coins are never unpaid, but check anyway */
               shop_filter ? (obj->unpaid ? TRUE : FALSE) :
               bucx_filter ? FALSE :
               unidentified_filter ? FALSE :
               TRUE; /* catchall: no filters specified, so accept */
    }

    if (Role_if(PM_PRIEST) && !obj->bknown) {
        set_bknown(obj, 1);
    }
    /* if class is expected but obj's class is not in the list, reject */
    if (class_filter && !index(valid_menu_classes, obj->oclass)) {
        return FALSE;
    }
    /* if unpaid is expected and obj isn't unpaid, reject (treat a container
       holding any unpaid object as unpaid even if isn't unpaid itself) */
    if (shop_filter && !obj->unpaid && !(Has_contents(obj) && count_unpaid(obj->cobj) > 0)) {
        return FALSE;
    }

    if (bucx_filter) {
        /* first categorize this object's bless/curse state */
        char bucx = !obj->bknown ? 'X' :
                    obj->blessed ? 'B' :
                    obj->cursed ?  'C' : 'U';

        /* if its category is not in the list, reject */
        if (!index(valid_menu_classes, bucx)) {
            return FALSE;
        }
    }

    if (unidentified_filter && !not_fully_identified(obj)) {
        return FALSE;
    }

    if (recently_picked_up_filter &&
         get_most_recently_picked_up_turn() > obj->picked_up_turn) {
        return FALSE;
    }

    /* obj didn't fail any of the filter checks, so accept */
    return TRUE;
}

#if 0 /* not used */
/* query_objlist callback: return TRUE if valid category (class), no uchain */
static boolean
allow_cat_no_uchain(obj)
struct obj *obj;
{
    if ((obj != uchain) &&
        (((index(valid_menu_classes, 'u') != (char *)0) && obj->unpaid) ||
         (index(valid_menu_classes, obj->oclass) != (char *)0)))
        return TRUE;
    else
        return FALSE;
}
#endif

/* query_objlist callback: return TRUE if valid class and worn */
boolean
is_worn_by_type(struct obj *otmp)
{
    return (is_worn(otmp) && allow_category(otmp));
}

/**
 * Returns the most recent turn on which an item got picked up.
 */
int
get_most_recently_picked_up_turn(void)
{
    struct obj *list = invent;
    int turn = 0;

    while (list) {
        if (list->picked_up_turn > turn) {
            turn = list->picked_up_turn;
        }
        list = list->nobj;
    }
    return turn;
}


/**
 * Returns the number of most recently picked up items
 */
int
count_most_recently_picked_up(struct obj *olist)
{
    struct obj *otmp;
    int cnt = 0;
    int turn = get_most_recently_picked_up_turn();

    for (otmp = olist; otmp; otmp = otmp->nobj) {
        if (otmp->picked_up_turn == turn) {
            cnt++;
        }
    }

    return cnt;
}

struct obj *
find_most_recently_picked_up_obj(struct obj *olist)
{
    struct obj *otmp = NULL;
    int turn = get_most_recently_picked_up_turn();

    for (otmp = olist; otmp; otmp = otmp->nobj) {
        if (otmp->picked_up_turn == turn) {
            return otmp;
        }
    }

    return (struct obj *) 0;
}

/*
 * Have the hero pick things from the ground
 * or a monster's inventory if swallowed.
 *
 * Arg what:
 *  >0  autopickup
 *  =0  interactive
 *  <0  pickup count of something
 *
 * Returns 1 if tried to pick something up, whether
 * or not it succeeded.
 */
int
pickup(int what) /* should be a long */
{
    int i, n, res, count, n_tried = 0, n_picked = 0;
    menu_item *pick_list = (menu_item *) 0;
    boolean autopickup = what > 0;
    struct obj *objchain;
    int traverse_how;

    flags.last_picked_up_otyp = STRANGE_OBJECT;

    /* we might have arrived here while fainted or sleeping, via
       random teleport or levitation timeout; if so, skip check_here
       and read_engr_at in addition to bypassing autopickup itself
       [probably ought to check whether hero is using a cockatrice
       corpse for a pillow here... (also at initial faint/sleep)] */
    if (autopickup && multi < 0 && unconscious()) {
        return 0;
    }

    if (what < 0)       /* pick N of something */
        count = -what;
    else            /* pick anything */
        count = 0;

    if (!u.uswallow) {
        struct trap *ttmp = t_at(u.ux, u.uy);

        /* no auto-pick if no-pick move, nothing there, or in a pool */
        if (autopickup && (flags.nopick || !OBJ_AT(u.ux, u.uy) ||
                           (is_pool(u.ux, u.uy) && !Underwater) || is_lava(u.ux, u.uy))) {
            read_engr_at(u.ux, u.uy);
            return (0);
        }

        /* no pickup if levitating & not on air or water level */
        if (!can_reach_floor(TRUE)) {
            if ((multi && !flags.run) || (autopickup && !flags.pickup))
                read_engr_at(u.ux, u.uy);
            return (0);
        }
        if (ttmp && ttmp->tseen) {
            /* Allow pickup from holes and trap doors that you escaped
             * from because that stuff is teetering on the edge just
             * like you, but not pits, because there is an elevation
             * discrepancy with stuff in pits.
             */
            if ((ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT) &&
                (!u.utrap || (u.utrap && u.utraptype != TT_PIT))) {
                read_engr_at(u.ux, u.uy);
                return(0);
            }
        }
        /* multi && !flags.run means they are in the middle of some other
         * action, or possibly paralyzed, sleeping, etc.... and they just
         * teleported onto the object.  They shouldn't pick it up.
         */
        if ((multi && !flags.run) || (autopickup && !flags.pickup)) {
            check_here(FALSE);
            return (0);
        }
        if (notake(youmonst.data)) {
            if (!autopickup)
                You("are physically incapable of picking anything up.");
            else
                check_here(FALSE);
            return (0);
        }

        /* if there's anything here, stop running */
        if (OBJ_AT(u.ux, u.uy) && flags.run && flags.run != 8 && !flags.nopick) nomul(0, 0);
    }

    add_valid_menu_class(0);    /* reset */
    if (!u.uswallow) {
        objchain = level.objects[u.ux][u.uy];
        traverse_how = BY_NEXTHERE;
    } else {
        objchain = u.ustuck->minvent;
        traverse_how = 0;   /* nobj */
    }
    /*
     * Start the actual pickup process.  This is split into two main
     * sections, the newer menu and the older "traditional" methods.
     * Automatic pickup has been split into its own menu-style routine
     * to make things less confusing.
     */
    if (autopickup) {
        n = autopick(objchain, traverse_how, &pick_list);
        goto menu_pickup;
    }

    if (flags.menu_style != MENU_TRADITIONAL || iflags.menu_requested) {
        /* use menus exclusively */
        if (count) {    /* looking for N of something */
            char buf[QBUFSZ];
            Sprintf(buf, "Pick %d of what?", count);
            val_for_n_or_more = count; /* set up callback selector */
            n = query_objlist(buf, objchain,
                              traverse_how|AUTOSELECT_SINGLE|INVORDER_SORT,
                              &pick_list, PICK_ONE, n_or_more);
            /* correct counts, if any given */
            for (i = 0; i < n; i++)
                pick_list[i].count = count;
        } else {
            n = query_objlist("Pick up what?", objchain,
                              traverse_how|AUTOSELECT_SINGLE|INVORDER_SORT|FEEL_COCKATRICE,
                              &pick_list, PICK_ANY, all_but_uchain);
        }
menu_pickup:
        n_tried = n;
        for (n_picked = i = 0; i < n; i++) {
            res = pickup_object(pick_list[i].item.a_obj, pick_list[i].count,
                                FALSE);
            if (res < 0) break; /* can't continue */
            n_picked += res;
        }
        if (pick_list) free((genericptr_t)pick_list);

    } else {
        /* old style interface */
        int ct = 0;
        long lcount;
        boolean all_of_a_type, selective;
        char oclasses[MAXOCLASSES + 20]; /* +10: room for B,U,C,X plus slop */
        struct obj *obj, *obj2;

        oclasses[0] = '\0';     /* types to consider (empty for all) */
        all_of_a_type = TRUE;   /* take all of considered types */
        selective = FALSE;      /* ask for each item */

        /* check for more than one object */
        for (obj = objchain;
             obj; obj = (traverse_how == BY_NEXTHERE) ? obj->nexthere : obj->nobj)
            ct++;

        if (ct == 1 && count) {
            /* if only one thing, then pick it */
            obj = objchain;
            lcount = min(obj->quan, (long)count);
            n_tried++;
            if (pickup_object(obj, lcount, FALSE) > 0)
                n_picked++; /* picked something */
            goto end_query;

        } else if (ct >= 2) {
            int via_menu = 0;

            There("are %s objects here.", (ct <= 10) ? "several" : "many");
            if (!query_classes(oclasses, &selective, &all_of_a_type,
                               "pick up", objchain,
                               (traverse_how & BY_NEXTHERE) ? TRUE : FALSE,
                               &via_menu)) {
                if (!via_menu) {
                    goto pickupdone;
                }
                if (selective) {
                    traverse_how |= INVORDER_SORT;
                }
                n = query_objlist("Pick up what?",
                                  objchain,
                                  traverse_how,
                                  &pick_list, PICK_ANY,
                                  via_menu == -2 ? allow_all : allow_category);
                goto menu_pickup;
            }
        }
        boolean bycat = (menu_class_present('B') ||
                         menu_class_present('U') ||
                         menu_class_present('C') ||
                         menu_class_present('X'));

        for (obj = objchain; obj; obj = obj2) {
            obj2 = FOLLOW(obj, traverse_how);
            if (bycat ? !allow_category(obj) :
                        (!selective && oclasses[0] && !index(oclasses, obj->oclass))) {
                continue;
            }

            lcount = -1L;
            if (!all_of_a_type) {
                char qbuf[BUFSZ];

                (void) safe_qbuf(qbuf, "Pick up ", "?", obj, doname,
                                 ansimpleoname, something);
                switch ((obj->quan < 2L) ? ynaq(qbuf) : ynNaq(qbuf)) {
                case 'q': goto end_query; /* out 2 levels */
                case 'n': continue;
                case 'a':
                    all_of_a_type = TRUE;
                    if (selective) {
                        selective = FALSE;
                        oclasses[0] = obj->oclass;
                        oclasses[1] = '\0';
                    }
                    break;

                case '#': /* count was entered */
                    if (!yn_number) continue; /* 0 count => No */
                    lcount = (long) yn_number;
                    if (lcount > obj->quan) lcount = obj->quan;
                    /* fall thru */

                default: /* 'y' */
                    break;
                }
            }
            if (lcount == -1L) lcount = obj->quan;

            n_tried++;
            if ((res = pickup_object(obj, lcount, FALSE)) < 0) break;
            n_picked += res;
        }
end_query:
        ;   /* semicolon needed by brain-damaged compilers */
    }

    if (!u.uswallow) {
        if (hides_under(youmonst.data)) {
            (void) hideunder(&youmonst);
        }

        /* position may need updating (invisible hero) */
        if (n_picked) {
            newsym_force(u.ux, u.uy);
        }

        /* see whether there's anything else here, after auto-pickup is done */
        if (autopickup) check_here(n_picked > 0);
    }

 pickupdone:
    add_valid_menu_class(0); /* reset */
    return (n_tried > 0);
}

#ifdef AUTOPICKUP_EXCEPTIONS
struct autopickup_exception *
check_autopickup_exceptions(struct obj *obj)
{
    /*
     *  Does the text description of this match an exception?
     */
    struct autopickup_exception *ape = apelist;

    if (ape) {
        char *objdesc = makesingular(doname(obj));

        while (ape && !regex_match(objdesc, ape->regex)) {
            ape = ape->next;
        }
    }
    return ape;
}
#endif /* AUTOPICKUP_EXCEPTIONS */

boolean
autopick_testobj(struct obj *otmp, boolean calc_costly)
{
    static boolean costly = FALSE;
    const char *otypes = flags.pickup_types;
    boolean pickit;

    /* calculate 'costly' just once for a given autopickup operation */
    if (calc_costly) {
        costly = (otmp->where == OBJ_FLOOR
                  && costly_spot(otmp->ox, otmp->oy));
    }

    /* first check: reject if an unpaid item in a shop */
    if (costly && !otmp->no_charge) {
        return FALSE;
    }

    /* check for pickup_types */
    pickit = (!*otypes || index(otypes, otmp->oclass));

#ifdef AUTOPICKUP_EXCEPTIONS
    /* check for autopickup exceptions */
    struct autopickup_exception *ape = check_autopickup_exceptions(otmp);
    if (ape) {
        pickit = ape->grab;
    }
#endif

    /* pickup_thrown overrides pickup_types and exceptions */
    if (!pickit) {
        pickit = (flags.pickup_thrown && otmp->was_thrown);
    }

    /* don't pick up dropped items */
    if (pickit) {
        pickit = flags.pickup_dropped || !otmp->was_dropped;
    }

    /* never pick up sokoban prize */
    if (Is_sokoprize(otmp)) {
        pickit = FALSE;
    }

    return pickit;
}

/*
 * Pick from the given list using flags.pickup_types.  Return the number
 * of items picked (not counts).  Create an array that returns pointers
 * and counts of the items to be picked up.  If the number of items
 * picked is zero, the pickup list is left alone.  The caller of this
 * function must free the pickup list.
 */
static int
autopick(
    struct obj *olist,     /**< the object list */
    int follow,            /**< how to follow the object list */
    menu_item **pick_list) /**< list of objects and counts to pick up */
{
    menu_item *pi; /* pick item */
    struct obj *curr;
    int n;
    boolean check_costly = TRUE;

    /* first count the number of eligible items */
    for (n = 0, curr = olist; curr; curr = FOLLOW(curr, follow)) {
        if (autopick_testobj(curr, check_costly)) {
            ++n;
        }
        check_costly = FALSE; /* only need to check once per autopickup */
    }

    if (n) {
        *pick_list = pi = (menu_item *) alloc(sizeof (menu_item) * n);
        for (n = 0, curr = olist; curr; curr = FOLLOW(curr, follow)) {
            if (autopick_testobj(curr, FALSE)) {
                pi[n].item.a_obj = curr;
                pi[n].count = curr->quan;
                n++;
            }
        }
    }

    return n;
}


/*
 * Put up a menu using the given object list.  Only those objects on the
 * list that meet the approval of the allow function are displayed.  Return
 * a count of the number of items selected, as well as an allocated array of
 * menu_items, containing pointers to the objects selected and counts.  The
 * returned counts are guaranteed to be in bounds and non-zero.
 *
 * Query flags:
 *      BY_NEXTHERE       - Follow object list via nexthere instead of nobj.
 *      AUTOSELECT_SINGLE - Don't ask if only 1 object qualifies - just
 *                          use it.
 *      USE_INVLET        - Use object's invlet.
 *      INVORDER_SORT     - Use hero's pack order.
 *      INCLUDE_HERO      - Showing engulfer's invent; show hero too.
 *      SIGNAL_NOMENU     - Return -1 rather than 0 if nothing passes "allow".
 *      SIGNAL_ESCAPE     - Return -1 rather than 0 if player uses ESC to
 *                          pick nothing.
 *      FEEL_COCKATRICE   - touch corpse.
 */
int
query_objlist(
    const char *qstr,        /**< query string */
    struct obj *olist,       /**< the list to pick from */
    int qflags,              /**< options to control the query */
    menu_item **pick_list,   /**< return list of items picked */
    int how,                 /**< type of query */
    boolean (*allow)(OBJ_P)) /**< allow function */
{
    int i;
#ifdef SORTLOOT
    int j;
#endif
    int n;
    winid win;
    struct obj *curr, *last;
#ifdef SORTLOOT
    struct obj **oarray;
#endif
    char *pack;
    anything any;
    boolean printed_type_name;

    *pick_list = (menu_item *) 0;
    if (!olist) return 0;

    /* count the number weight of items allowed */
    long weight_classes[MAXOCLASSES] = { 0 };
    for (n = 0, last = 0, curr = olist; curr; curr = FOLLOW(curr, qflags))
        if ((*allow)(curr)) {
            last = curr;
            n++;
            long weight = display_weight(curr);
            weight_classes[(int)curr->oclass] += weight;
        }

    if (n == 0) /* nothing to pick here */
        return (qflags & SIGNAL_NOMENU) ? -1 : 0;

    if (n == 1 && (qflags & AUTOSELECT_SINGLE)) {
        *pick_list = (menu_item *) alloc(sizeof(menu_item));
        (*pick_list)->item.a_obj = last;
        (*pick_list)->count = last->quan;
        return 1;
    }

#ifdef SORTLOOT
    /* Make a temporary array to store the objects sorted */
    oarray = (struct obj **)alloc(n*sizeof(struct obj*));

    /* Add objects to the array */
    i = 0;
    for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
        if ((*allow)(curr)) {
            if (iflags.sortloot == 'f' ||
                (iflags.sortloot == 'l' && !(qflags & USE_INVLET)))
            {
                /* Insert object at correct index */
                for (j = i; j; j--)
                {
                    if (sortloot_cmp(curr, oarray[j-1])>0) break;
                    oarray[j] = oarray[j-1];
                }
                oarray[j] = curr;
                i++;
            } else {
                /* Just add it to the array */
                oarray[i++] = curr;
            }
        }
    }
#endif /* SORTLOOT */

    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    any.a_obj = (struct obj *) 0;

    /*
     * Run through the list and add the objects to the menu.  If
     * INVORDER_SORT is set, we'll run through the list once for
     * each type so we can group them.  The allow function will only
     * be called once per object in the list.
     */
    pack = flags.inv_order;
    do {
        printed_type_name = FALSE;
#ifdef SORTLOOT
        for (i = 0; i < n; i++) {
            curr = oarray[i];
#else /* SORTLOOT */
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
#endif /* SORTLOOT */
            if ((qflags & FEEL_COCKATRICE) && curr->otyp == CORPSE &&
                will_feel_cockatrice(curr, FALSE)) {
                destroy_nhwindow(win); /* stop the menu and revert */
                (void) look_here(0, FALSE);
                return 0;
            }
            if ((!(qflags & INVORDER_SORT) || curr->oclass == *pack)
                && (*allow)(curr)) {

                /* if sorting, print type name (once only) */
                if (qflags & INVORDER_SORT && !printed_type_name) {
                    char buf[BUFSZ];
                    Sprintf(buf, "%s", let_to_name(*pack, FALSE));
                    if (weight_classes[(int)curr->oclass] > 0) {
                        Sprintf(eos(buf), " (%ld aum)", weight_classes[(int)curr->oclass]);
                    }
                    any.a_obj = (struct obj *) 0;
                    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
                             buf, MENU_UNSELECTED);
                    printed_type_name = TRUE;
                }

                any.a_obj = curr;
                add_menu(win, obj_to_glyph(curr), curr->quan, &any,
                         qflags & USE_INVLET ? curr->invlet : 0,
                         def_oc_syms[(int)objects[curr->otyp].oc_class],
                         ATR_NONE, doname_with_price(curr), MENU_UNSELECTED);
            }
        }
        pack++;
    } while (qflags & INVORDER_SORT && *pack);

#ifdef SORTLOOT
    free(oarray);
#endif
    end_menu(win, qstr);
    n = select_menu(win, how, pick_list);
    destroy_nhwindow(win);

    if (n > 0) {
        menu_item *mi;

        /* fix up counts:  -1 means no count used => pick all */
        for (i = 0, mi = *pick_list; i < n; i++, mi++)
            if (mi->count == -1L || mi->count > mi->item.a_obj->quan)
                mi->count = mi->item.a_obj->quan;
    } else if (n < 0) {
        n = 0;  /* caller's don't expect -1 */
    }
    return n;
}

/*
 * allow menu-based category (class) selection (for Drop,take off etc.)
 *
 */
int
query_category(
    const char *qstr,      /**< query string */
    struct obj *olist,     /**< the list to pick from */
    int qflags,            /**< behaviour modification flags */
    menu_item **pick_list, /**< return list of items picked */
    int how)               /**< type of query */
{
    int n;
    winid win;
    struct obj *curr;
    char *pack;
    anything any;
    boolean collected_type_name;
    char invlet;
    int ccount;
    boolean (*ofilter) (OBJ_P) = (boolean (*) (OBJ_P)) 0;
    boolean do_unpaid = FALSE;
    boolean do_unidentified = FALSE;
    boolean do_blessed = FALSE, do_cursed = FALSE, do_uncursed = FALSE,
            do_buc_unknown = FALSE;
    int num_most_recently_picked_up = 0;
    int num_buc_types = 0;

    *pick_list = (menu_item *) 0;
    if (!olist) return 0;
    if ((qflags & UNPAID_TYPES) && count_unpaid(olist)) do_unpaid = TRUE;
    if ((qflags & UNIDENTIFIED_TYPES) && count_unidentified(olist)) do_unidentified = TRUE;
    if (qflags & RECENTLY_PICKED_UP) {
        num_most_recently_picked_up = count_most_recently_picked_up(olist);
    }
    if (qflags & WORN_TYPES) {
        ofilter = is_worn;
    }
    if ((qflags & BUC_BLESSED) && count_buc(olist, BUC_BLESSED, ofilter)) {
        do_blessed = TRUE;
        num_buc_types++;
    }
    if ((qflags & BUC_CURSED) && count_buc(olist, BUC_CURSED, ofilter)) {
        do_cursed = TRUE;
        num_buc_types++;
    }
    if ((qflags & BUC_UNCURSED) && count_buc(olist, BUC_UNCURSED, ofilter)) {
        do_uncursed = TRUE;
        num_buc_types++;
    }
    if ((qflags & BUC_UNKNOWN) && count_buc(olist, BUC_UNKNOWN, ofilter)) {
        do_buc_unknown = TRUE;
        num_buc_types++;
    }

    ccount = count_categories(olist, qflags);
    /* no point in actually showing a menu for a single category */
    if (ccount == 1 && !do_unpaid && num_buc_types <= 1 && !(qflags & BILLED_TYPES)) {
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if (ofilter && !(*ofilter)(curr)) {
                continue;
            }
            break;
        }
        if (curr) {
            *pick_list = (menu_item *) alloc(sizeof(menu_item));
            (*pick_list)->item.a_int = curr->oclass;
            return 1;
        } else {
#ifdef DEBUG
            impossible("query_category: no single object match");
#endif
        }
        return 0;
    }

    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    pack = flags.inv_order;

    if (qflags & CHOOSE_ALL) {
        invlet = 'A';
        any = zeroany;
        any.a_int = 'A';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 (qflags & WORN_TYPES) ?
                 "Auto-select every item being worn" :
                 "Auto-select every item", MENU_UNSELECTED);

        /* empty line */
        any = zeroany;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    }

    if ((qflags & ALL_TYPES) && (ccount > 1)) {
        invlet = 'a';
        any = zeroany;
        any.a_int = ALL_TYPES_SELECTED;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 (qflags & WORN_TYPES) ? "All worn types" : "All types",
                 MENU_UNSELECTED);
        invlet = 'b';
    } else
        invlet = 'a';
    do {
        collected_type_name = FALSE;
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if (curr->oclass == *pack) {
                if (ofilter && !(*ofilter)(curr)) {
                    continue;
                }
                if (!collected_type_name) {
                    any = zeroany;
                    any.a_int = curr->oclass;
                    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet++,
                             def_oc_syms[(int)objects[curr->otyp].oc_class],
                             ATR_NONE, let_to_name(*pack, FALSE),
                             MENU_UNSELECTED);
                    collected_type_name = TRUE;
                }
            }
        }
        pack++;
        if (invlet >= 'u') {
            impossible("query_category: too many categories");
            n = 0;
            goto query_done;
        }
    } while (*pack);

    if (do_unpaid || (qflags & BILLED_TYPES) ||
         do_blessed || do_cursed || do_uncursed || do_buc_unknown ||
         do_unidentified) {
        any = zeroany;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    }

    /* unpaid items if there are any */
    if (do_unpaid) {
        invlet = 'u';
        any = zeroany;
        any.a_int = 'u';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Unpaid items", MENU_UNSELECTED);
    }
    /* billed items: checked by caller, so always include if BILLED_TYPES */
    if (qflags & BILLED_TYPES) {
        invlet = 'x';
        any = zeroany;
        any.a_int = 'x';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Unpaid items already used up", MENU_UNSELECTED);
    }
    /* unidentified items if there are any */
    if (do_unidentified) {
        invlet = 'I';
        any = zeroany;
        any.a_int = 'I';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Unidentified items", MENU_UNSELECTED);
    }
    /* items with b/u/c/unknown if there are any;
       this cluster of menu entries is in alphabetical order,
       reversing the usual sequence of 'U' and 'C' in BUCX */
    if (do_blessed) {
        invlet = 'B';
        any = zeroany;
        any.a_int = 'B';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Items known to be Blessed", MENU_UNSELECTED);
    }
    if (do_cursed) {
        invlet = 'C';
        any = zeroany;
        any.a_int = 'C';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Items known to be Cursed", MENU_UNSELECTED);
    }
    if (do_uncursed) {
        invlet = 'U';
        any = zeroany;
        any.a_int = 'U';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Items known to be Uncursed", MENU_UNSELECTED);
    }
    if (do_buc_unknown) {
        invlet = 'X';
        any = zeroany;
        any.a_int = 'X';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE,
                 "Items of unknown B/C/U status",
                 MENU_UNSELECTED);
    }
    /* most recently picked up items */
    if (num_most_recently_picked_up) {
        char tmpbuf[BUFSZ];

        if (num_most_recently_picked_up == 1) {
            Sprintf(tmpbuf, "Most recently picked up: %s", doname(find_most_recently_picked_up_obj(olist)));
        } else {
            Sprintf(tmpbuf, "Items most recently picked up");
        }
        invlet = 'P';
        any = zeroany;
        any.a_int = 'P';
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, invlet, 0, ATR_NONE, tmpbuf, MENU_UNSELECTED);
    }
    end_menu(win, qstr);
    n = select_menu(win, how, pick_list);
 query_done:
    destroy_nhwindow(win);
    if (n < 0)
        n = 0;  /* caller's don't expect -1 */
    return n;
}

static int
count_categories(struct obj *olist, int qflags)
{
    char *pack;
    boolean counted_category;
    int ccount = 0;
    struct obj *curr;

    pack = flags.inv_order;
    do {
        counted_category = FALSE;
        for (curr = olist; curr; curr = FOLLOW(curr, qflags)) {
            if (curr->oclass == *pack) {
                if ((qflags & WORN_TYPES) &&
                    !(curr->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL |
                                         W_WEP | W_SWAPWEP | W_QUIVER)))
                    continue;
                if (!counted_category) {
                    ccount++;
                    counted_category = TRUE;
                }
            }
        }
        pack++;
    } while (*pack);
    return ccount;
}

/*
 *  How much the weight of the given container will change when the given
 *  object is removed from it.  Use before and after weight amounts rather
 *  than trying to match the calculation used by weight() in mkobj.c.
 */
static int
delta_cwt(struct obj *container, struct obj *obj)
{
    struct obj **prev;
    int owt, nwt;

    if (container->otyp != BAG_OF_HOLDING) {
        return obj->owt;
    }

    owt = nwt = container->owt;
    /* find the object so that we can remove it */
    for (prev = &container->cobj; *prev; prev = &(*prev)->nobj) {
        if (*prev == obj) {
            break;
        }
    }
    if (!*prev) {
        panic("delta_cwt: obj not inside container?");
    } else {
        /* temporarily remove the object and calculate resulting weight */
        *prev = obj->nobj;
        nwt = weight(container);
        *prev = obj; /* put the object back; obj->nobj is still valid */
    }
    return owt - nwt;
}

/* could we carry `obj'? if not, could we carry some of it/them? */
static long
carry_count(struct obj *obj, struct obj *container, long int count, boolean telekinesis, int *wt_before, int *wt_after)
                             /* object to pick up, bag it's coming out of */



{
    boolean adjust_wt = container && carried(container),
            is_gold = obj->oclass == COIN_CLASS;
    int wt, iw, ow, oow;
    long qq, savequan;
    long umoney = money_cnt(invent);
    unsigned saveowt;
    const char *verb, *prefx1, *prefx2, *suffx;
    char obj_nambuf[BUFSZ], where[BUFSZ];

    savequan = obj->quan;
    saveowt = obj->owt;

    iw = max_capacity();

    if (count != savequan) {
        obj->quan = count;
        obj->owt = (unsigned)weight(obj);
    }
    wt = iw + (int)obj->owt;
    if (adjust_wt) {
        wt -= delta_cwt(container, obj);
    }

    /* This will go with silver+copper & new gold weight */
    if (is_gold)    /* merged gold might affect cumulative weight */
        wt -= (GOLD_WT(umoney) + GOLD_WT(count) - GOLD_WT(umoney + count));

    if (count != savequan) {
        obj->quan = savequan;
        obj->owt = saveowt;
    }
    *wt_before = iw;
    *wt_after  = wt;

    if (wt < 0)
        return count;

    /* see how many we can lift */
    if (is_gold) {
        iw -= (int)GOLD_WT(umoney);
        if (!adjust_wt) {
            qq = GOLD_CAPACITY((long)iw, umoney);
        } else {
            oow = 0;
            qq = 50L - (umoney % 100L) - 1L;
            if (qq < 0L) qq += 100L;
            for ( ; qq <= count; qq += 100L) {
                obj->quan = qq;
                obj->owt = (unsigned)GOLD_WT(qq);
                ow = (int)GOLD_WT(umoney + qq);
                ow -= delta_cwt(container, obj);
                if (iw + ow >= 0) break;
                oow = ow;
            }
            iw -= oow;
            qq -= 100L;
        }
        if (qq < 0L) qq = 0L;
        else if (qq > count) qq = count;
        wt = iw + (int)GOLD_WT(umoney + qq);
    } else if (count > 1 || count < obj->quan) {
        /*
         * Ugh. Calc num to lift by changing the quan of of the
         * object and calling weight.
         *
         * This works for containers only because containers
         * don't merge.     -dean
         */
        for (qq = 1L; qq <= count; qq++) {
            obj->quan = qq;
            obj->owt = (unsigned)(ow = weight(obj));
            if (adjust_wt) {
                ow -= delta_cwt(container, obj);
            }
            if (iw + ow >= 0)
                break;
            wt = iw + ow;
        }
        --qq;
    } else {
        /* there's only one, and we can't lift it */
        qq = 0L;
    }
    obj->quan = savequan;
    obj->owt = saveowt;

    if (qq < count) {
        /* some message will be given */
        Strcpy(obj_nambuf, doname(obj));
        if (container) {
            Sprintf(where, "in %s", the(xname(container)));
            verb = "carry";
        } else {
            Strcpy(where, "lying here");
            verb = telekinesis ? "acquire" : "lift";
        }
    } else {
        /* lint suppression */
        *obj_nambuf = *where = '\0';
        verb = "";
    }
    /* we can carry qq of them */
    if (qq > 0) {
        if (qq < count)
            You("can only %s %s of the %s %s.",
                verb, (qq == 1L) ? "one" : "some", obj_nambuf, where);
        *wt_after = wt;
        return qq;
    }

    if (!container) Strcpy(where, "here");  /* slightly shorter form */
    if (invent || umoney) {
        prefx1 = "you cannot ";
        prefx2 = "";
        suffx  = " any more";
    } else {
        prefx1 = (obj->quan == 1L) ? "it " : "even one ";
        prefx2 = "is too heavy for you to ";
        suffx  = "";
    }
    There("%s %s %s, but %s%s%s%s.", otense(obj, "are"), obj_nambuf, where,
          prefx1, prefx2, verb, suffx);

    /* *wt_after = iw; */
    return 0L;
}

/* determine whether character is able and player is willing to carry `obj' */
static int
lift_object(struct obj *obj, struct obj *container, long int *cnt_p, boolean telekinesis)
                             /* object to pick up, bag it's coming out of */


{
    int result, old_wt, new_wt, prev_encumbr, next_encumbr;

    if (obj->otyp == BOULDER && In_sokoban(&u.uz)) {
        You("cannot get your %s around this %s.",
            body_part(HAND), xname(obj));
        return -1;
    }
    /* override weight consideration for loadstone picked up by anybody
       and for boulder picked up by hero poly'd into a giant; override
       availability of open inventory slot iff not already carrying one */
    if (obj->otyp == LOADSTONE ||
        (obj->otyp == BOULDER && throws_rocks(youmonst.data))) {
        if (inv_cnt(FALSE) < 52 || !carrying(obj->otyp) || merge_choice(invent, obj)) {
            return 1; /* lift regardless of current situation */
        }
        /* if we reach here, we're out of slots and already have at least
           one of these, so treat this one more like a normal item */
        You("are carrying too much stuff to pick up %s %s.",
            (obj->quan == 1L) ? "another" : "more", simpleonames(obj));
        return -1;
    }

    *cnt_p = carry_count(obj, container, *cnt_p, telekinesis, &old_wt, &new_wt);
    if (*cnt_p < 1L) {
        result = -1; /* nothing lifted */
    } else if (obj->oclass != COIN_CLASS &&
               /* [exception for gold coins will have to change
                   if silver/copper ones ever get implemented] */
               inv_cnt(FALSE) >= 52 && !merge_choice(invent, obj)) {
        /* if there is some gold here (and we haven't already skipped it),
           we aren't limited by the 52 item limit for it, but caller and
           "grandcaller" aren't prepared to skip stuff and then pickup
           just gold, so the best we can do here is vary the message */
        Your("knapsack cannot accommodate any more items%s.",
             /* floor follows by nexthere, otherwise container so by nobj */
             nxtobj(obj, GOLD_PIECE, (boolean) (obj->where == OBJ_FLOOR)) ? " (except gold)" : "");
        result = -1; /* nothing lifted */
    } else {
        result = 1;
        prev_encumbr = near_capacity();
        if (prev_encumbr < flags.pickup_burden)
            prev_encumbr = flags.pickup_burden;
        next_encumbr = calc_capacity(new_wt - old_wt);
        if (next_encumbr > prev_encumbr) {
            if (telekinesis) {
                result = 0; /* don't lift */
            } else {
                char qbuf[BUFSZ];
                long savequan = obj->quan;

                obj->quan = *cnt_p;
                Strcpy(qbuf,
                       (next_encumbr > HVY_ENCUMBER) ? overloadmsg :
                       (next_encumbr > MOD_ENCUMBER) ? nearloadmsg :
                       moderateloadmsg);

                if (container)
                    (void) strsubst(qbuf, "lifting", "removing");
                Strcat(qbuf, " ");
                (void) safe_qbuf(qbuf, qbuf, ".  Continue?", obj, doname,
                                 ansimpleoname, something);
                obj->quan = savequan;
                switch (ynq(qbuf)) {
                case 'q':  result = -1; break;
                case 'n':  result =  0; break;
                default:   break;/* 'y' => result == 1 */
                }
                clear_nhwindow(WIN_MESSAGE);
            }
        }
    }

    return result;
}

/*
 * Pick up <count> of obj from the ground and add it to the hero's inventory.
 * Returns -1 if caller should break out of its loop, 0 if nothing picked
 * up, 1 if otherwise.
 */
int
pickup_object(
    struct obj *obj,
    long count,
    boolean telekinesis) /**< not picking it up directly by hand */
{
    int res, nearload;

    if (obj->quan < count) {
        impossible("pickup_object: count %ld > quan %ld?",
                   count, obj->quan);
        return 0;
    }

    /* In case of auto-pickup, where we haven't had a chance
       to look at it yet; affects docall(SCR_SCARE_MONSTER). */
    if (!Blind)
#ifdef INVISIBLE_OBJECTS
        if (!obj->oinvis || See_invisible)
#endif
        obj->dknown = 1;

    if (obj == uchain) {    /* do not pick up attached chain */
        return 0;
    } else if (obj->where == OBJ_MINVENT && obj->owornmask != 0L &&
               u.uswallow && obj->ocarry == u.ustuck) {
        You_cant("pick %s up.", ysimple_name(obj));
        return 0;
    } else if (obj->oartifact && !touch_artifact(obj, &youmonst)) {
        return 0;
    } else if (obj->otyp == CORPSE) {
        if (fatal_corpse_mistake(obj, telekinesis) || rider_corpse_revival(obj, telekinesis)) {
            return -1;
        }
    } else if (obj->otyp == SCR_SCARE_MONSTER) {
        if (obj->blessed) {
            obj->blessed = 0;
        } else if (!obj->cursed) {
            obj->cursed = 1;
        } else {
            pline_The("scroll%s %s to dust as you %s %s up.",
                      plur(obj->quan), otense(obj, "turn"),
                      telekinesis ? "raise" : "pick",
                      (obj->quan == 1L) ? "it" : "them");
            makeknown(obj->otyp);
            flags.last_picked_up_otyp = obj->otyp;
            useupf(obj, obj->quan);
            return 1; /* tried to pick something up and failed, but
                         don't want to terminate pickup loop yet   */
        }
        /* BUC known but scroll still unknown */
        if (obj->bknown && !objects[obj->otyp].oc_name_known) {
            Your("%s %s briefly.", xname(obj), otense(obj, "vibrate"));
            makeknown(obj->otyp);
        }
    }

    if ((res = lift_object(obj, (struct obj *)0, &count, telekinesis)) <= 0)
        return res;

    /* Whats left of the special case for gold :-) */
    if (obj->oclass == COIN_CLASS) flags.botl = 1;

    if (obj->quan != count && obj->otyp != LOADSTONE)
        obj = splitobj(obj, count);

    obj = pick_obj(obj);

    if (uwep && uwep == obj) mrg_to_wielded = TRUE;
    nearload = near_capacity();
    prinv(nearload == SLT_ENCUMBER ? moderateloadmsg : (char *) 0,
          obj, count);
    mrg_to_wielded = FALSE;

    flags.last_picked_up_otyp = obj->otyp;
    obj->picked_up_turn = moves;

    if (Is_sokoprize(obj)) {
        makeknown(obj->otyp); /* obj is already known */
        obj->sokoprize = FALSE; /* reset sokoprize flag */
        del_sokoprize();    /* delete other sokoprizes */
        /* stop picking up other objects to prevent picking up
           one of the just deleted other sokoban prizes */
        return -1;
    }
    return 1;
}

/*
 * Do the actual work of picking otmp from the floor or monster's interior
 * and putting it in the hero's inventory.  Take care of billing.  Return a
 * pointer to the object where otmp ends up.  This may be different
 * from otmp because of merging.
 */
struct obj *
pick_obj(struct obj *otmp)
{
    int ox = otmp->ox, oy = otmp->oy;
    boolean robshop = (!u.uswallow && otmp != uball && costly_spot(ox, oy));

    obj_extract_self(otmp);
    newsym(ox, oy);

    /* for shop items, addinv() needs to be after addtobill() (so that
       object merger can take otmp->unpaid into account) but before
       remote_robbery() (which calls rob_shop() which calls setpaid()
       after moving costs of unpaid items to shop debt; setpaid()
       calls clear_unpaid() for lots of object chains, but 'otmp' isn't
       on any of those between obj_extract_self() and addinv(); for
       3.6.0, 'otmp' remained flagged as an unpaid item in inventory
       and triggered impossible() every time inventory was examined) */
    if (robshop) {
        char saveushops[5], fakeshop[2];

        /* addtobill cares about your location rather than the object's;
           usually they'll be the same, but not when using telekinesis
           (if ever implemented) or a grappling hook */
        Strcpy(saveushops, u.ushops);
        fakeshop[0] = *in_rooms(ox, oy, SHOPBASE);
        fakeshop[1] = '\0';
        Strcpy(u.ushops, fakeshop);
        /* sets obj->unpaid if necessary */
        addtobill(otmp, TRUE, FALSE, FALSE);
        Strcpy(u.ushops, saveushops);
        robshop = otmp->unpaid && !index(u.ushops, *fakeshop);
    }

    struct obj *result = addinv(otmp);

    /* if you're taking a shop item from outside the shop, make shk notice */
    if (robshop) {
        remote_burglary(ox, oy);
    }

    return result;
}

/*
 * prints a message if encumbrance changed since the last check and
 * returns the new encumbrance value (from near_capacity()).
 */
int
encumber_msg(void)
{
    static int oldcap = UNENCUMBERED;
    int newcap = near_capacity();

    if (oldcap < newcap) {
        switch (newcap) {
        case 1: Your("movements are slowed slightly because of your load.");
            break;

        case 2: You("rebalance your load.  Movement is difficult.");
            break;

        case 3: You("%s under your heavy load.  Movement is very hard.",
                    stagger(youmonst.data, "stagger"));
            break;

        default: You("%s move a handspan with this load!",
                     newcap == 4 ? "can barely" : "can't even");
            break;
        }
        flags.botl = 1;
    } else if(oldcap > newcap) {
        switch(newcap) {
        case 0: Your("movements are now unencumbered.");
            break;

        case 1: Your("movements are only slowed slightly by your load.");
            break;

        case 2: You("rebalance your load.  Movement is still difficult.");
            break;

        case 3: You("%s under your load.  Movement is still very hard.",
                    stagger(youmonst.data, "stagger"));
            break;
        }
        flags.botl = 1;
    }

    oldcap = newcap;
    return (newcap);
}

/* Is there a container at x,y. Optional: return count of containers at x,y */
int
container_at(int x, int y, boolean countem)
{
    struct obj *cobj, *nobj;
    int container_count = 0;

    for (cobj = level.objects[x][y]; cobj; cobj = nobj) {
        nobj = cobj->nexthere;
        if (Is_container(cobj)) {
            container_count++;
            if (!countem) break;
        }
    }
    return container_count;
}

/* If called to check for the ability to tip a container,
 * allow despite the lack of limbs or free hands, and do
 * not bother mentioning anything if not possible. This
 * difference in responses is because #loot can only be
 * used on items on the ground or adjacent, while #tip can
 * also be used on containers in the player's inventory.
 * We don't want to add annoying messages between checking
 * for tippability and bringing up the inventory prompt.
 */
static boolean
able_to_loot(
    int x,
    int y,
    boolean looting) /**< loot vs tip */
{
    const char *verb = looting ? "loot" : "tip";

    if (!can_reach_floor(TRUE)) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
            rider_cant_reach(); /* not skilled enough to reach */
        else
            cant_reach_floor(x, y, FALSE, TRUE);
        return FALSE;
    } else if ((is_pool(x, y) && (looting || !Underwater)) || is_lava(x, y)) {
        /* at present, can't loot in water even when Underwater;
           can tip underwater, but not when over--or stuck in--lava */
        You("cannot %s things that are deep in the %s.", verb,
            hliquid(is_lava(x, y) ? "lava" : "water"));
        return FALSE;
    } else if (nolimbs(youmonst.data)) {
        pline("Without limbs, you cannot %s anything.", verb);
        return FALSE;
    } else if (looting && !freehand()) {
        pline("Without a free %s, you cannot loot anything.",
                body_part(HAND));
        return FALSE;
    }
    return TRUE;
}

static boolean
mon_beside(int x, int y)
{
    int i, j, nx, ny;

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            nx = x + i;
            ny = y + j;
            if (isok(nx, ny) && MON_AT(nx, ny)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static int
do_loot_cont(
    struct obj **cobjp,
    int cindex, /**< index of this container (1..N)... */
    int container_count) /**< ...number of them (N) */
{
    struct obj *cobj = *cobjp;

    if (!cobj) {
        return 0;
    }
    if (cobj->olocked) {
        if (container_count < 2) {
            pline("%s locked.", cobj->lknown ? "It is" : "Hmmm, it seems to be");
        } else if (cobj->lknown) {
            pline("%s is locked.", The(xname(cobj)));
        } else {
            pline("Hmmm, %s seems to be locked.", the(xname(cobj)));
        }
        cobj->lknown = 1;
        /* autounlock only works properly with one container */
        if (flags.autounlock && container_count == 1) {
            struct obj *otmp;
            if (cobj->otyp == IRON_SAFE) {
                if ((otmp = carrying(STETHOSCOPE))) {
                    pick_lock(otmp, u.ux, u.uy, TRUE);
                }
            } else {
                if (((otmp = carrying(SKELETON_KEY)) ||
                     (otmp = carrying(CREDIT_CARD)) ||
                     (otmp = carrying(LOCK_PICK)))) {
                    pick_lock(otmp, u.ux, u.uy, TRUE);
                }
            }
        }
        return 0;
    }
    cobj->lknown = 1; /* floor container, so no need for update_inventory() */

    if (cobj->otyp == BAG_OF_TRICKS && cobj->spe > 0) {
        You("carefully open %s...", the(xname(cobj)));
        pline("It develops a huge set of %s you!",
              Hallucination ? "lips and kisses" : "teeth and bites");
        losehp(Maybe_Half_Phys(rnd(10)),
                Hallucination ? "amorous bag" : "carnivorous bag", KILLED_BY_AN);
        makeknown(BAG_OF_TRICKS);
        abort_looting = TRUE;
        return 1;
    }

    You("%sopen %s...", (!cobj->cknown || !cobj->lknown) ? "carefully " : "", the(xname(cobj)));
    return use_container(cobjp, 0, (boolean) (cindex < container_count));
}
/* loot a container on the floor or loot saddle from mon. */
int
doloot(void)
{
    struct obj *cobj, *nobj, *otmp;
    int c = -1;
    int timepassed = 0;
    coord cc;
    boolean underfoot = TRUE;
    const char *dont_find_anything = "don't find anything";
    struct monst *mtmp;
    char qbuf[BUFSZ];
    int prev_inquiry = 0;
    boolean prev_loot = FALSE;
    int container_count = 0;

    if (check_capacity((char *)0)) {
        /* "Can't do that while carrying so much stuff." */
        return 0;
    }
    if (nohands(youmonst.data)) {
        You("have no hands!"); /* not `body_part(HAND)' */
        return 0;
    }
    cc.x = u.ux; cc.y = u.uy;

    if (iflags.menu_requested) {
        goto lootmon;
    }

lootcont:
    if ((container_count = container_at(cc.x, cc.y, TRUE))) {
        boolean any = FALSE;

        if (!able_to_loot(cc.x, cc.y, TRUE)) {
            return 0;
        }

        if (Blind && !uarmg) {
            /* if blind and without gloves, attempting to #loot at the
               location of a cockatrice corpse is fatal before asking
               whether to manipulate any containers */

            for (nobj = sobj_at(CORPSE, cc.x, cc.y); nobj; nobj = nxtobj(nobj, CORPSE, TRUE)) {
                if (will_feel_cockatrice(nobj, FALSE)) {
                    feel_cockatrice(nobj, FALSE);
                    /* if life-saved (or poly'd into stone golem),
                       terminate attempt to loot */
                    return 1;
                }
            }
        }

        if (container_count > 1) {
            /* use a menu to loot many containers */
            int n, i;
            anything any;
            menu_item *pick_list = (menu_item *) 0;

            any.a_void = 0;
            winid win = create_nhwindow(NHW_MENU);
            start_menu(win);

            for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = cobj->nexthere) {
                if (Is_container(cobj)) {
                    any.a_obj = cobj;
                    add_menu(win, obj_to_glyph(cobj), MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                             doname(cobj), MENU_UNSELECTED);
                }
            }
            end_menu(win, "Loot which containers?");
            n = select_menu(win, PICK_ANY, &pick_list);
            destroy_nhwindow(win);

            if (n > 0) {
                for (i = 1; i <= n; i++) {
                    cobj = pick_list[i-1].item.a_obj;
                    timepassed |= do_loot_cont(&cobj, i, n);
                    if (abort_looting) {
                        /* chest trap or magic bag explosion or <esc> */
                        free(pick_list);
                        return timepassed;
                    }
                }
                free(pick_list);
            }
            if (n != 0) {
                c = 'y';
            }
        } else {
            for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = nobj) {
                nobj = cobj->nexthere;

                if (Is_container(cobj)) {
                    /* don't ask if there is only one lootable object */
                    if (container_count != 1 || iflags.vanilla_ui_behavior) {
                        safe_qbuf(qbuf, "There is ", " here, loot it?",
                                cobj, doname, ansimpleoname,
                                "a container");
                        c = ynq(qbuf);
                        if (c == 'q') return (timepassed);
                        if (c == 'n') continue;
                    }
                    any = TRUE;

                    timepassed |= do_loot_cont(&cobj, 1, 1);
                    if (abort_looting) {
                        /* chest trap or magic bag explosion or <esc> */
                        return timepassed;
                    }
                }
            }
        }
        if (any) {
            c = 'y';
        }
    } else if (IS_GRAVE(levl[cc.x][cc.y].typ)) {
        You("need to dig up the grave to effectively loot it...");
    }
    /*
     * 3.3.1 introduced directional looting for some things.
     */
 lootmon:
    if (c != 'y' && mon_beside(u.ux, u.uy)) {
        if (!get_adjacent_loc("Loot in what direction?", "Invalid loot location",
                              u.ux, u.uy, &cc)) return 0;
        if (cc.x == u.ux && cc.y == u.uy) {
            underfoot = TRUE;
            if (container_at(cc.x, cc.y, FALSE))
                goto lootcont;
        } else
            underfoot = FALSE;
        if (u.dz < 0) {
            You("%s to loot on the %s.", dont_find_anything,
                ceiling(cc.x, cc.y));
            timepassed = 1;
            return timepassed;
        }
        mtmp = m_at(cc.x, cc.y);
        if (mtmp) timepassed = loot_mon(mtmp, &prev_inquiry, &prev_loot);
        /* always use a turn when choosing a direction is impaired,
           even if you've successfully targetted a saddled creature
           and then answered "no" to the "remove its saddle?" prompt */
        if (Confusion || Stunned) {
            timepassed = 1;
        }
        /* Preserve pre-3.3.1 behaviour for containers.
         * Adjust this if-block to allow container looting
         * from one square away to change that in the future.
         */
        if (!underfoot) {
            if (container_at(cc.x, cc.y, FALSE)) {
                if (mtmp) {
                    You_cant("loot anything %sthere with %s in the way.",
                             prev_inquiry ? "else " : "", mon_nam(mtmp));
                    return timepassed;
                } else {
                    You("have to be at a container to loot it.");
                }
            } else {
                You("%s %sthere to loot.", dont_find_anything,
                    (prev_inquiry || prev_loot) ? "else " : "");
                return timepassed;
            }
        }
    } else if (c != 'y' && c != 'n') {
        You("%s %s to loot.", dont_find_anything,
            underfoot ? "here" : "there");
    }
    return (timepassed);
}

boolean
shopclutter(void)
{
    /* try to be a civilized adventurer, will you? */
    if (*u.ushops && shop_keeper(*u.ushops)
        && inhishop(shop_keeper(*u.ushops))) {
        char buf[BUFSZ];
        if (!(shop_keeper(*u.ushops))->mpeaceful) {
            verbalize("Don't even think about cluttering my shop!");
        } else {
            Strcpy(buf, "\"Please, ");
            append_honorific(buf);
            pline("%s; try not to create a mess.\"", buf);
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Actually tip over and empty a container. Bags of tricks
 * with charges behave as though applied. Traps are still
 * set off this way, and quantum mechanic boxes and
 * vampires in coffins still work appropriately. (Note
 * that dump_container() is called twice in the case of
 * Schroedinger's Cat: once to empty the box, and again
 * to dump the corpse should the cat's wavefunction be
 * collapsed in an unfortunate manner.) The "seems to
 * be locked" message comes after the potential item
 * breakage which takes place in dump_container().
 */
int
tip_container(struct obj *cobj)
{
    /* tipping in shops is buggy, thus temporarily disallowed */
    if (*u.ushops && shop_keeper(*u.ushops) &&
        inhishop(shop_keeper(*u.ushops))) return shopclutter();

    You("tip %s over.", the(xname(cobj)));

    if (cobj->otyp == BAG_OF_TRICKS && cobj->spe > 0) {
        bagotricks(cobj);
    } else {
        sellobj_state(SELL_DELIBERATE);
        dump_container(cobj, FALSE);
        sellobj_state(SELL_NORMAL);
        if (cobj->olocked) {
            pline("%s to be locked.", Tobjnam(cobj, "seem"));
            return 0;
        } else if (cobj->otrapped) {
            (void) chest_trap(cobj, HAND, FALSE);
        } else if (cobj->spe > 0) {
            if (cobj->spe == 1) {
                observe_quantum_cat(cobj, FALSE);
                sellobj_state(SELL_DELIBERATE);
                dump_container(cobj, FALSE);
                sellobj_state(SELL_NORMAL);
            } else if (cobj->spe == 4) {
                open_coffin(cobj, TRUE);
            }
        }
    }
    return 1;
}

/* Called to tip over a container. Ask for boxes at the
 * hero's current location unless the hero is unable to
 * reach any that could be there, or if there are none.
 * Since tipping in shops is disallowed, act differently
 * when confused or hallucinating in such a circumstance.
 */
int
dotip(void)
{
    struct obj *cobj, *nobj;
    struct monst *shkp;
    int c = -1;
    coord cc;
    char qbuf[BUFSZ];
    const char tools[] = {TOOL_CLASS, 0};
    cc.x = u.ux; cc.y = u.uy;

    if (check_capacity((char *)0)) return 0;

    if (Underwater) {
        pline("Water turbulence prevents you from tipping anything.");
        return 0;
    }

    if (*u.ushops && (shkp = shop_keeper(*u.ushops)) &&
        inhishop(shkp) && (Confusion || Hallucination)) {
        if (uarmh && !cursed(uarmh) && rn2(3)) {
            You("tip your %s to %s.", xname(uarmh),
                Hallucination ? the(rndmonnam()) : shkname(shkp));
        } else {
            if (money_cnt(invent) >= 10) {
                int gratuity = rnd(10);
                You("tip %s for providing a pleasant shopping experience.",
                    Hallucination ? the(rndmonnam()) : shkname(shkp));
                money2mon(shkp, gratuity);
            } else {
                pline("Unfortunately, you do not have enough gold to tip %s.",
                      Hallucination ? the(rndmonnam()) : shkname(shkp));
            }
        }
        return 0;
    }

    if (!able_to_loot(cc.x, cc.y, FALSE)) {
        goto tipinventory;
    } else if (container_at(cc.x, cc.y, FALSE)) {
        for (cobj = level.objects[cc.x][cc.y]; cobj; cobj = nobj) {
            nobj = cobj->nexthere;

            if (Is_container(cobj)) {
                safe_qbuf(qbuf, "There is ", " here, tip it?",
                          cobj,
                          doname, ansimpleoname, "container");
                c = ynq(qbuf);
                if (c == 'q') return 0;
                if (c == 'n') continue;

                tip_container(cobj);
                /* can only tip one container at a time */
                return 1;
            }
        }
    }

tipinventory:
    cobj = getobj(tools, "empty");
    if(!cobj) return 0;

    /* TODO: empty out other items, like potions? */
    if (!Is_container(cobj)) {
        pline("That isn't a container.");
        return 0;
    }

    return tip_container(cobj);
}

/* loot_mon() returns amount of time passed.
 */
int
loot_mon(struct monst *mtmp, int *passed_info, boolean *prev_loot)
{
    int c = -1;
    int timepassed = 0;
    struct obj *otmp;
    char qbuf[QBUFSZ];

    /* 3.3.1 introduced the ability to remove saddle from a steed             */
    /*  *passed_info is set to TRUE if a loot query was given.               */
    /*  *prev_loot is set to TRUE if something was actually acquired in here. */
    if (mtmp && mtmp != u.usteed && (otmp = which_armor(mtmp, W_SADDLE))) {
        long unwornmask;

        if (passed_info) *passed_info = 1;
        Sprintf(qbuf, "Do you want to remove the saddle from %s?",
                x_monnam(mtmp, ARTICLE_THE, (char *)0, SUPPRESS_SADDLE, FALSE));
        if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
            if (nolimbs(youmonst.data)) {
                You_cant("do that without limbs."); /* not body_part(HAND) */
                return (0);
            }
            if (otmp->cursed) {
                You("can't.  The saddle seems to be stuck to %s.",
                    x_monnam(mtmp, ARTICLE_THE, (char *)0,
                             SUPPRESS_SADDLE, FALSE));

                /* the attempt costs you time */
                return (1);
            }
            obj_extract_self(otmp);
            if ((unwornmask = otmp->owornmask) != 0L) {
                mtmp->misc_worn_check &= ~unwornmask;
                otmp->owornmask = 0L;
                update_mon_intrinsics(mtmp, otmp, FALSE, FALSE);
            }
            otmp = hold_another_object(otmp, "You drop %s!", doname(otmp),
                                       (const char *)0);
            nhUse(otmp);
            timepassed = rnd(3);
            if (prev_loot) *prev_loot = TRUE;
        } else if (c == 'q') {
            return (0);
        }
    }

    /* 3.4.0 introduced the ability to pick things up from within swallower's stomach */
    if (u.uswallow) {
        int count = passed_info ? *passed_info : 0;

        timepassed = pickup(count);
    }
    return timepassed;
}

/*
 * Decide whether an object being placed into a magic bag will cause
 * it to explode.  If the object is a bag itself, check recursively.
 */
static boolean
mbag_explodes(struct obj *obj, int depthin)
{
    /* these won't cause an explosion when they're empty */
    if ((obj->otyp == WAN_CANCELLATION || obj->otyp == BAG_OF_TRICKS) &&
        obj->spe <= 0)
        return FALSE;

    /* odds: 1/1, 2/2, 3/4, 4/8, 5/16, 6/32, 7/64, 8/128, 9/128, 10/128,... */
    if ((Is_mbag(obj) || obj->otyp == WAN_CANCELLATION) &&
        (rn2(1 << (depthin > 7 ? 7 : depthin)) <= depthin))
        return TRUE;
    else if (Has_contents(obj)) {
        struct obj *otmp;

        for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
            if (mbag_explodes(otmp, depthin+1)) return TRUE;
    }
    return FALSE;
}

static long
boh_loss(struct obj *container, int held)
{
    /* sometimes toss objects if a cursed magic bag */
    if (Is_mbag(container) && container->cursed && Has_contents(container)) {
        long loss = 0L;
        struct obj *curr, *otmp;

        for (curr = container->cobj; curr; curr = otmp) {
            otmp = curr->nobj;
            if (!rn2(13)) {
                obj_extract_self(curr);
                loss += mbag_item_gone(held, curr);
            }
        }
        return loss;
    }
    return 0;
}

/* A variable set in use_container(), to be used by the callback routines   */
/* in_container(), and out_container() from askchain() and use_container(). */
static NEARDATA struct obj *current_container;
#define Icebox (current_container->otyp == ICE_BOX)

/* Returns: -1 to stop, 1 item was inserted, 0 item was not inserted. */
static int
in_container(struct obj *obj)
{
    boolean floor_container = !carried(current_container);
    boolean was_unpaid = FALSE;
    char buf[BUFSZ];

    if (!current_container) {
        impossible("<in> no current_container?");
        return 0;
    } else if (obj == uball || obj == uchain) {
        You("must be kidding.");
        return 0;
    } else if (obj == current_container &&
               objects[obj->otyp].oc_name_known &&
               obj->otyp == BAG_OF_HOLDING) {
        pline("Creating an artificial black hole could ruin your whole day!");
        return 0;
    } else if (obj == current_container) {
        pline("That would be an interesting topological exercise.");
        return 0;
    } else if (obj->owornmask & (W_ARMOR | W_ACCESSORY)) {
        Norep("You cannot %s %s you are wearing.",
              Icebox ? "refrigerate" : "stash", something);
        return 0;
    } else if ((obj->otyp == LOADSTONE) && obj->cursed) {
        set_bknown(obj, 1);   /* unambiguously cursed */
        makeknown(obj->otyp); /* unambiguously a loadstone */
        pline_The("stone%s won't leave your person.", plur(obj->quan));
        return 0;
    } else if (obj->otyp == AMULET_OF_YENDOR ||
               obj->otyp == CANDELABRUM_OF_INVOCATION ||
               obj->otyp == BELL_OF_OPENING ||
               obj->otyp == SPE_BOOK_OF_THE_DEAD) {
        /* Prohibit Amulets in containers; if you allow it, monsters can't
         * steal them.  It also becomes a pain to check to see if someone
         * has the Amulet.  Ditto for the Candelabrum, the Bell and the Book.
         */
        pline("%s cannot be confined in such trappings.", The(xname(obj)));
        return 0;
    } else if (obj->otyp == LEASH && obj->leashmon != 0) {
        pline("%s attached to your pet.", Tobjnam(obj, "are"));
        return 0;
    } else if (obj == uwep) {
        if (welded(obj)) {
            weldmsg(obj);
            return 0;
        }
        setuwep((struct obj *) 0);
        /* This uwep check is obsolete.  It dates to 3.0 and earlier when
         * unwielding Firebrand would be fatal in hell if hero had no other
         * fire resistance.  Life-saving would force it to be re-wielded.
         */
        if (uwep) return 0; /* unwielded, died, rewielded */
    } else if (obj == uswapwep) {
        setuswapwep((struct obj *) 0);
    } else if (obj == uquiver) {
        setuqwep((struct obj *) 0);
    }

    if (fatal_corpse_mistake(obj, FALSE)) {
        return -1;
    }

    /* boxes, boulders, and big statues can't fit into any container */
    if (obj->otyp == ICE_BOX || Is_box(obj) || obj->otyp == BOULDER ||
        (obj->otyp == STATUE && bigmonst(&mons[obj->corpsenm]))) {
        /*
         *  xname() uses a static result array.  Save obj's name
         *  before current_container's name is computed.  Don't
         *  use the result of strcpy() within You() --- the order
         *  of evaluation of the parameters is undefined.
         */
        Strcpy(buf, the(xname(obj)));
        You("cannot fit %s into %s.", buf,
            the(xname(current_container)));
        return 0;
    }

    freeinv(obj);

    if (obj_is_burning(obj))    /* this used to be part of freeinv() */
        (void) snuff_lit(obj);

    if (floor_container && costly_spot(u.ux, u.uy)) {
        /* defer gold until after put-in message */
        if (obj->oclass != COIN_CLASS) {
            /* sellobj() will take an unpaid item off the shop bill
             * note: coins are handled later */
            was_unpaid = obj->unpaid ? TRUE : FALSE;
            /* don't sell when putting the item into your own container,
               but handle billing correctly */
            sellobj_state(current_container->no_charge ?
                          SELL_DONTSELL : SELL_DELIBERATE);
            sellobj(obj, u.ux, u.uy);
            sellobj_state(SELL_NORMAL);
        }
    }
    if (Icebox && !age_is_relative(obj)) {
        obj->age = monstermoves - obj->age; /* actual age */
        /* stop any corpse timeouts when frozen */
        if (obj->otyp == CORPSE && obj->timed) {
            long rot_alarm = stop_timer(ROT_CORPSE, obj_to_any(obj));

            (void) stop_timer(REVIVE_MON, obj_to_any(obj));
            /* mark a non-reviving corpse as such */
            if (rot_alarm) obj->norevive = 1;
        }
    } else if ((current_container->otyp == BAG_OF_HOLDING) && mbag_explodes(obj, 0)) {
#ifdef LIVELOGFILE
        livelog_printf(LL_ACHIEVE, "just blew up %s %s with %s",
                       uhis(), dump_typename(current_container->otyp), an(dump_typename(obj->otyp)));
#endif
        /* explicitly mention what item is triggering the explosion */
        pline( "As you put %s inside, you are blasted by a magical explosion!", doname(obj));
        /* did not actually insert obj yet */
        if (was_unpaid) addtobill(obj, FALSE, FALSE, TRUE);
        /* At least ID the wand and give some experience for losing a bag of
         * holding this way. */
        if (obj->otyp == WAN_CANCELLATION) {
            makeknown(obj->otyp);
        }
        int items_scattered = 0;
        if (Is_container(obj)) {
            /* also dump contents of inserted object */
            items_scattered += dump_container(obj, TRUE);
        } else {
            obfree(obj, (struct obj *)0);
        }
        /* dump it out onto the floor so the scatterage can take effect */
        items_scattered += dump_container(current_container, TRUE);
        if (items_scattered > 0) {
            pline("The contents fly everywhere!");
        }
        scatter(u.ux, u.uy, 10, VIS_EFFECTS|MAY_HIT|MAY_DESTROY|MAY_FRACTURE, 0);

        losehp(d(6, 6), "magical explosion", KILLED_BY_AN);
        current_container = 0; /* baggone = TRUE; */
    }

    if (current_container) {
        Strcpy(buf, the(xname(current_container)));
        You("put %s into %s.", doname(obj), buf);

        /* gold in container always needs to be added to credit */
        if (floor_container && obj->oclass == COIN_CLASS)
            sellobj(obj, current_container->ox, current_container->oy);
        (void) add_to_container(current_container, obj);
        current_container->owt = weight(current_container);
    }

    /* blessed or uncursed scrolls of charging charge bag of tricks */
    if (current_container && (current_container->otyp == BAG_OF_TRICKS) &&
        obj->otyp == SCR_CHARGING && !obj->cursed) {
        makeknown(obj->otyp);
        makeknown(current_container->otyp);
        pline("The %s digests the magic of the %s!", xname(current_container), xname(obj));
        recharge(current_container, (obj->blessed ? 1 : 0));
        if (current_container->spe > 0) {
            current_container = NULL;
        }
    }

    /* gold needs this, and freeinv() many lines above may cause
     * the encumbrance to disappear from the status, so just always
     * update status immediately.
     */
    bot();

    return(current_container ? 1 : -1);
}

static int
ck_bag(struct obj *obj)
{
    return current_container && obj != current_container;
}

/* Returns: -1 to stop, 1 item was removed, 0 item was not removed. */
static int
out_container(struct obj *obj)
{
    struct obj *otmp;
    boolean is_gold = (obj->oclass == COIN_CLASS);
    int res, loadlev;
    long count;

    if (!current_container) {
        impossible("<out> no current_container?");
        return -1;
    } else if (is_gold) {
        obj->owt = weight(obj);
    }

    if(obj->oartifact && !touch_artifact(obj, &youmonst)) return 0;

    if (fatal_corpse_mistake(obj, FALSE)) {
        return -1;
    }

    count = obj->quan;
    if ((res = lift_object(obj, current_container, &count, FALSE)) <= 0)
        return res;

    if (obj->quan != count && obj->otyp != LOADSTONE)
        obj = splitobj(obj, count);

    /* Remove the object from the list. */
    obj_extract_self(obj);
    current_container->owt = weight(current_container);

    if (Icebox) {
        removed_from_icebox(obj);
    }

    if (!obj->unpaid && !carried(current_container) &&
       costly_spot(current_container->ox, current_container->oy)) {
        obj->ox = current_container->ox;
        obj->oy = current_container->oy;
        addtobill(obj, FALSE, FALSE, FALSE);
    }

    if (is_pick(obj)) {
        pick_pick(obj); /* shopkeeper feedback */
    }

    otmp = addinv(obj);
    loadlev = near_capacity();
    prinv(loadlev ?
          (loadlev < MOD_ENCUMBER ?
           "You have a little trouble removing" :
           "You have much trouble removing") : (char *)0,
          otmp, count);

    if (is_gold) {
        bot(); /* update character's gold piece count immediately */
    }
    return 1;
}

/* taking a corpse out of an ice box needs a couple of adjustments */
static void
removed_from_icebox(struct obj *obj)
{
    if (!age_is_relative(obj)) {
        obj->age = monstermoves - obj->age; /* actual age */
        if (obj->otyp == CORPSE) {
            start_corpse_timeout(obj);
        }
    }
}

/* an object inside a cursed bag of holding is being destroyed */
static long
mbag_item_gone(int held, struct obj *item)
{
    struct monst *shkp;
    long loss = 0L;

    if (item->dknown)
        pline("%s %s vanished!", Doname2(item), otense(item, "have"));
    else
        You("%s %s disappear!", Blind ? "notice" : "see", doname(item));

    if (*u.ushops && (shkp = shop_keeper(*u.ushops)) != 0) {
        if (held ? (boolean) item->unpaid : costly_spot(u.ux, u.uy))
            loss = stolen_value(item, u.ux, u.uy,
                                (boolean)shkp->mpeaceful, TRUE);
    }
    obfree(item, (struct obj *) 0);
    return loss;
}

/* used for #loot/apply, #tip, and final disclosure */
void
observe_quantum_cat(struct obj *box, boolean past)
{
    static NEARDATA const char sc[] = "Schroedinger's Cat";
    struct obj *deadcat;
    struct monst *livecat = 0;
    xchar ox, oy;
    boolean itsalive = rn2(2);

    box->spe = 0;       /* box->owt will be updated below */
    if (get_obj_location(box, &ox, &oy, 0))
        box->ox = ox, box->oy = oy; /* in case it's being carried */

    /* this isn't really right, since any form of observation
       (telepathic or monster/object/food detection) ought to
       force the determination of alive vs dead state; but basing
       it just on opening the box is much simpler to cope with */
    boolean makecat = (itsalive && !past);
    /* SchroedingersBox already has a cat corpse in it */
    deadcat = box->cobj;
    if (itsalive) {
        if (makecat) {
            livecat = makemon(&mons[PM_HOUSECAT], box->ox, box->oy, NO_MINVENT | MM_ADJACENTOK);
        }
        if (livecat) {
            livecat->mpeaceful = 1;
            set_malign(livecat);
            if (!canspotmon(livecat)) {
                You("think %s brushed your %s.", something, body_part(FOOT));
            } else {
                pline("%s inside the box is still alive!", Monnam(livecat));
            }
            (void) christen_monst(livecat, sc);
            if (deadcat) {
                obj_extract_self(deadcat);
                obfree(deadcat, (struct obj *) 0), deadcat = 0;
            }
            box->owt = weight(box);
            box->spe = 0;
        }
    } else {
        box->spe = 0; /* now an ordinary box (with a cat corpse inside) */
        if (deadcat) {
            /* set_corpsenm() will start the rot timer that was removed
               when makemon() created SchroedingersBox; start it from
               now rather than from when this special corpse got created */
            deadcat->age = monstermoves;
            set_corpsenm(deadcat, PM_HOUSECAT);
            deadcat = oname(deadcat, sc);
        }
        pline_The("%s%s inside the box is dead!",
                  Hallucination ? rndmonnam() : "housecat",
                  past ? " that was" : "");
    }
}

/* based largely on borrowed code from dNetHack by Chris_ANG */
void
open_coffin(struct obj *box, boolean past)
{
    /* static NEARDATA const char sc[] = "Schroedinger's Cat"; */
    /* Would be nice to name the vampire and put the name on the coffin. But not today. */
    struct monst *vampire;
    xchar ox, oy;

    pline("That %s %s, it%s a coffin!", past ? "wasn't" : "isn't",
          an(simple_typename(box->otyp)), past ? " was" : "'s");
    box->spe = 3;    /* box->owt will be updated below */
    if (get_obj_location(box, &ox, &oy, 0))
        box->ox = ox, box->oy = oy; /* in case it's being carried */

    vampire = makemon(mkclass(S_VAMPIRE, 0), box->ox, box->oy, NO_MM_FLAGS);
    set_malign(vampire);
    if (!canspotmon(vampire)) {
        You("think %s brushed against your %s.", something, body_part(HAND));
    } else {
        pline("There %s %s in the coffin.", past ? "was" : "is",
              Hallucination ? "a dark knight" : a_monnam(vampire));
        pline_The("%s rises!", Hallucination ? "dark knight" : m_monnam(vampire));
    }
    /* (void) christen_monst(vampire, sc); */
    box->owt = weight(box);
    return;
}

#undef Icebox

/* used by askchain() to check for magic bag explosion */
boolean
container_gone(int (*fn) (struct obj *))
{
    /* result is only meaningful while use_container() is executing */
    return ((fn == in_container || fn == out_container) && !current_container);
}

static void
explain_container_prompt(boolean more_containers)
{
    static const char *const explaintext[] = {
        "Container actions:",
        "",
        " : -- Look: examine contents",
        " o -- Out: take things out",
        " i -- In: put things in",
        " b -- Both: first take things out, then put things in",
        " r -- Reversed: put things in, then take things out",
        " s -- Stash: put one item in", "",
        " n -- Next: loot next selected container",
        " q -- Quit: finished",
        " ? -- Help: display this text.",
        "", 0
    };
    const char *const *txtpp;
    winid win;

    /* "Do what with <container>? [:oibrsq or ?] (q)" */
    if ((win = create_nhwindow(NHW_TEXT)) != WIN_ERR) {
        for (txtpp = explaintext; *txtpp; ++txtpp) {
            if (!more_containers && !strncmp(*txtpp, " n ", 3)) {
                continue;
            }
            putstr(win, 0, *txtpp);
        }
        display_nhwindow(win, FALSE);
        destroy_nhwindow(win);
    }
}

boolean
u_handsy(void)
{
    if (nohands(youmonst.data)) {
        You("have no hands!"); /* not `body_part(HAND)' */
        return FALSE;
    } else if (!freehand()) {
        You("have no free %s.", body_part(HAND));
        return FALSE;
    }
    return TRUE;
}

static const char stashable[] = { ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, 0 };

int
use_container(
    struct obj **objp,
    int held,
    boolean more_containers) /**< TRUE iff #loot multiple and this isn't last one */
{
    struct obj *otmp, *obj = *objp;
    boolean one_by_one, allflag, quantum_cat = FALSE,
            loot_out = FALSE, loot_in = FALSE;
    char select[MAXOCLASSES+1];
    char qbuf[BUFSZ], emptymsg[BUFSZ], pbuf[QBUFSZ];
    long loss = 0L;
    int cnt = 0, used = 0,
        menu_on_request;

    abort_looting = FALSE;
    emptymsg[0] = '\0';

    /* let the hero be creative with their body parts */
    if (!has_head(youmonst.data) && !u_handsy()) {
        return 0;
    }

    if (!obj->lknown) {
        /* do this in advance */
        obj->lknown = 1;
        if (held) {
            update_inventory();
        }
    }

    if (obj->olocked) {
        pline("%s to be locked.", Tobjnam(obj, "seem"));
        if (held) You("must put it down to unlock.");
        return 0;
    } else if (obj->otrapped) {
        if (held) You("open %s...", the(xname(obj)));
        (void) chest_trap(obj, HAND, FALSE);
        /* even if the trap fails, you've used up this turn */
        if (multi >= 0) {   /* in case we didn't become paralyzed */
            nomul(-1, "opening a container");
            nomovemsg = "";
        }
        abort_looting = TRUE;
        return 1;
    }
    current_container = obj;    /* for use by in/out_container */

    /*
     * From here on out, all early returns go through 'containerdone:'.
     */

    if (obj->spe == 1) {
        observe_quantum_cat(obj, FALSE); /* box was not destroyed, use present tense */
        used = 1;
        quantum_cat = TRUE;     /* for adjusting "it's empty" message */
    } else if (obj->spe == 4) {
        open_coffin(obj, FALSE);    /* box was not destroyed, use present tense */
        used = 1;
        return used;
    }

    /* Count the number of contained objects. Sometimes toss objects if */
    /* a cursed magic bag.                          */
    struct obj *curr;
    for (curr = obj->cobj; curr; curr = otmp) {
        otmp = curr->nobj;
        if (Is_mbag(obj) && obj->cursed && !rn2(13)) {
            obj_extract_self(curr);
            loss += mbag_item_gone(held, curr);
            used = 1;
        } else {
            cnt++;
        }
    }

    if (loss)   /* magic bag lost some shop goods */
        You("owe %ld %s for lost merchandise.", loss, currency(loss));
    obj->owt = weight(obj); /* in case any items were lost */

    boolean inokay = (invent != 0 && !(invent == current_container && !current_container->nobj));
    boolean outokay = Has_contents(current_container);
    if (!cnt)
        Sprintf(emptymsg, "%s is %sempty.", Yname2(obj),
                quantum_cat ? "now " : "");

    /*
     * What-to-do prompt's list of possible actions:
     * always include the look-inside choice (':');
     * include the take-out choice ('o') if container
     * has anything in it or if player doesn't yet know
     * that it's empty (latter can change on subsequent
     * iterations if player picks ':' response);
     * include the put-in choices ('i','s') if hero
     * carries any inventory (including gold);
     * include do-both when 'o' is available, even if
     * inventory is empty--taking out could alter that;
     * include do-both-reversed when 'i' is available,
     * even if container is empty--for similar reason;
     * include the next container choice ('n') when
     * relevant, and make it the default;
     * always include the quit choice ('q'), and make
     * it the default if there's no next containter;
     * include the help choice (" or ?") if `cmdassist'
     * run-time option is set;
     * (Player can pick any of (o,i,b,r,n,s,?) even when
     * they're not listed among the available actions.)
     *
     * Do what with <the/your/Shk's container>? [:oibrs nq or ?] (q)
     * or
     * <The/Your/Shk's container> is empty.  Do what with it? [:irs nq or ?]
     */
    char c;
    char xbuf[QBUFSZ];
    for (;;) {
        /* repeats iff '?' or ':' gets chosen */
        boolean outmaybe = (outokay || !current_container->cknown);
        if (!outmaybe) {
            (void) safe_qbuf(qbuf, (char *) 0, " is empty.  Do what with it?",
                             current_container, Yname2, Ysimple_name2, "This");
        } else {
            (void) safe_qbuf(qbuf, "Do what with ", "?", current_container, yname, ysimple_name, "it");
        }
        /* ask player about what to do with this container */
        if (flags.menu_style == MENU_PARTIAL || flags.menu_style == MENU_FULL) {
            if (!inokay && !outmaybe) {
                /* nothing to take out, nothing to put in;
                   trying to do both will yield proper feedback */
                c = 'b';
            } else {
                c = in_or_out_menu(qbuf, current_container, outmaybe, inokay,
                                   (boolean) (used != 0), more_containers);
            }
        } else { /* TRADITIONAL or COMBINATION */
            xbuf[0] = '\0'; /* list of extra acceptable responses */
            Strcpy(pbuf, ":");                   /* look inside */
            Strcat(outmaybe ? pbuf : xbuf, "o"); /* take out */
            Strcat(inokay ? pbuf : xbuf, "i");   /* put in */
            Strcat(outmaybe ? pbuf : xbuf, "b"); /* both */
            Strcat(inokay ? pbuf : xbuf, "rs");  /* reversed, stash */
            Strcat(pbuf, " ");                   /* separator */
            Strcat(more_containers ? pbuf : xbuf, "n"); /* next container */
            Strcat(pbuf, "q");                   /* quit */
            if (iflags.cmdassist) {
                /* this unintentionally allows user to answer with 'o' or
                   'r'; fortunately, those are already valid choices here */
                Strcat(pbuf, " or ?"); /* help */
            } else {
                Strcat(xbuf, "?");
            }
            if (*xbuf) {
                Strcat(strcat(pbuf, "\033"), xbuf);
            }
            c = yn_function(qbuf, pbuf, more_containers ? 'n' : 'q');
        } /* PARTIAL|FULL vs other modes */

        if (c == '?') {
            explain_container_prompt(more_containers);
        } else if (c == ':') { /* note: will set obj->cknown */
            if (!current_container->cknown) {
                used = 1; /* gaining info */
            }
            container_contents(current_container, FALSE, FALSE, TRUE);
        } else {
            break;
        }
    } /* loop until something other than '?' or ':' is picked */

    if (c == 'q') {
        abort_looting = TRUE;
    }
    /* [not strictly needed; falling thru works] */
    if (c == 'n' || c == 'q') {
        goto containerdone;
    }
    loot_out = (c == 'o' || c == 'b' || c == 'r');
    loot_in = (c == 'i' || c == 'b' || c == 'r');
    boolean loot_in_first = (c == 'r'); /* both, reversed */
    boolean stash_one = (c == 's');

    /* out-only or out before in */
    if (loot_out && !loot_in_first) {
        if (!Has_contents(current_container)) {
            pline("%s", emptymsg); /* <whatever> is empty. */
            if (!current_container->cknown) {
                used = 1;
            }
            current_container->cknown = 1;
        } else {
            add_valid_menu_class(0); /* reset */
            if (flags.menu_style == MENU_TRADITIONAL) {
                used |= traditional_loot(FALSE);
            } else {
                used |= (menu_loot(0, FALSE) > 0);
            }
            add_valid_menu_class(0);
        }
    }

    if ((loot_in || stash_one) &&
        (!invent || (invent == current_container && !invent->nobj))) {
        You("don't have anything%s to %s.",
            invent ? " else" : "",
            stash_one ? "stash" : "put in");
        loot_in = stash_one = FALSE;
    }

    /*
     * Gone: being nice about only selecting food if we know we are
     * putting things in an ice chest.
     */
    if (loot_in) {
        add_valid_menu_class(0); /* reset */
        if (flags.menu_style == MENU_TRADITIONAL) {
            used |= traditional_loot(TRUE);
        } else {
            used |= (menu_loot(0, TRUE) > 0);
        }
        add_valid_menu_class(0);
    } else if (stash_one) {
        /* put one item into container */
        if ((otmp = getobj(stashable, "stash")) != 0) {
            if (in_container(otmp)) {
                used = 1;
            } else {
                /* couldn't put selected item into container for some
                   reason; might need to undo splitobj() */
                (void) unsplitobj(otmp);
            }
        }
    }
    /* putting something in might have triggered magic bag explosion */
    if (!current_container) {
        loot_out = FALSE;
    }

    /* out after in */
    if (loot_out && loot_in_first) {
        if (!Has_contents(current_container)) {
            pline("%s", emptymsg); /* <whatever> is empty. */
            if (!current_container->cknown) {
                used = 1;
            }
            current_container->cknown = 1;
        } else {
            add_valid_menu_class(0); /* reset */
            if (flags.menu_style == MENU_TRADITIONAL) {
                used |= traditional_loot(FALSE);
            } else {
                used |= (menu_loot(0, FALSE) > 0);
            }
            add_valid_menu_class(0);
        }
    }

 containerdone:
    if (used) {
        /* Not completely correct; if we put something in without knowing
           whatever was already inside, now we suddenly do.  That can't
           be helped unless we want to track things item by item and then
           deal with containers whose contents are "partly known". */
        if (current_container) {
            current_container->cknown = 1;
        }
        update_inventory();
    }

    *objp = current_container; /* might have become null */
    if (current_container) {
        current_container = 0; /* avoid hanging on to stale pointer */
    } else {
        abort_looting = TRUE;
    }
    return used;
}

/* loot current_container (take things out or put things in), by prompting */
static int
traditional_loot(boolean put_in)
{
    int (*actionfunc)(OBJ_P), (*checkfunc)(OBJ_P);
    struct obj **objlist;
    char selection[MAXOCLASSES + 10]; /* +10: room for B,U,C,X plus slop */
    const char *action;
    boolean one_by_one, allflag;
    int used = 0, menu_on_request = 0;

    if (put_in) {
        action = "put in";
        objlist = &invent;
        actionfunc = in_container;
        checkfunc = ck_bag;
    } else {
        action = "take out";
        objlist = &(current_container->cobj);
        actionfunc = out_container;
        checkfunc = (int (*) (OBJ_P)) 0;
    }

    if (query_classes(selection, &one_by_one, &allflag, action, *objlist, FALSE, &menu_on_request)) {
        if (askchain(objlist, (one_by_one ? (char *) 0 : selection), allflag,
                     actionfunc, checkfunc, 0, action)) {
            used = 1;
        }
    } else if (menu_on_request < 0) {
        used = (menu_loot(menu_on_request, put_in) > 0);
    }
    return used;
}

/* Loot a container (take things out, put things in), using a menu. */
static int
menu_loot(int retry, boolean put_in)
{
    int n, i, n_looted = 0;
    boolean all_categories = TRUE, loot_everything = FALSE;
    char buf[BUFSZ];
    const char *action = put_in ? "Put in" : "Take out";
    struct obj *otmp, *otmp2;
    menu_item *pick_list;
    int mflags, res;
    long count;

    if (retry) {
        all_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
        all_categories = FALSE;
        Sprintf(buf, "%s what type of objects?", action);
        mflags = (ALL_TYPES | UNPAID_TYPES | BUCX_TYPES | UNIDENTIFIED_TYPES);
        if (put_in) {
            mflags |= RECENTLY_PICKED_UP;
        } else {
            mflags |= CHOOSE_ALL;
        }
        n = query_category(buf, put_in ? invent : current_container->cobj,
                           mflags, &pick_list, PICK_ANY);
        if (!n) return 0;
        for (i = 0; i < n; i++) {
            if (pick_list[i].item.a_int == 'A')
                loot_everything = TRUE;
            else if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
                all_categories = TRUE;
            else
                add_valid_menu_class(pick_list[i].item.a_int);
        }
        free((genericptr_t) pick_list);
    }

    if (loot_everything) {
        if (!put_in) {
            current_container->cknown = 1;
            for (otmp = current_container->cobj; otmp; otmp = otmp2) {
                otmp2 = otmp->nobj;
                res = out_container(otmp);
                if (res < 0) {
                    break;
                }
                n_looted += res;
            }
        } else {
            for (otmp = invent; otmp && current_container; otmp = otmp2) {
                otmp2 = otmp->nobj;
                res = in_container(otmp);
                if (res < 0) {
                    break;
                }
                n_looted += res;
            }
        }
    } else {
        mflags = INVORDER_SORT;
        if (put_in && flags.invlet_constant) mflags |= USE_INVLET;
        if (!put_in) {
            current_container->cknown = 1;
        }
        Sprintf(buf, "%s what?", action);
        n = query_objlist(buf, put_in ? invent : current_container->cobj,
                          mflags, &pick_list, PICK_ANY,
                          all_categories ? allow_all : allow_category);
        if (n) {
            n_looted = n;
            for (i = 0; i < n; i++) {
                otmp = pick_list[i].item.a_obj;
                count = pick_list[i].count;
                if (count > 0 && count < otmp->quan) {
                    otmp = splitobj(otmp, count);
                    /* special split case also handled by askchain() */
                }
                res = put_in ? in_container(otmp) : out_container(otmp);
                if (res < 0) {
                    if (!current_container) {
                        /* otmp caused current_container to explode;
                           both are now gone */
                        otmp = 0; /* and break loop */
                    } else if (otmp && otmp != pick_list[i].item.a_obj) {
                        /* split occurred, merge again */
                        (void) merged(&pick_list[i].item.a_obj, &otmp);
                    }
                    break;
                }
            }
            free((genericptr_t)pick_list);
        }
    }
    return n_looted;
}

static char
in_or_out_menu(const char *prompt, struct obj *obj, boolean outokay, boolean inokay, boolean alreadyused, boolean more_containers)
{
    /* underscore is not a choice; it's used to skip element [0] */
    static const char lootchars[] = "_:oibrsnq", abc_chars[] = "_:abcdenq";
    winid win;
    anything any;
    menu_item *pick_list;
    char buf[BUFSZ];
    int n;
    const char *menuselector = iflags.lootabc ? abc_chars : lootchars;

    any = zeroany;
    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    any.a_int = 1; /* ':' */
    Sprintf(buf, "Look inside %s", thesimpleoname(obj));
    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
             buf, MENU_UNSELECTED);

    if (outokay) {
        any.a_int = 2; /* 'o' */
        Sprintf(buf, "take %s out", something);
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 buf, MENU_UNSELECTED);
    }
    if (inokay) {
        any.a_int = 3; /* 'i' */
        Sprintf(buf, "put %s in", something);
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 buf, MENU_UNSELECTED);
    }
    if (outokay) {
        any.a_int = 4; /* 'b' */
        Sprintf(buf, "%stake out, then put in", inokay ? "both; " : "");
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 buf, MENU_UNSELECTED);
    }
    if (inokay) {
        any.a_int = 5; /* 'r' */
        Sprintf(buf, "%sput in, then take out", outokay ? "both reversed; " : "");
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 buf, MENU_UNSELECTED);
        any.a_int = 6; /* 's' */
        Sprintf(buf, "stash one item into %s", thesimpleoname(obj));
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 buf, MENU_UNSELECTED);
    }
    any.a_int = 0;
    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    if (more_containers) {
        any.a_int = 7; /* 'n' */
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE,
                 "loot next container", MENU_SELECTED);
    }
    any.a_int = 8; /* 'q' */
    Strcpy(buf, alreadyused ? "done" : "do nothing");
    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, menuselector[any.a_int], 0, ATR_NONE, buf,
             more_containers ? MENU_UNSELECTED : MENU_SELECTED);

    end_menu(win, prompt);
    n = select_menu(win, PICK_ONE, &pick_list);
    destroy_nhwindow(win);
    if (n > 0) {
        int k = pick_list[0].item.a_int;

        if (n > 1 && k == (more_containers ? 7 : 8)) {
            k = pick_list[1].item.a_int;
        }
        free((genericptr_t) pick_list);
        return lootchars[k]; /* :,o,i,b,r,s,n,q */
    }
    return (n == 0 && more_containers) ? 'n' : 'q'; /* next or quit */
}


/* Dumps out a container, possibly as the prelude/result of an explosion.
 * destroy_after trashes the container afterwards; try not to use it :P
 *
 * Assumption 1: Player is assumed to not be handling the contents directly.
 * Assumption 2: !destroy_after == #tip command
 * Assumption 3: #tip always occurs at the hero's location
 *
 * Returns 1 if at least one object was present, 0 if empty.
 */
int
dump_container(struct obj *container, boolean destroy_after)
{
    struct obj* otmp, *otmp2;
    int ret = 0;
    schar ltyp;
    ltyp = levl[u.ux][u.uy].typ;

    /* sanity checks */
    if (!container || !Is_container(container))
        return 0;

    /* cursed bags of holding are supposed to be generally
       difficult, so when trying to tip them while cursed,
       make them literally become bags of "holding" only */
    if (!destroy_after && Is_mbag(container) && container->cursed) {
        pline("%s reluctant to give up its contents.", Tobjnam(container, "seem"));
        return 0;
    }

    /* trying to empty a locked container may damage its contents */
    if (!destroy_after && container->olocked) {
        ret = !!container->cobj;
        container_impact_dmg(container, u.ux, u.uy);
        return ret;
    }

#if 0 /* DEFERRED */
    /* make sure floor container contents are billed properly when tipping */
    if (!destroy_after) {
        struct monst *shkp;

        /* logic courtesy of pick_obj() */
        if (container->where == OBJ_FLOOR &&
            !u.uswallow &&
            costly_spot(container->ox, container->oy) &&
            /* logic courtesy of addtobill() */
            *u.ushops &&
            (shkp = shop_keeper(*u.ushops)) &&
            inhishop(shkp)) {

            /* logic courtesy of addtobill() */
            long gltmp = contained_gold(container);
            if (gltmp) costly_gold(container->ox, container->oy, gltmp);
            bill_box_content(container, FALSE);
            picked_container(container);
        }
    }
#endif

    for (otmp = container->cobj; otmp; otmp = otmp2)
    {
        ret = 1;
        otmp2 = otmp->nobj;
        obj_extract_self(otmp);
        container->owt = weight(container);

        /* we do need to start the timer on these */
        if (container->otyp == ICE_BOX && !age_is_relative(otmp)) {
            otmp->age = monstermoves - otmp->age;
            if (otmp->otyp == CORPSE) {
                start_corpse_timeout(otmp);
            }
        }

        if (destroy_after) {
            /* bag of holding explosions */
            place_object(otmp, u.ux, u.uy);
        } else {
            if (!u.uswallow) {
                if (!can_reach_floor(TRUE)) {
                    hitfloor(otmp, TRUE);
                } else {
                    if (!IS_ALTAR(ltyp)) pline("%s spill%s %sto the %s.",
                                               Doname2(otmp), (otmp->otyp == LENSES || is_gloves(otmp)
                                                               || is_boots(otmp) || otmp->quan != 1L) ? "" : "s",
                                               (IS_SINK(ltyp) || IS_LAVA(ltyp) || IS_SOFT(ltyp)) ?
                                               "in" : "on", IS_SINK(ltyp) ? "sink" : surface(u.ux, u.uy));
                    dropx(otmp);
                }
                /* tipping is too uncoordinated to get rings to hit the drain */
                if ((otmp->oclass == RING_CLASS) && IS_SINK(ltyp))
                    You("hear a%s clatter.", is_metallic(otmp) ? " metallic" : "");
            } else {
                dropx(otmp);
            }
            /* consider spilled objects deliberately dropped */
            otmp->was_dropped = TRUE;
        }

        if (otmp->otyp == GOLD_PIECE) {
            bot(); /* update character's gold piece count immediately */
        }
    }

    if (destroy_after) {
        if (container->where == OBJ_INVENT) {
            useup(container);
        } else if (obj_here(container, u.ux, u.uy)) {
            useupf(container, container->quan);
        }
    }

    return ret;
}


static void
del_sokoprize(void)
{
    int x, y, cnt = 0;
    struct obj *otmp, *onext;
    /* check objs on floor */
    for (otmp = fobj; otmp; otmp = onext) {
        onext = otmp->nobj; /* otmp may be destroyed */
        if (Is_sokoprize(otmp)) {
            x = otmp->ox;
            y = otmp->oy;
            obj_extract_self(otmp);
            if (cansee(x, y)) {
                You_see("%s vanishing.", an(xname(otmp)));
                newsym(x, y);
            } else cnt++;
            obfree(otmp, (struct obj *)0);
        }
    }
    if (cnt && flags.soundok) You_hear("something popping.");
    /* check buried objs... do we need this? */
    for (otmp = level.buriedobjlist; otmp; otmp = onext) {
        onext = otmp->nobj; /* otmp may be destroyed */
        if (Is_sokoprize(otmp)) {
            obj_extract_self(otmp);
            obfree(otmp, (struct obj *)0);
        }
    }
}

/*pickup.c*/
