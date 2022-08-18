/*  SCCS Id: @(#)mklev.c    3.4 2001/11/29  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
/* #define DEBUG */ /* uncomment to enable code debugging */

#ifdef DEBUG
# ifdef WIZARD
#define debugpline  if (wizard) pline
# else
#define debugpline  pline
# endif
#endif

/* for UNIX, Rand #def'd to (long)lrand48() or (long)random() */
/* croom->lx etc are schar (width <= int), so % arith ensures that */
/* conversion of result to int is reasonable */


static void mkfount(int, struct mkroom *);
#ifdef SINKS
static void mksink(struct mkroom *);
#endif
static void mkaltar(struct mkroom *);
static void mkgrave(struct mkroom *);
static void makevtele(void);
static void clear_level_structures(void);
static void makelevel(void);
static struct mkroom *find_branch_room(coord *);
static struct mkroom *pos_to_room(xchar, xchar);
static boolean place_niche(struct mkroom *, int*, int*, int*);
static void makeniche(int);
static void make_niches(void);

static int  CFDECLSPEC do_comp(const genericptr, const genericptr);

static void dosdoor(xchar, xchar, struct mkroom *, int);
static void join(int, int, boolean);
static void do_room_or_subroom(struct mkroom *, int, int, int, int,
                               boolean, schar, boolean, boolean);
static void makerooms(void);
static void finddpos(coord *, xchar, xchar, xchar, xchar);
static void mkinvpos(xchar, xchar, int);
static void mk_knox_portal(xchar, xchar);

static void place_random_engravings(void);

#define create_vault()  create_room(-1, -1, 2, 2, -1, -1, VAULT, TRUE)
#define init_vault()    vault_x = -1
#define do_vault()  (vault_x != -1)
static xchar vault_x, vault_y;
boolean goldseen;
static boolean made_branch; /* used only during level creation */

/* Args must be (const genericptr) so that qsort will always be happy. */

static int CFDECLSPEC
do_comp(const void *vx, const void *vy)
{
#ifdef LINT
/* lint complains about possible pointer alignment problems, but we know
   that vx and vy are always properly aligned. Hence, the following
   bogus definition:
 */
    return (vx == vy) ? 0 : -1;
#else
    register const struct mkroom *x, *y;

    x = (const struct mkroom *)vx;
    y = (const struct mkroom *)vy;
    if(x->lx < y->lx) return(-1);
    return(x->lx > y->lx);
#endif /* LINT */
}

static void
finddpos(coord *cc, xchar xl, xchar yl, xchar xh, xchar yh)
{
    register xchar x, y;

    x = (xl == xh) ? xl : (xl + rn2(xh-xl+1));
    y = (yl == yh) ? yl : (yl + rn2(yh-yl+1));
    if(okdoor(x, y))
        goto gotit;

    for(x = xl; x <= xh; x++) for(y = yl; y <= yh; y++)
            if(okdoor(x, y))
                goto gotit;

    for(x = xl; x <= xh; x++) for(y = yl; y <= yh; y++)
            if(IS_DOOR(levl[x][y].typ) || levl[x][y].typ == SDOOR)
                goto gotit;
    /* cannot find something reasonable -- strange */
    x = xl;
    y = yh;
gotit:
    cc->x = x;
    cc->y = y;
    return;
}

void
sort_rooms(void)
{
#if defined(SYSV) || defined(DGUX)
    qsort((genericptr_t) rooms, (unsigned)nroom, sizeof(struct mkroom), do_comp);
#else
    qsort((genericptr_t) rooms, nroom, sizeof(struct mkroom), do_comp);
#endif
}

static void
do_room_or_subroom(register struct mkroom *croom, int lowx, int lowy, register int hix, register int hiy, boolean lit, schar rtype, boolean special, boolean is_room)
{
    register int x, y;
    struct rm *lev;

    /* locations might bump level edges in wall-less rooms */
    /* add/subtract 1 to allow for edge locations */
    if(!lowx) lowx++;
    if(!lowy) lowy++;
    if(hix >= COLNO-1) hix = COLNO-2;
    if(hiy >= ROWNO-1) hiy = ROWNO-2;

    if(lit) {
        for(x = lowx-1; x <= hix+1; x++) {
            lev = &levl[x][max(lowy-1, 0)];
            for(y = lowy-1; y <= hiy+1; y++)
                lev++->lit = 1;
        }
        croom->rlit = 1;
    } else
        croom->rlit = 0;

    croom->lx = lowx;
    croom->hx = hix;
    croom->ly = lowy;
    croom->hy = hiy;
    croom->rtype = rtype;
    croom->doorct = 0;
    /* if we're not making a vault, doorindex will still be 0
     * if we are, we'll have problems adding niches to the previous room
     * unless fdoor is at least doorindex
     */
    croom->fdoor = doorindex;
    croom->irregular = FALSE;

    croom->nsubrooms = 0;
    croom->sbrooms[0] = (struct mkroom *) 0;
    if (!special) {
        for(x = lowx-1; x <= hix+1; x++)
            for(y = lowy-1; y <= hiy+1; y += (hiy-lowy+2)) {
                levl[x][y].typ = HWALL;
                levl[x][y].horizontal = 1; /* For open/secret doors. */
            }
        for(x = lowx-1; x <= hix+1; x += (hix-lowx+2))
            for(y = lowy; y <= hiy; y++) {
                levl[x][y].typ = VWALL;
                levl[x][y].horizontal = 0; /* For open/secret doors. */
            }
        for(x = lowx; x <= hix; x++) {
            lev = &levl[x][lowy];
            for(y = lowy; y <= hiy; y++)
                lev++->typ = ROOM;
        }
        if (is_room) {
            levl[lowx-1][lowy-1].typ = TLCORNER;
            levl[hix+1][lowy-1].typ = TRCORNER;
            levl[lowx-1][hiy+1].typ = BLCORNER;
            levl[hix+1][hiy+1].typ = BRCORNER;
            wallification(lowx-1, lowy-1, hix+1, hiy+1);
        } else {    /* a subroom */
            wallification(lowx, lowy, hix, hiy); /* this is bugs */
        }
    }
}


void
add_room(register int lowx, register int lowy, register int hix, register int hiy, boolean lit, schar rtype, boolean special)
{
    register struct mkroom *croom;

    croom = &rooms[nroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit,
                       rtype, special, (boolean) TRUE);
    croom++;
    croom->hx = -1;
    nroom++;
}

void
add_subroom(struct mkroom *proom, register int lowx, register int lowy, register int hix, register int hiy, boolean lit, schar rtype, boolean special)
{
    register struct mkroom *croom;

    croom = &subrooms[nsubroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit,
                       rtype, special, (boolean) FALSE);
    proom->sbrooms[proom->nsubrooms++] = croom;
    croom++;
    croom->hx = -1;
    nsubroom++;
}

struct _rndvault {
    char *fname;
    long freq;
    struct _rndvault *next;
};
struct _rndvault_gen {
    int n_vaults;
    long total_freq;
    struct _rndvault *vaults;
};
struct _rndvault_gen *rndvault_gen = NULL;

void
rndvault_gen_load(void)
{
    if (!rndvault_gen) {
        dlb *fd;
        char line[BUFSZ];
        char fnamebuf[64];
        long frq;
        fd = dlb_fopen_area(NH_DATAAREA, "vaults.dat", "r");
        if (!fd) return;

        rndvault_gen = (struct _rndvault_gen *) alloc(sizeof(struct _rndvault_gen));
        if (!rndvault_gen) goto bailout;

        rndvault_gen->n_vaults = 0;
        rndvault_gen->total_freq = 0;
        rndvault_gen->vaults = NULL;

        while (dlb_fgets(line, sizeof line, fd)) {
            struct _rndvault *vlt = (struct _rndvault *) alloc(sizeof(struct _rndvault));
            char *tmpch = fnamebuf;
            fnamebuf[0] = '\0';
            if (sscanf(line, "%ld %63s", &frq, tmpch) == 2) {
                if (frq < 1) frq = 1;
                vlt->freq = frq;
                vlt->fname = strdup(fnamebuf);
                vlt->next = rndvault_gen->vaults;
                rndvault_gen->vaults = vlt;
                rndvault_gen->n_vaults++;
                rndvault_gen->total_freq += frq;
            }
        }

bailout:
        (void)dlb_fclose(fd);
    }
}

