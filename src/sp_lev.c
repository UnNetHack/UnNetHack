/*	SCCS Id: @(#)sp_lev.c	3.4	2001/09/06	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the various functions that are related to the special
 * levels.
 * It contains also the special level loader.
 *
 */

#include "hack.h"
#include "dlb.h"
/* #define DEBUG */	/* uncomment to enable code debugging */

#ifdef DEBUG
# ifdef WIZARD
#define debugpline	if (wizard) pline
# else
#define debugpline	pline
# endif
#endif

#include "sp_lev.h"
#include "rect.h"
#include "epri.h"
#include "eshk.h"


extern void FDECL(mkmap, (lev_init *));

STATIC_DCL void FDECL(get_room_loc, (schar *, schar *, struct mkroom *));
STATIC_DCL void FDECL(get_free_room_loc, (schar *, schar *, struct mkroom *));
STATIC_DCL void FDECL(create_trap, (trap *, struct mkroom *));
STATIC_DCL int FDECL(noncoalignment, (ALIGNTYP_P));
STATIC_DCL void FDECL(create_monster, (monster *, struct mkroom *));
STATIC_DCL void FDECL(create_object, (object *, struct mkroom *));
STATIC_DCL void FDECL(create_engraving, (engraving *,struct mkroom *));
STATIC_DCL void FDECL(create_stairs, (stair *, struct mkroom *));
STATIC_DCL void FDECL(create_altar, (altar *, struct mkroom *));
STATIC_DCL void FDECL(create_gold, (gold *, struct mkroom *));
STATIC_DCL void FDECL(create_feature, (int,int,struct mkroom *,int));
STATIC_DCL boolean FDECL(search_door, (struct mkroom *, xchar *, xchar *,
					XCHAR_P, int));
STATIC_DCL void NDECL(fix_stair_rooms);
STATIC_DCL void FDECL(create_corridor, (corridor *));
STATIC_DCL void NDECL(count_features);

STATIC_DCL boolean FDECL(create_subroom, (struct mkroom *, XCHAR_P, XCHAR_P,
					XCHAR_P, XCHAR_P, XCHAR_P, XCHAR_P));

#define LEFT	1
#define H_LEFT	2
#define CENTER	3
#define H_RIGHT	4
#define RIGHT	5

#define TOP	1
#define BOTTOM	5

#define XLIM	4
#define YLIM	3

#define Fread	(void)dlb_fread
#define Fgetc	(schar)dlb_fgetc
#define Free(ptr)		if(ptr) free((genericptr_t) (ptr))

extern struct engr *head_engr;

extern int min_rx, max_rx, min_ry, max_ry; /* from mkmap.c */

char SpLev_Map[COLNO][ROWNO];
static char robjects[MAX_REGISTERS], rmonst[MAX_REGISTERS];
static char rloc_x[MAX_REGISTERS], rloc_y[MAX_REGISTERS];
static aligntyp	ralign[3] = { AM_CHAOTIC, AM_NEUTRAL, AM_LAWFUL };
static NEARDATA xchar xstart, ystart;
static NEARDATA char xsize, ysize;
static NEARDATA int n_rloc, n_robj, n_rmon; /* # or random registers */

STATIC_DCL void FDECL(set_wall_property, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,int));
STATIC_DCL int NDECL(rnddoor);
STATIC_DCL int NDECL(rndtrap);
STATIC_DCL void FDECL(get_location, (schar *,schar *,int, struct mkroom *));
STATIC_DCL void FDECL(sp_lev_shuffle, (char *,char *,int));
STATIC_DCL void FDECL(light_region, (region *));
STATIC_DCL void FDECL(load_one_monster, (dlb *,monster *));
STATIC_DCL void FDECL(load_one_object, (dlb *,object *));
STATIC_DCL void FDECL(load_one_engraving, (dlb *,engraving *));
STATIC_DCL void FDECL(maze1xy, (coord *,int));
STATIC_DCL boolean FDECL(sp_level_loader, (dlb *, sp_lev *));
STATIC_DCL void FDECL(create_door, (room_door *, struct mkroom *));
STATIC_DCL struct mkroom *FDECL(build_room, (room *, struct mkroom *));

char *lev_message = 0;
lev_region *lregions = 0;
int num_lregions = 0;

struct obj *container_obj[MAX_CONTAINMENT];
int container_idx = 0;

struct monst *invent_carrying_monster = NULL;

#define SPLEV_STACK_RESERVE 128

void
splev_stack_init(st)
     struct splevstack *st;
{
    if (st) {
	st->depth = 0;
	st->depth_alloc = SPLEV_STACK_RESERVE;
	st->stackdata = (struct opvar **)alloc(st->depth_alloc * sizeof(struct opvar *));
    }
}

void
splev_stack_done(st)
     struct splevstack *st;
{
    if (st) {
	int i;

	if (st->stackdata && st->depth)
	    for (i = 0; i < st->depth; i++) {
		switch (st->stackdata[i]->spovartyp) {
		default:
		case SPOVAR_NULL:
		case SPOVAR_INT:
		    break;
		case SPOVAR_STRING:
		    if (st->stackdata[i]->vardata.str) Free(st->stackdata[i]->vardata.str);
		    st->stackdata[i]->vardata.str = NULL;
		    break;
		}
		Free(st->stackdata[i]);
		st->stackdata[i] = NULL;
	    }

	if (st->stackdata) free(st->stackdata);
	st->stackdata = NULL;
	st->depth = st->depth_alloc = 0;
	Free(st);
    }
}

void
splev_stack_push(st, v)
     struct splevstack *st;
     struct opvar *v;
{
    if (!st) return;
    if (!st->stackdata) panic("splev_stack_push: no stackdata allocated?");

    if (st->depth >= st->depth_alloc) {
	struct opvar **tmp = (struct opvar **)alloc((st->depth_alloc + SPLEV_STACK_RESERVE) * sizeof(struct opvar *));
	(void)memcpy(tmp, st->stackdata, st->depth_alloc * sizeof(struct opvar *));
	Free(st->stackdata);
	st->stackdata = tmp;
	st->depth_alloc += SPLEV_STACK_RESERVE;
    }

    st->stackdata[st->depth] = v;
    st->depth++;
}

struct opvar *
splev_stack_pop(st)
     struct splevstack *st;
{
    struct opvar *ret = NULL;
    if (!st) return ret;
    if (!st->stackdata) panic("splev_stack_push: no stackdata allocated?");

    if (st->depth) {
	st->depth--;
	ret = st->stackdata[st->depth];
	st->stackdata[st->depth] = NULL;
	return ret;
    } else impossible("splev_stack_pop: empty stack?");
    return ret;
}

struct opvar *
splev_stack_getdat(st, typ)
     struct splevstack *st;
     xchar typ;
{
    if (st) {
	struct opvar *tmp = splev_stack_pop(st);
	if (tmp->spovartyp == typ)
	    return tmp;
    }
    return NULL;
}


#define OV_typ(o) (o->spovartyp)
#define OV_i(o) (o->vardata.l)
#define OV_s(o) (o->vardata.str)

#define OV_pop_i(x) (x = splev_stack_getdat(coder->stack, SPOVAR_INT))
#define OV_pop_s(x) (x = splev_stack_getdat(coder->stack, SPOVAR_STRING))


struct opvar *
opvar_new_str(s)
     char *s;
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) panic("could not alloc opvar struct");
    tmpov->spovartyp = SPOVAR_STRING;
    if (s) {
	int len = strlen(s);
	tmpov->vardata.str = (char *)alloc(len + 1);
	(void)memcpy((genericptr_t)tmpov->vardata.str,
		     (genericptr_t)s, len);
	tmpov->vardata.str[len] = '\0';
    } else
	tmpov->vardata.str = NULL;
    return tmpov;
}

struct opvar *
opvar_new_int(i)
     long i;
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) panic("could not alloc opvar struct");
    tmpov->spovartyp = SPOVAR_INT;
    tmpov->vardata.l = i;
    return tmpov;
}

void
opvar_free_x(ov)
     struct opvar *ov;
{
    if (!ov) return;
    switch (ov->spovartyp) {
    case SPOVAR_INT:
	break;
    case SPOVAR_STRING:
	if (ov->vardata.str)
	    Free(ov->vardata.str);
	break;
    default: impossible("Unknown opvar value type (%i)!", ov->spovartyp);
    }
    Free(ov);
}

#define opvar_free(ov) { opvar_free_x(ov); ov = NULL; }

struct opvar *
opvar_clone(ov)
     struct opvar *ov;
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) panic("could not alloc opvar struct");
    switch (ov->spovartyp) {
    case SPOVAR_INT:
	{
	    tmpov->spovartyp = ov->spovartyp;
	    tmpov->vardata.l = ov->vardata.l;
	}
	break;
    case SPOVAR_STRING:
	{
	    int len = strlen(ov->vardata.str);
	    tmpov->spovartyp = ov->spovartyp;
	    tmpov->vardata.str = (char *)alloc(len+1);
	    (void)memcpy((genericptr_t)tmpov->vardata.str,
			 (genericptr_t)ov->vardata.str, len);
	    tmpov->vardata.str[len] = '\0';
	}
	break;
    default: impossible("Unknown push value type (%i)!", ov->spovartyp);
    }
    return tmpov;
}

void
lvlfill_maze_grid(x1,y1,x2,y2,filling)
int x1,y1,x2,y2;
schar filling;
{
	int x,y;

	for (x = x1; x <= x2; x++)
		for (y = y1; y <= y2; y++) {
#ifndef WALLIFIED_MAZE
			levl[x][y].typ = STONE;
#else
			levl[x][y].typ =
				(y < 2 || ((x % 2) && (y % 2))) ? STONE : filling;
#endif
		}
}

void
lvlfill_solid(filling,lit)
schar filling;
schar lit;
{
	int x,y;
	for (x = 2; x <= x_maze_max; x++)
	for (y = 0; y <= y_maze_max; y++) {
			levl[x][y].typ = filling;
		levl[x][y].lit = lit;
	}
}

void flip_drawbridge_horizontal(lev)
struct rm *lev;
{
	if (IS_DRAWBRIDGE(lev->typ)) {
		if ((lev->drawbridgemask & DB_DIR) == DB_WEST) {
			lev->drawbridgemask &= ~DB_WEST;
			lev->drawbridgemask |=  DB_EAST;
		} else if ((lev->drawbridgemask & DB_DIR) == DB_EAST) {
			lev->drawbridgemask &= ~DB_EAST;
			lev->drawbridgemask |=  DB_WEST;
		}
	}
}

void flip_drawbridge_vertical(lev)
struct rm *lev;
{
	if (IS_DRAWBRIDGE(lev->typ)) {
		if ((lev->drawbridgemask & DB_DIR) == DB_NORTH) {
			lev->drawbridgemask &= ~DB_NORTH;
			lev->drawbridgemask |=  DB_SOUTH;
		} else if ((lev->drawbridgemask & DB_DIR) == DB_SOUTH) {
			lev->drawbridgemask &= ~DB_SOUTH;
			lev->drawbridgemask |=  DB_NORTH;
		}
	}
}

void
flip_level(int flp)
{
    int x2 = COLNO-1;
    int y2 = ROWNO-1;

    int x, y, i;

    struct rm trm;

    struct trap *ttmp;
    struct obj *otmp;
    struct monst *mtmp;
    struct engr *etmp;
    struct mkroom *sroom;

    /* stairs and ladders */
    if (flp & 1) {
	yupstair = y2 - yupstair;
	ydnstair = y2 - ydnstair;
	yupladder = y2 - yupladder;
	ydnladder = y2 - ydnladder;
    }
    if (flp & 2) {
	xupstair = x2 - xupstair;
	xdnstair = x2 - xdnstair;
	xupladder = x2 - xupladder;
	xdnladder = x2 - xdnladder;
    }

    /* traps */
    for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
	if (flp & 1) {
	    ttmp->ty = y2 - ttmp->ty;
	    if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
		ttmp->launch.y = y2 - ttmp->launch.y;
		ttmp->launch2.y = y2 - ttmp->launch2.y;
	    }
	}
	if (flp & 2) {
	    ttmp->tx = x2 - ttmp->tx;
	    if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
		ttmp->launch.x = x2 - ttmp->launch.x;
		ttmp->launch2.x = x2 - ttmp->launch2.x;
	    }
	}
    }

    /* objects */
    for (otmp = fobj; otmp; otmp = otmp->nobj) {
	if (flp & 1)
	    otmp->oy = y2 - otmp->oy;
	if (flp & 2)
	    otmp->ox = x2 - otmp->ox;
    }

    /* buried objects */
    for (otmp = level.buriedobjlist; otmp; otmp = otmp->nobj) {
	if (flp & 1)
	    otmp->oy = y2 - otmp->oy;
	if (flp & 2)
	    otmp->ox = x2 - otmp->ox;
    }

    /* monsters */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	if (flp & 1) {
	    mtmp->my = y2 - mtmp->my;
	    if (mtmp->ispriest)
		EPRI(mtmp)->shrpos.y = y2 - EPRI(mtmp)->shrpos.y;
	    else if (mtmp->isshk) {
		ESHK(mtmp)->shk.y = y2 - ESHK(mtmp)->shk.y;
		ESHK(mtmp)->shd.y = y2 - ESHK(mtmp)->shd.y;
	    }
	}
	if (flp & 2) {
	    mtmp->mx = x2 - mtmp->mx;
	    if (mtmp->ispriest)
		EPRI(mtmp)->shrpos.x = x2 - EPRI(mtmp)->shrpos.x;
	    else if (mtmp->isshk) {
		ESHK(mtmp)->shk.x = x2 - ESHK(mtmp)->shk.x;
		ESHK(mtmp)->shd.x = x2 - ESHK(mtmp)->shd.x;
	    }
	}
    }

    /* engravings */
    for (etmp = head_engr; etmp; etmp = etmp->nxt_engr) {
	if (flp & 1)
	    etmp->engr_y = y2 - etmp->engr_y;
	if (flp & 2)
	    etmp->engr_x = x2 - etmp->engr_x;
    }

    /* regions */
    for (i = 0; i < num_lregions; i++) {
	if (flp & 1) {
	    lregions[i].inarea.y1 = y2 - lregions[i].inarea.y1;
	    lregions[i].inarea.y2 = y2 - lregions[i].inarea.y2;
	    if (lregions[i].inarea.y1 > lregions[i].inarea.y2) {
		int tmp = lregions[i].inarea.y1;
		lregions[i].inarea.y1 = lregions[i].inarea.y2;
		lregions[i].inarea.y2 = tmp;
	    }

	    lregions[i].delarea.y1 = y2 - lregions[i].delarea.y1;
	    lregions[i].delarea.y2 = y2 - lregions[i].delarea.y2;
	    if (lregions[i].delarea.y1 > lregions[i].delarea.y2) {
		int tmp = lregions[i].delarea.y1;
		lregions[i].delarea.y1 = lregions[i].delarea.y2;
		lregions[i].delarea.y2 = tmp;
	    }
	}
	if (flp & 2) {
	    lregions[i].inarea.x1 = x2 - lregions[i].inarea.x1;
	    lregions[i].inarea.x2 = x2 - lregions[i].inarea.x2;
	    if (lregions[i].inarea.x1 > lregions[i].inarea.x2) {
		int tmp = lregions[i].inarea.x1;
		lregions[i].inarea.x1 = lregions[i].inarea.x2;
		lregions[i].inarea.x2 = tmp;
	    }

	    lregions[i].delarea.x1 = x2 - lregions[i].delarea.x1;
	    lregions[i].delarea.x2 = x2 - lregions[i].delarea.x2;
	    if (lregions[i].delarea.x1 > lregions[i].delarea.x2) {
		int tmp = lregions[i].delarea.x1;
		lregions[i].delarea.x1 = lregions[i].delarea.x2;
		lregions[i].delarea.x2 = tmp;
	    }
	}
    }

    /* rooms */
    for(sroom = &rooms[0]; ; sroom++) {
	if (sroom->hx < 0) break;

	if (flp & 1) {
	    sroom->ly = y2 - sroom->ly;
	    sroom->hy = y2 - sroom->hy;
	    if (sroom->ly > sroom->hy) {
		int tmp = sroom->ly;
		sroom->ly = sroom->hy;
		sroom->hy = tmp;
	    }
	}
	if (flp & 2) {
	    sroom->lx = x2 - sroom->lx;
	    sroom->hx = x2 - sroom->hx;
	    if (sroom->lx > sroom->hx) {
		int tmp = sroom->lx;
		sroom->lx = sroom->hx;
		sroom->hx = tmp;
	    }
	}

	if (sroom->nsubrooms)
	    for (i = 0; i < sroom->nsubrooms; i++) {
		struct mkroom *rroom = sroom->sbrooms[i];
		if (flp & 1) {
		    rroom->ly = y2 - rroom->ly;
		    rroom->hy = y2 - rroom->hy;
		    if (rroom->ly > rroom->hy) {
			int tmp = rroom->ly;
			rroom->ly = rroom->hy;
			rroom->hy = tmp;
		    }
		}
		if (flp & 2) {
		    rroom->lx = x2 - rroom->lx;
		    rroom->hx = x2 - rroom->hx;
		    if (rroom->lx > rroom->hx) {
			int tmp = rroom->lx;
			rroom->lx = rroom->hx;
			rroom->hx = tmp;
		    }
		}
	    }
    }

    /* doors */
    for (i = 0; i < doorindex; i++) {
	if (flp & 1)
	    doors[i].y = y2 - doors[i].y;
	if (flp & 2)
	    doors[i].x = x2 - doors[i].x;
    }

    /* the map */
    if (flp & 1) {
	for (x = 0; x <= x2; x++)
	    for (y = 0; y <= (y2 / 2); y++) {

		flip_drawbridge_vertical(&levl[x][y]);
		flip_drawbridge_vertical(&levl[x][y2-y]);

		trm = levl[x][y];
		levl[x][y] = levl[x][y2-y];
		levl[x][y2-y] = trm;

		otmp = level.objects[x][y];
		level.objects[x][y] = level.objects[x][y2-y];
		level.objects[x][y2-y] = otmp;

		mtmp = level.monsters[x][y];
		level.monsters[x][y] = level.monsters[x][y2-y];
		level.monsters[x][y2-y] = mtmp;
	    }
    }
    if (flp & 2) {
	for (x = 0; x <= (x2 / 2); x++)
	    for (y = 0; y <= y2; y++) {

		flip_drawbridge_horizontal(&levl[x][y]);
		flip_drawbridge_horizontal(&levl[x2-x][y]);

		trm = levl[x][y];
		levl[x][y] = levl[x2-x][y];
		levl[x2-x][y] = trm;

		otmp = level.objects[x][y];
		level.objects[x][y] = level.objects[x2-x][y];
		level.objects[x2-x][y] = otmp;

		mtmp = level.monsters[x][y];
		level.monsters[x][y] = level.monsters[x2-x][y];
		level.monsters[x2-x][y] = mtmp;
	    }
    }

    wall_extends(1, 0, COLNO-1, ROWNO-1);
}

