/* Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the various functions that are related to the special
 * levels.
 *
 * It contains also the special level loader.
 */

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

#include "sp_lev.h"
#include "rect.h"

/* from mkmap.c */
extern void mkmap(lev_init *);

/* from sp_lev.c */
extern struct obj * create_object(object *o, struct mkroom *croom);
extern void create_altar(altar *, struct mkroom *);
extern void create_monster(monster *, struct mkroom *);
extern void spo_end_moninvent(void);
extern void spo_pop_container(void);

static void get_room_loc(coordxy *, coordxy *, struct mkroom *);
static void get_free_room_loc(coordxy *, coordxy *, struct mkroom *, packed_coord);
static int noncoalignment(aligntyp);
static boolean search_door(struct mkroom *, coordxy *, coordxy *, xint16, int);
static void create_corridor(corridor *);
static void create_trap(spltrap *, struct mkroom *);

static boolean create_subroom(struct mkroom *, coordxy, coordxy,
                              coordxy, coordxy, xint16, xint16);

long opvar_array_length(struct sp_coder *);

#define LEFT    1
#define H_LEFT  2
#define CENTER  3
#define H_RIGHT 4
#define RIGHT   5

#define TOP 1
#define BOTTOM  5

#define sq(x) ((x) * (x))

#define XLIM    4
#define YLIM    3

#define Fread   (void)dlb_fread
#define Fgetc   (schar)dlb_fgetc
#define New(type) (type *) alloc(sizeof(type))
#define NewTab(type, size) (type **) alloc(sizeof(type *) * (unsigned) size)
#define Free(ptr)       if (ptr) free((genericptr_t) (ptr))

extern struct engr *head_engr;

extern int min_rx, max_rx, min_ry, max_ry; /* from mkmap.c */

/* positions touched by level elements explicitly defined in the des-file */
char SpLev_Map[COLNO][ROWNO];

static aligntyp ralign[3] = { AM_CHAOTIC, AM_NEUTRAL, AM_LAWFUL };

static void set_wall_property(coordxy, coordxy, coordxy, coordxy, int);
static int rnddoor(void);
static int rndtrap(void);
static void light_region(region *);
static void maze1xy(coord *, int);
static boolean sp_level_loader(dlb *, sp_lev *);
static void create_door(room_door *, struct mkroom *);
static struct mkroom *build_room(room *, struct mkroom *);

char *lev_message = 0;
lev_region *lregions = 0;
int num_lregions = 0;

static boolean splev_init_present = FALSE;
static boolean icedpools = FALSE;
static int mines_prize_count = 0, soko_prize_count = 0; /* achievements */

#define SPLEV_STACK_RESERVE 128

void
solidify_map(void)
{
    coordxy x, y;

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (IS_STWALL(levl[x][y].typ) && !SpLev_Map[x][y]) {
                levl[x][y].wall_info |= (W_NONDIGGABLE | W_NONPASSWALL);
            }
        }
    }
}

void
splev_stack_init(struct splevstack *st)
{
    if (st) {
        st->depth = 0;
        st->depth_alloc = SPLEV_STACK_RESERVE;
        st->stackdata = (struct opvar **)alloc(st->depth_alloc * sizeof(struct opvar *));
        if (!st->stackdata) {
            panic("stack init alloc");
        }
    }
}

void
splev_stack_done(struct splevstack *st)
{
    if (st) {
        int i;

        if (st->stackdata && st->depth) {
            for (i = 0; i < st->depth; i++) {
                switch (st->stackdata[i]->spovartyp) {
                default:
                case SPOVAR_NULL:
                case SPOVAR_COORD:
                case SPOVAR_REGION:
                case SPOVAR_MAPCHAR:
                case SPOVAR_MONST:
                case SPOVAR_OBJ:
                case SPOVAR_INT:
                    break;
                case SPOVAR_VARIABLE:
                case SPOVAR_STRING:
                case SPOVAR_SEL:
                    Free(st->stackdata[i]->vardata.str);
                    st->stackdata[i]->vardata.str = NULL;
                    break;
                }
                Free(st->stackdata[i]);
                st->stackdata[i] = NULL;
            }
        }

        Free(st->stackdata);
        st->stackdata = NULL;
        st->depth = st->depth_alloc = 0;
        Free(st);
    }
}

void
splev_stack_push(struct splevstack *st, struct opvar *v)
{
    if (!st || !v) {
        return;
    }
    if (!st->stackdata) {
        panic("splev_stack_push: no stackdata allocated?");
    }

    if (st->depth >= st->depth_alloc) {
        struct opvar **tmp = (struct opvar **)alloc((st->depth_alloc + SPLEV_STACK_RESERVE) * sizeof(struct opvar *));
        if (!tmp) {
            panic("stack push alloc");
        }
        (void)memcpy(tmp, st->stackdata, st->depth_alloc * sizeof(struct opvar *));
        Free(st->stackdata);
        st->stackdata = tmp;
        st->depth_alloc += SPLEV_STACK_RESERVE;
    }

    st->stackdata[st->depth] = v;
    st->depth++;
}

struct opvar *
splev_stack_pop(struct splevstack *st)
{
    struct opvar *ret = NULL;

    if (!st) {
        return ret;
    }
    if (!st->stackdata) {
        panic("splev_stack_pop: no stackdata allocated?");
    }

    if (st->depth) {
        st->depth--;
        ret = st->stackdata[st->depth];
        st->stackdata[st->depth] = NULL;
        return ret;
    } else {
        impossible("splev_stack_pop: empty stack?");
    }
    return ret;
}

struct splevstack *
splev_stack_reverse(struct splevstack *st)
{
    long i;
    struct opvar *tmp;

    if (!st) {
        return NULL;
    }
    if (!st->stackdata) {
        panic("splev_stack_reverse: no stackdata allocated?");
    }
    for (i = 0; i < (st->depth / 2); i++) {
        tmp = st->stackdata[i];
        st->stackdata[i] = st->stackdata[st->depth - i - 1];
        st->stackdata[st->depth - i - 1] = tmp;
    }
    return st;
}

#define OV_typ(o) (o->spovartyp)
#define OV_i(o) (o->vardata.l)
#define OV_s(o) (o->vardata.str)

#define OV_pop_i(x) (x = splev_stack_getdat(coder, SPOVAR_INT))
#define OV_pop_c(x) (x = splev_stack_getdat(coder, SPOVAR_COORD))
#define OV_pop_r(x) (x = splev_stack_getdat(coder, SPOVAR_REGION))
#define OV_pop_s(x) (x = splev_stack_getdat(coder, SPOVAR_STRING))
#define OV_pop(x)   (x = splev_stack_getdat_any(coder))
#define OV_pop_typ(x, typ) (x = splev_stack_getdat(coder, typ))


struct opvar *
opvar_new_str(const char *s)
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) {
        panic("could not alloc opvar struct");
    }

    tmpov->spovartyp = SPOVAR_STRING;
    if (s) {
        int len = strlen(s);
        tmpov->vardata.str = (char *)alloc(len + 1);
        if (!tmpov->vardata.str) {
            panic("opvar new str alloc");
        }
        (void)memcpy((genericptr_t)tmpov->vardata.str,
                     (genericptr_t)s, len);
        tmpov->vardata.str[len] = '\0';
    } else {
        tmpov->vardata.str = NULL;
    }
    return tmpov;
}

struct opvar *
opvar_new_int(long int i)
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) {
        panic("could not alloc opvar struct");
    }
    tmpov->spovartyp = SPOVAR_INT;
    tmpov->vardata.l = i;
    return tmpov;
}

struct opvar *
opvar_new_coord(coordxy x, coordxy y)
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) {
        panic("could not alloc opvar struct");
    }
    tmpov->spovartyp = SPOVAR_COORD;
    tmpov->vardata.l = SP_COORD_PACK(x, y);
    return tmpov;
}

struct opvar *
opvar_new_region(int x1, int y1, int x2, int y2)
{
    struct opvar *tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) {
        panic("could not alloc opvar struct");
    }
    tmpov->spovartyp = SPOVAR_REGION;
    tmpov->vardata.l = SP_REGION_PACK(x1, y1, x2, y2);
    return tmpov;
}

void
opvar_free_x(struct opvar *ov)
{
    if (!ov) {
        return;
    }
    switch (ov->spovartyp) {
    case SPOVAR_COORD:
    case SPOVAR_REGION:
    case SPOVAR_MAPCHAR:
    case SPOVAR_MONST:
    case SPOVAR_OBJ:
    case SPOVAR_INT:
        break;
    case SPOVAR_VARIABLE:
    case SPOVAR_STRING:
    case SPOVAR_SEL:
        Free(ov->vardata.str);
        break;
    default: impossible("Unknown opvar value type (%i)!", ov->spovartyp);
    }
    Free(ov);
}

/*
 * Name of current function for use in messages:
 * __func__     -- C99 standard;
 * __FUNCTION__ -- gcc extension, starting before C99 and continuing after;
 *                 picked up by other compilers (or vice versa?);
 * __FUNC__     -- supported by Borland;
 * nhFunc       -- slightly intrusive but fully portable nethack construct
 *                 for any version of any compiler.
 */
#define opvar_free(ov)                                    \
    do {                                                  \
        if (ov) {                                         \
            opvar_free_x(ov);                             \
            ov = NULL;                                    \
        } else {                                          \
            impossible("opvar_free(), %s", __func__);     \
        }                                                 \
    } while (0)

struct opvar *
opvar_clone(struct opvar *ov)
{
    struct opvar *tmpov;

    if (!ov) {
        panic("no opvar to clone");
    }
    tmpov = (struct opvar *)alloc(sizeof(struct opvar));
    if (!tmpov) {
        panic("could not alloc opvar struct");
    }
    tmpov->spovartyp = ov->spovartyp;
    switch (ov->spovartyp) {
    case SPOVAR_COORD:
    case SPOVAR_REGION:
    case SPOVAR_MAPCHAR:
    case SPOVAR_MONST:
    case SPOVAR_OBJ:
    case SPOVAR_INT:
        tmpov->vardata.l = ov->vardata.l;
        break;
    case SPOVAR_VARIABLE:
    case SPOVAR_STRING:
    case SPOVAR_SEL:
        tmpov->vardata.str = dupstr(ov->vardata.str);
        break;
    default: impossible("Unknown push value type (%i)!", ov->spovartyp);
    }
    return tmpov;
}


struct opvar *
opvar_var_conversion(struct sp_coder *coder, struct opvar *ov)
{
    struct splev_var *tmp;
    struct opvar *tmpov;

    if (!coder || !ov) {
        return NULL;
    }
    if (ov->spovartyp != SPOVAR_VARIABLE) {
        return ov;
    }
    tmp = coder->frame->variables;
    while (tmp) {
        if (!strcmp(tmp->name, OV_s(ov))) {
            if ((tmp->svtyp & SPOVAR_ARRAY)) {
                struct opvar *variable = splev_stack_pop(coder->stack);
                struct opvar *array_idx = opvar_var_conversion(coder, variable);

                if (!array_idx || OV_typ(array_idx) != SPOVAR_INT) {
                    panic("array idx not an int");
                }
                if (tmp->array_len < 1) {
                    panic("array len < 1");
                }
                OV_i(array_idx) = (OV_i(array_idx) % tmp->array_len);
                tmpov = opvar_clone(tmp->data.arrayvalues[OV_i(array_idx)]);

                opvar_free_x(array_idx);
                if (array_idx != variable) {
                    opvar_free_x(variable);
                }

                return tmpov;
            } else {
                tmpov = opvar_clone(tmp->data.value);
                return tmpov;
            }
        }
        tmp = tmp->next;
    }
    return NULL;
}

struct splev_var *
opvar_var_defined(struct sp_coder *coder, char *name)
{
    struct splev_var *tmp;

    if (!coder) {
        return NULL;
    }
    tmp = coder->frame->variables;
    while (tmp) {
        if (!strcmp(tmp->name, name)) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

struct opvar *
splev_stack_getdat(struct sp_coder *coder, coordxy typ)
{
    if (coder && coder->stack) {
        struct opvar *tmp = splev_stack_pop(coder->stack);
        struct opvar *ret = NULL;

        if (!tmp) {
            panic("no value type %i in stack.", typ);
        }
        if (tmp->spovartyp == SPOVAR_VARIABLE) {
            ret = opvar_var_conversion(coder, tmp);
            opvar_free(tmp);
            tmp = ret;
        }
        if (tmp->spovartyp == typ) {
            return tmp;
        } else {
            opvar_free(tmp);
        }
    }
    return NULL;
}

struct opvar *
splev_stack_getdat_any(struct sp_coder *coder)
{
    if (coder && coder->stack) {
        struct opvar *tmp = splev_stack_pop(coder->stack);
        if (tmp && tmp->spovartyp == SPOVAR_VARIABLE) {
            struct opvar *ret = opvar_var_conversion(coder, tmp);
            opvar_free(tmp);
            return ret;
        }
        return tmp;
    }
    return NULL;
}

void
variable_list_del(struct splev_var *varlist)
{
    struct splev_var *tmp = varlist;

    if (!tmp) {
        return;
    }
    while (tmp) {
        Free(tmp->name);
        if ((tmp->svtyp & SPOVAR_ARRAY)) {
            long idx = tmp->array_len;

            while (idx-- > 0) {
                opvar_free(tmp->data.arrayvalues[idx]);
            };
            Free(tmp->data.arrayvalues);
        } else {
            opvar_free(tmp->data.value);
        }
        tmp = varlist->next;
        Free(varlist);
        varlist = tmp;
    }
}

void
lvlfill_maze_grid(int x1, int y1, int x2, int y2, schar filling)
{
    int x, y;

    for (x = x1; x <= x2; x++) {
        for (y = y1; y <= y2; y++) {
            if (level.flags.corrmaze) {
                levl[x][y].typ = STONE;
            } else {
                levl[x][y].typ = (y < 2 || ((x % 2) && (y % 2))) ? STONE : filling;
            }
        }
    }
}

void
lvlfill_solid(schar filling, schar lit)
{
    int x, y;

    for (x = 2; x <= x_maze_max; x++) {
        for (y = 0; y <= y_maze_max; y++) {
            SET_TYPLIT(x, y, filling, lit);
        }
    }
}

void flip_drawbridge_horizontal(struct rm *lev)
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

void flip_drawbridge_vertical(struct rm *lev)
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

#define FlipX(val) (inFlipArea(val, miny) ? ((maxx - (val)) + minx) : val)
#define FlipY(val) (inFlipArea(minx, val) ? ((maxy - (val)) + miny) : val)
#define inFlipArea(x,y) \
    ((x) >= minx && (x) <= maxx && (y) >= miny && (y) <= maxy)
#define Flip_coord(cc) \
    do {                                            \
        if ((cc).x && inFlipArea((cc).x, (cc).y)) { \
            if (flp & 1) {                          \
                (cc).y = FlipY((cc).y);             \
            }                                       \
            if (flp & 2) {                          \
                (cc).x = FlipX((cc).x);             \
            }                                       \
        }                                           \
    } while (0)

/*
 * Make walls of the area (x1, y1, x2, y2) non diggable/non passwall-able
 */
static void
set_wall_property(coordxy x1, coordxy y1, coordxy x2, coordxy y2, int prop)
{
    coordxy x, y;
    struct rm *lev;

    x1 = max(x1, 1);
    x2 = min(x2, COLNO - 1);
    y1 = max(y1, 0);
    y2 = min(y2, ROWNO - 1);
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            lev = &levl[x][y];
            if (IS_STWALL(lev->typ) || IS_TREES(lev->typ) ||
                /* 3.6.2: made iron bars eligible to be flagged nondiggable
                   (checked by chewing(hack.c) and zap_over_floor(zap.c)) */
                 lev->typ == IRONBARS) {
                lev->wall_info |= prop;
            }
        }
    }
}

static void
shuffle_alignments(void)
{
    int i;
    aligntyp atmp;
    /* shuffle 3 alignments */
    i = rn2(3);   atmp=ralign[2]; ralign[2]=ralign[i]; ralign[i]=atmp;
    if (rn2(2)) {
        atmp=ralign[1]; ralign[1]=ralign[0]; ralign[0]=atmp;
    }
}

void
remove_boundary_syms(void)
{
    /*
     * If any CROSSWALLs are found, must change to ROOM after REGION's
     * are laid out.  CROSSWALLS are used to specify "invisible"
     * boundaries where DOOR syms look bad or aren't desirable.
     */
    coordxy x, y;
    boolean has_bounds = FALSE;

    for (x = 0; x < COLNO-1; x++) {
        for (y = 0; y < ROWNO-1; y++) {
            if (levl[x][y].typ == CROSSWALL) {
                has_bounds = TRUE;
                break;
            }
        }
    }
    if (has_bounds) {
        for (x = 0; x < x_maze_max; x++) {
            for (y = 0; y < y_maze_max; y++) {
                if ((levl[x][y].typ == CROSSWALL) && SpLev_Map[x][y]) {
                    levl[x][y].typ = ROOM;
                }
            }
        }
    }
}
/* used by sel_set_door() and link_doors_rooms() */
static void
set_door_orientation(coordxy x, coordxy y)
{
    boolean wleft, wright, wup, wdown;

    /* If there's a wall or door on either the left side or right
     * side (or both) of this secret door, make it be horizontal.
     *
     * It is feasible to put SDOOR in a corner, tee, or crosswall
     * position, although once the door is found and opened it won't
     * make a lot sense (diagonal access required).  Still, we try to
     * handle that as best as possible.  For top or bottom tee, using
     * horizontal is the best we can do.  For corner or crosswall,
     * either horizontal or vertical are just as good as each other;
     * we produce horizontal for corners and vertical for crosswalls.
     * For left or right tee, using vertical is best.
     *
     * A secret door with no adjacent walls is also feasible and makes
     * even less sense.  It will be displayed as a vertical wall while
     * hidden and become a vertical door when found.  Before resorting
     * to that, we check for solid rock which hasn't been wallified
     * yet (cf lower leftside of leader's room in Cav quest).
     */
    wleft  = (isok(x - 1, y) && (IS_WALL(levl[x - 1][y].typ) ||
                                 IS_DOOR(levl[x - 1][y].typ) ||
                                 levl[x - 1][y].typ == SDOOR));
    wright = (isok(x + 1, y) && (IS_WALL(levl[x + 1][y].typ) ||
                                 IS_DOOR(levl[x + 1][y].typ) ||
                                 levl[x + 1][y].typ == SDOOR));
    wup    = (isok(x, y - 1) && (IS_WALL(levl[x][y - 1].typ) ||
                                 IS_DOOR(levl[x][y - 1].typ) ||
                                 levl[x][y - 1].typ == SDOOR));
    wdown  = (isok(x, y + 1) && (IS_WALL(levl[x][y + 1].typ) ||
                                 IS_DOOR(levl[x][y + 1].typ) ||
                                 levl[x][y + 1].typ == SDOOR));
    if (!wleft && !wright && !wup && !wdown) {
        /* out of bounds is treated as implicit wall; should be academic
           because we don't expect to have doors so near the level's edge */
        wleft  = (!isok(x - 1, y) || IS_DOORJOIN(levl[x - 1][y].typ));
        wright = (!isok(x + 1, y) || IS_DOORJOIN(levl[x + 1][y].typ));
        wup    = (!isok(x, y - 1) || IS_DOORJOIN(levl[x][y - 1].typ));
        wdown  = (!isok(x, y + 1) || IS_DOORJOIN(levl[x][y + 1].typ));
    }
    levl[x][y].horizontal = ((wleft || wright) && !(wup && wdown)) ? 1 : 0;
}

