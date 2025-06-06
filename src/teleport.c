/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static boolean tele_jump_ok(coordxy, coordxy, coordxy, coordxy);
static boolean teleok(coordxy, coordxy, boolean);
static void vault_tele(void);
static boolean rloc_pos_ok(coordxy, coordxy, struct monst *);
static void rloc_to_core(struct monst *, coordxy, coordxy, unsigned);
static void mvault_tele(struct monst *);

/* non-null when teleporting via having read this scroll */
static struct obj *telescroll = 0;

/*
 * Is (x, y) a bad position of mtmp?  If mtmp is NULL, then is (x, y) bad
 * for an object?
 *
 * Caller is responsible for checking (x, y) with isok() if required.
 *
 * Returns: -1: Inaccessible, 0: Good pos, 1: Temporally inacessible
 */
static int
badpos(coordxy x, coordxy y, struct monst *mtmp, unsigned int gpflags)
{
    int is_badpos = 0, pool;
    struct permonst *mdat = NULL;
    boolean ignorewater = ((gpflags & MM_IGNOREWATER) != 0);
    struct monst *mtmp2;

    /* in many cases, we're trying to create a new monster, which
     * can't go on top of the player or any existing monster.
     * however, occasionally we are relocating engravings or objects,
     * which could be co-located and thus get restricted a bit too much.
     * oh well.
     */
    if (mtmp != &youmonst &&
         x == u.ux &&
         y == u.uy &&
         (!u.usteed || mtmp != u.usteed)) {
        is_badpos = 1;
    }

    if (mtmp) {
        mtmp2 = m_at(x, y);

        /* Be careful with long worms.  A monster may be placed back in
         * its own location.  Normally, if m_at() returns the same monster
         * that we're trying to place, the monster is being placed in its
         * own location.  However, that is not correct for worm segments,
         * because all the segments of the worm return the same m_at().
         * Actually we overdo the check a little bit--a worm can't be placed
         * in its own location, period.  If we just checked for mtmp->mx
         * != x || mtmp->my != y, we'd miss the case where we're called
         * to place the worm segment and the worm's head is at x,y.
         */
        if (mtmp2 && (mtmp2 != mtmp || mtmp->wormno)) {
            is_badpos = 1;
        }

        mdat = mtmp->data;
        pool = is_pool(x, y);
        if (mdat->mlet == S_EEL && !pool && rn2(13) && !ignorewater) {
            is_badpos = 1;
        }

        if (pool && !ignorewater) {
            if (mtmp == &youmonst) {
                return (HLevitation || Flying || Wwalking ||
                        Swimming || Amphibious) ? is_badpos : -1;
            } else return (is_flyer(mdat) || is_swimmer(mdat) ||
                         is_clinger(mdat)) ? is_badpos : -1;
        } else if (is_lava(x, y)) {
            if (mtmp == &youmonst) {
                return HLevitation ? is_badpos : -1;
            } else {
                return (is_flyer(mdat) || likes_lava(mdat)) ?
                       is_badpos : -1;
            }
        }
        if (is_open_air(x, y) && grounded(mdat)) {
            return FALSE;
        }
        if (passes_walls(mdat) && may_passwall(x, y)) {
            return is_badpos;
        }
    }
    if (!ACCESSIBLE(levl[x][y].typ)) {
        if (!(is_pool(x, y) && ignorewater)) {
            return -1;
        }
    }

    if (closed_door(x, y) && (!mdat || !amorphous(mdat))) {
        return mdat && (nohands(mdat) || verysmall(mdat)) ? -1 : 1;
    }
    if (sobj_at(BOULDER, x, y) && (!mdat || !throws_rocks(mdat))) {
        return mdat ? -1 : 1;
    }
    return is_badpos;
}

/* teleporting is prevented on this level for this monster? */
boolean
noteleport_level(struct monst *mon)
{
    struct monst *mtmp;

    /* demon court in Gehennom prevent others from teleporting */
    if (In_hell(&u.uz) && !(is_dlord(mon->data) || is_dprince(mon->data))) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (is_dlord(mtmp->data) || is_dprince(mtmp->data)) {
                return TRUE;
            }
    }

    /* natural no-teleport level */
    if (level.flags.noteleport) {
        return TRUE;
    }

    return FALSE;
}

/*
 * Is (x,y) a good position of mtmp?  If mtmp is NULL, then is (x,y) good
 * for an object?
 *
 * This function will only look at mtmp->mdat, so makemon, mplayer, etc can
 * call it to generate new monster positions with fake monster structures.
 */
boolean
goodpos(coordxy x, coordxy y, struct monst *mtmp, unsigned int gpflags)
{
    struct permonst *mdat = NULL;
    boolean ignorewater = ((gpflags & MM_IGNOREWATER) != 0);

    if (!isok(x, y)) {
        return FALSE;
    }

    /* in many cases, we're trying to create a new monster, which
     * can't go on top of the player or any existing monster.
     * however, occasionally we are relocating engravings or objects,
     * which could be co-located and thus get restricted a bit too much.
     * oh well.
     */
    if (u_at(x, y) &&
        mtmp != &youmonst &&
        (mtmp != u.ustuck || !u.uswallow)
        && (!u.usteed || mtmp != u.usteed)
        )
        return FALSE;

    if (mtmp) {
        struct monst *mtmp2 = m_at(x, y);

        /* Be careful with long worms.  A monster may be placed back in
         * its own location.  Normally, if m_at() returns the same monster
         * that we're trying to place, the monster is being placed in its
         * own location.  However, that is not correct for worm segments,
         * because all the segments of the worm return the same m_at().
         * Actually we overdo the check a little bit--a worm can't be placed
         * in its own location, period.  If we just checked for mtmp->mx
         * != x || mtmp->my != y, we'd miss the case where we're called
         * to place the worm segment and the worm's head is at x,y.
         */
        if (mtmp2 && (mtmp2 != mtmp || mtmp->wormno)) {
            return FALSE;
        }

        mdat = mtmp->data;
        if (is_pool(x, y) && !ignorewater) {
            /* [what about Breathless?] */
            if (mtmp == &youmonst) {
                return (Swimming || Amphibious ||
                        (!Is_waterlevel(&u.uz) &&
                            /* water on the Plane of Water has no surface
                               so there's no way to be on or above that */
                            (Levitation || Flying || Wwalking)));
            } else {
                return (is_swimmer(mdat) ||
                        (!Is_waterlevel(&u.uz) &&
                         (is_floater(mdat) || is_flyer(mdat) || is_clinger(mdat))));
            }
        } else if (mdat->mlet == S_EEL && rn2(13) && !ignorewater) {
            return FALSE;
        } else if (is_lava(x, y)) {
            /* 3.6.3: floating eye can levitate over lava but it avoids
               that due the effect of the heat causing it to dry out */
            if (mdat == &mons[PM_FLOATING_EYE]) {
                return FALSE;
            }
            if (mtmp == &youmonst) {
                return (Levitation ||
                        Flying ||
                        (Fire_resistance && Wwalking && uarmf && uarmf->oerodeproof) ||
                        (Upolyd && likes_lava(youmonst.data)));
            } else {
                return (is_floater(mdat) || is_flyer(mdat) || likes_lava(mdat));
            }
        }
        if (passes_walls(mdat) && may_passwall(x, y)) {
            return TRUE;
        }
        if (amorphous(mdat) && closed_door(x, y)) {
            return TRUE;
        }
    }
    if (!accessible(x, y)) {
        if (!(is_pool(x, y) && ignorewater)) {
            return FALSE;
        }
    }

    if (sobj_at(BOULDER, x, y) && (!mdat || !throws_rocks(mdat))) {
        return FALSE;
    }

    return TRUE;
}

/*
 * "entity next to"
 *
 * Attempt to find a good place for the given monster type in the closest
 * position to (xx,yy).  Do so in successive square rings around (xx,yy).
 * If there is more than one valid position in the ring, choose one randomly.
 *
 * Return TRUE and the position chosen when successful, FALSE otherwise.
 */
boolean
enexto(coord *cc, coordxy xx, coordxy yy, struct permonst *mdat)
{
    return enexto_core(cc, xx, yy, mdat, NO_MM_FLAGS);
}

boolean
enexto_core(coord *cc, coordxy xx, coordxy yy, struct permonst *mdat, unsigned int entflags)
{
    return enexto_core_range(cc, xx, yy, mdat, entflags, 1);
}

