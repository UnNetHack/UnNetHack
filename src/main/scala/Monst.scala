/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import C2Scala._
import Config._
import Coord._
import Global._
import Obj._
import PerMonst._

object Monst {

/* The weapon_check flag is used two ways:
 * 1) When calling mon_wield_item, is 2-6 depending on what is desired.
 * 2) Between calls to mon_wield_item, is 0 or 1 depending on whether or not
 *    the weapon is known by the monster to be cursed (so it shouldn't bother
 *    trying for another weapon).
 * I originally planned to also use 0 if the monster already had its best
 * weapon, to avoid the overhead of a call to mon_wield_item, but it turns out
 * that there are enough situations which might make a monster change its
 * weapon that this is impractical.  --KAA
 */
val NO_WEAPON_WANTED = 0
val NEED_WEAPON = 1
val NEED_RANGED_WEAPON = 2
val NEED_HTH_WEAPON = 3
val NEED_PICK_AXE = 4
val NEED_AXE = 5
val NEED_PICK_OR_AXE = 6

/* The following flags are used for the second argument to display_minventory
 * in invent.c:
 *
 * MINV_NOLET  If set, don't display inventory letters on monster's inventory.
 * MINV_ALL    If set, display all items in monster's inventory, otherwise
 *	       just display wielded weapons and worn items.
 */
val MINV_NOLET = 0x01
val MINV_ALL = 0x02

class monst {
	var nmon: monst = null
	var data: permonst = null
	var m_id: size_t = _
	var mnum: Short = 0		/* permanent monster index number */
	var movement: Short = 0		/* movement points (derived from permonst definition and added effects */
	var m_lev: uchar = _		/* adjusted difficulty level of monster */
	var malign = A_NONE		/* alignment of this monster, relative to the
				  	 player (positive = good to kill) */
	var mx, my: xchar = _
	var mux, muy: xchar = _		/* where the monster thinks you are */
val MTSZ = 4
	var mtrack = new Array[coord](MTSZ) /* monster track */
	var mhp, mhpmax = 0
	var mappearance = 0		/* for undetected mimics and the wiz */
	var m_ap_type: uchar = _	/* what mappearance is describing: */
val M_AP_NOTHING = 0	/* mappearance is unused -- monster appears
				   as itself */
val M_AP_FURNITURE = 1	/* stairs, a door, an altar, etc. */
val M_AP_OBJECT = 2	/* an object */
val M_AP_MONSTER = 3	/* a monster */

	var mtame: schar = _		/* level of tameness, implies peaceful */
	var mintrinsics = 0		/* low 8 correspond to mresists */
	var mspec_used = 0		/* monster's special ability attack timeout */

	var mtrapped = 0		/* trapped in a pit, web or bear trap */
	var mfeetfrozen = 0 		/* monster's feet are frozen so it can't
					   move (0 = not frozen) */

	var female = false	/* is female */
	var minvis = false	/* currently invisible */
	var invis_blkd = false /* invisibility blocked */
	var perminvis = false	/* intrinsic minvis value */
	var cham: Byte = 0	/* shape-changer */
/* note: lychanthropes are handled elsewhere */
val CHAM_ORDINARY = 0	/* not a shapechanger */
val CHAM_CHAMELEON = 1	/* animal */
val CHAM_DOPPELGANGER = 2	/* demi-human */
val CHAM_SANDESTIN = 3	/* demon */
val CHAM_MAX_INDX = CHAM_SANDESTIN
	var mundetected = false	/* not seen in present hiding place */
				/* implies one of M1_CONCEAL or M1_HIDE,
				 * but not mimic (that is, snake, spider,
				 * trapper, piercer, eel)
				 */

	var mcan = false	/* has been cancelled */
	var mburied = false	/* has been buried */
	var mspeed: Byte = 0	/* current speed */
	var permspeed: Byte = 0	/* intrinsic mspeed value */
	var mrevived = false	/* has been revived from the dead */
	var mavenge = false	/* did something to deserve retaliation */

	var mflee = false	/* fleeing */
	var mfleetim: Byte = 0	/* timeout for mflee */