static void
maybe_add_door(coordxy x, coordxy y, struct mkroom *droom)
{
    if (droom->hx >= 0 && doorindex < DOORMAX && inside_room(droom, x, y)) {
        int i;
        for (i = droom->fdoor; i < droom->fdoor + droom->doorct; i++) {
            if (doors[i].x == x && doors[i].y == y) {
                return;
            }
        }
        add_door(x, y, droom);
    }
}

static void
link_doors_rooms(void)
{
    int x, y;
    int tmpi, m;

    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            if (IS_DOOR(levl[x][y].typ) || levl[x][y].typ == SDOOR) {
                /* in case this door was a '+' or 'S' from the
                   MAP...ENDMAP section without an explicit DOOR
                   directive, set/clear levl[][].horizontal for it */
                set_door_orientation(x, y);

                for (tmpi = 0; tmpi < nroom; tmpi++) {
                    maybe_add_door(x, y, &rooms[tmpi]);
                    for (m = 0; m < rooms[tmpi].nsubrooms; m++) {
                        maybe_add_door(x, y, rooms[tmpi].sbrooms[m]);
                    }
                }
            }
        }
    }
}

void
fill_rooms(void)
{
    int tmpi;

    for (tmpi = 0; tmpi < nroom; tmpi++) {
        int m;
        if (rooms[tmpi].needfill) {
            fill_room(&rooms[tmpi], (rooms[tmpi].needfill == 2));
        }
        for (m = 0; m < rooms[tmpi].nsubrooms; m++) {
            if (rooms[tmpi].sbrooms[m]->needfill) {
                fill_room(rooms[tmpi].sbrooms[m], FALSE);
            }
        }
    }
}

/*
 * Choose randomly the state (nodoor, open, closed or locked) for a door
 */
static int
rnddoor(void)
{
    int i = 1 << rn2(5);

    i >>= 1;
    return i;
}

/*
 * Select a random trap
 */
static int
rndtrap(void)
{
    int rtrap;

    do {
        rtrap = rnd(TRAPNUM-1);
        switch (rtrap) {
        case HOLE: /* no random holes on special levels */
        case VIBRATING_SQUARE:
        case MAGIC_PORTAL: rtrap = NO_TRAP;
            break;
        case TRAPDOOR: if (!Can_dig_down(&u.uz)) {
            rtrap = NO_TRAP;
        }
            break;
        case LEVEL_TELEP:
        case TELEP_TRAP:   if (level.flags.noteleport) {
            rtrap = NO_TRAP;
        }
            break;
        case ROLLING_BOULDER_TRAP:
        case ROCKTRAP: if (In_endgame(&u.uz)) {
            rtrap = NO_TRAP;
        }
            break;
        }
    } while (rtrap == NO_TRAP);
    return rtrap;
}

/*
 * Coordinates in special level files are handled specially:
 *
 *  if x or y is < 0, we generate a random coordinate.
 *  The "humidity" flag is used to insure that engravings aren't
 *  created underwater, or eels on dry land.
 */
static boolean is_ok_location(coordxy, coordxy, int);

static void
get_location(
    coordxy *x, coordxy *y,
    int humidity,
    struct mkroom *croom)
{
    int cpt = 0;
    int mx, my, sx, sy;

    if (croom) {
        mx = croom->lx;
        my = croom->ly;
        sx = croom->hx - mx + 1;
        sy = croom->hy - my + 1;
    } else {
        mx = gx.xstart;
        my = gy.ystart;
        sx = gx.xsize;
        sy = gy.ysize;
    }

    if (*x >= 0) { /* normal locations */
        *x += mx;
        *y += my;
    } else { /* random location */
        do {
            if (croom) { /* handle irregular areas */
                coord tmpc;
                somexy(croom, &tmpc);
                *x = tmpc.x;
                *y = tmpc.y;
            } else {
                *x = mx + rn2((int)sx);
                *y = my + rn2((int)sy);
            }
            if (is_ok_location(*x, *y, humidity)) {
                break;
            }
        } while (++cpt < 100);
        if (cpt >= 100) {
            int xx, yy;

            /* last try */
            for (xx = 0; xx < sx; xx++) {
                for (yy = 0; yy < sy; yy++) {
                    *x = mx + xx;
                    *y = my + yy;
                    if (is_ok_location(*x, *y, humidity)) {
                        goto found_it;
                    }
                }
            }
            if (!(humidity & NO_LOC_WARN)) {
                impossible("get_location:  can't find a place!  mx: %d, my: %d, sx: %d, sy: %d",
                           mx, my, sx, sy);
            } else {
                *x = *y = -1;
            }
        }
    }
found_it:;

    if (!(humidity & ANY_LOC) && !isok(*x, *y)) {
        if (!(humidity & NO_LOC_WARN)) {
            warning("get_location:  (%d,%d) out of bounds", *x, *y);
            *x = x_maze_max; *y = y_maze_max;
        } else {
            *x = *y = -1;
        }
    }
}

static boolean
is_ok_location(coordxy x, coordxy y, int humidity)
{
    int typ;

    if (Is_waterlevel(&u.uz)) {
        return TRUE; /* accept any spot */
    }

    /* TODO: Should perhaps check if wall is diggable/passwall? */
    if (humidity & ANY_LOC) {
        return TRUE;
    }

    if ((humidity & SOLID) && IS_ROCK(levl[x][y].typ)) {
        return TRUE;
    }
    if (humidity & DRY) {
        typ = levl[x][y].typ;
        if (typ == ROOM || typ == AIR ||
            typ == CLOUD || typ == ICE || typ == CORR)
            return TRUE;
    }
    if ((humidity & SPACELOC) && SPACE_POS(levl[x][y].typ)) {
        return TRUE;
    }
    if ((humidity & WET) && is_pool(x, y)) {
        return TRUE;
    }
    if ((humidity & HOT) && is_lava(x, y)) {
        return TRUE;
    }
    return FALSE;
}

