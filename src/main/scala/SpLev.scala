/* Copyright (c) 1989 by Jean-Christophe Collet			  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import Config._
import Global._

object SpLev {

    /* wall directions */
val W_NORTH = 1
val W_SOUTH = 2
val W_EAST = 4
val W_WEST = 8
val W_ANY =	(W_NORTH|W_SOUTH|W_EAST|W_WEST)

    /* MAP limits */
val MAP_X_LIM = 76
val MAP_Y_LIM = 21

    /* Per level flags */
val NOTELEPORT = 0x00000001L
val HARDFLOOR = 0x00000002L
val NOMMAP = 0x00000004L
val SHORTSIGHTED = 0x00000008L
val ARBOREAL = 0x00000010L
val NOFLIPX = 0x00000020L
val NOFLIPY = 0x00000040L
val MAZELEVEL = 0x00000080L
val PREMAPPED = 0x00000100L
val SHROUD = 0x00000200L
val STORMY = 0x00000400L
val GRAVEYARD = 0x00000800L
val SKYMAP = 0x00001000L
val SHEOL_LEV = 0x00002000L
val FLAG_RNDVAULT = 0x00004000L


/* different level layout initializers */
val LVLINIT_NONE = 0
val LVLINIT_SOLIDFILL = 1
val LVLINIT_MAZEGRID = 2
val LVLINIT_MINES = 3
val LVLINIT_SHEOL = 4
val LVLINIT_ROGUE = 5

/* max. layers of object containment */
val MAX_CONTAINMENT = 10

/* max. # of random registers */
val MAX_REGISTERS = 10

/* max. nested depth of subrooms */
val MAX_NESTED_ROOMS = 5

/* max. # of opcodes per special level */
val SPCODER_MAX_RUNTIME = 65536

/* Opcodes for creating the level
 * If you change these, also change opcodestr[] in util/lev_main.c
 */
object opcode_defs extends Enumeration {
  type opcode_defs = Value
  val SPO_NULL,
  SPO_MESSAGE,
  SPO_MONSTER,
  SPO_OBJECT,
  SPO_ENGRAVING,
  SPO_ROOM,
  SPO_SUBROOM,
  SPO_DOOR,
  SPO_STAIR,
  SPO_LADDER,
  SPO_ALTAR,
  SPO_FOUNTAIN,
  SPO_SINK,
  SPO_POOL,
  SPO_TRAP,
  SPO_GOLD,
  SPO_CORRIDOR,
  SPO_LEVREGION,
  SPO_DRAWBRIDGE,
  SPO_MAZEWALK,
  SPO_NON_DIGGABLE,
  SPO_NON_PASSWALL,
  SPO_WALLIFY,
  SPO_MAP,
  SPO_ROOM_DOOR,
  SPO_REGION,
  SPO_MINERALIZE,
  SPO_CMP,
  SPO_JMP,
  SPO_JL,
  SPO_JLE,
  SPO_JG,
  SPO_JGE,
  SPO_JE,
  SPO_JNE,
  SPO_SPILL,
  SPO_TERRAIN,
  SPO_REPLACETERRAIN,
  SPO_EXIT,
  SPO_ENDROOM,
  SPO_POP_CONTAINER,
  SPO_PUSH,
  SPO_POP,
  SPO_RN2,
  SPO_DEC,
  SPO_INC,
  SPO_MATH_ADD,
  SPO_MATH_SUB,
  SPO_MATH_MUL,
  SPO_MATH_DIV,
  SPO_MATH_MOD,
  SPO_MATH_SIGN,
  SPO_COPY,
  SPO_MON_GENERATION,
  SPO_END_MONINVENT,
  SPO_GRAVE,
  SPO_FRAME_PUSH,
  SPO_FRAME_POP,
  SPO_CALL,
  SPO_RETURN,
  SPO_INITLEVEL,
  SPO_LEVEL_FLAGS,
  SPO_LEVEL_SOUNDS,
  SPO_WALLWALK,
  SPO_VAR_INIT, /* variable_name data */
  SPO_SHUFFLE_ARRAY,
  SPO_DICE,
  SPO_COREFUNC,