boolean
enexto_core_range(
    coord *cc,
    coordxy xx,
    coordxy yy,
    struct permonst *mdat,
    unsigned int entflags,
    int start_range) /**< Distance of checked tiles to begin with. Should be >=1. */
{
#define MAX_GOOD 15
    coord good[MAX_GOOD], *good_ptr;
    int x, y, range, i;
    int xmin, xmax, ymin, ymax, rangemax;
    struct monst fakemon; /* dummy monster */
    boolean allow_xx_yy = (boolean) ((entflags & GP_ALLOW_XY) != 0);

    entflags &= ~GP_ALLOW_XY;
    if (!mdat) {
#ifdef DEBUG
        pline("enexto() called with mdat==0");
#endif
        /* default to player's original monster type */
        mdat = &mons[u.umonster];
    }
    fakemon = zeromonst;
    set_mon_data(&fakemon, mdat); /* set up for goodpos */

    /* used to use 'if (range > ROWNO && range > COLNO) return FALSE' below,
       so effectively 'max(ROWNO, COLNO)' which performs useless iterations
       (possibly many iterations if <xx,yy> is in the center of the map) */
    xmax = max(xx - 1, (COLNO - 1) - xx);
    ymax = max(yy - 0, (ROWNO - 1) - yy);
    rangemax = max(xmax, ymax);
    /* setup: no suitable spots yet, first iteration checks adjacent spots */
    good_ptr = good;
    range = start_range;
    /*
     * Walk around the border of the square with center (xx,yy) and
     * radius range.  Stop when we find at least one valid position.
     */
    do {
        xmin = max(1, xx-range);
        xmax = min(COLNO-1, xx+range);
        ymin = max(0, yy-range);
        ymax = min(ROWNO-1, yy+range);

        for (x = xmin; x <= xmax; x++) {
            if (goodpos(x, ymin, &fakemon, entflags)) {
                good_ptr->x = x;
                good_ptr->y = ymin;
                /* beware of accessing beyond segment boundaries.. */
                if (good_ptr++ == &good[MAX_GOOD-1]) {
                    goto full;
                }
            }
            if (goodpos(x, ymax, &fakemon, entflags)) {
                good_ptr->x = x;
                good_ptr->y = ymax;
                if (good_ptr++ == &good[MAX_GOOD-1]) {
                    goto full;
                }
            }
        }
        /* 3.6.3: this used to use 'ymin+1' which left top row unchecked */
        for (y = ymin; y < ymax; y++) {
            if (goodpos(xmin, y, &fakemon, entflags)) {
                good_ptr->x = xmin;
                good_ptr->y = y;
                if (good_ptr++ == &good[MAX_GOOD-1]) {
                    goto full;
                }
            }
            if (goodpos(xmax, y, &fakemon, entflags)) {
                good_ptr->x = xmax;
                good_ptr->y = y;
                if (good_ptr++ == &good[MAX_GOOD-1]) {
                    goto full;
                }
            }
        }
    } while (++range <= rangemax && good_ptr == good);

    /* return false if we exhausted 'range' without finding anything */
    if (good_ptr == good) {
        /* 3.6.3: earlier versions didn't have the option to try <xx,yy>,
           and left 'cc' uninitialized when returning False */
        cc->x = xx;
        cc->y = yy;
        /* if every spot other than <xx,yy> has failed, try <xx,yy> itself */
        if (allow_xx_yy && goodpos(xx, yy, &fakemon, entflags)) {
            return TRUE; /* 'cc' is set */
        } else {
            debug_pline("enexto(\"%s\",%d,%d) failed", mdat->mname, xx, yy);
            return FALSE;
        }
    }

full:
    /* we've got between 1 and SIZE(good) candidates; choose one */
    i = rn2((int)(good_ptr - good));
    cc->x = good[i].x;
    cc->y = good[i].y;
    return TRUE;
}

/*
 * "entity path to"
 *
 * Attempt to find nc good places for the given monster type with the shortest
 * path to (xx,yy).  Where there is more than one valid set of positions, one
 * will be chosen at random.  Return the number of positions found.
 * Warning:  This routine is much slower than enexto and should be used
 * with caution.
 */

#define EPATHTO_UNSEEN        0x0
#define EPATHTO_INACCESSIBLE  0x1
#define EPATHTO_DONE          0x2
#define EPATHTO_TAIL(n)      (0x3 + ((n) & 7))

#define EPATHTO_XY(x, y)    (((y) + 1) * COLNO + (x))
#define EPATHTO_Y(xy)       ((xy) / COLNO - 1)
#define EPATHTO_X(xy)       ((xy) % COLNO)

#ifdef DEBUG
coord epathto_debug_cc[100];
#endif

int
epathto(coord *cc, int nc, coordxy xx, coordxy yy, struct permonst *mdat)
{
    int i, j, dir, ndirs, xy, x, y, r;
    int path_len, postype;
    int first_col, last_col;
    int nd, n;
    unsigned char *map;
    static const int dirs[8] =
        /* N, S, E, W, NW, NE, SE, SW */
    { -COLNO, COLNO, 1, -1, -COLNO-1, -COLNO+1, COLNO+1, COLNO-1};
    struct monst fakemon;   /* dummy monster */
    fakemon.data = mdat;    /* set up for badpos */
    map = (unsigned char *)alloc(COLNO * (ROWNO + 2));
    (void) memset((genericptr_t)map, EPATHTO_INACCESSIBLE, COLNO * (ROWNO + 2));
    for (i = 1; i < COLNO; i++) {
        for (j = 0; j < ROWNO; j++) {
            map[EPATHTO_XY(i, j)] = EPATHTO_UNSEEN;
        }
    }
    map[EPATHTO_XY(xx, yy)] = EPATHTO_TAIL(0);
    if (badpos(xx, yy, &fakemon, 0) == 0) {
        cc[0].x = xx;
        cc[0].y = yy;
        nd = n = 1;
    } else {
        nd = n = 0;
    }
    for (path_len = 0; nd < nc; path_len++) {
        first_col = max(1, xx - path_len);
        last_col = min(COLNO - 1, xx + path_len);
        for (j = max(0, yy - path_len); j <= min(ROWNO - 1, yy + path_len); j++) {
            for (i = first_col; i <= last_col; i++) {
                if (map[EPATHTO_XY(i, j)] == EPATHTO_TAIL(path_len)) {
                    map[EPATHTO_XY(i, j)] = EPATHTO_DONE;
                    ndirs = mdat == &mons[PM_GRID_BUG] ? 4 : 8;
                    for (dir = 0; dir < ndirs; dir++) {
                        xy = EPATHTO_XY(i, j) + dirs[dir];
                        if (map[xy] == EPATHTO_UNSEEN) {
                            x = EPATHTO_X(xy);
                            y = EPATHTO_Y(xy);
                            postype = badpos(x, y, &fakemon, 0);
                            map[xy] = postype < 0 ? EPATHTO_INACCESSIBLE :
                                      EPATHTO_TAIL(path_len + 1);
                            if (postype == 0) {
                                if (n < nc) {
                                    cc[n].x = x;
                                    cc[n].y = y;
                                } else if (rn2(n - nd + 1) < nc - nd) {
                                    r = rn2(nc - nd) + nd;
                                    cc[r].x = x;
                                    cc[r].y = y;
                                }
                                ++n;
                            }
                        }
                    }
                }
            }
        }
        if (nd == n) {
            break; /* No more positions */
        } else {
            nd = n;
        }
    }
    if (nd > nc) {
        nd = nc;
    }
#ifdef DEBUG
    if (cc == epathto_debug_cc) {
        winid win;
        int glyph;
        char row[COLNO+1];

        win = create_nhwindow(NHW_TEXT);
        putstr(win, 0, "");
        for (y = 0; y < ROWNO; y++) {
            for (x = 1; x < COLNO; x++) {
                xy = EPATHTO_XY(x, y);
                if (map[xy] == EPATHTO_INACCESSIBLE) {
                    glyph = back_to_glyph(x, y);
                    row[x] = showsyms[glyph_to_cmap(glyph)];
                } else {
                    row[x] = ' ';
                }
            }
            for (i = 0; i < nd; i++) {
                if (cc[i].y == y) {
                    row[cc[i].x] = i < 10 ? '0' + i :
                                   i < 36 ? 'a' + i - 10 :
                                   i < 62 ? 'A' + i - 36 :
                                   '?';
                }
            }
            /* remove trailing spaces */
            for (x = COLNO-1; x >= 1; x--) {
                if (row[x] != ' ') {
                    break;
                }
            }
            row[x+1] = '\0';

            putstr(win, 0, &row[1]);
        }
        display_nhwindow(win, TRUE);
        destroy_nhwindow(win);
    }
#endif

    free((genericptr_t)map);
    return nd;
}

/*
 * Check for restricted areas present in some special levels.  (This might
 * need to be augmented to allow deliberate passage in wizard mode, but
 * only for explicitly chosen destinations.)
 */