unpacked_coord
get_unpacked_coord(long int loc, int defhumidity)
{
    static unpacked_coord c;

    if (loc & SP_COORD_IS_RANDOM) {
        c.x = c.y = -1;
        c.is_random = 1;
        c.getloc_flags = (loc & ~SP_COORD_IS_RANDOM);
        if (!c.getloc_flags) {
            c.getloc_flags = defhumidity;
        }
    } else {
        c.is_random = 0;
        c.getloc_flags = defhumidity;
        c.x = SP_COORD_X(loc);
        c.y = SP_COORD_Y(loc);
    }
    return c;
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

static void
get_room_loc(coordxy *x, coordxy *y, struct mkroom *croom)
{
    coord c;

    if (*x <0 && *y <0) {
        if (somexy(croom, &c)) {
            *x = c.x;
            *y = c.y;
        } else {
            panic("get_room_loc : can't find a place!");
        }
    } else {
        if (*x < 0) {
            *x = rn2(croom->hx - croom->lx + 1);
        }
        if (*y < 0) {
            *y = rn2(croom->hy - croom->ly + 1);
        }
        *x += croom->lx;
        *y += croom->ly;
    }
}

/*
 * Get a relative position inside a room.
 * negative values for x or y means RANDOM!
 */

static void
get_free_room_loc(
    coordxy *x, coordxy *y,
    struct mkroom *croom,
    long int pos)
{
    coordxy try_x, try_y;
    int trycnt = 0;

    get_location_coord(&try_x, &try_y, DRY, croom, pos);
    if (levl[try_x][try_y].typ != ROOM) {
        do {
            try_x = *x,  try_y = *y;
            get_room_loc(&try_x, &try_y, croom);
        } while (levl[try_x][try_y].typ != ROOM && ++trycnt <= 100);

        if (trycnt > 100) {
            panic("get_free_room_loc:  can't find a place!");
        }
    }
    *x = try_x,  *y = try_y;
}

/*
 * Create a subroom in room proom at pos x,y with width w & height h.
 * x & y are relative to the parent room.
 */
static boolean
create_subroom(
    struct mkroom *proom,
    coordxy x, coordxy y,
    coordxy w, coordxy h,
    xint16 rtype, xint16 rlit)
{
    coordxy width, height;

    width = proom->hx - proom->lx + 1;
    height = proom->hy - proom->ly + 1;

    /* There is a minimum size for the parent room */
    if (width < 4 || height < 4) {
        return FALSE;
    }

    /* Check for random position, size, etc... */

    if (w == -1) {
        w = rnd(width - 3);
    }
    if (h == -1) {
        h = rnd(height - 3);
    }
    if (x == -1) {
        x = rnd(width - w - 1) - 1;
    }
    if (y == -1) {
        y = rnd(height - h - 1) - 1;
    }
    if (x == 1) {
        x = 0;
    }
    if (y == 1) {
        y = 0;
    }
    if ((x + w + 1) == width) {
        x++;
    }
    if ((y + h + 1) == height) {
        y++;
    }
    if (rtype == -1) {
        rtype = OROOM;
    }
    if (rlit == -1) {
        rlit = (rnd(1 + abs(depth(&u.uz))) < 11 && rn2(77)) ? TRUE : FALSE;
    }
    add_subroom(proom, proom->lx + x, proom->ly + y,
                proom->lx + x + w - 1, proom->ly + y + h - 1,
                rlit, rtype, FALSE);
    return TRUE;
}

/*
 * Create a new door in a room.
 * It's placed on a wall (north, south, east or west).
 */
static void
create_door(room_door *dd, struct mkroom *broom)
{
    int x = 0, y = 0;
    int trycnt = 0, wtry = 0;

    if (dd->secret == -1) {
        dd->secret = rn2(2);
    }

    if (dd->mask == -1) {
        /* is it a locked door, closed, or a doorway? */
        if (!dd->secret) {
            if (!rn2(3)) {
                if (!rn2(5)) {
                    dd->mask = D_ISOPEN;
                } else if (!rn2(6)) {
                    dd->mask = D_LOCKED;
                } else {
                    dd->mask = D_CLOSED;
                }
                if (dd->mask != D_ISOPEN && !rn2(25)) {
                    dd->mask |= D_TRAPPED;
                }
            } else {
                dd->mask = D_NODOOR;
            }
        } else {
            if (!rn2(5)) {
                dd->mask = D_LOCKED;
            } else {
                dd->mask = D_CLOSED;
            }

            if (!rn2(20)) {
                dd->mask |= D_TRAPPED;
            }
        }
    }

    do {
        int dwall, dpos;

        dwall = dd->wall;
        if (dwall == -1) { /* The wall is RANDOM */
            dwall = 1 << rn2(4);
        }

        dpos = dd->pos;

        /* Convert wall and pos into an absolute coordinate! */
        wtry = rn2(4);
        switch (wtry) {
        case 0:
            if (!(dwall & W_NORTH)) {
                goto redoloop;
            }
            y = broom->ly - 1;
            x = broom->lx + ((dpos == -1) ? rn2(1+(broom->hx - broom->lx)) : dpos);
            if (IS_ROCK(levl[x][y-1].typ)) {
                goto redoloop;
            }
            goto outdirloop;
        case 1:
            if (!(dwall & W_SOUTH)) {
                goto redoloop;
            }
            y = broom->hy + 1;
            x = broom->lx + ((dpos == -1) ? rn2(1+(broom->hx - broom->lx)) : dpos);
            if (IS_ROCK(levl[x][y+1].typ)) {
                goto redoloop;
            }
            goto outdirloop;
        case 2:
            if (!(dwall & W_WEST)) {
                goto redoloop;
            }
            x = broom->lx - 1;
            y = broom->ly + ((dpos == -1) ? rn2(1+(broom->hy - broom->ly)) : dpos);
            if (IS_ROCK(levl[x-1][y].typ)) {
                goto redoloop;
            }
            goto outdirloop;
        case 3:
            if (!(dwall & W_EAST)) {
                goto redoloop;
            }
            x = broom->hx + 1;
            y = broom->ly + ((dpos == -1) ? rn2(1+(broom->hy - broom->ly)) : dpos);
            if (IS_ROCK(levl[x+1][y].typ)) {
                goto redoloop;
            }
            goto outdirloop;
        default:
            x = y = 0;
            panic("create_door: No wall for door!");
            goto outdirloop;
        }
outdirloop:
        if (okdoor(x, y)) {
            break;
        }
redoloop:;
    } while (++trycnt <= 100);
    if (trycnt > 100) {
        impossible("create_door: Can't find a proper place!");
        return;
    }
    add_door(x, y, broom);
    levl[x][y].typ = (dd->secret ? SDOOR : DOOR);
    levl[x][y].doormask = dd->mask;
}

/*
 * Create a secret door in croom on any one of the specified walls.
 */
void
create_secret_door(
    struct mkroom *croom,
    coordxy walls) /**< any of W_NORTH | W_SOUTH | W_EAST | W_WEST (or W_ANY) */
{
    coordxy sx, sy; /* location of the secret door */
    int count;

    for (count = 0; count < 100; count++) {
        sx = rn1(croom->hx - croom->lx + 1, croom->lx);
        sy = rn1(croom->hy - croom->ly + 1, croom->ly);

        switch (rn2(4)) {
        case 0: /* top */
            if (!(walls & W_NORTH)) {
                continue;
            }
            sy = croom->ly-1; break;
        case 1: /* bottom */
            if (!(walls & W_SOUTH)) {
                continue;
            }
            sy = croom->hy+1; break;
        case 2: /* left */
            if (!(walls & W_EAST)) {
                continue;
            }
            sx = croom->lx-1; break;
        case 3: /* right */
            if (!(walls & W_WEST)) {
                continue;
            }
            sx = croom->hx+1; break;
        }

        if (okdoor(sx, sy)) {
            levl[sx][sy].typ = SDOOR;
            levl[sx][sy].doormask = D_CLOSED;
            add_door(sx, sy, croom);
            return;
        }
    }

    impossible("couldn't create secret door on any walls 0x%x", walls);
}

/*
 * Create a trap in a room.
 */
static void
create_trap(spltrap *t, struct mkroom *croom)
{
    coordxy x = -1, y = -1;
    coord tm;

    if (croom) {
        get_free_room_loc(&x, &y, croom, t->coord);
    } else {
        int trycnt = 0;
        do {
            get_location_coord(&x, &y, DRY, croom, t->coord);
        } while ((levl[x][y].typ == STAIRS || levl[x][y].typ == LADDER) && ++trycnt <= 100);
        if (trycnt > 100) {
            return;
        }
    }

    tm.x = x;
    tm.y = y;

    mktrap(t->type, 1, (struct mkroom*) 0, &tm);
}

static void
spill_terrain(spill *sp, struct mkroom *croom)
{
    coordxy x, y, nx, ny, qx, qy;
    int j, k, lastdir, guard;
    boolean found = FALSE;

    if (sp->typ >= MAX_TYPE) {
        return;
    }

    /* This code assumes that you're going to spill one particular
     * type of terrain from a wall into somewhere.
     *
     * If we were given a specific coordinate, though, it doesn't have
     * to start from a wall... */
    if (sp->x < 0 || sp->y < 0) {
        for (j = 0; j < 500; j++) {
            x = sp->x;
            y = sp->y;
            get_location(&x, &y, ANY_LOC, croom);
            nx = x; ny = y;
            switch (sp->direction) {
            case W_NORTH: ny++; break;        /* backwards to make sure we're against a wall */
            case W_SOUTH: ny--; break;
            case W_WEST: nx++; break;
            case W_EAST: nx--; break;
            default: return; break;
            }
            if (!isok(nx, ny)) {
                continue;
            }
            if (IS_WALL(levl[nx][ny].typ)) {     /* mark it as broken through */
                SET_TYPLIT(nx, ny, sp->typ, sp->lit);
                found = TRUE;
                break;
            }
        }
    } else {
        found = TRUE;
        x = sp->x;
        y = sp->y;
        get_location(&x, &y, ANY_LOC, croom); /* support random registers too */
    }

    if (!found) {
        return;
    }

    /* gloop! */
    lastdir = -1; nx = x; ny = y;
    for (j = sp->count; j > 0; j--) {
        guard = 0;
        SET_TYPLIT(nx, ny, sp->typ, sp->lit);
        do {
            guard++;
            do {
                k = rn2(5);
                qx = nx; qy = ny;
                if (k > 3) {
                    k = sp->direction;
                } else {
                    k = 1 << k;
                }
                switch (k) {
                case W_NORTH: qy--; break;
                case W_SOUTH: qy++; break;
                case W_WEST: qx--; break;
                case W_EAST: qx++; break;
                }
            } while (!isok(qx, qy));
        } while ((k == lastdir || levl[qx][qy].typ == sp->typ) && guard < 200);
        /* tend to not make rivers, but pools; and don't redo stuff of the same type! */

        switch (k) {
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
static int
noncoalignment(aligntyp alignment)
{
    int k;

    k = rn2(2);
    if (!alignment) {
        return k ? -1 : 1;
    }
    return k ? -alignment : 0;
}

/* attempt to screen out locations where a mimic-as-boulder shouldn't occur */
static boolean
m_bad_boulder_spot(coordxy x, coordxy y)
{
    struct rm *lev;

    /* avoid trap locations */
    if (t_at(x, y)) {
        return TRUE;
    }
    /* try to avoid locations which already have a boulder (this won't
       actually work; we get called before objects have been placed...) */
    if (sobj_at(BOULDER, x, y)) {
        return TRUE;
    }
    /* avoid closed doors */
    lev = &levl[x][y];
    if (IS_DOOR(lev->typ) && (lev->doormask & (D_CLOSED | D_LOCKED)) != 0) {
        return TRUE;
    }
    /* spot is ok */
    return FALSE;
}

static int
pm_to_humidity(struct permonst *pm)
{
    int loc = DRY;
    if (!pm) {
        return loc;
    }
    if (pm->mlet == S_EEL || amphibious(pm) || is_swimmer(pm)) {
        loc = WET;
    }
    if (is_flyer(pm) || is_floater(pm)) {
        loc |= (HOT | WET);
    }
    if (passes_walls(pm) || noncorporeal(pm)) {
        loc |= SOLID;
    }
    if (flaming(pm)) {
        loc |= HOT;
    }
    return loc;
}

void
replace_terrain(replaceterrain *terr, struct mkroom *croom)
{
    coordxy x, y, x1, y1, x2, y2;

    if (terr->toter >= MAX_TYPE) {
        return;
    }

    x1 = terr->x1;  y1 = terr->y1;
    get_location(&x1, &y1, ANY_LOC, croom);

    x2 = terr->x2;  y2 = terr->y2;
    get_location(&x2, &y2, ANY_LOC, croom);

    for (x = max(x1, 0); x <= min(x2, COLNO-1); x++) {
        for (y = max(y1, 0); y <= min(y2, ROWNO-1); y++) {
            if ((levl[x][y].typ == terr->fromter) && (rn2(100) < terr->chance)) {
                SET_TYPLIT(x, y, terr->toter, terr->tolit);
            }
        }
    }
}


/*
 * Search for a door in a room on a specified wall.
 */
static boolean
search_door(
    struct mkroom *croom,
    coordxy *x, coordxy *y,
    xint16 wall, int cnt)
{
    int dx, dy;
    int xx, yy;

    switch (wall) {
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
            if (cnt-- <= 0) {
                return TRUE;
            }
        }
        xx += dx;
        yy += dy;
    }
    return FALSE;
}

/*
 * Corridors always start from a door. But it can end anywhere...
 * Basically we search for door coordinates or for endpoints coordinates
 * (from a distance).
 */
static void
create_corridor(corridor *c)
{
    coord org, dest;

    if (c->src.room == -1) {
        makecorridors(c->src.door);
        return;
    }

    if ( !search_door(&rooms[c->src.room], &org.x, &org.y, c->src.wall,
                     c->src.door)) {
        return;
    }

    if (c->dest.room != -1) {
        if (!search_door(&rooms[c->dest.room], &dest.x, &dest.y,
                        c->dest.wall, c->dest.door)) {
            return;
        }
        switch (c->src.wall) {
        case W_NORTH: org.y--; break;
        case W_SOUTH: org.y++; break;
        case W_WEST:  org.x--; break;
        case W_EAST:  org.x++; break;
        }
        switch (c->dest.wall) {
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
fill_room(struct mkroom *croom, boolean prefilled)
{
    if (!croom || croom->rtype == OROOM) {
        return;
    }

    if (!prefilled) {
        int x, y;

        /* Shop ? */
        if (croom->rtype >= SHOPBASE) {
            stock_room(croom->rtype - SHOPBASE, croom);
            level.flags.has_shop = TRUE;
            return;
        }

        switch (croom->rtype) {
        case VAULT:
            for (x=croom->lx; x<=croom->hx; x++) {
                for (y=croom->ly; y<=croom->hy; y++) {
                    (void) mkgold((long)rn1(abs(depth(&u.uz))*100, 51), x, y);
                }
            }
            break;
        case GARDEN:
        case COURT:
        case ZOO:
        case BEEHIVE:
        case ANTHOLE:
        case COCKNEST:
        case LEPREHALL:
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
        break;
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

static struct mkroom *
build_room(room *r, struct mkroom *mkr)
{
    boolean okroom;
    struct mkroom   *aroom;
    xint16 rtype = (!r->chance || rn2(100) < r->chance) ? r->rtype : OROOM;

    if (mkr) {
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
        topologize(aroom, FALSE);        /* set roomno */
#else
        topologize(aroom);          /* set roomno */
#endif
        aroom->needfill = r->needfill;
        aroom->needjoining = r->joined;
        return aroom;
    }
    return (struct mkroom *)0;
}

/*
 * set lighting in a region that will not become a room.
 */
static void
light_region(region *tmpregion)
{
    boolean litstate = tmpregion->rlit ? 1 : 0;
    int hiy = tmpregion->y2;
    int x, y;
    struct rm *lev;
    int lowy = tmpregion->y1;
    int lowx = tmpregion->x1, hix = tmpregion->x2;

    if (litstate) {
        /* adjust region size for walls, but only if lighted */
        lowx = max(lowx-1, 1);
        hix = min(hix+1, COLNO-1);
        lowy = max(lowy-1, 0);
        hiy = min(hiy+1, ROWNO-1);
    }
    for (x = lowx; x <= hix; x++) {
        lev = &levl[x][lowy];
        for (y = lowy; y <= hiy; y++) {
            if (lev->typ != LAVAPOOL) { /* this overrides normal lighting */
                lev->lit = litstate;
            }
            lev++;
        }
    }
}

/*
 * Select a random coordinate in the maze.
 *
 * We want a place not 'touched' by the loader.  That is, a place in
 * the maze outside every part of the special level.
 */
static void
maze1xy(coord *m, int humidity)
{
    int x, y, tryct = 2000;
    /* tryct:  normally it won't take more than ten or so tries due
       to the circumstances under which we'll be called, but the
       `humidity' screening might drastically change the chances */

    do {
        x = rn1(x_maze_max - 3, 3);
        y = rn1(y_maze_max - 3, 3);
        if (--tryct < 0) {
            break; /* give up */
        }
    } while (!(x % 2) || !(y % 2) || !SpLev_Map[x][y] ||
             !is_ok_location((schar)x, (schar)y, humidity));

    m->x = (coordxy)x,  m->y = (coordxy)y;
}

/*
 * If there's a significant portion of maze unused by the special level,
 * we don't want it empty.
 *
 * Makes the number of traps, monsters, etc. proportional
 * to the size of the maze.
 */
static void
fill_empty_maze(void)
{
    int mapcountmax, mapcount, mapfact;
    coordxy x, y;
    coord mm;

    mapcountmax = mapcount = (x_maze_max - 2) * (y_maze_max - 2);
    mapcountmax = mapcountmax / 2;

    for (x = 2; x < x_maze_max; x++) {
        for (y = 0; y < y_maze_max; y++) {
            if (!SpLev_Map[x][y]) {
                mapcount--;
            }
        }
    }

    if ((mapcount > (int) (mapcountmax / 10))) {
        mapfact = (int) ((mapcount * 100L) / mapcountmax);
        for (x = rnd((int) (20 * mapfact) / 100); x; x--) {
            maze1xy(&mm, DRY);
            (void) mkobj_at(rn2(2) ? GEM_CLASS : RANDOM_CLASS,
                            mm.x, mm.y, TRUE);
        }
        for (x = rnd((int) (12 * mapfact) / 100); x; x--) {
            maze1xy(&mm, DRY);
            (void) mksobj_at(BOULDER, mm.x, mm.y, TRUE, FALSE);
        }
        for (x = rn2(2); x; x--) {
            maze1xy(&mm, DRY);
            (void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y, NO_MM_FLAGS);
        }
        for (x = rnd((int) (12 * mapfact) / 100); x; x--) {
            maze1xy(&mm, DRY);
            (void) makemon((struct permonst *) 0, mm.x, mm.y, NO_MM_FLAGS);
        }
        for (x = rn2((int) (15 * mapfact) / 100); x; x--) {
            maze1xy(&mm, DRY);
            (void) mkgold(0L, mm.x, mm.y);
        }
        for (x = rn2((int) (15 * mapfact) / 100); x; x--) {
            int trytrap;

            maze1xy(&mm, DRY);
            trytrap = rndtrap();
            if (sobj_at(BOULDER, mm.x, mm.y)) {
                while (is_pit(trytrap) || is_hole(trytrap)) {
                    trytrap = rndtrap();
                }
            }
            (void) maketrap(mm.x, mm.y, trytrap);
        }
    }
}

/*
 * special level loader
 */
static boolean
sp_level_loader(dlb *fd, sp_lev *lvl)
{
    long n_opcode = 0;
    struct opvar *opdat;
    int opcode;

    Fread((genericptr_t)&(lvl->n_opcodes), 1, sizeof(lvl->n_opcodes), fd);

    lvl->opcodes = (_opcode *)alloc(sizeof(_opcode) * (lvl->n_opcodes));
    if (!lvl->opcodes) {
        panic("sp lvl load opcodes alloc");
    }

    while (n_opcode < lvl->n_opcodes) {
        Fread((genericptr_t) &lvl->opcodes[n_opcode].opcode, 1,
              sizeof(lvl->opcodes[n_opcode].opcode), fd);
        opcode = lvl->opcodes[n_opcode].opcode;

        opdat = NULL;

        if (opcode < SPO_NULL || opcode >= MAX_SP_OPCODES) {
            panic("sp_level_loader: impossible opcode %i.", opcode);
        }

        if (opcode == SPO_PUSH) {
            int nsize;
            struct opvar *ov = (struct opvar *) alloc(sizeof(struct opvar));

            opdat = ov;
            if (!ov) {
                panic("push ov alloc");
            }
            ov->spovartyp = SPO_NULL;
            ov->vardata.l = 0;
            Fread((genericptr_t)&(ov->spovartyp), 1, sizeof(ov->spovartyp), fd);

            switch (ov->spovartyp) {
            case SPOVAR_NULL: break;
            case SPOVAR_COORD:
            case SPOVAR_REGION:
            case SPOVAR_MAPCHAR:
            case SPOVAR_MONST:
            case SPOVAR_OBJ:
            case SPOVAR_INT:
                Fread((genericptr_t)&(ov->vardata.l), 1, sizeof(ov->vardata.l), fd);
                break;
            case SPOVAR_VARIABLE:
            case SPOVAR_STRING:
            case SPOVAR_SEL:
            {
                char *opd;
                Fread((genericptr_t) &nsize, 1, sizeof(nsize), fd);
                opd = (char *)alloc(nsize + 1);
                if (!opd) {
                    panic("sp lvl load opd alloc");
                }
                if (nsize) {
                    Fread(opd, 1, nsize, fd);
                }
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
static boolean
sp_level_free(sp_lev *lvl)
{
    long n_opcode = 0;

    while (n_opcode < lvl->n_opcodes) {
        int opcode = lvl->opcodes[n_opcode].opcode;
        struct opvar *opdat = lvl->opcodes[n_opcode].opdat;

        if (opcode < SPO_NULL || opcode >= MAX_SP_OPCODES) {
            panic("sp_level_free: unknown opcode %i", opcode);
        }

        if (opdat) {
            opvar_free(opdat);
        }
        n_opcode++;
    }
    Free(lvl->opcodes);
    lvl->opcodes = NULL;
    return TRUE;
}

void
splev_initlev(lev_init *linit)
{
    switch (linit->init_style) {
    default: impossible("Unrecognized level init style."); break;
    case LVLINIT_NONE: break;
    case LVLINIT_SOLIDFILL:
        if (linit->lit == -1) {
            linit->lit = rn2(2);
        }
        lvlfill_solid(linit->filling, linit->lit);
        break;
    case LVLINIT_MAZEGRID:
        lvlfill_maze_grid(2, 0, x_maze_max, y_maze_max, linit->filling);
        break;
    case LVLINIT_SHEOL:
        mksheol(linit);
        break;
#ifdef REINCARNATION
    case LVLINIT_ROGUE:
        makeroguerooms();
        break;
#endif
    case LVLINIT_MINES:
        if (linit->lit == -1) {
            linit->lit = rn2(2);
        }
        if (linit->filling > -1) {
            lvlfill_solid(linit->filling, 0);
        }
        linit->icedpools = icedpools;
        mkmap(linit);
        break;
    }
}

struct sp_frame *
frame_new(long int execptr)
{
    struct sp_frame *frame = (struct sp_frame *)alloc(sizeof(struct sp_frame));
    if (!frame) {
        panic("could not create execution frame.");
    }

    frame->next = NULL;
    frame->variables = NULL;
    frame->n_opcode = execptr;
    frame->stack = (struct splevstack *)alloc(sizeof(struct splevstack));
    if (!frame->stack) {
        panic("could not create execution frame stack.");
    }
    splev_stack_init(frame->stack);
    return frame;
}

void
frame_del(struct sp_frame *frame)
{
    if (!frame) {
        return;
    }
    if (frame->stack) {
        splev_stack_done(frame->stack);
        frame->stack = NULL;
    }
    if (frame->variables) {
        variable_list_del(frame->variables);
        frame->variables = NULL;
    }
    Free(frame);
}

void
spo_corefunc(struct sp_coder *coder, long int fn)
{
    struct opvar *i;
    struct opvar *s;
    switch (fn) {
    default: impossible("Unknown sp_lev core function %li", fn); break;
    case COREFUNC_LEVEL_DIFFICULTY: i = opvar_new_int(level_difficulty()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_LEVEL_DEPTH: i = opvar_new_int(depth(&u.uz)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_DISCORDIAN_HOLIDAY: i = opvar_new_int(discordian_holiday()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_PIRATEDAY: i = opvar_new_int(pirateday()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_APRILFOOLSDAY: i = opvar_new_int(aprilfoolsday()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_PIDAY: i = opvar_new_int(piday()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_TOWELDAY: i = opvar_new_int(towelday()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_MIDNIGHT: i = opvar_new_int(midnight()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_NIGHT: i = opvar_new_int(night()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_FRIDAY_13TH: i = opvar_new_int(friday_13th()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_POM: i = opvar_new_int(phase_of_the_moon()); splev_stack_push(coder->stack, i); break;
    case COREFUNC_YYYYMMDD: i = opvar_new_int(yyyymmdd(0)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_PLNAME: s = opvar_new_str(plname); splev_stack_push(coder->stack, s); break;
    case COREFUNC_ROLE: s = opvar_new_str(urole.name.m); splev_stack_push(coder->stack, s); break;
    case COREFUNC_RACE: s = opvar_new_str(urace.noun); splev_stack_push(coder->stack, s); break;
    case COREFUNC_ROOM_WID: i = opvar_new_int((coder->croom ? (coder->croom->hx - coder->croom->lx + 1) : 0)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_ROOM_HEI: i = opvar_new_int((coder->croom ? (coder->croom->hy - coder->croom->ly + 1) : 0)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_ROOM_X: i = opvar_new_int((coder->croom ? coder->croom->lx : 0)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_ROOM_Y: i = opvar_new_int((coder->croom ? coder->croom->ly : 0)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_ARRAY_LEN: i = opvar_new_int(opvar_array_length(coder)); splev_stack_push(coder->stack, i); break;
    case COREFUNC_RN2:
        if (OV_pop_i(i)) {
            long li = ((OV_i(i) > 0) ? rn2(OV_i(i)) : 0);
            opvar_free(i);
            i = opvar_new_int(li);
            splev_stack_push(coder->stack, i);
        } else {
            impossible("No int in stack for rnd()");
        }
        break;
    case COREFUNC_COORD_X:
    {
        struct opvar *crd;
        if (OV_pop_c(crd)) {
            i = opvar_new_int(SP_COORD_X(OV_i(crd)));
            splev_stack_push(coder->stack, i);
            opvar_free(crd);
        } else {
            impossible("No coord in stack for $coord.x method");
        }
    }
    break;
    case COREFUNC_COORD_Y:
    {
        struct opvar *crd;
        if (OV_pop_c(crd)) {
            i = opvar_new_int(SP_COORD_Y(OV_i(crd)));
            splev_stack_push(coder->stack, i);
            opvar_free(crd);
        } else {
            impossible("No coord in stack for $coord.y method");
        }
    }
    break;
    case COREFUNC_TOSTRING:
        if (OV_pop_i(i)) {
            char tmpbuf[64];
            sprintf(tmpbuf, "%li", OV_i(i));
            s = opvar_new_str(tmpbuf);
            splev_stack_push(coder->stack, s);
            opvar_free(i);
        } else {
            impossible("No int in stack for tostring()");
        }
        break;
    case COREFUNC_TOINT:
        if (OV_pop_s(s)) {
            long li = atoi(OV_s(s));
            i = opvar_new_int(li);
            splev_stack_push(coder->stack, i);
            opvar_free(s);
        } else {
            impossible("No string in stack for toint()?");
        }
        break;
    case COREFUNC_TOCOORD:
    {
        struct opvar *x, *y;
        if (!OV_pop_i(y) || !OV_pop_i(x)) {
            impossible("No int values for coord()");
            return;
        }
        splev_stack_push(coder->stack, opvar_new_coord(OV_i(x), OV_i(y)));
        opvar_free(x);
        opvar_free(y);
    }
    break;
    case COREFUNC_TOREGION:
    {
        struct opvar *x1, *y1, *x2, *y2;
        if (!OV_pop_i(y2) || !OV_pop_i(x2) || !OV_pop_i(y1) || !OV_pop_i(x1)) {
            impossible("No int values for region()");
            return;
        }
        splev_stack_push(coder->stack, opvar_new_region(OV_i(x1), OV_i(y1), OV_i(x2), OV_i(y2)));
        opvar_free(x1); opvar_free(y1);
        opvar_free(x2); opvar_free(y2);
    }
    break;
    case COREFUNC_SOBJ_AT:
    {
        long retval = 0;
        struct opvar *obj;
        struct opvar *crd;
        int otyp;
        coordxy ox, oy;
        if (!OV_pop_c(crd) || !OV_pop_typ(obj, SPOVAR_OBJ)) {
            impossible("No coord and obj for obj_at()");
            return;
        }
        otyp = SP_OBJ_TYP(OV_i(obj));
        get_location_coord(&ox, &oy, ANY_LOC, coder->croom, OV_i(crd));
        if (otyp >= 0 && ox >= 0 && oy >= 0 && ox < COLNO && oy < ROWNO) {
            retval = sobj_at(otyp, ox, oy) ? 1 : 0;
        }
        i = opvar_new_int(retval);
        splev_stack_push(coder->stack, i);
        opvar_free(obj);
        opvar_free(crd);
    }
    break;
    case COREFUNC_MON_AT:
    {
        long rv = 0;
        struct opvar *mon;
        struct opvar *crd;
        coordxy ox, oy;
        if (!OV_pop_c(crd) || !OV_pop_typ(mon, SPOVAR_MONST)) {
            impossible("No coord and mon for mon_at()");
            return;
        }
        get_location_coord(&ox, &oy, ANY_LOC, coder->croom, OV_i(crd));
        if (ox >= 0 && oy >= 0 && ox < COLNO && oy < ROWNO) {
            struct monst *mtmp = m_at(ox, oy);
            if (mtmp) {
                if (OV_i(mon) == -1) {
                    rv = 1; /* -1 == random monster, always matches */
                } else {
                    int mclass = SP_MONST_CLASS(OV_i(mon));
                    int pm = SP_MONST_PM(OV_i(mon));
                    rv = ((((pm != 65535) && (mtmp->data == &mons[pm])) || (pm == 65535)) &&
                          (((mclass != 65535) && (def_monsyms[(int)mtmp->data->mlet] == mclass)) || (mclass == 65535))) ? 1 : 0;
                }
            }
        }
        i = opvar_new_int(rv);
        splev_stack_push(coder->stack, i);
        opvar_free(mon);
        opvar_free(crd);
    }
    break;
    case COREFUNC_CARRYING:
    {
        struct opvar *obj;
        int otyp;
        if (!OV_pop_typ(obj, SPOVAR_OBJ)) {
            impossible("No obj for carrying()");
            return;
        }
        otyp = SP_OBJ_TYP(OV_i(obj));
        i = opvar_new_int(carrying(otyp) ? 1 : 0);
        splev_stack_push(coder->stack, i);
        opvar_free(obj);
    }
    break;
    }
}

void
spo_frame_push(struct sp_coder *coder)
{
    struct sp_frame *tmpframe = frame_new(coder->frame->n_opcode);

    tmpframe->next = coder->frame;
    coder->frame = tmpframe;
}

void
spo_frame_pop(struct sp_coder *coder)
{
    if (coder->frame && coder->frame->next) {
        struct sp_frame *tmpframe = coder->frame->next;

        frame_del(coder->frame);
        coder->frame = tmpframe;
        coder->stack = coder->frame->stack;
    }
}

long
sp_code_jmpaddr(long int curpos, long int jmpaddr)
{
    return (curpos + jmpaddr);
}

void
spo_call(struct sp_coder *coder)
{
    struct opvar *addr;
    struct opvar *params;
    struct sp_frame *tmpframe;

    if (!OV_pop_i(addr) || !OV_pop_i(params)) {
        return;
    }
    if (OV_i(params) < 0) {
        return;
    }

    tmpframe = frame_new(sp_code_jmpaddr(coder->frame->n_opcode, OV_i(addr)-1));

    while (OV_i(params)-- > 0) {
        splev_stack_push(tmpframe->stack, splev_stack_getdat_any(coder));
    }
    splev_stack_reverse(tmpframe->stack);

    /* push a frame */
    tmpframe->next = coder->frame;
    coder->frame = tmpframe;

    opvar_free(addr);
    opvar_free(params);
}

void
spo_return(struct sp_coder *coder)
{
    struct opvar *params;

    if (!coder->frame || !coder->frame->next) {
        panic("return: no frame.");
    }
    if (!OV_pop_i(params)) {
        return;
    }
    if (OV_i(params) < 0) {
        return;
    }

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
spo_message(struct sp_coder *coder)
{
    struct opvar *op;
    char *msg, *levmsg;
    int old_n, n;

    if (!OV_pop_s(op)) {
        return;
    }

    msg = OV_s(op);
    if (!msg) {
        return;
    }

    old_n = lev_message ? (strlen(lev_message)+1) : 0;
    n = strlen(msg);

    levmsg = (char *) alloc(old_n+n+1);
    if (!levmsg) {
        panic("spo_message alloc");
    }
    if (old_n) {
        levmsg[old_n-1] = '\n';
    }
    if (lev_message) {
        (void) memcpy((genericptr_t)levmsg, (genericptr_t)lev_message, old_n-1);
    }
    (void) memcpy((genericptr_t)&levmsg[old_n], msg, n);
    levmsg[old_n+n] = '\0';
    Free(lev_message);
    lev_message = levmsg;
    opvar_free(op);
}

void
spo_monster(struct sp_coder *coder)
{
    int nparams = 0;

    struct opvar *varparam;
    struct opvar *id, *coord, *has_inv;
    monster tmpmons = { 0 };

    tmpmons.peaceful = -1;
    tmpmons.asleep = -1;
    tmpmons.name.str = (char *)0;
    tmpmons.appear = 0;
    tmpmons.appear_as.str = (char *)0;
    tmpmons.sp_amask = AM_SPLEV_RANDOM;
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
    tmpmons.waiting = 0;
    tmpmons.mm_flags = NO_MM_FLAGS;

    if (!OV_pop_i(has_inv)) {
        return;
    }

    if (!OV_pop_i(varparam)) {
        return;
    }

    while ((nparams++ < (SP_M_V_END+1)) &&
           (OV_typ(varparam) == SPOVAR_INT) &&
           (OV_i(varparam) >= 0) &&
           (OV_i(varparam) < SP_M_V_END)) {
        struct opvar *parm = NULL;

        OV_pop(parm);
        switch (OV_i(varparam)) {
        case SP_M_V_NAME:
            if ((OV_typ(parm) == SPOVAR_STRING) &&
                !tmpmons.name.str)
                tmpmons.name.str = dupstr(OV_s(parm));
            break;
        case SP_M_V_APPEAR:
            if ((OV_typ(parm) == SPOVAR_INT) &&
                !tmpmons.appear_as.str) {
                tmpmons.appear = OV_i(parm);
                opvar_free(parm);
                OV_pop(parm);
                tmpmons.appear_as.str = dupstr(OV_s(parm));
            }
            break;
        case SP_M_V_ASLEEP:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.asleep = OV_i(parm);
            }
            break;
        case SP_M_V_ALIGN:
            if (OV_typ(parm) == SPOVAR_INT) {
                int align = OV_i(parm);
                if (align >= 0) {
                    tmpmons.sp_amask = align;
                } else {
                    tmpmons.sp_amask = ralign[-align - 1];
                }
            }
            break;
        case SP_M_V_PEACEFUL:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.peaceful = OV_i(parm);
            }
            break;
        case SP_M_V_FEMALE:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.female = OV_i(parm);
            }
            break;
        case SP_M_V_INVIS:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.invis = OV_i(parm);
            }
            break;
        case SP_M_V_CANCELLED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.cancelled = OV_i(parm);
            }
            break;
        case SP_M_V_REVIVED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.revived = OV_i(parm);
            }
            break;
        case SP_M_V_AVENGE:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.avenge = OV_i(parm);
            }
            break;
        case SP_M_V_FLEEING:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.fleeing = OV_i(parm);
            }
            break;
        case SP_M_V_BLINDED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.blinded = OV_i(parm);
            }
            break;
        case SP_M_V_PARALYZED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.paralyzed = OV_i(parm);
            }
            break;
        case SP_M_V_STUNNED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.stunned = OV_i(parm);
            }
            break;
        case SP_M_V_CONFUSED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.confused = OV_i(parm);
            }
            break;
        case SP_M_V_SEENTRAPS:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpmons.seentraps = OV_i(parm);
            }
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
            OV_pop(varparam);
        }
    }

    if (!OV_pop_c(coord)) {
        panic("no monster coord?");
    }

    if (!OV_pop_typ(id, SPOVAR_MONST)) {
        panic("no mon type");
    }

    tmpmons.id = SP_MONST_PM(OV_i(id));
    tmpmons.class = SP_MONST_CLASS(OV_i(id));
    tmpmons.coord = OV_i(coord);
    tmpmons.has_invent = OV_i(has_inv);

    use_mon_rng++;
    create_monster(&tmpmons, coder->croom);
    use_mon_rng--;

    Free(tmpmons.name.str);
    Free(tmpmons.appear_as.str);

    opvar_free(id);
    opvar_free(coord);
    opvar_free(has_inv);
    opvar_free(varparam);
}

void
spo_object(struct sp_coder *coder)
{
    int nparams = 0;
    long quancnt;

    struct opvar *varparam;
    struct opvar *id, *containment;
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
    tmpobj.trapped = -1;
    tmpobj.recharged = 0;
    tmpobj.invis = 0;
    tmpobj.greased = 0;
    tmpobj.broken = 0;
    tmpobj.coord = SP_COORD_PACK_RANDOM(0);

    if (!OV_pop_i(containment)) {
        return;
    }

    if (!OV_pop_i(varparam)) {
        return;
    }

    while ((nparams++ < (SP_O_V_END+1)) &&
           (OV_typ(varparam) == SPOVAR_INT) &&
           (OV_i(varparam) >= 0) &&
           (OV_i(varparam) < SP_O_V_END)) {
        struct opvar *parm;

        OV_pop(parm);
        switch (OV_i(varparam)) {
        case SP_O_V_NAME:
            if ((OV_typ(parm) == SPOVAR_STRING) &&
                !tmpobj.name.str)
                tmpobj.name.str = dupstr(OV_s(parm));
            break;
        case SP_O_V_CORPSENM:
            if (OV_typ(parm) == SPOVAR_MONST) {
                char monclass = SP_MONST_CLASS(OV_i(parm));
                int monid = SP_MONST_PM(OV_i(parm));

                if (monid >= LOW_PM && monid < NUMMONS) {
                    tmpobj.corpsenm = monid;
                    break; /* we're done! */
                } else {
                    struct permonst *pm = (struct permonst *)0;

                    if (def_char_to_monclass(monclass) != MAXMCLASSES) {
                        pm = mkclass(def_char_to_monclass(monclass), G_NOGEN);
                    } else {
                        pm = rndmonst();
                    }
                    if (pm) {
                        tmpobj.corpsenm = monsndx(pm);
                    }
                }
            }
            break;
        case SP_O_V_CURSE:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.curse_state = OV_i(parm);
            }
            break;
        case SP_O_V_SPE:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.spe = OV_i(parm);
            }
            break;
        case SP_O_V_QUAN:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.quan = OV_i(parm);
            }
            break;
        case SP_O_V_BURIED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.buried = OV_i(parm);
            }
            break;
        case SP_O_V_LIT:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.lit = OV_i(parm);
            }
            break;
        case SP_O_V_ERODED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.eroded = OV_i(parm);
            }
            break;
        case SP_O_V_LOCKED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.locked = OV_i(parm);
            }
            break;
        case SP_O_V_TRAPPED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.trapped = OV_i(parm);
            }
            break;
        case SP_O_V_RECHARGED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.recharged = OV_i(parm);
            }
            break;
        case SP_O_V_INVIS:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.invis = OV_i(parm);
            }
            break;
        case SP_O_V_GREASED:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.greased = OV_i(parm);
            }
            break;
        case SP_O_V_BROKEN:
            if (OV_typ(parm) == SPOVAR_INT) {
                tmpobj.broken = OV_i(parm);
            }
            break;
        case SP_O_V_COORD:
            if (OV_typ(parm) != SPOVAR_COORD) {
                panic("no coord for obj?");
            }
            tmpobj.coord = OV_i(parm);
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
            OV_pop(varparam);
        }
    }

    if (!OV_pop_typ(id, SPOVAR_OBJ)) {
        panic("no obj type");
    }

    tmpobj.id = SP_OBJ_TYP(OV_i(id));
    tmpobj.class = SP_OBJ_CLASS(OV_i(id));
    tmpobj.containment = OV_i(containment);

    quancnt = (tmpobj.id > STRANGE_OBJECT) ? tmpobj.quan : 0;

    do {
        create_object(&tmpobj, coder->croom);
        quancnt--;
    } while ((quancnt > 0) &&
             ((tmpobj.id > STRANGE_OBJECT) &&
              !objects[tmpobj.id].oc_merge));

    Free(tmpobj.name.str);

    opvar_free(varparam);
    opvar_free(id);
    opvar_free(containment);
}

void
spo_level_flags(struct sp_coder *coder)
{
    struct opvar *flagdata;
    long flags;

    if (!OV_pop_i(flagdata)) {
        return;
    }
    flags = OV_i(flagdata);

    if (flags & NOTELEPORT) {
        level.flags.noteleport = 1;
    }
    if (flags & HARDFLOOR) {
        level.flags.hardfloor = 1;
    }
    if (flags & NOMMAP) {
        level.flags.nommap = 1;
    }
    if (flags & SHORTSIGHTED) {
        level.flags.shortsighted = 1;
    }
    if (flags & ARBOREAL) {
        level.flags.arboreal = 1;
    }
    if (flags & NOFLIPX) {
        coder->allow_flips &= ~1;
    }
    if (flags & NOFLIPY) {
        coder->allow_flips &= ~2;
    }
    if (flags & MAZELEVEL) {
        level.flags.is_maze_lev = 1;
    }
    if (flags & PREMAPPED) {
        coder->premapped = TRUE;
    }
    if (flags & SHROUD) {
        level.flags.hero_memory = 0;
    }
    if (flags & STORMY) {
        level.flags.stormy = 1;
    }
    if (flags & GRAVEYARD) {
        level.flags.graveyard = 1;
    }
    if (flags & ICEDPOOLS) {
        icedpools = TRUE;
    }
    if (flags & SOLIDIFY) {
        coder->solidify = TRUE;
    }
    if (flags & CORRMAZE) {
        level.flags.corrmaze = TRUE;
    }
    if (flags & CHECK_INACCESSIBLES) {
        coder->check_inaccessibles = TRUE;
    }
    if (flags & SKYMAP) {
        level.flags.sky = 1;
    }
    if (flags & FLAG_RNDVAULT) {
        coder->allow_flips = 0;
    }

    opvar_free(flagdata);
}

void
spo_initlevel(struct sp_coder *coder)
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

    splev_init_present = TRUE;

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
spo_mon_generation(struct sp_coder *coder)
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
    if (!mg) {
        panic("mongen mg alloc");
    }
    mg->override_chance = OV_i(freq);
    mg->total_mon_freq = 0;
    mg->gen_chances = NULL;
    while (OV_i(n_tuples)-- > 0) {
        struct opvar *mfreq = NULL, *is_sym, *mon = NULL;
        mgtuple = (struct mon_gen_tuple *)alloc(sizeof(struct mon_gen_tuple));
        if (!mgtuple) {
            panic("mgtuple alloc");
        }
        if (!OV_pop_i(is_sym) ||
            !OV_pop_i(mon) ||
            !OV_pop_i(mfreq)) {
            panic("oopsie when loading mon_gen chances.");
        }

        mgtuple->freq = OV_i(mfreq);
        if (OV_i(mfreq) < 1) {
            OV_i(mfreq) = 1;
        }
        mgtuple->is_sym = OV_i(is_sym);
        if (OV_i(is_sym)) {
            mgtuple->monid = def_char_to_monclass(OV_i(mon));
        } else {
            mgtuple->monid = OV_i(mon);
        }
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
spo_level_sounds(struct sp_coder *coder)
{
    struct opvar *freq, *n_tuples;
    struct lvl_sounds *mg;

    if (level.sounds) {
        impossible("level sounds already defined.");
        return;
    }

    if (!OV_pop_i(n_tuples)) {
        return;
    }

    if (OV_i(n_tuples) < 1) {
        impossible("no level sounds attached to the sound opcode?");
    }

    mg = (struct lvl_sounds *)alloc(sizeof(struct lvl_sounds));
    if (!mg) {
        panic("lvlsnds mg alloc");
    }
    mg->n_sounds = OV_i(n_tuples);
    mg->sounds = (struct lvl_sound_bite *)alloc(sizeof(struct lvl_sound_bite) * mg->n_sounds);
    if (!mg->sounds) {
        panic("lvlsnds snds alloc");
    }
    while (OV_i(n_tuples)-- > 0) {
        struct opvar *flags, *msg = NULL;

        if (!OV_pop_i(flags) || !OV_pop_s(msg)) {
            panic("oopsie when loading lvl_sound_bite.");
        }

        mg->sounds[OV_i(n_tuples)].flags = OV_i(flags);
        mg->sounds[OV_i(n_tuples)].msg = strdup(OV_s(msg));

        opvar_free(flags);
        opvar_free(msg);
    }

    if (!OV_pop_i(freq)) {
        mg->freq = 1;
    } else {
        mg->freq = OV_i(freq);
    }
    if (mg->freq < 0) {
        mg->freq = -(mg->freq);
    }

    level.sounds = mg;

    opvar_free(freq);
    opvar_free(n_tuples);
}

void
spo_engraving(struct sp_coder *coder)
{
    struct opvar *etyp, *txt, *coord;
    coordxy x, y;

    if (!OV_pop_i(etyp) ||
        !OV_pop_s(txt) ||
        !OV_pop_c(coord)) return;

    get_location_coord(&x, &y, DRY, coder->croom, OV_i(coord));
    make_engr_at(x, y, OV_s(txt), 0L, OV_i(etyp));

    opvar_free(etyp);
    opvar_free(txt);
    opvar_free(coord);
}

void
spo_mineralize(struct sp_coder *coder)
{
    struct opvar *kelp_pool, *kelp_moat, *gold_prob, *gem_prob;

    if (!OV_pop_i(gem_prob) ||
        !OV_pop_i(gold_prob) ||
        !OV_pop_i(kelp_moat) ||
        !OV_pop_i(kelp_pool)) return;

    mineralize(OV_i(kelp_pool), OV_i(kelp_moat), OV_i(gold_prob), OV_i(gem_prob), TRUE);

    opvar_free(gem_prob);
    opvar_free(gold_prob);
    opvar_free(kelp_moat);
    opvar_free(kelp_pool);
}

void
spo_room(struct sp_coder *coder)
{
    int isbigrm = FALSE;
    if (coder->n_subroom > MAX_NESTED_ROOMS) {
        panic("Too deeply nested rooms?!");
    } else {
        struct opvar *flags, *h, *w, *yalign, *xalign,
                     *y, *x, *rlit, *chance, *rtype;

        room tmproom;
        struct mkroom *tmpcr;

        if (!OV_pop_i(h) ||
            !OV_pop_i(w) ||
            !OV_pop_i(y) ||
            !OV_pop_i(x) ||
            !OV_pop_i(yalign) ||
            !OV_pop_i(xalign) ||
            !OV_pop_i(flags) ||
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
        tmproom.needfill = (OV_i(flags) & (1 << 0));
        /*tmproom.irregular = (OV_i(flags) & (1 << 1));*/
        tmproom.joined = !(OV_i(flags) & (1 << 2));

        isbigrm = ((tmproom.w * tmproom.h) > 20);

        opvar_free(x);
        opvar_free(y);
        opvar_free(w);
        opvar_free(h);
        opvar_free(xalign);
        opvar_free(yalign);
        opvar_free(rtype);
        opvar_free(chance);
        opvar_free(rlit);
        opvar_free(flags);

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
    if (gi.in_mk_themerooms && coder->opcode == SPO_ROOM && !isbigrm) {
        gt.themeroom_failed = TRUE;
    }
}

void
spo_endroom(struct sp_coder *coder)
{
    if (coder->n_subroom > 1) {
        coder->n_subroom--;
        coder->tmproomlist[coder->n_subroom] = NULL;
        coder->failed_room[coder->n_subroom] = TRUE;
    } else {
        /* no subroom, get out of top-level room */
        /* Need to ensure gxxstart/gy.ystart/gx.xsize/gy.ysize have something sensible,
           in case there's some stuff to be created outside the outermost room,
           and there's no MAP.
         */
        if (gx.xsize <= 1 && gy.ysize <= 1) {
            gx.xstart = 1;
            gy.ystart = 0;
            gx.xsize = COLNO-1;
            gy.ysize = ROWNO;
        }
    }
}

void
spo_stair(struct sp_coder *coder)
{
    coordxy x, y;
    struct opvar *up, *scoord;
    struct trap *badtrap;

    if (!OV_pop_i(up) || !OV_pop_c(scoord)) {
        return;
    }

    get_location_coord(&x, &y, DRY | STAIRS_LOC, coder->croom, OV_i(scoord));
    if ((badtrap = t_at(x, y)) != 0) {
        deltrap(badtrap);
    }
    mkstairs(x, y, (char) OV_i(up), coder->croom, !(OV_i(scoord) & SP_COORD_IS_RANDOM));
    SpLev_Map[x][y] = 1;

    opvar_free(scoord);
    opvar_free(up);
}

void
spo_ladder(struct sp_coder *coder)
{
    coordxy x, y;
    struct opvar *up, *coord;

    if (!OV_pop_i(up) ||
        !OV_pop_c(coord)) return;

    get_location_coord(&x, &y, DRY, coder->croom, OV_i(coord));

    levl[x][y].typ = LADDER;
    SpLev_Map[x][y] = 1;
    if (OV_i(up)) {
        d_level dest;

        dest.dnum = u.uz.dnum;
        dest.dlevel = u.uz.dlevel - 1;
        stairway_add(x, y, TRUE, TRUE, &dest);
        levl[x][y].ladder = LA_UP;
    } else {
        d_level dest;

        dest.dnum = u.uz.dnum;
        dest.dlevel = u.uz.dlevel + 1;
        stairway_add(x, y, FALSE, TRUE, &dest);
        levl[x][y].ladder = LA_DOWN;
    }
    opvar_free(coord);
    opvar_free(up);
}

void
spo_grave(struct sp_coder *coder)
{
    struct opvar *coord, *typ, *txt;
    coordxy x, y;
    if (!OV_pop_i(typ) ||
        !OV_pop_s(txt) ||
        !OV_pop_c(coord)) return;

    get_location_coord(&x, &y, DRY, coder->croom, OV_i(coord));

    if (isok(x, y) && !t_at(x, y)) {
        levl[x][y].typ = GRAVE;
        switch (OV_i(typ)) {
        case 2: make_grave(x, y, OV_s(txt)); break;
        case 1: make_grave(x, y, NULL); break;
        default: del_engr_at(x, y); break;
        }
    }

    opvar_free(coord);
    opvar_free(typ);
    opvar_free(txt);
}

void
spo_altar(struct sp_coder *coder)
{
    struct opvar *al, *shrine, *coord;
    altar tmpaltar;

    if (!OV_pop_i(al) ||
        !OV_pop_i(shrine) ||
        !OV_pop_c(coord)) return;

    tmpaltar.coord = OV_i(coord);
    int align = OV_i(al);
    tmpaltar.shrine = OV_i(shrine);

    if (align >= 0) {
        tmpaltar.sp_amask = align;
    } else {
        tmpaltar.sp_amask = ralign[-align - 1];
    }

    create_altar(&tmpaltar, coder->croom);

    opvar_free(coord);
    opvar_free(shrine);
    opvar_free(al);
}

void
spo_wallwalk(struct sp_coder *coder)
{
    struct opvar *coord, *fgtyp, *bgtyp, *chance;
    coordxy x, y;

    if (!OV_pop_i(chance) ||
        !OV_pop_typ(bgtyp, SPOVAR_MAPCHAR) ||
        !OV_pop_typ(fgtyp, SPOVAR_MAPCHAR) ||
        !OV_pop_c(coord)) return;

    get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(coord));
    if (!isok(x, y)) {
        return;
    }
    if (SP_MAPCHAR_TYP(OV_i(fgtyp)) >= MAX_TYPE) {
        return;
    }
    if (SP_MAPCHAR_TYP(OV_i(bgtyp)) >= MAX_TYPE) {
        return;
    }

    wallwalk_right(x, y, SP_MAPCHAR_TYP(OV_i(fgtyp)), SP_MAPCHAR_LIT(OV_i(fgtyp)), SP_MAPCHAR_TYP(OV_i(bgtyp)), OV_i(chance));

    opvar_free(coord);
    opvar_free(chance);
    opvar_free(fgtyp);
    opvar_free(bgtyp);
}

void
spo_trap(struct sp_coder *coder)
{
    struct opvar *type;
    struct opvar *coord;
    spltrap tmptrap;

    if (!OV_pop_i(type) ||
         !OV_pop_c(coord)) {
        return;
    }

    tmptrap.coord = OV_i(coord);
    tmptrap.type = OV_i(type);

    create_trap(&tmptrap, coder->croom);
    opvar_free(coord);
    opvar_free(type);
}

void
spo_gold(struct sp_coder *coder)
{
    struct opvar *coord, *amt;
    coordxy x, y;
    long amount;

    if (!OV_pop_c(coord) || !OV_pop_i(amt)) {
        return;
    }

    amount = OV_i(amt);
    get_location_coord(&x, &y, DRY, coder->croom, OV_i(coord));
    if (amount == -1) {
        amount = rnd(200);
    }
    mkgold(amount, x, y);
    opvar_free(coord);
    opvar_free(amt);
}

void
spo_corridor(struct sp_coder *coder)
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

struct opvar *
op_selection_opvar(char *nbuf)
{
    struct opvar *ov;
    char buf[(COLNO*ROWNO)+1];

    if (!nbuf) {
        (void) memset(buf, 1, sizeof(buf));
        buf[(COLNO*ROWNO)] = '\0';
        ov = opvar_new_str(buf);
    } else {
        ov = opvar_new_str(nbuf);
    }
    ov->spovartyp = SPOVAR_SEL;
    return ov;
}

coordxy
op_selection_getpoint(coordxy x, coordxy y, struct opvar *ov)
{
    if (!ov || ov->spovartyp != SPOVAR_SEL) {
        return 0;
    }
    if (x < 0 || y < 0 || x >= COLNO || y >= ROWNO) {
        return 0;
    }

    return (ov->vardata.str[COLNO*y + x] - 1);
}

void
op_selection_setpoint(coordxy x, coordxy y, struct opvar *ov, char c)
{
    if (!ov || ov->spovartyp != SPOVAR_SEL) {
        return;
    }
    if (x < 0 || y < 0 || x >= COLNO || y >= ROWNO) {
        return;
    }

    ov->vardata.str[COLNO*y + x] = (c + 1);
}

struct opvar *
op_selection_not(struct opvar *s)
{
    struct opvar *ov;
    int x, y;

    ov = op_selection_opvar(NULL);
    if (!ov) {
        return NULL;
    }

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (!op_selection_getpoint(x, y, s)) {
                op_selection_setpoint(x, y, ov, 1);
            }
        }
    }

    return ov;
}

struct opvar *
op_selection_logical_oper(struct opvar *s1, struct opvar *s2, char oper)
{
    struct opvar *ov;
    int x, y;

    ov = op_selection_opvar(NULL);
    if (!ov) {
        return NULL;
    }

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            switch (oper) {
            default:
            case '|':
                if (op_selection_getpoint(x, y, s1) || op_selection_getpoint(x, y, s2)) {
                    op_selection_setpoint(x, y, ov, 1);
                }
                break;
            case '&':
                if (op_selection_getpoint(x, y, s1) && op_selection_getpoint(x, y, s2)) {
                    op_selection_setpoint(x, y, ov, 1);
                }
                break;
            }
        }
    }

    return ov;
}

struct opvar *
op_selection_filter_mapchar(struct opvar *ov, struct opvar *mc)
{
    int x, y;
    schar mapc;
    coordxy lit;
    struct opvar *ret = op_selection_opvar(NULL);

    if (!ov || !mc || !ret) {
        return NULL;
    }

    mapc = SP_MAPCHAR_TYP(OV_i(mc));
    lit = SP_MAPCHAR_LIT(OV_i(mc));
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (op_selection_getpoint(x, y, ov) && (levl[x][y].typ == mapc)) {
                switch (lit) {
                default:
                case -2: op_selection_setpoint(x, y, ret, 1); break;
                case -1: op_selection_setpoint(x, y, ret, rn2(2)); break;
                case 0:
                case 1:
                         if (levl[x][y].lit == lit) {
                             op_selection_setpoint(x, y, ret, 1);
                         }
                         break;
                }
            }
        }
    }

    return ret;
}


void
op_selection_filter_percent(struct opvar *ov, int percent)
{
    int x, y;

    if (!ov) {
        return;
    }
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (op_selection_getpoint(x, y, ov) && (rn2(100) >= percent)) {
                op_selection_setpoint(x, y, ov, 0);
            }
        }
    }
}

