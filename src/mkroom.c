/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Entry points:
 *  mkroom() -- make and stock a room of a given type
 *  nexttodoor() -- return TRUE if adjacent to a door
 *  has_dnstairs() -- return TRUE if given room has a down staircase
 *  has_upstairs() -- return TRUE if given room has an up staircase
 *  courtmon() -- generate a court monster
 *  save_rooms() -- save rooms into file fd
 *  rest_rooms() -- restore rooms from file fd
 *  cmap_to_type() -- convert S_xxx symbol to XXX topology code
 */

#include "hack.h"

static boolean isbig(struct mkroom *);
static void mkshop(void), mkzoo(int), mkswamp(void);
static void mktemple(void);
static void mkgarden(struct mkroom *);
static coord * shrine_pos(int);
static struct permonst * morguemon(void);
static struct permonst * antholemon(void);
static struct permonst * squadmon(void);
static struct permonst * armorymon(void);
static void save_room(int, struct mkroom *);
static void rest_room(int, struct mkroom *);

#define sq(x) ((x)*(x))

extern const struct shclass shtypes[];  /* defined in shknam.c */

static boolean
isbig(register struct mkroom *sroom)
{
    register int area = (sroom->hx - sroom->lx + 1)
                        * (sroom->hy - sroom->ly + 1);

    return((boolean)( area > 20 ));
}

/* make and stock a room of a given type */
void
mkroom(int roomtype)
{
    if (roomtype >= SHOPBASE)
        mkshop(); /* someday, we should be able to specify shop type */
    else switch(roomtype) {
        case COURT: mkzoo(COURT); break;
        case ZOO:   mkzoo(ZOO); break;
        case BEEHIVE:   mkzoo(BEEHIVE); break;
        case MORGUE:    mkzoo(MORGUE); break;
        case BARRACKS:  mkzoo(BARRACKS); break;
        case SWAMP: mkswamp(); break;
        case GARDEN:    mkgarden((struct mkroom *)0); break;
        case TEMPLE:    mktemple(); break;
        case LEPREHALL: mkzoo(LEPREHALL); break;
        case COCKNEST:  mkzoo(COCKNEST); break;
        case ARMORY:    mkzoo(ARMORY); break;
        case ANTHOLE:   mkzoo(ANTHOLE); break;
        case LEMUREPIT: mkzoo(LEMUREPIT); break;
        case POOLROOM:  mkpoolroom(); break;
        default:    impossible("Tried to make a room of type %d.", roomtype);
        }
}

