/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Coord._
import Dungeon._
import Global._

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

object Trap {

/*** MONOTE: Was a union ***/
class vlaunchinfo {
	var v_launch_otyp: Short = 0	/* type of object to be triggered */
	val v_launch2 = new coord()	/* secondary launch point (for boulders) */
}

class trap {
	var ntrap: trap = null
	var x,ty: xchar = _
	val dst = new d_level()	/* destination for portals */
	var launch = new coord()
	var ttyp: Byte = 0
	var tseen = false
	var once = false
	var madeby_u = false /* So monsters may take offence when you trap
				 them.	Recognizing who made the trap isn't
				 completely unreasonable, everybody has
				 their own style.  This flag is also needed
				 when you untrap a monster.  It would be too
				 easy to make a monster peaceful if you could
				 set a trap for it and then untrap it. */
	var vl = new vlaunchinfo()
def launch_otyp	= vl.v_launch_otyp
def launch2	= vl.v_launch2
}

// MOTODO #define newtrap()	(struct trap *) alloc(sizeof(struct trap))
// MOTODO #define dealloc_trap(trap) free((genericptr_t) (trap))

/* reasons for statue animation */
val ANIMATE_NORMAL = 0
val ANIMATE_SHATTER = 1
val ANIMATE_SPELL = 2

/* reasons for animate_statue's failure */
val AS_OK = 0	/* didn't fail */
val AS_NO_MON = 1	/* makemon failed */
val AS_MON_IS_UNIQUE = 2	/* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
val NO_TRAP = 0
val ARROW_TRAP = 1
val DART_TRAP = 2
val ROCKTRAP = 3
val SQKY_BOARD = 4
val BEAR_TRAP = 5
val LANDMINE = 6
val ROLLING_BOULDER_TRAP = 7
val SLP_GAS_TRAP = 8
val RUST_TRAP = 9
val FIRE_TRAP = 10
val PIT = 11
val SPIKED_PIT = 12
val HOLE = 13
val TRAPDOOR = 14
val TELEP_TRAP = 15
val LEVEL_TELEP = 16
val MAGIC_PORTAL = 17
val WEB = 18
val STATUE_TRAP = 19
val MAGIC_TRAP = 20
val ANTI_MAGIC = 21
val ICE_TRAP = 22
val POLY_TRAP = 23
val TRAPNUM = 24
}
