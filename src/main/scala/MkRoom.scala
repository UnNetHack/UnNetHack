/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Decl._
import Global._
import Monst._

object MkRoom {
/* mkroom.h - types and structures for room and shop initialization */

class mkroom {
	var lx,hx,ly,hy: schar = _	/* usually xchar, but hx may be -1 */
	var rtype: schar = _		/* type of room (zoo, throne, etc...) */
	var rlit: schar = _		/* is the room lit ? */
	var needfill: schar = _		/* does the room need filling? */
	var needjoining: schar = _
	var doorct: schar = _		/* door count */
	var fdoor: schar = _		/* index for the first door of the room */
	var nsubrooms: schar = _	/* number of subrooms */
	var irregular = false		/* true if room is non-rectangular */
	val sbrooms = new Array[mkroom](MAX_SUBROOMS)  /* Subrooms pointers */
	val resident: monst = null /* priest/shopkeeper/guard for this room */
};

class shclass {
	var name: String = null	/* name of the shop type */
	var symb: Char = 0	/* this identifies the shop type */
	var prob = 0		/* the shop type probability in % */
	var shdist: schar = _	/* object placement type */
val D_SCATTER = 0	/* normal placement */
val D_SHOP = 1	/* shop-like placement */
val D_TEMPLE = 2	/* temple-like placement */
	class itp {
	    var iprob = 0	/* probability of an item type */
	    var itype = 0	/* item type: if >=0 a class, if < 0 a specific item */
	}
	val iprobs = new Array[itp](20)
	var shknms: List[String] = List()	/* list of shopkeeper names for this type */
};

/* the normal rooms on the current level are described in rooms[0..n] for
 * some n<MAXNROFROOMS
 * the vault, if any, is described by rooms[n+1]
 * the next rooms entry has hx -1 as a flag
 * there is at most one non-vault special room on a level
 */

/* values for rtype in the room definition structure */
val OROOM = 0	/* ordinary room */
val COURT = 2	/* contains a throne */
val SWAMP = 3	/* contains pools */
val VAULT = 4	/* contains piles of gold */
val BEEHIVE = 5	/* contains killer bees and royal jelly */
val MORGUE = 6	/* contains corpses, undead and ghosts */
val BARRACKS = 7	/* contains soldiers and their gear */
val ZOO = 8	/* floor covered with treasure and monsters */
val DELPHI = 9	/* contains Oracle and peripherals */
val TEMPLE = 10	/* contains a shrine */
val LEPREHALL = 11	/* leprechaun hall (Tom Proudfoot) */
val COCKNEST = 12	/* cockatrice nest (Tom Proudfoot) */
val ANTHOLE = 13	/* ants (Tom Proudfoot) */
val GARDEN = 14	/* nymphs, trees and fountains */
val ARMORY = 15	/* weapons, armor and rust monsters (L) */
val LEMUREPIT = 16	/* contains lemures and horned devils */
val POOLROOM = 17	/*  */
val RNDVAULT = 18
val SHOPBASE = 20	/* everything above this is a shop */
val ARMORSHOP	= SHOPBASE+ 1	/* specific shop defines for level compiler */
val SCROLLSHOP	= SHOPBASE+ 2
val POTIONSHOP	= SHOPBASE+ 3
val WEAPONSHOP	= SHOPBASE+ 4
val FOODSHOP	= SHOPBASE+ 5
val RINGSHOP	= SHOPBASE+ 6
val WANDSHOP	= SHOPBASE+ 7
val TOOLSHOP	= SHOPBASE+ 8
val BOOKSHOP	= SHOPBASE+ 9
val TINSHOP		= SHOPBASE+10
val INSTRUMENTSHOP	= SHOPBASE+11
val PETSHOP		= SHOPBASE+12	/* Stephen White */
val UNIQUESHOP	= SHOPBASE+13	/* shops here & above not randomly gen'd. */
val CANDLESHOP = UNIQUESHOP
val MAXRTYPE        = UNIQUESHOP      /* maximum valid room type */

/* Special type for search_special() */
val ANY_TYPE = -1
val ANY_SHOP = -2

val NO_ROOM = 0	/* indicates lack of room-occupancy */
val SHARED = 1	/* indicates normal shared boundary */
val SHARED_PLUS = 2	/* indicates shared boundary - extra adjacent-
				 * square searching required */

val ROOMOFFSET = 3	/*
				 * (levl[x][y].roomno - ROOMOFFSET) gives
				 * rooms[] index, for inside-squares and
				 * non-shared boundaries.
				 */

/*** MOTODO Pointer math
def IS_ROOM_PTR(x: Int) =		((x) >= rooms && (x) < rooms + MAXNROFROOMS)
def IS_ROOM_INDEX(x: Int) =	((x) >= 0 && (x) < MAXNROFROOMS)
def IS_SUBROOM_PTR(x: Int) =	((x) >= subrooms && 
			 (x) < subrooms + MAXNROFROOMS)
def IS_SUBROOM_INDEX(x: Int) =	((x) > MAXNROFROOMS && (x) < (MAXNROFROOMS*2))
def ROOM_INDEX(x: Int) =		((x) - rooms)
def SUBROOM_INDEX(x: Int) =	((x) - subrooms)
def IS_LAST_ROOM_PTR(x: Int) =	(ROOM_INDEX(x) == nroom)
def IS_LAST_SUBROOM_PTR(x: Int) =	(!nsubroom || SUBROOM_INDEX(x) == nsubroom)
***/
}