static void
mkshop(void)
{
    register struct mkroom *sroom;
    int i = -1;
#ifdef WIZARD
    char *ep = (char *)0;   /* (init == lint suppression) */

    /* first determine shoptype */
    if(wizard) {
#ifndef MAC
        ep = nh_getenv("SHOPTYPE");
        if(ep) {
            for (i=0; shtypes[i].name; i++) {
                if (!strcmp(shtypes[i].name, ep) ||
                    (*ep == def_oc_syms[(int)shtypes[i].symb])) {
                    goto gottype;
                }
            }
            if(*ep == 'z' || *ep == 'Z') {
                mkzoo(ZOO);
                return;
            }
            if(*ep == 'm' || *ep == 'M') {
                mkzoo(MORGUE);
                return;
            }
            if(*ep == 'b' || *ep == 'B') {
                mkzoo(BEEHIVE);
                return;
            }
            if(*ep == 'p' || *ep == 'P') {
                mkzoo(LEMUREPIT);
                return;
            }
            if(*ep == 't' || *ep == 'T' || *ep == '\\') {
                mkzoo(COURT);
                return;
            }
            if(*ep == 's' || *ep == 'S') {
                mkzoo(BARRACKS);
                return;
            }
            if(*ep == 'a' || *ep == 'A') {
                mkzoo(ANTHOLE);
                return;
            }
            if(*ep == 'c' || *ep == 'C') {
                mkzoo(COCKNEST);
                return;
            }
            if(*ep == 'r' || *ep == 'R') {
                mkzoo(ARMORY);
                return;
            }
            if(*ep == 'l' || *ep == 'L') {
                mkzoo(LEPREHALL);
                return;
            }
            if(*ep == 'o' || *ep == 'O') {
                mkpoolroom();
                return;
            }
            if(*ep == '_') {
                mktemple();
                return;
            }
            if(*ep == 'n') {
                mkgarden((struct mkroom *)0);
                return;
            }
            if(*ep == '}') {
                mkswamp();
                return;
            }
            if(*ep == 'g' || *ep == 'G')
                i = 0;
            else
                i = -1;
        }
#endif
    }
#ifndef MAC
gottype:
#endif
#endif
    for(sroom = &rooms[0]; ; sroom++) {
        if(sroom->hx < 0) return;
        if(sroom - rooms >= nroom) {
            pline("rooms not closed by -1?");
            return;
        }
        if(sroom->rtype != OROOM) continue;
        if(has_dnstairs(sroom) || has_upstairs(sroom))
            continue;
        if(
#ifdef WIZARD
            (wizard && ep && sroom->doorct != 0) ||
#endif
            sroom->doorct == 1) break;
    }
    if (!sroom->rlit) {
        int x, y;

        for(x = sroom->lx - 1; x <= sroom->hx + 1; x++)
            for(y = sroom->ly - 1; y <= sroom->hy + 1; y++)
                levl[x][y].lit = 1;
        sroom->rlit = 1;
    }

    if(i < 0) {         /* shoptype not yet determined */
        register int j;

        /* pick a shop type at random */
        for (j = rnd(100), i = 0; (j -= shtypes[i].prob) > 0; i++)
            continue;

        /* big rooms cannot be wand or book shops,
         * - so make them general stores
         */
        if(isbig(sroom) && (shtypes[i].symb == WAND_CLASS
                            || shtypes[i].symb == SPBOOK_CLASS)) i = 0;

        /* all rainbow shops are general stores */
        if (is_rainbow_shop(sroom)) {
            i = 0;
        }
    }
    sroom->rtype = SHOPBASE + i;

    /* set room bits before stocking the shop */
#ifdef SPECIALIZATION
    topologize(sroom, FALSE); /* doesn't matter - this is a special room */
#else
    topologize(sroom);
#endif

    /* stock the room with a shopkeeper and artifacts */
    stock_room(i, sroom);
}

/* pick an unused room, preferably with only one door */
struct mkroom *
pick_room(boolean strict)
{
    struct mkroom *sroom;
    int i = nroom;

    for (sroom = &rooms[rn2(nroom)]; i--; sroom++) {
        if(sroom == &rooms[nroom])
            sroom = &rooms[0];
        if(sroom->hx < 0)
            return (struct mkroom *)0;
        if(sroom->rtype != OROOM) continue;
        if(!strict) {
            if(has_upstairs(sroom) || (has_dnstairs(sroom) && rn2(3)))
                continue;
        } else if(has_upstairs(sroom) || has_dnstairs(sroom))
            continue;
        if(sroom->doorct == 1 || !rn2(5)
#ifdef WIZARD
           || wizard
#endif
           )
            return sroom;
    }
    return (struct mkroom *)0;
}

static void
mkzoo(int type)
{
    register struct mkroom *sroom;

    if ((sroom = pick_room(FALSE)) != 0) {
        sroom->rtype = type;
        fill_zoo(sroom);
    }
}

void
mk_zoo_thronemon(int x, int y)
{
    int i = rnd(level_difficulty());
    int pm = (i > 9) ? PM_OGRE_KING :
             (i > 5) ? PM_ELVENKING :
             (i > 2) ? PM_DWARF_KING : PM_GNOME_KING;
    struct monst *mon = makemon(&mons[pm], x, y, NO_MM_FLAGS);

    if (mon) {
        mon->msleeping = 1;
        mon->mpeaceful = 0;
        set_malign(mon);
        /* Give him a sceptre to pound in judgment */
        (void) mongets(mon, MACE);
    }
}