char *
rndvault_getname(void)
{
    if (!rndvault_gen) rndvault_gen_load();
    if (rndvault_gen) {
        long frq = rn2(rndvault_gen->total_freq);
        struct _rndvault *tmp = rndvault_gen->vaults;
        while (tmp && ((frq -= tmp->freq) > 0)) tmp = tmp->next;
        if (tmp && tmp->fname)
            return tmp->fname;
    }
    return NULL;
}

static void
makerooms(void)
{
    boolean tried_vault = FALSE;

    /* make rooms until satisfied */
    /* rnd_rect() will returns 0 if no more rects are available... */
    while(nroom < MAXNROFROOMS && rnd_rect()) {
        if(nroom >= (MAXNROFROOMS/6) && rn2(2) && !tried_vault) {
            tried_vault = TRUE;
            if (create_vault()) {
                vault_x = rooms[nroom].lx;
                vault_y = rooms[nroom].ly;
                rooms[nroom].hx = -1;
            }
        } else {
            char protofile[64];
            char *fnam = rndvault_getname();
            if (fnam) {
                Sprintf(protofile, "%s", fnam);
                Strcat(protofile, LEV_EXT);
                in_mk_rndvault = TRUE;
                rndvault_failed = FALSE;
                (void) load_special(protofile);
                in_mk_rndvault = FALSE;
                if (rndvault_failed) return;
            } else {
                if (!create_room(-1, -1, -1, -1, -1, -1, OROOM, -1))
                    return;
            }
        }
    }
    return;
}

static void
join(register int a, register int b, boolean nxcor)
{
    coord cc, tt, org, dest;
    register xchar tx, ty, xx, yy;
    register struct mkroom *croom, *troom;
    register int dx, dy;

    croom = &rooms[a];
    troom = &rooms[b];

    /* find positions cc and tt for doors in croom and troom
       and direction for a corridor between them */

    if (!croom->needjoining || !troom->needjoining) return;

    if(troom->hx < 0 || croom->hx < 0 || doorindex >= DOORMAX) return;
    if(troom->lx > croom->hx) {
        dx = 1;
        dy = 0;
        xx = croom->hx+1;
        tx = troom->lx-1;
        finddpos(&cc, xx, croom->ly, xx, croom->hy);
        finddpos(&tt, tx, troom->ly, tx, troom->hy);
    } else if(troom->hy < croom->ly) {
        dy = -1;
        dx = 0;
        yy = croom->ly-1;
        finddpos(&cc, croom->lx, yy, croom->hx, yy);
        ty = troom->hy+1;
        finddpos(&tt, troom->lx, ty, troom->hx, ty);
    } else if(troom->hx < croom->lx) {
        dx = -1;
        dy = 0;
        xx = croom->lx-1;
        tx = troom->hx+1;
        finddpos(&cc, xx, croom->ly, xx, croom->hy);
        finddpos(&tt, tx, troom->ly, tx, troom->hy);
    } else {
        dy = 1;
        dx = 0;
        yy = croom->hy+1;
        ty = troom->ly-1;
        finddpos(&cc, croom->lx, yy, croom->hx, yy);
        finddpos(&tt, troom->lx, ty, troom->hx, ty);
    }
    xx = cc.x;
    yy = cc.y;
    tx = tt.x - dx;
    ty = tt.y - dy;
    if(nxcor && levl[xx+dx][yy+dy].typ)
        return;
    if (okdoor(xx, yy) || !nxcor)
        dodoor(xx, yy, croom);

    org.x  = xx+dx; org.y  = yy+dy;
    dest.x = tx; dest.y = ty;

    if (!dig_corridor(&org, &dest, nxcor,
                      level.flags.arboreal ? ROOM : CORR, STONE))
        return;

    /* we succeeded in digging the corridor */
    if (okdoor(tt.x, tt.y) || !nxcor)
        dodoor(tt.x, tt.y, troom);

    if(smeq[a] < smeq[b])
        smeq[b] = smeq[a];
    else
        smeq[a] = smeq[b];
}

void
makecorridors(int style)
{
    int a, b, i;
    boolean any = TRUE;

    if (style == -1) style = rn2(4);

    switch (style) {

    default: /* vanilla style */
        for(a = 0; a < nroom-1; a++) {
            join(a, a+1, FALSE);
            if(!rn2(50)) break; /* allow some randomness */
        }
        for(a = 0; a < nroom-2; a++)
            if(smeq[a] != smeq[a+2])
                join(a, a+2, FALSE);
        for(a = 0; any && a < nroom; a++) {
            any = FALSE;
            for(b = 0; b < nroom; b++)
                if(smeq[a] != smeq[b]) {
                    join(a, b, FALSE);
                    any = TRUE;
                }
        }
        if(nroom > 2)
            for(i = rn2(nroom) + 4; i; i--) {
                a = rn2(nroom);
                b = rn2(nroom-2);
                if(b >= a) b += 2;
                join(a, b, TRUE);
            }
        break;
    case 1: /* at least one corridor leaves from each room and goes to random room */
        if (nroom > 1) {
            int cnt = 0;
            for (a = 0; a < nroom; a++) {
                do {
                    b = rn2(nroom-1);
                } while (((a == b) || (rooms[b].doorct) || !rooms[b].needjoining) && cnt++ < 100);
                if (cnt >= 100) {
                    for (b = 0; b < nroom-1; b++)
                        if (!rooms[b].doorct && (a != b)) break;
                }
                if (a == b) b++;
                join(a, b, FALSE);
            }
        }
        break;
    case 2: /* circular path: room1 -> room2 -> room3 -> ... -> room1  */
        if (nroom > 1) {
            for (a = 0; a < nroom; a++) {
                if (rooms[a].needjoining) {
                    int cnt = 0;
                    b = a;
                    do {
                        b++;
                        b = b % nroom;
                    } while (!rooms[b].needjoining && (++cnt < nroom));
                    join(a, b, FALSE);
                }
            }
        }
        break;
    case 3: /* all roads lead to rome. or to the first room. */
        if (nroom > 1) {
            b = 0;
            while (!rooms[b].needjoining && (b < nroom)) b++;
            if (b < nroom)
                for (a = 0; a < nroom; a++) {
                    if ((a != b))
                        join(a, b, FALSE);
                }
        }
        break;
    }
}

void
add_door(register int x, register int y, register struct mkroom *aroom)
{
    register struct mkroom *broom;
    register int tmp;

    aroom->doorct++;
    broom = aroom+1;
    if(broom->hx < 0)
        tmp = doorindex;
    else
        for(tmp = doorindex; tmp > broom->fdoor; tmp--)
            doors[tmp] = doors[tmp-1];
    doorindex++;
    doors[tmp].x = x;
    doors[tmp].y = y;
    for( ; broom->hx >= 0; broom++) broom->fdoor++;
}

static void
dosdoor(register xchar x, register xchar y, register struct mkroom *aroom, register int type)
{
    boolean shdoor = ((*in_rooms(x, y, SHOPBASE)) ? TRUE : FALSE);

    if(!IS_WALL(levl[x][y].typ)) /* avoid SDOORs on already made doors */
        type = DOOR;
    levl[x][y].typ = type;
    if(type == DOOR) {
        if(!rn2(3)) {      /* is it a locked door, closed, or a doorway? */
            if(!rn2(5))
                levl[x][y].doormask = D_ISOPEN;
            else if(!rn2(6))
                levl[x][y].doormask = D_LOCKED;
            else
                levl[x][y].doormask = D_CLOSED;

            if (levl[x][y].doormask != D_ISOPEN && !shdoor &&
                level_difficulty() >= 5 && !rn2(25))
                levl[x][y].doormask |= D_TRAPPED;
        } else
#ifdef STUPID
        if (shdoor)
            levl[x][y].doormask = D_ISOPEN;
        else
            levl[x][y].doormask = D_NODOOR;
#else
            levl[x][y].doormask = (shdoor ? D_ISOPEN : D_NODOOR);
#endif
        if(levl[x][y].doormask & D_TRAPPED) {
            struct monst *mtmp;

            if (level_difficulty() >= 9 && !rn2(5) &&
                !((mvitals[PM_SMALL_MIMIC].mvflags & G_GONE) &&
                  (mvitals[PM_LARGE_MIMIC].mvflags & G_GONE) &&
                  (mvitals[PM_GIANT_MIMIC].mvflags & G_GONE))) {
                /* make a mimic instead */
                levl[x][y].doormask = D_NODOOR;
                mtmp = makemon(mkclass(S_MIMIC, 0), x, y, NO_MM_FLAGS);
                if (mtmp)
                    set_mimic_sym(mtmp);
            }
        }
        /* newsym(x,y); */
    } else { /* SDOOR */
        if(shdoor || !rn2(5)) levl[x][y].doormask = D_LOCKED;
        else levl[x][y].doormask = D_CLOSED;

        if(!shdoor && level_difficulty() >= 4 && !rn2(20))
            levl[x][y].doormask |= D_TRAPPED;
    }

    add_door(x, y, aroom);
}

