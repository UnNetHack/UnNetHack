/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */
/* track.c - version 1.0.2 */

import Coord._

object Track {

val UTSZ = 50
private var utcnt, utpnt: Int = 0
private var utrack = new Array[coord](UTSZ)

private def initrack(): Unit = 
{
	utcnt = utpnt = 0
}

/* add to track */
private def settrack(): Unit =
{
	if(utcnt < UTSZ) utcnt = utcnt + 1
	if(utpnt == UTSZ) utpnt = 0
	utrack(utpnt).x = u.ux
	utrack(utpnt).y = u.uy
	utpnt = utpnt + 1
}

private def gettrack(x: Int, y: Int): coord =
{
	var ndist = 0
	var cnt = utcnt
/*** MOTODO: Convert
    register coord *tc;
    for(tc = &utrack[utpnt]; cnt--; ){
	if(tc == utrack) tc = &utrack[UTSZ-1];
	else tc = tc - 1
	ndist = distmin(x,y,tc.x,tc.y)

	/* if far away, skip track entries til we're closer */
	if(ndist > 2) {
	    ndist = ndist - 2 /* be careful due to extra decrement at top of loop */
	    cnt = cnt - ndist
	    if(cnt <= 0)
		return null /* too far away, no matches possible */
	    if(tc < &utrack[ndist])
		tc = tc + (UTSZ-ndist)
	    else
		tc = tc - ndist;
	} else if(ndist <= 1)
	    return if(ndist != 0) tc else 0
    }
***/
    null
}
}