void
fill_zoo(struct mkroom *sroom)
{
    struct monst *mon;
    register int sx, sy, i;
    int sh, tx = 0, ty = 0, goldlim = 0, type = sroom->rtype;
    int rmno = (sroom - rooms) + ROOMOFFSET;
    coord mm;

    sh = sroom->fdoor;
    switch (type) {
    case GARDEN:
        mkgarden(sroom);
        /* mkgarden() sets flags and we don't want other fillings */
        return;

    case COURT:
        if(level.flags.is_maze_lev) {
            for(tx = sroom->lx; tx <= sroom->hx; tx++)
                for(ty = sroom->ly; ty <= sroom->hy; ty++)
                    if(IS_THRONE(levl[tx][ty].typ))
                        goto throne_placed;
        }
        i = 100;
        do {
            /* don't place throne on top of stairs */
            (void) somexy(sroom, &mm);
            tx = mm.x;
            ty = mm.y;
        } while (occupied((xchar)tx, (xchar)ty) && --i > 0);
throne_placed:
        mk_zoo_thronemon(tx, ty);
        break;

    case BEEHIVE:
        tx = sroom->lx + (sroom->hx - sroom->lx + 1)/2;
        ty = sroom->ly + (sroom->hy - sroom->ly + 1)/2;
        if(sroom->irregular) {
            /* center might not be valid, so put queen elsewhere */
            if ((int) levl[tx][ty].roomno != rmno ||
                levl[tx][ty].edge) {
                (void) somexy(sroom, &mm);
                tx = mm.x;
                ty = mm.y;
            }
        }
        break;

    case ZOO:
    case LEPREHALL:
        goldlim = 500 * level_difficulty();
        break;
    }

    for (sx = sroom->lx; sx <= sroom->hx; sx++) {
        for (sy = sroom->ly; sy <= sroom->hy; sy++) {
            if (sroom->irregular) {
                if ((int) levl[sx][sy].roomno != rmno ||
                    levl[sx][sy].edge ||
                    (sroom->doorct &&
                     distmin(sx, sy, doors[sh].x, doors[sh].y) <= 1))
                    continue;
            } else if (!SPACE_POS(levl[sx][sy].typ) ||
                      (sroom->doorct &&
                       ((sx == sroom->lx && doors[sh].x == sx-1) ||
                        (sx == sroom->hx && doors[sh].x == sx+1) ||
                        (sy == sroom->ly && doors[sh].y == sy-1) ||
                        (sy == sroom->hy && doors[sh].y == sy+1))))
                continue;
            /* don't place monster on explicitly placed throne */
            if (type == COURT && IS_THRONE(levl[sx][sy].typ)) {
                continue;
            }
            mon = ((struct monst *)0);
            if (type == ARMORY) {
                /* armories don't contain as many monsters */
                if (rnf(1, 3)) {
                    mon = makemon(armorymon(), sx, sy, MM_ASLEEP);
                }
            } else {
                mon = makemon(
                    (type == COURT) ? courtmon() :
                    (type == BARRACKS) ? squadmon() :
                    (type == MORGUE) ? morguemon() :
                    (type == BEEHIVE) ?
                    (sx == tx && sy == ty ? &mons[PM_QUEEN_BEE] :
                     &mons[PM_KILLER_BEE]) :
                    (type == LEMUREPIT) ?
                    (!rn2(10) ? &mons[PM_HORNED_DEVIL] :
                     &mons[PM_LEMURE]) :
                    (type == LEPREHALL) ? &mons[PM_LEPRECHAUN] :
                    (type == COCKNEST) ? &mons[PM_COCKATRICE] :
                    (type == ANTHOLE) ? antholemon() :
                    (struct permonst *) 0,
                    sx, sy, MM_ASLEEP);
            }
            if (mon) {
                mon->msleeping = 1;
                if (type==COURT && mon->mpeaceful) {
                    mon->mpeaceful = 0;
                    set_malign(mon);
                }
            }
            switch(type) {
            case ZOO:
            case LEPREHALL:
                if(sroom->doorct)
                {
                    int distval = dist2(sx, sy, doors[sh].x, doors[sh].y);
                    i = sq(distval);
                }
                else
                    i = goldlim;
                if(i >= goldlim) i = 5*level_difficulty();
                goldlim -= i;
                (void) mkgold((long) rn1(i, 10), sx, sy);
                break;
            case MORGUE:
                if(!rn2(5))
                    (void) mk_tt_object(CORPSE, sx, sy);
                if(!rn2(10)) /* lots of treasure buried with dead */
                    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST,
                                     sx, sy, TRUE, FALSE);
                if (!rn2(5)) {
                    make_grave(sx, sy, (char *)0);
                } else if (In_moria(&u.uz) && rnf(1, 1000)) {
                    /* there's only one graveyard in Moria */
                    static int once = 1;
                    if (once == 1) {
                        make_grave(sx, sy, "Guest41, Wherever you are, I hope you're doing fine");
                    }
                    once++;
                }
                break;
            case BEEHIVE:
                if(!rn2(3))
                    (void) mksobj_at(LUMP_OF_ROYAL_JELLY,
                                     sx, sy, TRUE, FALSE);
                break;
            case BARRACKS:
                if(!rn2(20)) /* the payroll and some loot */
                    (void) mksobj_at((rn2(3) || depth(&u.uz) < 16) ? CHEST : IRON_SAFE,
                                     sx, sy, TRUE, FALSE);
                break;
            case COCKNEST:
                if(!rn2(3)) {
                    struct obj *sobj = mk_tt_object(STATUE, sx, sy);

                    if (sobj) {
                        for (i = rn2(5); i; i--)
                            (void) add_to_container(sobj,
                                                    mkobj(RANDOM_CLASS, FALSE));
                        sobj->owt = weight(sobj);
                    }
                }
                break;
            case ARMORY:
            {
                struct obj *otmp;
                if (rnf(3, 4)) {
                    if (rn2(2)) {
                        otmp = mkobj_at(WEAPON_CLASS, sx, sy, FALSE);
                    } else {
                        otmp = mkobj_at(ARMOR_CLASS, sx, sy, FALSE);
                    }
                    otmp->spe = 0;
                    if (is_rustprone(otmp)) {
                        otmp->oeroded = rn2(4);
                    } else if (is_rottable(otmp)) {
                        otmp->oeroded2 = rn2(4);
                    }
                }
            }
            break;
            case ANTHOLE:
                if(!rn2(3))
                    (void) mkobj_at(FOOD_CLASS, sx, sy, FALSE);
                break;
            }
        }
    }

    switch (type) {
    case COURT:
    {
        struct obj *chest;
        levl[tx][ty].typ = THRONE;
        (void) somexy(sroom, &mm);
        /* the royal coffers */
        if (depth(&u.uz) > 15) {
            chest = mksobj_at(IRON_SAFE, mm.x, mm.y, TRUE, FALSE);
        } else {
            chest = mksobj_at(CHEST, mm.x, mm.y, TRUE, FALSE);
        }
        struct obj *gold = mkgold(rn1(50 * level_difficulty(), 10), mm.x, mm.y);
        obj_extract_self(gold);
        add_to_container(chest, gold);
        chest->owt = weight(chest);
        chest->spe = 2;   /* so it can be found later */
        level.flags.has_court = 1;
        break;
    }
    case BARRACKS:
        level.flags.has_barracks = 1;
        break;
    case ZOO:
        level.flags.has_zoo = 1;
        break;
    case MORGUE:
        level.flags.has_morgue = 1;
        break;
    case SWAMP:
        level.flags.has_swamp = 1;
        break;
    case BEEHIVE:
        level.flags.has_beehive = 1;
        break;
    case LEMUREPIT:
        level.flags.has_lemurepit = 1;
        break;

    }
}