static boolean
place_niche(register struct mkroom *aroom, int *dy, int *xx, int *yy)
{
    coord dd;

    if(rn2(2)) {
        *dy = 1;
        finddpos(&dd, aroom->lx, aroom->hy+1, aroom->hx, aroom->hy+1);
    } else {
        *dy = -1;
        finddpos(&dd, aroom->lx, aroom->ly-1, aroom->hx, aroom->ly-1);
    }
    *xx = dd.x;
    *yy = dd.y;
    return((boolean)((isok(*xx, *yy+*dy) && levl[*xx][*yy+*dy].typ == STONE)
                     && (isok(*xx, *yy-*dy) && !IS_POOL(levl[*xx][*yy-*dy].typ)
                         && !IS_FURNITURE(levl[*xx][*yy-*dy].typ))));
}

/* there should be one of these per trap, in the same order as trap.h */
static NEARDATA const char *trap_engravings[TRAPNUM] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0,
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0,
    (char *)0, (char *)0, (char *)0, (char *)0,
    /* 14..16: trap door, teleport, level-teleport */
    "Vlad was here", "ad aerarium", "ad aerarium",
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0,
    (char *)0, (char *)0
};

/* Quotes from or about the Illuminati */
static const char * const illuminati_engravings[] = {
    "Ewige Blumenkraft!", /* the Illuminatus' motto */
    "Ewige Blumenkraft Und Ewige Schlangenkraft", /* the complete Illuminatus' motto */
    "fnord", /* you can't see it */
    "FUCKUP calculates your odds of ascending as negligibly small",
    "Hail Eris!",
    "Jeder Gedanke ist verschenkt, wenn Adam Weishaupt fuer dich denkt", /* Welle:Erdball */
    "Novus Ordo Seclorum",
    "Think for yourself, schmuck",
    "Adam Weishaupt wasn't here!",
    "Nothing is True, Everything is Permitted", /* Hassan-i Sabbah */
};
/* Quotes from or about the Hitchhikers Guide to the Galaxy */
static const char * const hhgtg_engravings[] = {
    "Beeblebrox for President!",
    "Don't Panic",
    "Got your towel?",
    "Time is an illusion. Lunchtime doubly so.",
    "Oh no, not again.",
    "Resistance is useless!",
    "Slartibartfast was here!",
    "So long, and thanks for all the fish",
    "Visit Milliways, the Restaurant at the End of the Universe!",
    "We apologize for the inconvenience",
};

static void
makeniche(int trap_type)
{
    register struct mkroom *aroom;
    register struct rm *rm;
    register int vct = 8;
    int dy, xx, yy;
    register struct trap *ttmp;

    if(doorindex < DOORMAX)
        while(vct--) {
            aroom = &rooms[rn2(nroom)];
            if(aroom->rtype != OROOM) continue; /* not an ordinary room */
            if(aroom->doorct == 1 && rn2(5)) continue;
            if(!aroom->needjoining) continue;
            if(!place_niche(aroom, &dy, &xx, &yy)) continue;

            rm = &levl[xx][yy+dy];
            if(trap_type || !rn2(4)) {

                rm->typ = SCORR;
                if(trap_type) {
                    if((trap_type == HOLE || trap_type == TRAPDOOR)
                       && !Can_fall_thru(&u.uz))
                        trap_type = ROCKTRAP;
                    ttmp = maketrap(xx, yy+dy, trap_type);
                    if (ttmp) {
                        if (trap_type != ROCKTRAP) ttmp->once = 1;
                        if (trap_engravings[trap_type]) {
                            make_engr_at(xx, yy-dy,
                                         trap_engravings[trap_type], 0L, DUST);
                            wipe_engr_at(xx, yy-dy, 5); /* age it a little */
                        }
                    }
                }
                dosdoor(xx, yy, aroom, SDOOR);
            } else {
                rm->typ = CORR;
                if(rn2(7))
                    dosdoor(xx, yy, aroom, rn2(5) ? SDOOR : DOOR);
                else {
                    /* inaccessible niches occasionally have iron bars */
                    if (!rn2(8) && IS_WALL(levl[xx][yy].typ)) {
                        levl[xx][yy].typ = IRONBARS;
                        if (rn2(3))
                            (void) mkcorpstat(CORPSE,
                                              (struct monst *)0,
                                              mkclass(S_HUMAN, 0),
                                              xx, yy+dy, TRUE);
                    }
                    if (!level.flags.noteleport)
                        (void) mksobj_at(SCR_TELEPORTATION,
                                         xx, yy+dy, TRUE, FALSE);
                    if (!rn2(3)) (void) mkobj_at(0, xx, yy+dy, TRUE);
                }
            }
            return;
        }
}

/* replaces horiz/vert walls with iron bars,
   iff there's no door next to the place, and there's space
   on the other side of the wall */
void
make_ironbarwalls(int chance)
{
    xchar x, y;

    if (chance < 1) return;

    for (x = 1; x < COLNO-1; x++) {
        for(y = 1; y < ROWNO-1; y++) {
            schar typ = levl[x][y].typ;
            if (typ == HWALL) {
                if ((IS_WALL(levl[x-1][y].typ) || levl[x-1][y].typ == IRONBARS) &&
                    (IS_WALL(levl[x+1][y].typ) || levl[x+1][y].typ == IRONBARS) &&
                    SPACE_POS(levl[x][y-1].typ) && SPACE_POS(levl[x][y+1].typ) &&
                    rn2(100) < chance)
                    levl[x][y].typ = IRONBARS;
            } else if (typ == VWALL) {
                if ((IS_WALL(levl[x][y-1].typ) || levl[x][y-1].typ == IRONBARS) &&
                    (IS_WALL(levl[x][y+1].typ) || levl[x][y+1].typ == IRONBARS) &&
                    SPACE_POS(levl[x-1][y].typ) && SPACE_POS(levl[x+1][y].typ) &&
                    rn2(100) < chance)
                    levl[x][y].typ = IRONBARS;
            }
        }
    }
}


static void
make_niches(void)
{
    register int ct = rnd((nroom>>1) + 1), dep = depth(&u.uz);

    boolean ltptr = (!level.flags.noteleport && dep > 15),
            vamp = (dep > 5 && dep < 25);

    while(ct--) {
        if (ltptr && !rn2(6)) {
            ltptr = FALSE;
            makeniche(LEVEL_TELEP);
        } else if (vamp && !rn2(6)) {
            vamp = FALSE;
            makeniche(TRAPDOOR);
        } else makeniche(NO_TRAP);
    }
}

static void
makevtele(void)
{
    makeniche(TELEP_TRAP);
}

/* clear out various globals that keep information on the current level.
 * some of this is only necessary for some types of levels (maze, normal,
 * special) but it's easier to put it all in one place than make sure
 * each type initializes what it needs to separately.
 */
