/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Coord._
import Decl._
import Global._
import Monst._
import Obj._

object Rm {

/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation.	See drawing.c for
 * the code that permits the user to set the contents of the symbol structure.
 *
 * The door representation was changed by Ari Huttunen(ahuttune@niksula.hut.fi)
 */

/*
 * TLCORNER	TDWALL		TRCORNER
 * +-		-+-		-+
 * |		 |		 |
 *
 * TRWALL	CROSSWALL	TLWALL		HWALL
 * |		 |		 |
 * +-		-+-		-+		---
 * |		 |		 |
 *
 * BLCORNER	TUWALL		BRCORNER	VWALL
 * |		 |		 |		|
 * +-		-+-		-+		|
 */

/* Level location types */
object LevelLocationType extends Enumeration {
	type LevelLocationType = Value
    val STONE,
    VWALL,
    HWALL,
    TLCORNER,
    TRCORNER,
    BLCORNER,
    BRCORNER,
    CROSSWALL,	/* For pretty mazes and special levels */
    TUWALL,
    TDWALL,
    TLWALL,
    TRWALL,
    DBWALL,
    TREE,	/* KMH */
    DEADTREE,	/* youkan */
    ICEWALL,
    CRYSTALICEWALL,
    SDOOR,
    SCORR,
    POOL,
    MOAT,	/* pool that doesn't boil, adjust messages */
    WATER,
    DRAWBRIDGE_UP,
    LAVAPOOL,
    IRONBARS,	/* KMH */
    DOOR,
    CORR,
    ROOM,
    STAIRS,
    LADDER,
    FOUNTAIN,
    THRONE,
    SINK,
    GRAVE,
    ALTAR,
    ICE,
    BOG,
    DRAWBRIDGE_DOWN,
    AIR,
    CLOUD,
    MAX_TYPE = Value
}
val INVALID_TYPE = 127

/*
 * Avoid using the level types in inequalities:
 * these types are subject to change.
 * Instead, use one of the macros below.
 */

  import LevelLocationType._
def IS_WALL(typ: LevelLocationType) =	(typ != null && typ <= DBWALL)
def IS_STWALL(typ: LevelLocationType) =	(typ <= DBWALL)	/* STONE <= (typ) <= DBWALL */
def IS_ROCK(typ: LevelLocationType) =	(typ < POOL)		/* absolutely nonaccessible */
def IS_ICEWALL(typ: LevelLocationType) = (typ == ICEWALL)
def IS_CRYSTALICEWALL(typ: LevelLocationType) = (typ == CRYSTALICEWALL)
def IS_ANY_ICEWALL(typ: LevelLocationType) = (IS_ICEWALL(typ) || IS_CRYSTALICEWALL(typ))
def IS_DOOR(typ: LevelLocationType) =	(typ == DOOR)
def IS_TREE(typ: LevelLocationType) =	(typ == TREE || 
			(level.flags.arboreal && typ == STONE))
def IS_DEADTREE(typ: LevelLocationType) = (typ == DEADTREE)
def IS_TREES(typ: LevelLocationType) =	IS_TREE(typ) || IS_DEADTREE(typ)
def ACCESSIBLE(typ: LevelLocationType) = (typ >= DOOR)		/* good position */
def IS_ROOM(typ: LevelLocationType) =	(typ >= ROOM)		/* ROOM, STAIRS, furniture.. */
def ZAP_POS(typ: LevelLocationType) =	(typ >= POOL)
def SPACE_POS(typ: LevelLocationType) =	(typ > DOOR)
def IS_POOL(typ: LevelLocationType) =	(typ >= POOL && typ <= DRAWBRIDGE_UP)
def IS_THRONE(typ: LevelLocationType) =	(typ == THRONE)
def IS_FOUNTAIN(typ: LevelLocationType) = (typ == FOUNTAIN)
def IS_SINK(typ: LevelLocationType) =	(typ == SINK)
def IS_GRAVE(typ: LevelLocationType) =	(typ == GRAVE)
def IS_ALTAR(typ: LevelLocationType) =	(typ == ALTAR)
def IS_SWAMP(typ: LevelLocationType) =	(typ == BOG)
def IS_DRAWBRIDGE(typ: LevelLocationType) = (typ == DRAWBRIDGE_UP || typ == DRAWBRIDGE_DOWN)
def IS_FURNITURE(typ: LevelLocationType) = (typ >= STAIRS && typ <= ALTAR)
def IS_AIR(typ: LevelLocationType) =	(typ == AIR || (typ) == CLOUD)
def IS_SOFT(typ: LevelLocationType) =	(typ == AIR || (typ) == CLOUD || IS_POOL(typ) || typ == BOG)
def IS_LAVA(typ: LevelLocationType) =	(typ == LAVAPOOL)

/*
 * The screen symbols may be the default or defined at game startup time.
 * See drawing.c for defaults.
 * Note: {ibm|dec}_graphics[] arrays (also in drawing.c) must be kept in synch.
 */

/* begin dungeon characters */

object DungeonCharacters extends Enumeration {
	type DungeonCharacters = Value
    val S_stone,
    S_vwall,
    S_hwall,
    S_tlcorn,
    S_trcorn,
    S_blcorn,
    S_brcorn,
    S_crwall,
    S_tuwall,
    S_tdwall,
    S_tlwall,
    S_trwall,
    S_ndoor,
    S_vodoor,
    S_hodoor,
    S_vcdoor,	/* closed door, vertical wall */
    S_hcdoor,	/* closed door, horizontal wall */
    S_bars,	/* KMH -- iron bars */
    S_tree,	/* KMH */
    S_deadtree,	/* youkan */
    S_room,
    S_darkroom,
    S_corr,
    S_litcorr,
    S_upstair,
    S_dnstair,
    S_upladder,
    S_dnladder,
    S_altar,
    S_grave,
    S_throne,
    S_sink,
    S_fountain,
    S_pool,
    S_ice,
    S_bog,
    S_lava,
    S_vodbridge,
    S_hodbridge,
    S_vcdbridge,	/* closed drawbridge, vertical wall */
    S_hcdbridge,	/* closed drawbridge, horizontal wall */
    S_air,
    S_cloud,
    S_icewall,
    S_crystalicewall,
    S_water,

/* end dungeon characters, begin traps */