/* make a swarm of undead around mm */
void
mkundead(coord *mm, boolean revive_corpses, int mm_flags)
{
    int cnt = (level_difficulty() + 1)/10 + rnd(5);
    struct permonst *mdat;
    struct obj *otmp;
    coord cc;

    while (cnt--) {
        mdat = morguemon();
        if (mdat && enexto(&cc, mm->x, mm->y, mdat) &&
            (!revive_corpses ||
             !(otmp = sobj_at(CORPSE, cc.x, cc.y)) ||
             !revive(otmp, FALSE)))
            (void) makemon(mdat, cc.x, cc.y, mm_flags);
    }
    level.flags.graveyard = TRUE; /* reduced chance for undead corpse */
}

static struct permonst *
morguemon(void)
{
    register int i = rn2(100), hd = rn2(level_difficulty());

    if (hd > 10 && i < 10) {
        if (Inhell || In_endgame(&u.uz)) {
            return mkclass(S_DEMON, 0);
        } else {
            int ndemon_res = ndemon(A_NONE);
            if (ndemon_res != NON_PM) {
                return &mons[ndemon_res];
            }
            /* else do what? As is, it will drop to ghost/wraith/zombie */
        }
    }

    if (hd > 8 && i > 85) {
        return mkclass(S_VAMPIRE, 0);
    }

    return ((i < 20) ? &mons[PM_GHOST] :
            (i < 40) ? &mons[PM_WRAITH] : mkclass(S_ZOMBIE, 0));
}

