/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Coord._

object EDog {

/*	various types of food, the lower, the better liked.	*/

val DOGFOOD = 0
val CADAVER = 1
val ACCFOOD = 2
val MANFOOD = 3
val APPORT = 4
val POISON = 5
val UNDEF = 6
val TABU = 7

class edog {
	var uroptime =  0			/* moment dog dropped object */
	var dropdist = 0		/* dist of drpped obj from @ */
	var int apport = 0			/* amount of training */
	var whistletime = 0		/* last time he whistled */
	var hungrytime = 0		/* will get hungry at this time */
	var ogoal = new coord()		/* previous goal location */
	var abuse = 0			/* track abuses to this pet */
	var revivals = 0			/* count pet deaths */
	var mhpmax_penalty = 0		/* while starving, points reduced */
	var killed_by_u = false	/* you attempted to kill him */
}
// MOTODO #define EDOG(mon)	((struct edog *)&(mon)->mextra[0])

}