static int
op_selection_rndcoord(struct opvar *ov, coordxy *x, coordxy *y, boolean removeit)
{
    int idx = 0;
    int c;
    int dx, dy;

    for (dx = 0; dx < COLNO; dx++) {
        for (dy = 0; dy < ROWNO; dy++) {
            if (isok(dx, dy) && op_selection_getpoint(dx, dy, ov)) {
                idx++;
            }
        }
    }

    if (idx) {
        c = rn2(idx);
        for (dx = 0; dx < COLNO; dx++) {
            for (dy = 0; dy < ROWNO; dy++) {
                if (isok(dx, dy) && op_selection_getpoint(dx, dy, ov)) {
                    if (!c) {
                        *x = dx;
                        *y = dy;
                        if (removeit) {
                            op_selection_setpoint(dx, dy, ov, 0);
                        }
                        return 1;
                    }
                    c--;
                }
            }
        }
    }
    *x = *y = -1;
    return 0;
}

void
op_selection_do_grow(struct opvar *ov, int dir)
{
    int x, y;
    char tmp[COLNO][ROWNO];

    if (ov->spovartyp != SPOVAR_SEL) {
        return;
    }
    if (!ov) {
        return;
    }

    (void) memset(tmp, 0, sizeof(tmp));

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            int c = 0;
            if ((dir &  W_WEST) && (x > 0) && (op_selection_getpoint(x-1, y, ov))) {
                c++;
            }
            if ((dir & (W_WEST|W_NORTH)) && (x > 0) && (y > 0) && (op_selection_getpoint(x-1, y-1, ov))) {
                c++;
            }
            if ((dir &  W_NORTH) && (y > 0) && (op_selection_getpoint(x, y-1, ov))) {
                c++;
            }
            if ((dir & (W_NORTH|W_EAST)) && (y > 0) && (x < COLNO-1) && (op_selection_getpoint(x+1, y-1, ov))) {
                c++;
            }
            if ((dir &  W_EAST) && (x < COLNO-1) && (op_selection_getpoint(x+1, y, ov))) {
                c++;
            }
            if ((dir & (W_EAST|W_SOUTH)) && (x < COLNO-1) && (y < ROWNO-1) && (op_selection_getpoint(x+1, y+1, ov))) {
                c++;
            }
            if ((dir &  W_SOUTH) && (y < ROWNO-1) && (op_selection_getpoint(x, y+1, ov))) {
                c++;
            }
            if ((dir & (W_SOUTH|W_WEST)) && (y < ROWNO-1) && (x > 0) && (op_selection_getpoint(x-1, y+1, ov))) {
                c++;
            }
            if (c) {
                tmp[x][y] = 1;
            }
        }
    }

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (tmp[x][y]) {
                op_selection_setpoint(x, y, ov, 1);
            }
        }
    }
}