void
flip_level_rnd(int flp)
{
    int c = 0;
    if ((flp & 1) && rn2(2)) c |= 1;
    if ((flp & 2) && rn2(2)) c |= 2;
    flip_level(c);
}



/*
 * Make walls of the area (x1, y1, x2, y2) non diggable/non passwall-able
 */

STATIC_OVL void
set_wall_property(x1,y1,x2,y2, prop)
xchar x1, y1, x2, y2;
int prop;
{
	register xchar x, y;

	for(y = y1; y <= y2; y++)
	    for(x = x1; x <= x2; x++)
		if(IS_STWALL(levl[x][y].typ))
		    levl[x][y].wall_info |= prop;
}

STATIC_OVL void
shuffle_alignments()
{
    int	i;
    aligntyp atmp;
    /* shuffle 3 alignments; can't use sp_lev_shuffle() on aligntyp's */
    i = rn2(3);   atmp=ralign[2]; ralign[2]=ralign[i]; ralign[i]=atmp;
    if (rn2(2)) { atmp=ralign[1]; ralign[1]=ralign[0]; ralign[0]=atmp; }
}

/*
 * Count the different features (sinks, fountains) in the level.
 */
STATIC_OVL void
count_features()
{
    xchar x,y;
    level.flags.nfountains = level.flags.nsinks = 0;
    for (y = 0; y < ROWNO; y++)
	for (x = 0; x < COLNO; x++) {
	    int typ = levl[x][y].typ;
	    if (typ == FOUNTAIN)
		level.flags.nfountains++;
	    else if (typ == SINK)
		level.flags.nsinks++;
	}
}

void
remove_boundary_syms()
{
    /*
     * If any CROSSWALLs are found, must change to ROOM after REGION's
     * are laid out.  CROSSWALLS are used to specify "invisible"
     * boundaries where DOOR syms look bad or aren't desirable.
     */
    xchar x,y;
    boolean has_bounds = FALSE;
    for (x = 0; x < COLNO-1; x++)
	for (y = 0; y < ROWNO-1; y++)
	    if ((levl[x][y].typ == CROSSWALL)) {
		has_bounds = TRUE;
		break;
	    }
    if (has_bounds) {
	for(x = 0; x < x_maze_max; x++)
	    for(y = 0; y < y_maze_max; y++)
		if ((levl[x][y].typ == CROSSWALL) && !SpLev_Map[x][y])
		    levl[x][y].typ = ROOM;
    }
}

void
fill_rooms()
{
    int tmpi;
    for (tmpi = 0; tmpi < nroom; tmpi++) {
	int m;
	if (rooms[tmpi].needfill)
	    fill_room(&rooms[tmpi], (rooms[tmpi].needfill == 2));
	for (m = 0; m < rooms[tmpi].nsubrooms; m++)
	    if (rooms[tmpi].sbrooms[m]->needfill)
		fill_room(rooms[tmpi].sbrooms[m], FALSE);
    }
}

/*
 * Choose randomly the state (nodoor, open, closed or locked) for a door
 */
STATIC_OVL int
rnddoor()
{
	int i = 1 << rn2(5);
	i >>= 1;
	return i;
}

/*
 * Select a random trap
 */
STATIC_OVL int
rndtrap()
{
	int rtrap;

	do {
	    rtrap = rnd(TRAPNUM-1);
	    switch (rtrap) {
	     case HOLE:		/* no random holes on special levels */
	     case MAGIC_PORTAL:	rtrap = NO_TRAP;
				break;
	     case TRAPDOOR:	if (!Can_dig_down(&u.uz)) rtrap = NO_TRAP;
				break;
	     case LEVEL_TELEP:
	     case TELEP_TRAP:	if (level.flags.noteleport) rtrap = NO_TRAP;
				break;
	     case ROLLING_BOULDER_TRAP:
	     case ROCKTRAP:	if (In_endgame(&u.uz)) rtrap = NO_TRAP;
				break;
	    }
	} while (rtrap == NO_TRAP);
	return rtrap;
}

/*
 * Coordinates in special level files are handled specially:
 *
 *	if x or y is -(MAX_REGISTERS+1), we generate a random coordinate.
 *	if x or y is between -1 and -MAX_REGISTERS, we read one from the corresponding
 *	register (x0, x1, ... x9).
 *	if x or y is nonnegative, we convert it from relative to the local map
 *	to global coordinates.
 *	The "humidity" flag is used to insure that engravings aren't
 *	created underwater, or eels on dry land.
 */
#define DRY	0x1
#define WET	0x2

STATIC_DCL boolean FDECL(is_ok_location, (SCHAR_P, SCHAR_P, int));

STATIC_OVL void
get_location(x, y, humidity, croom)
schar *x, *y;
int humidity;
struct mkroom *croom;
{
	int cpt = 0;
        int mx, my, sx, sy;

        if (croom) {
	    mx = croom->lx;
	    my = croom->ly;
	    sx = croom->hx - mx + 1;
	    sy = croom->hy - my + 1;
	} else {
	    mx = xstart;
	    my = ystart;
	    sx = xsize;
	    sy = ysize;
	}

	if (*x >= 0) {			/* normal locations */
		*x += mx;
		*y += my;
	} else if (*x > -(MAX_REGISTERS+1)) {		/* special locations */
		*y = my + rloc_y[ - *y - 1];
		*x = mx + rloc_x[ - *x - 1];
	} else {			/* random location */
	    do {
		*x = mx + rn2((int)sx);
		*y = my + rn2((int)sy);
		if (is_ok_location(*x,*y,humidity)) break;
	    } while (++cpt < 100);
	    if (cpt >= 100) {
		register int xx, yy;
		/* last try */
		for (xx = 0; xx < sx; xx++)
		    for (yy = 0; yy < sy; yy++) {
			*x = mx + xx;
			*y = my + yy;
			if (is_ok_location(*x,*y,humidity)) goto found_it;
		    }
		impossible("get_location:  can't find a place!");
	    }
	}
found_it:;

	if (!isok(*x,*y)) {
	    warning("get_location:  (%d,%d) out of bounds", *x, *y);
	    *x = x_maze_max; *y = y_maze_max;
	}
}

STATIC_OVL boolean
is_ok_location(x, y, humidity)
register schar x, y;
register int humidity;
{
	register int typ;

	if (Is_waterlevel(&u.uz)) return TRUE;	/* accept any spot */

	if (humidity & DRY) {
	    typ = levl[x][y].typ;
	    if (typ == ROOM || typ == AIR ||
		    typ == CLOUD || typ == ICE || typ == CORR)
		return TRUE;
	}
	if (humidity & WET) {
	    if (is_pool(x,y) || is_lava(x,y))
		return TRUE;
	}
	return FALSE;
}

/*
 * Shuffle the registers for locations, objects or monsters
 */

