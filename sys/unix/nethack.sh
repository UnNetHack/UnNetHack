#!/bin/sh
#	SCCS Id: @(#)nethack.sh	3.4	1990/02/26

HACKDIR=/usr/games/lib/nethackdir
export HACKDIR
HACK=$HACKDIR/nethack
MAXNROFPLAYERS=4

# Since Nethack.ad is installed in HACKDIR, add it to XUSERFILESEARCHPATH
case "x$XUSERFILESEARCHPATH" in
x)	XUSERFILESEARCHPATH="$HACKDIR/%N.ad"
	;;
*)	XUSERFILESEARCHPATH="$XUSERFILESEARCHPATH:$HACKDIR/%N.ad"
	;;
esac
export XUSERFILESEARCHPATH

# copy default config file to home directory if none
# exists for unnethack or vanilla
if [ ! -e "$HOME/.unnethackrc" -a ! -e "$HOME/.nethackrc" ]
then
	cp "$HACKDIR/unnethackrc.default" "$HOME/.unnethackrc"
fi

cd $HACKDIR
case $1 in
	-s*)
		exec $HACK "$@"
		;;
	*)
		exec $HACK "$@" $MAXNROFPLAYERS
		;;
esac
