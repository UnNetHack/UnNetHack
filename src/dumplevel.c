/* NetHack may be freely redistributed.  See license for details. */

/*
 * Headless level dump.
 *
 * Loads one compiled or Lua special level and prints a canonical,
 * pointer-free description of the result.  Used by the golden-master
 * tests; only available in builds with dummy graphics.
 *
 * Usage:
 *   unnethack --dump-level NAME [--seed N] [--dump-file PATH]
 *
 * The dump goes to stderr unless --dump-file is given, because the dummy
 * window port writes its messages to stdout.
 */

#include "hack.h"
#include "dlb.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef DUMMY_GRAPHICS
extern struct window_procs dummy_procs;
#endif

static void
dump_level_print(FILE *out, const char *name, unsigned seed)
{
    static const char typs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int x, y, i;
    struct trap *t;
    struct obj *o;
    struct monst *m;
    stairway *st;
    unsigned long wallhash = 2166136261UL;
    unsigned long lithash = 2166136261UL;

    fprintf(out, "level %s seed %u\n", name, seed);
    fprintf(out, "flags maze=%d corrmaze=%d temperature=%d\n",
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
    fprintf(out, "wallhash %08lx\n", wallhash & 0xffffffffUL);
    fprintf(out, "lithash %08lx\n", lithash & 0xffffffffUL);

    for (st = stairs; st; st = st->next) {
        fprintf(out, "stair %d %d %s %s\n", st->sx, st->sy,
                st->up ? "up" : "down", st->isladder ? "ladder" : "stair");
    }

    for (y = 0; y < ROWNO; y++) {
        fprintf(out, "row %02d ", y);
        for (x = 0; x < COLNO; x++) {
            int typ = levl[x][y].typ;

            fputc(typ >= 0 && typ < (int) sizeof typs - 1 ? typs[typ] : '?',
                  out);
        }
        fputc('\n', out);
    }

    for (i = 0; i < nroom; i++) {
        struct mkroom *r = &rooms[i];

        fprintf(out, "room %d %d %d %d %d type=%d doors=%d\n",
                i, r->lx, r->ly, r->hx, r->hy, r->rtype, r->doorct);
    }

    for (i = 0; i < doorindex; i++) {
        coordxy dx = doors[i].x, dy = doors[i].y;

        fprintf(out, "door %d %d mask=%d horizontal=%d\n",
                dx, dy, levl[dx][dy].doormask, levl[dx][dy].horizontal);
    }

    for (t = ftrap; t; t = t->ntrap) {
        fprintf(out, "trap %d %d %d\n", t->tx, t->ty, t->ttyp);
    }

    for (o = fobj; o; o = o->nobj) {
        fprintf(out, "obj %d %d %d %ld\n",
                o->otyp, o->ox, o->oy, (long) o->quan);
    }

    for (m = fmon; m; m = m->nmon) {
        fprintf(out, "mon %d %d %d peaceful=%d asleep=%d\n",
                m->mnum, m->mx, m->my, m->mpeaceful, m->msleeping);
    }
}

/*
 * Returns -1 if no dump was requested, otherwise the process exit code.
 */
int
dump_level_main(int argc, char **argv)
{
    const char *name = NULL, *outpath = NULL;
    unsigned seed = 42u;
    FILE *out = stderr;
    int i, rc = 0;
    boolean ok;

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "--dump-level=", 13)) {
            name = argv[i] + 13;
        } else if (!strcmp(argv[i], "--dump-level") && i + 1 < argc) {
            name = argv[++i];
        } else if (!strncmp(argv[i], "--seed=", 7)) {
            seed = (unsigned) atoi(argv[i] + 7);
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {
            seed = (unsigned) atoi(argv[++i]);
        } else if (!strncmp(argv[i], "--dump-file=", 12)) {
            outpath = argv[i] + 12;
        } else if (!strcmp(argv[i], "--dump-file") && i + 1 < argc) {
            outpath = argv[++i];
        }
    }
    if (!name) {
        return -1;
    }

#ifndef DUMMY_GRAPHICS
    fprintf(stderr, "--dump-level requires a dummy-graphics build\n");
    return 1;
#else
    memset(&level, 0, sizeof level);
    memset(&u, 0, sizeof u);

    windowprocs = dummy_procs;
    windowprocs.win_init_nhwindows(&argc, argv);

    if (!dlb_init()) {
        fprintf(stderr, "dlb_init failed\n");
        return 1;
    }

    set_random_state(seed);
    init_rect();
    vision_init();

    flags.ident = 1;
    for (i = 0; i < NUMMONS; i++) {
        mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;
    }
    flags.pantheon = -1;
    flags.initrole = 0;
    flags.initrace = 0;
    flags.initgend = 0;
    flags.initalign = 0;
    Strcpy(plname, "dump");

    /* honor the usual role/race/name options in dump mode */
    for (i = 1; i < argc; i++) {
        const char *arg = argv[i], *val = NULL;

        if (!strncmp(arg, "-u", 2)) {
            val = arg + 2;
            if (*val == '=' || *val == ':') {
                val++;
            }
            if (!*val && i + 1 < argc) {
                val = argv[++i];
            }
            if (val && *val) {
                copynchars(plname, val, sizeof(plname) - 1);
            }
        } else if (!strncmp(arg, "-p", 2)) {
            int role;

            val = arg + 2;
            if (*val == '=' || *val == ':') {
                val++;
            }
            if (!*val && i + 1 < argc) {
                val = argv[++i];
            }
            role = val ? str2role(val) : ROLE_NONE;
            if (role != ROLE_NONE) {
                flags.initrole = role;
            }
        } else if (!strncmp(arg, "-r", 2)) {
            int race;

            val = arg + 2;
            if (*val == '=' || *val == ':') {
                val++;
            }
            if (!*val && i + 1 < argc) {
                val = argv[++i];
            }
            race = val ? str2race(val) : ROLE_NONE;
            if (race != ROLE_NONE) {
                flags.initrace = race;
            }
        }
    }

    /* honor role/race/gender/alignment suffixes in the name */
    plnamesuffix();

    u.uz.dnum = 0;
    u.uz.dlevel = 5;

    role_init();
    init_objects();
    init_dungeons();
    init_artifacts();
    u_init();

    in_mklev = TRUE;
    vision_reset();

    if (strlen(name) > 4 && !strcmp(name + strlen(name) - 4, ".lua")) {
        ok = load_special_lua(name);
    } else {
        ok = load_special_des(name);
    }
    if (!ok) {
        fprintf(stderr, "cannot load level %s\n", name);
        return 2;
    }

    if (outpath) {
        out = fopen(outpath, "w");
        if (!out) {
            fprintf(stderr, "cannot open %s\n", outpath);
            return 1;
        }
    }

    dump_level_print(out, name, seed);

    if (out != stderr) {
        fclose(out);
    }
    return rc;
#endif
}
