/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Global {

val BETA = true /* if a beta-test copy	[MRS] */

/*
 * Files expected to exist in the playground directory if file areas are not
 * enabled and in the named areas otherwise.
 */

val NH_RECORD = "record"  /* a file containing list of topscorers */
val NH_RECORD_AREA = FILE_AREA_VAR
val NH_HELP = "help"	  /* a file containing command descriptions */
val NH_HELP_AREA = FILE_AREA_SHARE
val NH_SHELP = "hh"		/* abbreviated form of the same */
val NH_SHELP_AREA = FILE_AREA_SHARE
val NH_DEBUGHELP = "wizhelp"	/* a file containing debug mode cmds */
val NH_DEBUGHELP_AREA = FILE_AREA_SHARE
val NH_RUMORFILE = "rumors"	/* a file with fortune cookies */
val NH_RUMORAREA = FILE_AREA_SHARE
val NH_ORACLEFILE = "oracles"	/* a file with oracular information */
val NH_ORACLEAREA = FILE_AREA_SHARE
val NH_DATAFILE = "data"	/* a file giving the meaning of symbols used */
val NH_DATAAREA = FILE_AREA_SHARE
val NH_CMDHELPFILE = "cmdhelp"	/* file telling what commands do */
val NH_CMDHELPAREA = FILE_AREA_SHARE
val NH_HISTORY = "history"	/* a file giving nethack's history */
val NH_HISTORY_AREA = FILE_AREA_SHARE
val NH_LICENSE = "license"	/* file with license information */
val NH_LICENSE_AREA = FILE_AREA_DOC
val NH_OPTIONFILE = "opthelp"	/* a file explaining runtime options */
val NH_OPTIONAREA = FILE_AREA_SHARE
val NH_OPTIONS_USED = "options"	/* compile-time options, for #version */
val NH_OPTIONS_USED_AREA = FILE_AREA_SHARE
val NH_GUIDEBOOK = "Guidebook.txt"       /* Nethack Guidebook*/
val NH_GUIDEBOOK_AREA = FILE_AREA_DOC


val LEV_EXT = ".lev"		/* extension for special level files */
val RECORD = "record"	/* file containing list of topscorers */
val HELP = "help"	/* file containing command descriptions */
val SHELP = "hh"	/* abbreviated form of the same */
val DEBUGHELP = "wizhelp" /* file containing debug mode cmds */
val RUMORFILE = "rumors"	/* file with fortune cookies */
val ORACLEFILE = "oracles" /* file with oracular information */
val DATAFILE = "data"	/* file giving the meaning of symbols used */
val CMDHELPFILE = "cmdhelp" /* file telling what commands do */
val HISTORY = "history" /* file giving nethack's history */
val LICENSE = "license" /* file with license information */
val OPTIONFILE = "opthelp" /* file explaining runtime options */
val OPTIONS_USED = "options" /* compile-time options, for #version */

val LEV_EXT = ".lev"		/* extension for special level files */

/* Assorted definitions that may depend on selections in config.h. */

/*
 * type xchar: small integers in the range 0 - 127, usually coordinates
 * although they are nonnegative they must not be declared unsigned
 * since otherwise comparisons with signed quantities are done incorrectly
 */
type xchar = schar
type boolean = xchar /* 0 or 1 */

val TRUE = true
val FALSE = false

def strcmpi(a: String,b: String) = strncmpi((a),(b),-1)

/*** MOTODO: Decide what savefile compression to use 
/* comment out to test effects of each #define -- these will probably
 * disappear eventually
 */
# define RLECOMP	/* run-length compression of levl array - JLee */
# define ZEROCOMP	/* zero-run compression of everything - Olaf Seibert */
***/

type CHAR_P = char
type SCHAR_P = schar
type UCHAR_P = uchar
type XCHAR_P = xchar
type SHORT_P = short
type BOOLEAN_P = boolean
type ALIGNTYP_P = aligntyp
type OBJ_P = obj
type MONST_P = monst

// MOTODO: Not direct analogue of sizeof
// #define SIZE(x) (int)(sizeof(x) / sizeof(x[0]))


/* A limit for some NetHack int variables.  It need not, and for comparable
 * scoring should not, depend on the actual limit on integers for a
 * particular machine, although it is set to the minimum required maximum
 * signed integer for C (2^15 -1).
 */
val LARGEST_INT = 32767

/* Displayable name of this port; don't redefine if defined in *conf.h */
val PORT_ID = "Scala"
val PORT_SUB_ID = "UnNetHack" // MOTODO Is port sub ID needed?  What should it be?

val EXIT_SUCCESS = 0
val EXIT_FAILURE = 1

/*** MOTODO: Finish if needed
#define Sprintf  (void) sprintf
#define Strcat   (void) strcat
#define Strcpy   (void) strcpy
#ifdef NEED_VARARGS
#define Vprintf  (void) vprintf
#define Vfprintf (void) vfprintf
#define Vsprintf (void) vsprintf
#endif
***/

/* Used for consistency checks of various data files; declare it here so
   that utility programs which include config.h but not hack.h can see it. */
class version_info {
	var incarnation = 0L	/* actual version number */
	var feature_set = 0L	/* bitmask of config settings */
	var entity_count = 0L	/* # of monsters and objects */
	var struct_sizes = 0L	/* size of key structs */
}

/*
 * Configurable internal parameters.
 *
 * Please be very careful if you are going to change one of these.  Any
 * changes in these parameters, unless properly done, can render the
 * executable inoperative.
 */

/* size of terminal screen is (at least) (ROWNO+3) by COLNO */
val COLNO = 80
val ROWNO = 21

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *	Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *	T:123456 Satiated Conf FoodPois Ill Blind Stun Hallu Overloaded
 * -- or somewhat over 130 characters
 */
val MAXCO = if (COLNO <= 140) 160 else (COLNO+20)

val MAXNROFROOMS = 40	/* max number of rooms per level */
val MAX_SUBROOMS = 24	/* max # of subrooms in a given room */
val DOORMAX = 120	/* max number of doors per level */

val BUFSZ = 256	/* for getlin buffers */
val QBUFSZ = 128	/* for building question text */
val TBUFSZ = 300	/* toplines[] buffer max msg: 3 81char names */
				/* plus longest prefix plus a few extra words */

val PL_NSIZ = 32	/* name of player, ghost, shopkeeper */
val PL_CSIZ = 32	/* sizeof pl_character */
val PL_FSIZ = 32	/* fruit name */
val PL_PSIZ = 63	/* player-given names for pets, other
				 * monsters, objects */

val MAXDUNGEON = 16	/* current maximum number of dungeons */
val MAXLEVEL = 32	/* max number of levels in one dungeon */
val MAXSTAIRS = 1	/* max # of special stairways in a dungeon */
val ALIGNWEIGHT = 4	/* generation weight of alignment */

val MAXULEV = 30	/* max character experience level */

val MAXMONNO = 120	/* extinct monst after this number created */
val MHPMAX = 500	/* maximum monster hp */
}