STATIC_OVL void
sp_lev_shuffle(list1, list2, n)
char list1[], list2[];
int n;
{
	register int i, j;
	register char k;

	for (i = n - 1; i > 0; i--) {
		if ((j = rn2(i + 1)) == i) continue;
		k = list1[j];
		list1[j] = list1[i];
		list1[i] = k;
		if (list2) {
			k = list2[j];
			list2[j] = list2[i];
			list2[i] = k;
		}
	}
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

STATIC_OVL void
get_room_loc(x,y, croom)
schar		*x, *y;
struct mkroom	*croom;
{
	coord c;

	if (*x <0 && *y <0) {
		if (somexy(croom, &c)) {
			*x = c.x;
			*y = c.y;
		} else
		    panic("get_room_loc : can't find a place!");
	} else {
		if (*x < 0)
		    *x = rn2(croom->hx - croom->lx + 1);
		if (*y < 0)
		    *y = rn2(croom->hy - croom->ly + 1);
		*x += croom->lx;
		*y += croom->ly;
	}
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

STATIC_OVL void
get_free_room_loc(x,y, croom)
schar		*x, *y;
struct mkroom	*croom;
{
	schar try_x, try_y;
	register int trycnt = 0;

        try_x = *x,  try_y = *y;

	get_location(&try_x, &try_y, DRY, croom);
        if (levl[try_x][try_y].typ != ROOM) {
	    do {
		try_x = *x,  try_y = *y;
		get_room_loc(&try_x, &try_y, croom);
	    } while (levl[try_x][try_y].typ != ROOM && ++trycnt <= 100);

	    if (trycnt > 100)
		panic("get_free_room_loc:  can't find a place!");
	}
        *x = try_x,  *y = try_y;
}

boolean
check_room(lowx, ddx, lowy, ddy, vault)
xchar *lowx, *ddx, *lowy, *ddy;
boolean vault;
{
	register int x,y,hix = *lowx + *ddx, hiy = *lowy + *ddy;
	register struct rm *lev;
	int xlim, ylim, ymax;

	xlim = XLIM + (vault ? 1 : 0);
	ylim = YLIM + (vault ? 1 : 0);

	if (*lowx < 3)		*lowx = 3;
	if (*lowy < 2)		*lowy = 2;
	if (hix > COLNO-3)	hix = COLNO-3;
	if (hiy > ROWNO-3)	hiy = ROWNO-3;
chk:
	if (hix <= *lowx || hiy <= *lowy)	return FALSE;

	/* check area around room (and make room smaller if necessary) */
	for (x = *lowx - xlim; x<= hix + xlim; x++) {
		if(x <= 0 || x >= COLNO) continue;
		y = *lowy - ylim;	ymax = hiy + ylim;
		if(y < 0) y = 0;
		if(ymax >= ROWNO) ymax = (ROWNO-1);
		lev = &levl[x][y];
		for (; y <= ymax; y++) {
			if (lev++->typ) {
#ifdef DEBUG
				if(!vault)
				    debugpline("strange area [%d,%d] in check_room.",x,y);
#endif
				if (!rn2(3))	return FALSE;
				if (x < *lowx)
				    *lowx = x + xlim + 1;
				else
				    hix = x - xlim - 1;
				if (y < *lowy)
				    *lowy = y + ylim + 1;
				else
				    hiy = y - ylim - 1;
				goto chk;
			}
		}
	}
	*ddx = hix - *lowx;
	*ddy = hiy - *lowy;
	return TRUE;
}

/*
 * Create a new room.
 * This is still very incomplete...
 */

boolean
create_room(x,y,w,h,xal,yal,rtype,rlit)
xchar	x,y;
xchar	w,h;
xchar	xal,yal;
xchar	rtype, rlit;
{
	xchar	xabs, yabs;
	int	wtmp, htmp, xaltmp, yaltmp, xtmp, ytmp;
	NhRect	*r1 = 0, r2;
	int	trycnt = 0;
	boolean	vault = FALSE;
	int	xlim = XLIM, ylim = YLIM;

	if (rtype == -1)	/* Is the type random ? */
	    rtype = OROOM;

	if (rtype == VAULT) {
		vault = TRUE;
		xlim++;
		ylim++;
	}

	/* on low levels the room is lit (usually) */
	/* some other rooms may require lighting */

	/* is light state random ? */
	if (rlit == -1)
	    rlit = (rnd(1+abs(depth(&u.uz))) < 11 && rn2(77)) ? TRUE : FALSE;

	/*
	 * Here we will try to create a room. If some parameters are
	 * random we are willing to make several try before we give
	 * it up.
	 */
	do {
		xchar xborder, yborder;
		wtmp = w; htmp = h;
		xtmp = x; ytmp = y;
		xaltmp = xal; yaltmp = yal;

		/* First case : a totaly random room */

		if((xtmp < 0 && ytmp <0 && wtmp < 0 && xaltmp < 0 &&
		   yaltmp < 0) || vault) {
			xchar hx, hy, lx, ly, dx, dy;
			r1 = rnd_rect(); /* Get a random rectangle */

			if (!r1) { /* No more free rectangles ! */
#ifdef DEBUG
				debugpline("No more rects...");
#endif
				return FALSE;
			}
			hx = r1->hx;
			hy = r1->hy;
			lx = r1->lx;
			ly = r1->ly;
			if (vault)
			    dx = dy = 1;
			else {
				dx = 2 + rn2((hx-lx > 28) ? 12 : 8);
				dy = 2 + rn2(4);
				if(dx*dy > 50)
				    dy = 50/dx;
			}
			xborder = (lx > 0 && hx < COLNO -1) ? 2*xlim : xlim+1;
			yborder = (ly > 0 && hy < ROWNO -1) ? 2*ylim : ylim+1;
			if(hx-lx < dx + 3 + xborder ||
			   hy-ly < dy + 3 + yborder) {
				r1 = 0;
				continue;
			}
			xabs = lx + (lx > 0 ? xlim : 3)
			    + rn2(hx - (lx>0?lx : 3) - dx - xborder + 1);
			yabs = ly + (ly > 0 ? ylim : 2)
			    + rn2(hy - (ly>0?ly : 2) - dy - yborder + 1);
			if (ly == 0 && hy >= (ROWNO-1) &&
			    (!nroom || !rn2(nroom)) && (yabs+dy > ROWNO/2)) {
			    yabs = rn1(3, 2);
			    if(nroom < 4 && dy>1) dy--;
		        }
			if (!check_room(&xabs, &dx, &yabs, &dy, vault)) {
				r1 = 0;
				continue;
			}
			wtmp = dx+1;
			htmp = dy+1;
			r2.lx = xabs-1; r2.ly = yabs-1;
			r2.hx = xabs + wtmp;
			r2.hy = yabs + htmp;
		} else {	/* Only some parameters are random */
			int rndpos = 0;
			if (xtmp < 0 && ytmp < 0) { /* Position is RANDOM */
				xtmp = rnd(5);
				ytmp = rnd(5);
				rndpos = 1;
			}
			if (wtmp < 0 || htmp < 0) { /* Size is RANDOM */
				wtmp = rn1(15, 3);
				htmp = rn1(8, 2);
			}
			if (xaltmp == -1) /* Horizontal alignment is RANDOM */
			    xaltmp = rnd(3);
			if (yaltmp == -1) /* Vertical alignment is RANDOM */
			    yaltmp = rnd(3);

			/* Try to generate real (absolute) coordinates here! */

			xabs = (((xtmp-1) * COLNO) / 5) + 1;
			yabs = (((ytmp-1) * ROWNO) / 5) + 1;
			switch (xaltmp) {
			      case LEFT:
				break;
			      case RIGHT:
				xabs += (COLNO / 5) - wtmp;
				break;
			      case CENTER:
				xabs += ((COLNO / 5) - wtmp) / 2;
				break;
			}
			switch (yaltmp) {
			      case TOP:
				break;
			      case BOTTOM:
				yabs += (ROWNO / 5) - htmp;
				break;
			      case CENTER:
				yabs += ((ROWNO / 5) - htmp) / 2;
				break;
			}

			if (xabs + wtmp - 1 > COLNO - 2)
			    xabs = COLNO - wtmp - 3;
			if (xabs < 2)
			    xabs = 2;
			if (yabs + htmp - 1> ROWNO - 2)
			    yabs = ROWNO - htmp - 3;
			if (yabs < 2)
			    yabs = 2;

			/* Try to find a rectangle that fit our room ! */

			r2.lx = xabs-1; r2.ly = yabs-1;
			r2.hx = xabs + wtmp + rndpos;
			r2.hy = yabs + htmp + rndpos;
			r1 = get_rect(&r2);
		}
	} while (++trycnt <= 100 && !r1);
	if (!r1) {	/* creation of room failed ? */
		return FALSE;
	}
	split_rects(r1, &r2);

	if (!vault) {
		smeq[nroom] = nroom;
		add_room(xabs, yabs, xabs+wtmp-1, yabs+htmp-1,
			 rlit, rtype, FALSE);
	} else {
		rooms[nroom].lx = xabs;
		rooms[nroom].ly = yabs;
	}
	return TRUE;
}

/*
 * Create a subroom in room proom at pos x,y with width w & height h.
 * x & y are relative to the parent room.
 */

STATIC_OVL boolean
create_subroom(proom, x, y, w,  h, rtype, rlit)
struct mkroom *proom;
xchar x,y;
xchar w,h;
xchar rtype, rlit;
{
	xchar width, height;

	width = proom->hx - proom->lx + 1;
	height = proom->hy - proom->ly + 1;

	/* There is a minimum size for the parent room */
	if (width < 4 || height < 4)
	    return FALSE;

	/* Check for random position, size, etc... */

	if (w == -1)
	    w = rnd(width - 3);
	if (h == -1)
	    h = rnd(height - 3);
	if (x == -1)
	    x = rnd(width - w - 1) - 1;
	if (y == -1)
	    y = rnd(height - h - 1) - 1;
	if (x == 1)
	    x = 0;
	if (y == 1)
	    y = 0;
	if ((x + w + 1) == width)
	    x++;
	if ((y + h + 1) == height)
	    y++;
	if (rtype == -1)
	    rtype = OROOM;
	if (rlit == -1)
	    rlit = (rnd(1+abs(depth(&u.uz))) < 11 && rn2(77)) ? TRUE : FALSE;
	add_subroom(proom, proom->lx + x, proom->ly + y,
		    proom->lx + x + w - 1, proom->ly + y + h - 1,
		    rlit, rtype, FALSE);
	return TRUE;
}

/*
 * Create a new door in a room.
 * It's placed on a wall (north, south, east or west).
 */

STATIC_OVL void
create_door(dd, broom)
room_door *dd;
struct mkroom *broom;
{
	int	x = 0, y = 0;
	int	trycnt = 0, walltry = 0, wtry = 0;

	if (dd->secret == -1)
	    dd->secret = rn2(2);

	if (dd->mask == -1) {
		/* is it a locked door, closed, or a doorway? */
		if (!dd->secret) {
			if(!rn2(3)) {
				if(!rn2(5))
				    dd->mask = D_ISOPEN;
				else if(!rn2(6))
				    dd->mask = D_LOCKED;
				else
				    dd->mask = D_CLOSED;
				if (dd->mask != D_ISOPEN && !rn2(25))
				    dd->mask |= D_TRAPPED;
			} else
			    dd->mask = D_NODOOR;
		} else {
			if(!rn2(5))	dd->mask = D_LOCKED;
			else		dd->mask = D_CLOSED;

			if(!rn2(20)) dd->mask |= D_TRAPPED;
		}
	}

	do {
		register int dwall, dpos;

		dwall = dd->wall;
		if (dwall == -1)	/* The wall is RANDOM */
		    dwall = 1 << rn2(4);

		dpos = dd->pos;
		if (dpos == -1)	/* The position is RANDOM */
		    dpos = rn2(((dwall & (W_WEST|W_EAST)) ? 2 : 1) ?
			    (broom->hy - broom->ly + 1) : (broom->hx - broom->lx + 1));

		/* Convert wall and pos into an absolute coordinate! */
		wtry = rn2(4);
		for (walltry = 0; walltry < 4; walltry++) {
		    switch ((wtry+walltry) % 4) {
		      case 0:
			if (!(dwall & W_NORTH)) break;
			y = broom->ly - 1;
			x = broom->lx + dpos;
			goto outdirloop;
		      case 1:
			if (!(dwall & W_SOUTH)) break;
			y = broom->hy + 1;
			x = broom->lx + dpos;
			goto outdirloop;
		      case 2:
			if (!(dwall & W_WEST)) break;
			x = broom->lx - 1;
			y = broom->ly + dpos;
			goto outdirloop;
		      case 3:
			if (!(dwall & W_EAST)) break;
			x = broom->hx + 1;
			y = broom->ly + dpos;
			goto outdirloop;
		      default:
			x = y = 0;
			panic("create_door: No wall for door!");
			goto outdirloop;
		    }
		}
outdirloop:
		if (okdoor(x,y))
		    break;
	} while (++trycnt <= 100);
	if (trycnt > 100) {
		impossible("create_door: Can't find a proper place!");
		return;
	}
	add_door(x,y,broom);
	levl[x][y].typ = (dd->secret ? SDOOR : DOOR);
	levl[x][y].doormask = dd->mask;
}

/*
 * Create a secret door in croom on any one of the specified walls.
 */
void
create_secret_door(croom, walls)
    struct mkroom *croom;
    xchar walls; /* any of W_NORTH | W_SOUTH | W_EAST | W_WEST (or W_ANY) */
{
    xchar sx, sy; /* location of the secret door */
    int count;

    for(count = 0; count < 100; count++) {
	sx = rn1(croom->hx - croom->lx + 1, croom->lx);
	sy = rn1(croom->hy - croom->ly + 1, croom->ly);

	switch(rn2(4)) {
	case 0:  /* top */
	    if(!(walls & W_NORTH)) continue;
	    sy = croom->ly-1; break;
	case 1: /* bottom */
	    if(!(walls & W_SOUTH)) continue;
	    sy = croom->hy+1; break;
	case 2: /* left */
	    if(!(walls & W_EAST)) continue;
	    sx = croom->lx-1; break;
	case 3: /* right */
	    if(!(walls & W_WEST)) continue;
	    sx = croom->hx+1; break;
	}

	if(okdoor(sx,sy)) {
	    levl[sx][sy].typ = SDOOR;
	    levl[sx][sy].doormask = D_CLOSED;
	    add_door(sx,sy,croom);
	    return;
	}
    }

    impossible("couldn't create secret door on any walls 0x%x", walls);
}

/*
 * Create a trap in a room.
 */

STATIC_OVL void
create_trap(t,croom)
trap	*t;
struct mkroom	*croom;
{
    schar	x,y;
    coord	tm;

	x = t->x;
	y = t->y;
	if (croom)
	    get_free_room_loc(&x, &y, croom);
	else
	    get_location(&x, &y, DRY, croom);

	tm.x = x;
	tm.y = y;

	mktrap(t->type, 1, (struct mkroom*) 0, &tm);
}

STATIC_OVL void
spill_terrain(sp, croom)
spill* sp;
struct mkroom* croom;
{
	schar x,y,nx,ny,qx,qy;
	int j,k,lastdir,guard;
	boolean found = FALSE;

	if (sp->typ >= MAX_TYPE) return;

	/* This code assumes that you're going to spill one particular
	 * type of terrain from a wall into somewhere.
	 *
	 * If we were given a specific coordinate, though, it doesn't have
	 * to start from a wall... */
	if (sp->x <= -(MAX_REGISTERS+1) || sp->y <= -(MAX_REGISTERS+1)) {
		for (j = 0;j < 500;j++) {
			x = sp->x;
			y = sp->y;
			get_location(&x, &y, DRY|WET, croom);
			nx = x; ny = y;
			switch (sp->direction) {
				case W_NORTH: ny++; break;	  /* backwards to make sure we're against a wall */
				case W_SOUTH: ny--; break;
				case W_WEST: nx++; break;
				case W_EAST: nx--; break;
				default: return; break;
			}
			if (!isok(nx,ny)) { continue; }
			if (IS_WALL(levl[nx][ny].typ)) {	 /* mark it as broken through */
				levl[nx][ny].typ = sp->typ;
				levl[nx][ny].lit = sp->lit;
				found = TRUE;
				break;
			}
		}
	} else {
		found = TRUE;
		x = sp->x;
		y = sp->y;
		get_location(&x, &y, DRY|WET, croom); /* support random registers too */
	}

	if (!found) { return; }

	/* gloop! */
	lastdir = -1; nx = x; ny = y;
	for (j = sp->count;j > 0;j--) {
		guard = 0;
		levl[nx][ny].typ = sp->typ;
		levl[nx][ny].lit = sp->lit;
		do {
			guard++;
			do {
				k = rn2(5);
				qx = nx;qy = ny;
				if (k > 3) { k = sp->direction; }
				else { k = 1 << k; }
				switch(k) {
					case W_NORTH: qy--; break;
					case W_SOUTH: qy++; break;
					case W_WEST: qx--; break;
					case W_EAST: qx++; break;
				}
			} while (!isok(qx,qy));
		} while ((k == lastdir || levl[qx][qy].typ == sp->typ) && guard < 200);
		/* tend to not make rivers, but pools; and don't redo stuff of the same type! */

		switch(k) {
			case W_NORTH: ny--; break;
			case W_SOUTH: ny++; break;
			case W_WEST: nx--; break;
			case W_EAST: nx++; break;
		}
		lastdir = k;
	}
}


/*
 * Create a monster in a room.
 */

STATIC_OVL int
noncoalignment(alignment)
aligntyp alignment;
{
	int k;

	k = rn2(2);
	if (!alignment)
		return(k ? -1 : 1);
	return(k ? -alignment : 0);
}

STATIC_OVL void
create_monster(m,croom)
monster	*m;
struct mkroom	*croom;
{
    struct monst *mtmp;
    schar x, y;
    char class;
    aligntyp amask;
    coord cc;
    struct permonst *pm;
    unsigned g_mvflags;


	if (m->class >= 0)
	    class = (char) def_char_to_monclass((char)m->class);
	else if (m->class > -(MAX_REGISTERS+1))
	    class = (char) def_char_to_monclass(rmonst[- m->class - 1]);
	else
	    class = 0;

	if (class == MAXMCLASSES)
	    panic("create_monster: unknown monster class '%c'", m->class);

	amask = (m->align == AM_SPLEV_CO) ?
			Align2amask(u.ualignbase[A_ORIGINAL]) :
		(m->align == AM_SPLEV_NONCO) ?
			Align2amask(noncoalignment(u.ualignbase[A_ORIGINAL])) :
		(m->align <= -(MAX_REGISTERS+1)) ? induced_align(80) :
		(m->align < 0 ? ralign[-m->align-1] : m->align);

	if (!class)
	    pm = (struct permonst *) 0;
	else if (m->id != NON_PM) {
	    pm = &mons[m->id];
	    g_mvflags = (unsigned) mvitals[monsndx(pm)].mvflags;
	    if ((pm->geno & G_UNIQ) && (g_mvflags & G_EXTINCT))
		return;
	    else if (g_mvflags & G_GONE)	/* genocided or extinct */
		pm = (struct permonst *) 0;	/* make random monster */
	} else {
	    pm = mkclass(class,G_NOGEN);
	    /* if we can't get a specific monster type (pm == 0) then the
	       class has been genocided, so settle for a random monster */
	}
	if (In_mines(&u.uz) && pm && your_race(pm) &&
			(Race_if(PM_DWARF) || Race_if(PM_GNOME)) && rn2(3))
	    pm = (struct permonst *) 0;

	x = m->x;
	y = m->y;
	    if (!pm || !is_swimmer(pm))
		get_location(&x, &y, DRY, croom);
	    else if (pm->mlet == S_EEL)
		get_location(&x, &y, WET, croom);
	    else
		get_location(&x, &y, DRY|WET, croom);
	/* try to find a close place if someone else is already there */
	if (MON_AT(x,y) && enexto(&cc, x, y, pm))
	    x = cc.x,  y = cc.y;

	if(m->align != -(MAX_REGISTERS+2))
	    mtmp = mk_roamer(pm, Amask2align(amask), x, y, m->peaceful);
	else if(PM_ARCHEOLOGIST <= m->id && m->id <= PM_WIZARD)
	         mtmp = mk_mplayer(pm, x, y, FALSE);
	else mtmp = makemon(pm, x, y, NO_MM_FLAGS);

	if (mtmp) {
	    /* handle specific attributes for some special monsters */
	    if (m->name.str) mtmp = christen_monst(mtmp, m->name.str);

	    /*
	     * This is currently hardwired for mimics only.  It should
	     * eventually be expanded.
	     */
	    if (m->appear_as.str && mtmp->data->mlet == S_MIMIC) {
		int i;

		switch (m->appear) {
		    case M_AP_NOTHING:
			warning(
		"create_monster: mon has an appearance, \"%s\", but no type",
				m->appear_as.str);
			break;

		    case M_AP_FURNITURE:
			for (i = 0; i < MAXPCHARS; i++)
			    if (!strcmp(defsyms[i].explanation,
					m->appear_as.str))
				break;
			if (i == MAXPCHARS) {
			    warning(
				"create_monster: can't find feature \"%s\"",
				m->appear_as.str);
			} else {
			    mtmp->m_ap_type = M_AP_FURNITURE;
			    mtmp->mappearance = i;
			}
			break;

		    case M_AP_OBJECT:
			for (i = 0; i < NUM_OBJECTS; i++)
			    if (OBJ_NAME(objects[i]) &&
				!strcmp(OBJ_NAME(objects[i]),m->appear_as.str))
				break;
			if (i == NUM_OBJECTS) {
			    warning(
				"create_monster: can't find object \"%s\"",
				m->appear_as.str);
			} else {
			    mtmp->m_ap_type = M_AP_OBJECT;
			    mtmp->mappearance = i;
			}
			break;

		    case M_AP_MONSTER:
			/* note: mimics don't appear as monsters! */
			/*	 (but chameleons can :-)	  */
		    default:
			warning(
		"create_monster: unimplemented mon appear type [%d,\"%s\"]",
				m->appear, m->appear_as.str);
			break;
		}
		if (does_block(x, y, &levl[x][y]))
		    block_point(x, y);
	    }

	    if (m->peaceful >= 0) {
		mtmp->mpeaceful = m->peaceful;
		/* changed mpeaceful again; have to reset malign */
		set_malign(mtmp);
	    }
	    if (m->asleep >= 0) {
#ifdef UNIXPC
		/* optimizer bug strikes again */
		if (m->asleep)
			mtmp->msleeping = 1;
		else
			mtmp->msleeping = 0;
#else
		mtmp->msleeping = m->asleep;
#endif
	    }

	    if (m->seentraps) mtmp->mtrapseen = m->seentraps;
	    if (m->female) mtmp->female = 1;
	    if (m->cancelled) mtmp->mcan = 1;
	    if (m->revived) mtmp->mrevived = 1;
	    if (m->avenge) mtmp->mavenge = 1;
	    if (m->stunned) mtmp->mstun = 1;
	    if (m->confused) mtmp->mconf = 1;
	    if (m->invis) {
		mtmp->minvis = mtmp->perminvis = 1;
	    }
	    if (m->blinded) {
		mtmp->mcansee = 0;
		mtmp->mblinded = (m->blinded % 127);
	    }
	    if (m->paralyzed) {
		mtmp->mcanmove = 0;
		mtmp->mfrozen = (m->paralyzed % 127);
	    }
	    if (m->fleeing) {
		mtmp->mflee = 1;
		mtmp->mfleetim = (m->fleeing % 127);
	    }

	    if (m->has_invent) {
		discard_minvent(mtmp);
		invent_carrying_monster = mtmp;
	    }
	}

}

/*
 * Create an object in a room.
 */

STATIC_OVL void
create_object(o,croom)
object	*o;
struct mkroom	*croom;
{
    struct obj *otmp;
    schar x, y;
    char c;
    boolean named;	/* has a name been supplied in level description? */

	named = o->name.str ? TRUE : FALSE;

	x = o->x; y = o->y;
	get_location(&x, &y, DRY, croom);

	if (o->class >= 0)
	    c = o->class;
	else if (o->class > -(MAX_REGISTERS+1))
	    c = robjects[ -(o->class+1)];
	else
	    c = 0;

	if (!c)
	    otmp = mkobj_at(RANDOM_CLASS, x, y, !named);
	else if (o->id != -1)
	    otmp = mksobj_at(o->id, x, y, TRUE, !named);
	else {
	    /*
	     * The special levels are compiled with the default "text" object
	     * class characters.  We must convert them to the internal format.
	     */
	    char oclass = (char) def_char_to_objclass(c);

	    if (oclass == MAXOCLASSES)
		panic("create_object:  unexpected object class '%c'",c);

	    /* KMH -- Create piles of gold properly */
	    if (oclass == COIN_CLASS)
		otmp = mkgold(0L, x, y);
	    else
		otmp = mkobj_at(oclass, x, y, !named);
	}

	if (o->spe != -127)	/* That means NOT RANDOM! */
	    otmp->spe = (schar)o->spe;

	switch (o->curse_state) {
	      case 1:	bless(otmp); break; /* BLESSED */
	      case 2:	unbless(otmp); uncurse(otmp); break; /* uncursed */
	      case 3:	curse(otmp); break; /* CURSED */
	      default:	break;	/* Otherwise it's random and we're happy
				 * with what mkobj gave us! */
	}

	/*	corpsenm is "empty" if -1, random if -2, otherwise specific */
	if (o->corpsenm == NON_PM - 1) otmp->corpsenm = rndmonnum();
	else if (o->corpsenm != NON_PM) otmp->corpsenm = o->corpsenm;

	/* assume we wouldn't be given an egg corpsenm unless it was
	   hatchable */
	if (otmp->otyp == EGG && otmp->corpsenm != NON_PM) {
	    if (dead_species(otmp->otyp, TRUE))
		kill_egg(otmp);	/* make sure nothing hatches */
	    else
		attach_egg_hatch_timeout(otmp);	/* attach new hatch timeout */
	}

	if (named)
	    otmp = oname(otmp, o->name.str);

	if (o->eroded) {
	    if (o->eroded < 0) otmp->oerodeproof = 1;
	    else {
		otmp->oeroded = (o->eroded % 4);
		otmp->oeroded2 = ((o->eroded >> 2) % 4);
	    }
	}
	if (o->recharged) otmp->recharged = (o->recharged % 8);
	if (o->locked) otmp->olocked = 1;
	else if (o->broken) {
	    otmp->obroken = 1;
	    otmp->olocked = 0; /* obj generation may set */
	}
	if (o->trapped) otmp->otrapped = 1;
	if (o->greased) otmp->greased = 1;
#ifdef INVISIBLE_OBJECTS
	if (o->invis) otmp->oinvis = 1;
#endif

	if ((o->quan > 0) && objects[otmp->otyp].oc_merge) {
	    otmp->quan = o->quan;
	    otmp->owt = weight(otmp);
	}

	/* contents */
	if (o->containment & SP_OBJ_CONTENT) {
	    if (!container_idx) {
		if (!invent_carrying_monster)
		    warning("create_object: no container");
		else {
		    int c;
		    struct obj *objcheck = otmp;
		    int inuse = -1;
		    for (c = 0; c < container_idx; c++)
			if (container_obj[c] == objcheck)
			    inuse = c;
		    remove_object(otmp);
		    if (mpickobj(invent_carrying_monster, otmp)) {
			if (inuse > -1) {
			    impossible("container given to monster was merged or deallocated.");
			    for (c = inuse; c < container_idx-1; c++)
				container_obj[c] = container_obj[c+1];
			    container_obj[container_idx] = NULL;
			    container_idx--;
			}
			/* we lost track of it. */
			return;
		    }
		}
	    } else {
		remove_object(otmp);
		(void) add_to_container(container_obj[container_idx-1], otmp);
	    }
	}
	/* container */
	if (o->containment & SP_OBJ_CONTAINER) {
	    delete_contents(otmp);
	    if (container_idx < MAX_CONTAINMENT) {
		container_obj[container_idx] = otmp;
		container_idx++;
	    } else impossible("create_object: too deeply nested containers.");
	}

	/* Medusa level special case: statues are petrified monsters, so they
	 * are not stone-resistant and have monster inventory.  They also lack
	 * other contents, but that can be specified as an empty container.
	 */
	if (o->id == STATUE && Is_medusa_level(&u.uz) &&
		    o->corpsenm == NON_PM) {
	    struct monst *was;
	    struct obj *obj;
	    int wastyp;

	    /* Named random statues are of player types, and aren't stone-
	     * resistant (if they were, we'd have to reset the name as well as
	     * setting corpsenm).
	     */
	    for (wastyp = otmp->corpsenm; ; wastyp = rndmonnum()) {
		/* makemon without rndmonst() might create a group */
		was = makemon(&mons[wastyp], 0, 0, NO_MM_FLAGS);
		if (!resists_ston(was)) break;
		mongone(was);
	    }
	    otmp->corpsenm = wastyp;
	    while(was->minvent) {
		obj = was->minvent;
		obj->owornmask = 0;
		obj_extract_self(obj);
		(void) add_to_container(otmp, obj);
	    }
	    otmp->owt = weight(otmp);
	    mongone(was);
	}

#ifdef RECORD_ACHIEVE
	/* Nasty hack here: try to determine if this is the Mines 
	 * "prize" and then set record_achieve_special (maps to corpsenm)
	 * for the object.  That field will later be checked to find out if
	 * the player obtained the prize. */
	if(otmp->otyp == LUCKSTONE && Is_mineend_level(&u.uz)) {
		otmp->record_achieve_special = 1;
	}
#endif

	stackobj(otmp);

	if (o->lit) {
	    begin_burn(otmp, FALSE);
	}

	if (o->buried && !o->containment) {
	    /* What if we'd want to bury a container? bury_an_obj() may dealloc obj. */
	    (void) bury_an_obj(otmp);
	}
}

/*
 * Randomly place a specific engraving, then release its memory.
 */
STATIC_OVL void
create_engraving(e, croom)
engraving *e;
struct mkroom *croom;
{
	xchar x, y;

	x = e->x,  y = e->y;
	get_location(&x, &y, DRY, croom);

	make_engr_at(x, y, e->engr.str, 0L, e->etype);
}

/*
 * Create stairs in a room.
 *
 */

STATIC_OVL void
create_stairs(s,croom)
stair	*s;
struct mkroom	*croom;
{
	schar		x,y;

	x = s->x; y = s->y;
	get_location(&x, &y, DRY, croom);
	mkstairs(x,y,(char)s->up, croom);
}

/*
 * Create an altar in a room.
 */

STATIC_OVL void
create_altar(a, croom)
	altar		*a;
	struct mkroom	*croom;
{
	schar		sproom,x,y;
	aligntyp	amask;
	boolean		croom_is_temple = TRUE;
	int oldtyp;

	x = a->x; y = a->y;

	if (croom) {
	    get_free_room_loc(&x, &y, croom);
	    if (croom->rtype != TEMPLE)
		croom_is_temple = FALSE;
	} else {
	    get_location(&x, &y, DRY, croom);
	    if ((sproom = (schar) *in_rooms(x, y, TEMPLE)) != 0)
		croom = &rooms[sproom - ROOMOFFSET];
	    else
		croom_is_temple = FALSE;
	}

	/* check for existing features */
	oldtyp = levl[x][y].typ;
	if (oldtyp == STAIRS || oldtyp == LADDER)
	    return;

	a->x = x;
	a->y = y;

	/* Is the alignment random ?
	 * If so, it's an 80% chance that the altar will be co-aligned.
	 *
	 * The alignment is encoded as amask values instead of alignment
	 * values to avoid conflicting with the rest of the encoding,
	 * shared by many other parts of the special level code.
	 */

	amask = (a->align == AM_SPLEV_CO) ?
			Align2amask(u.ualignbase[A_ORIGINAL]) :
		(a->align == AM_SPLEV_NONCO) ?
			Align2amask(noncoalignment(u.ualignbase[A_ORIGINAL])) :
		(a->align == -(MAX_REGISTERS+1)) ? induced_align(80) :
		(a->align < 0 ? ralign[-a->align-1] : a->align);

	levl[x][y].typ = ALTAR;
	levl[x][y].altarmask = amask;

	if (a->shrine < 0) a->shrine = rn2(2);	/* handle random case */

	if (!croom_is_temple || !a->shrine) return;

	if (a->shrine) {	/* Is it a shrine  or sanctum? */
	    priestini(&u.uz, croom, x, y, (a->shrine > 1));
	    levl[x][y].altarmask |= AM_SHRINE;
	    level.flags.has_temple = TRUE;
	}
}

/*
 * Create a gold pile in a room.
 */

STATIC_OVL void
create_gold(g,croom)
gold *g;
struct mkroom	*croom;
{
	schar		x,y;

	x = g->x; y= g->y;
	get_location(&x, &y, DRY, croom);

	if (g->amount == -1)
	    g->amount = rnd(200);
	(void) mkgold((long) g->amount, x, y);
}

/*
 * Create a feature (e.g a fountain) in a room.
 */

STATIC_OVL void
create_feature(fx, fy, croom, typ)
int		fx, fy;
struct mkroom	*croom;
int		typ;
{
	schar		x,y;

	x = fx;  y = fy;
	get_location(&x, &y, DRY, croom);
	/* Don't cover up an existing feature (particularly randomly
	   placed stairs).  However, if the _same_ feature is already
	   here, it came from the map drawing and we still need to
	   update the special counters. */
	if (IS_FURNITURE(levl[x][y].typ) && levl[x][y].typ != typ)
	    return;

	levl[x][y].typ = typ;
}

void
replace_terrain(terr, croom)
replaceterrain *terr;
struct mkroom *croom;
{
    schar x, y, x1, y1, x2, y2;

    if (terr->toter >= MAX_TYPE) return;

    x1 = terr->x1;  y1 = terr->y1;
    get_location(&x1, &y1, DRY|WET, croom);

    x2 = terr->x2;  y2 = terr->y2;
    get_location(&x2, &y2, DRY|WET, croom);

    for (x = x1; x <= x2; x++)
	for (y = y1; y <= y2; y++)
	    if ((levl[x][y].typ == terr->fromter) && (rn2(100) < terr->chance)) {
	    levl[x][y].typ = terr->toter;
	    levl[x][y].lit = terr->tolit;
	}
}


void
put_terr_spot(x,y,ter,lit,thick)
     schar x,y,ter,lit,thick;
{
    int dx, dy;

    if (thick < 1) thick = 1;
    else if (thick > 10) thick = 10;

    for (dx = x-(thick/2); dx < x+((thick+1)/2); dx++)
	for (dy = y-(thick/2); dy < y+((thick+1)/2); dy++)
	    if (!(dx >= COLNO-1 || dx <= 0 || dy <= 0 || dy >= ROWNO-1)) {
		levl[dx][dy].typ = ter;
		levl[dx][dy].lit = lit;
	    }
}

void
line_midpoint_core(x1,y1,x2,y2,rough, ter,lit,thick, rec)
     schar x1,y1,x2,y2,rough,ter,lit,thick,rec;
{
    int mx, my;
    int dx, dy;

    if (rec < 1) {
	return;
    }

    if ((x2 == x1) && (y2 == y1)) {
	put_terr_spot(x1,y1,ter,lit,thick);
	return;
    }

    if (rough > max(abs(x2-x1), abs(y2-y1)))
	rough = max(abs(x2-x1), abs(y2-y1));

    if (rough < 2) {
	mx = ((x1 + x2) / 2);
	my = ((y1 + y2) / 2);
    } else {
	do {
	    dx = (rand() % rough) - (rough / 2);
	    dy = (rand() % rough) - (rough / 2);
	    mx = ((x1 + x2) / 2) + dx;
	    my = ((y1 + y2) / 2) + dy;
	} while ((mx > COLNO-1 || mx < 0 || my < 0 || my > ROWNO-1));
    }

    put_terr_spot(mx,my,ter,lit,thick);

    rough = (rough * 2) / 3;

    rec--;

    line_midpoint_core(x1,y1,mx,my, rough,ter,lit,thick, rec);
    line_midpoint_core(mx,my,x2,y2, rough,ter,lit,thick, rec);
}

void
line_midpoint(rndline,croom)
randline *rndline;
struct mkroom *croom;
{
    schar x1,y1,x2,y2,thick;

    x1 = rndline->x1; y1 = rndline->y1;
    get_location(&x1, &y1, DRY|WET, croom);

    x2 = rndline->x2; y2 = rndline->y2;
    get_location(&x2, &y2, DRY|WET, croom);

    thick = rndline->thick;

    line_midpoint_core(x1,y1,x2,y2,rndline->roughness, rndline->fg, rndline->lit, thick, 12);
}



void
set_terrain(terr, croom)
terrain *terr;
struct mkroom *croom;
{
    schar x, y, x1, y1, x2, y2;

    if (terr->ter >= MAX_TYPE) return;

    x1 = terr->x1;  y1 = terr->y1;
    get_location(&x1, &y1, DRY|WET, croom);

    switch (terr->areatyp) {
    case 0: /* point */
    default:
	levl[x1][y1].typ = terr->ter;
	levl[x1][y1].lit = terr->tlit;
	/* handle doors and secret doors */
	if (levl[x1][y1].typ == SDOOR || IS_DOOR(levl[x1][y1].typ)) {
	    if(levl[x1][y1].typ == SDOOR)
		levl[x1][y1].doormask = D_CLOSED;
	    if (x1 && (IS_WALL(levl[x1-1][y1].typ) ||
				levl[x1-1][y1].horizontal))
		levl[x1][y1].horizontal = 1;
	}
	break;
    case 1: /* horiz line */
	for (x = 0; x < (terr->x2); x++) {
	    levl[x + x1][y1].typ = terr->ter;
	    levl[x + x1][y1].lit = terr->tlit;
	}
	break;
    case 2: /* vert line */
	for (y = 0; y < (terr->y2); y++) {
	    levl[x1][y + y1].typ = terr->ter;
	    levl[x1][y + y1].lit = terr->tlit;
	}
	break;
    case 3: /* filled rectangle */
	x2 = terr->x2;  y2 = terr->y2;
	get_location(&x2, &y2, DRY|WET, croom);
	for (x = x1; x <= x2; x++) {
	    for (y = y1; y <= y2; y++) {
		levl[x][y].typ = terr->ter;
		levl[x][y].lit = terr->tlit;
	    }
	}
	break;
    case 4: /* rectangle */
	x2 = terr->x2;  y2 = terr->y2;
	get_location(&x2, &y2, DRY|WET, croom);
	for (x = x1; x <= x2; x++) {
	    levl[x][y1].typ = terr->ter;
	    levl[x][y1].lit = terr->tlit;
	    levl[x][y2].typ = terr->ter;
	    levl[x][y2].lit = terr->tlit;
	}
	for (y = y1; y <= y2; y++) {
	    levl[x1][y].typ = terr->ter;
	    levl[x1][y].lit = terr->tlit;
	    levl[x2][y].typ = terr->ter;
	    levl[x2][y].lit = terr->tlit;
	}
	break;
    }
}


/*
 * Search for a door in a room on a specified wall.
 */

STATIC_OVL boolean
search_door(croom,x,y,wall,cnt)
struct mkroom *croom;
xchar *x, *y;
xchar wall;
int cnt;
{
	int dx, dy;
	int xx,yy;

	switch(wall) {
	      case W_NORTH:
		dy = 0; dx = 1;
		xx = croom->lx;
		yy = croom->hy + 1;
		break;
	      case W_SOUTH:
		dy = 0; dx = 1;
		xx = croom->lx;
		yy = croom->ly - 1;
		break;
	      case W_EAST:
		dy = 1; dx = 0;
		xx = croom->hx + 1;
		yy = croom->ly;
		break;
	      case W_WEST:
		dy = 1; dx = 0;
		xx = croom->lx - 1;
		yy = croom->ly;
		break;
	      default:
		dx = dy = xx = yy = 0;
		panic("search_door: Bad wall!");
		break;
	}
	while (xx <= croom->hx+1 && yy <= croom->hy+1) {
		if (IS_DOOR(levl[xx][yy].typ) || levl[xx][yy].typ == SDOOR) {
			*x = xx;
			*y = yy;
			if (cnt-- <= 0)
			    return TRUE;
		}
		xx += dx;
		yy += dy;
	}
	return FALSE;
}

/*
 * Dig a corridor between two points.
 */

boolean
dig_corridor(org,dest,nxcor,ftyp,btyp)
coord *org, *dest;
boolean nxcor;
schar ftyp, btyp;
{
	register int dx=0, dy=0, dix, diy, cct;
	register struct rm *crm;
	register int tx, ty, xx, yy;

	xx = org->x;  yy = org->y;
	tx = dest->x; ty = dest->y;
	if (xx <= 0 || yy <= 0 || tx <= 0 || ty <= 0 ||
	    xx > COLNO-1 || tx > COLNO-1 ||
	    yy > ROWNO-1 || ty > ROWNO-1) {
#ifdef DEBUG
		debugpline("dig_corridor: bad coords : (%d,%d) (%d,%d).",
			   xx,yy,tx,ty);
#endif
		return FALSE;
	}
	if (tx > xx)		dx = 1;
	else if (ty > yy)	dy = 1;
	else if (tx < xx)	dx = -1;
	else			dy = -1;

	xx -= dx;
	yy -= dy;
	cct = 0;
	while(xx != tx || yy != ty) {
	    /* loop: dig corridor at [xx,yy] and find new [xx,yy] */
	    if(cct++ > 500 || (nxcor && !rn2(35)))
		return FALSE;

	    xx += dx;
	    yy += dy;

	    if(xx >= COLNO-1 || xx <= 0 || yy <= 0 || yy >= ROWNO-1)
		return FALSE;		/* impossible */

	    crm = &levl[xx][yy];
	    if(crm->typ == btyp) {
		if(ftyp != CORR || rn2(100)) {
			crm->typ = ftyp;
			if(nxcor && !rn2(50))
				(void) mksobj_at(BOULDER, xx, yy, TRUE, FALSE);
		} else {
			crm->typ = SCORR;
		}
	    } else
	    if(crm->typ != ftyp && crm->typ != SCORR) {
		/* strange ... */
		return FALSE;
	    }

	    /* find next corridor position */
	    dix = abs(xx-tx);
	    diy = abs(yy-ty);

	    /* do we have to change direction ? */
	    if(dy && dix > diy) {
		register int ddx = (xx > tx) ? -1 : 1;

		crm = &levl[xx+ddx][yy];
		if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR) {
		    dx = ddx;
		    dy = 0;
		    continue;
		}
	    } else if(dx && diy > dix) {
		register int ddy = (yy > ty) ? -1 : 1;

		crm = &levl[xx][yy+ddy];
		if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR) {
		    dy = ddy;
		    dx = 0;
		    continue;
		}
	    }

	    /* continue straight on? */
	    crm = &levl[xx+dx][yy+dy];
	    if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR)
		continue;

	    /* no, what must we do now?? */
	    if(dx) {
		dx = 0;
		dy = (ty < yy) ? -1 : 1;
	    } else {
		dy = 0;
		dx = (tx < xx) ? -1 : 1;
	    }
	    crm = &levl[xx+dx][yy+dy];
	    if(crm->typ == btyp || crm->typ == ftyp || crm->typ == SCORR)
		continue;
	    dy = -dy;
	    dx = -dx;
	}
	return TRUE;
}