static struct permonst *
antholemon(void)
{
    int mtyp;

    /* Same monsters within a level, different ones between levels */
    switch ((level_difficulty() + game_seed) % 4) {
    default:    mtyp = PM_GIANT_ANT; break;
    case 0:     mtyp = PM_SOLDIER_ANT; break;
    case 1:     mtyp = PM_FIRE_ANT; break;
    case 2:     mtyp = PM_SNOW_ANT; break;
    }
    return ((mvitals[mtyp].mvflags & G_GONE) ?
            (struct permonst *)0 : &mons[mtyp]);
}

static struct permonst *
armorymon(void)
{
    return (rnf(1, 5) ? mkclass(S_RUSTMONST, 0) : &mons[PM_BROWN_PUDDING]);
}

/** Create a special room with trees, fountains and nymphs.
 * @author Pasi Kallinen
 */
static void
mkgarden(struct mkroom *croom) /**< NULL == choose random room */
{
    register int tryct = 0;
    boolean maderoom = FALSE;
    coord pos;
    register int i, tried;

    while ((tryct++ < 25) && !maderoom) {
        register struct mkroom *sroom = croom ? croom : &rooms[rn2(nroom)];

        if (sroom->hx < 0 || (!croom && (sroom->rtype != OROOM ||
                                         !sroom->rlit || has_upstairs(sroom) || has_dnstairs(sroom))))
            continue;

        sroom->rtype = GARDEN;
        maderoom = TRUE;
        level.flags.has_garden = 1;

        tried = 0;
        i = rn1(5, 3);
        while ((tried++ < 50) && (i >= 0) && somexy(sroom, &pos)) {
            struct permonst *pmon;
            if (!MON_AT(pos.x, pos.y) && (pmon = mkclass(S_NYMPH, 0))) {
                struct monst *mtmp = makemon(pmon, pos.x, pos.y, NO_MM_FLAGS);
                if (mtmp) mtmp->msleeping = 1;
                i--;
            }
        }
        tried = 0;
        i = rn1(3, 3);
        while ((tried++ < 50) && (i >= 0) && somexy(sroom, &pos)) {
            if (levl[pos.x][pos.y].typ == ROOM && !MON_AT(pos.x, pos.y) &&
                !nexttodoor(pos.x, pos.y)) {
                if (rn2(3))
                    levl[pos.x][pos.y].typ = TREE;
                else {
                    levl[pos.x][pos.y].typ = FOUNTAIN;
                    level.flags.nfountains++;
                }
                i--;
            }
        }
    }
}

static void
mkswamp(void) /* Michiel Huisjes & Fred de Wilde */
{
    register struct mkroom *sroom;
    register int sx, sy, i, eelct = 0;

    for(i=0; i<5; i++) {        /* turn up to 5 rooms swampy */
        sroom = &rooms[rn2(nroom)];
        if(sroom->hx < 0 || sroom->rtype != OROOM ||
           has_upstairs(sroom) || has_dnstairs(sroom))
            continue;

        /* satisfied; make a swamp */
        sroom->rtype = SWAMP;
        for(sx = sroom->lx; sx <= sroom->hx; sx++)
            for(sy = sroom->ly; sy <= sroom->hy; sy++)
                if(!OBJ_AT(sx, sy) &&
                   !MON_AT(sx, sy) && !t_at(sx, sy) && !nexttodoor(sx, sy)) {
                    if((sx+sy)%2) {
                        levl[sx][sy].typ = POOL;
                        if(!eelct || !rn2(4)) {
                            /* mkclass() won't do, as we might get kraken */
                            (void) makemon(rn2(5) ? &mons[PM_GIANT_EEL]
                                           : rn2(2) ? &mons[PM_PIRANHA]
                                           : &mons[PM_ELECTRIC_EEL],
                                           sx, sy, NO_MM_FLAGS);
                            eelct++;
                        }
                    } else
                    if(!rn2(4)) /* swamps tend to be moldy */
                        (void) makemon(mkclass(S_FUNGUS, 0),
                                       sx, sy, NO_MM_FLAGS);
                    else if(!rn2(6))
                        levl[sx][sy].typ = BOG;
                    if (!rn2(2)) /* swamp ferns like swamps */
                        (void) makemon(&mons[PM_SWAMP_FERN],
                                       sx, sy, NO_MM_FLAGS);
                }
        level.flags.has_swamp = 1;
    }
}

