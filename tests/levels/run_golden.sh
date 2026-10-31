#!/bin/sh
#
# Regenerate the special-level dumps and compare them with the committed
# golden files.
#
# The dumps must be produced by the same build as the game data that the
# drivers read through DLB (FILE_AREA_UNSHARE / NH_DATAAREA of the
# configured prefix).  Run `make` and then `make dlb`, so nhdat is
# repacked from the current dat/*.lev; a stale nhdat silently invalidates
# every dump.  Run with --update to accept a change.
#
# Usage: sh tests/levels/run_golden.sh [--update]

set -u

here=$(dirname "$0")
update=no
[ "${1:-}" = "--update" ] && update=yes

seeds="42 7 1234"
des_levels="castle-1 medusa-1 asmod-1 baalz-1 knox-1 oracle valley-1 minetn-1 bigrm-1 bigrm-13 bigrm-14 soko1-1 juiblex sanctum blkmar-2 Arc-loca Rog-strt town-1 sheolfil sheolmid"
lua_levels="Arc-strt Bar-strt Mon-strt Ran-strt Val-strt Arc-goal Bar-goal Val-goal Arc-loca Mon-loca Val-loca"

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

failed=0

# run <driver> <golden-dir> <extension> <level> <seed>
run() {
    driver=$1
    goldendir=$2
    ext=$3
    level=$4
    seed=$5
    name="$level.$seed.txt"

    if [ ! -x "$driver" ]; then
        echo "missing $driver; run: make -C tests $(basename "$driver")" >&2
        exit 2
    fi

    if ! "$driver" "$level$ext" "$seed" >/dev/null 2>"$tmp/$name"; then
        echo "driver failed: $level seed $seed" >&2
        failed=1
        return
    fi

    if [ "$update" = yes ]; then
        cp "$tmp/$name" "$here/$goldendir/$name"
    elif ! diff -u "$here/$goldendir/$name" "$tmp/$name"; then
        echo "golden mismatch: $level seed $seed" >&2
        failed=1
    fi
}

for lv in $des_levels; do
    for seed in $seeds; do
        run "$here/dump_des" golden-des .lev "$lv" "$seed"
    done
done

for lv in $lua_levels; do
    for seed in $seeds; do
        run "$here/dump_lua" golden-lua .lua "$lv" "$seed"
    done
done

if [ "$update" = yes ]; then
    echo "golden files updated"
elif [ "$failed" -eq 0 ]; then
    echo "golden files match"
fi

exit "$failed"
