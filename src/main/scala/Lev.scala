/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Global._

/*	Common include file for save and restore routines */
object Lev {

val COUNT_SAVE = 0x1
val WRITE_SAVE = 0x2
val FREE_SAVE = 0x4

/* operations of the various saveXXXchn & co. routines */
def perform_bwrite(mode: Int) = (mode != 0) & (COUNT_SAVE|WRITE_SAVE)
def release_data(mode: Int) = (mode != 0) & FREE_SAVE

/* The following are used in mkmaze.c */
class container {
	var next: container = null
	var x, y: xchar = _
	var what: Short = 0
	var list: Object = null
}

val CONS_OBJ = 0
val CONS_MON = 1
val CONS_HERO = 2
val CONS_TRAP = 3

class bubble {
	var x, y: xchar = _	/* coordinates of the upper left corner */
	var dx, dy: schar = _	/* the general direction of the bubble's movement */
	// MOTOD: Is a pointer
	// uchar *bm;	/* pointer to the bubble bit mask */
	var prev, next: bubble = null /* need to traverse the list up and down */
	var cons: container = null
}

/* used in light.c */
class light_source {
	var next: light_source = null
	var x, y: xchar = _		/* source's position */
	var range: Short = 0	/* source's current range */
	var flags: Short = 0
	var `type`: Short = 0		/* type of light source */
	var id: Object = null	/* source's identifier */
}
}