/*
 * Disgusting hack: since special levels have their rooms filled before
 * sorting the rooms, we have to re-arrange the speed values upstairs_room
 * and dnstairs_room after the rooms have been sorted.  On normal levels,
 * stairs don't get created until _after_ sorting takes place.
 */
STATIC_OVL void
fix_stair_rooms()
{
    int i;
    struct mkroom *croom;

    if(xdnstair &&
       !((dnstairs_room->lx <= xdnstair && xdnstair <= dnstairs_room->hx) &&
	 (dnstairs_room->ly <= ydnstair && ydnstair <= dnstairs_room->hy))) {
	for(i=0; i < nroom; i++) {
	    croom = &rooms[i];
	    if((croom->lx <= xdnstair && xdnstair <= croom->hx) &&
	       (croom->ly <= ydnstair && ydnstair <= croom->hy)) {
		dnstairs_room = croom;
		break;
	    }
	}
	if(i == nroom)
	    panic("Couldn't find dnstair room in fix_stair_rooms!");
    }
    if(xupstair &&
       !((upstairs_room->lx <= xupstair && xupstair <= upstairs_room->hx) &&
	 (upstairs_room->ly <= yupstair && yupstair <= upstairs_room->hy))) {
	for(i=0; i < nroom; i++) {
	    croom = &rooms[i];
	    if((croom->lx <= xupstair && xupstair <= croom->hx) &&
	       (croom->ly <= yupstair && yupstair <= croom->hy)) {
		upstairs_room = croom;
		break;
	    }
	}
	if(i == nroom)
	    panic("Couldn't find upstair room in fix_stair_rooms!");
    }
}

/*
 * Corridors always start from a door. But it can end anywhere...
 * Basically we search for door coordinates or for endpoints coordinates
 * (from a distance).
 */

STATIC_OVL void
create_corridor(c)
corridor	*c;
{
	coord org, dest;

	if (c->src.room == -1) {
	    /*sort_rooms();*/
		fix_stair_rooms();
		makecorridors(c->src.door);
		return;
	}

	if( !search_door(&rooms[c->src.room], &org.x, &org.y, c->src.wall,
			 c->src.door))
	    return;

	if (c->dest.room != -1) {
		if(!search_door(&rooms[c->dest.room], &dest.x, &dest.y,
				c->dest.wall, c->dest.door))
		    return;
		switch(c->src.wall) {
		      case W_NORTH: org.y--; break;
		      case W_SOUTH: org.y++; break;
		      case W_WEST:  org.x--; break;
		      case W_EAST:  org.x++; break;
		}
		switch(c->dest.wall) {
		      case W_NORTH: dest.y--; break;
		      case W_SOUTH: dest.y++; break;
		      case W_WEST:  dest.x--; break;
		      case W_EAST:  dest.x++; break;
		}
		(void) dig_corridor(&org, &dest, FALSE, CORR, STONE);
	}
}


/*
 * Fill a room (shop, zoo, etc...) with appropriate stuff.
 */