    S_arrow_trap,
    S_dart_trap,
    S_falling_rock_trap,
    S_squeaky_board,
    S_bear_trap,
    S_land_mine,
    S_rolling_boulder_trap,
    S_sleeping_gas_trap,
    S_rust_trap,
    S_fire_trap,
    S_pit,
    S_spiked_pit,
    S_hole,
    S_trap_door,
    S_teleportation_trap,
    S_level_teleporter,
    S_magic_portal,
    S_web,
    S_statue_trap,
    S_magic_trap,
    S_anti_magic_trap,
    S_ice_trap,
    S_polymorph_trap,

/* end traps, begin special effects */


    S_vbeam,	/* The 4 zap beam symbols.  Do NOT separate. */
    S_hbeam,	/* To change order or add, see function     */
    S_lslant,	/* zapdir_to_glyph() in display.c.	    */
    S_rslant,
    S_digbeam,	/* dig beam symbol */
    S_flashbeam,	/* camera flash symbol */
    S_boomleft,	/* thrown boomerang, open left, e.g ')'    */
    S_boomright,	/* thrown boomerand, open right, e.g. '('  */
    S_ss1,	/* 4 magic shield glyphs */
    S_ss2,
    S_ss3,
    S_ss4,

/* The 8 swallow symbols.  Do NOT separate.  To change order or add, see */
/* the function swallow_to_glyph() in display.c.			 */
    S_sw_tl,		/* swallow top left [1]			*/
    S_sw_tc,		/* swallow top center [2]	Order:	*/
    S_sw_tr,		/* swallow top right [3]		*/
    S_sw_ml,		/* swallow middle left [4]	1 2 3	*/
    S_sw_mr,		/* swallow middle right [6]	4 5 6	*/
    S_sw_bl,		/* swallow bottom left [7]	7 8 9	*/
    S_sw_bc,		/* swallow bottom center [8]		*/
    S_sw_br,		/* swallow bottom right [9]		*/