static void
clear_level_structures(void)
{
    static struct rm zerorm = { cmap_to_glyph(S_stone), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int x, y;
    struct rm *lev;

    for(x=0; x<COLNO; x++) {
        lev = &levl[x][0];
        for(y=0; y<ROWNO; y++) {
            *lev++ = zerorm;
#ifdef MICROPORT_BUG
            level.objects[x][y] = (struct obj *)0;
            level.monsters[x][y] = (struct monst *)0;
#endif
        }
    }
#ifndef MICROPORT_BUG
    (void) memset((genericptr_t)level.objects, 0, sizeof(level.objects));
    (void) memset((genericptr_t)level.monsters, 0, sizeof(level.monsters));
#endif
    level.objlist = (struct obj *)0;
    level.buriedobjlist = (struct obj *)0;
    level.monlist = (struct monst *)0;
    level.damagelist = (struct damage *)0;
    level.mon_gen = (struct mon_gen_override *)0;
    level.sounds = NULL;

    level.flags.nfountains = 0;
    level.flags.nsinks = 0;
    level.flags.has_shop = 0;
    level.flags.has_vault = 0;
    level.flags.has_zoo = 0;
    level.flags.has_court = 0;
    level.flags.has_morgue = level.flags.graveyard = 0;
    level.flags.has_beehive = 0;
    level.flags.has_barracks = 0;
    level.flags.has_temple = 0;
    level.flags.has_lemurepit = 0;
    level.flags.has_swamp = 0;
    level.flags.has_garden = 0;
    level.flags.noteleport = 0;
    level.flags.hardfloor = 0;
    level.flags.nommap = 0;
    level.flags.hero_memory = 1;
    level.flags.shortsighted = 0;
    level.flags.arboreal = 0;
    level.flags.is_maze_lev = 0;
    level.flags.is_cavernous_lev = 0;
    level.flags.stormy = 0;
    level.flags.sky = 0;

    nroom = 0;
    rooms[0].hx = -1;
    nsubroom = 0;
    subrooms[0].hx = -1;
    doorindex = 0;
    init_rect();
    init_vault();
    xdnstair = ydnstair = xupstair = yupstair = 0;
    sstairs.sx = sstairs.sy = 0;
    xdnladder = ydnladder = xupladder = yupladder = 0;
    made_branch = FALSE;
    clear_regions();
}

/** Create a vault with the magic portal to Fort Ludios. */
void
mk_knox_vault(int x, int y, int w, int h)
{
    add_room(x, y, x+w, y+h, TRUE, VAULT, FALSE);
    level.flags.has_vault = 1;
    fill_room(&rooms[nroom-1], FALSE);
    mk_knox_portal(x+w, y+h);
}

static void
makelevel(void)
{
    register struct mkroom *croom, *troom;
    register int tryct;
    register int i;
    struct monst *tmonst;   /* always put a web with a spider */
    branch *branchp;
    int room_threshold, boxtype;
    coord pos;

    if(wiz1_level.dlevel == 0) init_dungeons();
    oinit();    /* assign level dependent obj probabilities */
    clear_level_structures();

    {
        register s_level *slev = Is_special(&u.uz);

        /* check for special levels */
#ifdef REINCARNATION
        if (slev && !Is_rogue_level(&u.uz))
#else
        if (slev)
#endif
        {
            makemaz(slev->proto);
            return;
        } else if (dungeons[u.uz.dnum].proto[0]) {
            makemaz("");
            return;
        } else if (In_mines(&u.uz)) {
            makemaz("minefill");
            return;
        } else if (In_sheol(&u.uz)) {
            makemaz("sheolfil");
            return;
        } else if (In_quest(&u.uz)) {
            char fillname[9];
            s_level *loc_lev;

            Sprintf(fillname, "%s-loca", urole.filecode);
            loc_lev = find_level(fillname);

            Sprintf(fillname, "%s-fil", urole.filecode);
            Strcat(fillname,
                   (u.uz.dlevel < loc_lev->dlevel.dlevel) ? "a" : "b");
            makemaz(fillname);
            return;
        } else if (u.uz.dnum == medusa_level.dnum &&
                depth(&u.uz) > depth(&medusa_level)) {
            makemaz("medufill");
            return;
        } else if (In_hell(&u.uz)) {
            /* The vibrating square code is hardcoded into mkmaze --
             * rather than fiddle around trying to port it to a 'generalist'
             * sort of level, just go ahead and let the VS level be a maze */
            if (!Invocation_lev(&u.uz)) {
                makemaz("hellfill");
            } else {
                makemaz("");
            }
            return;
        }
    }

    /* otherwise, fall through - it's a "regular" level. */

#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) {
        makeroguerooms();
        makerogueghost();
    } else
#endif
    makerooms();
    /*sort_rooms();*/ /* messes up roomno order. */

    /* construct stairs (up and down in different rooms if possible) */
    tryct = 0;
    do {
        croom = &rooms[rn2(nroom)];
    } while (!croom->needjoining && ++tryct < 100);
    if (!Is_botlevel(&u.uz)) {
        if (!somexyspace(croom, &pos, 0)) {
            pos.x = somex(croom);
            pos.y = somey(croom);
        }
        mkstairs(pos.x, pos.y, 0, croom);    /* down */
    }
    if (nroom > 1) {
        troom = croom;
        tryct = 0;
        do {
            croom = &rooms[rn2(nroom-1)];
        } while ((!croom->needjoining || (croom == troom)) && ++tryct < 100);
    }

    if (u.uz.dlevel != 1) {
        if (!somexyspace(croom, &pos, 0)) {
            if (!somexy(croom, &pos)) {
                pos.x = somex(croom);
                pos.y = somey(croom);
            }
        }
        mkstairs(pos.x, pos.y, 1, croom);   /* up */
    }

    branchp = Is_branchlev(&u.uz);  /* possible dungeon branch */
    room_threshold = branchp ? 4 : 3; /* minimum number of rooms needed
                                         to allow a random special room */
#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) goto skip0;
#endif
    makecorridors(0);
    make_niches();

    if (!rn2(5)) make_ironbarwalls(rn2(20) ? rn2(20) : rn2(50));

    /* make a secret treasure vault, not connected to the rest */
    if(do_vault()) {
        xchar w, h;
#ifdef DEBUG
        debugpline("trying to make a vault...");
#endif
        w = 1;
        h = 1;
        if (check_room(&vault_x, &w, &vault_y, &h, TRUE)) {
fill_vault:
            mk_knox_vault(vault_x, vault_y, w, h);
            ++room_threshold;
            if(!level.flags.noteleport && !rn2(3)) makevtele();
        } else if(rnd_rect() && create_vault()) {
            vault_x = rooms[nroom].lx;
            vault_y = rooms[nroom].ly;
            if (check_room(&vault_x, &w, &vault_y, &h, TRUE))
                goto fill_vault;
            else
                rooms[nroom].hx = -1;
        }
    }

    {
        register int u_depth = depth(&u.uz);

#ifdef WIZARD
        if(wizard && nh_getenv("SHOPTYPE")) mkroom(SHOPBASE); else
#endif
        if (u_depth > 1 &&
            u_depth < depth(&medusa_level) &&
            nroom >= room_threshold &&
            rn2(u_depth) < 3) mkroom(SHOPBASE);
        else if (u_depth > 4 && !rn2(6)) mkroom(COURT);
        else if (u_depth > 5 && !rn2(8) &&
                 !(mvitals[PM_LEPRECHAUN].mvflags & G_GONE)) mkroom(LEPREHALL);
        else if (u_depth > 6 && !rn2(7)) mkroom(ZOO);
        else if (u_depth > 7 && !rn2(6)) mkroom(GARDEN);
        else if (u_depth > 7 && !rn2(6) &&
                 !(mvitals[PM_RUST_MONSTER].mvflags & G_GONE)) mkroom(ARMORY);
        else if (u_depth > 8 && !rn2(5)) mkroom(TEMPLE);
        else if (u_depth > 9 && !rn2(5) &&
                 !(mvitals[PM_KILLER_BEE].mvflags & G_GONE)) mkroom(BEEHIVE);
        else if (u_depth > 11 && !rn2(6)) mkroom(MORGUE);
        else if (u_depth > 12 && !rn2(8)) mkroom(ANTHOLE);
        else if (u_depth > 14 && !rn2(4) &&
                 !(mvitals[PM_SOLDIER].mvflags & G_GONE)) mkroom(BARRACKS);
        else if (u_depth > 15 && !rn2(6)) mkroom(SWAMP);
        else if (u_depth > 16 && !rn2(8) &&
                 !(mvitals[PM_COCKATRICE].mvflags & G_GONE)) mkroom(COCKNEST);
    }

#ifdef REINCARNATION
skip0:
#endif
    /* Place multi-dungeon branch. */
    place_branch(branchp, 0, 0);

    /* for each room: put things inside */
    for(croom = rooms; croom->hx > 0; croom++) {
        if (croom->rtype != OROOM && croom->rtype != RNDVAULT) continue;
        if (!croom->needfill) continue;

        /* put a sleeping monster inside */
        /* Note: monster may be on the stairs. This cannot be
           avoided: maybe the player fell through a trap door
           while a monster was on the stairs. Conclusion:
           we have to check for monsters on the stairs anyway. */

        if(u.uhave.amulet || !rn2(3)) {
            if (somexyspace(croom, &pos, 0)) {
                tmonst = makemon((struct permonst *) 0, pos.x, pos.y, NO_MM_FLAGS);
                if (tmonst && tmonst->data == &mons[PM_GIANT_SPIDER] &&
                    !occupied(pos.x, pos.y))
                    (void) maketrap(pos.x, pos.y, WEB);
            }
        }
        /* put traps and mimics inside */
        goldseen = FALSE;
        i = 8 - (level_difficulty()/6);
        if (i <= 1) i = 2;
        while (!rn2(i))
            mktrap(0, 0, croom, (coord*)0);
        if (!goldseen && !rn2(3)) {
            if (somexyspace(croom, &pos, 0))
                (void) mkgold(0L, pos.x, pos.y);
        }
#ifdef REINCARNATION
        if(Is_rogue_level(&u.uz)) goto skip_nonrogue;
#endif
        if(!rn2(10)) mkfount(0, croom);
#ifdef SINKS
        if(!rn2(60)) mksink(croom);
#endif
        if(!rn2(60)) mkaltar(croom);
        i = 80 - (depth(&u.uz) * 2);
        if (i < 2) i = 2;
        if(!rn2(i)) mkgrave(croom);

        /* put statues inside */
        if(!rn2(20)) {
            if (somexyspace(croom, &pos, 0))
                (void) mkcorpstat(STATUE, (struct monst *)0,
                                  (struct permonst *)0,
                                  pos.x, pos.y, TRUE);
        }
        /* put box/chest/safe inside;
         *  40% chance for at least 1 box, regardless of number
         *  of rooms; about 5 - 7.5% for 2 boxes, least likely
         *  when few rooms; chance for 3 or more is neglible.
         *
         *  Safes only show up below level 15 since they're not unlockable.
         */
        if(!rn2(nroom * 5 / 2)) {
            i = rn2(5);
            if (!i && depth(&u.uz) > 15) {
                boxtype = IRON_SAFE;
            } else if (i > 2) {
                boxtype = CHEST;
            } else {
                boxtype = LARGE_BOX;
            }
            if (somexyspace(croom, &pos, 0))
                (void) mksobj_at(boxtype, pos.x, pos.y, TRUE, FALSE);
        }

        /* maybe make some graffiti */
        if(!rn2(27 + 3 * abs(depth(&u.uz)))) {
            char buf[BUFSZ];
            const char *mesg = random_engraving(buf);
            if (mesg) {
                if (somexyspace(croom, &pos, 1))
                    make_engr_at(pos.x, pos.y, mesg, 0L, MARK);
            }
        }

#ifdef REINCARNATION
skip_nonrogue:
#endif
        if(!rn2(3)) {
            if (somexyspace(croom, &pos, 0))
                (void) mkobj_at(0, pos.x, pos.y, TRUE);
            tryct = 0;
            while(!rn2(5)) {
                if(++tryct > 100) {
                    impossible("tryct overflow4");
                    break;
                }
                if (somexyspace(croom, &pos, 0))
                    (void) mkobj_at(0, pos.x, pos.y, TRUE);
            }
        }
    }
}