void
fill_room(croom, prefilled)
struct mkroom *croom;
boolean prefilled;
{
	if (!croom || croom->rtype == OROOM)
	    return;

	if (!prefilled) {
	    int x,y;

	    /* Shop ? */
	    if (croom->rtype >= SHOPBASE) {
		    stock_room(croom->rtype - SHOPBASE, croom);
		    level.flags.has_shop = TRUE;
		    return;
	    }

	    switch (croom->rtype) {
		case VAULT:
		    for (x=croom->lx;x<=croom->hx;x++)
			for (y=croom->ly;y<=croom->hy;y++)
			    (void) mkgold((long)rn1(abs(depth(&u.uz))*100, 51), x, y);
		    break;
		case GARDEN:
		case COURT:
		case ZOO:
		case BEEHIVE:
		case LEMUREPIT:
		case MORGUE:
		case BARRACKS:
		    fill_zoo(croom);
		    break;
	    }
	}
	switch (croom->rtype) {
	    case VAULT:
		level.flags.has_vault = TRUE;
		break;
	    case ZOO:
		level.flags.has_zoo = TRUE;
		break;
	    case GARDEN:
		level.flags.has_garden = TRUE;
	    case COURT:
		level.flags.has_court = TRUE;
		break;
	    case MORGUE:
		level.flags.has_morgue = TRUE;
		break;
	    case BEEHIVE:
		level.flags.has_beehive = TRUE;
		break;
	    case LEMUREPIT:
		level.flags.has_lemurepit = TRUE;
		break;
	    case BARRACKS:
		level.flags.has_barracks = TRUE;
		break;
	    case TEMPLE:
		level.flags.has_temple = TRUE;
		break;
	    case SWAMP:
		level.flags.has_swamp = TRUE;
		break;
	}
}

STATIC_OVL struct mkroom *
build_room(r, mkr)
room *r;
struct mkroom *mkr;
{
	boolean okroom;
	struct mkroom	*aroom;
	xchar rtype = (!r->chance || rn2(100) < r->chance) ? r->rtype : OROOM;

	if(mkr) {
		aroom = &subrooms[nsubroom];
		okroom = create_subroom(mkr, r->x, r->y, r->w, r->h,
					rtype, r->rlit);
	} else {
		aroom = &rooms[nroom];
		okroom = create_room(r->x, r->y, r->w, r->h, r->xalign,
				     r->yalign, rtype, r->rlit);
	}

	if (okroom) {
#ifdef SPECIALIZATION
		topologize(aroom,FALSE);		/* set roomno */
#else
		topologize(aroom);			/* set roomno */
#endif
		aroom->needfill = ((aroom->rtype != OROOM) && r->filled);
	        return aroom;
	}
	return (struct mkroom *)0;
}

/*
 * set lighting in a region that will not become a room.
 */
STATIC_OVL void
light_region(tmpregion)
    region  *tmpregion;
{
    register boolean litstate = tmpregion->rlit ? 1 : 0;
    register int hiy = tmpregion->y2;
    register int x, y;
    register struct rm *lev;
    int lowy = tmpregion->y1;
    int lowx = tmpregion->x1, hix = tmpregion->x2;

    if(litstate) {
	/* adjust region size for walls, but only if lighted */
	lowx = max(lowx-1,1);
	hix = min(hix+1,COLNO-1);
	lowy = max(lowy-1,0);
	hiy = min(hiy+1, ROWNO-1);
    }
    for(x = lowx; x <= hix; x++) {
	lev = &levl[x][lowy];
	for(y = lowy; y <= hiy; y++) {
	    if (lev->typ != LAVAPOOL) /* this overrides normal lighting */
		lev->lit = litstate;
	    lev++;
	}
    }
}

STATIC_OVL void
load_one_monster(fd, m)
dlb *fd;
monster *m;
{
	int size;

	Fread((genericptr_t) m, 1, sizeof *m, fd);
	if ((size = m->name.len) != 0) {
	    m->name.str = (char *) alloc((unsigned)size + 1);
	    Fread((genericptr_t) m->name.str, 1, size, fd);
	    m->name.str[size] = '\0';
	} else
	    m->name.str = (char *) 0;
	if ((size = m->appear_as.len) != 0) {
	    m->appear_as.str = (char *) alloc((unsigned)size + 1);
	    Fread((genericptr_t) m->appear_as.str, 1, size, fd);
	    m->appear_as.str[size] = '\0';
	} else
	    m->appear_as.str = (char *) 0;
}

STATIC_OVL void
load_one_object(fd, o)
dlb *fd;
object *o;
{
	int size;

	Fread((genericptr_t) o, 1, sizeof *o, fd);
	if ((size = o->name.len) != 0) {
	    o->name.str = (char *) alloc((unsigned)size + 1);
	    Fread((genericptr_t) o->name.str, 1, size, fd);
	    o->name.str[size] = '\0';
	} else
	    o->name.str = (char *) 0;
}

STATIC_OVL void
load_one_engraving(fd, e)
dlb *fd;
engraving *e;
{
	int size;

	Fread((genericptr_t) e, 1, sizeof *e, fd);
	size = e->engr.len;
	e->engr.str = (char *) alloc((unsigned)size+1);
	Fread((genericptr_t) e->engr.str, 1, size, fd);
	e->engr.str[size] = '\0';
}

STATIC_OVL void
load_one_room(fd, r)
dlb *fd;
room *r;
{
    int size;

    Fread((genericptr_t) r, 1, sizeof *r, fd);
    size = r->name.len;
    if (size > 0) {
	r->name.str = (char *) alloc((unsigned)size + 1);
	Fread((genericptr_t) r->name.str, 1, size, fd);
	r->name.str[size] = '\0';
    }
    size = r->parent.len;
    if (size > 0) {
	r->parent.str = (char *) alloc((unsigned)size + 1);
	Fread((genericptr_t) r->parent.str, 1, size, fd);
	r->parent.str[size] = '\0';
    }
}

void
wallify_map(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
	int x, y, xx, yy, lo_xx, lo_yy, hi_xx, hi_yy;

	for (y = y1; y <= y2; y++) {
	    lo_yy = (y > 0) ? y - 1 : 0;
	    hi_yy = (y < y2) ? y + 1 : y2;
	    for (x = x1; x <= x2; x++) {
		if (levl[x][y].typ != STONE) continue;
		lo_xx = (x > 0) ? x - 1 : 0;
		hi_xx = (x < x2) ? x + 1 : x2;
		for (yy = lo_yy; yy <= hi_yy; yy++)
		    for (xx = lo_xx; xx <= hi_xx; xx++)
			if (IS_ROOM(levl[xx][yy].typ) ||
				levl[xx][yy].typ == CROSSWALL) {
			    levl[x][y].typ = (yy != y) ? HWALL : VWALL;
			    yy = hi_yy;		/* end `yy' loop */
			    break;		/* end `xx' loop */
			}
	    }
	}
}

/*
 * Select a random coordinate in the maze.
 *
 * We want a place not 'touched' by the loader.  That is, a place in
 * the maze outside every part of the special level.
 */

STATIC_OVL void
maze1xy(m, humidity)
coord *m;
int humidity;
{
	register int x, y, tryct = 2000;
	/* tryct:  normally it won't take more than ten or so tries due
	   to the circumstances under which we'll be called, but the
	   `humidity' screening might drastically change the chances */

	do {
	    x = rn1(x_maze_max - 3, 3);
	    y = rn1(y_maze_max - 3, 3);
	    if (--tryct < 0) break;	/* give up */
	} while (!(x % 2) || !(y % 2) || !SpLev_Map[x][y] ||
		 !is_ok_location((schar)x, (schar)y, humidity));

	m->x = (xchar)x,  m->y = (xchar)y;
}

/*
 * If there's a significant portion of maze unused by the special level,
 * we don't want it empty.
 *
 * Makes the number of traps, monsters, etc. proportional
 * to the size of the maze.
 */
STATIC_OVL void
fill_empty_maze()
{
    int mapcountmax, mapcount, mapfact;
    xchar x,y;
    coord mm;

    mapcountmax = mapcount = (x_maze_max - 2) * (y_maze_max - 2);
    mapcountmax = mapcountmax / 2;

    for(x = 2; x < x_maze_max; x++)
	for(y = 0; y < y_maze_max; y++)
	    if(!SpLev_Map[x][y]) mapcount--;

    if ((mapcount > (int) (mapcountmax / 10))) {
	    mapfact = (int) ((mapcount * 100L) / mapcountmax);
	    for(x = rnd((int) (20 * mapfact) / 100); x; x--) {
		    maze1xy(&mm, DRY);
		    (void) mkobj_at(rn2(2) ? GEM_CLASS : RANDOM_CLASS,
							mm.x, mm.y, TRUE);
	    }
	    for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
		    maze1xy(&mm, DRY);
		    (void) mksobj_at(BOULDER, mm.x, mm.y, TRUE, FALSE);
	    }
	    for (x = rn2(2); x; x--) {
		maze1xy(&mm, DRY);
		(void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y, NO_MM_FLAGS);
	    }
	    for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
		    maze1xy(&mm, WET|DRY);
		    (void) makemon((struct permonst *) 0, mm.x, mm.y, NO_MM_FLAGS);
	    }
	    for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
		    maze1xy(&mm, DRY);
		    (void) mkgold(0L,mm.x,mm.y);
	    }
	    for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
		    int trytrap;

		    maze1xy(&mm, DRY);
		    trytrap = rndtrap();
		    if (sobj_at(BOULDER, mm.x, mm.y))
			while (trytrap == PIT || trytrap == SPIKED_PIT ||
				trytrap == TRAPDOOR || trytrap == HOLE)
			    trytrap = rndtrap();
		    (void) maketrap(mm.x, mm.y, trytrap);
	    }
    }
}

/*
 * special level loader
 */
STATIC_OVL boolean
sp_level_loader(fd, lvl)
dlb *fd;
sp_lev *lvl;
{
    long n_opcode = 0;
    struct opvar *opdat;
    int opcode;

    Fread((genericptr_t)&(lvl->n_opcodes), 1, sizeof(lvl->n_opcodes), fd);

    lvl->opcodes = (_opcode *)alloc(sizeof(_opcode) * (lvl->n_opcodes));

    while (n_opcode < lvl->n_opcodes) {

	Fread((genericptr_t) &lvl->opcodes[n_opcode].opcode, 1,
	      sizeof(lvl->opcodes[n_opcode].opcode), fd);
	opcode = lvl->opcodes[n_opcode].opcode;

	opdat = NULL;

	if (opcode < SPO_NULL || opcode >= MAX_SP_OPCODES)
	    panic("sp_level_loader: impossible opcode %i.", opcode);

	if (opcode == SPO_PUSH) {
	    struct opvar *ov = (opdat = (struct opvar *)alloc(sizeof(struct opvar)));
	    int nsize;

	    ov->spovartyp = SPO_NULL;
	    ov->vardata.l = 0;
	    Fread((genericptr_t)&(ov->spovartyp), 1, sizeof(ov->spovartyp), fd);

	    switch (ov->spovartyp) {
	    case SPOVAR_NULL: break;
	    case SPOVAR_INT:
		Fread((genericptr_t)&(ov->vardata.l), 1, sizeof(ov->vardata.l), fd);
		break;
	    case SPOVAR_STRING:
		{
		    char *opd;
		    Fread((genericptr_t) &nsize, 1, sizeof(nsize), fd);
		    opd = (char *)alloc(nsize + 1);
		    if (nsize) Fread(opd, 1, nsize, fd);
		    opd[nsize] = 0;
		    ov->vardata.str = opd;
		}
		break;
	    default:
		panic("sp_level_loader: unknown opvar type %i", ov->spovartyp);
	    }
	}

	lvl->opcodes[n_opcode].opdat = opdat;
	n_opcode++;
    } /*while*/

    return TRUE;
}


/* Frees the memory allocated for special level creation structs */
STATIC_OVL boolean
sp_level_free(lvl)
sp_lev *lvl;
{
    long n_opcode = 0;

    while (n_opcode < lvl->n_opcodes) {
	int opcode = lvl->opcodes[n_opcode].opcode;
	struct opvar *opdat = lvl->opcodes[n_opcode].opdat;

	if (opcode < SPO_NULL || opcode >= MAX_SP_OPCODES)
	    panic("sp_level_free: unknown opcode %i", opcode);

	if (opdat) opvar_free(opdat);
	n_opcode++;
    }
    free(lvl->opcodes);
    return TRUE;
}

void
splev_initlev(linit)
lev_init *linit;
{
    switch (linit->init_style) {
    default: impossible("Unrecognized level init style."); break;
    case LVLINIT_NONE: break;
    case LVLINIT_SOLIDFILL:
	if (linit->lit < 0) linit->lit = rn2(2);
	lvlfill_solid(linit->filling, linit->lit);
	break;
    case LVLINIT_MAZEGRID:
	lvlfill_maze_grid(2,0, x_maze_max,y_maze_max, linit->filling);
	break;
    case LVLINIT_MINES:
	if (linit->lit < 0) linit->lit = rn2(2);
	if (linit->filling > -1) lvlfill_solid(linit->filling);
	mkmap(linit);
	break;
    }
}

struct sp_frame *
frame_new(execptr)
     long execptr;
{
    struct sp_frame *frame = (struct sp_frame *)alloc(sizeof(struct sp_frame));
    if (!frame) panic("could not create execution frame.");
    frame->next = NULL;
    frame->n_opcode = execptr;
    frame->stack = (struct splevstack *)alloc(sizeof(struct splevstack));
    if (!frame->stack) panic("could not create execution frame stack.");
    splev_stack_init(frame->stack);
    return frame;
}

void
frame_del(frame)
     struct sp_frame *frame;
{
    if (!frame) return;
    if (frame->stack) {
	splev_stack_done(frame->stack);
	frame->stack = NULL;
    }
    Free(frame);
}


void
spo_frame_push(coder)
     struct sp_coder *coder;
{
    struct sp_frame *tmpframe = frame_new(coder->frame->n_opcode);
    tmpframe->next = coder->frame;
    coder->frame = tmpframe;
}

void
spo_frame_pop(coder)
     struct sp_coder *coder;
{
    if (coder->frame->next) {
	struct sp_frame *tmpframe = coder->frame->next;
	frame_del(coder->frame);
	coder->frame = tmpframe;
    }
}

long
sp_code_jmpaddr(curpos, jmpaddr)
     long curpos, jmpaddr;
{
    return (curpos + jmpaddr);
}

void
spo_call(coder)
     struct sp_coder *coder;
{
    struct opvar *addr;
    struct opvar *params;
    struct sp_frame *tmpframe;

    if (!OV_pop_i(addr) || !OV_pop_i(params)) return;
    if (OV_i(params) < 0) return;

    /* push a frame */
    tmpframe = frame_new(sp_code_jmpaddr(coder->frame->n_opcode, OV_i(addr)-1));
    tmpframe->next = coder->frame;
    coder->frame = tmpframe;

    while (OV_i(params)-- > 0) {
	splev_stack_push(tmpframe->stack, splev_stack_pop(coder->stack));
    }
    opvar_free(addr);
    opvar_free(params);
}

void
spo_return(coder)
     struct sp_coder *coder;
{
    struct opvar *params;
    if (!coder->frame->next) return; /* something is seriously wrong */
    if (!OV_pop_i(params)) return;
    if (OV_i(params) < 0) return;

    while (OV_i(params)-- > 0) {
	splev_stack_push(coder->frame->next->stack, splev_stack_pop(coder->stack));
    }

    /* pop the frame */
    if (coder->frame->next) {
	struct sp_frame *tmpframe = coder->frame->next;
	frame_del(coder->frame);
	coder->frame = tmpframe;
	coder->stack = coder->frame->stack;
    }

    opvar_free(params);
}

void
spo_end_moninvent(coder)
     struct sp_coder *coder;
{
    if (invent_carrying_monster)
	m_dowear(invent_carrying_monster, TRUE);
    invent_carrying_monster = NULL;
}

void
spo_pop_container(coder)
     struct sp_coder *coder;
{
    if (container_idx > 0) {
	container_idx--;
	container_obj[container_idx] = NULL;
    }
}


void
spo_message(coder)
     struct sp_coder *coder;
{
    struct opvar *op;
    char *msg, *levmsg;
    int old_n, n;
    if (!OV_pop_s(op)) return;
    msg = string_subst(OV_s(op));
    if (!msg) return;

    old_n = lev_message ? (strlen(lev_message)+1) : 0;
    n = strlen(msg);

    levmsg = (char *) alloc(old_n+n+1);
    if (old_n) levmsg[old_n-1] = '\n';
    if (lev_message)
	(void) memcpy((genericptr_t)levmsg, (genericptr_t)lev_message, old_n-1);
    (void) memcpy((genericptr_t)&levmsg[old_n], msg, n);
    levmsg[old_n+n] = '\0';
    Free(lev_message);
    lev_message = levmsg;
    opvar_free(op);
}

void
spo_monster(coder)
     struct sp_coder *coder;
{
    int nparams = 0;

    struct opvar *varparam;
    struct opvar *id, *x, *y, *class, *has_inv;
    monster tmpmons;

    tmpmons.peaceful = -1;
    tmpmons.asleep = -1;
    tmpmons.name.str = (char *)0;
    tmpmons.appear = 0;
    tmpmons.appear_as.str = (char *)0;
    tmpmons.align = - MAX_REGISTERS - 2;
    tmpmons.female = 0;
    tmpmons.invis = 0;
    tmpmons.cancelled = 0;
    tmpmons.revived = 0;
    tmpmons.avenge = 0;
    tmpmons.fleeing = 0;
    tmpmons.blinded = 0;
    tmpmons.paralyzed = 0;
    tmpmons.stunned = 0;
    tmpmons.confused = 0;
    tmpmons.seentraps = 0;
    tmpmons.has_invent = 0;

    if (!OV_pop_i(has_inv) ||
	!OV_pop_i(id) ||
	!OV_pop_i(class) ||
	!OV_pop_i(y) ||
	!OV_pop_i(x)) return;

    if (!OV_pop_i(varparam)) return;