static boolean
tele_jump_ok(coordxy x1, coordxy y1, coordxy x2, coordxy y2)
{
    if (!isok(x2, y2)) {
        return FALSE;
    }
    if (dndest.nlx > 0) {
        /* if inside a restricted region, can't teleport outside */
        if (within_bounded_area(x1, y1, dndest.nlx, dndest.nly,
                                dndest.nhx, dndest.nhy) &&
            !within_bounded_area(x2, y2, dndest.nlx, dndest.nly,
                                 dndest.nhx, dndest.nhy)) {
            return FALSE;
        }
        /* and if outside, can't teleport inside */
        if (!within_bounded_area(x1, y1, dndest.nlx, dndest.nly,
                                 dndest.nhx, dndest.nhy) &&
            within_bounded_area(x2, y2, dndest.nlx, dndest.nly,
                                dndest.nhx, dndest.nhy)) {
            return FALSE;
        }
    }
    if (updest.nlx > 0) { /* ditto */
        if (within_bounded_area(x1, y1, updest.nlx, updest.nly,
                                updest.nhx, updest.nhy) &&
            !within_bounded_area(x2, y2, updest.nlx, updest.nly,
                                 updest.nhx, updest.nhy)) {
            return FALSE;
        }
        if (!within_bounded_area(x1, y1, updest.nlx, updest.nly,
                                 updest.nhx, updest.nhy) &&
            within_bounded_area(x2, y2, updest.nlx, updest.nly,
                                updest.nhx, updest.nhy)) {
            return FALSE;
        }
    }
    return TRUE;
}

static boolean
teleok(coordxy x, coordxy y, boolean trapok)
{
    if (!trapok) {
        /* allow teleportation onto vibrating square, it's not a real trap */
        struct trap *trap = t_at(x, y);

        if (trap && trap->ttyp != VIBRATING_SQUARE) {
            return FALSE;
        }
    }
    if (!goodpos(x, y, &youmonst, 0)) {
        return FALSE;
    }
    if (!tele_jump_ok(u.ux, u.uy, x, y)) {
        return FALSE;
    }
    if (!in_out_region(x, y)) {
        return FALSE;
    }
    return TRUE;
}

void
teleds(coordxy nux, coordxy nuy, boolean allow_drag)
{
    unsigned was_swallowed;

    if (u.utraptype == TT_BURIEDBALL) {
        /* unearth it */
        buried_ball_to_punishment();
    }
    boolean ball_active = (Punished && uball->where != OBJ_FREE);
    boolean ball_still_in_range = FALSE;

    /* If they have to move the ball, then drag if allow_drag is true;
     * otherwise they are teleporting, so unplacebc().
     * If they don't have to move the ball, then always "drag" whether or
     * not allow_drag is true, because we are calling that function, not
     * to drag, but to move the chain.  *However* there are some dumb
     * special cases:
     *    0                          0
     *   _X  move east       ----->  X_
     *    @                           @
     * These are permissible if teleporting, but not if dragging.  As a
     * result, drag_ball() needs to know about allow_drag and might end
     * up dragging the ball anyway.  Also, drag_ball() might find that
     * dragging the ball is completely impossible (ball in range but there's
     * rock in the way), in which case it teleports the ball on its own.
     */
    if (ball_active) {
        if (!carried(uball) && distmin(nux, nuy, uball->ox, uball->oy) <= 2) {
            ball_still_in_range = TRUE; /* don't have to move the ball */
        } else {
            /* have to move the ball */
            if (!allow_drag || distmin(u.ux, u.uy, nux, nuy) > 1) {
                /* we should not have dist > 1 and allow_drag at the same
                 * time, but just in case, we must then revert to teleport.
                 */
                allow_drag = FALSE;
                unplacebc();
            }
        }
    }
    u.ufeetfrozen = 0;   /* ice doesn't teleport with the player */
    reset_utrap(FALSE);
    was_swallowed = u.uswallow; /* set_ustuck(Null) clears uswallow */
    set_ustuck((struct monst *) 0);
    u.ux0 = u.ux;
    u.uy0 = u.uy;

    if (!hideunder(&youmonst) && youmonst.data->mlet == S_MIMIC) {
        /* mimics stop being unnoticed */
        youmonst.m_ap_type = M_AP_NOTHING;
    }

    if (was_swallowed) {
        u.uswldtim = u.uswallow = 0;
        if (Punished && !ball_active) {
            /* ensure ball placement, like unstuck */
            ball_active = TRUE;
            allow_drag = FALSE;
        }
        docrt();
    }
    if (ball_active) {
        if (ball_still_in_range || allow_drag) {
            int bc_control;
            coordxy ballx, bally, chainx, chainy;
            boolean cause_delay;

            if (drag_ball(nux, nuy, &bc_control, &ballx, &bally,
                          &chainx, &chainy, &cause_delay, allow_drag)) {
                move_bc(0, bc_control, ballx, bally, chainx, chainy);
            } else {
                /* dragging fails if hero is encumbered beyond 'burdened' */
                allow_drag = FALSE; /* teleport b&c to hero's new spot */
                unplacebc(); /* to match placebc() below */
            }
        }
    }
    /* must set u.ux, u.uy after drag_ball(), which may need to know
       the old position if allow_drag is true... */
    u_on_newpos(nux, nuy); /* set u.<x,y>, usteed-><mx,my>; cliparound() */
    fill_pit(u.ux0, u.uy0);
    if (ball_active) {
        if (!ball_still_in_range && !allow_drag) {
            placebc();
        }
    }
    initrack(); /* teleports mess up tracking monsters without this */
    update_player_regions();
    /*
     *  Make sure the hero disappears from the old location.  This will
     *  not happen if she is teleported within sight of her previous
     *  location.  Force a full vision recalculation because the hero
     *  is now in a new location.
     */
    newsym(u.ux0, u.uy0);
    see_monsters();
    vision_full_recalc = 1;
    nomul(0, 0);
    vision_recalc(0); /* vision before effects */
    /* if terrain type changes, levitation or flying might become blocked
       or unblocked; might issue message, so do this after map+vision has
       been updated for new location instead of right after u_on_newpos() */
    if (levl[u.ux][u.uy].typ != levl[u.ux0][u.uy0].typ) {
        switch_terrain();
    }
    if (telescroll) {
        /* when teleporting by scroll, we need to handle discovery
           now before getting feedback about any objects at our
           destination since we might land on another such scroll */
        if (distu(u.ux0, u.uy0) >= 16 || !couldsee(u.ux0, u.uy0)) {
            learn_scroll(telescroll);
        } else {
            telescroll = 0; /* no discovery by scrolltele()'s caller */
        }
    }
    /* sequencing issue:  we want guard's alarm, if any, to occur before
       room entry message, if any, so need to check for vault exit prior
       to spoteffects; but spoteffects() sets up new value for u.urooms
       and vault code depends upon that value, so we need to fake it */
    struct monst *vault_guard = vault_occupied(u.urooms) ? findgd() : 0;
    if (vault_guard) {
        char save_urooms[5]; /* [sizeof u.urooms] */

        Strcpy(save_urooms, u.urooms);
        Strcpy(u.urooms, in_rooms(u.ux, u.uy, VAULT));
        /* if hero has left vault, make guard notice */
        if (!vault_occupied(u.urooms)) {
            uleftvault(vault_guard);
        }
        Strcpy(u.urooms, save_urooms); /* reset prior to spoteffects() */
    }
    /* possible shop entry message comes after guard's shrill whistle */
    spoteffects(TRUE);
    invocation_message();
}

boolean
safe_teleds(boolean allow_drag)
{
    int nux, nuy, tcnt = 0;

    do {
        nux = rnd(COLNO-1);
        nuy = rn2(ROWNO);
    } while (!teleok(nux, nuy, (boolean)(tcnt > 200)) && ++tcnt <= 400);

    if (tcnt <= 400) {
        teleds(nux, nuy, allow_drag);
        return TRUE;
    } else {
        return FALSE;
    }
}

static void
vault_tele(void)
{
    struct mkroom *croom = search_special(VAULT);
    coord c;

    if (croom && somexy(croom, &c) && teleok(c.x, c.y, FALSE)) {
        teleds(c.x, c.y, FALSE);
        return;
    }
    tele();
}

boolean
teleport_pet(struct monst *mtmp, boolean force_it)
{
    struct obj *otmp;

    if (mtmp == u.usteed) {
        return (FALSE);
    }

    if (mtmp->mleashed) {
        otmp = get_mleash(mtmp);
        if (!otmp) {
            warning("%s is leashed, without a leash.", Monnam(mtmp));
            goto release_it;
        }
        if (otmp->cursed && !force_it) {
            yelp(mtmp);
            return FALSE;
        } else {
            Your("leash goes slack.");
release_it:
            m_unleash(mtmp, FALSE);
            return TRUE;
        }
    }
    return TRUE;
}

