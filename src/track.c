/*  SCCS Id: @(#)track.c    3.4 87/08/08    */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* track.c - version 1.0.2 */

#include "hack.h"

#define UTSZ    50

static NEARDATA int utcnt, utpnt;
static NEARDATA coord utrack[UTSZ];

void
initrack(void)
{
    utcnt = utpnt = 0;
}

/* add to track */
void
settrack(void)
{
    if(utcnt < UTSZ) utcnt++;
    if(utpnt == UTSZ) utpnt = 0;
    utrack[utpnt].x = u.ux;
    utrack[utpnt].y = u.uy;
    utpnt++;
}

coord *
gettrack(coordxy x, coordxy y)
{
    int cnt, ndist;
    coord *tc;
    cnt = utcnt;
    for(tc = &utrack[utpnt]; cnt--; ) {
        if(tc == utrack) tc = &utrack[UTSZ-1];
        else tc--;
        ndist = distmin(x, y, tc->x, tc->y);

        /* if far away, skip track entries til we're closer */
        if(ndist > 2) {
            ndist -= 2; /* be careful due to extra decrement at top of loop */
            cnt -= ndist;
            if(cnt <= 0)
                return (coord *) 0; /* too far away, no matches possible */
            if(tc < &utrack[ndist])
                tc += (UTSZ-ndist);
            else
                tc -= ndist;
        } else if(ndist <= 1)
            return(ndist ? tc : 0);
    }
    return (coord *)0;
}

/*track.c*/