static int (*selection_flood_check_func)(coordxy, coordxy);
static schar floodfillchk_match_under_typ;

static int
floodfillchk_match_accessible(coordxy x, coordxy y)
{
    return (ACCESSIBLE(levl[x][y].typ) ||
            levl[x][y].typ == SDOOR ||
            levl[x][y].typ == SCORR);
}

/* check whethere <x,y> is already in xs[],ys[] */
static boolean
sel_flood_havepoint(coordxy x, coordxy y, coordxy *xs, coordxy *ys, int n)
{
    coordxy xx = (coordxy) x, yy = (coordxy) y;

    while (n > 0) {
        --n;
        if (xs[n] == xx && ys[n] == yy) {
            return TRUE;
        }
    }
    return FALSE;
}

void
op_selection_floodfill(struct opvar *ov, coordxy x, coordxy y, boolean diagonals)
{
    struct opvar *tmp = op_selection_opvar(NULL);
#define SEL_FLOOD_STACK (COLNO*ROWNO)
#define SEL_FLOOD(nx, ny) \
    do {                                      \
        if (idx < SEL_FLOOD_STACK) {          \
            dx[idx] = (nx);                   \
            dy[idx] = (ny);                   \
            idx++;                            \
        } else {                              \
            panic(floodfill_stack_overrun);   \
        }                                     \
    } while (0)
#define SEL_FLOOD_CHKDIR(mx, my, sel) \
    do {                                                        \
        if (isok((mx), (my)) &&                                 \
            (*selection_flood_check_func)((mx), (my)) &&        \
            !op_selection_getpoint((mx), (my), (sel)) &&        \
            !sel_flood_havepoint((mx), (my), dx, dy, idx)) {    \
            SEL_FLOOD((mx), (my));                              \
        }                                                       \
    } while (0)

    static const char floodfill_stack_overrun[] = "floodfill stack overrun";
    int idx = 0;
    coordxy dx[SEL_FLOOD_STACK];
    coordxy dy[SEL_FLOOD_STACK];

    if (selection_flood_check_func == (int (*) (coordxy, coordxy)) 0) {
        opvar_free(tmp);
        return;
    }
    SEL_FLOOD(x, y);
    do {
        idx--;
        x = dx[idx];
        y = dy[idx];
        if (isok(x, y)) {
            op_selection_setpoint(x, y, ov, 1);
            op_selection_setpoint(x, y, tmp, 1);
        }
        SEL_FLOOD_CHKDIR((x + 1), y, tmp);
        SEL_FLOOD_CHKDIR((x - 1), y, tmp);
        SEL_FLOOD_CHKDIR(x, (y + 1), tmp);
        SEL_FLOOD_CHKDIR(x, (y - 1), tmp);
        if (diagonals) {
            SEL_FLOOD_CHKDIR((x + 1), (y + 1), tmp);
            SEL_FLOOD_CHKDIR((x - 1), (y - 1), tmp);
            SEL_FLOOD_CHKDIR((x - 1), (y + 1), tmp);
            SEL_FLOOD_CHKDIR((x + 1), (y - 1), tmp);
        }
    } while (idx > 0);
#undef SEL_FLOOD
#undef SEL_FLOOD_STACK
#undef SEL_FLOOD_CHKDIR
    opvar_free(tmp);
}

/* McIlroy's Ellipse Algorithm */
void
op_selection_do_ellipse(struct opvar *ov, int xc, int yc, int a, int b, int filled)
{
    /* e(x,y) = b^2*x^2 + a^2*y^2 - a^2*b^2 */
    int x = 0, y = b;
    long a2 = (long)a*a, b2 = (long)b*b;
    long crit1 = -(a2/4 + a%2 + b2);
    long crit2 = -(b2/4 + b%2 + a2);
    long crit3 = -(b2/4 + b%2);
    long t = -a2*y; /* e(x+1/2,y-1/2) - (a^2+b^2)/4 */
    long dxt = 2*b2*x, dyt = -2*a2*y;
    long d2xt = 2*b2, d2yt = 2*a2;
    long width = 1;
    long i;

    if (!ov) {
        return;
    }

    filled = !filled;

    if (!filled) {
        while (y>=0 && x<=a) {
            op_selection_setpoint(xc+x, yc+y, ov, 1);
            if (x!=0 || y!=0) {
                op_selection_setpoint(xc-x, yc-y, ov, 1);
            }
            if (x!=0 && y!=0) {
                op_selection_setpoint(xc+x, yc-y, ov, 1);
                op_selection_setpoint(xc-x, yc+y, ov, 1);
            }
            if (t + b2*x <= crit1 || /* e(x+1,y-1/2) <= 0 */
                t + a2*y <= crit3) { /* e(x+1/2,y) <= 0 */
                x++; dxt += d2xt; t += dxt;
            } else if (t - a2*y > crit2) { /* e(x+1/2,y-1) > 0 */
                y--; dyt += d2yt; t += dyt;
            } else {
                x++; dxt += d2xt; t += dxt;
                y--; dyt += d2yt; t += dyt;
            }
        }
    } else {
        while (y >= 0 && x <= a) {
            if (t + b2*x <= crit1 || /* e(x+1,y-1/2) <= 0 */
                t + a2*y <= crit3) { /* e(x+1/2,y) <= 0 */
                x++; dxt += d2xt; t += dxt;
                width += 2;
            } else if (t - a2*y > crit2) { /* e(x+1/2,y-1) > 0 */
                for (i = 0; i < width; i++) {
                    op_selection_setpoint(xc-x+i, yc-y, ov, 1);
                }
                if (y != 0) {
                    for (i = 0; i < width; i++) {
                        op_selection_setpoint(xc-x+i, yc+y, ov, 1);
                    }
                }
                y--; dyt += d2yt; t += dyt;
            } else {
                for (i = 0; i < width; i++) {
                    op_selection_setpoint(xc-x+i, yc-y, ov, 1);
                }
                if (y != 0) {
                    for (i = 0; i < width; i++) {
                        op_selection_setpoint(xc-x+i, yc+y, ov, 1);
                    }
                }
                x++; dxt += d2xt; t += dxt;
                y--; dyt += d2yt; t += dyt;
                width += 2;
            }
        }
    }
}

/* distance from line segment (x1,y1, x2,y2) to point (x3,y3) */
long
line_dist_coord(long int x1, long int y1, long int x2, long int y2, long int x3, long int y3)
{
    long px = x2-x1;
    long py = y2-y1;

    if (x1 == x2 && y1 == y2) {
        return isqrt(dist2(x1, y1, x3, y3));
    }

    long s = px*px + py*py;
    float u = ((x3 - x1) * px + (y3 - y1) * py) / (float)s;

    if (u > 1) {
        u = 1;
    } else if (u < 0) {
        u = 0;
    }

    long x = x1 + u * px;
    long y = y1 + u * py;
    long dx = x - x3;
    long dy = y - y3;
    long dist = isqrt(dx*dx + dy*dy);

    return dist;
}

void
op_selection_do_gradient(struct opvar *ov, long int x, long int y, long int x2, long int y2, long int gtyp, long int mind, long int maxd, long int limit)
{
    long dx, dy, dofs;

    if (mind > maxd) {
        long tmp = mind;
        mind = maxd;
        maxd = tmp;
    }

    dofs = maxd - mind;
    if (dofs < 1) {
        dofs = 1;
    }

    switch (gtyp) {
    default:
    case SEL_GRADIENT_RADIAL:
    {
        for (dx = 0; dx < COLNO; dx++) {
            for (dy = 0; dy < ROWNO; dy++) {
                long d = line_dist_coord(x, y, x2, y2, dx, dy);
                if (d >= mind && (!limit || (d <= maxd))) {
                    if ((d - mind) > rn2(dofs)) {
                        op_selection_setpoint(dx, dy, ov, 1);
                    }
                }
            }
        }
    }
    break;
    case SEL_GRADIENT_SQUARE:
    {
        for (dx = 0; dx < COLNO; dx++) {
            for (dy = 0; dy < ROWNO; dy++) {
                long d1 = line_dist_coord(x, y, x2, y2, x, dy);
                long d2 = line_dist_coord(x, y, x2, y2, dx, y);
                long d3 = line_dist_coord(x, y, x2, y2, x2, dy);
                long d4 = line_dist_coord(x, y, x2, y2, dx, y2);
                long d5 = line_dist_coord(x, y, x2, y2, dx, dy);
                long d = min(d5, min(max(d1, d2), max(d3, d4)));

                if (d >= mind && (!limit || (d <= maxd))) {
                    if ((d - mind) > rn2(dofs)) {
                        op_selection_setpoint(dx, dy, ov, 1);
                    }
                }
            }
        }
    }
    break;
    }
}

/* bresenham line algo */
void
op_selection_do_line(coordxy x1, schar y1, coordxy x2, schar y2, struct opvar *ov)
{
    int d, dx, dy, ai, bi, xi, yi;

    if (x1 < x2) {
        xi = 1;
        dx = x2 - x1;
    } else {
        xi = -1;
        dx = x1 - x2;
    }

    if (y1 < y2) {
        yi = 1;
        dy = y2 - y1;
    } else {
        yi = -1;
        dy = y1 - y2;
    }

    op_selection_setpoint(x1, y1, ov, 1);

    if (dx > dy) {
        ai = (dy - dx) * 2;
        bi = dy * 2;
        d  = bi - dx;
        do {
            if (d >= 0) {
                y1 += yi;
                d += ai;
            } else {
                d += bi;
            }
            x1 += xi;
            op_selection_setpoint(x1, y1, ov, 1);
        } while (x1 != x2);
    } else {
        ai = (dx - dy) * 2;
        bi = dx * 2;
        d  = bi - dy;
        do {
            if (d >= 0) {
                x1 += xi;
                d += ai;
            } else {
                d += bi;
            }
            y1 += yi;
            op_selection_setpoint(x1, y1, ov, 1);
        } while (y1 != y2);
    }
}

void
op_selection_do_randline(coordxy x1, schar y1, coordxy x2, schar y2, schar rough, schar rec, struct opvar *ov)
{
    int mx, my;
    int dx, dy;

    if (rec < 1) {
        return;
    }

    if ((x2 == x1) && (y2 == y1)) {
        op_selection_setpoint(x1, y1, ov, 1);
        return;
    }

    if (rough > max(abs(x2-x1), abs(y2-y1))) {
        rough = max(abs(x2-x1), abs(y2-y1));
    }

    if (rough < 2) {
        mx = ((x1 + x2) / 2);
        my = ((y1 + y2) / 2);
    } else {
        do {
            dx = rn2(rough) - (rough / 2);
            dy = rn2(rough) - (rough / 2);
            mx = ((x1 + x2) / 2) + dx;
            my = ((y1 + y2) / 2) + dy;
        } while ((mx > COLNO-1 || mx < 0 || my < 0 || my > ROWNO-1));
    }

    op_selection_setpoint(mx, my, ov, 1);

    rough = (rough * 2) / 3;

    rec--;

    op_selection_do_randline(x1, y1, mx, my, rough, rec, ov);
    op_selection_do_randline(mx, my, x2, y2, rough, rec, ov);
}