void
tele(void)
{
    coord cc;

    /* Disable teleportation in stronghold && Vlad's Tower */
    if (noteleport_level(&youmonst)) {
        if (!wizard) {
            pline("A mysterious force prevents you from teleporting!");
            return;
        } else {
            pline("Overriding non-teleport flag.");
        }
    }

    /* don't show trap if "Sorry..." */
    if (!Blinded) {
        make_blinded(0L, FALSE);
    }

    if
#ifdef WIZARD
    (
#endif
        (u.uhave.amulet || On_W_tower_level(&u.uz)
         || (u.usteed && mon_has_amulet(u.usteed))
        )
#ifdef WIZARD
        && (!wizard) )
#endif
    {
        You_feel("disoriented for a moment.");
        return;
    }
    if ((Teleport_control && !Stunned)
#ifdef WIZARD
        || wizard
#endif
        ) {
        if (unconscious()) {
            pline("Being unconscious, you cannot control your teleport.");
        } else {
            char buf[BUFSZ];
            if (u.usteed) {
                Sprintf(buf, " and %s", mon_nam(u.usteed));
            }

            pline("To what position do you%s want to be teleported?", u.usteed ? buf : "");
            cc.x = u.ux;
            cc.y = u.uy;
            if (getpos(&cc, TRUE, "the desired position") < 0) {
                return;     /* abort */
            }
            /* possible extensions: introduce a small error if
               magic power is low; allow transfer to solid rock */
            if (teleok(cc.x, cc.y, FALSE)) {
                teleds(cc.x, cc.y, FALSE);
                return;
            }
            pline("Sorry...");
        }
    }

    (void) safe_teleds(FALSE);
}

/** ^T command; 'm ^T' == choose among several teleport modes */
int
dotelecmd(void)
{
    long save_HTele, save_ETele;
    int res, added, hidden;
    boolean ignore_restrictions = FALSE;
/* also defined in spell.c */
#define NOOP_SPELL  0
#define HIDE_SPELL  1
#define ADD_SPELL   2
#define UNHIDESPELL 3
#define REMOVESPELL 4

    /* normal mode; ignore 'm' prefix if it was given */
    if (!wizard) {
        return dotele(FALSE);
    }

    added = hidden = NOOP_SPELL;
    save_HTele = HTeleportation, save_ETele = ETeleportation;
    if (!iflags.menu_requested) {
        ignore_restrictions = TRUE;
    } else {
        static const struct tporttypes {
            char menulet;
            const char *menudesc;
        } tports[] = {
            /*
             * Potential combinations:
             *  1) attempt ^T without intrinsic, not know spell;
             *  2) via intrinsic, not know spell, obey restrictions;
             *  3) via intrinsic, not know spell, ignore restrictions;
             *  4) via intrinsic, know spell, obey restrictions;
             *  5) via intrinsic, know spell, ignore restrictions;
             *  6) via spell, not have intrinsic, obey restrictions;
             *  7) via spell, not have intrinsic, ignore restrictions;
             *  8) force, obey other restrictions;
             *  9) force, ignore restrictions.
             * We only support the 1st (t), 2nd (n), 6th (s), and 9th (w).
             *
             * This ignores the fact that there is an experience level
             * (or poly-form) requirement which might make normal ^T fail.
             */
            { 'n', "normal ^T on demand; no spell, obey restrictions" },
            { 's', "via spellcast; no intrinsic teleport" },
            { 't', "try ^T without having it; no spell" },
            { 'w', "debug mode; ignore restrictions" }, /* trad wizard mode */
        };
        menu_item *picks = (menu_item *) 0;
        anything any;
        winid win;
        int i, tmode;

        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        any = zeroany;
        for (i = 0; i < SIZE(tports); ++i) {
            any.a_int = (int) tports[i].menulet;
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, (char) any.a_int, 0, ATR_NONE,
                     tports[i].menudesc,
                     (tports[i].menulet == 'w') ? MENU_SELECTED
                                                : MENU_UNSELECTED);
        }
        end_menu(win, "Which way do you want to teleport?");
        i = select_menu(win, PICK_ONE, &picks);
        destroy_nhwindow(win);
        if (i > 0) {
            tmode = picks[0].item.a_int;
            /* if we got 2, use the one which wasn't preselected */
            if (i > 1 && tmode == 'w') {
                tmode = picks[1].item.a_int;
            }
            free(picks);
        } else if (i == 0) {
            /* preselected one was explicitly chosen and got toggled off */
            tmode = 'w';
        } else { /* ESC */
            return 0;
        }
        switch (tmode) {
        case 'n':
            HTeleportation |= I_SPECIAL; /* confer intrinsic teleportation */
            hidden = tport_spell(HIDE_SPELL); /* hide teleport-away */
            break;
        case 's':
            HTeleportation = ETeleportation = 0L; /* suppress intrinsic */
            added = tport_spell(ADD_SPELL); /* add teleport-away */
            break;
        case 't':
            HTeleportation = ETeleportation = 0L; /* suppress intrinsic */
            hidden = tport_spell(HIDE_SPELL); /* hide teleport-away */
            break;
        case 'w':
            ignore_restrictions = TRUE;
            break;
        }
    }

    /* if dotele() can be fatal, final disclosure might lie about
       intrinsic teleportation; we should be able to live with that
       since the menu finagling is only applicable in wizard mode */
    res = dotele(ignore_restrictions);

    HTeleportation = save_HTele;
    ETeleportation = save_ETele;
    if (added != NOOP_SPELL || hidden != NOOP_SPELL) {
        /* can't both be non-NOOP so addition will yield the non-NOOP one */
        (void) tport_spell(added + hidden - NOOP_SPELL);
    }

    return res;
}

/** the #teleport command; 'm ^T' == choose among several teleport modes */
int
dotele(boolean break_the_rules) /**< TRUE: wizard mode ^T */
{
    struct trap *trap;
    const char *cantdoit;
    boolean trap_once = FALSE;

    trap = t_at(u.ux, u.uy);
    if (trap && (!trap->tseen || trap->ttyp != TELEP_TRAP)) {
        trap = 0;
    }

    if (trap) {
        trap_once = trap->once; /* trap may get deleted, save this */
        if (trap->once) {
            pline("This is a vault teleport, usable once only.");
            if (yn("Jump in?") == 'n') {
                trap = 0;
            } else {
                deltrap(trap);
                newsym(u.ux, u.uy);
            }
        }
        if (trap) {
            You("%s onto the teleportation trap.",
                locomotion(youmonst.data, "jump"));
        }
    }
    if (!trap) {
        boolean castit = FALSE;
        int energy = 0;

        if (!Teleportation || (u.ulevel < (Role_if(PM_WIZARD) ? 8 : 12) &&
                               !can_teleport(youmonst.data))) {
            /* Try to use teleport away spell. */
            int knownsp = known_spell(SPE_TELEPORT_AWAY);
            /* casting isn't inhibited by being Stunned (...it ought to be) */
            castit = (knownsp >= spe_Fresh && !Confusion);
            if (!castit && !break_the_rules) {
                You("%s.", (!Teleportation ? ((knownsp != spe_Unknown) ?
                                              "can't cast that spell" :
                                              "don't know that spell") :
                            "are not able to teleport at will"));
                return 0;
            }
        }

        cantdoit = 0;
        /* 3.6.2: the magic numbers for hunger, strength, and energy
           have been changed to match the ones used in spelleffects().
           Also, failing these tests used to return 1 and use a move
           even though casting failure due to these reasons doesn't.
           [Note: this spellev() is different from the one in spell.c
           but they both yield the same result.] */
#define spellev(spell_otyp) ((int) objects[spell_otyp].oc_level)
        energy = 5 * spellev(SPE_TELEPORT_AWAY);
        if (break_the_rules) {
            if (!castit) {
                energy = 0;

            } else if (u.uen < energy) {
                /* spell will cost more if carrying the Amulet, but the
                   amount is rnd(2 * energy) so we can't know by how much;
                   average is twice the normal cost, but could be triple;
                   the extra energy is spent even if that results in not
                   having enough to cast (which also uses the move) */
                u.uen = energy;
            }
        } else if (u.uhunger <= 10) {
            cantdoit = "are too weak from hunger";

        } else if (ACURR(A_STR) < 4) {
            cantdoit = "lack the strength";

        } else if (energy > u.uen) {
            cantdoit = "lack the energy";
        }
        if (cantdoit) {
            You("%s %s.", cantdoit,
                castit ? "for a teleport spell" : "to teleport");
            return 0;

        } else if (check_capacity("Your concentration falters from carrying so much.")) {
            return 1; /* this failure in spelleffects() also uses the move */
        }

        if (castit) {
            /* energy cost is deducted in spelleffects() */
            exercise(A_WIS, TRUE);
            if (spelleffects(SPE_TELEPORT_AWAY, TRUE)) {
                return 1;
            } else if (!break_the_rules) {
                return 0;
            }
        } else {
            /* bypassing spelleffects(); apply energy cost directly */
            u.uen -= energy;
            flags.botl = 1;
        }
    }

    if (next_to_u()) {
        if (trap && trap_once) {
            vault_tele();
        } else {
            tele();
        }
        (void) next_to_u();
    } else {
        You("%s", shudder_for_moment);
        return 0;
    }
    if (!trap) {
        morehungry(100);
    }
    return 1;
}

/** Return a d_level or NULL if no other portal stone portal exists. */
static d_level *
random_portal_stone_destination(void)
{
    int shuffled[NUM_PORTAL_STONE_TYPE] = { 0, 1, 2 };
    shuffle_int_array(shuffled, NUM_PORTAL_STONE_TYPE);

    int idx = -1;
    for (int i = 0; i < NUM_PORTAL_STONE_TYPE; i++) {
        if ((flags.portal_stone_location[shuffled[i]].dnum != -1) &&
            (ledger_no(&flags.portal_stone_location[shuffled[i]]) != ledger_no(&u.uz))) {
            idx = shuffled[i];
            break;
        }
    }

    if (idx >= 0) {
        return &flags.portal_stone_location[idx];
    }

    return NULL;
}

