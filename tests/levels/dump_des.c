/*
 * Golden-master driver for the .des loader (spo_lev.c).
 *
 * Loads a compiled special level through the normal loader and prints a
 * canonical dump on stderr.
 *
 * Build from src/ against the game objects except unixmain.o.
 * The level files are read through DLB from FILE_AREA_UNSHARE, i.e. the
 * data directory of the configured install prefix.
 */

#include "hack.h"
#include "dlb.h"
#include "dump_common.h"

int
main(int argc, char **argv)
{
    const char *name = (argc > 1) ? argv[1] : "castle-1.lev";
    unsigned seed = (argc > 2) ? (unsigned) atoi(argv[2]) : 42u;

    golden_init(&argc, argv, seed);

    if (!load_special_des(name)) {
        fprintf(stderr, "load_special_des(%s) failed\n", name);
        return 2;
    }

    golden_print(name, seed);
    return 0;
}
