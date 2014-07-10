/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Coord._

object EShk {
val REPAIR_DELAY = 5	/* minimum delay between shop damage & repair */

val BILLSZ = 200

class bill_x {
	unsigned bo_id;
	boolean useup;
	long price;		/* price per unit */
	long bquan;		/* amount used up */
}

class eshk {
	var robbed = 0		/* amount stolen by most recent customer */
	var credit = 0		/* amount credited to customer */
	var debit = 0		/* amount of debt for using unpaid items */
	var loan = 0		/* shop-gold picked (part of debit) */
	var shoptype = 0	/* the value of rooms[shoproom].rtype */
	var shoproom: schar = 0	/* index in rooms; set by inshop() */
	var unused: schar = 0	/* to force alignment for stupid compilers */
	var following = false	/* following customer since he owes us sth */
	var surcharge = false	/* angry shk inflates prices */
	var cheapskate = false	/* offers less than normal for sold goods */
	var pbanned = false	/* player is banned from the shop */
	var shk = new coord()	/* usual position shopkeeper */
	var shd = new coord()	/* position shop door */
	var shoplevel = new d_level()	/* level (& dungeon) of his shop */
	var billct = 0		/* no. of entries of bill[] in use */
	var bill = new Array[bill_x](BILLSZ)
	var bill_p: bill_x = null
	var visitct = 0		/* nr of visits by most recent customer */
	var customer: String = null /* most recent customer */
	var shknam: String = null
}

// MOTODO #define ESHK(mon)	((struct eshk *)&(mon)->mextra[0])

// MOTODO #define NOTANGRY(mon)	((mon)->mpeaceful)
// MOTODO #define ANGRY(mon)	(!NOTANGRY(mon))
}