void
level_tele(void)
{
    int newlev;
    d_level newlevel;
    const char *escape_by_flying = 0; /* when surviving dest of -N */
    char buf[BUFSZ];
    boolean force_dest = FALSE;

    if (iflags.debug_fuzzer) {
        goto random_levtport;
    }

    if ((u.uhave.amulet || In_endgame(&u.uz) || In_sokoban(&u.uz))
#ifdef WIZARD
        && !wizard
#endif
        ) {
        You_feel("very disoriented for a moment.");
        return;
    }
    if ((Teleport_control && !Stunned)
#ifdef WIZARD
        || wizard
#endif
        ) {
        char qbuf[BUFSZ];
        int trycnt = 0;

        Strcpy(qbuf, "To what level do you want to teleport?");
        do {
            if (iflags.menu_requested) {
                /* wizard mode 'm ^V' skips prompting on first pass
                   (note: level Tport via menu won't have any second pass) */
                iflags.menu_requested = FALSE;
                if (wizard) {
                    goto levTport_menu;
                }
            }
            if (++trycnt == 2) {
#ifdef WIZARD
                if (wizard) {
                    Strcat(qbuf, " [type a number, name, or ? for a menu]");
                } else {
#endif
                    Strcat(qbuf, " [type a number or name]");
#ifdef WIZARD
                }
#endif
            }
            *buf = '\0'; /* EDIT_GETLIN: if we're on second or later pass,
                            the previous input was invalid so don't use it
                            as getlin()'s preloaded default answer */
            getlin(qbuf, buf);
            if (!strcmp(buf, "\033")) { /* cancelled */
                if (Confusion && rnl(5)) {
                    pline("Oops...");
                    goto random_levtport;
                }
                return;
            } else if (!strcmp(buf, "*")) {
                goto random_levtport;
            } else if (Confusion && rnl(5)) {
                pline("Oops...");
                goto random_levtport;
            }
#ifdef WIZARD
            if (wizard && !strcmp(buf, "?")) {
                schar destlev;
                xint16 destdnum;

 levTport_menu:
                destlev = 0;
                destdnum = 0;
                newlev = (int) print_dungeon(TRUE, &destlev, &destdnum);
                if (!newlev) {
                    return;
                }
                newlevel.dnum = destdnum;
                newlevel.dlevel = destlev;
                if (In_endgame(&newlevel) && !In_endgame(&u.uz)) {
                    struct obj *amu;

                    if (!u.uhave.amulet && (amu = mksobj(AMULET_OF_YENDOR, TRUE, FALSE))) {
                        /* ordinarily we'd use hold_another_object()
                           for something like this, but we don't want
                           fumbling or already full pack to interfere */
                        amu = addinv(amu);
                        prinv("Endgame prerequisite:", amu, 0L);
                    }
                }
                force_dest = TRUE;
            } else if ((newlev = lev_by_name(buf)) == 0) {
                newlev = atoi(buf);
            }
        } while (!newlev && !digit(buf[0]) &&
                 (buf[0] != '-' || !digit(buf[1])) &&
                 trycnt < 10);
#endif /* WIZARD */

        /* no dungeon escape via this route */
        if (newlev == 0) {
            if (trycnt >= 10) {
                goto random_levtport;
            }
            if (ynq("Go to Nowhere.  Are you sure?") != 'y') {
                return;
            }
            You("%s in agony as your body begins to warp...",
                is_silent(youmonst.data) ? "writhe" : "scream");
            display_nhwindow(WIN_MESSAGE, FALSE);
            You("cease to exist.");
            if (invent) {
                Your("possessions land on the %s with a thud.",
                     surface(u.ux, u.uy));
            }
            killer.format = NO_KILLER_PREFIX;
            Strcpy(killer.name, "committed suicide");
            done(DIED);
            pline("An energized cloud of dust begins to coalesce.");
            Your("body rematerializes%s.", invent ?
                 ", and you gather up all your possessions" : "");
            return;
#ifdef WIZARD
            /* allow only jump beyond the Dungeons of Doom branch */
        } else if (!wizard && newlev > 0 && u.uz.dnum != 0) {
#else
        } else if (newlev > 0 && u.uz.dnum != 0) {
#endif
            /* random teleport for destination level outside of the current dungeon branch */
            if (newlev > dungeons[u.uz.dnum].depth_start + dunlevs_in_dungeon(&u.uz)) {
                You_feel("like bouncing off a solid wall!");
                goto random_levtport;
            }
        }

        /* if in Knox and the requested level > 0, stay put.
         * we let negative values requests fall into the "heaven" loop.
         */
        if ((Is_knox(&u.uz)
#ifdef BLACKMARKET
             || Is_blackmarket(&u.uz)
#endif
             ) && !force_dest) {
            You("%s", shudder_for_moment);
            return;
        }
        /* if in Quest, the player sees "Home 1", etc., on the status
         * line, instead of the logical depth of the level.  controlled
         * level teleport request is likely to be relativized to the
         * status line, and consequently it should be incremented to
         * the value of the logical depth of the target level.
         *
         * we let negative values requests fall into the "heaven" loop.
         */
        if (In_quest(&u.uz) && newlev > 0) {
            newlev = newlev + dungeons[u.uz.dnum].depth_start - 1;
        }
    } else { /* involuntary level tele */
random_levtport:
        newlev = random_teleport_level();
        if (newlev == depth(&u.uz)) {
            You("%s", shudder_for_moment);
            return;
        }
    }

    if (u.utrap && u.utraptype == TT_BURIEDBALL) {
        buried_ball_to_punishment();
    }

    if (!next_to_u()) {
        You("%s", shudder_for_moment);
        return;
    }
#ifdef WIZARD
    if (In_endgame(&u.uz)) { /* must already be wizard */
        int llimit = dunlevs_in_dungeon(&u.uz);

        if (newlev >= 0 || newlev <= -llimit) {
            You_cant("get there from here.");
            return;
        }
        newlevel.dnum = u.uz.dnum;
        newlevel.dlevel = llimit + newlev;
        schedule_goto(&newlevel, UTOTYPE_NONE, (char *) 0, (char *) 0);
        return;
    }
#endif

    killer.name[0] = 0; /* still alive, so far... */

    if (iflags.debug_fuzzer && newlev < 0) {
        goto random_levtport;
    }

    if (newlev < 0 && !force_dest) {
        if (*u.ushops0) {
            /* take unpaid inventory items off of shop bills */
            in_mklev = TRUE; /* suppress map update */
            u_left_shop(u.ushops0, TRUE);
            /* you're now effectively out of the shop */
            *u.ushops0 = *u.ushops = '\0';
            in_mklev = FALSE;
        }
        if (newlev <= -10) {
            You("arrive in heaven.");
            verbalize("Thou art early, but we'll admit thee.");
            killer.format = NO_KILLER_PREFIX;
            Strcpy(killer.name, "went to heaven prematurely");
        } else if (newlev == -9) {
            You_feel("deliriously happy. ");
            pline("(In fact, you're on Cloud 9!) ");
            display_nhwindow(WIN_MESSAGE, FALSE);
        } else {
            You("are now high above the clouds...");
        }

        if (killer.name[0]) {
            ; /* arrival in heaven is pending */
        } else if (Levitation) {
            escape_by_flying = "float gently down to earth";
        } else if (Flying) {
            escape_by_flying = "fly down to the ground";
        } else {
            pline("Unfortunately, you don't know how to fly.");
            You("plummet a few thousand feet to your death.");
            Sprintf(killer.name,
                    "teleported out of the dungeon and fell to %s death",
                    uhis());
            killer.format = NO_KILLER_PREFIX;
        }
    }

    if (killer.name[0]) {
        /* the chosen destination was not survivable */
        d_level lsav;

        /* set specific death location; this also suppresses bones */
        lsav = u.uz;    /* save current level, see below */
        u.uz.dnum = 0;  /* main dungeon */
        u.uz.dlevel = (newlev <= -10) ? -10 : 0; /* heaven or surface */
        done(DIED);
        /* can only get here via life-saving (or declining to die in
           explore|debug mode); the hero has now left the dungeon... */
        escape_by_flying = "find yourself back on the surface";
        u.uz = lsav; /* restore u.uz so escape code works */
    }

    /* calls done(ESCAPED) if newlevel==0 */
    if (escape_by_flying) {
        You("%s.", escape_by_flying);
        newlevel.dnum = 0;      /* specify main dungeon */
        newlevel.dlevel = 0;    /* escape the dungeon */
        /* [dlevel used to be set to 1, but it doesn't make sense to
           teleport out of the dungeon and float or fly down to the
           surface but then actually arrive back inside the dungeon] */
    } else if (u.uz.dnum == medusa_level.dnum &&
               newlev >= dungeons[u.uz.dnum].depth_start +
               dunlevs_in_dungeon(&u.uz)) {
#ifdef WIZARD
        if (!(wizard && force_dest)) {
#endif
        find_hell(&newlevel);
        }
    } else {
        /* if invocation did not yet occur, teleporting into
         * the last level of Gehennom is forbidden.
         */
#ifdef WIZARD
        if (!wizard) {
#endif
            if (Inhell && !u.uevent.invoked &&
                newlev >= (dungeons[u.uz.dnum].depth_start +
                        dunlevs_in_dungeon(&u.uz) - 1)) {
                newlev = dungeons[u.uz.dnum].depth_start +
                        dunlevs_in_dungeon(&u.uz) - 2;
                pline("Sorry...");
            }
#ifdef WIZARD
        }
#endif
        /* no teleporting out of quest dungeon */
        if (In_quest(&u.uz) && newlev < depth(&qstart_level)) {
            newlev = depth(&qstart_level);
        }
        /* the player thinks of levels purely in logical terms, so
         * we must translate newlev to a number relative to the
         * current dungeon.
         */
#ifdef WIZARD
        if (!(wizard && force_dest)) {
#endif
            get_level(&newlevel, newlev);
#ifdef WIZARD
        }
#endif
    }
    schedule_goto(&newlevel, UTOTYPE_NONE, (char *) 0,
                  flags.verbose ? "You materialize on a different level!" : (char *) 0);
    /* in case player just read a scroll and is about to be asked to
       call it something, we can't defer until the end of the turn */
    if (u.utotype && !flags.mon_moving) {
        deferred_goto();
    }
}