  SPO_SEL_ADD,
  SPO_SEL_POINT,
  SPO_SEL_RECT,
  SPO_SEL_FILLRECT,
  SPO_SEL_LINE,
  SPO_SEL_RNDLINE,
  SPO_SEL_GROW,
  SPO_SEL_FLOOD,
  SPO_SEL_RNDCOORD,
  SPO_SEL_ELLIPSE,
  SPO_SEL_FILTER,
  SPO_SEL_GRADIENT,
  SPO_SEL_COMPLEMENT,

  MAX_SP_OPCODES = Value
}

/* "Functions" exposed from the core to the level compiler.
 * Function handling is in spo_corefunc() in sp_lev.c
 * See also core_vars[] in util/lev_main.c for their public
 * names and what parameters they take and return.
 */
object corefuncs extends Enumeration {
  type corefuncs = Value
  val COREFUNC_NONE,
    COREFUNC_LEVEL_DIFFICULTY,
    COREFUNC_LEVEL_DEPTH,
    COREFUNC_DISCORDIAN_HOLIDAY,
    COREFUNC_PIRATEDAY,
    COREFUNC_APRILFOOLSDAY,
    COREFUNC_PIDAY,
    COREFUNC_TOWELDAY,
    COREFUNC_MIDNIGHT,
    COREFUNC_NIGHT,
    COREFUNC_FRIDAY_13TH,
    COREFUNC_POM,
    COREFUNC_YYYYMMDD,
    COREFUNC_PLNAME,
    COREFUNC_TOSTRING,
    COREFUNC_TOINT,
    COREFUNC_TOCOORD,
    COREFUNC_TOREGION,
    COREFUNC_RN2,
    COREFUNC_SOBJ_AT,
    COREFUNC_MON_AT,
    COREFUNC_CARRYING,
    COREFUNC_ROLE,
    COREFUNC_RACE,

    COREFUNC_ROOM_WID,
    COREFUNC_ROOM_HEI,
    COREFUNC_ROOM_X,
    COREFUNC_ROOM_Y,

    /* variable methods */
    COREFUNC_COORD_X,
    COREFUNC_COORD_Y,
    COREFUNC_ARRAY_LEN,

    NUM_COREFUNCS = Value
}

/* MONSTER and OBJECT can take a variable number of parameters,
 * they also pop different # of values from the stack. So,
 * first we pop a value that tells what the _next_ value will
 * mean.
 */
/* MONSTER */
val SP_M_V_PEACEFUL = 0
val SP_M_V_ALIGN = 1
val SP_M_V_ASLEEP = 2
val SP_M_V_APPEAR = 3
val SP_M_V_NAME = 4

val SP_M_V_FEMALE = 5
val SP_M_V_INVIS = 6
val SP_M_V_CANCELLED = 7
val SP_M_V_REVIVED = 8
val SP_M_V_AVENGE = 9
val SP_M_V_FLEEING = 10
val SP_M_V_BLINDED = 11
val SP_M_V_PARALYZED = 12
val SP_M_V_STUNNED = 13
val SP_M_V_CONFUSED = 14
val SP_M_V_SEENTRAPS = 15

val SP_M_V_END = 16 /* end of variable parameters */

/* OBJECT */
val SP_O_V_SPE = 0
val SP_O_V_CURSE = 1
val SP_O_V_CORPSENM = 2
val SP_O_V_NAME = 3
val SP_O_V_QUAN = 4
val SP_O_V_BURIED = 5
val SP_O_V_LIT = 6
val SP_O_V_ERODED = 7
val SP_O_V_LOCKED = 8
val SP_O_V_TRAPPED = 9
val SP_O_V_RECHARGED = 10
val SP_O_V_INVIS = 11
val SP_O_V_GREASED = 12
val SP_O_V_BROKEN = 13
val SP_O_V_COORD = 14
val SP_O_V_END = 15 /* end of variable parameters */

/* When creating objects, we need to know whether
 * it's a container and/or contents.
 */
val SP_OBJ_CONTENT = 0x1
val SP_OBJ_CONTAINER = 0x2

/* SPO_FILTER types */
val SPOFILTER_PERCENT = 0
val SPOFILTER_SELECTION = 1
val SPOFILTER_MAPCHAR = 2

/* gradient filter types */
val SEL_GRADIENT_RADIAL = 0
val SEL_GRADIENT_SQUARE = 1