    while ((nparams++ < (SP_M_V_END+1)) &&
	   (OV_typ(varparam) == SPOVAR_INT) &&
	   (OV_i(varparam) >= 0) &&
	   (OV_i(varparam) < SP_M_V_END)) {
	struct opvar *parm = splev_stack_pop(coder->stack);
	switch (OV_i(varparam)) {
	case SP_M_V_NAME:
	    if ((OV_typ(parm) == SPOVAR_STRING) &&
		!tmpmons.name.str)
		tmpmons.name.str = strdup(OV_s(parm));
	    opvar_free(parm);
	    break;
	case SP_M_V_APPEAR:
	    if ((OV_typ(parm) == SPOVAR_INT) &&
		!tmpmons.appear_as.str) {
		tmpmons.appear = OV_i(parm);
		opvar_free(parm);
		parm = splev_stack_pop(coder->stack);
		tmpmons.appear_as.str = strdup(OV_s(parm));
	    }
	    break;
	case SP_M_V_ASLEEP:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.asleep = OV_i(parm);
	    break;
	case SP_M_V_ALIGN:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.align = OV_i(parm);
	    break;
	case SP_M_V_PEACEFUL:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.peaceful = OV_i(parm);
	    break;
	case SP_M_V_FEMALE:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.female = OV_i(parm);
	    break;
	case SP_M_V_INVIS:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.invis = OV_i(parm);
	    break;
	case SP_M_V_CANCELLED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.cancelled = OV_i(parm);
	    break;
	case SP_M_V_REVIVED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.revived = OV_i(parm);
	    break;
	case SP_M_V_AVENGE:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.avenge = OV_i(parm);
	    break;
	case SP_M_V_FLEEING:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.fleeing = OV_i(parm);
	    break;
	case SP_M_V_BLINDED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.blinded = OV_i(parm);
	    break;
	case SP_M_V_PARALYZED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.paralyzed = OV_i(parm);
	    break;
	case SP_M_V_STUNNED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.stunned = OV_i(parm);
	    break;
	case SP_M_V_CONFUSED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.confused = OV_i(parm);
	    break;
	case SP_M_V_SEENTRAPS:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpmons.seentraps = OV_i(parm);
	    break;
	case SP_M_V_END:
	    nparams = SP_M_V_END+1;
	    break;
	default:
	    impossible("MONSTER with unknown variable param type!");
	    break;
	}
	opvar_free(parm);
	if (OV_i(varparam) != SP_M_V_END) {
	    opvar_free(varparam);
	    varparam = splev_stack_pop(coder->stack);
	}
    }

    tmpmons.id = OV_i(id);
    tmpmons.x = OV_i(x);
    tmpmons.y = OV_i(y);
    tmpmons.class = OV_i(class);
    tmpmons.has_invent = OV_i(has_inv);

    create_monster(&tmpmons, coder->croom);

    free(tmpmons.name.str);
    free(tmpmons.appear_as.str);

    opvar_free(id);
    opvar_free(x);
    opvar_free(y);
    opvar_free(class);
    opvar_free(has_inv);
    opvar_free(varparam);
}

void
spo_object(coder)
     struct sp_coder *coder;
{
    int nparams = 0;

    struct opvar *varparam;
    struct opvar *id, *x, *y, *class, *containment;

    object tmpobj;

    tmpobj.spe = -127;
    tmpobj.curse_state = -1;
    tmpobj.corpsenm = NON_PM;
    tmpobj.name.str = (char *)0;
    tmpobj.quan = -1;
    tmpobj.buried = 0;
    tmpobj.lit = 0;
    tmpobj.eroded = 0;
    tmpobj.locked = 0;
    tmpobj.trapped = 0;
    tmpobj.recharged = 0;
    tmpobj.invis = 0;
    tmpobj.greased = 0;
    tmpobj.broken = 0;

    if (!OV_pop_i(containment) ||
	!OV_pop_i(id) ||
	!OV_pop_i(class) ||
	!OV_pop_i(y) ||
	!OV_pop_i(x)) return;

    if (!OV_pop_i(varparam)) return;

    while ((nparams++ < (SP_O_V_END+1)) &&
	   (OV_typ(varparam) == SPOVAR_INT) &&
	   (OV_i(varparam) >= 0) &&
	   (OV_i(varparam) < SP_O_V_END)) {
	struct opvar *parm = splev_stack_pop(coder->stack);
	switch (OV_i(varparam)) {
	case SP_O_V_NAME:
	    if ((OV_typ(parm) == SPOVAR_STRING) &&
		!tmpobj.name.str)
		tmpobj.name.str = strdup(OV_s(parm));
	    break;
	case SP_O_V_CORPSENM:
	    if ((OV_typ(parm) == SPOVAR_INT))
		tmpobj.corpsenm = OV_i(parm);
	    break;
	case SP_O_V_CURSE:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.curse_state = OV_i(parm);
	    break;
	case SP_O_V_SPE:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.spe = OV_i(parm);
	    break;
	case SP_O_V_QUAN:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.quan = OV_i(parm);
	    break;
	case SP_O_V_BURIED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.buried = OV_i(parm);
	    break;
	case SP_O_V_LIT:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.lit = OV_i(parm);
	    break;
	case SP_O_V_ERODED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.eroded = OV_i(parm);
	    break;
	case SP_O_V_LOCKED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.locked = OV_i(parm);
	    break;
	case SP_O_V_TRAPPED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.trapped = OV_i(parm);
	    break;
	case SP_O_V_RECHARGED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.recharged = OV_i(parm);
	    break;
	case SP_O_V_INVIS:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.invis = OV_i(parm);
	    break;
	case SP_O_V_GREASED:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.greased = OV_i(parm);
	    break;
	case SP_O_V_BROKEN:
	    if (OV_typ(parm) == SPOVAR_INT)
		tmpobj.broken = OV_i(parm);
	    break;
	case SP_O_V_END:
	    nparams = SP_O_V_END+1;
	    break;
	default:
	    impossible("OBJECT with unknown variable param type!");
	    break;
	}
	opvar_free(parm);
	if (OV_i(varparam) != SP_O_V_END) {
	    opvar_free(varparam);
	    varparam = splev_stack_pop(coder->stack);
	}
    }

    tmpobj.id = OV_i(id);
    tmpobj.x = OV_i(x);
    tmpobj.y = OV_i(y);
    tmpobj.class = OV_i(class);
    tmpobj.containment = OV_i(containment);

    create_object(&tmpobj, coder->croom);

    Free(tmpobj.name.str);

    opvar_free(varparam);
    opvar_free(id);
    opvar_free(x);
    opvar_free(y);
    opvar_free(class);
    opvar_free(containment);
}

void
spo_level_flags(coder)
     struct sp_coder *coder;
{
    struct opvar *flagdata;
    long flags;

    if (!OV_pop_i(flagdata)) return;
    flags = OV_i(flagdata);

    if (flags & NOTELEPORT)   level.flags.noteleport = 1;
    if (flags & HARDFLOOR)    level.flags.hardfloor = 1;
    if (flags & NOMMAP)       level.flags.nommap = 1;
    if (flags & SHORTSIGHTED) level.flags.shortsighted = 1;
    if (flags & ARBOREAL)     level.flags.arboreal = 1;
    if (flags & NOFLIPX)      coder->allow_flips &= ~1;
    if (flags & NOFLIPY)      coder->allow_flips &= ~2;
    if (flags & MAZELEVEL)    level.flags.is_maze_lev = 1;
    if (flags & PREMAPPED)    coder->premapped = TRUE;
    if (flags & SHROUD)       level.flags.hero_memory = 0;
    if (flags & STORMY)       level.flags.stormy = 1;
    if (flags & GRAVEYARD)    level.flags.graveyard = 1;

    opvar_free(flagdata);
}

void
spo_initlevel(coder)
     struct sp_coder *coder;
{
    lev_init init_lev;
    struct opvar *init_style, *fg, *bg, *smoothed, *joined, *lit, *walled, *filling;

    if (!OV_pop_i(fg) ||
	!OV_pop_i(bg) ||
	!OV_pop_i(smoothed) ||
	!OV_pop_i(joined) ||
	!OV_pop_i(lit) ||
	!OV_pop_i(walled) ||
	!OV_pop_i(filling) ||
	!OV_pop_i(init_style)) return;

    init_lev.init_style = OV_i(init_style);
    init_lev.fg = OV_i(fg);
    init_lev.bg = OV_i(bg);
    init_lev.smoothed = OV_i(smoothed);
    init_lev.joined = OV_i(joined);
    init_lev.lit = OV_i(lit);
    init_lev.walled = OV_i(walled);
    init_lev.filling = OV_i(filling);

    coder->lvl_is_joined = OV_i(joined);

    splev_initlev(&init_lev);

    opvar_free(init_style);
    opvar_free(fg);
    opvar_free(bg);
    opvar_free(smoothed);
    opvar_free(joined);
    opvar_free(lit);
    opvar_free(walled);
    opvar_free(filling);
}

void
spo_mon_generation(coder)
     struct sp_coder *coder;
{
    struct opvar *freq, *n_tuples;
    struct mon_gen_override *mg;
    struct mon_gen_tuple *mgtuple;

    if (level.mon_gen) {
	impossible("monster generation override already defined.");
	return;
    }

    if (!OV_pop_i(n_tuples) ||
	!OV_pop_i(freq)) return;

    mg = (struct mon_gen_override *)alloc(sizeof(struct mon_gen_override));
    mg->override_chance = OV_i(freq);
    mg->total_mon_freq = 0;
    mg->gen_chances = NULL;
    while (OV_i(n_tuples)-- > 0) {
	struct opvar *mfreq, *is_sym, *mon;
	mgtuple = (struct mon_gen_tuple *)alloc(sizeof(struct mon_gen_tuple));

	if (!OV_pop_i(mfreq) ||
	    !OV_pop_i(is_sym) ||
	    !OV_pop_i(mon)) {
	    panic("oopsie when loading mon_gen chances.");
	}

	mgtuple->freq = OV_i(mfreq);
	mgtuple->is_sym = OV_i(is_sym);
	if (OV_i(is_sym))
	    mgtuple->monid = def_char_to_monclass(OV_i(mon));
	else mgtuple->monid = OV_i(mon);
	mgtuple->next = mg->gen_chances;
	mg->gen_chances = mgtuple;
	mg->total_mon_freq += OV_i(mfreq);
	opvar_free(mfreq);
	opvar_free(is_sym);
	opvar_free(mon);
    }
    level.mon_gen = mg;
    opvar_free(freq);
    opvar_free(n_tuples);
}

void
spo_level_sounds(coder)
     struct sp_coder *coder;
{
    struct opvar *freq, *n_tuples;
    struct lvl_sounds *mg;
    struct lvl_sound_bite *mgtuple;

    if (level.sounds) {
	impossible("level sounds already defined.");
	return;
    }

    if (!OV_pop_i(n_tuples) || !OV_pop_i(freq)) return;

    if (OV_i(n_tuples) < 1) {
	impossible("no level sounds attached to the sound opcode?");
    }

    mg = (struct lvl_sounds *)alloc(sizeof(struct lvl_sounds));
    mg->freq = OV_i(freq);
    mg->n_sounds = OV_i(n_tuples);
    mg->sounds = (struct lvl_sound_bite *)alloc(sizeof(struct lvl_sound_bite) * mg->n_sounds);

    while (OV_i(n_tuples)-- > 0) {
	struct opvar *flags, *msg;

	if (!OV_pop_s(msg) || !OV_pop_i(flags)) {
	    panic("oopsie when loading lvl_sound_bite.");
	}

	mg->sounds[OV_i(n_tuples)].flags = OV_i(flags);
	mg->sounds[OV_i(n_tuples)].msg = strdup(OV_s(msg));

	opvar_free(flags);
	opvar_free(msg);
    }
    level.sounds = mg;

    opvar_free(freq);
    opvar_free(n_tuples);
}

void
spo_engraving(coder)
     struct sp_coder *coder;
{
    struct opvar *etyp, *txt, *fy, *fx;
    engraving tmpe;

    if (!OV_pop_i(etyp) ||
	!OV_pop_s(txt) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    tmpe.x = OV_i(fx);
    tmpe.y = OV_i(fy);
    tmpe.engr.str = OV_s(txt);
    tmpe.etype = OV_i(etyp);

    create_engraving(&tmpe, coder->croom);

    opvar_free(etyp);
    opvar_free(txt);
    opvar_free(fx);
    opvar_free(fy);
}

void
spo_room(coder)
     struct sp_coder *coder;
{
    if (coder->n_subroom > MAX_NESTED_ROOMS)
	panic("Too deeply nested rooms?!");
    else {
	struct opvar *filled, *h, *w, *yalign, *xalign,
	    *y, *x, *rlit, *chance, *rtype;

	room tmproom;
	struct mkroom *tmpcr;

	if (!OV_pop_i(h) ||
	    !OV_pop_i(w) ||
	    !OV_pop_i(y) ||
	    !OV_pop_i(x) ||
	    !OV_pop_i(yalign) ||
	    !OV_pop_i(xalign) ||
	    !OV_pop_i(filled) ||
	    !OV_pop_i(rlit) ||
	    !OV_pop_i(chance) ||
	    !OV_pop_i(rtype)) return;


	tmproom.x = OV_i(x);
	tmproom.y = OV_i(y);
	tmproom.w = OV_i(w);
	tmproom.h = OV_i(h);
	tmproom.xalign = OV_i(xalign);
	tmproom.yalign = OV_i(yalign);
	tmproom.rtype = OV_i(rtype);
	tmproom.chance = OV_i(chance);
	tmproom.rlit = OV_i(rlit);
	tmproom.filled = OV_i(filled);

	opvar_free(x);
	opvar_free(y);
	opvar_free(w);
	opvar_free(h);
	opvar_free(xalign);
	opvar_free(yalign);
	opvar_free(rtype);
	opvar_free(chance);
	opvar_free(rlit);
	opvar_free(filled);

	if (!coder->failed_room[coder->n_subroom-1]) {
	    tmpcr = build_room(&tmproom, coder->croom);
	    if (tmpcr) {
		coder->tmproomlist[coder->n_subroom] = tmpcr;
		coder->failed_room[coder->n_subroom] = FALSE;
		coder->n_subroom++;
		return;
	    }
	} /* failed to create parent room, so fail this too */
    }
    coder->tmproomlist[coder->n_subroom] = (struct mkroom *)0;
    coder->failed_room[coder->n_subroom] = TRUE;
    coder->n_subroom++;
}

void
spo_endroom(coder)
     struct sp_coder *coder;
{
    if (coder->n_subroom > 1) {
	coder->n_subroom--;
    } else {
	/* no subroom, get out of top-level room */
	/* Need to ensure xstart/ystart/xsize/ysize have something sensible,
	   in case there's some stuff to be created outside the outermost room,
	   and there's no MAP.
	*/
	if(xsize <= 1 && ysize <= 1) {
	    xstart = 1;
	    ystart = 0;
	    xsize = COLNO-1;
	    ysize = ROWNO;
	}
    }
}

void
spo_door(coder)
     struct sp_coder *coder;
{
    schar x,y;
    struct opvar *msk, *fy, *fx;
    struct mkroom *droom;
    xchar typ;

    if (!OV_pop_i(msk) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    droom = &rooms[0];

    x = OV_i(fx); y = OV_i(fy);
    typ = OV_i(msk) == -1 ? rnddoor() : (xchar)OV_i(msk);

    get_location(&x, &y, DRY, (struct mkroom *)0);
    if (levl[x][y].typ != SDOOR)
	levl[x][y].typ = DOOR;
    else {
	if(typ < D_CLOSED)
	    typ = D_CLOSED; /* force it to be closed */
    }
    levl[x][y].doormask = typ;
    /*SpLev_Map[x][y] = 1;*/

    /* Now the complicated part, list it with each subroom */
    /* The dog move and mail daemon routines use this */
    while(droom->hx >= 0 && doorindex < DOORMAX) {
	if(droom->hx >= x-1 && droom->lx <= x+1 &&
	   droom->hy >= y-1 && droom->ly <= y+1) {
	    /* Found it */
	    add_door(x, y, droom);
	}
	droom++;
    }

    opvar_free(fx);
    opvar_free(fy);
    opvar_free(msk);
}

void
spo_stair(coder)
     struct sp_coder *coder;
{
    xchar x,y;
    struct opvar *up, *fy, *fx;
    struct trap *badtrap;

    if (!OV_pop_i(up) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    if (coder->croom) {
	x = OV_i(fx);
	y = OV_i(fy);
	get_location(&x, &y, DRY, coder->croom);
	mkstairs(x,y,(char)OV_i(up), coder->croom);
	SpLev_Map[x][y] = 1;
    } else {
	x = OV_i(fx);
	y = OV_i(fy);
	get_location(&x, &y, DRY, coder->croom);
	if ((badtrap = t_at(x,y)) != 0) deltrap(badtrap);
	mkstairs(x, y, (char)OV_i(up), coder->croom);
	SpLev_Map[x][y] = 1;
    }

    opvar_free(fx);
    opvar_free(fy);
    opvar_free(up);
}

void
spo_ladder(coder)
     struct sp_coder *coder;
{
    xchar x,y;
    struct opvar *up, *fy, *fx;

    if (!OV_pop_i(up) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    x = OV_i(fx);
    y = OV_i(fy);

    get_location(&x, &y, DRY, coder->croom);

    levl[x][y].typ = LADDER;
    SpLev_Map[x][y] = 1;
    if (OV_i(up)) {
	xupladder = x; yupladder = y;
	levl[x][y].ladder = LA_UP;
    } else {
	xdnladder = x; ydnladder = y;
	levl[x][y].ladder = LA_DOWN;
    }
    opvar_free(fx);
    opvar_free(fy);
    opvar_free(up);
}

void
spo_grave(coder)
     struct sp_coder *coder;
{
    struct opvar *gx,*gy, *typ, *txt;
    schar x,y;
    if (!OV_pop_i(typ) ||
	!OV_pop_s(txt) ||
	!OV_pop_i(gy) ||
	!OV_pop_i(gx)) return;