void
domagicportal(struct trap *ttmp)
{
    struct d_level target_level;

    if (u.utrap && u.utraptype == TT_BURIEDBALL) {
        buried_ball_to_punishment();
    }

    if (!next_to_u()) {
        You("%s", shudder_for_moment);
        return;
    }

    /* if landed from another portal, do nothing */
    /* problem: level teleport landing escapes the check */
    if (!on_level(&u.uz, &u.uz0)) {
        return;
    }

    You("activated a magic portal!");

    /* prevent the poor shnook, whose amulet was stolen while in
     * the endgame, from accidently triggering the portal to the
     * next level, and thus losing the game
     */
    if (In_endgame(&u.uz) && !u.uhave.amulet) {
        You_feel("dizzy for a moment, but nothing happens...");
        return;
    }

    /* Prevent the player from using non Quest portals. */
    if (!In_endgame(&u.uz) && u.uhave.amulet &&
        (!at_dgn_entrance("The Quest") && !In_quest(&u.uz))) {
        pline("The Amulet of Yendor drains the energy of this portal!");
        return;
    }

    int utotype = UTOTYPE_PORTAL;
    target_level = ttmp->dst;

    /* choose a random portal stone destination */
    boolean portal_stone_portal = ((ttmp->dst.dnum == -1) && (ttmp->dst.dlevel == -1));
    if (portal_stone_portal) {
        d_level *dest = random_portal_stone_destination();

        if (dest) {
            assign_level(&target_level, dest);
            utotype |= UTOTYPE_PORTAL_STONE;
        } else {
            pline("The magic portal is malfunctioning!");
            tele();
            return;
        }
    }

    schedule_goto(&target_level, utotype,
                  "You feel dizzy for a moment, but the sensation passes.",
                  (char *) 0);
}

void
tele_trap(struct trap *trap)
{
    if (In_endgame(&u.uz) || Antimagic) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
        }
        You_feel("a wrenching sensation.");
    } else if (!next_to_u()) {
        You("%s", shudder_for_moment);
    } else if (trap->once) {
        deltrap(trap);
        newsym(u.ux, u.uy); /* get rid of trap symbol */
        vault_tele();
    } else {
        tele();
    }
}

void
level_tele_trap(struct trap *trap, unsigned int trflags)
{
    char verbbuf[BUFSZ];

    if ((trflags & VIASITTING) != 0) {
        Strcpy(verbbuf, "trigger"); /* follows "You sit down." */
    } else {
        Sprintf(verbbuf, "%s onto",
                Levitation ? (const char *)"float" :
                locomotion(youmonst.data, "step"));
    }
    You("%s a level teleport trap!", verbbuf);

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
    }
    if (Antimagic || In_endgame(&u.uz)) {
        You_feel("a wrenching sensation.");
        return;
    }
    if (!Blind) {
        You("are momentarily blinded by a flash of light.");
    } else {
        You("are momentarily disoriented.");
    }
    deltrap(trap);
    newsym(u.ux, u.uy); /* get rid of trap symbol */
    level_tele();
}

/* check whether monster can arrive at location <x,y> via Tport (or fall) */
static boolean
rloc_pos_ok(
    coordxy x, coordxy y, /**< coordinates of candidate location */
    struct monst *mtmp)
{
    coordxy xx, yy;

    if (!goodpos(x, y, mtmp, GP_CHECKSCARY)) {
        return FALSE;
    }
    /*
     * Check for restricted areas present in some special levels.
     *
     * `xx' is current column; if 0, then `yy' will contain flag bits
     * rather than row:  bit #0 set => moving upwards; bit #1 set =>
     * inside the Wizard's tower.
     */
    xx = mtmp->mx;
    yy = mtmp->my;
    if (!xx) {
        /* no current location (migrating monster arrival) */
        if (dndest.nlx && On_W_tower_level(&u.uz)) {
            return ((yy & 2) != 0) ^ /* inside xor not within */
                   !within_bounded_area(x, y, dndest.nlx, dndest.nly,
                                        dndest.nhx, dndest.nhy);
        }
        if (updest.lx && (yy & 1) != 0) { /* moving up */
            return (within_bounded_area(x, y, updest.lx, updest.ly,
                                        updest.hx, updest.hy) &&
                    (!updest.nlx ||
                     !within_bounded_area(x, y, updest.nlx, updest.nly,
                                          updest.nhx, updest.nhy)));
        }
        if (dndest.lx && (yy & 1) == 0) { /* moving down */
            return (within_bounded_area(x, y, dndest.lx, dndest.ly,
                                        dndest.hx, dndest.hy) &&
                    (!dndest.nlx ||
                     !within_bounded_area(x, y, dndest.nlx, dndest.nly,
                                          dndest.nhx, dndest.nhy)));
        }
    } else {
        /* [try to] prevent a shopkeeper or temple priest from being
           sent out of his room (caller might resort to goodpos() if
           we report failure here, so this isn't full prevention) */
        if (mtmp->isshk && inhishop(mtmp)) {
            if (levl[x][y].roomno != ESHK(mtmp)->shoproom) {
                return FALSE;
            }
        } else if (mtmp->ispriest && inhistemple(mtmp)) {
            if (levl[x][y].roomno != EPRI(mtmp)->shroom) {
                return FALSE;
            }
        }
        /* current location is <xx,yy> */
        if (!tele_jump_ok(xx, yy, x, y)) {
            return FALSE;
        }
    }
    /* <x,y> is ok */
    return TRUE;
}

/*
 * rloc_to()
 *
 * Pulls a monster from its current position and places a monster at
 * a new x and y.  If oldx is 0, then the monster was not in the
 * levels.monsters array.  However, if oldx is 0, oldy may still have
 * a value because mtmp is a migrating_mon.  Worm tails are always
 * placed randomly around the head of the worm.
 */