void
op_selection_iterate(struct opvar *ov, void (*func) (coordxy, coordxy, genericptr_t), genericptr_t arg)
{
    int x, y;
    /* yes, this is very naive, but it's not _that_ expensive. */
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (op_selection_getpoint(x, y, ov)) {
                (*func)(x, y, arg);
            }
        }
    }
}

void
sel_set_ter(coordxy x, coordxy y, genericptr_t arg)
{
    if (levl[x][y].typ == STAIRS) {
        return;
    }
    terrain terr;

    terr = (*(terrain *)arg);
    SET_TYPLIT(x, y, terr.ter, terr.tlit);
    /* handle doors and secret doors */
    if (levl[x][y].typ == SDOOR || IS_DOOR(levl[x][y].typ)) {
        if (levl[x][y].typ == SDOOR) {
            levl[x][y].doormask = D_CLOSED;
        }
        if (x && (IS_WALL(levl[x-1][y].typ) ||
                  levl[x-1][y].horizontal))
            levl[x][y].horizontal = 1;
    }
}

void
sel_set_feature(coordxy x, coordxy y, genericptr_t arg)
{
    if (IS_FURNITURE(levl[x][y].typ)) {
        return;
    }
    levl[x][y].typ = (*(int *)arg);
}

void
sel_set_door(coordxy dx, coordxy dy, genericptr_t arg)
{
    coordxy typ = (*(coordxy *)arg);
    coordxy x = dx;
    coordxy y = dy;

    /*get_location(&x, &y, DRY, (struct mkroom *)0);*/
    if (!IS_DOOR(levl[x][y].typ) && levl[x][y].typ != SDOOR) {
        levl[x][y].typ = (typ & D_SECRET) ? SDOOR : DOOR;
    }
    if (typ & D_SECRET) {
        typ &= ~D_SECRET;
        if (typ < D_CLOSED) {
            typ = D_CLOSED;
        }
    }
    set_door_orientation(x, y); /* set/clear levl[x][y].horizontal */
    levl[x][y].doormask = typ;
    SpLev_Map[x][y] = 1;
}

void
spo_door(struct sp_coder *coder)
{
    struct opvar *msk, *sel;
    coordxy typ;

    if (!OV_pop_i(msk) ||
        !OV_pop_typ(sel, SPOVAR_SEL)) return;

    typ = OV_i(msk) == -1 ? rnddoor() : (coordxy)OV_i(msk);

    op_selection_iterate(sel, sel_set_door, (genericptr_t)&typ);

    opvar_free(sel);
    opvar_free(msk);
}

void
spo_feature(struct sp_coder *coder)
{
    struct opvar *sel;
    int typ = -1;

    if (!OV_pop_typ(sel, SPOVAR_SEL)) {
        return;
    }

    switch (coder->opcode) {
    default: impossible("spo_feature called with wrong opcode %i.", coder->opcode); break;
    case SPO_FOUNTAIN: typ = FOUNTAIN; break;
    case SPO_SINK:     typ = SINK;     break;
    case SPO_POOL:     typ = POOL;     break;
    }
    if (typ >= 0) {
        op_selection_iterate(sel, sel_set_feature, (genericptr_t)&typ);
    }
    opvar_free(sel);
}

void
spo_terrain(struct sp_coder *coder)
{
    terrain tmpterrain;
    struct opvar *ter, *sel;

    if (!OV_pop_typ(ter, SPOVAR_MAPCHAR) ||
        !OV_pop_typ(sel, SPOVAR_SEL)) return;

    tmpterrain.ter = SP_MAPCHAR_TYP(OV_i(ter));
    tmpterrain.tlit = SP_MAPCHAR_LIT(OV_i(ter));
    op_selection_iterate(sel, sel_set_ter, (genericptr_t)&tmpterrain);

    opvar_free(ter);
    opvar_free(sel);
}

void
spo_replace_terrain(struct sp_coder *coder)
{
    replaceterrain rt;
    struct opvar *reg, *from_ter, *to_ter, *chance;

    if (!OV_pop_i(chance) ||
        !OV_pop_typ(to_ter, SPOVAR_MAPCHAR) ||
        !OV_pop_typ(from_ter, SPOVAR_MAPCHAR) ||
        !OV_pop_r(reg)) return;

    rt.chance = OV_i(chance);
    rt.tolit = SP_MAPCHAR_LIT(OV_i(to_ter));
    rt.toter = SP_MAPCHAR_TYP(OV_i(to_ter));
    rt.fromter = SP_MAPCHAR_TYP(OV_i(from_ter));
    /* TODO: use SP_MAPCHAR_LIT(OV_i(from_ter)) too */
    rt.x1 = SP_REGION_X1(OV_i(reg));
    rt.y1 = SP_REGION_Y1(OV_i(reg));
    rt.x2 = SP_REGION_X2(OV_i(reg));
    rt.y2 = SP_REGION_Y2(OV_i(reg));

    replace_terrain(&rt, coder->croom);

    opvar_free(reg);
    opvar_free(from_ter);
    opvar_free(to_ter);
    opvar_free(chance);
}

static boolean
generate_way_out_method(int nx, int ny, struct opvar *ov)
{
    const int escapeitems[] = {
        PICK_AXE,
        DWARVISH_MATTOCK,
        CRYSTAL_PICK,
        WAN_DIGGING,
        WAN_TELEPORTATION,
        SCR_TELEPORTATION,
        RIN_TELEPORTATION
    };
    struct opvar *ov2 = op_selection_opvar((char *) 0), *ov3;
    coordxy x, y;
    boolean res = TRUE;

    op_selection_floodfill(ov2, nx, ny, TRUE);
    ov3 = opvar_clone(ov2);

    /* try to make a door */
    while (op_selection_rndcoord(ov3, &x, &y, TRUE)) {
        if (isok(x+1, y) &&
             !op_selection_getpoint(x+1, y, ov) && IS_WALL(levl[x+1][y].typ) &&
            isok(x+2, y) &&
              op_selection_getpoint(x+2, y, ov) && ACCESSIBLE(levl[x+2][y].typ)) {
            levl[x+1][y].typ = DOOR;
            goto gotitdone;
        }
        if (isok(x-1, y) &&
             !op_selection_getpoint(x-1, y, ov) && IS_WALL(levl[x-1][y].typ) &&
            isok(x-2, y) &&
             op_selection_getpoint(x-2, y, ov) && ACCESSIBLE(levl[x-2][y].typ)) {
            levl[x-1][y].typ = DOOR;
            goto gotitdone;
        }
        if (isok(x, y+1) &&
             !op_selection_getpoint(x, y+1, ov) && IS_WALL(levl[x][y+1].typ) &&
            isok(x, y+2) &&
             op_selection_getpoint(x, y+2, ov) && ACCESSIBLE(levl[x][y+2].typ)) {
            levl[x][y+1].typ = DOOR;
            goto gotitdone;
        }
        if (isok(x, y-1) &&
             !op_selection_getpoint(x, y-1, ov) && IS_WALL(levl[x][y-1].typ) &&
            isok(x, y-2) &&
             op_selection_getpoint(x, y-2, ov) && ACCESSIBLE(levl[x][y-2].typ)) {
            levl[x][y-1].typ = DOOR;
            goto gotitdone;
        }
    }

    /* try to make a hole or a trapdoor */
    if (Can_fall_thru(&u.uz)) {
        opvar_free(ov3);
        ov3 = opvar_clone(ov2);
        while (op_selection_rndcoord(ov3, &x, &y, TRUE)) {
            if (maketrap(x,y, rn2(2) ? HOLE : TRAPDOOR)) {
                goto gotitdone;
            }
        }
    }

    /* generate one of the escape items */
    if (op_selection_rndcoord(ov2, &x, &y, FALSE)) {
        mksobj_at(escapeitems[rn2(SIZE(escapeitems))], x, y, TRUE, FALSE);
        goto gotitdone;
    }

    res = FALSE;
 gotitdone:
    opvar_free(ov2);
    opvar_free(ov3);
    return res;
}

static void
ensure_way_out(void)
{
    struct opvar *ov = op_selection_opvar((char *) 0);
    struct trap *ttmp = ftrap;
    int x,y;
    boolean ret = TRUE;
    stairway *stway = stairs;

    set_selection_floodfillchk(floodfillchk_match_accessible);

    while (stway) {
        if (stway->tolev.dnum == u.uz.dnum) {
            op_selection_floodfill(ov, stway->sx, stway->sy, TRUE);
        }
        stway = stway->next;
    }

    while (ttmp) {
        if ((ttmp->ttyp == MAGIC_PORTAL || ttmp->ttyp == VIBRATING_SQUARE || is_hole(ttmp->ttyp)) &&
             !op_selection_getpoint(ttmp->tx, ttmp->ty, ov)) {
            op_selection_floodfill(ov, ttmp->tx, ttmp->ty, TRUE);
        }
        ttmp = ttmp->ntrap;
    }

    do {
        ret = TRUE;
        for (x = 0; x < COLNO; x++) {
            for (y = 0; y < ROWNO; y++) {
                if (ACCESSIBLE(levl[x][y].typ) && !op_selection_getpoint(x, y, ov)) {
                    if (generate_way_out_method(x,y, ov)) {
                        op_selection_floodfill(ov, x,y, TRUE);
                    }
                    ret = FALSE;
                    goto outhere;
                }
            }
        }
    outhere: ;
    } while (!ret);
    opvar_free(ov);
}

void
spo_spill(struct sp_coder *coder)
{
    spill sp;
    struct opvar *coord, *typ, *dir, *count;
    if (!OV_pop_i(dir) ||
        !OV_pop_i(count) ||
        !OV_pop_typ(typ, SPOVAR_MAPCHAR) ||
        !OV_pop_c(coord)) return;

    if (coord->vardata.l & SP_COORD_IS_RANDOM) {
        sp.x = -1;
        sp.y = -1;
    } else {
        sp.x = SP_COORD_X(OV_i(coord));
        sp.y = SP_COORD_Y(OV_i(coord));
    }

    sp.lit = SP_MAPCHAR_LIT(OV_i(typ));
    sp.count = OV_i(count);
    sp.direction = OV_i(dir);
    sp.typ = SP_MAPCHAR_TYP(OV_i(typ));

    spill_terrain(&sp, coder->croom);

    opvar_free(coord);
    opvar_free(typ);
    opvar_free(dir);
    opvar_free(count);
}

void
spo_levregion(struct sp_coder *coder)
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
    if (!tmplregion) {
        panic("levreg alloc");
    }
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
    tmplregion->rname.str = dupstr(OV_s(rname));

    if (!tmplregion->in_islev) {
        get_location(&tmplregion->inarea.x1, &tmplregion->inarea.y1,
                     ANY_LOC, (struct mkroom *)0);
        get_location(&tmplregion->inarea.x2, &tmplregion->inarea.y2,
                     ANY_LOC, (struct mkroom *)0);
    }

    if (!tmplregion->del_islev) {
        get_location(&tmplregion->delarea.x1, &tmplregion->delarea.y1,
                     ANY_LOC, (struct mkroom *)0);
        get_location(&tmplregion->delarea.x2, &tmplregion->delarea.y2,
                     ANY_LOC, (struct mkroom *)0);
    }
    if (num_lregions) {
        /* realloc the lregion space to add the new one */
        lev_region *newl = (lev_region *) alloc(sizeof(lev_region) *
                                                (unsigned)(1+num_lregions));
        if (!newl) {
            panic("levreg newl alloc");
        }
        (void) memcpy((genericptr_t)(newl), (genericptr_t)lregions,
                      sizeof(lev_region) * num_lregions);
        Free(lregions);
        num_lregions++;
        lregions = newl;
    } else {
        num_lregions = 1;
        lregions = (lev_region *) alloc(sizeof(lev_region));
        if (!lregions) {
            panic("lregions alloc");
        }
    }
    (void) memcpy(&lregions[num_lregions-1], tmplregion, sizeof(lev_region));
    free(tmplregion);

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
spo_region(struct sp_coder *coder)
{
    struct opvar *rtype, *rlit, *flags, *area;
    coordxy dx1, dy1, dx2, dy2;
    struct mkroom *troom;
    boolean prefilled, room_not_needed, irregular, joined;

    if (!OV_pop_i(flags) ||
        !OV_pop_i(rtype) ||
        !OV_pop_i(rlit) ||
        !OV_pop_r(area)) return;

    prefilled = !(OV_i(flags) & (1 << 0));
    irregular = (OV_i(flags) & (1 << 1));
    joined = !(OV_i(flags) & (1 << 2));

    if (OV_i(rtype) > MAXRTYPE) {
        OV_i(rtype) -= MAXRTYPE+1;
        prefilled = TRUE;
    } else {
        prefilled = FALSE;
    }

    if (OV_i(rlit) < 0) {
        OV_i(rlit) = (rnd(1 + abs(depth(&u.uz))) < 11 && rn2(77)) ? TRUE : FALSE;
    }

    dx1 = SP_REGION_X1(OV_i(area));
    dy1 = SP_REGION_Y1(OV_i(area));
    dx2 = SP_REGION_X2(OV_i(area));
    dy2 = SP_REGION_Y2(OV_i(area));

    get_location(&dx1, &dy1, ANY_LOC, (struct mkroom *)0);
    get_location(&dx2, &dy2, ANY_LOC, (struct mkroom *)0);

    /* for an ordinary room, `prefilled' is a flag to force
       an actual room to be created (such rooms are used to
       control placement of migrating monster arrivals) */
    room_not_needed = (OV_i(rtype) == OROOM &&
                       !irregular && !prefilled && !gi.in_mk_themerooms);
    if (room_not_needed || nroom >= MAXNROFROOMS) {
        region tmpregion;
        if (!room_not_needed) {
            impossible("Too many rooms on new level!");
        }
        tmpregion.rlit = OV_i(rlit);
        tmpregion.x1 = dx1;
        tmpregion.y1 = dy1;
        tmpregion.x2 = dx2;
        tmpregion.y2 = dy2;
        light_region(&tmpregion);

        opvar_free(area);
        opvar_free(flags);
        opvar_free(rlit);
        opvar_free(rtype);

        return;
    }

    troom = &rooms[nroom];

    /* mark rooms that must be filled, but do it later */
    if (OV_i(rtype) != OROOM) {
        troom->needfill = (prefilled ? 2 : 1);
    }

    troom->needjoining = joined;

    if (irregular) {
        min_rx = max_rx = dx1;
        min_ry = max_ry = dy1;
        smeq[nroom] = nroom;
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
        topologize(troom, FALSE);         /* set roomno */
#else
        topologize(troom);                /* set roomno */
#endif
    }

    if (gi.in_mk_themerooms && prefilled) {
        troom->needfill = 1;
    }

    if (!room_not_needed) {
        if (coder->n_subroom > 1) {
            impossible("region as subroom");
        } else {
            coder->tmproomlist[coder->n_subroom] = troom;
            coder->failed_room[coder->n_subroom] = FALSE;
            coder->n_subroom++;
        }
    }

    opvar_free(area);
    opvar_free(flags);
    opvar_free(rlit);
    opvar_free(rtype);
}

void
spo_drawbridge(struct sp_coder *coder)
{
    coordxy x, y;
    int dopen;
    struct opvar *dir, *db_open, *coord;

    if (!OV_pop_i(dir) ||
        !OV_pop_i(db_open) ||
        !OV_pop_c(coord)) return;

    get_location_coord(&x, &y, DRY|WET|HOT, coder->croom, OV_i(coord));
    if ((dopen = OV_i(db_open)) == -1) {
        dopen = !rn2(2);
    }
    if (!create_drawbridge(x, y, OV_i(dir), dopen ? TRUE : FALSE)) {
        impossible("Cannot create drawbridge.");
    }
    SpLev_Map[x][y] = 1;

    opvar_free(coord);
    opvar_free(db_open);
    opvar_free(dir);
}

void
spo_mazewalk(struct sp_coder *coder)
{
    coordxy x, y;
    struct opvar *ftyp, *fstocked, *fdir, *coord;
    int dir;

    if (!OV_pop_i(ftyp) ||
        !OV_pop_i(fstocked) ||
        !OV_pop_i(fdir) ||
        !OV_pop_c(coord)) return;

    dir = OV_i(fdir);

    get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(coord));
    if (!isok(x, y)) {
        return;
    }

    if (OV_i(ftyp) < 1) {
        OV_i(ftyp) = level.flags.corrmaze ? CORR : ROOM;
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

    if (!IS_DOOR(levl[x][y].typ)) {
        levl[x][y].typ = OV_i(ftyp);
        levl[x][y].flags = 0;
    }

    /*
     * We must be sure that the parity of the coordinates for
     * walkfrom() is odd.  But we must also take into account
     * what direction was chosen.
     */
    if (!(x % 2)) {
        if (dir == W_EAST) {
            x++;
        } else {
            x--;
        }

        /* no need for IS_DOOR check; out of map bounds */
        levl[x][y].typ = OV_i(ftyp);
        levl[x][y].flags = 0;
    }

    if (!(y % 2)) {
        if (dir == W_SOUTH) {
            y++;
        } else {
            y--;
        }
    }

    walkfrom(x, y, OV_i(ftyp));
    if (OV_i(fstocked)) {
        fill_empty_maze();
    }

    opvar_free(coord);
    opvar_free(fdir);
    opvar_free(fstocked);
    opvar_free(ftyp);
}

void
spo_wall_property(struct sp_coder *coder)
{
    struct opvar *r;
    coordxy dx1, dy1, dx2, dy2;
    int wprop = (coder->opcode == SPO_NON_DIGGABLE) ? W_NONDIGGABLE : W_NONPASSWALL;

    if (!OV_pop_r(r)) {
        return;
    }

    dx1 = SP_REGION_X1(OV_i(r));
    dy1 = SP_REGION_Y1(OV_i(r));
    dx2 = SP_REGION_X2(OV_i(r));
    dy2 = SP_REGION_Y2(OV_i(r));

    get_location(&dx1, &dy1, ANY_LOC, (struct mkroom *)0);
    get_location(&dx2, &dy2, ANY_LOC, (struct mkroom *)0);

    set_wall_property(dx1, dy1, dx2, dy2, wprop);

    opvar_free(r);
}

void
spo_room_door(struct sp_coder *coder)
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
sel_set_wallify(coordxy x, coordxy y, void *arg UNUSED)
{
    wallify_map(x, y, x, y);
}

void
spo_wallify(struct sp_coder *coder)
{
    struct opvar *typ, *r;
    int dx1, dy1, dx2, dy2;

    if (!OV_pop_i(typ)) {
        return;
    }
    switch (OV_i(typ)) {
    default:
    case 0:
    {
        if (!OV_pop_r(r)) {
            return;
        }
        dx1 = (coordxy)SP_REGION_X1(OV_i(r));
        dy1 = (coordxy)SP_REGION_Y1(OV_i(r));
        dx2 = (coordxy)SP_REGION_X2(OV_i(r));
        dy2 = (coordxy)SP_REGION_Y2(OV_i(r));
        wallify_map(dx1 < 0 ? (gx.xstart - 1) : dx1,
                    dy1 < 0 ? (gy.ystart - 1) : dy1,
                    dx2 < 0 ? (gx.xstart + gx.xsize + 1) : dx2,
                    dy2 < 0 ? (gy.ystart + gy.ysize + 1) : dy2);
    }
    break;
    case 1:
    {
        if (!OV_pop_typ(r, SPOVAR_SEL)) {
            return;
        }
        op_selection_iterate(r, sel_set_wallify, NULL);
    }
    break;
    }
    opvar_free(r);
    opvar_free(typ);
}