    x = OV_i(gx); y = OV_i(gy);
    get_location(&x, &y, DRY, coder->croom);

    if (isok(x, y) && !t_at(x, y)) {
	levl[x][y].typ = GRAVE;
	switch (OV_i(typ)) {
	case 2: make_grave(x, y, OV_s(txt)); break;
	case 1: make_grave(x, y, NULL); break;
	default: del_engr_at(x, y); break;
	}
    }

    opvar_free(gx);
    opvar_free(gy);
    opvar_free(typ);
    opvar_free(txt);
}

void
spo_altar(coder)
     struct sp_coder *coder;
{
    struct opvar *al, *shrine, *fy, *fx;
    altar tmpaltar;

    if (!OV_pop_i(al) ||
	!OV_pop_i(shrine) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    tmpaltar.x = OV_i(fx);
    tmpaltar.y = OV_i(fy);
    tmpaltar.align = OV_i(al);
    tmpaltar.shrine = OV_i(shrine);

    create_altar(&tmpaltar, coder->croom);

    opvar_free(fx);
    opvar_free(fy);
    opvar_free(shrine);
    opvar_free(al);
}

void
spo_wallwalk(coder)
     struct sp_coder *coder;
{
    struct opvar *y, *x, *fgtyp, *bgtyp, *chance;

    if (!OV_pop_i(bgtyp) ||
	!OV_pop_i(fgtyp) ||
	!OV_pop_i(chance) ||
	!OV_pop_i(y) ||
	!OV_pop_i(x)) return;

    wallwalk_right(OV_i(x), OV_i(y), OV_i(fgtyp), OV_i(bgtyp), OV_i(chance));

    opvar_free(x);
    opvar_free(y);
    opvar_free(chance);
    opvar_free(fgtyp);
    opvar_free(bgtyp);
}

void
spo_feature(coder)
     struct sp_coder *coder;
{
    struct opvar *fy, *fx;
    int typ;

    if (!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    switch (coder->opcode) {
    default: impossible("spo_feature called with wrong opcode %i.", coder->opcode); break;
    case SPO_FOUNTAIN: typ = FOUNTAIN; break;
    case SPO_SINK:     typ = SINK;     break;
    case SPO_POOL:     typ = POOL;     break;
    }

    create_feature(OV_i(fx), OV_i(fy), coder->croom, typ);
    opvar_free(fx);
    opvar_free(fy);
}

void
spo_trap(coder)
     struct sp_coder *coder;
{
    struct opvar *type, *fy, *fx;
    trap tmptrap;

    if (!OV_pop_i(type) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    tmptrap.x = OV_i(fx);
    tmptrap.y = OV_i(fy);
    tmptrap.type = OV_i(type);

    create_trap(&tmptrap, coder->croom);
    opvar_free(fx);
    opvar_free(fy);
    opvar_free(type);
}

void
spo_gold(coder)
     struct sp_coder *coder;
{
    struct opvar *fy, *fx, *amt;
    gold tmpgold;
    if (!OV_pop_i(fx) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(amt)) return;
    tmpgold.x = OV_i(fx);
    tmpgold.y = OV_i(fy);
    tmpgold.amount = OV_i(amt);
    create_gold(&tmpgold, coder->croom);
    opvar_free(fx);
    opvar_free(fy);
    opvar_free(amt);
}

void
spo_corridor(coder)
     struct sp_coder *coder;
{
    struct opvar *deswall, *desdoor, *desroom,
	*srcwall, *srcdoor, *srcroom;
    corridor tc;

    if (!OV_pop_i(deswall) ||
	!OV_pop_i(desdoor) ||
	!OV_pop_i(desroom) ||
	!OV_pop_i(srcwall) ||
	!OV_pop_i(srcdoor) ||
	!OV_pop_i(srcroom)) return;

    tc.src.room = OV_i(srcroom);
    tc.src.door = OV_i(srcdoor);
    tc.src.wall = OV_i(srcwall);
    tc.dest.room = OV_i(desroom);
    tc.dest.door = OV_i(desdoor);
    tc.dest.wall = OV_i(deswall);

    create_corridor(&tc);

    opvar_free(deswall);
    opvar_free(desdoor);
    opvar_free(desroom);
    opvar_free(srcwall);
    opvar_free(srcdoor);
    opvar_free(srcroom);
}

void
spo_terrain(coder)
     struct sp_coder *coder;
{
    terrain tmpterrain;
    struct opvar *x1,*y1,*x2,*y2,*areatyp,*ter,*tlit;

    if (!OV_pop_i(tlit) ||
	!OV_pop_i(ter) ||
	!OV_pop_i(areatyp) ||
	!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;

    tmpterrain.x1 = OV_i(x1);
    tmpterrain.y1 = OV_i(y1);
    tmpterrain.x2 = OV_i(x2);
    tmpterrain.y2 = OV_i(y2);
    tmpterrain.areatyp = OV_i(areatyp);
    tmpterrain.ter = OV_i(ter);
    tmpterrain.tlit = OV_i(tlit);

    set_terrain(&tmpterrain, coder->croom);
    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
    opvar_free(areatyp);
    opvar_free(ter);
    opvar_free(tlit);
}

void
spo_replace_terrain(coder)
     struct sp_coder *coder;
{
    replaceterrain rt;
    struct opvar *x1,*y1,*x2,*y2,*from_ter,*to_ter,*to_lit,*chance;

    if (!OV_pop_i(chance) ||
	!OV_pop_i(to_lit) ||
	!OV_pop_i(to_ter) ||
	!OV_pop_i(from_ter) ||
	!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;

    rt.chance = OV_i(chance);
    rt.tolit = OV_i(to_lit);
    rt.toter = OV_i(to_ter);
    rt.fromter = OV_i(from_ter);
    rt.x1 = OV_i(x1);
    rt.y1 = OV_i(y1);
    rt.x2 = OV_i(x2);
    rt.y2 = OV_i(y2);

    replace_terrain(&rt, coder->croom);

    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
    opvar_free(from_ter);
    opvar_free(to_ter);
    opvar_free(to_lit);
    opvar_free(chance);
}

void
spo_randline(coder)
     struct sp_coder *coder;
{
    randline rl;
    struct opvar *x1,*y1,*x2,*y2,*fg,*lit,*roughness,*thick;
    if (!OV_pop_i(thick) ||
	!OV_pop_i(roughness) ||
	!OV_pop_i(lit) ||
	!OV_pop_i(fg) ||
	!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;

    rl.thick = OV_i(thick);
    rl.roughness = OV_i(roughness);
    rl.lit = OV_i(lit);
    rl.fg = OV_i(fg);
    rl.x1 = OV_i(x1);
    rl.y1 = OV_i(y1);
    rl.x2 = OV_i(x2);
    rl.y2 = OV_i(y2);

    line_midpoint(&rl, coder->croom);

    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
    opvar_free(fg);
    opvar_free(lit);
    opvar_free(roughness);
    opvar_free(thick);
}

void
spo_spill(coder)
     struct sp_coder *coder;
{
    spill sp;
    struct opvar *x,*y,*typ,*dir,*count,*lit;
    if (!OV_pop_i(lit) ||
	!OV_pop_i(count) ||
	!OV_pop_i(dir) ||
	!OV_pop_i(typ) ||
	!OV_pop_i(y) ||
	!OV_pop_i(x)) return;

    sp.x = OV_i(x);
    sp.y = OV_i(y);

    sp.lit = OV_i(lit);
    sp.count = OV_i(count);
    sp.direction = OV_i(dir);
    sp.typ = OV_i(typ);

    spill_terrain(&sp, coder->croom);

    opvar_free(x);
    opvar_free(y);
    opvar_free(typ);
    opvar_free(dir);
    opvar_free(count);
    opvar_free(lit);
}

void
spo_levregion(coder)
     struct sp_coder *coder;
{
    struct opvar *rname, *padding, *rtype,
	*del_islev, *dy2, *dx2, *dy1, *dx1,
	*in_islev,  *iy2, *ix2, *iy1, *ix1;

    lev_region *tmplregion;

    if (!OV_pop_s(rname) ||
	!OV_pop_i(padding) ||
	!OV_pop_i(rtype) ||
	!OV_pop_i(del_islev) ||
	!OV_pop_i(dy2) ||
	!OV_pop_i(dx2) ||
	!OV_pop_i(dy1) ||
	!OV_pop_i(dx1) ||
	!OV_pop_i(in_islev) ||
	!OV_pop_i(iy2) ||
	!OV_pop_i(ix2) ||
	!OV_pop_i(iy1) ||
	!OV_pop_i(ix1)) return;

    tmplregion = (lev_region *)alloc(sizeof(lev_region));

    tmplregion->inarea.x1 = OV_i(ix1);
    tmplregion->inarea.y1 = OV_i(iy1);
    tmplregion->inarea.x2 = OV_i(ix2);
    tmplregion->inarea.y2 = OV_i(iy2);

    tmplregion->delarea.x1 = OV_i(dx1);
    tmplregion->delarea.y1 = OV_i(dy1);
    tmplregion->delarea.x2 = OV_i(dx2);
    tmplregion->delarea.y2 = OV_i(dy2);

    tmplregion->in_islev = OV_i(in_islev);
    tmplregion->del_islev = OV_i(del_islev);
    tmplregion->rtype = OV_i(rtype);
    tmplregion->padding = OV_i(padding);
    tmplregion->rname.str = strdup(OV_s(rname));

    if(!tmplregion->in_islev) {
	get_location(&tmplregion->inarea.x1, &tmplregion->inarea.y1,
		     DRY|WET, (struct mkroom *)0);
	get_location(&tmplregion->inarea.x2, &tmplregion->inarea.y2,
		     DRY|WET, (struct mkroom *)0);
    }

    if(!tmplregion->del_islev) {
	get_location(&tmplregion->delarea.x1, &tmplregion->delarea.y1,
		     DRY|WET, (struct mkroom *)0);
	get_location(&tmplregion->delarea.x2, &tmplregion->delarea.y2,
		     DRY|WET, (struct mkroom *)0);
    }
    if(num_lregions) {
	/* realloc the lregion space to add the new one */
	lev_region *newl = (lev_region *) alloc(sizeof(lev_region) *
						(unsigned)(1+num_lregions));
	(void) memcpy((genericptr_t)(newl), (genericptr_t)lregions,
		      sizeof(lev_region) * num_lregions);
	Free(lregions);
	num_lregions++;
	lregions = newl;
    } else {
	num_lregions = 1;
	lregions = (lev_region *) alloc(sizeof(lev_region));
    }
    (void) memcpy(&lregions[num_lregions-1], tmplregion, sizeof(lev_region));

    opvar_free(dx1);
    opvar_free(dy1);
    opvar_free(dx2);
    opvar_free(dy2);

    opvar_free(ix1);
    opvar_free(iy1);
    opvar_free(ix2);
    opvar_free(iy2);

    opvar_free(del_islev);
    opvar_free(in_islev);
    opvar_free(rname);
    opvar_free(rtype);
    opvar_free(padding);
}

void
spo_region(coder)
     struct sp_coder *coder;
{
    struct opvar *rtype, *rlit, *rirreg, *y2,*x2,*y1,*x1;
    xchar dx1,dy1,dx2,dy2;
    register struct mkroom *troom;
    boolean prefilled, room_not_needed;

    if (!OV_pop_i(rirreg) ||
	!OV_pop_i(rtype) ||
	!OV_pop_i(rlit) ||
	!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;

    if(OV_i(rtype) > MAXRTYPE) {
	OV_i(rtype) -= MAXRTYPE+1;
	prefilled = TRUE;
    } else
	prefilled = FALSE;

    if(OV_i(rlit) < 0)
	OV_i(rlit) = (rnd(1+abs(depth(&u.uz))) < 11 && rn2(77))
	    ? TRUE : FALSE;

    dx1 = OV_i(x1);
    dy1 = OV_i(y1);
    dx2 = OV_i(x2);
    dy2 = OV_i(y2);

    get_location(&dx1, &dy1, DRY|WET, (struct mkroom *)0);
    get_location(&dx2, &dy2, DRY|WET, (struct mkroom *)0);

    /* for an ordinary room, `prefilled' is a flag to force
       an actual room to be created (such rooms are used to
       control placement of migrating monster arrivals) */
    room_not_needed = (OV_i(rtype) == OROOM &&
		       !OV_i(rirreg) && !prefilled);
    if (room_not_needed || nroom >= MAXNROFROOMS) {
	region tmpregion;
	if (!room_not_needed)
	    impossible("Too many rooms on new level!");
	tmpregion.rlit = OV_i(rlit);
	tmpregion.x1 = dx1;
	tmpregion.y1 = dy1;
	tmpregion.x2 = dx2;
	tmpregion.y2 = dy2;
	light_region(&tmpregion);
	return;
    }

    troom = &rooms[nroom];

    /* mark rooms that must be filled, but do it later */
    if (OV_i(rtype) != OROOM)
	troom->needfill = (prefilled ? 2 : 1);

    if (OV_i(rirreg)) {
	min_rx = max_rx = dx1;
	min_ry = max_ry = dy1;
	flood_fill_rm(dx1, dy1, nroom+ROOMOFFSET,
		      OV_i(rlit), TRUE);
	add_room(min_rx, min_ry, max_rx, max_ry,
		 FALSE, OV_i(rtype), TRUE);
	troom->rlit = OV_i(rlit);
	troom->irregular = TRUE;
    } else {
	add_room(dx1, dy1, dx2, dy2,
		 OV_i(rlit), OV_i(rtype), TRUE);
#ifdef SPECIALIZATION
	topologize(troom,FALSE);              /* set roomno */
#else
	topologize(troom);                    /* set roomno */
#endif
    }

    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
    opvar_free(rirreg);
    opvar_free(rlit);
    opvar_free(rtype);
}

void
spo_random_objects(coder)
     struct sp_coder *coder;
{
    struct opvar *robj;
    if (!OV_pop_s(robj)) return;
    n_robj = strlen(OV_s(robj));
    if ((n_robj <= 0) || (n_robj > MAX_REGISTERS)) {
	impossible("sp_level_coder: rnd_objs idx out-of-bounds");
	return;
    } else
	(void) memcpy(robjects, OV_s(robj), n_robj);
    opvar_free(robj);
    sp_lev_shuffle(robjects, (char *)0, n_robj);
}

void
spo_random_places(coder)
     struct sp_coder *coder;
{
    struct opvar *places;
    int tmpidx;

    if (!OV_pop_s(places)) return;
    if (strlen(OV_s(places)) % 2) impossible("sp_level_codes: rnd_places?");
    n_rloc = (strlen(OV_s(places)) / 2);
    if ((n_rloc <= 0) || (n_rloc > 2*MAX_REGISTERS)) {
	impossible("sp_level_coder: rnd_places idx out-of-bounds");
	return;
    } else {
	for (tmpidx = 0; tmpidx < n_rloc; tmpidx++) {
	    rloc_x[tmpidx] = (places->vardata.str[tmpidx*2] - 1);
	    rloc_y[tmpidx] = (places->vardata.str[tmpidx*2+1] - 1);
	}
    }
    opvar_free(places);
    sp_lev_shuffle(rloc_x, rloc_y, n_rloc);
}

void
spo_random_monsters(coder)
     struct sp_coder *coder;
{
    struct opvar *rmon;

    if (!OV_pop_s(rmon)) return;

    n_rmon = strlen(OV_s(rmon));
    if ((n_rmon <= 0) || (n_rmon > MAX_REGISTERS)) {
	impossible("sp_level_coder: rnd_mons idx out-of-bounds");
	return;
    } else
	(void) memcpy(rmonst, OV_s(rmon), n_rmon);
    opvar_free(rmon);
    sp_lev_shuffle(rmonst, (char *)0, n_rmon);
}

void
spo_drawbridge(coder)
     struct sp_coder *coder;
{
    xchar x,y;
    struct opvar *dir, *db_open, *fy, *fx;

    if (!OV_pop_i(dir) ||
	!OV_pop_i(db_open) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    x = OV_i(fx);
    y = OV_i(fy);
    get_location(&x, &y, DRY|WET, coder->croom);
    if (!create_drawbridge(x, y, OV_i(dir), OV_i(db_open)))
	impossible("Cannot create drawbridge.");
    SpLev_Map[x][y] = 1;

    opvar_free(fx);
    opvar_free(fy);
    opvar_free(db_open);
    opvar_free(dir);
}

void
spo_mazewalk(coder)
     struct sp_coder *coder;
{
    xchar x,y;
    struct opvar *ftyp, *fstocked,*fdir, *fy, *fx;
    int dir;

    if (!OV_pop_i(ftyp) ||
	!OV_pop_i(fstocked) ||
	!OV_pop_i(fdir) ||
	!OV_pop_i(fy) ||
	!OV_pop_i(fx)) return;

    dir = OV_i(fdir);
    x = OV_i(fx);
    y = OV_i(fy);

    get_location(&x, &y, DRY|WET, coder->croom);

    if (OV_i(ftyp) < 1) {
#ifndef WALLIFIED_MAZE
	OV_i(ftyp) = CORR;
#else
	OV_i(ftyp) = ROOM;
#endif
    }

    /* don't use move() - it doesn't use W_NORTH, etc. */
    switch (dir) {
    case W_NORTH: --y; break;
    case W_SOUTH: y++; break;
    case W_EAST:  x++; break;
    case W_WEST:  --x; break;
    default:
	impossible("sp_level_coder: Bad MAZEWALK direction");
    }

    if(!IS_DOOR(levl[x][y].typ)) {
	levl[x][y].typ = OV_i(ftyp);
	levl[x][y].flags = 0;
    }

    /*
     * We must be sure that the parity of the coordinates for
     * walkfrom() is odd.  But we must also take into account
     * what direction was chosen.
     */
    if(!(x % 2)) {
	if (dir == W_EAST)
	    x++;
	else
	    x--;

	/* no need for IS_DOOR check; out of map bounds */
	levl[x][y].typ = OV_i(ftyp);
	levl[x][y].flags = 0;
    }

    if (!(y % 2)) {
	if (dir == W_SOUTH)
	    y++;
	else
	    y--;
    }