static void
rloc_to_core(
    struct monst *mtmp,
    coordxy x, coordxy y,
    unsigned rlocflags)
{
    coordxy oldx = mtmp->mx, oldy = mtmp->my;
    boolean resident_shk = mtmp->isshk && inhishop(mtmp);
    boolean preventmsg = (rlocflags & RLOC_NOMSG) != 0;
    boolean vanishmsg = (rlocflags & RLOC_MSG) != 0;
    boolean appearmsg = (mtmp->mstrategy & STRAT_APPEARMSG) != 0;
    boolean domsg = !in_mklev && (vanishmsg || appearmsg) && !preventmsg;
    boolean telemsg = FALSE;

    if (x == mtmp->mx && y == mtmp->my && m_at(x, y) == mtmp) {
        return; /* that was easy */
    }

    if (oldx) { /* "pick up" monster */
        if (domsg && canspotmon(mtmp)) {
            if (couldsee(x, y) || sensemon(mtmp)) {
                telemsg = TRUE;
            } else {
                pline("%s vanishes!", Monnam(mtmp));
            }
            /* avoid "It suddenly appears!" for a STRAT_APPEARMSG monster
               that has just teleported away if we won't see it after this
               vanishing (the regular appears message will be given if we
               do see it) */
            appearmsg = FALSE;
        }

        if (mtmp->wormno) {
            remove_worm(mtmp);
        } else {
            remove_monster(oldx, oldy);
            newsym(oldx, oldy); /* update old location */
        }
    }

    mon_track_clear(mtmp);
    place_monster(mtmp, x, y); /* put monster down */
    update_monster_region(mtmp);

    if (mtmp->wormno) {
        /* now put down tail */
        place_worm_tail_randomly(mtmp, x, y);
    }

    if (u.ustuck == mtmp) {
        if (u.uswallow) {
            u_on_newpos(mtmp->mx, mtmp->my);
            docrt();
        } else if (!next2u(mtmp->mx, mtmp->my)) {
            unstuck(mtmp);
        }
    }

    maybe_unhide_at(x, y);
    newsym(x, y);      /* update new location */
    set_apparxy(mtmp); /* orient monster */

    if (domsg && (canspotmon(mtmp) || appearmsg)) {
        int du = distu(x, y), olddu;
        const char *next = (du <= 2) ? " next to you" : 0, /* next2u() */
                   *nearu = (du <= BOLT_LIM * BOLT_LIM) ? " close by" : 0;

        mtmp->mstrategy &= ~STRAT_APPEARMSG; /* one chance only */
        if (telemsg && (couldsee(x, y) || sensemon(mtmp))) {
            pline("%s vanishes and reappears%s.",
                  Monnam(mtmp),
                  next ? next :
                  nearu ? nearu :
                  ((olddu = distu(oldx, oldy)) == du) ? "" :
                  (du < olddu) ? " closer to you" : " farther away");
        } else {
            pline("%s %s%s%s!",
                  appearmsg ? Amonnam(mtmp) : Monnam(mtmp),
                  appearmsg ? "suddenly " : "",
                  !Blind ? "appears" : "arrives",
                  next ? next :
                  nearu ? nearu : "");
        }
    }

    /* shopkeepers will only teleport if you zap them with a wand of
       teleportation or if they've been transformed into a jumpy monster;
       the latter only happens if you've attacked them with polymorph */
    if (resident_shk && !inhishop(mtmp)) {
        make_angry_shk(mtmp, oldx, oldy);
    }

    /* if a monster carrying shop goods teleports out of the shop, blame
       it on the hero; chance of an unpaid item is vanishingly small, but
       no_charge is easily possible and needs to be cleared if not in shop;
       a for-sale item is ordinary here--shk won't notice it leaving; if
       mtmp teleports from one shop into another, no_charge status sticks
       and an item on the first shk's bill stays there */
    if (mtmp->minvent && !costly_spot(x, y)) {
        struct obj *otmp;
        struct monst *shkp = find_objowner(mtmp->minvent, oldx, oldy);
        boolean peaceful = !shkp || shkp->mpeaceful;

        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
            if (otmp->no_charge)
                otmp->no_charge = 0;
            else if (shkp && onshopbill(otmp, shkp, TRUE))
                stolen_value(otmp, oldx, oldy, peaceful, FALSE);
        }
    }

    /* if hero is busy, maybe stop occupation */
    if (occupation) {
        (void) dochugw(mtmp, FALSE);
    }

    /* trapped monster teleported away */
    if (mtmp->mtrapped && !mtmp->wormno) {
        (void) mintrap(mtmp, NO_TRAP_FLAGS);
    }
}

void
rloc_to(struct monst *mtmp, coordxy x, coordxy y)
{
    rloc_to_core(mtmp, x, y, RLOC_NOMSG);
}

void
rloc_to_flag(
    struct monst *mtmp,
    coordxy x, coordxy y,
    unsigned rlocflags)
{
    rloc_to_core(mtmp, x, y, rlocflags);
}

static stairway *
stairway_find_forwiz(boolean isladder, boolean up)
{
    stairway *stway = stairs;

    while (stway &&
           !(stway->isladder == isladder &&
             stway->up == up &&
             stway->tolev.dnum == u.uz.dnum)) {
        stway = stway->next;
    }

    return stway;
}

/* place a monster at a random location, typically due to teleport;
   return TRUE if successful, FALSE if not; rlocflags is RLOC_foo flags */
boolean
rloc(
    struct monst *mtmp, /**< mtmp->mx==0 implies migrating monster arrival */
    unsigned rlocflags)
{
    coordxy x, y;
    int trycount;

    if (mtmp == u.usteed) {
        tele();
        return TRUE;
    }

    if (mtmp->iswiz && mtmp->mx) { /* Wizard, not just arriving */
        stairway *stway;

        if (!In_W_tower(u.ux, u.uy, &u.uz)) {
            stway = stairway_find_forwiz(FALSE, TRUE);
        } else if (!stairway_find_forwiz(TRUE, FALSE)) { /* bottom of tower */
            stway = stairway_find_forwiz(TRUE, TRUE);
        } else {
            stway = stairway_find_forwiz(TRUE, FALSE);
        }

        x = stway ? stway->sx : 0;
        y = stway ? stway->sy : 0;

        /* if the wiz teleports away to heal, try the up staircase,
           to block the player's escaping before he's healed
           (deliberately use `goodpos' rather than `rloc_pos_ok' here) */
        if (goodpos(x, y, mtmp, 0)) {
            goto found_xy;
        }
    }

    trycount = 0;
    do {
        x = rn1(COLNO-3, 2);
        y = rn2(ROWNO);
        if ((trycount < 500) ? rloc_pos_ok(x, y, mtmp) :
                               goodpos(x, y, mtmp, NO_MM_FLAGS)) {
            goto found_xy;
        }
    } while (++trycount < 1000);

    /* last ditch attempt to find a good place */
    for (x = 2; x < COLNO - 1; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (goodpos(x, y, mtmp, 0)) {
                goto found_xy;
            }
        }
    }

    /* level either full of monsters or somehow faulty */
    if ((rlocflags & RLOC_ERR) != 0) {
        warning("rloc(): couldn't relocate monster");
    }
    return FALSE;

found_xy:
    rloc_to_core(mtmp, x, y, rlocflags);
    return TRUE;
}

static void
mvault_tele(struct monst *mtmp)
{
    struct mkroom *croom = search_special(VAULT);
    coord c;

    if (croom && somexy(croom, &c) &&
        goodpos(c.x, c.y, mtmp, 0)) {
        rloc_to(mtmp, c.x, c.y);
        return;
    }
    (void) rloc(mtmp, RLOC_NONE);
}

boolean
tele_restrict(struct monst *mon)
{
    if (noteleport_level(mon)) {
        if (canseemon(mon)) {
            pline("A mysterious force prevents %s from teleporting!", mon_nam(mon));
        }
        return TRUE;
    }
    return FALSE;
}

void
mtele_trap(struct monst *mtmp, struct trap *trap, int in_sight)
{
    char *monname;

    if (tele_restrict(mtmp)) {
        return;
    }
    if (teleport_pet(mtmp, FALSE)) {
        /* save name with pre-movement visibility */
        monname = Monnam(mtmp);

        /* Note: don't remove the trap if a vault.  Other-
         * wise the monster will be stuck there, since
         * the guard isn't going to come for it...
         */
        if (trap->once) {
            mvault_tele(mtmp);
        } else {
            (void) rloc(mtmp, RLOC_NONE);
        }

        if (in_sight) {
            if (canseemon(mtmp)) {
                pline("%s seems disoriented.", monname);
            } else {
                pline("%s suddenly disappears!", monname);
            }
            seetrap(trap);
        }
    }
}

