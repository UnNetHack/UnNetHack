/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._

object Hack {

val TELL = 1
val NOTELL = 0
val ON = 1
val OFF = 0
val BOLT_LIM = 8 /* from this distance ranged attacks will be made */
val MAX_CARR_CAP = 1000	/* so that boulders can be heavier */

/* symbolic names for capacity levels */
val UNENCUMBERED = 0
val SLT_ENCUMBER = 1	/* Burdened */
val MOD_ENCUMBER = 2	/* Stressed */
val HVY_ENCUMBER = 3	/* Strained */
val EXT_ENCUMBER = 4	/* Overtaxed */
val OVERLOADED = 5	/* Overloaded */

/* Macros for how a rumor was delivered in outrumor() */
val BY_ORACLE = 0
val BY_COOKIE = 1
val BY_PAPER = 2
val BY_OTHER = 9

/* Macros for why you are no longer riding */
val DISMOUNT_GENERIC = 0
val DISMOUNT_FELL = 1
val DISMOUNT_THROWN = 2
val DISMOUNT_POLY = 3
val DISMOUNT_ENGULFED = 4
val DISMOUNT_BONES = 5
val DISMOUNT_BYCHOICE = 6
val DISMOUNT_VANISHED = 7

/* Special returns from mapglyph() */
val MG_CORPSE = 0x01
val MG_INVIS = 0x02
val MG_DETECT = 0x04
val MG_PET = 0x08
val MG_RIDDEN = 0x10
val MG_INVERSE = 0x20 /* use inverse video */

/* sellobj_state() states */
val SELL_NORMAL = 0
val SELL_DELIBERATE = 1
val SELL_DONTSELL = 2

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.  Some parts of the
 * code assume that PANIC separates the deaths from the non-deaths.
 */
val DIED = 0
val CHOKING = 1
val POISONING = 2
val STARVING = 3
val DROWNING = 4
val BURNING = 5
val DISSOLVED = 6
val CRUSHING = 7
val STONING = 8
val TURNED_SLIME = 9
val GENOCIDED = 10
val DISINTEGRATED = 11
val MAX_SURVIVABLE_DEATH = 11
val PANICKED = 12
val TRICKED = 13
val QUIT = 14
val ESCAPED = 15
val DEFIED = 16
val ASCENDED = 17

/* types of calls to bhit() */
val ZAPPED_WAND = 0
val THROWN_WEAPON = 1
val KICKED_WEAPON = 2
val FLASHED_LIGHT = 3
val INVIS_BEAM = 4

val NO_SPELL = 0

/* flags to control makemon() */
val NO_MM_FLAGS = 0x00	/* use this rather than plain 0 */
val NO_MINVENT = 0x01	/* suppress minvent when creating mon */
val MM_NOWAIT = 0x02	/* don't set STRAT_WAITMASK flags */
val MM_EDOG = 0x04	/* add edog structure */
val MM_EMIN = 0x08	/* add emin structure */
val MM_ANGRY = 0x10  /* monster is created angry */
val MM_NONAME = 0x20  /* monster is not christened */
val MM_NOCOUNTBIRTH = 0x40  /* don't increment born counter (for revival) */
val MM_IGNOREWATER = 0x80	/* ignore water when positioning */
val MM_ADJACENTOK = 0x100 /* it is acceptable to use adjacent coordinates */

/* special mhpmax value when loading bones monster to flag as extinct or genocided */
val DEFUNCT_MONSTER = -100

/* flags for special ggetobj status returns */
val ALL_FINISHED = 0x01  /* called routine already finished the job */

/* flags to control query_objlist() */
val BY_NEXTHERE = 0x1	/* follow objlist by nexthere field */
val AUTOSELECT_SINGLE = 0x2	/* if only 1 object, don't ask */
val USE_INVLET = 0x4	/* use object's invlet */
val INVORDER_SORT = 0x8	/* sort objects by packorder */
val SIGNAL_NOMENU = 0x10	/* return -1 rather than 0 if none allowed */
val FEEL_COCKATRICE = 0x20  /* engage cockatrice checks and react */

/* Flags to control query_category() */
/* BY_NEXTHERE used by query_category() too, so skip 0x01 */
val UNPAID_TYPES = 0x02
val GOLD_TYPES = 0x04
val WORN_TYPES = 0x08
val ALL_TYPES = 0x10
val BILLED_TYPES = 0x20
val CHOOSE_ALL = 0x40
val BUC_BLESSED = 0x80
val BUC_CURSED = 0x100
val BUC_UNCURSED = 0x200
val BUC_UNKNOWN = 0x400
val BUC_ALLBKNOWN = (BUC_BLESSED|BUC_CURSED|BUC_UNCURSED)
val UNIDENTIFIED_TYPES = 0x800
val ALL_TYPES_SELECTED = -2

/* Flags to control find_mid() */
val FM_FMON = 0x01	/* search the fmon chain */
val FM_MIGRATE = 0x02	/* search the migrating monster chain */
val FM_MYDOGS = 0x04	/* search mydogs */
val FM_EVERYWHERE = (FM_FMON | FM_MIGRATE | FM_MYDOGS)

/* Flags to control pick_[race,role,gend,align] routines in role.c */
val PICK_RANDOM = 0
val PICK_RIGID = 1

/* Flags to control dotrap() in trap.c */
val NOWEBMSG = 0x01	/* suppress stumble into web message */
val FORCEBUNGLE = 0x02	/* adjustments appropriate for bungling */
val RECURSIVETRAP = 0x04	/* trap changed into another type this same turn */

/* Flags to control test_move in hack.c */
val DO_MOVE = 0	/* really doing the move */
val TEST_MOVE = 1	/* test a normal move (move there next) */
val TEST_TRAV = 2	/* test a future travel location */
val TEST_TRAP = 3	/* check if a future travel location is a trap */

/*** some utility macros ***/
def yn(query: String) = yn_function(query,ynchars, 'n')
def ynq(query: String) = yn_function(query,ynqchars, 'q')
def ynaq(query: String) = yn_function(query,ynaqchars, 'y')
def nyaq(query: String) = yn_function(query,ynaqchars, 'n')
def nyNaq(query: String) = yn_function(query,ynNaqchars, 'n')
def ynNaq(query: String) = yn_function(query,ynNaqchars, 'y')

/* Macros for scatter */
val VIS_EFFECTS = 0x01	/* display visual effects */
val MAY_HITMON = 0x02	/* objects may hit monsters */
val MAY_HITYOU = 0x04	/* objects may hit you */
val MAY_HIT = (MAY_HITMON|MAY_HITYOU)
val MAY_DESTROY = 0x08	/* objects may be destroyed at random */
val MAY_FRACTURE = 0x10	/* boulders & statues may fracture */

/* Macros for launching objects */
val ROLL = 0x01	/* the object is rolling */
val FLING = 0x02	/* the object is flying thru the air */
val LAUNCH_UNSEEN = 0x40	/* hero neither caused nor saw it */
val LAUNCH_KNOWN = 0x80	/* the hero caused this by explicit action */

/* Macros for explosion types */
val EXPL_DARK = 0
val EXPL_NOXIOUS = 1
val EXPL_MUDDY = 2
val EXPL_WET = 3
val EXPL_MAGICAL = 4
val EXPL_FIERY = 5
val EXPL_FROSTY = 6
val EXPL_MAX = 7

/* Macros for messages referring to hands, eyes, feet, etc... */
val ARM = 0
val EYE = 1
val FACE = 2
val FINGER = 3
val FINGERTIP = 4
val FOOT = 5
val HAND = 6
val HANDED = 7
val HEAD = 8
val LEG = 9
val LIGHT_HEADED = 10
val NECK = 11
val SPINE = 12
val TOE = 13
val HAIR = 14
val BLOOD = 15
val LUNG = 16
val NOSE = 17
val STOMACH = 18

/* Flags to control menus */
// MOTODO #define MENUTYPELEN sizeof("traditional ")
val MENU_TRADITIONAL = 0
val MENU_COMBINATION = 1
val MENU_PARTIAL = 2
val MENU_FULL = 3

val MENU_SELECTED = TRUE
val MENU_UNSELECTED = FALSE

val MENU_DEFCNT = 1

/*
 * Option flags
 * Each higher number includes the characteristics of the numbers
 * below it.
 */
val SET_IN_FILE = 0 /* config file option only */
val SET_VIA_PROG = 1 /* may be set via extern program, not seen in game */
val DISP_IN_GAME = 2 /* may be set via extern program, displayed in game */
val SET_IN_GAME = 3 /* may be set via extern program or set in the game */

def FEATURE_NOTICE_VER(major: Long ,minor: Long, patch: Long) = (major << 24) | (minor << 16) | (patch << 8) 

def FEATURE_NOTICE_VER_MAJ = (flags.suppress_alert >> 24)
def FEATURE_NOTICE_VER_MIN = (((0x0000000000FF0000L & flags.suppress_alert)) >> 16)
def FEATURE_NOTICE_VER_PATCH = (((0x000000000000FF00L & flags.suppress_alert)) >>  8)

def max(a: Long,b: Long) = if(a > b) a else b
def min(x: Long,y: Long) = if(x < y) x else y
def plur(x: Long) = if(x == 1) "" else "s"

def ARM_BONUS(obj: obj) = (objects(obj.otyp).a_ac + obj.spe +
			 /* extra AC point for racial armor */ 
			 (if(is_racial_armor(obj)) 1 else 0)
			 - min(greatest_erosion(obj),objects(obj.otyp).a_ac))

/*** MOTODO
def makeknown(x)	discover_object((x),TRUE,TRUE)
def distu(xx,yy)	dist2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)
def onlineu(xx,yy)	online2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)
***/

def rn1(x: Int,y: Int) = rn2(x)+(y)

/* negative armor class is randomly weakened to prevent invulnerability */
def AC_VALUE(AC: Int) = if((AC) >= 0 ) (AC) else -rnd(-(AC))
}