/*
 *  Place deposits of minerals (gold and misc gems) in the stone
 *  surrounding the rooms on the map.
 *  Also place kelp in water.
 *      mineralize(-1, -1, -1, -1, 0); => "default" behaviour
 */
void
mineralize(int kelp_pool, int kelp_moat, int goldprob, int gemprob, boolean skip_lvl_checks)
{
    s_level *sp;
    struct obj *otmp;
    int x, y, cnt;

    if (kelp_pool < 0) kelp_pool = 10;
    if (kelp_moat < 0) kelp_moat = 30;

    /* Place kelp, except on the plane of water */
    if (!skip_lvl_checks && In_endgame(&u.uz)) return;
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if ((kelp_pool && levl[x][y].typ == POOL && !rn2(kelp_pool)) ||
                (kelp_moat && levl[x][y].typ == MOAT && !rn2(kelp_moat)))
                (void) mksobj_at(KELP_FROND, x, y, TRUE, FALSE);

    /* determine if it is even allowed;
       almost all special levels are excluded */
    if (!skip_lvl_checks && (In_hell(&u.uz) || In_V_tower(&u.uz) ||
#ifdef REINCARNATION
                             Is_rogue_level(&u.uz) ||
#endif
                             level.flags.arboreal ||
                             ((sp = Is_special(&u.uz)) != 0 && !Is_oracle_level(&u.uz)
                              && (!In_mines(&u.uz) || sp->flags.town)
                             ))) return;

    /* basic level-related probabilities */
    if (goldprob < 0) goldprob = 20 + depth(&u.uz) / 3;
    if (gemprob < 0) gemprob = goldprob / 4;

    /* mines have ***MORE*** goodies - otherwise why mine? */
    if (!skip_lvl_checks) {
        if (In_mines(&u.uz)) {
            goldprob *= 2;
            gemprob *= 3;
        } else if (In_quest(&u.uz)) {
            goldprob /= 4;
            gemprob /= 6;
        }
    }

    /*
     * Seed rock areas with gold and/or gems.
     * We use fairly low level object handling to avoid unnecessary
     * overhead from placing things in the floor chain prior to burial.
     */
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if (levl[x][y+1].typ != STONE) { /* <x,y> spot not eligible */
                y += 2; /* next two spots aren't eligible either */
            } else if (levl[x][y].typ != STONE) { /* this spot not eligible */
                y += 1; /* next spot isn't eligible either */
            } else if (!(levl[x][y].wall_info & W_NONDIGGABLE) &&
                       levl[x][y-1].typ   == STONE &&
                       levl[x+1][y-1].typ == STONE && levl[x-1][y-1].typ == STONE &&
                       levl[x+1][y].typ   == STONE && levl[x-1][y].typ   == STONE &&
                       levl[x+1][y+1].typ == STONE && levl[x-1][y+1].typ == STONE) {
                if (rn2(1000) < goldprob) {
                    if ((otmp = mksobj(GOLD_PIECE, FALSE, FALSE)) != 0) {
                        otmp->ox = x,  otmp->oy = y;
                        otmp->quan = 1L + rnd(goldprob * 3);
                        otmp->owt = weight(otmp);
                        if (!rn2(3)) add_to_buried(otmp);
                        else place_object(otmp, x, y);
                    }
                }
                if (rn2(1000) < gemprob) {
                    for (cnt = rnd(2 + dunlev(&u.uz) / 3); cnt > 0; cnt--)
                        if ((otmp = mkobj(GEM_CLASS, FALSE)) != 0) {
                            if (otmp->otyp == ROCK) {
                                dealloc_obj(otmp); /* discard it */
                            } else {
                                otmp->ox = x,  otmp->oy = y;
                                if (!rn2(3)) add_to_buried(otmp);
                                else place_object(otmp, x, y);
                            }
                        }
                }
            }
}


void
wallwalk_right(xchar x, xchar y, schar fgtyp, schar fglit, schar bgtyp, int chance)
{
    int sx, sy, nx, ny, dir, cnt;
    schar tmptyp;
    sx = x;
    sy = y;
    dir = 1;

    if (!isok(x, y)) return;
    if (levl[x][y].typ != bgtyp) return;

    do {
        if (!t_at(x, y) && !bydoor(x, y) && levl[x][y].typ == bgtyp && (chance >= rn2(100))) {
            SET_TYPLIT(x, y, fgtyp, fglit);
        }
        cnt = 0;
        do {
            nx = x;
            ny = y;
            switch (dir % 4) {
            case 0: y--; break;
            case 1: x++; break;
            case 2: y++; break;
            case 3: x--; break;
            }
            if (isok(x, y)) {
                tmptyp = levl[x][y].typ;
                if (tmptyp != bgtyp && tmptyp != fgtyp) {
                    dir++; x = nx; y = ny; cnt++;
                } else {
                    dir = (dir + 3) % 4;
                }
            } else {
                dir++; x = nx; y = ny; cnt++;
            }
        } while ((nx == x && ny == y) && (cnt < 5));
    } while ((x != sx) || (y != sy));
}


