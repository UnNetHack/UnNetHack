/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Dungeon {

class d_flags {	/* dungeon/level type flags */
	var town = false	/* is this a town? (levels only) */
	var hellish = false	/* is this part of hell? */
	var maze_like = false /* is this a maze? */
	var rogue_like = false /* is this an old-fashioned presentation? */
	var align: Byte = 0	/* dungeon alignment. */
	var unused = false	/* etc... */
}

class d_level {	/* basic dungeon level element */
	var dnum: xchar = 0		/* dungeon number */
	var dlevel: xchar = 0		/* level number */
} 

class s_level {	/* special dungeon level element */
	var next: s_level = null
	var dlevel = new d_level()		/* dungeon & level numbers */
	var proto: String = null	/* name of prototype file (eg. "tower") */
	var boneid: Char = 0		/* character to id level in bones files */
	var rndlevs: uchar = 0	/* no. of randomly available similar levels */
	var flags = new d_flags()		/* type flags */
}

class stairway {	/* basic stairway identifier */
	var sx, sy: xchar = 0		/* x / y location of the stair */
	var tolev = new d_level()		/* where does it go */
	var up: Char = 0		/* what type of stairway (up/down) */
}

/* level region types */
val LR_DOWNSTAIR = 0
val LR_UPSTAIR = 1
val LR_PORTAL = 2
val LR_BRANCH = 3
val LR_TELE = 4
val LR_UPTELE = 5
val LR_DOWNTELE = 6

class dest_area {	/* non-stairway level change indentifier */
	var lx, ly: xchar = 0	/* "lower" left corner (near [0,0]) */
	var hx, hy: xchar = 0	/* "upper" right corner (near [COLNO,ROWNO]) */
	var nlx, nly: xchar = 0	/* outline of invalid area */
	var nhx, nhy: xchar = 0	/* opposite corner of invalid area */
} 

class dungeon {	/* basic dungeon identifier */
	var dname: String = null	/* name of the dungeon (eg. "Hell") */
	var proto: String = null	/* name of prototype file (eg. "tower") */
	var boneid: Char = 0		/* character to id dungeon in bones files */
	var flags = new d_flags()	/* dungeon flags */
	var entry_lev: xchar = 0	/* entry level */
	var num_dunlevs: xchar = 0	/* number of levels in this dungeon */
	var dunlev_ureached: xchar = 0	/* how deep you have been in this dungeon */
	var ledger_start,		/* the starting depth in "real" terms */
		depth_start = 0		/* the starting depth in "logical" terms */
}

/*
 * A branch structure defines the connection between two dungeons.  They
 * will be ordered by the dungeon number/level number of 'end1'.  Ties
 * are resolved by 'end2'.  'Type' uses 'end1' arbitrarily as the primary
 * point.
 */
class branch {
    var next: branch = null	/* next in the branch chain */
    var id = 0		/* branch identifier */
    var typ = 0		/* type of branch */ // MONOTE: Was named "type", a reserve word0
    var end1 = new d_level()		/* "primary" end point */
    var end2 = new d_level()		/* other end point */
    var end1_up = false	/* does end1 go up? */
}

/* branch types */
val BR_STAIR = 0	/* "Regular" connection, 2 staircases. */
val BR_NO_END1 = 1	/* "Regular" connection.  However, no stair from  */
			/*	end1 to end2.  There is a stair from end2 */
			/*	to end1.				  */
val BR_NO_END2 = 2	/* "Regular" connection.  However, no stair from  */
			/*	end2 to end1.  There is a stair from end1 */
			/*	to end2.				  */
val BR_PORTAL = 3	/* Connection by magic portals (traps) */


/* A particular dungeon contains num_dunlevs d_levels with dlevel 1..
 * num_dunlevs.  Ledger_start and depth_start are bases that are added
 * to the dlevel of a particular d_level to get the effective ledger_no
 * and depth for that d_level.
 *
 * Ledger_no is a bookkeeping number that gives a unique identifier for a
 * particular d_level (for level.?? files, e.g.).
 *
 * Depth corresponds to the number of floors below the surface.
 */
def Is_astralevel(x: d_level)		= (on_level(x, astral_level))
def Is_earthlevel(x: d_level)		= (on_level(x, earth_level))
def Is_waterlevel(x: d_level)		= (on_level(x, water_level))
def Is_firelevel(x: d_level)		= (on_level(x, fire_level))
def Is_airlevel(x: d_level)		= (on_level(x, air_level))
def Is_medusa_level(x: d_level)		= (on_level(x, medusa_level))
def Is_oracle_level(x: d_level)		= (on_level(x, oracle_level))
def Is_valley(x: d_level)		= (on_level(x, valley_level))
def Is_juiblex_level(x: d_level)	= (on_level(x, juiblex_level))
def Is_asmo_level(x: d_level)		= (on_level(x, asmodeus_level))
def Is_baal_level(x: d_level)		= (on_level(x, baalzebub_level))
def Is_wiz1_level(x: d_level)		= (on_level(x, wiz1_level))
def Is_wiz2_level(x: d_level)		= (on_level(x, wiz2_level))
def Is_wiz3_level(x: d_level)		= (on_level(x, wiz3_level))
def Is_sanctum(x: d_level)		= (on_level(x, sanctum_level))
def Is_portal_level(x: d_level)		= (on_level(x, portal_level))
def Is_rogue_level(x: d_level)		= (on_level(x, rogue_level))
def Is_stronghold(x: d_level)		= (on_level(x, stronghold_level))
def Is_bigroom(x: d_level)		= (on_level(x, bigroom_level))
def Is_qstart(x: d_level)		= (on_level(x, qstart_level))
def Is_qlocate(x: d_level)		= (on_level(x, qlocate_level))
def Is_nemesis(x: d_level)		= (on_level(x, nemesis_level))
def Is_knox(x: d_level)			= (on_level(x, knox_level))
def Is_nymph_level(x: d_level)		= (on_level(x, nymph_level))
def Is_mineend_level(x: d_level)	= (on_level(x, mineend_level))
def Is_sokoend_level(x: d_level)	= (on_level(x, sokoend_level))
def Is_blackmarket(x: d_level)		= (FALSE)
def Is_minetown_level(x: d_level)	= (on_level(x, minetown_level))
def Is_town_level(x: d_level)		= (on_level(x, town_level))
def Is_moria_level(x: d_level)		= (on_level(x, moria_level))

def In_sokoban(x: d_level)	= x.dnum == sokoban_dnum
/*** MOTODO
def Inhell			= In_hell(&u.uz)	/* now gehennom */
def Insheol  			= In_sheol(&u.uz)	/* now sheol */
***/
def In_endgame(x: d_level)	= xdnum == astral_level.dnum

def within_bounded_area(X: Int,Y: Int,LX: Int,LY: Int,HX: Int,HY: Int) =
		((X) >= (LX) && (X) <= (HX) && (Y) >= (LY) && (Y) <= (HY))

/* monster and object migration codes */

val MIGR_NOWHERE = -1	/* failure flag for down_gate() */
val MIGR_RANDOM = 0
val MIGR_APPROX_XY = 1	/* approximate coordinates */
val MIGR_EXACT_XY = 2	/* specific coordinates */
val MIGR_STAIRS_UP = 3
val MIGR_STAIRS_DOWN = 4
val MIGR_LADDER_UP = 5
val MIGR_LADDER_DOWN = 6
val MIGR_SSTAIRS = 7	/* dungeon branch */
val MIGR_PORTAL = 8	/* magic portal */
val MIGR_NEAR_PLAYER = 9	/* mon: followers; obj: trap door */

/* level information (saved via ledger number) */

class linfo {
	var flags: Short = 0
val VISITED = 0x01	/* hero has visited this level */
val FORGOTTEN = 0x02	/* hero will forget this level when reached */
val LFILE_EXISTS = 0x04	/* a level file exists for this level */
/*
 * Note:  VISITED and LFILE_EXISTS are currently almost always set at the
 * same time.  However they _mean_ different things.
 */
}

/* types and structures for dungeon map recording
 *
 * It is designed to eliminate the need for an external notes file for some of
 * the more mundane dungeon elements.  "Where was the last altar I passed?" etc...
 * Presumably the character can remember this sort of thing even if, months
 * later in real time picking up an old save game, I can't.
 *
 * To be consistent, one can assume that this map is in the player's mind and
 * has no physical correspondence (eliminating illiteracy/blind/hands/hands free
 * concerns.) Therefore, this map is not exaustive nor detailed ("some fountains").
 * This makes it also subject to player conditions (amnesia).
 */

/* Because clearly Nethack needs more ways to specify alignment */
def Amask2msa(x: aligntyp) = if(x == 4) 3 else (x & AM_MASK)
def Msa2amask(x: aligntyp) = if(x == 3) 4 else x
val MSA_NONE = 0  /* unaligned or multiple alignments */
val MSA_LAWFUL = 1
val MSA_NEUTRAL = 2
val MSA_CHAOTIC = 3

class mapseen_feat {
	/* feature knowledge that must be calculated from levl array */
	var nfount: Byte = 0
	var nsink: Byte = 0
	var naltar: Byte = 0
	var msalign: Byte = 0 /* corresponds to MSA_* above */
	var nthrone: Byte = 0
	var ntree: Byte = 0
	/* water, lava, ice are too verbose so commented out for now */
	/*
	var water = false
	var lava = false
	var ice = false
	*/

	/* calculated from rooms array */
	var nshop: Byte = 0
	var ntemple: Byte = 0
	var shoptype: Byte = 0

	var forgot = false /* player has forgotten about this level? */
} 

/* for mapseen->rooms */
val MSR_SEEN = 1

/* what the player knows about a single dungeon level */
/* initialized in mklev() */
class mapseen  {
	var next: mapseen = null /* next map in the chain */
	var br: branch = null /* knows about branch via taking it in goto_level */
	var lev = new d_level() /* corresponding dungeon level */

	var feat = new mapseen_feat()

	/* custom naming */
	var custom: String = null
	var custom_lth = 0

	/* maybe this should just be in struct mkroom? */
	var rooms = new Array[schar]((MAXNROFROOMS+1)*2)
}
}