    S_explode1,		/* explosion top left			*/
    S_explode2,		/* explosion top center			*/
    S_explode3,		/* explosion top right		 Ex.	*/
    S_explode4,		/* explosion middle left		*/
    S_explode5,		/* explosion middle center	 /-\	*/
    S_explode6,		/* explosion middle right	 |@|	*/
    S_explode7,		/* explosion bottom left	 \-/	*/
    S_explode8,		/* explosion bottom center		*/
    S_explode9,		/* explosion bottom right		*/

/* end effects */

    MAXPCHARS = Value		/* maximum number of mapped characters */
}

import DungeonCharacters._
val MAXDCHARS =	(S_water+1)	/* maximum of mapped dungeon characters */
val MAXTCHARS = (S_polymorph_trap-S_water)	/* maximum of mapped trap characters */
val MAXECHARS = (S_explode9-S_vbeam+1)	/* maximum of mapped effects characters */
val MAXEXPCHARS = 9	/* number of explosion characters */

def DARKROOMSYM	 = if(Is_rogue_level(u.uz)) S_stone else S_darkroom

class symdef {
    var sym: uchar = _
    var explanation: String = null
    var color: uchar = _
}

/*
 * Graphics sets for display symbols
 */
val ASCII_GRAPHICS = 0	/* regular characters: '-', '+', &c */
val IBM_GRAPHICS = 1	/* PC graphic characters */
val DEC_GRAPHICS = 2	/* VT100 line drawing characters */
val CURS_GRAPHICS = 4	/* Portable curses drawing characters */
val UTF8_GRAPHICS = 5	/* UTF8 characters */

/*
 * The 5 possible states of doors
 */

val D_NODOOR = 0
val D_BROKEN = 1
val D_ISOPEN = 2
val D_CLOSED = 4
val D_LOCKED = 8
val D_TRAPPED = 16
val D_SECRET = 32 /* only used by sp_lev.c, NOT in rm-struct */

/*
 * Some altars are considered as shrines, so we need a flag.
 */
val AM_SHRINE = 8

/*
 * Thrones should only be looted once.
 */
val T_LOOTED = 1

/*
 * Trees have more than one kick result.
 */
val TREE_LOOTED = 1
val TREE_SWARM = 2
val TREE_FLOCK = 3

/*
 * Fountains have limits, and special warnings.
 */
val F_LOOTED = 1
val F_WARNED = 2
def FOUNTAIN_IS_WARNED(x: Int, y: Int) =	(levl(x,y).looted & F_WARNED)
def FOUNTAIN_IS_LOOTED(x: Int, y: Int) =	(levl(x,y).looted & F_LOOTED)
def SET_FOUNTAIN_WARNED(x: Int, y: Int) =	levl(x,y).looted |= F_WARNED
def SET_FOUNTAIN_LOOTED(x: Int, y: Int) =	levl(x,y).looted |= F_LOOTED
def CLEAR_FOUNTAIN_WARNED(x: Int, y: Int) =	levl(x,y).looted &= ~F_WARNED
def CLEAR_FOUNTAIN_LOOTED(x: Int, y: Int) =	levl(x,y).looted &= ~F_LOOTED

/*
 * Doors are even worse :-) The special warning has a side effect
 * of instantly trapping the door, and if it was defined as trapped,
 * the guards consider that you have already been warned!
 */
val D_WARNED = 16

/*
 * Sinks have 3 different types of loot that shouldn't be abused
 */
val S_LPUDDING = 1
val S_LDWASHER = 2
val S_LRING = 4

/*
 * The four directions for a DrawBridge.
 */
val DB_NORTH = 0
val DB_SOUTH = 1
val DB_EAST = 2
val DB_WEST = 3
val DB_DIR = 3	/* mask for direction */

/*
 * What's under a drawbridge.
 */
//val DB_MOAT = 0
//val DB_LAVA = 4
//val DB_ICE = 8
//val DB_FLOOR = 16
val DB_MOAT = 0x00
val DB_LAVA = 0x04
val DB_ICE = 0x08
val DB_FLOOR = 0x0c
val DB_BOG = 0x10
val DB_GROUND = 0x14
val DB_UNDER = 28	/* mask for underneath */

/*
 * Wall information.
 */
val WM_MASK = 0x07	/* wall mode (bottom three bits) */
val W_NONDIGGABLE = 0x08
val W_NONPASSWALL = 0x10

/*
 * Ladders (in Vlad's tower) may be up or down.
 */
val LA_UP = 1
val LA_DOWN = 2

/*
 * Room areas may be iced pools
 */
val ICED_BOG = 4
val ICED_POOL = 8
val ICED_MOAT = 16

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
class rm {
	var glyph = 0		/* what the hero thinks is there */
	var typ: schar = _		/* what is really there */
	var styp: Byte = 0	/* last seen/touched dungeon typ */
	var seenv: uchar = _		/* seen vector */
	var flags: Byte = 0	/* extra information for typ */
	var horizontal = false /* wall/door/etc is horiz. (more typ info) */
	var lit = false	/* speed hack for lit rooms */
	var waslit = false	/* remember if a location was lit */
	var roomno: Byte = 0	/* room # for special rooms */
	var edge = false	/* marks boundaries for special rooms*/
	var stepped_on = false	/* player has stepped on this square */

// MONOTE: Was a #define, had global scope
def blessedftn = horizontal  /* a fountain that grants attribs */
def disturbed =	horizontal  /* a grave that has been disturbed */
}

def SET_TYPLIT(x: Int,y: Int,ttyp: LevelLocationType,llit: Int) = {
  if ((x) >= 0 && (y) >= 0 && (x) < COLNO && (y) < ROWNO) {
    if ((ttyp) < MAX_TYPE) levl(x,y).typ = (ttyp)
    if ((ttyp) == LAVAPOOL) levl(x,y).lit = 1
    else if (llit != -2) {
	if (llit == -1) levl(x,y).lit = rn2(2);
	else levl(x,y).lit = (llit)
    }
  }
}

/*
 * Add wall angle viewing by defining "modes" for each wall type.  Each
 * mode describes which parts of a wall are finished (seen as as wall)
 * and which are unfinished (seen as rock).
 *
 * We use the bottom 3 bits of the flags field for the mode.  This comes
 * in conflict with secret doors, but we avoid problems because until
 * a secret door becomes discovered, we know what sdoor's bottom three
 * bits are.
 *
 * The following should cover all of the cases.
 *
 *	type	mode				Examples: R=rock, F=finished
 *	-----	----				----------------------------
 *	WALL:	0 none				hwall, mode 1
 *		1 left/top (1/2 rock)			RRR
 *		2 right/bottom (1/2 rock)		---
 *							FFF
 *
 *	CORNER: 0 none				trcorn, mode 2
 *		1 outer (3/4 rock)			FFF
 *		2 inner (1/4 rock)			F+-
 *							F|R
 *
 *	TWALL:	0 none				tlwall, mode 3
 *		1 long edge (1/2 rock)			F|F
 *		2 bottom left (on a tdwall)		-+F
 *		3 bottom right (on a tdwall)		R|F
 *
 *	CRWALL: 0 none				crwall, mode 5
 *		1 top left (1/4 rock)			R|F
 *		2 top right (1/4 rock)			-+-
 *		3 bottom left (1/4 rock)		F|R
 *		4 bottom right (1/4 rock)
 *		5 top left & bottom right (1/2 rock)
 *		6 bottom left & top right (1/2 rock)
 */

val WM_W_LEFT = 1			/* vertical or horizontal wall */
val WM_W_RIGHT = 2
val WM_W_TOP = WM_W_LEFT
val WM_W_BOTTOM = WM_W_RIGHT

val WM_C_OUTER = 1			/* corner wall */
val WM_C_INNER = 2

val WM_T_LONG = 1			/* T wall */
val WM_T_BL = 2
val WM_T_BR = 3

val WM_X_TL = 1			/* cross wall */
val WM_X_TR = 2
val WM_X_BL = 3
val WM_X_BR = 4
val WM_X_TLBR = 5
val WM_X_BLTR = 6

/*
 * Seen vector values.	The seen vector is an array of 8 bits, one for each
 * octant around a given center x:
 *
 *			0 1 2
 *			7 x 3
 *			6 5 4
 *
 * In the case of walls, a single wall square can be viewed from 8 possible
 * directions.	If we know the type of wall and the directions from which
 * it has been seen, then we can determine what it looks like to the hero.
 */
val SV0 = 0x1
val SV1 = 0x2
val SV2 = 0x4
val SV3 = 0x8
val SV4 = 0x10
val SV5 = 0x20
val SV6 = 0x40
val SV7 = 0x80
val SVALL = 0xFF

def doormask	= flags
def altarmask	= flags
def wall_info	= flags
def ladder	= flags
def drawbridgemask = flags
def looted	= flags
def icedpool	= flags

class damage {
	var next: damage = null
	var when, cost = 0 
	val place = new coord()
	var typ: schar = _
}

class levelflags {
	var nfountains: uchar = _		/* number of fountains on level */
	var nsinks: uchar = _			/* number of sinks on the level */
	/* Several flags that give hints about what's on the level */
	var has_shop = false
	var has_vault = false
	var has_zoo = false
	var has_court = false
	var has_morgue = false
	var has_garden = false
	var has_beehive = false
	var has_barracks = false
	var has_temple = false
	var has_lemurepit = false