void
mkpoolroom(void)
{
    struct mkroom *sroom;
    schar typ;

    if (!(sroom = pick_room(TRUE))) return;

    if (sroom->hx - sroom->lx < 3 || sroom->hy - sroom->ly < 3) return;

    sroom->rtype = POOLROOM;
    typ = !rn2(5) ? POOL : LAVAPOOL;

    wallwalk_right(sroom->lx, sroom->ly, typ, sroom->rlit, ROOM, 96);
}

static void
place_axe(void)
{
    /* add a random axe to a level with trees */
    int trees = 0, x, y;

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (!isok(x,y)) {
                continue;
            }
            if (IS_TREES(levl[x][y].typ)) {
                trees++;
            }
        }
    }

    if (trees > 0) {
        if (!rnf(ilog2(trees), trees)) {
            int trycount = 0;
            do {
                /* random placement anywhere */
                x = rn2(COLNO);
                y = rn2(ROWNO);
            } while (!SPACE_POS(levl[x][y].typ) && (++trycount < 100));

            if (trycount < 100) {
                struct obj *otmp = mksobj(AXE, TRUE, FALSE);
                place_object(otmp, x, y);
            }
        }
    }
}

static void
post_process_level(void)
{
    place_random_engravings();
    place_axe();
}

void
mklev(void)
{
    struct mkroom *croom;

    set_random_state(level_info[ledger_no(&u.uz)].seed);

    init_mapseen(&u.uz);
    if (getbones()) {
        level_info[ledger_no(&u.uz)].flags |= BONES_LEVEL;
        return;
    }
    in_mklev = TRUE;
    makelevel();
    bound_digging();
    mineralize(-1, -1, -1, -1, FALSE);
    post_process_level();
    in_mklev = FALSE;
    /* has_morgue gets cleared once morgue is entered; graveyard stays
       set (graveyard might already be set even when has_morgue is clear,
       so don't update it unconditionally) */
    if (level.flags.has_morgue)
        level.flags.graveyard = 1;
    if (!level.flags.is_maze_lev) {
        for (croom = &rooms[0]; croom != &rooms[nroom]; croom++)
#ifdef SPECIALIZATION
            topologize(croom, FALSE);
#else
            topologize(croom);
#endif
    }
    set_wall_state();
    /* for many room types, rooms[].rtype is zeroed once the room has been
       entered; rooms[].orig_rtype always retains original rtype value */
    for (int ridx = 0; ridx < SIZE(rooms); ridx++) {
        rooms[ridx].orig_rtype = rooms[ridx].rtype;
    }

    if (!iflags.debug_fuzzer) {
        /* deterministic game state is a feature with fuzzing */
        reseed_random();
    }
}

void
#ifdef SPECIALIZATION
topologize(croom, do_ordinary)
register struct mkroom *croom;
boolean do_ordinary;
#else
topologize(register struct mkroom *croom)

#endif
{
    register int x, y, roomno = (croom - rooms) + ROOMOFFSET;
    register int lowx = croom->lx, lowy = croom->ly;
    register int hix = croom->hx, hiy = croom->hy;
#ifdef SPECIALIZATION
    register schar rtype = croom->rtype;
#endif
    register int subindex, nsubrooms = croom->nsubrooms;

    /* skip the room if already done; i.e. a shop handled out of order */
    /* also skip if this is non-rectangular (it _must_ be done already) */
    if ((int) levl[lowx][lowy].roomno == roomno || croom->irregular)
        return;
#ifdef SPECIALIZATION
# ifdef REINCARNATION
    if (Is_rogue_level(&u.uz))
        do_ordinary = TRUE;     /* vision routine helper */
# endif
    if ((rtype != OROOM) || do_ordinary)
#endif
    {
        /* do innards first */
        for(x = lowx; x <= hix; x++)
            for(y = lowy; y <= hiy; y++)
#ifdef SPECIALIZATION
                if (rtype == OROOM)
                    levl[x][y].roomno = NO_ROOM;
                else
#endif
                levl[x][y].roomno = roomno;
        /* top and bottom edges */
        for(x = lowx-1; x <= hix+1; x++)
            for(y = lowy-1; y <= hiy+1; y += (hiy-lowy+2)) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
        /* sides */
        for(x = lowx-1; x <= hix+1; x += (hix-lowx+2))
            for(y = lowy; y <= hiy; y++) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
    }
    /* subrooms */
    for (subindex = 0; subindex < nsubrooms; subindex++)
#ifdef SPECIALIZATION
        topologize(croom->sbrooms[subindex], (rtype != OROOM));
#else
        topologize(croom->sbrooms[subindex]);
#endif
}

/* Find an unused room for a branch location. */
static struct mkroom *
find_branch_room(coord *mp)
{
    struct mkroom *croom = 0;

    if (nroom == 0) {
        mazexy(mp); /* already verifies location */
    } else {
        int tryct = 0;
        boolean good_room;

        /* search for an ideal place */
        while (++tryct < 100) {
            croom = &rooms[rn2(nroom)];

            good_room = croom != dnstairs_room && croom != upstairs_room && croom->rtype == OROOM;
            if (good_room && (somexyspace(croom, mp, 2) || somexyspace(croom, mp, 0))) {
                return croom;
            }
        }

        /* last effort to place the stairs */
        if (!somexyspace(croom, mp, 2) && !somexyspace(croom, mp, 0)) {
            impossible("can't place branch!");
        }
    }
    return croom;
}

/* Find the room for (x,y).  Return null if not in a room. */
static struct mkroom *
pos_to_room(xchar x, xchar y)
{
    int i;
    struct mkroom *curr;

    for (curr = rooms, i = 0; i < nroom; curr++, i++)
        if (inside_room(curr, x, y)) return curr; ;
    return (struct mkroom *) 0;
}


/* If given a branch, randomly place a special stair or portal. */
void
place_branch(
    branch *br,       /**< branch to place */
    xchar x, xchar y) /**< location */
{
    coord m;
    d_level       *dest;
    boolean make_stairs;
    struct mkroom *br_room;

    /*
     * Return immediately if there is no branch to make or we have
     * already made one.  This routine can be called twice when
     * a special level is loaded that specifies an SSTAIR location
     * as a favored spot for a branch.
     */
    if (!br || made_branch) return;

    if (!x) {   /* find random coordinates for branch */
        br_room = find_branch_room(&m);
        x = m.x;
        y = m.y;
    } else {
        br_room = pos_to_room(x, y);
    }

    if (on_level(&br->end1, &u.uz)) {
        /* we're on end1 */
        make_stairs = br->type != BR_NO_END1;
        dest = &br->end2;
    } else {
        /* we're on end2 */
        make_stairs = br->type != BR_NO_END2;
        dest = &br->end1;
    }

    if (br->type == BR_PORTAL) {
        mkportal(x, y, dest->dnum, dest->dlevel);
    } else if (make_stairs) {
        sstairs.sx = x;
        sstairs.sy = y;
        sstairs.up = (char) on_level(&br->end1, &u.uz) ?
                     br->end1_up : !br->end1_up;
        assign_level(&sstairs.tolev, dest);
        sstairs_room = br_room;

        levl[x][y].ladder = sstairs.up ? LA_UP : LA_DOWN;
        levl[x][y].typ = STAIRS;
    }
    /*
     * Set made_branch to TRUE even if we didn't make a stairwell (i.e.
     * make_stairs is false) since there is currently only one branch
     * per level, if we failed once, we're going to fail again on the
     * next call.
     */
    made_branch = TRUE;
}

boolean
bydoor(register xchar x, register xchar y)
{
    register int typ;

    if (isok(x+1, y)) {
        typ = levl[x+1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR) return TRUE;
    }
    if (isok(x-1, y)) {
        typ = levl[x-1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR) return TRUE;
    }
    if (isok(x, y+1)) {
        typ = levl[x][y+1].typ;
        if (IS_DOOR(typ) || typ == SDOOR) return TRUE;
    }
    if (isok(x, y-1)) {
        typ = levl[x][y-1].typ;
        if (IS_DOOR(typ) || typ == SDOOR) return TRUE;
    }
    return FALSE;
}

/* see whether it is allowable to create a door at [x,y] */
int
okdoor(register xchar x, register xchar y)
{
    register boolean near_door = bydoor(x, y);

    return((levl[x][y].typ == HWALL || levl[x][y].typ == VWALL) &&
           doorindex < DOORMAX && !near_door);
}