static coord *
shrine_pos(int roomno)
{
    static coord buf;
    int delta;
    struct mkroom *troom = &rooms[roomno - ROOMOFFSET];

    /* if width and height are odd, placement will be the exact center;
       if either or both are even, center point is a hypothetical spot
       between map locations and placement will be adjacent to that */
    delta = troom->hx - troom->lx;
    buf.x = troom->lx + delta / 2;
    if ((delta % 2) && rn2(2)) {
        buf.x++;
    }
    delta = troom->hy - troom->ly;
    buf.y = troom->ly + delta / 2;
    if ((delta % 2) && rn2(2)) {
        buf.y++;
    }
    return &buf;
}

static void
mktemple(void)
{
    register struct mkroom *sroom;
    coord *shrine_spot;
    register struct rm *lev;

    if(!(sroom = pick_room(TRUE))) return;

    /* set up Priest and shrine */
    sroom->rtype = TEMPLE;
    /*
     * In temples, shrines are blessed altars
     * located in the center of the room
     */
    shrine_spot = shrine_pos((sroom - rooms) + ROOMOFFSET);
    lev = &levl[shrine_spot->x][shrine_spot->y];
    lev->typ = ALTAR;
    lev->altarmask = induced_align(80);
    priestini(&u.uz, sroom, shrine_spot->x, shrine_spot->y, FALSE);
    lev->altarmask |= AM_SHRINE;
    level.flags.has_temple = 1;
}

boolean
nexttodoor(register int sx, register int sy)
{
    register int dx, dy;
    register struct rm *lev;
    for(dx = -1; dx <= 1; dx++) for(dy = -1; dy <= 1; dy++) {
            if(!isok(sx+dx, sy+dy)) continue;
            if(IS_DOOR((lev = &levl[sx+dx][sy+dy])->typ) ||
               lev->typ == SDOOR)
                return(TRUE);
        }
    return(FALSE);
}

boolean
has_dnstairs(register struct mkroom *sroom)
{
    if (sroom == dnstairs_room)
        return TRUE;
    if (sstairs.sx && !sstairs.up)
        return((boolean)(sroom == sstairs_room));
    return FALSE;
}

boolean
has_upstairs(register struct mkroom *sroom)
{
    if (sroom == upstairs_room)
        return TRUE;
    if (sstairs.sx && sstairs.up)
        return((boolean)(sroom == sstairs_room));
    return FALSE;
}

int
somex(register struct mkroom *croom)
{
    return rn1(croom->hx - croom->lx + 1, croom->lx);
}

int
somey(register struct mkroom *croom)
{
    return rn1(croom->hy - croom->ly + 1, croom->ly);
}

boolean
inside_room(struct mkroom *croom, xchar x, xchar y)
{
    return((boolean)(x >= croom->lx-1 && x <= croom->hx+1 &&
                     y >= croom->ly-1 && y <= croom->hy+1));
}

boolean
somexy(struct mkroom *croom, coord *c)
{
    int try_cnt = 0;
    int i;

    if (croom->irregular) {
        i = (croom - rooms) + ROOMOFFSET;

        while(try_cnt++ < 100) {
            c->x = somex(croom);
            c->y = somey(croom);
            if (!levl[c->x][c->y].edge &&
                (int) levl[c->x][c->y].roomno == i)
                return TRUE;
        }
        /* try harder; exhaustively search until one is found */
        for(c->x = croom->lx; c->x <= croom->hx; c->x++)
            for(c->y = croom->ly; c->y <= croom->hy; c->y++)
                if (!levl[c->x][c->y].edge &&
                    (int) levl[c->x][c->y].roomno == i)
                    return TRUE;
        return FALSE;
    }

    if (!croom->nsubrooms) {
        c->x = somex(croom);
        c->y = somey(croom);
        return TRUE;
    }

    /* Check that coords doesn't fall into a subroom or into a wall */

    while(try_cnt++ < 100) {
        c->x = somex(croom);
        c->y = somey(croom);
        if (IS_WALL(levl[c->x][c->y].typ))
            continue;
        for(i=0; i<croom->nsubrooms; i++)
            if(inside_room(croom->sbrooms[i], c->x, c->y))
                goto you_lose;
        break;
you_lose:;
    }
    if (try_cnt >= 100)
        return FALSE;
    return TRUE;
}