void
spo_map(struct sp_coder *coder)
{
    mazepart tmpmazepart;
    struct opvar *mpxs, *mpys, *mpmap, *mpa, *mpkeepr, *mpzalign;
    coordxy halign, valign;
    coordxy tmpxstart, tmpystart, tmpxsize, tmpysize;
    int tryct = 0;
    unpacked_coord upc;

    if (!OV_pop_i(mpxs) ||
        !OV_pop_i(mpys) ||
        !OV_pop_s(mpmap) ||
        !OV_pop_i(mpkeepr) ||
        !OV_pop_i(mpzalign) ||
        !OV_pop_c(mpa)) return;

redo_maploc:

    tmpmazepart.xsize = OV_i(mpxs);
    tmpmazepart.ysize = OV_i(mpys);
    tmpmazepart.zaligntyp = OV_i(mpzalign);

    upc = get_unpacked_coord(OV_i(mpa), ANY_LOC);
    tmpmazepart.halign = upc.x;
    tmpmazepart.valign = upc.y;

    tmpxsize = gx.xsize; tmpysize = gy.ysize;
    tmpxstart = gx.xstart; tmpystart = gy.ystart;

    halign = tmpmazepart.halign;
    valign = tmpmazepart.valign;
    gx.xsize = tmpmazepart.xsize;
    gy.ysize = tmpmazepart.ysize;
    switch (tmpmazepart.zaligntyp) {
    default:
    case 0:
        break;
    case 1:
        switch ((int) halign) {
        case LEFT:    gx.xstart = splev_init_present ? 1 : 3;   break;
        case H_LEFT:  gx.xstart = 2 + ((x_maze_max - 2 - gx.xsize) / 4);     break;
        case CENTER:  gx.xstart = 2 + ((x_maze_max - 2 - gx.xsize) / 2);     break;
        case H_RIGHT: gx.xstart = 2 + ((x_maze_max - 2 - gx.xsize) * 3 / 4); break;
        case RIGHT:   gx.xstart = x_maze_max - gx.xsize - 1;                 break;
        }
        switch ((int) valign) {
        case TOP:     gy.ystart = 3;                                     break;
        case CENTER:  gy.ystart = 2 + ((y_maze_max - 2 - gy.ysize) / 2); break;
        case BOTTOM:  gy.ystart = y_maze_max - gy.ysize - 1;                break;
        }
        if (!(gx.xstart % 2)) {
            gx.xstart++;
        }
        if (!(gy.ystart % 2)) {
            gy.ystart++;
        }
        break;
    case 2:
        if (!coder->croom) {
            gx.xstart = 1;
            gy.ystart = 0;
            gx.xsize = COLNO-1-tmpmazepart.xsize;
            gy.ysize = ROWNO-tmpmazepart.ysize;
        }
        get_location_coord(&halign, &valign, ANY_LOC, coder->croom, OV_i(mpa));
        gx.xsize = tmpmazepart.xsize;
        gy.ysize = tmpmazepart.ysize;
        gx.xstart = halign;
        gy.ystart = valign;
        break;
    }
    if ((gy.ystart < 0) || (gy.ystart + gy.ysize > ROWNO)) {
        if (gi.in_mk_themerooms) {
            coder->exit_script = TRUE;
            goto skipmap;
        }
        /* try to move the start a bit */
        gy.ystart += (gy.ystart > 0) ? -2 : 2;
        if (gy.ysize == ROWNO) {
            gy.ystart = 0;
        }
        if (gy.ystart < 0 || gy.ystart + gy.ysize > ROWNO) {
            panic("reading special level with gy.ysize too large");
        }
    }
    if (gx.xsize <= 1 && gy.ysize <= 1) {
        gx.xstart = 1;
        gy.ystart = 0;
        gx.xsize = COLNO-1;
        gy.ysize = ROWNO;
    } else {
        coordxy x, y;
        /* random vault should never overwrite anything */
        if (gi.in_mk_themerooms) {
            boolean isokp = TRUE;
            for (y = gy.ystart - 1; y < gy.ystart + gy.ysize + 1; y++) {
                for (x = gx.xstart - 1; x < gx.xstart + gx.xsize + 1; x++) {
                    coordxy mptyp;
                    if (!isok(x, y)) {
                        isokp = FALSE;
                    } else if (y < gy.ystart || y >= (gy.ystart + gy.ysize) ||
                               x < gx.xstart || x >= (gx.xstart + gx.xsize)) {
                        if (levl[x][y].typ != STONE) {
                            isokp = FALSE;
                        }
                        if (levl[x][y].roomno != NO_ROOM) {
                            isokp = FALSE;
                        }
                    } else {
                        mptyp = (mpmap->vardata.str[(y - gy.ystart) * gx.xsize + (x - gx.xstart + 1)] - 1);
                        if (mptyp >= MAX_TYPE) {
                            continue;
                        }
                        if (isok(x, y)) {
                            if (levl[x][y].typ != STONE && levl[x][y].typ != mptyp) {
                                isokp = FALSE;
                            }
                            if (levl[x][y].roomno != NO_ROOM) {
                                isokp = FALSE;
                            }
                        } else {
                            isokp = FALSE;
                        }
                    }
                    if (!isokp) {
                        if ((tryct++ < 100) && (tmpmazepart.zaligntyp == 2) &&
                            ((tmpmazepart.halign < 0) || (tmpmazepart.valign < 0)) /* rnd pos */ )
                            goto redo_maploc;
                        if (!((gx.xsize * gy.ysize) > 20)) { /* !isbig() */
                            gt.themeroom_failed = TRUE;
                        }
                        coder->exit_script = TRUE;
                        goto skipmap;
                    }
                }
            }
        }
        /* Load the map */
        for (y = gy.ystart; y < gy.ystart + gy.ysize; y++) {
            for (x = gx.xstart; x < gx.xstart + gx.xsize; x++) {
                coordxy mptyp = (mpmap->vardata.str[(y - gy.ystart) * gx.xsize + (x - gx.xstart)] - 1);
                if (mptyp >= MAX_TYPE) {
                    continue;
                }
                levl[x][y].typ = mptyp;
                levl[x][y].lit = FALSE;
                /* clear out levl: load_common_data may set them */
                levl[x][y].flags = 0;
                levl[x][y].horizontal = 0;
                levl[x][y].roomno = 0;
                levl[x][y].edge = 0;
                SpLev_Map[x][y] = 1;
                /*
                 *  Set secret doors to closed (why not trapped too?).  Set
                 *  the horizontal bit.
                 */
                if (levl[x][y].typ == SDOOR || IS_DOOR(levl[x][y].typ)) {
                    if (levl[x][y].typ == SDOOR) {
                        levl[x][y].doormask = D_CLOSED;
                    }
                    /*
                     *  If there is a wall to the left that connects to a
                     *  (secret) door, then it is horizontal.  This does
                     *  not allow (secret) doors to be corners of rooms.
                     */
                    if (x != gx.xstart && (IS_WALL(levl[x-1][y].typ) ||
                                        levl[x-1][y].horizontal))
                        levl[x][y].horizontal = 1;
                } else if (levl[x][y].typ == HWALL ||
                          levl[x][y].typ == IRONBARS) {
                    levl[x][y].horizontal = 1;
                } else if (levl[x][y].typ == LAVAPOOL) {
                    levl[x][y].lit = 1;
                } else if (splev_init_present && levl[x][y].typ == ICE) {
                    levl[x][y].icedpool = icedpools ? ICED_POOL : ICED_MOAT;
                }
            }
        }
        if (coder->lvl_is_joined && !gi.in_mk_themerooms) {
            remove_rooms(gx.xstart, gy.ystart, gx.xstart + gx.xsize, gy.ystart + gy.ysize);
        }
    }
    if (!OV_i(mpkeepr)) {
        gx.xstart = tmpxstart; gy.ystart = tmpystart;
        gx.xsize = tmpxsize; gy.ysize = tmpysize;
    }

skipmap:
    opvar_free(mpxs);
    opvar_free(mpys);
    opvar_free(mpmap);
    opvar_free(mpa);
    opvar_free(mpkeepr);
    opvar_free(mpzalign);
}

void
spo_jmp(struct sp_coder *coder, sp_lev *lvl)
{
    struct opvar *tmpa;
    long a;

    if (!OV_pop_i(tmpa)) {
        return;
    }
    a = sp_code_jmpaddr(coder->frame->n_opcode, (OV_i(tmpa) - 1));
    if ((a >= 0) && (a < lvl->n_opcodes) &&
        (a != coder->frame->n_opcode))
        coder->frame->n_opcode = a;
    opvar_free(tmpa);
}

void
spo_conditional_jump(struct sp_coder *coder, sp_lev *lvl)
{
    struct opvar *oa, *oc;
    long a, c;
    int test = 0;

    if (!OV_pop_i(oa) || !OV_pop_i(oc)) {
        return;
    }

    a = sp_code_jmpaddr(coder->frame->n_opcode, (OV_i(oa) - 1));
    c = OV_i(oc);

    switch (coder->opcode) {
    default: impossible("spo_conditional_jump: illegal opcode"); break;
    case SPO_JL:  test = (c & SP_CPUFLAG_LT); break;
    case SPO_JLE: test = (c & (SP_CPUFLAG_LT|SP_CPUFLAG_EQ)); break;
    case SPO_JG:  test = (c & SP_CPUFLAG_GT); break;
    case SPO_JGE: test = (c & (SP_CPUFLAG_GT|SP_CPUFLAG_EQ)); break;
    case SPO_JE:  test = (c & SP_CPUFLAG_EQ); break;
    case SPO_JNE: test = (c & ~SP_CPUFLAG_EQ); break;
    }

    if ((test) && (a >= 0) &&
        (a < lvl->n_opcodes) &&
        (a != coder->frame->n_opcode))
        coder->frame->n_opcode = a;

    opvar_free(oa);
    opvar_free(oc);
}


void
spo_var_init(struct sp_coder *coder)
{
    struct opvar *vname;
    struct opvar *arraylen;
    struct opvar *vvalue;
    struct splev_var *tmpvar;
    struct splev_var *tmp2;
    long idx;

    OV_pop_s(vname);
    OV_pop_i(arraylen);

    if (!vname || !arraylen) {
        panic("no values for SPO_VAR_INIT");
    }

    tmpvar = opvar_var_defined(coder, OV_s(vname));

    if (tmpvar) {
        /* variable redefinition */
        if (OV_i(arraylen) < 0) {
            /* copy variable */
            if (tmpvar->array_len) {
                idx = tmpvar->array_len;
                while (idx-- > 0) {
                    opvar_free(tmpvar->data.arrayvalues[idx]);
                }
                Free(tmpvar->data.arrayvalues);
            } else {
                opvar_free(tmpvar->data.value);
            }
            tmpvar->data.arrayvalues = NULL;
            goto copy_variable;
        } else if (OV_i(arraylen)) {
            /* redefined array */
            idx = tmpvar->array_len;
            while (idx-- > 0) {
                opvar_free(tmpvar->data.arrayvalues[idx]);
            }
            Free(tmpvar->data.arrayvalues);
            tmpvar->data.arrayvalues = NULL;
            goto create_new_array;
        } else {
            /* redefined single value */
            OV_pop(vvalue);
            if (tmpvar->svtyp != vvalue->spovartyp) {
                panic("redefining variable as different type");
            }
            opvar_free(tmpvar->data.value);
            tmpvar->data.value = vvalue;
            tmpvar->array_len = 0;
        }
    } else {
        /* new variable definition */
        tmpvar = (struct splev_var *)malloc(sizeof(struct splev_var));
        if (!tmpvar) {
            panic("newvar tmpvar alloc");
        }
        tmpvar->next = coder->frame->variables;
        tmpvar->name = strdup(OV_s(vname));
        coder->frame->variables = tmpvar;

        if (OV_i(arraylen) < 0) {
            /* copy variable */
copy_variable:
            OV_pop(vvalue);
            tmp2 = opvar_var_defined(coder, OV_s(vvalue));
            if (!tmp2) {
                panic("no copyable var");
            }
            tmpvar->svtyp = tmp2->svtyp;
            tmpvar->array_len = tmp2->array_len;
            if (tmpvar->array_len) {
                idx = tmpvar->array_len;
                tmpvar->data.arrayvalues = (struct opvar **)alloc(sizeof(struct opvar *) * idx);
                if (!tmpvar->data.arrayvalues) {
                    panic("tmpvar->data.arrayvalues alloc");
                }
                while (idx-- > 0) {
                    tmpvar->data.arrayvalues[idx] = opvar_clone(tmp2->data.arrayvalues[idx]);
                }
            } else {
                tmpvar->data.value = opvar_clone(tmp2->data.value);
            }
            opvar_free(vvalue);
        } else if (OV_i(arraylen)) {
            /* new array */
create_new_array:
            idx = OV_i(arraylen);
            tmpvar->array_len = idx;
            tmpvar->data.arrayvalues = (struct opvar **)alloc(sizeof(struct opvar *) * idx);
            if (!tmpvar->data.arrayvalues) {
                panic("malloc tmpvar->data.arrayvalues");
            }
            while (idx-- > 0) {
                OV_pop(vvalue);
                if (!vvalue) {
                    panic("no value for arrayvariable");
                }
                tmpvar->data.arrayvalues[idx] = vvalue;
            }
            tmpvar->svtyp = SPOVAR_ARRAY;
        } else {
            /* new single value */
            OV_pop(vvalue);
            if (!vvalue) {
                panic("no value for variable");
            }
            tmpvar->svtyp = OV_typ(vvalue);
            tmpvar->data.value = vvalue;
            tmpvar->array_len = 0;
        }
    }

    opvar_free(vname);
    opvar_free(arraylen);
}


long
opvar_array_length(struct sp_coder *coder)
{
    struct opvar *vname;
    struct splev_var *tmp;
    long len = 0;

    if (!coder) {
        return 0;
    }

    vname = splev_stack_pop(coder->stack);
    if (!vname) {
        return 0;
    }
    if (vname->spovartyp != SPOVAR_VARIABLE) {
        goto pass;
    }

    tmp = coder->frame->variables;
    while (tmp) {
        if (!strcmp(tmp->name, OV_s(vname))) {
            if ((tmp->svtyp & SPOVAR_ARRAY)) {
                len = tmp->array_len;
                if (len < 1) {
                    len = 0;
                }
            }
            goto pass;
        }
        tmp = tmp->next;
    }

pass:
    opvar_free(vname);
    return len;
}


void
spo_shuffle_array(struct sp_coder *coder)
{
    struct opvar *vname;
    struct splev_var *tmp;
    struct opvar *tmp2;
    long i, j;

    if (!OV_pop_s(vname)) {
        return;
    }

    tmp = opvar_var_defined(coder, OV_s(vname));
    if (!tmp || (tmp->array_len < 1)) {
        opvar_free(vname);
        return;
    }
    for (i = tmp->array_len - 1; i > 0; i--) {
        if ((j = rn2(i + 1)) == i) {
            continue;
        }
        tmp2 = tmp->data.arrayvalues[j];
        tmp->data.arrayvalues[j] = tmp->data.arrayvalues[i];
        tmp->data.arrayvalues[i] = tmp2;
    }

    opvar_free(vname);
}

/* Special level coder, creates the special level from the sp_lev codes.
 * Does not free the allocated memory.
 */