void
dodoor(register int x, register int y, register struct mkroom *aroom)
{
    if(doorindex >= DOORMAX) {
        impossible("DOORMAX exceeded?");
        return;
    }

    dosdoor(x, y, aroom, DOOR);
}

boolean
occupied(register xchar x, register xchar y)
{
    return((boolean)(t_at(x, y)
                     || IS_FURNITURE(levl[x][y].typ)
                     || is_lava(x, y)
                     || is_pool(x, y)
                     || invocation_pos(x, y)
                     ));
}

/* make a trap somewhere (in croom if mazeflag = 0 && !tm) */
/* if tm != null, make trap at that location */
void
mktrap(register int num, register int mazeflag, register struct mkroom *croom, coord *tm)
{
    int kind;
    coord m;

    /* no traps in pools */
    if (tm && is_pool(tm->x, tm->y)) return;

    /* no traps in swamps */
    if (tm && is_swamp(tm->x, tm->y)) {
        return;
    }

    if (num > 0 && num < TRAPNUM) {
        kind = num;
#ifdef REINCARNATION
    } else if (Is_rogue_level(&u.uz)) {
        switch (rn2(7)) {
        default: kind = BEAR_TRAP; break; /* 0 */
        case 1: kind = ARROW_TRAP; break;
        case 2: kind = DART_TRAP; break;
        case 3: kind = TRAPDOOR; break;
        case 4: kind = PIT; break;
        case 5: kind = SLP_GAS_TRAP; break;
        case 6: kind = RUST_TRAP; break;
        }
#endif
    } else if (Inhell && !Insheol && !rn2(5)) {
        /* bias the frequency of fire traps in Gehennom */
        kind = FIRE_TRAP;
    } else if (Insheol && !rn2(3)) {
        /* similarly for ice traps */
        kind = ICE_TRAP;
    } else {
        unsigned lvl = level_difficulty();

        do {
            kind = rnd(TRAPNUM-1);
            /* reject "too hard" traps */
            switch (kind) {
            case MAGIC_PORTAL:
            case VIBRATING_SQUARE:
                kind = NO_TRAP; break;
            case ROLLING_BOULDER_TRAP:
            case SLP_GAS_TRAP:
                if (lvl < 2) {
                    kind = NO_TRAP;
                }
                break;
            case LEVEL_TELEP:
                if (lvl < 5 || level.flags.noteleport) {
                    kind = NO_TRAP;
                }
                break;
            case SPIKED_PIT:
                if (lvl < 5) {
                    kind = NO_TRAP;
                }
                break;
            case LANDMINE:
                if (lvl < 6) {
                    kind = NO_TRAP;
                }
                break;
            case WEB:
                if (lvl < 7) {
                    kind = NO_TRAP;
                }
                break;
            case STATUE_TRAP:
            case POLY_TRAP:
                if (lvl < 8) {
                    kind = NO_TRAP;
                }
                break;
            case FIRE_TRAP:
                if (!Inhell || Insheol) {
                    kind = NO_TRAP;
                }
                break;
            case ICE_TRAP:
                if (!Insheol) {
                    kind = NO_TRAP;
                }
                break;
            case TELEP_TRAP:
                if (level.flags.noteleport) {
                    kind = NO_TRAP;
                }
                break;
            case HOLE:
                /* make these much less often than other traps */
                if (rn2(7)) {
                    kind = NO_TRAP;
                }
                break;
            }
        } while (kind == NO_TRAP);
    }

    if ((kind == TRAPDOOR || kind == HOLE) && !Can_fall_thru(&u.uz))
        kind = ROCKTRAP;

    if (tm)
        m = *tm;
    else {
        boolean avoid_boulder = (kind == PIT || kind == SPIKED_PIT ||
                                 kind == TRAPDOOR || kind == HOLE);

        if (mazeflag)
            (void)somexyspace(NULL, &m, 16);
        else if (!somexyspace(croom, &m, (avoid_boulder ? 4 : 0)))
            return;
    }

    (void) maketrap(m.x, m.y, kind);
    /* don't create giant spiders too soon and not always */
    if ((level_difficulty() >= 7) && rnf(6,7)) {
        if (kind == WEB) (void) makemon(&mons[PM_GIANT_SPIDER],
                m.x, m.y, NO_MM_FLAGS);
    }
}

void
mkstairs(xchar x, xchar y, char up, struct mkroom *croom)
{
    if (!x) {
        impossible("mkstairs:  bogus stair attempt at <%d,%d>", x, y);
        return;
    }

    /*
     * We can't make a regular stair off an end of the dungeon.  This
     * attempt can happen when a special level is placed at an end and
     * has an up or down stair specified in its description file.
     */
    if ((dunlev(&u.uz) == 1 && up) ||
        (dunlev(&u.uz) == dunlevs_in_dungeon(&u.uz) && !up))
        return;

    if(up) {
        xupstair = x;
        yupstair = y;
        upstairs_room = croom;
    } else {
        xdnstair = x;
        ydnstair = y;
        dnstairs_room = croom;
    }

    levl[x][y].typ = STAIRS;
    levl[x][y].ladder = up ? LA_UP : LA_DOWN;
}

static void
mkfount(register int mazeflag, register struct mkroom *croom)
{
    coord m;

    if(mazeflag)
        (void)somexyspace(NULL, &m, 16);
    else if (!somexyspace(croom, &m, 8))
        return;

    /* Put a fountain at m.x, m.y */
    levl[m.x][m.y].typ = FOUNTAIN;
    /* Is it a "blessed" fountain? (affects drinking from fountain) */
    if(!rn2(7)) levl[m.x][m.y].blessedftn = 1;

    level.flags.nfountains++;
}

#ifdef SINKS
static void
mksink(register struct mkroom *croom)
{
    coord m;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put a sink at m.x, m.y */
    levl[m.x][m.y].typ = SINK;

    level.flags.nsinks++;
}
#endif /* SINKS */


static void
mkaltar(register struct mkroom *croom)
{
    coord m;
    aligntyp al;

    if (croom->rtype != OROOM) return;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put an altar at m.x, m.y */
    levl[m.x][m.y].typ = ALTAR;

    /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
    al = rn2((int)A_LAWFUL+2) - 1;
    levl[m.x][m.y].altarmask = Align2amask( al );
}

static void
mkgrave(struct mkroom *croom)
{
    coord m;
    register int tryct = 0;
    register struct obj *otmp;
    boolean doobj = !rn2(10);
    boolean dobell = !rn2(2);


    if(croom->rtype != OROOM) return;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put a grave at m.x, m.y */
    if (doobj)
        make_grave(m.x, m.y, dobell ? "Saved by the bell!" : "Apres moi, le deluge.");
    else
        make_grave(m.x, m.y, (char *) 0);

    /* Possibly fill it with objects */
    if (!rn2(3)) (void) mkgold(0L, m.x, m.y);
    for (tryct = rn2(5); tryct; tryct--) {
        otmp = mkobj(RANDOM_CLASS, TRUE);
        if (!otmp) return;
        curse(otmp);
        otmp->ox = m.x;
        otmp->oy = m.y;
        add_to_buried(otmp);
    }

    if (doobj) {
        /* Leave a bell, in case we accidentally buried someone alive... */
        if (dobell) (void) mksobj_at(BELL, m.x, m.y, TRUE, FALSE);
        /* ...or a scroll of flood if the deluge comes after them */
        else (void) mksobj_at(SCR_FLOOD, m.x, m.y, TRUE, FALSE);
    }
    return;
}


/* maze levels have slightly different constraints from normal levels */
#define x_maze_min 2
#define y_maze_min 2
/*
 * Major level transmutation: add a set of stairs (to the Sanctum) after
 * an earthquake that leaves behind a a new topology, centered at inv_pos.
 * Assumes there are no rooms within the invocation area and that inv_pos
 * is not too close to the edge of the map.  Also assume the hero can see,
 * which is guaranteed for normal play due to the fact that sight is needed
 * to read the Book of the Dead.
 */