boolean
somexyspace(struct mkroom *croom, coord *pos, int flags)
{
    int tryct = 0;
    boolean isok;
    do {
        isok = TRUE;
        if (croom && !somexy(croom, pos)) isok = FALSE;
        if ((flags & 16)) mazexy(pos);
        if ((flags & 1) && (IS_POOL(levl[pos->x][pos->y].typ) || IS_FURNITURE(levl[pos->x][pos->y].typ))) isok = FALSE;
        if ((flags & 2) && ((levl[pos->x][pos->y].typ != CORR) || (levl[pos->x][pos->y].typ != ROOM))) isok = FALSE;
        if ((flags & 4) && (sobj_at(BOULDER, pos->x, pos->y))) isok = FALSE;
        if ((flags & 8) && bydoor(pos->x, pos->y)) isok = FALSE;
    } while ((!isok || !SPACE_POS(levl[pos->x][pos->y].typ) || occupied(pos->x, pos->y)) && (++tryct < 100));
    if ((tryct < 100) && isok) return TRUE;
    return FALSE;
}


/*
 * Search for a special room given its type (zoo, court, etc...)
 *  Special values :
 *      - ANY_SHOP
 *      - ANY_TYPE
 */

struct mkroom *
search_special(schar type)
{
    register struct mkroom *croom;

    for(croom = &rooms[0]; croom->hx >= 0; croom++)
        if((type == ANY_TYPE && croom->rtype != OROOM) ||
           (type == ANY_SHOP && croom->rtype >= SHOPBASE) ||
           croom->rtype == type)
            return croom;
    for(croom = &subrooms[0]; croom->hx >= 0; croom++)
        if((type == ANY_TYPE && croom->rtype != OROOM) ||
           (type == ANY_SHOP && croom->rtype >= SHOPBASE) ||
           croom->rtype == type)
            return croom;
    return (struct mkroom *) 0;
}

struct permonst *
courtmon(void)
{
    int i = rn2(60) + rn2(3*level_difficulty());
    if (i > 100) return(mkclass(S_DRAGON, 0));
    else if (i > 95) return(mkclass(S_GIANT, 0));
    else if (i > 85) return(mkclass(S_TROLL, 0));
    else if (i > 75) return(mkclass(S_CENTAUR, 0));
    else if (i > 60) return(mkclass(S_ORC, 0));
    else if (i > 45) return(&mons[PM_BUGBEAR]);
    else if (i > 30) return(&mons[PM_HOBGOBLIN]);
    else if (i > 15) return(mkclass(S_GNOME, 0));
    else return(mkclass(S_KOBOLD, 0));
}

#define NSTYPES (PM_CAPTAIN - PM_SOLDIER + 1)

static struct {
    unsigned pm;
    unsigned prob;
} squadprob[NSTYPES] = {
    {PM_SOLDIER, 80}, {PM_SERGEANT, 15}, {PM_LIEUTENANT, 4}, {PM_CAPTAIN, 1}
};

static struct permonst *
squadmon(void) /* return soldier types. */
{
    int sel_prob, i, cpro, mndx;

    sel_prob = rnd(80+level_difficulty());

    cpro = 0;
    for (i = 0; i < NSTYPES; i++) {
        cpro += squadprob[i].prob;
        if (cpro > sel_prob) {
            mndx = squadprob[i].pm;
            goto gotone;
        }
    }
    mndx = squadprob[rn2(NSTYPES)].pm;
gotone:
    if (!(mvitals[mndx].mvflags & G_GONE)) return(&mons[mndx]);
    else return((struct permonst *) 0);
}

/*
 * save_room : A recursive function that saves a room and its subrooms
 * (if any).
 */

static void
save_room(int fd, struct mkroom *r)
{
    short i;
    /*
     * Well, I really should write only useful information instead
     * of writing the whole structure. That is I should not write
     * the subrooms pointers, but who cares ?
     */
    bwrite(fd, (genericptr_t) r, sizeof(struct mkroom));
    for(i=0; i<r->nsubrooms; i++)
        save_room(fd, r->sbrooms[i]);
}

/*
 * save_rooms : Save all the rooms on disk!
 */

void
save_rooms(int fd)
{
    short i;

    /* First, write the number of rooms */
    bwrite(fd, (genericptr_t) &nroom, sizeof(nroom));
    for(i=0; i<nroom; i++)
        save_room(fd, &rooms[i]);
}

