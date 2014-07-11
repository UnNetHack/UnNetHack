/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Engrave {

class engr {
	var nxt_engr: engr = null
	var engr_txt: String = null
	var engr_x, engr_y: xchar = 0
	var engr_lth = 0	/* for save & restore; not length of text */
	var engr_time = 0;	/* moment engraving was (will be) finished */
	var engr_type: xchar = 0
val DUST = 1
val ENGRAVE = 2
val BURN = 3
val MARK = 4
val ENGR_BLOOD = 5
val HEADSTONE = 6
val N_ENGRAVE = 6
}

// MOTODO #define newengr(lth) (struct engr *)alloc((unsigned)(lth) + sizeof(struct engr))
// MOTODO #define dealloc_engr(engr) free((genericptr_t) (engr))

}