	var mcansee = false	/* cansee 1, temp.blinded 0, blind 0 */
	var mblinded: Byte = 0	/* cansee 0, temp.blinded n, blind 0 */

	var mcanmove = false	/* paralysis, similar to mblinded */
	var mfrozen: Byte = 0

	var msleeping = false	/* asleep until woken */
	var mstun = false	/* stunned (off balance) */
	var mconf = false	/* confused */
	var mpeaceful = false	/* does not attack unprovoked */
	var mleashed = false	/* monster is on a leash */
	var isshk = false	/* is shopkeeper */
	var isminion = false	/* is a minion */

	var isgd = false	/* is guard */
	var ispriest = false	/* is a priest */
	var iswiz = false	/* is the Wizard of Yendor */
	var wormno: Byte = 0	/* at most 31 worms on any level */
val MAX_NUM_WORMS = 32	/* should be 2^(wormno bitfield size) */

	var mstrategy = 0	/* for monsters with mflag3: current strategy */
val STRAT_ARRIVE = 0x40000000L	/* just arrived on current level */
val STRAT_WAITFORU = 0x20000000L
val STRAT_CLOSE = 0x10000000L
val STRAT_WAITMASK = 0x30000000L
val STRAT_HEAL = 0x08000000L
val STRAT_GROUND = 0x04000000L
val STRAT_MONSTR = 0x02000000L
val STRAT_PLAYER = 0x01000000L
val STRAT_NONE = 0x00000000L
val STRAT_STRATMASK = 0x0f000000L
val STRAT_XMASK = 0x00ff0000L
val STRAT_YMASK = 0x0000ff00L
val STRAT_GOAL = 0x000000ffL
def STRAT_GOALX(s: Int) = (s & STRAT_XMASK) >> 16
def STRAT_GOALY(s: Int)	= (s & STRAT_YMASK) >> 8

	var mtrapseen = 0		/* bitmap of traps we've been trapped in */
	var mlstmv = 0		/* for catching up with lost time */
	var mgold = 0
	var minvent: obj = null

	var mw: obj = null
	var misc_worn_check = 0
	var weapon_check: xchar = _

	var mgrlastattack = 0	    /* turn when a grouper last attacked.
				       at certain points, the attackers decide
				       to get back to attacking all at once. */


	var former_rank = new Array[Char](25)	/* for bones' ghost rank in the former life */

	var mnamelth: uchar = _		/* length of name (following mxlth) */
	var mxlth: Short = 0		/* length of following data */
	/* in order to prevent alignment problems mextra should
	   be (or follow) a long int */
	var meating = 0		/* monster is eating timeout */
	var mextra: Object = null /* monster dependent info */
}

/*
 * Note that mextra[] may correspond to any of a number of structures, which
 * are indicated by some of the other fields.
 *	isgd	 ->	struct egd
 *	ispriest ->	struct epri
 *	isshk	 ->	struct eshk
 *	isminion ->	struct emin
 *			(struct epri for roaming priests and angels, which is
 *			 compatible with emin for polymorph purposes)
 *	mtame	 ->	struct edog
 *			(struct epri for guardian angels, which do not eat
 *			 or do other doggy things)
 * Since at most one structure can be indicated in this manner, it is not
 * possible to tame any creatures using the other structures (the only
 * exception being the guardian angels which are tame on creation).
 */

// MOTODO #define newmonst(xl) (struct monst *)alloc((unsigned)(xl) + sizeof(struct monst))
// MOTODO #define dealloc_monst(mon) free((genericptr_t)(mon))

/* these are in mspeed */
val MSLOW = 1		/* slow monster */
val MFAST = 2		/* speeded monster */

def NAME(mtmp: monst) = mtmp.mextra.toString

def MON_WEP(mon: monst)	= mon.mw
def MON_NOWEP(mon: monst) = mon.mw = null

def DEADMONSTER(mon: monst) = mon.mhp < 1

/* Returns true if monster is trapped in place but can still attack.
 * (mon evaluated twice) */
def IS_TRAPPED(mon: monst) = mon.mtrapped || mon.mfeetfrozen
}