	var has_swamp = false
	var noteleport = false
	var hardfloor = false
	var nommap = false
	var hero_memory = false	/* hero has memory */
	var shortsighted = false	/* monsters are shortsighted */
	var graveyard = false		/* has_morgue, but remains set */
	var is_maze_lev = false
	var stormy = false		/* thunderous clouds */

	var is_cavernous_lev = false
	var arboreal = false		/* Trees replace rock */
	var sky = false		/* map has sky instead of ceiling */
	var sheol = false
}

class mon_gen_tuple {
    var freq = 0
    var is_sym = false
    var monid = 0
    var next: mon_gen_tuple = null
}

class mon_gen_override {
    var override_chance = 0
    var total_mon_freq = 0
    var gen_chances: mon_gen_tuple = null
}

val LVLSND_HEARD = 0	/* You_hear(msg); */
val LVLSND_PLINED = 1	/* pline(msg); */
val LVLSND_VERBAL = 2	/* verbalize(msg); */
val LVLSND_FELT = 3	/* You_feel(msg); */

class lvl_sound_bite {
    var flags = 0 /* LVLSND_foo */
    var msg: String = null
}

class lvl_sounds {
    var freq = 0
    var n_sounds = 0
    var sounds: lvl_sound_bite = null
}

class dlevel_t {
    val locations: Array[Array[rm]] = Array.ofDim(COLNO, ROWNO)
    val objects: Array[Array[obj]] = Array.ofDim(COLNO, ROWNO)
    val monsters: Array[Array[monst]] = Array.ofDim(COLNO, ROWNO)
    var objlist: obj = null
    var buriedobjlist: obj = null
    var monlist: monst = null
    var damagelist: damage = null
    val flags = new levelflags()
    var mon_gen: mon_gen_override = null
    var sounds: lvl_sounds = null
}

/*
 * Macros for compatibility with old code. Someday these will go away.
 */
def levl		= level.locations
def fobj		= level.objlist
def fmon		= level.monlist

/*
 * Covert a trap number into the defsym graphics array.
 * Convert a defsym number into a trap number.
 * Assumes that arrow trap will always be the first trap.
 */
def trap_to_defsym(t: Int) = (S_arrow_trap+(t)-1)
def defsym_to_trap(d: Int) = ((d)-S_arrow_trap+1)

def OBJ_AT(x: Int,y: Int) = level.objects(x,y) != null
/*
 * Macros for encapsulation of level.monsters references.
 */
def MON_AT(x: Int, y: Int): Boolean =	(level.monsters(x,y) != null &&
			 !(level.monsters(x,y))->mburied)
def MON_BURIED_AT(x: Int, y: Int): Boolean =	(level.monsters(x,y) != null &&
				(level.monsters(x,y))->mburied)
def place_worm_seg(m: Int,x: Int, y: Int) =level.monsters(x,y) = m
def m_at(x: Int, y: Int) =		if(MON_AT(x,y)) level.monsters(x,y) else null
def m_buried_at(x: Int, y: Int) =	if(MON_BURIED_AT(x,y)) level.monsters(x,y) else null
}