    walkfrom(x, y, OV_i(ftyp));
    if (OV_i(fstocked)) fill_empty_maze();

    opvar_free(fx);
    opvar_free(fy);
    opvar_free(fdir);
    opvar_free(fstocked);
    opvar_free(ftyp);
}

void
spo_wall_property(coder)
     struct sp_coder *coder;
{
    struct opvar *x1,*y1,*x2,*y2;
    xchar dx1,dy1,dx2,dy2;
    int wprop = (coder->opcode == SPO_NON_DIGGABLE) ? W_NONDIGGABLE : W_NONPASSWALL;

    if (!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;

    dx1 = OV_i(x1);
    dy1 = OV_i(y1);
    dx2 = OV_i(x2);
    dy2 = OV_i(y2);

    get_location(&dx1, &dy1, DRY|WET, (struct mkroom *)0);
    get_location(&dx2, &dy2, DRY|WET, (struct mkroom *)0);

    set_wall_property(dx1, dy1, dx2, dy2, wprop);

    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
}

void
spo_room_door(coder)
     struct sp_coder *coder;
{
    struct opvar *wall, *secret, *mask, *pos;
    room_door tmpd;

    if (!OV_pop_i(wall) ||
	!OV_pop_i(secret) ||
	!OV_pop_i(mask) ||
	!OV_pop_i(pos) ||
	!coder->croom) return;

    tmpd.secret = OV_i(secret);
    tmpd.mask = OV_i(mask);
    tmpd.pos = OV_i(pos);
    tmpd.wall = OV_i(wall);

    create_door(&tmpd, coder->croom);

    opvar_free(wall);
    opvar_free(secret);
    opvar_free(mask);
    opvar_free(pos);
}

void
spo_wallify(coder)
     struct sp_coder *coder;
{
    struct opvar *x1,*y1,*x2,*y2;
    int dx1,dy1,dx2,dy2;
    if (!OV_pop_i(y2) ||
	!OV_pop_i(x2) ||
	!OV_pop_i(y1) ||
	!OV_pop_i(x1)) return;
    dx1 = (OV_i(x1) < 0) ? xstart : OV_i(x1);
    dy1 = (OV_i(y1) < 0) ? ystart : OV_i(y1);
    dx2 = (OV_i(x2) < 0) ? xstart+xsize : OV_i(x2);
    dy2 = (OV_i(y2) < 0) ? ystart+ysize : OV_i(y2);
    wallify_map(dx1, dy1, dx2, dy2);
    opvar_free(x1);
    opvar_free(y1);
    opvar_free(x2);
    opvar_free(y2);
}

void
spo_map(coder)
     struct sp_coder *coder;
{
    mazepart tmpmazepart;
    struct opvar *mpxs, *mpys, *mpmap, *mpva, *mpha, *mpkeepr, *mpzalign;
    xchar halign, valign;
    xchar tmpxstart, tmpystart, tmpxsize, tmpysize;

    if (!OV_pop_i(mpxs) ||
	!OV_pop_i(mpys) ||
	!OV_pop_s(mpmap) ||
	!OV_pop_i(mpva) ||
	!OV_pop_i(mpha) ||
	!OV_pop_i(mpkeepr) ||
	!OV_pop_i(mpzalign)) return;

    tmpmazepart.xsize = OV_i(mpxs);
    tmpmazepart.ysize = OV_i(mpys);
    tmpmazepart.zaligntyp = OV_i(mpzalign);
    tmpmazepart.halign = OV_i(mpha);
    tmpmazepart.valign = OV_i(mpva);

    tmpxsize = xsize; tmpysize = ysize;
    tmpxstart = xstart; tmpystart = ystart;

    halign = tmpmazepart.halign;
    valign = tmpmazepart.valign;
    xsize = tmpmazepart.xsize;
    ysize = tmpmazepart.ysize;
    switch (tmpmazepart.zaligntyp) {
    default:
    case 0:
	break;
    case 1:
	switch((int) halign) {
	case LEFT:	    xstart = 3;					break;
	case H_LEFT:    xstart = 2+((x_maze_max-2-xsize)/4);	break;
	case CENTER:    xstart = 2+((x_maze_max-2-xsize)/2);	break;
	case H_RIGHT:   xstart = 2+((x_maze_max-2-xsize)*3/4);	break;
	case RIGHT:     xstart = x_maze_max-xsize-1;		break;
	}
	switch((int) valign) {
	case TOP:	    ystart = 3;					break;
	case CENTER:    ystart = 2+((y_maze_max-2-ysize)/2);	break;
	case BOTTOM:    ystart = y_maze_max-ysize-1;		break;
	}
	if (!(xstart % 2)) xstart++;
	if (!(ystart % 2)) ystart++;
	break;
    case 2:
	get_location(&halign, &valign, DRY|WET, coder->croom);
	xstart = halign;
	ystart = valign;
	break;
    }
    if ((ystart < 0) || (ystart + ysize > ROWNO)) {
	/* try to move the start a bit */
	ystart += (ystart > 0) ? -2 : 2;
	if(ysize == ROWNO) ystart = 0;
	if(ystart < 0 || ystart + ysize > ROWNO)
	    panic("reading special level with ysize too large");
    }
    if (xsize <= 1 && ysize <= 1) {
	xstart = 1;
	ystart = 0;
	xsize = COLNO-1;
	ysize = ROWNO;
    } else {
	xchar x,y;
	/* Load the map */
	for(y = ystart; y < ystart+ysize; y++)
	    for(x = xstart; x < xstart+xsize; x++) {
		xchar mptyp = (mpmap->vardata.str[(y-ystart) * xsize + (x-xstart)] - 1);
		if (mptyp >= MAX_TYPE) continue;
		levl[x][y].typ = mptyp;
		levl[x][y].lit = FALSE;
		/* clear out levl: load_common_data may set them */
		levl[x][y].flags = 0;
		levl[x][y].horizontal = 0;
		levl[x][y].roomno = 0;
		levl[x][y].edge = 0;
		/*
		 *  Set secret doors to closed (why not trapped too?).  Set
		 *  the horizontal bit.
		 */
		if (levl[x][y].typ == SDOOR || IS_DOOR(levl[x][y].typ)) {
		    if(levl[x][y].typ == SDOOR)
			levl[x][y].doormask = D_CLOSED;
		    /*
		     *  If there is a wall to the left that connects to a
		     *  (secret) door, then it is horizontal.  This does
		     *  not allow (secret) doors to be corners of rooms.
		     */
		    if (x != xstart && (IS_WALL(levl[x-1][y].typ) ||
					levl[x-1][y].horizontal))
			levl[x][y].horizontal = 1;
		} else if(levl[x][y].typ == HWALL ||
			  levl[x][y].typ == IRONBARS)
		    levl[x][y].horizontal = 1;
		else if(levl[x][y].typ == LAVAPOOL)
		    levl[x][y].lit = 1;
	    }
	if (coder->lvl_is_joined)
	    remove_rooms(xstart, ystart, xstart+xsize, ystart+ysize);
    }
    if (!OV_i(mpkeepr)) {
	xstart = tmpxstart; ystart = tmpystart;
	xsize = tmpxsize; ysize = tmpysize;
    }

    opvar_free(mpxs);
    opvar_free(mpys);
    opvar_free(mpmap);
    opvar_free(mpva);
    opvar_free(mpha);
    opvar_free(mpkeepr);
    opvar_free(mpzalign);
}

void
spo_jmp(coder, lvl)
     struct sp_coder *coder;
     sp_lev *lvl;
{
    struct opvar *tmpa;
    long a;
    if (!OV_pop_i(tmpa)) return;
    a = sp_code_jmpaddr(coder->frame->n_opcode, (OV_i(tmpa) - 1));
    if ((a >= 0) && (a < lvl->n_opcodes) &&
	(a != coder->frame->n_opcode))
	coder->frame->n_opcode = a;
    opvar_free(tmpa);
}

void
spo_conditional_jump(coder,lvl)
     struct sp_coder *coder;
     sp_lev *lvl;
{
    struct opvar *oa, *oc;
    long a,c;
    int test;
    if (!OV_pop_i(oa) || !OV_pop_i(oc)) return;

    a = sp_code_jmpaddr(coder->frame->n_opcode, (OV_i(oa) - 1));
    c = OV_i(oc);

    switch (coder->opcode) {
    default: impossible("spo_conditional_jump: illegal opcode"); break;
    case SPO_JL:  test = (c < 0); break;
    case SPO_JLE: test = (c <= 0); break;
    case SPO_JG:  test = (c > 0); break;
    case SPO_JGE: test = (c >= 0); break;
    case SPO_JE:  test = (c == 0); break;
    case SPO_JNE: test = (c != 0); break;
    }

    if ((test) && (a >= 0) &&
	(a < lvl->n_opcodes) &&
	(a != coder->frame->n_opcode))
	coder->frame->n_opcode = a;

    opvar_free(oa);
    opvar_free(oc);
}

/* Special level coder, creates the special level from the sp_lev codes.
 * Does not free the allocated memory.
 */
STATIC_OVL boolean
sp_level_coder(lvl)
sp_lev *lvl;
{
    long exec_opcodes = 0;
    int     tmpi;
    struct sp_coder *coder = (struct sp_coder *)alloc(sizeof(struct sp_coder));

    coder->frame = frame_new(0);
    coder->stack = NULL;
    coder->premapped = FALSE;
    coder->allow_flips = 3;
    coder->croom = NULL;
    coder->n_subroom = 1;
    coder->exit_script = FALSE;
    coder->lvl_is_joined = 0;

    for (tmpi = 0; tmpi <= MAX_NESTED_ROOMS; tmpi++) {
	coder->tmproomlist[tmpi] = (struct mkroom *)0;
	coder->failed_room[tmpi] = FALSE;
    }

    shuffle_alignments();

    for (tmpi = 0; tmpi < MAX_CONTAINMENT; tmpi++) container_obj[tmpi] = NULL;
    container_idx = 0;

    invent_carrying_monster = NULL;

    (void) memset((genericptr_t)&SpLev_Map[0][0], 0, sizeof SpLev_Map);

    level.flags.is_maze_lev = 0;

    xstart = 1;
    ystart = 0;
    xsize = COLNO-1;
    ysize = ROWNO;

    while (coder->frame->n_opcode < lvl->n_opcodes && !coder->exit_script) {
	coder->opcode = lvl->opcodes[coder->frame->n_opcode].opcode;
	coder->opdat = lvl->opcodes[coder->frame->n_opcode].opdat;

	coder->stack = coder->frame->stack;

	if (exec_opcodes++ > SPCODER_MAX_RUNTIME) {
	    impossible("Level script is taking too much time, stopping.");
	    coder->exit_script = TRUE;
	}

	if (coder->failed_room[coder->n_subroom-1] &&
	    coder->opcode != SPO_ENDROOM &&
	    coder->opcode != SPO_ROOM &&
	    coder->opcode != SPO_SUBROOM) goto next_opcode;

	coder->croom = coder->tmproomlist[coder->n_subroom-1];

	switch (coder->opcode) {
        case SPO_NULL: break;
	case SPO_EXIT: coder->exit_script = TRUE; break;
	case SPO_FRAME_PUSH: spo_frame_push(coder); break;
	case SPO_FRAME_POP:  spo_frame_pop(coder); break;
	case SPO_CALL:       spo_call(coder); break;
	case SPO_RETURN:     spo_return(coder); break;
	case SPO_END_MONINVENT: spo_end_moninvent(coder); break;
	case SPO_POP_CONTAINER: spo_pop_container(coder); break;
	case SPO_POP:
	    {
		struct opvar *ov = splev_stack_pop(coder->stack);
		opvar_free(ov);
	    }
	    break;
	case SPO_PUSH: splev_stack_push(coder->stack, opvar_clone(coder->opdat)); break;
	case SPO_MESSAGE:        spo_message(coder);        break;
	case SPO_MONSTER:        spo_monster(coder);        break;
	case SPO_OBJECT:         spo_object(coder);         break;
	case SPO_LEVEL_FLAGS:    spo_level_flags(coder);    break;
	case SPO_INITLEVEL:      spo_initlevel(coder);      break;
	case SPO_MON_GENERATION: spo_mon_generation(coder); break;
	case SPO_LEVEL_SOUNDS:   spo_level_sounds(coder);   break;
	case SPO_ENGRAVING:      spo_engraving(coder);      break;
	case SPO_SUBROOM:
	case SPO_ROOM:           spo_room(coder);           break;
	case SPO_ENDROOM:        spo_endroom(coder);        break;
	case SPO_DOOR:           spo_door(coder);           break;
	case SPO_STAIR:          spo_stair(coder);          break;
	case SPO_LADDER:         spo_ladder(coder);         break;
	case SPO_GRAVE:          spo_grave(coder);          break;
	case SPO_ALTAR:          spo_altar(coder);          break;
	case SPO_SINK:
	case SPO_POOL:
	case SPO_FOUNTAIN:       spo_feature(coder);        break;
	case SPO_WALLWALK:       spo_wallwalk(coder);        break;
	case SPO_TRAP:           spo_trap(coder);           break;
	case SPO_GOLD:           spo_gold(coder);           break;
	case SPO_CORRIDOR:       spo_corridor(coder);       break;
	case SPO_TERRAIN:        spo_terrain(coder);        break;
	case SPO_REPLACETERRAIN: spo_replace_terrain(coder); break;
	case SPO_RANDLINE:       spo_randline(coder);        break;
	case SPO_SPILL:          spo_spill(coder);    break;
	case SPO_LEVREGION:      spo_levregion(coder); break;
	case SPO_REGION:         spo_region(coder);    break;
	case SPO_RANDOM_OBJECTS:  spo_random_objects(coder);   break;
	case SPO_RANDOM_PLACES:   spo_random_places(coder);    break;
	case SPO_RANDOM_MONSTERS: spo_random_monsters(coder);  break;
	case SPO_DRAWBRIDGE:      spo_drawbridge(coder);  break;
	case SPO_MAZEWALK:        spo_mazewalk(coder);    break;
	case SPO_NON_PASSWALL:
	case SPO_NON_DIGGABLE:    spo_wall_property(coder);   break;
	case SPO_ROOM_DOOR:       spo_room_door(coder);    break;
	case SPO_WALLIFY:         spo_wallify(coder);    break;
	case SPO_COPY:
	    {
		struct opvar *a = splev_stack_pop(coder->stack);
		splev_stack_push(coder->stack, opvar_clone(a));
		splev_stack_push(coder->stack, opvar_clone(a));
		opvar_free(a);
	    }
	    break;
	case SPO_DEC:
	    {
		struct opvar *a;
		if (!OV_pop_i(a)) break;
		OV_i(a)--;
		splev_stack_push(coder->stack, a);
	    }
	    break;
        case SPO_CMP:
	    {
		struct opvar *a = splev_stack_pop(coder->stack);
		struct opvar *b = splev_stack_pop(coder->stack);
		struct opvar *c;

		if (OV_typ(a) != OV_typ(b)) {
		    impossible("spo_cmp: trying to compare differing datatypes");
		    break;
		}

		switch (OV_typ(a)) {
		case SPOVAR_INT:
		    c = opvar_new_int((OV_i(b) - OV_i(a)));
		    break;
		case SPOVAR_STRING:
		    c = opvar_new_int(strcmp(OV_s(b), OV_s(a)));
		    break;
		default:
		    c = opvar_new_int(0);
		    break;
		}
		splev_stack_push(coder->stack, c);
		opvar_free(a);
		opvar_free(b);
	    }
	    break;
        case SPO_JMP:
	    spo_jmp(coder, lvl); break;
        case SPO_JL:
        case SPO_JLE:
        case SPO_JG:
        case SPO_JGE:
        case SPO_JE:
        case SPO_JNE:
	    spo_conditional_jump(coder, lvl); break;
	case SPO_RN2:
	    {
		struct opvar *tmpv;
		struct opvar *t;
		if (!OV_pop_i(tmpv)) break;
		t = opvar_new_int((OV_i(tmpv) > 0) ? rn2(OV_i(tmpv)) : 0);
		splev_stack_push(coder->stack, t);
		opvar_free(tmpv);
	    }
         break;
	case SPO_MAP:
	    spo_map(coder); break;
	default:
	    panic("sp_level_coder: Unknown opcode %i", coder->opcode);
	}

next_opcode:
	coder->frame->n_opcode++;
    } /*while*/

    fill_rooms();
    remove_boundary_syms();
    wallification(1, 0, COLNO-1, ROWNO-1);

	/* disable level flipping for some levels */
	if (!Is_astralevel(&u.uz) &&
	    !Is_blackmarket(&u.uz) &&
	    !Is_knox(&u.uz) &&
	    !Is_oracle_level(&u.uz) &&
	    !Is_minetown_level(&u.uz) &&
	    !Is_town_level(&u.uz) &&
	    /* When returning from the Valley the player gets
	     * placed on the right side of the screen. Regardless
	     * of flipped state. */
	    !Is_stronghold(&u.uz) &&
	    /* up and down ladders should be at the same position */
	    !In_V_tower(&u.uz)) {
		flip_level_rnd(coder->allow_flips);
	}

    count_features();

    if (coder->premapped) sokoban_detect();

    if (coder->frame) {
	struct sp_frame *tmpframe;
	do {
	    tmpframe = coder->frame->next;
	    frame_del(coder->frame);
	    coder->frame = tmpframe;
	} while (coder->frame);
    }

    return TRUE;
}

/*
 * General loader
 */

boolean
load_special(name)
const char *name;
{
	dlb *fd;
	sp_lev lvl;
	boolean result = FALSE;
	struct version_info vers_info;

	fd = dlb_fopen(name, RDBMODE);
	if (!fd) return FALSE;

	Fread((genericptr_t) &vers_info, sizeof vers_info, 1, fd);
	if (!check_version(&vers_info, name, TRUE))
	    goto give_up;

	result = sp_level_loader(fd, &lvl);
	if (result) result = sp_level_coder(&lvl);
	sp_level_free(&lvl);
 give_up:
	(void)dlb_fclose(fd);
	return result;
}

/*sp_lev.c*/