/* return 0 if still on level, 3 if not */
int
mlevel_tele_trap(struct monst *mtmp, struct trap *trap, boolean force_it, int in_sight)
{
    int tt = (trap ? trap->ttyp : NO_TRAP);
    struct permonst *mptr = mtmp->data;

    if (mtmp == u.ustuck) { /* probably a vortex */
        return 0;       /* temporary? kludge */
    }
    if (teleport_pet(mtmp, force_it)) {
        d_level tolevel;
        int migrate_typ = MIGR_RANDOM;

        if (is_hole(tt)) {
            if (Is_stronghold(&u.uz)) {
                assign_level(&tolevel, &valley_level);
            } else if (Is_botlevel(&u.uz)) {
                if (in_sight && trap->tseen) {
                    pline("%s avoids the %s.", Monnam(mtmp),
                          (tt == HOLE) ? "hole" : "trap");
                }
                return 0;
            } else {
                get_level(&tolevel, depth(&u.uz) + 1);
            }
        } else if (tt == MAGIC_PORTAL) {
            boolean portal_stone_portal = ((trap->dst.dnum == -1) && (trap->dst.dlevel == -1));
            if (In_endgame(&u.uz) &&
                (mon_has_amulet(mtmp) || is_home_elemental(mptr) || rn2(7))) {
                if (in_sight && mptr->mlet != S_ELEMENTAL) {
                    pline("%s seems to shimmer for a moment.",
                          Monnam(mtmp));
                    seetrap(trap);
                }
                return 0;
#ifdef BLACKMARKET
            } else if (mtmp->mtame &&
                       (Is_blackmarket(&trap->dst) || Is_blackmarket(&u.uz))) {
                if (in_sight) {
                    pline("%s seems to shimmer for a moment.",
                          Monnam(mtmp));
                    seetrap(trap);
                }
                return 0;
            } else if (Is_blackmarket(&u.uz) &&
                       mtmp->data == &mons[PM_ONE_EYED_SAM]) {
                return 0;
#endif /* BLACKMARKET */
            } else if (portal_stone_portal) {
                d_level *dst = random_portal_stone_destination();

                if (dst) {
                    assign_level(&tolevel, dst);
                    migrate_typ = MIGR_PORTAL;
                } else {
                    pline("%s seems to shimmer for a moment.", Monnam(mtmp));
                    return 0;
                }
            } else {
                assign_level(&tolevel, &trap->dst);
                migrate_typ = MIGR_PORTAL;
            }
        } else if (tt == LEVEL_TELEP || tt == NO_TRAP) {
            int nlev;

            if (mon_has_amulet(mtmp) || In_endgame(&u.uz) ||
                /* NO_TRAP is used when forcing a monster off the level;
                   onscary(0,0,) is true for the Wizard, Riders, lawful
                   minions, Angels of any alignment, shopkeeper or priest
                   currently inside his or her own special room */
                 (tt == NO_TRAP && onscary(0, 0, mtmp))) {
                if (in_sight) {
                    pline("%s seems very disoriented for a moment.",
                          Monnam(mtmp));
                }
                return 0;
            }
            if (tt == NO_TRAP) {
                /* creature is being forced off the level to make room;
                   it will try to return to this level (at a random spot
                   rather than its current one) if the level is left by
                   the hero and then revisited */
                assign_level(&tolevel, &u.uz);
            } else {
                nlev = random_teleport_level();
                if (nlev == depth(&u.uz)) {
                    if (in_sight) {
                        pline("%s shudders for a moment.", Monnam(mtmp));
                    }
                    return 0;
                }
                get_level(&tolevel, nlev);
            }
        } else {
            impossible("mlevel_tele_trap: unexpected trap type (%d)", tt);
            return 0;
        }

        if (in_sight) {
            pline("Suddenly, %s disappears out of sight.", mon_nam(mtmp));
            if (trap) {
                seetrap(trap);
            }
        }
        migrate_to_level(mtmp, ledger_no(&tolevel),
                         migrate_typ, (coord *)0);
        return 3; /* no longer on this level */
    }
    return 0;
}


/* place object randomly, returns False if it's gone (eg broken) */
boolean
rloco(struct obj *obj)
{
    coordxy tx, ty, otx, oty;
    boolean restricted_fall;
    int try_limit = 4000;

    if (obj->otyp == CORPSE && is_rider(&mons[obj->corpsenm])) {
        if (revive_corpse(obj)) {
            return FALSE;
        }
    }

    obj_extract_self(obj);
    otx = obj->ox;
    oty = obj->oy;
    restricted_fall = (otx == 0 && dndest.lx);
    do {
        tx = rn1(COLNO-3, 2);
        ty = rn2(ROWNO);
        if (!--try_limit) {
            break;
        }
    } while (!goodpos(tx, ty, (struct monst *)0, 0) ||
             /* bug: this lacks provision for handling the Wizard's tower */
             (restricted_fall &&
              (!within_bounded_area(tx, ty, dndest.lx, dndest.ly,
                                    dndest.hx, dndest.hy) ||
               (dndest.nlx &&
                within_bounded_area(tx, ty, dndest.nlx, dndest.nly,
                                    dndest.nhx, dndest.nhy))))
             ||
             /* on the Wizard Tower levels, objects inside should
                stay inside and objects outside should stay outside */
             (dndest.nlx && On_W_tower_level(&u.uz) &&
                 within_bounded_area(tx, ty, dndest.nlx, dndest.nly, dndest.nhx, dndest.nhy) !=
                 within_bounded_area(otx, oty, dndest.nlx, dndest.nly, dndest.nhx, dndest.nhy)));

    if (flooreffects(obj, tx, ty, "fall")) {
        /* update old location since flooreffects() couldn't;
           unblock_point() for boulder handled by obj_extract_self() */
        newsym(otx, oty);
        return FALSE;
    } else if (otx == 0 && oty == 0) {
        ; /* fell through a trap door; no update of old loc needed */
    } else {
        if (costly_spot(otx, oty)
            && (!costly_spot(tx, ty) ||
                !index(in_rooms(tx, ty, 0), *in_rooms(otx, oty, 0)))) {
            if (costly_spot(u.ux, u.uy) &&
                index(u.urooms, *in_rooms(otx, oty, 0)))
                addtobill(obj, FALSE, FALSE, FALSE);
            else (void)stolen_value(obj, otx, oty, FALSE, FALSE);
        }
        newsym(otx, oty); /* update old location */
    }
    place_object(obj, tx, ty);
    /* note: block_point() for boulder handled by place_object() */
    newsym(tx, ty);
    return TRUE;
}

/* Returns an absolute depth */
int
random_teleport_level(void)
{
    int nlev, max_depth, min_depth,
        cur_depth = (int)depth(&u.uz);

    /* [the endgame case can only occur in wizard mode] */
    if (!rn2(5) || Is_knox(&u.uz) || In_endgame(&u.uz)
#ifdef BLACKMARKET
        || Is_blackmarket(&u.uz)
#endif
        )
        return cur_depth;

    /* What I really want to do is as follows:
     * -- If in a dungeon that goes down, the new level is to be restricted
     *    to [top of parent, bottom of current dungeon]
     * -- If in a dungeon that goes up, the new level is to be restricted
     *    to [top of current dungeon, bottom of parent]
     * -- If in a quest dungeon or similar dungeon entered by portals,
     *    the new level is to be restricted to [top of current dungeon,
     *    bottom of current dungeon]
     * The current behavior is not as sophisticated as that ideal, but is
     * still better what we used to do, which was like this for players
     * but different for monsters for no obvious reason.  Currently, we
     * must explicitly check for special dungeons.  We check for Knox
     * above; endgame is handled in the caller due to its different
     * message ("disoriented").
     * --KAA
     * 3.4.2: explicitly handle quest here too, to fix the problem of
     * monsters sometimes level teleporting out of it into main dungeon.
     * Also prevent monsters reaching the Sanctum prior to invocation.
     */
    if (In_quest(&u.uz)) {
        int bottom = dunlevs_in_dungeon(&u.uz);
        int qlocate_depth = qlocate_level.dlevel;

        /* if hero hasn't reached the middle locate level yet,
           no one can randomly teleport past it */
        if (dunlev_reached(&u.uz) < qlocate_depth) {
            bottom = qlocate_depth;
        }
        min_depth = dungeons[u.uz.dnum].depth_start;
        max_depth = bottom + (dungeons[u.uz.dnum].depth_start - 1);
    } else {
        min_depth = 1;
        max_depth = dunlevs_in_dungeon(&u.uz) + (dungeons[u.uz.dnum].depth_start - 1);
        /* can't reach Sanctum if the invocation hasn't been performed */
        if (Inhell && !u.uevent.invoked) {
            max_depth -= 1;
        }
    }

    /* Get a random value relative to the current dungeon */
    /* Range is 1 to current+3, current not counting */
    nlev = rn2(cur_depth + 3 - min_depth) + min_depth;
    if (nlev >= cur_depth) {
        nlev++;
    }

    if (nlev > max_depth) {
        nlev = max_depth;
        /* teleport up if already on bottom */
        if (Is_botlevel(&u.uz)) {
            nlev -= rnd(3);
        }
    }
    if (nlev < min_depth) {
        nlev = min_depth;
        if (nlev == cur_depth) {
            nlev += rnd(3);
            if (nlev > max_depth) {
                nlev = max_depth;
            }
        }
    }
    return nlev;
}

/** You teleport a monster (via wand, spell, or poly'd q.mechanic attack).
   returns false iff the attempt fails */
boolean
u_teleport_mon(struct monst *mtmp, boolean give_feedback)
{
    coord cc;

    if (mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE)) {
        if (give_feedback) {
            pline("%s resists your magic!", Monnam(mtmp));
        }
        return FALSE;
    } else if (u.uswallow && mtmp == u.ustuck && noteleport_level(mtmp)) {
        if (give_feedback) {
            You("are no longer inside %s!", mon_nam(mtmp));
        }
        unstuck(mtmp);
        (void) rloc(mtmp, RLOC_MSG);
#ifdef BLACKMARKET
    } else if (((mtmp->data == &mons[PM_BLACK_MARKETEER] && rn2(5)) ||
                (mtmp->data == &mons[PM_ONE_EYED_SAM] && rn2(13))) &&
               enexto_core_range(&cc, u.ux, u.uy, mtmp->data, 0,
                                 rnf(1, 10) ? 4 : 3)) {
        rloc_to(mtmp, cc.x, cc.y);
#endif
    } else if (is_rider(mtmp->data) && rn2(13) &&
               enexto(&cc, u.ux, u.uy, mtmp->data))
        rloc_to(mtmp, cc.x, cc.y);
    else
        (void) rloc(mtmp, RLOC_MSG);
    return TRUE;
}

/*teleport.c*/
