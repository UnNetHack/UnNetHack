#!/bin/bash
# Smoke tests for the dummy window port.
#
# Requires a built and installed game.  Every game runs headless with
# windowtype:dummy, an isolated HOME (no rc file, hence no marathon mode
# and no dump files) and its own player name.  The dummy port consumes
# one full stdin line per command.  Scenarios:
#   1. save+quit   moves, S, exit code 0, save file, no locks left
#   2. eof         input ends -> hangup autosave, exit 1, no locks left
#   3. restore     rerun the same player, quit (paranoid prompt: "yes")
#   4. death       rest 2000 turns, tombstone flow, xlogfile entry
#
# Usage:  bash tests/smoketest.sh
# Env:    GAME    game binary    (default: /tmp/unnethack install)
#         HACKDIR game directory (default: /tmp/unnethack install)
#         VARDIR  var directory  (default: $HACKDIR/../../var/unnethack)
#         SMOKETEST_TIMEOUT  per game timeout in seconds (default: 600)
#         SMOKETEST_KEEP=1   keep artifacts under /tmp/smoketest.*

set -u

GAME=${GAME:-/tmp/unnethack/share/unnethack/unnethack}
HACKDIR=${HACKDIR:-/tmp/unnethack/share/unnethack}
VARDIR=${VARDIR:-$HACKDIR/../../var/unnethack}
TMO=${SMOKETEST_TIMEOUT:-600}

[[ -x $GAME ]] || { echo "smoketest: game binary not found: $GAME (set GAME/HACKDIR)" >&2; exit 2; }
[[ -d $HACKDIR && -d $VARDIR ]] || { echo "smoketest: missing $HACKDIR or $VARDIR" >&2; exit 2; }
printf 'smoketest: game=%s\nsmoketest: hackdir=%s\n' "$GAME" "$HACKDIR"

TMP=$(mktemp -d /tmp/smoketest.XXXXXX) || exit 2
if [[ ${SMOKETEST_KEEP:-0} == 1 ]]; then
    trap 'printf "smoketest: artifacts kept in %s\n" "$TMP"' EXIT
else
    trap 'rm -rf "$TMP"' EXIT
fi
trap 'rm -rf "$TMP"; exit 2' INT TERM HUP

FAILED=0
ok()      { printf 'ok   %s\n' "$1"; }
fail()    { printf 'FAIL  %s\n' "$1"; (( ++FAILED )); }
rc_is()   { (( $2 == $1 )) && ok "$3" || fail "$3 (exit $2, expected $1)"; }
grep_ok() { grep -q "$2" "$3" && ok "$1" || fail "$1 (pattern '$2' missing in $3)"; }
has_files()  { ls "$1"/*"$2"* >/dev/null 2>&1; }
no_files()   { ! has_files "$1" "$2"; }

# play <player> <input> <outfile>: one scripted game, one command per line
play() {
    ( cd "$HACKDIR" && exec timeout "$TMO" env \
        HOME="$TMP" HACKDIR="$HACKDIR" \
        NETHACKOPTIONS="windowtype:dummy,name:$1" "$GAME" ) <<< "$2" > "$3"
}

rm -f "$VARDIR"/saves/*smk*.bz2 "$VARDIR"/smk*.whereis /tmp/unnethack_dumped_*smk*.dump*

# --- 1: save and quit ----------------------------------------------------------
play smkq $'h\nj\nk\nl\nS\ny\n\n' "$TMP/save.out"; rc=$?
rc_is 0 "$rc" "save+quit: exit 0"
grep_ok "save+quit: dummy port initialized" "dummy_init_nhwindows" "$TMP/save.out"
grep_ok "save+quit: game saved" "Saving..." "$TMP/save.out"
no_files "$VARDIR/level" smkq && ok "save+quit: no leftover locks" || fail "save+quit: lock files left behind"
has_files "$VARDIR/saves" smkq && ok "save+quit: save file exists" || fail "save+quit: no save file"

# --- 2: EOF ends the game via hangup autosave -----------------------------------
play smkr $'h\nj\nk\n' "$TMP/eof.out"; rc=$?
rc_is 1 "$rc" "eof: hangup autosave on EOF (exit 1)"
no_files "$VARDIR/level" smkr && ok "eof: no leftover locks" || fail "eof: lock files left behind"
has_files "$VARDIR/saves" smkr && ok "eof: save file exists" || fail "eof: no save file"

# --- 3: restore the autosave and quit --------------------------------------------
# a restored game asks for confirmation via getlin and expects the word "yes"
restore_input=$'\n\nh\n#\nquit\nyes\nn\n\n\n\n'
play smkr "$restore_input" "$TMP/restore.out"; rc=$?
rc_is 0 "$rc" "restore: exit 0"
grep_ok "restore: autosave restored" "welcome back" "$TMP/restore.out"
grep_ok "restore: end of game flow ran" "dummy_outrip\|create_nhwindow(NHW_TEXT)" "$TMP/restore.out"
no_files "$VARDIR/level" smkr && ok "restore: no leftover locks" || fail "restore: lock files left behind"

# --- 4: play until death -----------------------------------------------------------
dots=$(printf '.\n%.0s' {1..2000})
play smkd "$dots" "$TMP/death.out"; rc=$?
(( rc == 0 || rc == 1 )) && ok "death: game terminated (exit $rc)" || fail "death: unexpected exit $rc"
grep_ok "death: xlogfile entry written" "name=smkd" "$VARDIR/xlogfile"
grep_ok "death: tombstone flow ran" "dummy_outrip" "$TMP/death.out"
no_files "$VARDIR/level" smkd && ok "death: no leftover locks" || fail "death: lock files left behind"

rm -f "$VARDIR"/saves/*smk*.bz2 "$VARDIR"/smk*.whereis /tmp/unnethack_dumped_*smk*.dump*

if (( FAILED > 0 )); then
    for out in "$TMP"/*.out; do
        printf '\n--- tail of %s ---\n' "$out"
        grep -a '^dummy_' "$out" | tail -n 10
    done
    printf '%s check(s) failed\n' "$FAILED"
    exit 1
fi
printf 'all smoke tests passed\n'
exit 0
