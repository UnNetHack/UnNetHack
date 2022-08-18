/*  SCCS Id: @(#)mkmap.c    3.4 1996/05/23  */
/* Copyright (c) J. C. Collet, M. Stephenson and D. Cohrs, 1992   */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sp_lev.h"

#define HEIGHT  (ROWNO - 1)
#define WIDTH   (COLNO - 2)

static void init_map(schar);
static void init_fill(schar, schar);
static schar get_map(int, int, schar);
static void pass_one(schar, schar);
static void pass_two(schar, schar);
static void pass_three(schar, schar);
static void wallify_map(void);
static void join_map(schar, schar);
static void finish_map(schar, schar, coordxy, coordxy);
static void remove_room(unsigned);
static void backfill(schar, schar);
void mkmap(lev_init *);

char *new_locations;
int min_rx, max_rx, min_ry, max_ry; /* rectangle bounds for regions */
static int n_loc_filled;

static void
init_map(schar bg_typ)
{
    int i, j;

    if (bg_typ >= MAX_TYPE) return;

    for(i=1; i<COLNO; i++)
        for(j=0; j<ROWNO; j++) {
            levl[i][j].typ = bg_typ;
            levl[i][j].lit = FALSE;
        }
}


static void
backfill(schar bg_typ, schar filler)
{
    int x, y;

    if (filler >= MAX_TYPE) return;

    for(x=1; x<COLNO; x++) {
        for(y=0; y<ROWNO; y++) {
            if (levl[x][y].typ == bg_typ) {
                levl[x][y].typ = filler;
                if (filler == LAVAPOOL) {    /* lava's always lit */
                    levl[x][y].lit = 1;
                }
            }
        }
    }

}


static void
init_fill(schar bg_typ, schar fg_typ)
{
    int i, j;
    long limit, count;

    if (fg_typ >= MAX_TYPE) return;

    limit = (WIDTH * HEIGHT * 2) / 5;
    count = 0;
    while(count < limit) {
        i = rn1(WIDTH-1, 2);
        j = rnd(HEIGHT-1);
        if (levl[i][j].typ == bg_typ) {
            levl[i][j].typ = fg_typ;
            count++;
        }
    }
}

static schar
get_map(int col, int row, schar bg_typ)
{
    if (col <= 0 || row < 0 || col > WIDTH || row >= HEIGHT)
        return bg_typ;
    return levl[col][row].typ;
}

static int dirs[16] = {
    -1, -1 /**/, -1, 0 /**/, -1, 1 /**/,
    0, -1 /**/,              0, 1 /**/,
    1, -1 /**/,  1, 0 /**/,  1, 1
};

static void
pass_one(schar bg_typ, schar fg_typ)
{
    int i, j;
    short count, dr;

    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++) {
            for(count=0, dr=0; dr < 8; dr++)
                if(get_map(i+dirs[dr*2], j+dirs[(dr*2)+1], bg_typ)
                   == fg_typ)
                    count++;

            switch(count) {
            case 0: /* death */
            case 1:
            case 2:
                if (bg_typ >= MAX_TYPE) break;
                levl[i][j].typ = bg_typ;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                if (fg_typ >= MAX_TYPE) break;
                levl[i][j].typ = fg_typ;
                break;
            default:
                break;
            }
        }
}

#define new_loc(i, j)    *(new_locations+ ((j)*(WIDTH+1)) + (i))

static void
pass_two(schar bg_typ, schar fg_typ)
{
    int i, j;
    short count, dr;

    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++) {
            for(count=0, dr=0; dr < 8; dr++)
                if(get_map(i+dirs[dr*2], j+dirs[(dr*2)+1], bg_typ)
                   == fg_typ)
                    count++;
            if (count == 5)
                new_loc(i, j) = bg_typ;
            else
                new_loc(i, j) = get_map(i, j, bg_typ);
        }

    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++)
            if (new_loc(i, j) < MAX_TYPE)
                levl[i][j].typ = new_loc(i, j);
}

static void
pass_three(schar bg_typ, schar fg_typ)
{
    int i, j;
    short count, dr;

    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++) {
            for(count=0, dr=0; dr < 8; dr++)
                if(get_map(i+dirs[dr*2], j+dirs[(dr*2)+1], bg_typ)
                   == fg_typ)
                    count++;
            if (count < 3)
                new_loc(i, j) = bg_typ;
            else
                new_loc(i, j) = get_map(i, j, bg_typ);
        }

    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++)
            if (new_loc(i, j) < MAX_TYPE)
                levl[i][j].typ = new_loc(i, j);
}