val SPOVAR_NULL = 0x00
val SPOVAR_INT = 0x01 /* l */
val SPOVAR_STRING = 0x02 /* str */
val SPOVAR_VARIABLE = 0x03 /* str (contains the variable name) */
val SPOVAR_COORD = 0x04 /* coordinate, encoded in l; use SP_COORD_X() and SP_COORD_Y() */
val SPOVAR_REGION = 0x05 /* region, encoded in l; use SP_REGION_X1() etc */
val SPOVAR_MAPCHAR = 0x06 /* map char, in l */
val SPOVAR_MONST = 0x07 /* monster class & specific monster, encoded in l; use SP_MONST_... */
val SPOVAR_OBJ = 0x08 /* object class & specific object type, encoded in l; use SP_OBJ_... */
val SPOVAR_SEL = 0x09 /* selection. char[COLNO][ROWNO] in str */
val SPOVAR_ARRAY = 0x40 /* used in splev_var & lc_vardefs, not in opvar */

val SP_COORD_IS_RANDOM = 0x01000000
/* Humidity flags for get_location() and friends, used with SP_COORD_PACK_RANDOM() */
val DRY = 0x1
val WET = 0x2
val HOT = 0x4
val SOLID = 0x8
val ANY_LOC = 0x10 /* even outside the level */
val NO_LOC_WARN = 0x20 /* no complaints and set x & y to -1, if no loc */

def SP_COORD_X(l: Int) =	(l & 0xff)
def SP_COORD_Y(l: Int)	= ((l >> 16) & 0xff)
def SP_COORD_PACK(x: Int,y: Int) = ((( x ) & 0xff) + ((( y ) & 0xff) << 16))
def SP_COORD_PACK_RANDOM(f: Int) = (SP_COORD_IS_RANDOM | (f))

def SP_REGION_X1(l: Int)	= (l & 0xff)
def SP_REGION_Y1(l: Int)	= ((l >> 8) & 0xff)
def SP_REGION_X2(l: Int)	= ((l >> 16) & 0xff)
def SP_REGION_Y2(l: Int)	= ((l >> 24) & 0xff)
def SP_REGION_PACK(x1: Int,y1: Int,x2: Int,y2: Int) = ((( x1 ) & 0xff) + ((( y1 ) & 0xff) << 8) + ((( x2 ) & 0xff) << 16) + ((( y2 ) & 0xff) << 24))

def SP_MONST_CLASS(l: Int) = (l & 0xff)
def SP_MONST_PM(l: Int)	  = ((l >> 8) & 0xffff)
def SP_MONST_PACK(m: Int,c: Int) = ((( m ) << 8) + c.toChar)

def SP_OBJ_CLASS(l: Int)	  = (l & 0xff)
def SP_OBJ_TYP(l: Int)	  = ((l >> 8) & 0xffff)
def SP_OBJ_PACK(o: Int,c: Int)  = ((( o ) << 8) + c.toChar)

def SP_MAPCHAR_TYP(l: Int) = (l & 0xff)
def SP_MAPCHAR_LIT(l: Int) = ((l >> 8) & 0xff)
def SP_MAPCHAR_PACK(typ: Int,lit: Int) = ((( lit ) << 8) + typ.toChar)

class opvar {
    var spovartyp: xchar = _ /* one of SPOVAR_foo */
    val vardata = new {  // MONOTE Was a union
      var str: String = nulil
      var l = 0
    }
}

class splev_var {
    var next: splev_var = null
    var name: String = null
    var svtyp: xchar = _ /* SPOVAR_foo */
    val data = new {  // MONOTE Was a union
	    var value: opvar = null
      val arrayvalues: Array[Array[opvar]] = null
    }
    var array_len = 0
}

class splevstack {
  var depth = 0
  var depth_alloc = 0
  var stackdata: Array[Array[opvar]] = null
}

class sp_frame {
  var next: sp_frame = null
  var stack: splevstack = null
  var variables: splev_var = null
  var n_opcode = 0
}

class sp_coder {
  var stack: splevstack = null
  var frame: sp_frame = null
  var allow_flips = 0
  var premapped = 0
  var croom: mkroom = null
  val tmproomlist = new Array[mkroom](MAX_NESTED_ROOMS+1)
  val failed_room = new Array[Boolean](MAX_NESTED_ROOMS+1)
  var n_subroom = 0
  var exit_script = false
  var lvl_is_joined = 0

  var opcode = 0  /* current opcode */
  var opdat: opvar = null /* current push data (req. opcode == SPO_PUSH) */
}

