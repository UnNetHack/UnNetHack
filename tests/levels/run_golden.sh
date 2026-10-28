#!/bin/sh
#
# Regenerate the special-level dumps and compare them with the committed
# golden files.
#
# The dumps must be produced by the same build as the game data that the
# driver reads through DLB (FILE_AREA_UNSHARE of the configured prefix),
# so run `make` before this script.  Run with --update to accept a change.
#
# Usage: sh tests/levels/run_golden.sh [--update]

set -u

here=$(dirname "$0")
driver="$here/dump_des"
update=no
[ "${1:-}" = "--update" ] && update=yes

levels="castle-1 medusa-1 asmod-1 baalz-1 knox-1 oracle valley-1 minetn-1 bigrm-1 soko1-1 juiblex sanctum"
seeds="42 7 1234"

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

if [ ! -x "$driver" ]; then
    echo "missing $driver; run: make -C tests levels/dump_des" >&2
    exit 2
fi

failed=0
for lv in $levels; do
    for seed in $seeds; do
        name="$lv.$seed.txt"
        if ! "$driver" "$lv.lev" "$seed" >/dev/null 2>"$tmp/$name"; then
            echo "driver failed: $lv seed $seed" >&2
            failed=1
            continue
        fi

        if [ "$update" = yes ]; then
            cp "$tmp/$name" "$here/golden/$name"
        elif ! diff -u "$here/golden/$name" "$tmp/$name"; then
            echo "golden mismatch: $lv seed $seed" >&2
            failed=1
        fi
    done
done

if [ "$update" = yes ]; then
    echo "golden files updated"
elif [ "$failed" -eq 0 ]; then
    echo "golden files match"
fi

exit "$failed"