boolean
check_flood_anyroom(coordxy x, coordxy y, schar fg_typ, boolean anyroom)
{
    if (!isok(x, y)) return FALSE;
    return (anyroom ? IS_ROOM(levl[x][y].typ) : levl[x][y].typ == fg_typ);
}

/*
 * use a flooding algorithm to find all locations that should
 * have the same rm number as the current location.
 * if anyroom is TRUE, use IS_ROOM to check room membership instead of
 * exactly matching levl[sx][sy].typ and walls are included as well.
 */
void
flood_fill_rm(int sx, int sy, int rmno, boolean lit, boolean anyroom)
{
    int i;
    int nx;
    schar fg_typ = levl[sx][sy].typ;

    /* back up to find leftmost uninitialized location */
    while (sx > 0 &&
           (check_flood_anyroom(sx, sy, fg_typ, anyroom)) &&
           (int) levl[sx][sy].roomno != rmno)
        sx--;
    sx++; /* compensate for extra decrement */

    /* assume sx,sy is valid */
    if(sx < min_rx) min_rx = sx;
    if(sy < min_ry) min_ry = sy;

    for(i=sx; i<=WIDTH && check_flood_anyroom(i, sy, fg_typ, anyroom); i++) {
        levl[i][sy].roomno = rmno;
        levl[i][sy].lit = lit;
        if(anyroom) {
            /* add walls to room as well */
            int ii, jj;
            for(ii= (i == sx ? i-1 : i); ii <= i+1; ii++)
                for(jj = sy-1; jj <= sy+1; jj++)
                    if(isok(ii, jj) &&
                       (IS_WALL(levl[ii][jj].typ) ||
                        IS_DOOR(levl[ii][jj].typ))) {
                        levl[ii][jj].edge = 1;
                        if(lit) levl[ii][jj].lit = lit;
                        if ((int) levl[ii][jj].roomno != rmno)
                            levl[ii][jj].roomno = SHARED;
                    }
        }
        n_loc_filled++;
    }
    nx = i;

    if(isok(sx, sy-1)) {
        for(i=sx; i<nx; i++)
            if(check_flood_anyroom(i, sy-1, fg_typ, anyroom)) {
                if ((int) levl[i][sy-1].roomno != rmno)
                    flood_fill_rm(i, sy-1, rmno, lit, anyroom);
            } else {
                if((i>sx || isok(i-1, sy-1)) &&
                   check_flood_anyroom(i-1, sy-1, fg_typ, anyroom)) {
                    if ((int) levl[i-1][sy-1].roomno != rmno)
                        flood_fill_rm(i-1, sy-1, rmno, lit, anyroom);
                }
                if((i<nx-1 || isok(i+1, sy-1)) &&
                   check_flood_anyroom(i+1, sy-1, fg_typ, anyroom)) {
                    if ((int) levl[i+1][sy-1].roomno != rmno)
                        flood_fill_rm(i+1, sy-1, rmno, lit, anyroom);
                }
            }
    }
    if(isok(sx, sy+1)) {
        for(i=sx; i<nx; i++)
            if(check_flood_anyroom(i, sy+1, fg_typ, anyroom)) {
                if ((int) levl[i][sy+1].roomno != rmno)
                    flood_fill_rm(i, sy+1, rmno, lit, anyroom);
            } else {
                if((i>sx || isok(i-1, sy+1)) &&
                   check_flood_anyroom(i-1, sy+1, fg_typ, anyroom)) {
                    if ((int) levl[i-1][sy+1].roomno != rmno)
                        flood_fill_rm(i-1, sy+1, rmno, lit, anyroom);
                }
                if((i<nx-1 || isok(i+1, sy+1)) &&
                   check_flood_anyroom(i+1, sy+1, fg_typ, anyroom)) {
                    if ((int) levl[i+1][sy+1].roomno != rmno)
                        flood_fill_rm(i+1, sy+1, rmno, lit, anyroom);
                }
            }
    }

    if(nx > max_rx) max_rx = nx - 1; /* nx is just past valid region */
    if(sy > max_ry) max_ry = sy;
}