void
mkinvokearea(void)
{
    int dist;
    xchar xmin = inv_pos.x, xmax = inv_pos.x;
    xchar ymin = inv_pos.y, ymax = inv_pos.y;
    register xchar i;

    pline_The("floor shakes violently under you!");
    pline_The("walls around you begin to bend and crumble!");
    display_nhwindow(WIN_MESSAGE, TRUE);

    mkinvpos(xmin, ymin, 0);        /* middle, before placing stairs */

    for(dist = 1; dist < 7; dist++) {
        xmin--; xmax++;

        /* top and bottom */
        if(dist != 3) { /* the area is wider that it is high */
            ymin--; ymax++;
            for(i = xmin+1; i < xmax; i++) {
                mkinvpos(i, ymin, dist);
                mkinvpos(i, ymax, dist);
            }
        }

        /* left and right */
        for(i = ymin; i <= ymax; i++) {
            mkinvpos(xmin, i, dist);
            mkinvpos(xmax, i, dist);
        }

        flush_screen(1); /* make sure the new glyphs shows up */
        delay_output();
    }

    You("are standing at the top of a stairwell leading down!");
    mkstairs(u.ux, u.uy, 0, (struct mkroom *)0); /* down */
    newsym(u.ux, u.uy);
    vision_full_recalc = 1; /* everything changed */

#ifdef RECORD_ACHIEVE
    achieve.perform_invocation = 1;
#ifdef LIVELOGFILE
    livelog_achieve_update();
#endif
#endif
}

/* Change level topology.  Boulders in the vicinity are eliminated.
 * Temporarily overrides vision in the name of a nice effect.
 */
static void
mkinvpos(xchar x, xchar y, int dist)
{
    struct trap *ttmp;
    struct obj *otmp;
    boolean make_rocks;
    register struct rm *lev = &levl[x][y];

    /* clip at existing map borders if necessary */
    if (!within_bounded_area(x, y, x_maze_min + 1, y_maze_min + 1,
                             x_maze_max - 1, y_maze_max - 1)) {
        /* only outermost 2 columns and/or rows may be truncated due to edge */
        if (dist < (7 - 2))
            panic("mkinvpos: <%d,%d> (%d) off map edge!", x, y, dist);
        return;
    }

    /* clear traps */
    if ((ttmp = t_at(x, y)) != 0) deltrap(ttmp);

    /* clear boulders; leave some rocks for non-{moat|trap} locations */
    make_rocks = (dist != 1 && dist != 4 && dist != 5) ? TRUE : FALSE;
    while ((otmp = sobj_at(BOULDER, x, y)) != 0) {
        if (make_rocks) {
            fracture_rock(otmp);
            make_rocks = FALSE; /* don't bother with more rocks */
        } else {
            obj_extract_self(otmp);
            obfree(otmp, (struct obj *)0);
        }
    }
    unblock_point(x, y); /* make sure vision knows this location is open */

    /* fake out saved state */
    lev->seenv = 0;
    lev->doormask = 0;
    if(dist < 6) lev->lit = TRUE;
    lev->waslit = TRUE;
    lev->horizontal = FALSE;
    viz_array[y][x] = (dist < 6 ) ?
                      (IN_SIGHT|COULD_SEE) : /* short-circuit vision recalc */
                      COULD_SEE;

    switch(dist) {
    case 1: /* fire traps */
        if (is_pool(x, y)) break;
        lev->typ = ROOM;
        ttmp = maketrap(x, y, FIRE_TRAP);
        if (ttmp) ttmp->tseen = TRUE;
        break;
    case 0: /* lit room locations */
    case 2:
    case 3:
    case 6: /* unlit room locations */
        lev->typ = ROOM;
        break;
    case 4: /* pools (aka a wide moat) */
    case 5:
        lev->typ = MOAT;
        /* No kelp! */
        break;
    default:
        impossible("mkinvpos called with dist %d", dist);
        break;
    }

    /* display new value of position; could have a monster/object on it */
    newsym(x, y);
}

/** Gets the level structure for the source level of the Fort Ludios portal.
 * @return the d_level structure
 */
d_level *
get_floating_branch(d_level *target, branch *br)
{
    extern int n_dgns; /* from dungeon.c */
    d_level *source;
    if (on_level(target, &br->end1)) {
        source = &br->end2;
    } else {
        /* disallow branch on a level with one branch already */
        if (Is_branchlev(&u.uz)) {
            return NULL;
        }
        source = &br->end1;
    }

    /* Already set. */
    if (source->dnum < n_dgns) {
        return NULL;
    }
    return source;
}


/*
 * The portal to Ludios is special.  The entrance can only occur within a
 * vault in the main dungeon at a depth greater than 10.  The Ludios branch
 * structure reflects this by having a bogus "source" dungeon:  the value
 * of n_dgns (thus, Is_branchlev() will never find it).
 *
 * Ludios will remain isolated until the branch is corrected by this function.
 */
static void
mk_knox_portal(xchar x, xchar y)
{
    extern int n_dgns;      /* from dungeon.c */
    d_level *source;
    schar u_depth;
    branch *br = dungeon_branch("Fort Ludios");

    source = get_floating_branch(&knox_level, br);
    /* Already set. */
    if (!source) {
        return;
    }

    if (!(u.uz.dnum == oracle_level.dnum        /* in main dungeon */
          && !at_dgn_entrance("The Quest")  /* but not Quest's entry */
          && (u_depth = depth(&u.uz)) > 10  /* beneath 10 */
          && u_depth <= depth(&medusa_level))) { /* and on or above Medusa */
        return;
    }

    /* Adjust source to be current level and re-insert branch. */
    *source = u.uz;
    insert_branch(br, TRUE);

#ifdef DEBUG
    pline("Made knox portal.");
#endif
    place_branch(br, x, y);
}


/*
 * Places a random engraving from an array of engravings into
 * a random room on the current level.
 */
void
place_random_engraving(
    const char *const *engravings,
    const int size /* number of engravings in array */
)
{
    struct mkroom *some_room;
    xchar sx, sy;
    char const *engraving = engravings[rn2(size)];

    int trycount=0;
    do {
        if (Inhell || nroom <= 1) {
            /* random placement in Gehennom or levels with no rooms */
            sx = rn2(COLNO);
            sy = rn2(ROWNO);
        } else {
            coord pos;
            /* placement inside of rooms, not corridors */
            some_room = &rooms[rn2(nroom-1)];
            if (somexyspace(some_room, &pos, 0)) {
                sx = pos.x;
                sy = pos.y;
            } else {
                sx = somex(some_room);
                sy = somey(some_room);
            }
        }
    } while (occupied(sx, sy) && (++trycount < 1000));

    if (trycount < 1000) {
        make_engr_at(sx, sy, engraving, 0L, BURN);
    }
}

/*
 * Places dungeon level dependent engravings on the current level.
 */
void
place_random_engravings(void)
{
    switch (depth(&u.uz)) {
    case 23: place_random_engraving(illuminati_engravings, SIZE(illuminati_engravings)); break;
    case 42: place_random_engraving(hhgtg_engravings, SIZE(hhgtg_engravings)); break;
    }
}

#ifdef ADVENT_CALENDAR
/**
 * The portal to the Advent Calender is special.
 * It does not only lead to a floating branch like knox portal.
 * It also may appear upon reentering a existing level if it is the
 * right time of the year.
 */
boolean
mk_advcal_portal(void)
{
    extern int n_dgns;      /* from dungeon.c */
    d_level *source;
    branch *br;

    /* made_branch remains unchanged when entering a already created
     * level. This leads to the branch inserted in the dungeon level
     * list but no portal created, the branch is unreachable.
     *
     * Technically this is a bug but nobody anticipated a branch
     * that could be inserted after level creation.
     */
    if (made_branch) return FALSE;

    br = dungeon_branch("Advent Calendar");
    source = get_floating_branch(&advcal_level, br);
    /* Already set. */
    if (!source) {
        return FALSE;
    }

    if (!(u.uz.dnum == oracle_level.dnum        /* in main dungeon */
          && !at_dgn_entrance("The Quest")  /* but not Quest's entry */
          && depth(&u.uz) < depth(&medusa_level))) /* and above Medusa */
        return FALSE;

    /* Adjust source to be current level and re-insert branch. */
    *source = u.uz;
    insert_branch(br, TRUE);

#ifdef DEBUG
    pline("Made advent calendar portal.");
#endif
    place_branch(br, 0, 0);

    return TRUE;
}
#endif

/*mklev.c*/