/* special level coder CPU flags */
val SP_CPUFLAG_LT = 1
val SP_CPUFLAG_GT = 2
val SP_CPUFLAG_EQ = 4
val SP_CPUFLAG_ZERO = 8

/*
 * Structures manipulated by the special levels loader & compiler
 */

type packed_coord = Long
class unpacked_coord {
  var is_random: xchar = _
  var getloc_flags = 0
  var x, y = 0
}

class opcmp {
	var cmp_what = 0
	var cmp_val = 0
}

class opjmp {
	var jmp_target = 0
}

class Str_or_Len {
  var str: String = null
  var len = 0
}

class lev_init {
	var init_style: xchar = _ /* one of LVLINIT_foo */
	var fg, bg: Char = 0
	var smoothed, joined = false
	var lit, walled: xchar = _
	var flags = 0
	var filling: schar = _
}

class room_door {
	var wall, pos, secret, mask: xchar = _
}

class trap {
  val coord: packed_coord = 0
  var x, y, `type`: xchar = _ // MONOTE: Rename "type" so it isn't a reserved word
}

class monster {
	val name = new Str_or_Len()
  val appear_as = new Str_or_Len();
	var id: Short = 0
	var align = A_NONE
  val coord: packed_coord = 0
	var x, y, `class`, appear: xchar = _
	var peaceful, asleep: schar = _
  var female, invis, cancelled, revived, avenge, fleeing, blinded, paralyzed, stunned, confused: Short = 0
  var seentraps = 0
	var has_invent: Short = 0
}

class `object` { // MOTODO: Rename class so it's not a reserved word
	val name = new Str_or_Len()
	var corpsenm = 0
	var id, spe: Short = 0
  val coord: packed_coord = 0
	var x, y, `class`, containment: xchar = _
	var curse_state: schar = _
	var quan = 0
	var buried: Short = 0
	var lit: Short = 0
  var eroded, locked, trapped, recharged, invis, greased, broken: Short = _
}

class altar {
  val coord: packed_coord = 0
	var x, y: xchar = _
	var	align = A_NONE
	var shrine: xchar = _
}

class region {
	var x1, y1, x2, y2: xchar = _
	var rtype, rlit, rirreg: xchar = _
}

class terrain {
  var ter, tlit: xchar = _
}

class replaceterrain {
  var chance: xchar = _
  var x1,y1,x2,y2: xchar = _
  var fromter, toter, tolit: xchar = _
}

/* values for rtype are defined in dungeon.h */
class lev_region {
	val inarea = new  { var x1, y1, x2, y2: xchar = _ }
  val delarea = new { var x1, y1, x2, y2: xchar = _ }
	var in_islev, del_islev = false
  var rtype, padding: xchar = _
	val rname = new Str_or_Len()
}

class room {
	val name = new Str_or_Len()
	val parent = new Str_or_Len()
	var x, y, w, h: xchar = _
	var xalign, yalign: xchar = _
	var rtype, chance, rlit, filled, joined: xchar = _
}

class mazepart {
	var zaligntyp: schar = _
	var keep_region: schar = _
	var halign, valign: schar = _
	var xsize, ysize:  Char = 0
	var map: Array[Array[Char]] = null
}

class corridor {
  class corridor_item {
    var room: xchar = _
    var wall: xchar = _
    var door: xchar = _
  }
  val src = new corridor_item()
  var dest =new corridor_item()
}

class _opcode {
  var opcode = 0
  var opdat: opvar = null
}

class sp_lev {
  var opcodes: _opcode = null
  var n_opcodes = 0
}

class spill {
	var x, y, direction, count, lit: xchar = _
	var typ: Char = 0
}

/* only used by lev_comp */
class lc_funcdefs_parm {
    var name: String = null
    var parmtype: Char = 0
    var next: lc_funcdefs_parm = null
}

class lc_funcdefs {
    var next: lc_funcdefs = null
    var name: String = null
    var addr = 0
    var code = new sp_lev()
    var n_called = 0
    var params: lc_funcdefs_parm = null
    var n_params = 0
}

class lc_vardefs {
    var next: lc_vardefs = null
    var name: String = null
    var var_type = 0 /* SPOVAR_foo */
    var n_used = 0
}

class lc_breakdef {
    var next: lc_breakdef = null
    var breakpoint: opvar = null
    var break_depth = 0
}
}