/*
 *  If we have drawn a map without walls, this allows us to
 *  auto-magically wallify it.  Taken from lev_main.c.
 */
static void
wallify_map(void)
{

    int x, y, xx, yy;

    for(x = 1; x < COLNO; x++)
        for(y = 0; y < ROWNO; y++)
            if(levl[x][y].typ == STONE) {
                for(yy = y - 1; yy <= y+1; yy++)
                    for(xx = x - 1; xx <= x+1; xx++)
                        if(isok(xx, yy) && levl[xx][yy].typ == ROOM) {
                            if(yy != y) levl[x][y].typ = HWALL;
                            else levl[x][y].typ = VWALL;
                        }
            }
}

static void
join_map(schar bg_typ, schar fg_typ)
{
    struct mkroom *croom, *croom2;

    int i, j;
    int sx, sy;
    coord sm, em;

    /* first, use flood filling to find all of the regions that need joining */
    for(i=2; i<=WIDTH; i++)
        for(j=1; j<HEIGHT; j++) {
            if(levl[i][j].typ == fg_typ && levl[i][j].roomno == NO_ROOM) {
                min_rx = max_rx = i;
                min_ry = max_ry = j;
                n_loc_filled = 0;
                flood_fill_rm(i, j, nroom+ROOMOFFSET, FALSE, FALSE);
                if(n_loc_filled > 3) {
                    add_room(min_rx, min_ry, max_rx, max_ry,
                             FALSE, OROOM, TRUE);
                    rooms[nroom-1].irregular = TRUE;
                    if(nroom >= (MAXNROFROOMS*2))
                        goto joinm;
                } else {
                    /*
                     * it's a tiny hole; erase it from the map to avoid
                     * having the player end up here with no way out.
                     */
                    for(sx = min_rx; sx<=max_rx; sx++)
                        for(sy = min_ry; sy<=max_ry; sy++)
                            if ((int) levl[sx][sy].roomno ==
                                nroom + ROOMOFFSET) {
                                levl[sx][sy].typ = bg_typ;
                                levl[sx][sy].roomno = NO_ROOM;
                            }
                }
            }
        }

joinm:
    /*
     * Ok, now we can actually join the regions with fg_typ's.
     * The rooms are already sorted due to the previous loop,
     * so don't call sort_rooms(), which can screw up the roomno's
     * validity in the levl structure.
     */
    for(croom = &rooms[0], croom2 = croom + 1; croom2 < &rooms[nroom]; ) {
        /* pick random starting and end locations for "corridor" */
        if(!somexy(croom, &sm) || !somexy(croom2, &em)) {
            /* ack! -- the level is going to be busted */
            /* arbitrarily pick centers of both rooms and hope for the best */
            impossible("No start/end room loc in join_map.");
            sm.x = croom->lx + ((croom->hx - croom->lx) / 2);
            sm.y = croom->ly + ((croom->hy - croom->ly) / 2);
            em.x = croom2->lx + ((croom2->hx - croom2->lx) / 2);
            em.y = croom2->ly + ((croom2->hy - croom2->ly) / 2);
        }

        (void) dig_corridor(&sm, &em, FALSE, fg_typ, bg_typ);

        /* choose next region to join */
        /* only increment croom if croom and croom2 are non-overlapping */
        if(croom2->lx > croom->hx ||
           ((croom2->ly > croom->hy || croom2->hy < croom->ly) && rn2(3))) {
            croom = croom2;
        }
        croom2++; /* always increment the next room */
    }
}

static void
finish_map(schar fg_typ, schar bg_typ, boolean lit, boolean walled)
{
    int i, j;

    if(walled) wallify_map();

    if(lit) {
        for(i=1; i<COLNO; i++)
            for(j=0; j<ROWNO; j++)
                if((!IS_ROCK(fg_typ) && levl[i][j].typ == fg_typ) ||
                   (!IS_ROCK(bg_typ) && levl[i][j].typ == bg_typ) ||
                   (bg_typ == TREE && levl[i][j].typ == bg_typ) ||
                   (walled && IS_WALL(levl[i][j].typ)))
                    levl[i][j].lit = TRUE;
        for(i = 0; i < nroom; i++)
            rooms[i].rlit = 1;
    }
    /* light lava even if everything's otherwise unlit */
    for(i=1; i<COLNO; i++)
        for(j=0; j<ROWNO; j++)
            if (levl[i][j].typ == LAVAPOOL)
                levl[i][j].lit = TRUE;
}

