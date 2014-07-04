/* Copyright (c) Dean Luick, with acknowledgements to Dave Cohrs, 1990. */
/* NetHack may be freely redistributed.  See license for details.	*/

#ifndef VISION_H
#define VISION_H

#define COULD_SEE 0x1		/* location could be seen, if it were lit */
#define IN_SIGHT  0x2		/* location can be seen */
#define TEMP_LIT  0x4		/* location is temporarily lit */

/*
 * Light source sources
 */
#define LS_OBJECT 0
#define LS_MONSTER 1

/*
 *  cansee()	- Returns true if the hero can see the location.
 *
 *  couldsee()	- Returns true if the hero has a clear line of sight to
 *		  the location.
 */
#define cansee(x,y)	(isok(x,y) && (viz_array[y][x] & IN_SIGHT))
#define couldsee(x,y)	(isok(x,y) && (viz_array[y][x] & COULD_SEE))
#define templit(x,y)	(isok(x,y) && (viz_array[y][x] & TEMP_LIT))

/*
 *  The following assume the monster is not blind.
 *
 *  m_cansee()	- Returns true if the monster can see the given location.
 *
 *  m_canseeu() - Returns true if the monster could see the hero.  Assumes
 *		  that if the hero has a clear line of sight to the monster's
 *		  location and the hero is visible, then monster can see the
 *		  hero.
 */
#define m_cansee(mtmp,x2,y2)	clear_path((mtmp)->mx,(mtmp)->my,(x2),(y2))

#define m_canseeu(m)	((!Invis || perceives((m)->data)) && \
			  !(Underwater || u.uburied || (m)->mburied) ? \
			     couldsee((m)->mx,(m)->my) : 0)

/*
 *  Circle information
 */
#define MAX_RADIUS 15	/* this is in points from the source */

/* Use this macro to get a list of distances of the edges (see vision.c). */
#define circle_ptr(z) (&circle_data[(int)circle_start[z]])

#endif /* VISION_H */