static boolean
sp_level_coder(sp_lev *lvl)
{
    unsigned long exec_opcodes = 0;
    int tmpi;
    long room_stack = 0;
    unsigned long max_execution = SPCODER_MAX_RUNTIME;
    struct sp_coder *coder = (struct sp_coder *)alloc(sizeof(struct sp_coder));
    if (!coder) {
        panic("coder alloc");
    }
    coder->frame = frame_new(0);
    coder->stack = NULL;
    coder->premapped = FALSE;
    coder->solidify = FALSE;
    coder->check_inaccessibles = FALSE;
    coder->allow_flips = 3;
    coder->croom = NULL;
    coder->n_subroom = 1;
    coder->exit_script = FALSE;
    coder->lvl_is_joined = 0;
    gt.themeroom_failed = FALSE;

    splev_init_present = FALSE;
    icedpools = FALSE;
    /* achievement tracking; static init would suffice except we need to
       reset if #wizmakemap is used to recreate mines' end or sokoban end;
       once either level is created, these values can be forgotten */
    mines_prize_count = soko_prize_count = 0;

    if (wizard) {
        char *met = nh_getenv("SPCODER_MAX_RUNTIME");
        if (met && met[0] == '1') {
            max_execution = (1<<30) - 1;
        }
    }

    for (tmpi = 0; tmpi <= MAX_NESTED_ROOMS; tmpi++) {
        coder->tmproomlist[tmpi] = (struct mkroom *)0;
        coder->failed_room[tmpi] = FALSE;
    }

    shuffle_alignments();

    (void) memset((genericptr_t)&SpLev_Map[0][0], 0, sizeof SpLev_Map);

    level.flags.is_maze_lev = 0;

    gx.xstart = 1;
    gy.ystart = 0;
    gx.xsize = COLNO-1;
    gy.ysize = ROWNO;

    while (coder->frame->n_opcode < lvl->n_opcodes && !coder->exit_script) {
        coder->opcode = lvl->opcodes[coder->frame->n_opcode].opcode;
        coder->opdat = lvl->opcodes[coder->frame->n_opcode].opdat;

        coder->stack = coder->frame->stack;

        if (exec_opcodes++ > max_execution) {
            impossible("Level script is taking too much time, stopping.");
            coder->exit_script = TRUE;
        }

        if (gt.themeroom_failed) {
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
        case SPO_END_MONINVENT:
            spo_end_moninvent();
            break;
        case SPO_POP_CONTAINER:
            spo_pop_container();
            break;
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
        case SPO_MINERALIZE:     spo_mineralize(coder);     break;
        case SPO_SUBROOM:
        case SPO_ROOM:
            if (!coder->failed_room[coder->n_subroom-1]) {
                spo_room(coder);
            } else {
                room_stack++;
            }
            break;
        case SPO_ENDROOM:
            if (coder->failed_room[coder->n_subroom-1]) {
                if (!room_stack) {
                    spo_endroom(coder);
                } else {
                    room_stack--;
                }
            } else {
                spo_endroom(coder);
            }
            break;
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
        case SPO_SPILL:          spo_spill(coder);    break;
        case SPO_LEVREGION:      spo_levregion(coder); break;
        case SPO_REGION:         spo_region(coder);    break;
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
            if (!OV_pop_i(a)) {
                break;
            }
            OV_i(a)--;
            splev_stack_push(coder->stack, a);
        }
        break;
        case SPO_INC:
        {
            struct opvar *a;
            if (!OV_pop_i(a)) {
                break;
            }
            OV_i(a)++;
            splev_stack_push(coder->stack, a);
        }
        break;
        case SPO_MATH_SIGN:
        {
            struct opvar *a;
            if (!OV_pop_i(a)) {
                break;
            }
            OV_i(a) = ((OV_i(a) < 0) ? -1 : ((OV_i(a) > 0) ? 1 : 0));
            splev_stack_push(coder->stack, a);
        }
        break;
        case SPO_MATH_ADD:
        {
            struct opvar *a, *b;
            if (!OV_pop(b) || !OV_pop(a)) {
                break;
            }
            if (OV_typ(b) == OV_typ(a)) {
                if (OV_typ(a) == SPOVAR_INT) {
                    OV_i(a) = OV_i(a) + OV_i(b);
                    splev_stack_push(coder->stack, a);
                    opvar_free(b);
                } else if (OV_typ(a) == SPOVAR_STRING) {
                    char *tmpbuf = (char *)alloc(strlen(OV_s(a)) + strlen(OV_s(b)) + 1);
                    if (tmpbuf) {
                        struct opvar *c;
                        (void) sprintf(tmpbuf, "%s%s", OV_s(a), OV_s(b));
                        c = opvar_new_str(tmpbuf);
                        splev_stack_push(coder->stack, c);
                        opvar_free(a);
                        opvar_free(b);
                        Free(tmpbuf);
                    } else {
                        splev_stack_push(coder->stack, a);
                        opvar_free(b);
                        impossible("malloc at str concat");
                    }
                } else {
                    splev_stack_push(coder->stack, a);
                    opvar_free(b);
                    impossible("adding weird types");
                }
            } else {
                splev_stack_push(coder->stack, a);
                opvar_free(b);
                impossible("adding different types");
            }
        }
        break;
        case SPO_MATH_SUB:
        {
            struct opvar *a, *b;
            if (!OV_pop_i(b) || !OV_pop_i(a)) {
                break;
            }
            OV_i(a) = OV_i(a) - OV_i(b);
            splev_stack_push(coder->stack, a);
            opvar_free(b);
        }
        break;
        case SPO_MATH_MUL:
        {
            struct opvar *a, *b;
            if (!OV_pop_i(b) || !OV_pop_i(a)) {
                break;
            }
            OV_i(a) = OV_i(a) * OV_i(b);
            splev_stack_push(coder->stack, a);
            opvar_free(b);
        }
        break;
        case SPO_MATH_DIV:
        {
            struct opvar *a, *b;
            if (!OV_pop_i(b) || !OV_pop_i(a)) {
                break;
            }
            if (OV_i(b) >= 1) {
                OV_i(a) = OV_i(a) / OV_i(b);
            } else {
                OV_i(a) = 0;
            }
            splev_stack_push(coder->stack, a);
            opvar_free(b);
        }
        break;
        case SPO_MATH_MOD:
        {
            struct opvar *a, *b;
            if (!OV_pop_i(b) || !OV_pop_i(a)) {
                break;
            }
            if (OV_i(b) > 0) {
                OV_i(a) = OV_i(a) % OV_i(b);
            } else {
                OV_i(a) = 0;
            }
            splev_stack_push(coder->stack, a);
            opvar_free(b);
        }
        break;
        case SPO_CMP:
        {
            struct opvar *a;
            struct opvar *b;
            struct opvar *c;
            long val = 0;

            OV_pop(b);
            OV_pop(a);

            if (!a || !b) {
                impossible("spo_cmp: no values in stack");
                break;
            }

            if (OV_typ(a) != OV_typ(b)) {
                impossible("spo_cmp: trying to compare differing datatypes");
                break;
            }

            switch (OV_typ(a)) {
            case SPOVAR_COORD:
            case SPOVAR_REGION:
            case SPOVAR_MAPCHAR:
            case SPOVAR_MONST:
            case SPOVAR_OBJ:
            case SPOVAR_INT:
                if (OV_i(b) > OV_i(a)) {
                    val |= SP_CPUFLAG_LT;
                }
                if (OV_i(b) < OV_i(a)) {
                    val |= SP_CPUFLAG_GT;
                }
                if (OV_i(b) == OV_i(a)) {
                    val |= SP_CPUFLAG_EQ;
                }
                c = opvar_new_int(val);
                break;
            case SPOVAR_STRING:
                c = opvar_new_int(((!strcmp(OV_s(b), OV_s(a))) ? SP_CPUFLAG_EQ : 0));
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
            if (!OV_pop_i(tmpv)) {
                break;
            }
            t = opvar_new_int((OV_i(tmpv) > 1) ? rn2(OV_i(tmpv)) : 0);
            splev_stack_push(coder->stack, t);
            opvar_free(tmpv);
        }
        break;
        case SPO_COREFUNC:
        {
            struct opvar *a;
            if (!OV_pop_i(a)) {
                break;
            }
            spo_corefunc(coder, OV_i(a));
            opvar_free(a);
        }
        break;
        case SPO_DICE:
        {
            struct opvar *a, *b, *t;
            if (!OV_pop_i(b) || !OV_pop_i(a)) {
                break;
            }
            if (OV_i(b) < 1) {
                OV_i(b) = 1;
            }
            if (OV_i(a) < 1) {
                OV_i(a) = 1;
            }
            t = opvar_new_int(d(OV_i(a), OV_i(b)));
            splev_stack_push(coder->stack, t);
            opvar_free(a);
            opvar_free(b);
        }
        break;
        case SPO_MAP:
            spo_map(coder); break;
        case SPO_VAR_INIT:
            spo_var_init(coder); break;
        case SPO_SHUFFLE_ARRAY:
            spo_shuffle_array(coder); break;
        case SPO_SEL_ADD: /* actually, logical or */
        {
            struct opvar *sel1, *sel2, *pt;
            if (!OV_pop_typ(sel1, SPOVAR_SEL)) {
                panic("no sel1 for add");
            }
            if (!OV_pop_typ(sel2, SPOVAR_SEL)) {
                panic("no sel1 for add");
            }
            pt = op_selection_logical_oper(sel1, sel2, '|');
            opvar_free(sel1);
            opvar_free(sel2);
            splev_stack_push(coder->stack, pt);
        }
        break;
        case SPO_SEL_COMPLEMENT:
        {
            struct opvar *sel, *pt;
            if (!OV_pop_typ(sel, SPOVAR_SEL)) {
                panic("no sel for not");
            }
            pt = op_selection_not(sel);
            opvar_free(sel);
            splev_stack_push(coder->stack, pt);
        }
        break;
        case SPO_SEL_FILTER: /* sorta like logical and */
        {
            struct opvar *filtertype;
            if (!OV_pop_i(filtertype)) {
                panic("no sel filter type");
            }
            switch (OV_i(filtertype)) {
            case SPOFILTER_PERCENT:
            {
                struct opvar *tmp1, *sel;
                if (!OV_pop_i(tmp1)) {
                    panic("no sel filter percent");
                }
                if (!OV_pop_typ(sel, SPOVAR_SEL)) {
                    panic("no sel filter");
                }
                op_selection_filter_percent(sel, OV_i(tmp1));
                splev_stack_push(coder->stack, sel);
                opvar_free(tmp1);
            }
            break;
            case SPOFILTER_SELECTION: /* logical and */
            {
                struct opvar *pt, *sel1, *sel2;
                if (!OV_pop_typ(sel1, SPOVAR_SEL)) {
                    panic("no sel filter sel1");
                }
                if (!OV_pop_typ(sel2, SPOVAR_SEL)) {
                    panic("no sel filter sel2");
                }
                pt = op_selection_logical_oper(sel1, sel2, '&');
                splev_stack_push(coder->stack, pt);
                opvar_free(sel1);
                opvar_free(sel2);
            }
            break;
            case SPOFILTER_MAPCHAR:
            {
                struct opvar *pt, *tmp1, *sel;
                if (!OV_pop_typ(sel, SPOVAR_SEL)) {
                    panic("no sel filter");
                }
                if (!OV_pop_typ(tmp1, SPOVAR_MAPCHAR)) {
                    panic("no sel filter mapchar");
                }
                pt = op_selection_filter_mapchar(sel, tmp1);
                splev_stack_push(coder->stack, pt);
                opvar_free(tmp1);
                opvar_free(sel);
            }
            break;
            default: panic("unknown sel filter type");
            }
            opvar_free(filtertype);
        }
        break;
        case SPO_SEL_POINT:
        {
            struct opvar *tmp;
            struct opvar *pt = op_selection_opvar(NULL);
            coordxy x, y;
            if (!OV_pop_c(tmp)) {
                panic("no ter sel coord");
            }
            get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(tmp));
            op_selection_setpoint(x, y, pt, 1);
            splev_stack_push(coder->stack, pt);
            opvar_free(tmp);
        }
        break;
        case SPO_SEL_RECT:
        case SPO_SEL_FILLRECT:
        {
            struct opvar *tmp, *pt = op_selection_opvar(NULL);
            coordxy x, y, x1, y1, x2, y2;
            if (!OV_pop_r(tmp)) {
                panic("no ter sel region");
            }
            x1 = min(SP_REGION_X1(OV_i(tmp)), SP_REGION_X2(OV_i(tmp)));
            y1 = min(SP_REGION_Y1(OV_i(tmp)), SP_REGION_Y2(OV_i(tmp)));
            x2 = max(SP_REGION_X1(OV_i(tmp)), SP_REGION_X2(OV_i(tmp)));
            y2 = max(SP_REGION_Y1(OV_i(tmp)), SP_REGION_Y2(OV_i(tmp)));
            get_location(&x1, &y1, ANY_LOC, coder->croom);
            get_location(&x2, &y2, ANY_LOC, coder->croom);
            x1 = (x1 < 0) ? 0 : x1;
            y1 = (y1 < 0) ? 0 : y1;
            x2 = (x2 >= COLNO) ? COLNO-1 : x2;
            y2 = (y2 >= ROWNO) ? ROWNO-1 : y2;
            if (coder->opcode == SPO_SEL_RECT) {
                for (x = x1; x <= x2; x++) {
                    op_selection_setpoint(x, y1, pt, 1);
                    op_selection_setpoint(x, y2, pt, 1);
                }
                for (y = y1; y <= y2; y++) {
                    op_selection_setpoint(x1, y, pt, 1);
                    op_selection_setpoint(x2, y, pt, 1);
                }
            } else {
                for (x = x1; x <= x2; x++) {
                    for (y = y1; y <= y2; y++) {
                        op_selection_setpoint(x, y, pt, 1);
                    }
                }
            }
            splev_stack_push(coder->stack, pt);
            opvar_free(tmp);
        }
        break;
        case SPO_SEL_LINE:
        {
            struct opvar *tmp = NULL, *tmp2 = NULL, *pt = op_selection_opvar(NULL);
            coordxy x1, y1, x2, y2;
            if (!OV_pop_c(tmp) || !OV_pop_c(tmp2)) {
                panic("no ter sel linecoord");
            }
            get_location_coord(&x1, &y1, ANY_LOC, coder->croom, OV_i(tmp));
            get_location_coord(&x2, &y2, ANY_LOC, coder->croom, OV_i(tmp2));
            x1 = (x1 < 0) ? 0 : x1;
            y1 = (y1 < 0) ? 0 : y1;
            x2 = (x2 >= COLNO) ? COLNO-1 : x2;
            y2 = (y2 >= ROWNO) ? ROWNO-1 : y2;
            op_selection_do_line(x1, y1, x2, y2, pt);
            splev_stack_push(coder->stack, pt);
            opvar_free(tmp);
            opvar_free(tmp2);
        }
        break;
        case SPO_SEL_RNDLINE:
        {
            struct opvar *tmp = NULL, *tmp2 = NULL, *tmp3 = NULL, *pt = op_selection_opvar(NULL);
            coordxy x1, y1, x2, y2;
            if (!OV_pop_i(tmp3) || !OV_pop_c(tmp) || !OV_pop_c(tmp2)) {
                panic("no ter sel randline");
            }
            get_location_coord(&x1, &y1, ANY_LOC, coder->croom, OV_i(tmp));
            get_location_coord(&x2, &y2, ANY_LOC, coder->croom, OV_i(tmp2));
            x1 = (x1 < 0) ? 0 : x1;
            y1 = (y1 < 0) ? 0 : y1;
            x2 = (x2 >= COLNO) ? COLNO-1 : x2;
            y2 = (y2 >= ROWNO) ? ROWNO-1 : y2;
            op_selection_do_randline(x1, y1, x2, y2, OV_i(tmp3), 12, pt);
            splev_stack_push(coder->stack, pt);
            opvar_free(tmp);
            opvar_free(tmp2);
            opvar_free(tmp3);
        }
        break;
        case SPO_SEL_GROW:
        {
            struct opvar *dirs, *pt;
            if (!OV_pop_i(dirs)) {
                panic("no dirs for grow");
            }
            if (!OV_pop_typ(pt, SPOVAR_SEL)) {
                panic("no selection for grow");
            }
            op_selection_do_grow(pt, OV_i(dirs));
            splev_stack_push(coder->stack, pt);
            opvar_free(dirs);
        }
        break;
        case SPO_SEL_FLOOD:
        {
            struct opvar *tmp;
            coordxy x, y;
            if (!OV_pop_c(tmp)) {
                panic("no ter sel flood coord");
            }
            get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(tmp));
            if (isok(x, y)) {
                struct opvar *pt = op_selection_opvar(NULL);
                floodfillchk_match_under_typ = levl[x][y].typ;
                op_selection_floodfill(pt, x, y, FALSE);
                splev_stack_push(coder->stack, pt);
            }
            opvar_free(tmp);
        }
        break;
        case SPO_SEL_RNDCOORD:
        {
            struct opvar *pt;
            coordxy x, y;
            if (!OV_pop_typ(pt, SPOVAR_SEL)) {
                panic("no selection for rndcoord");
            }
            if (op_selection_rndcoord(pt, &x, &y, FALSE)) {
                x -= gx.xstart;
                y -= gy.ystart;
            }
            /*get_location(&x, &y, DRY|WET, coder->croom);*/
            splev_stack_push(coder->stack, opvar_new_coord(x, y));
            opvar_free(pt);
        }
        break;
        case SPO_SEL_ELLIPSE:
        {
            struct opvar *filled, *xaxis, *yaxis, *pt;
            struct opvar *sel = op_selection_opvar(NULL);
            coordxy x, y;
            if (!OV_pop_i(filled)) {
                panic("no filled for ellipse");
            }
            if (!OV_pop_i(yaxis)) {
                panic("no yaxis for ellipse");
            }
            if (!OV_pop_i(xaxis)) {
                panic("no xaxis for ellipse");
            }
            if (!OV_pop_c(pt)) {
                panic("no pt for ellipse");
            }
            get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(pt));
            op_selection_do_ellipse(sel, x, y, OV_i(xaxis), OV_i(yaxis), OV_i(filled));
            splev_stack_push(coder->stack, sel);
            opvar_free(filled);
            opvar_free(yaxis);
            opvar_free(xaxis);
            opvar_free(pt);
        }
        break;
        case SPO_SEL_GRADIENT:
        {
            struct opvar *gtyp, *glim, *mind, *maxd, *coord, *coord2;
            struct opvar *sel;
            coordxy x, y, x2, y2;
            if (!OV_pop_i(gtyp)) {
                panic("no gtyp for grad");
            }
            if (!OV_pop_i(glim)) {
                panic("no glim for grad");
            }
            if (!OV_pop_c(coord2)) {
                panic("no coord2 for grad");
            }
            if (!OV_pop_c(coord)) {
                panic("no coord for grad");
            }
            if (!OV_pop_i(maxd)) {
                panic("no maxd for grad");
            }
            if (!OV_pop_i(mind)) {
                panic("no mind for grad");
            }
            get_location_coord(&x, &y, ANY_LOC, coder->croom, OV_i(coord));
            get_location_coord(&x2, &y2, ANY_LOC, coder->croom, OV_i(coord2));

            sel = op_selection_opvar(NULL);
            op_selection_do_gradient(sel, x, y, x2, y2, OV_i(gtyp), OV_i(mind), OV_i(maxd), OV_i(glim));
            splev_stack_push(coder->stack, sel);

            opvar_free(gtyp);
            opvar_free(glim);
            opvar_free(coord);
            opvar_free(coord2);
            opvar_free(maxd);
            opvar_free(mind);
        }
        break;
        default:
            panic("sp_level_coder: Unknown opcode %i", coder->opcode);
        }

next_opcode:
        coder->frame->n_opcode++;
    } /*while*/
    link_doors_rooms();
    fill_rooms();
    remove_boundary_syms();

    if (coder->check_inaccessibles) {
        ensure_way_out();
    }
    /* FIXME: Ideally, we want this call to only cover areas of the map
     * which were not inserted directly by the special level file (see
     * the insect legs on Baalzebub's level, for instance). Since that
     * is currently not possible, we overload the corrmaze flag for this
     * purpose.
     */
    if (!level.flags.corrmaze) {
        wallification(1, 0, COLNO-1, ROWNO-1);
    }

    flip_level_rnd(coder->allow_flips, 0);

    count_level_features();

    if (coder->solidify) {
        solidify_map();
    }

    /* This must be done before sokoban_detect(),
     * otherwise branch stairs won't be premapped. */
    fixup_special();

    if (coder->premapped) {
        premap_detect();
    }

    if (coder->frame) {
        struct sp_frame *tmpframe;
        do {
            tmpframe = coder->frame->next;
            frame_del(coder->frame);
            coder->frame = tmpframe;
        } while (coder->frame);
    }
    Free(coder);

    return TRUE;
}

/*
 * General loader
 */
struct _sploader_cache {
    char *fname;
    sp_lev *lvl;
    struct _sploader_cache *next;
};

struct _sploader_cache *sp_loader_cache = NULL;

sp_lev *
sp_lev_cache(const char *fnam)
{
    struct _sploader_cache *tmp = sp_loader_cache;

    while (tmp) {
        if (!strcmp(tmp->fname, fnam)) {
            return tmp->lvl;
        }
        tmp = tmp->next;
    }
    return NULL;
}

void
sp_lev_savecache(const char *fnam, sp_lev *lvl)
{
    struct _sploader_cache *tmp = (struct _sploader_cache *)alloc(sizeof(struct _sploader_cache));
    if (!tmp) {
        panic("save splev cache");
    }
    tmp->lvl = lvl;
    tmp->fname = strdup(fnam);
    tmp->next = sp_loader_cache;
    sp_loader_cache = tmp;
}

boolean
load_special_des(const char *name)
{
    dlb *fd;
    sp_lev *lvl = NULL;
    boolean result = FALSE;
    struct version_info vers_info;

    if (!(lvl = sp_lev_cache(name))) {
        fd = dlb_fopen_area(FILE_AREA_UNSHARE, name, RDBMODE);
        if (!fd) {
            return FALSE;
        }
        Fread((genericptr_t) &vers_info, sizeof vers_info, 1, fd);
        if (!check_version(&vers_info, name, TRUE)) {
            (void)dlb_fclose(fd);
            goto give_up;
        }
        lvl = (sp_lev *)alloc(sizeof(sp_lev));
        if (!lvl) {
            panic("alloc sp_lev");
        }
        result = sp_level_loader(fd, lvl);
        (void)dlb_fclose(fd);
        if (gi.in_mk_themerooms) {
            sp_lev_savecache(name, lvl);
        }
        if (result) {
            result = sp_level_coder(lvl);
        }
        if (!gi.in_mk_themerooms) {
            sp_level_free(lvl);
            Free(lvl);
        }
    } else {
        result = sp_level_coder(lvl);
    }

give_up:
    return result;
}

/*sp_lev.c*/