/*
 * When level processed by join_map is overlaid by a MAP, some rooms may no
 * longer be valid.  All rooms in the region lx <= x < hx, ly <= y < hy are
 * removed.  Rooms partially in the region are truncated.  This function
 * must be called before the REGIONs or ROOMs of the map are processed, or
 * those rooms will be removed as well.  Assumes roomno fields in the
 * region are already cleared, and roomno and irregular fields outside the
 * region are all set.
 */
void
remove_rooms(int lx, int ly, int hx, int hy)
{
    int i;
    struct mkroom *croom;

    for (i = nroom - 1; i >= 0; --i) {
        croom = &rooms[i];
        if (croom->hx < lx || croom->lx >= hx ||
            croom->hy < ly || croom->ly >= hy) continue; /* no overlap */

        if (croom->lx < lx || croom->hx >= hx ||
            croom->ly < ly || croom->hy >= hy) { /* partial overlap */
            /* TODO: ensure remaining parts of room are still joined */

            if (!croom->irregular) impossible("regular room in joined map");
        } else {
            /* total overlap, remove the room */
            remove_room((unsigned)i);
        }
    }
}

/*
 * Remove roomno from the rooms array, decrementing nroom.  Also updates
 * all level roomno values of affected higher numbered rooms.  Assumes
 * level structure contents corresponding to roomno have already been reset.
 * Currently handles only the removal of rooms that have no subrooms.
 */
static void
remove_room(unsigned int roomno)
{
    struct mkroom *croom = &rooms[roomno];
    struct mkroom *maxroom = &rooms[--nroom];
    int i, j;
    unsigned oroomno;

    if (croom != maxroom) {
        /* since the order in the array only matters for making corridors,
         * copy the last room over the one being removed on the assumption
         * that corridors have already been dug. */
        (void) memcpy((genericptr_t)croom, (genericptr_t)maxroom,
                      sizeof(struct mkroom));

        /* since maxroom moved, update affected level roomno values */
        oroomno = nroom + ROOMOFFSET;
        roomno += ROOMOFFSET;
        for (i = croom->lx; i <= croom->hx; ++i)
            for (j = croom->ly; j <= croom->hy; ++j) {
                if (levl[i][j].roomno == oroomno)
                    levl[i][j].roomno = roomno;
            }
    }

    maxroom->hx = -1;           /* just like add_room */
}

#define N_P1_ITER   1   /* tune map generation via this value */
#define N_P2_ITER   1   /* tune map generation via this value */
#define N_P3_ITER   2   /* tune map smoothing via this value */

void
mkmap(lev_init *init_lev)
{
    schar bg_typ = init_lev->bg,
          fg_typ = init_lev->fg;
    boolean smooth = init_lev->smoothed,
            join = init_lev->joined;
    xint16 lit = init_lev->lit,
          walled = init_lev->walled;
    int i;

    if(lit < 0)
        lit = (rnd(1+abs(depth(&u.uz))) < 11 && rn2(77)) ? 1 : 0;

    new_locations = (char *)alloc((WIDTH+1) * HEIGHT);

    if (bg_typ < MAX_TYPE)
        init_map(bg_typ);
    init_fill(bg_typ, fg_typ);

    for(i = 0; i < N_P1_ITER; i++)
        pass_one(bg_typ, fg_typ);

    for(i = 0; i < N_P2_ITER; i++)
        pass_two(bg_typ, fg_typ);

    if(smooth)
        for(i = 0; i < N_P3_ITER; i++)
            pass_three(bg_typ, fg_typ);

    if(join)
        join_map(bg_typ, fg_typ);

    finish_map(fg_typ, bg_typ, (boolean)lit, (boolean)walled);
    /* a walled, joined level is cavernous, not mazelike -dlc
     *
     * also, caverns have a defined "inside" and "outside"; the outside
     * doesn't _have_ to be stone, say, for hell.  so if the player
     * defined a maze filler originally, go ahead and backfill the
     * background in with that filler - DSR */
    if (walled && join && (init_lev->filling > -1)) {
        level.flags.is_maze_lev = FALSE;
        level.flags.is_cavernous_lev = TRUE;
        backfill(bg_typ, init_lev->filling);
    }
    free(new_locations);
}

/*mkmap.c*/