static void
rest_room(int fd, struct mkroom *r)
{
    short i;

    mread(fd, (genericptr_t) r, sizeof(struct mkroom));
    for(i=0; i<r->nsubrooms; i++) {
        r->sbrooms[i] = &subrooms[nsubroom];
        rest_room(fd, &subrooms[nsubroom]);
        subrooms[nsubroom++].resident = (struct monst *)0;
    }
}

/*
 * rest_rooms : That's for restoring rooms. Read the rooms structure from
 * the disk.
 */

void
rest_rooms(int fd)
{
    short i;

    mread(fd, (genericptr_t) &nroom, sizeof(nroom));
    nsubroom = 0;
    for(i = 0; i<nroom; i++) {
        rest_room(fd, &rooms[i]);
        rooms[i].resident = (struct monst *)0;
    }
    rooms[nroom].hx = -1;       /* restore ending flags */
    subrooms[nsubroom].hx = -1;
}

/* convert a display symbol for terrain into topology type;
   used for remembered terrain when mimics pose as furniture */
int
cmap_to_type(int sym)
{
    int typ = STONE; /* catchall */

    switch (sym) {
    case S_stone:
        typ = STONE;
        break;
    case S_vwall:
        typ = VWALL;
        break;
    case S_hwall:
        typ = HWALL;
        break;
    case S_tlcorn:
        typ = TLCORNER;
        break;
    case S_trcorn:
        typ = TRCORNER;
        break;
    case S_blcorn:
        typ = BLCORNER;
        break;
    case S_brcorn:
        typ = BRCORNER;
        break;
    case S_crwall:
        typ = CROSSWALL;
        break;
    case S_tuwall:
        typ = TUWALL;
        break;
    case S_tdwall:
        typ = TDWALL;
        break;
    case S_tlwall:
        typ = TLWALL;
        break;
    case S_trwall:
        typ = TRWALL;
        break;
    case S_ndoor:  /* no door (empty doorway) */
    case S_vodoor: /* open door in vertical wall */
    case S_hodoor: /* open door in horizontal wall */
    case S_vcdoor: /* closed door in vertical wall */
    case S_hcdoor:
        typ = DOOR;
        break;
    case S_bars:
        typ = IRONBARS;
        break;
    case S_tree:
        typ = TREE;
        break;
    case S_deadtree:
        typ = DEADTREE;
        break;
    case S_room:
        typ = ROOM;
        break;
    case S_corr:
    case S_litcorr:
        typ = CORR;
        break;
    case S_upstair:
    case S_dnstair:
        typ = STAIRS;
        break;
    case S_upladder:
    case S_dnladder:
        typ = LADDER;
        break;
    case S_altar:
        typ = ALTAR;
        break;
    case S_grave:
        typ = GRAVE;
        break;
    case S_throne:
        typ = THRONE;
        break;
    case S_sink:
        typ = SINK;
        break;
    case S_fountain:
        typ = FOUNTAIN;
        break;
    case S_pool:
        typ = POOL;
        break;
    case S_ice:
        typ = ICE;
        break;
    case S_lava:
        typ = LAVAPOOL;
        break;
    case S_bog:
        typ = BOG;
        break;
    case S_vodbridge: /* open drawbridge spanning north/south */
    case S_hodbridge:
        typ = DRAWBRIDGE_DOWN;
        break;        /* east/west */
    case S_vcdbridge: /* closed drawbridge in vertical wall */
    case S_hcdbridge:
        typ = DBWALL;
        break;
    case S_air:
        typ = AIR;
        break;
    case S_cloud:
        typ = CLOUD;
        break;
    case S_water:
        typ = WATER;
        break;
    case S_icewall:
    case S_crystalicewall:
        typ = ICEWALL;
        break;
    default:
        break; /* not a cmap symbol? */
    }
    return typ;
}

boolean
is_rainbow_shop(struct mkroom *sroom)
{
    if (!is_june() || sroom->irregular) {
        return FALSE;
    }

    int offset = 1;
    if (doors[sroom->fdoor].y < sroom->ly || doors[sroom->fdoor].y > sroom->hy) {
        offset = 0;
    }
    /* We need 6 horizontal aisles */
    if (sroom->hy - sroom->ly + offset == 6) {
        return TRUE;
    }

    return FALSE;
}

/*mkroom.c*/
