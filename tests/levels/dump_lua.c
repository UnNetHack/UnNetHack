/*
 * Golden-master driver for the Lua level builder (sp_lev.c).
 *
 * Loads a Lua level script through load_special_lua() and prints a
 * canonical dump on stderr.
 *
 * Build from src/ against the game objects except unixmain.o.
 * The scripts are read through DLB from NH_DATAAREA, i.e. the data
 * directory of the configured install prefix.
 */

#include "hack.h"
#include "dlb.h"
#include "dump_common.h"

int
main(int argc, char **argv)
{
    const char *name = (argc > 1) ? argv[1] : "Arc-strt.lua";
    unsigned seed = (argc > 2) ? (unsigned) atoi(argv[2]) : 42u;

    golden_init(&argc, argv, seed);

    if (!load_special_lua(name)) {
        fprintf(stderr, "load_special_lua(%s) failed\n", name);
        return 2;
    }

    golden_print(name, seed);
    return 0;
}
