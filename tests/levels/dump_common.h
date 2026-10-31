/*
 * Shared fixture for the golden-master drivers.
 *
 * The output is meant to be diffed between two builds of the game, so it
 * must not contain addresses, timestamps or anything else unstable.
 * Game messages go to stdout (dummy window port); the dump goes to
 * stderr so the two streams cannot interleave.
 */

#ifndef DUMP_COMMON_H
#define DUMP_COMMON_H

#include <stdio.h>
#include <stdlib.h>

extern struct window_procs dummy_procs;

static void
golden_init(int *argc, char **argv, unsigned seed)
{
    int i;

    memset(&level, 0, sizeof level);
    memset(&u, 0, sizeof u);

    windowprocs = dummy_procs;
    windowprocs.win_init_nhwindows(argc, argv);

    if (!dlb_init()) {
        fprintf(stderr, "dlb_init failed\n");
        exit(1);
    }

    set_random_state(seed);
    init_rect();
    vision_init();

    /* The part of newgame() that monster and object creation rely on. */
    flags.ident = 1;
    for (i = 0; i < NUMMONS; i++)
        mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;
    flags.pantheon = -1;
    flags.initrole = 0;
    flags.initrace = 0;
    flags.initgend = 0;
    flags.initalign = 0;
    /* plnamesuffix() asks for a name when plname is empty */
    Strcpy(plname, "golden");

    u.uz.dnum = 0;
    u.uz.dlevel = 5;

    role_init();
    init_objects();
    init_dungeons();
    init_artifacts();
    u_init();

    in_mklev = TRUE;
    vision_reset();
}

static void
golden_print(const char *name, unsigned seed)
{
    static const char typs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int x, y, i;
    struct trap *t;
    struct obj *o;
    struct monst *m;
    stairway *st;
    unsigned long wallhash = 2166136261UL;
    unsigned long lithash = 2166136261UL;

    fprintf(stderr, "level %s seed %u\n", name, seed);

    fprintf(stderr, "flags maze=%d corrmaze=%d temperature=%d\n",
           level.flags.is_maze_lev, level.flags.corrmaze,
           (int) level.flags.temperature);

    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            wallhash ^= (unsigned long) levl[x][y].wall_info;
            wallhash *= 16777619UL;
            lithash ^= (unsigned long) levl[x][y].lit;
            lithash *= 16777619UL;
        }
    }
    fprintf(stderr, "wallhash %08lx\n", wallhash & 0xffffffffUL);
    fprintf(stderr, "lithash %08lx\n", lithash & 0xffffffffUL);

    for (st = stairs; st; st = st->next) {
        fprintf(stderr, "stair %d %d %s %s\n", st->sx, st->sy,
               st->up ? "up" : "down", st->isladder ? "ladder" : "stair");
    }

    for (y = 0; y < ROWNO; y++) {
        fprintf(stderr, "row %02d ", y);
        for (x = 0; x < COLNO; x++) {
            int typ = levl[x][y].typ;
            fputc(typ >= 0 && typ < (int) sizeof typs - 1 ? typs[typ] : '?',
                  stderr);
        }
        fputc('\n', stderr);
    }

    for (i = 0; i < nroom; i++) {
        struct mkroom *r = &rooms[i];
        fprintf(stderr, "room %d %d %d %d %d type=%d doors=%d\n",
               i, r->lx, r->ly, r->hx, r->hy, r->rtype, r->doorct);
    }

    for (i = 0; i < doorindex; i++) {
        coordxy dx = doors[i].x, dy = doors[i].y;
        fprintf(stderr, "door %d %d mask=%d horizontal=%d\n",
               dx, dy, levl[dx][dy].doormask, levl[dx][dy].horizontal);
    }

    for (t = ftrap; t; t = t->ntrap) {
        fprintf(stderr, "trap %d %d %d\n", t->tx, t->ty, t->ttyp);
    }

    for (o = fobj; o; o = o->nobj) {
        fprintf(stderr, "obj %d %d %d %ld\n",
               o->otyp, o->ox, o->oy, (long) o->quan);
    }

    for (m = fmon; m; m = m->nmon) {
        fprintf(stderr, "mon %d %d %d peaceful=%d asleep=%d\n",
               m->mnum, m->mx, m->my, m->mpeaceful, m->msleeping);
    }
}

#endif /* DUMP_COMMON_H */
