#!/bin/bash -e

ulimit -c unlimited # dump core

DIR=/opt/nethack-tmp/games/lib/nethackdir

rm -f $DIR/level_info.txt $DIR/core $DIR/?lock.*

while [ ! -f $DIR/core ] ; do
	nice -20 /opt/nethack-tmp/games/nethack
done
