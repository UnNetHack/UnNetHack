/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import Attrib._
import Monst._
import Skills._

object You {
/*** Substructures ***/

class RoleName(val m: String, val f: String) {
	var m: String = null	/* name when character is male */
	var f: String = null	/* when female; null if same as male */
}

class RoleAdvance {
	/* "fix" is the fixed amount, "rnd" is the random amount */
	var infix, inrnd: String = null	/* at character initialization */
	var lofix, lornd: String = null	/* gained per level <  urole.xlev */
	var hifix, hirnd: String = null	/* gained per level >= urole.xlev */
}

class u_have {
	var amulet = false	/* carrying Amulet	*/
	var bell = false	/* carrying Bell	*/
	var book = false	/* carrying Book	*/
	var menorah = false	/* carrying Candelabrum */
	var questart = false	/* carrying the Quest Artifact */
	var unused: Byte = 0
}

class u_event {
	var minor_oracle = false	/* received at least 1 cheap oracle */
	var major_oracle = false	/*  "  expensive oracle */
	var qcalled = false		/* called by Quest leader to do task */
	var qexpelled = false		/* expelled from the Quest dungeon */
	var qcompleted = false		/* successfully completed Quest task */
	var uheard_tune: Byte = 0	/* 1=know about, 2=heard passtune */
	var uopened_dbridge = false	/* opened the drawbridge */

	var invoked = false		/* invoked Gate to the Sanctum level */
	var gehennom_entered = false	/* entered Gehennom via Valley */
	var uhand_of_elbereth: Byte = 0	/* became Hand of Elbereth */
	var udemigod = false		/* killed the wiz */
	var ascended = false		/* has offered the Amulet */
}

/* KMH, conduct --
 * These are voluntary challenges.  Each field denotes the number of
 * times a challenge has been violated.
 */
class u_conduct {		/* number of times... */
	var unvegetarian = 0		/* eaten any animal */
	var unvegan = 0			/* ... or any animal byproduct */
	var food = 0			/* ... or any comestible */
	var gnostic = 0			/* used prayer, priest, or altar */
	var weaphit = 0			/* hit a monster with a weapon */
	var killer = 0			/* killed a monster yourself */
	var literate = 0		/* read something (other than BotD) */
	var polypiles = 0		/* polymorphed an object */
	var polyselfs = 0		/* transformed yourself */
	var wishes = 0			/* used a wish */
	var wisharti = 0		/* wished for an artifact */
	var wishmagic = 0		/* wished for a magic object */
	var armoruses = 0		/* put on a piece of armor */
	var unblinded = 0		/* starting non-blindfolded and removing a blindfold */
	var robbed = 0			/* killed an artifact-guardian (like an ordinary robber) */
	var elbereths = 0		/* engraved Elbereth */
	var bones = 0			/* encountered bones level */
	var non_racial_armor = 0	/* put on non-racial armor */
	var non_racial_weapons = 0	/* wielded non-racial weapons */
	var sokoban = 0			/* how many times violated sokoban "rules" */
	var heaven_or_hell = 0		/* heaven or hell mode */
	var hell_and_hell = 0		/* hell and hell mode */
					/* genocides already listed at end of game */
}

def heaven_or_hell_mode = u.uconduct.heaven_or_hell
def hell_and_hell_mode = u.uconduct.hell_and_hell

/*             --- roleplay intrinsics --- 
 *
 * In a lot of situations it makes sense to make special cases for
 * conduct-characters. Here's the structure to store a character's
 * abilities/intentions.
 *
 * These are allowed to change during gameplay. So e.g. it's possible
 * to gain/lose the ability to read.
 *
 * It also allows to stop giving special treatment to characters which
 * obviously abandoned a selected conduct. E.g vegetarians stop feeling
 * guilty after eating a lot of corpses.
*/
class u_roleplay {		/* Your character is a/an ... */
	var ascet = false	/* foodless character */
	var atheist = false	/* atheist */
	var blindfolded = false	/* blindfolded character */
	var illiterate = false	/* illiterate character */
	var pacifist = false	/* pacifist */
	var sadist = false	/* sadist */
	var nudist = false	/* nudist */
	var vegan = false	/* vegan */
	var vegetarian = false	/* vegetarian */
}

/*** Unified structure containing role information ***/
class Role {
	/*** Strings that name various things ***/
	val name = new RoleName()	/* the role's name (from u_init.c) */
	val rank = new Array[RoleName](9) /* names for experience levels (from botl.c) */
	var lgod, ngod, cgod: String = null /* god names (from pray.c) */
	var filecode: String = null	/* abbreviation for use in file names */
	var homebase: String = null	/* quest leader's location (from questpgr.c) */
	var intermed: String = null	/* quest intermediate goal (from questpgr.c) */

	/*** Indices of important monsters and objects ***/
	var malenum,			/* index (PM_) as a male (botl.c) */
	      femalenum,		/* ...or as a female (NON_PM == same) */
	      petnum,			/* PM_ of preferred pet (NON_PM == random) */
	      ldrnum,			/* PM_ of quest leader (questpgr.c) */
	      guardnum,			/* PM_ of quest guardians (questpgr.c) */
	      neminum: Short = 0	/* PM_ of quest nemesis (questpgr.c) */
	var questarti: Short = 0	/* index (ART_) of quest artifact (questpgr.c) */

	/*** Bitmasks ***/
	var allow: Short = 0		/* bit mask of allowed variations */
val ROLE_RACEMASK = 0x0ff8		/* allowable races */
val ROLE_GENDMASK = 0xf000		/* allowable genders */
val ROLE_MALE = 0x1000
val ROLE_FEMALE = 0x2000
val ROLE_NEUTER = 0x4000
val ROLE_ALIGNMASK = AM_MASK		/* allowable alignments */
val ROLE_LAWFUL = AM_LAWFUL
val ROLE_NEUTRAL = AM_NEUTRAL
val ROLE_CHAOTIC = AM_CHAOTIC

	/*** Attributes (from attrib.c and exper.c) ***/
	var attrbase = new Array[xchar](A_MAX)	/* lowest initial attributes */
	var attrdist = new Array[xchar](A_MAX)	/* distribution of initial attributes */
	val hpadv = new RoleAdvance() /* hit point advancement */
	val enadv = new RoleAdvance() /* energy advancement */
	var xlev: xchar = 0		/* cutoff experience level */
	var initrecord: xchar = 0	/* initial alignment record */

	/*** Spell statistics (from spell.c) ***/
	var spelbase = 0		/* base spellcasting penalty */
	var spelheal = 0		/* penalty (-bonus) for healing spells */
	var spelshld = 0		/* penalty for wearing any shield */
	var spelarmr = 0		/* penalty for wearing metal armour */
	var spelstat = 0		/* which stat (A_) is used */
	var spelspec = 0		/* spell (SPE_) the class excels at */
	var spelsbon = 0		/* penalty (-bonus) for that spell */

	/*** Properties in variable-length arrays ***/
	/* intrinsics (see attrib.c) */
	/* initial inventory (see u_init.c) */
	/* skills (see u_init.c) */

	/*** Don't forget to add... ***/
	/* quest leader, guardians, nemesis (monst.c) */
	/* quest artifact (artilist.h) */
	/* quest dungeon definition (dat/Xyz.dat) */
	/* quest text (dat/quest.txt) */
	/* dictionary entries (dat/data.bas) */
}

def Role_if(X: Short) = (urole.malenum == (X))
def Role_switch	= (urole.malenum)

/* used during initialization for race, gender, and alignment
   as well as for character class */
val ROLE_NONE = -1
val ROLE_RANDOM = -2

/*** Unified structure specifying race information ***/

class Race {
	/*** Strings that name various things ***/
	var noun: String = null	/* noun ("human", "elf") */
	var adj: String = null	/* adjective ("human", "elven") */
	var coll: String = null	/* collective ("humanity", "elvenkind") */
	var filecode: String = null	/* code for filenames */
	val individual = new RoleName() /* individual as a noun ("man", "elf") */

	/*** Indices of important monsters and objects ***/
	var malenum,			/* PM_ as a male monster */
	      femalenum,		/* ...or as a female (NON_PM == same) */
	      mummynum,			/* PM_ as a mummy */
	      zombienum: Short =0	/* PM_ as a zombie */

	/*** Bitmasks ***/
	var allow: Short = 0		/* bit mask of allowed variations */
	var selfmask,			/* your own race's bit mask */
	      lovemask,			/* bit mask of always peaceful */
	      hatemask: Short = 0	/* bit mask of always hostile */

	/*** Attributes ***/
	val attrmin = new Array[xchar](A_MAX)	/* minimum allowable attribute */
	val attrmax = new Array[xchar](A_MAX)	/* maximum allowable attribute */
	val hpadv = new RoleAdvance() /* hit point advancement */
	val enadv = new RoleAdvance() /* energy advancement */

	/*** Properties in variable-length arrays ***/
	/* intrinsics (see attrib.c) */

	/*** Don't forget to add... ***/
	/* quest leader, guardians, nemesis (monst.c) */
	/* quest dungeon definition (dat/Xyz.dat) */
	/* quest text (dat/quest.txt) */
	/* dictionary entries (dat/data.bas) */
}

def Race_if(X: Short) = (urace.malenum == (X))
def Race_switch = (urace.malenum)

/*** Unified structure specifying gender information ***/
class Gender {
	var adj: String = null	/* male/female/neuter */
	var he: String = null		/* he/she/it */
	var him: String = null	/* him/her/it */
	var his: String = null	/* his/her/its */
	var filecode: String = null	/* file code */
	var allow: Short = 0		/* equivalent ROLE_ mask */
}
val ROLE_GENDERS = 2	/* number of permitted player genders */
				/* increment to 3 if you allow neuter roles */

def uhe()	= (genders(if(flags.female) 1 else 0).he)
def uhim()	= (genders(if(flags.female) 1 else 0).him)
def uhis()	= (genders(if(flags.female) 1 else 0).his)
def mhe(mtmp: monst)	= (genders(pronoun_gender(mtmp)).he)
def mhim(mtmp: monst)	= (genders(pronoun_gender(mtmp)).him)
def mhis(mtmp: monst)	= (genders(pronoun_gender(mtmp)).his)

/*** Unified structure specifying alignment information ***/
class Align {
	var noun: String = null	/* law/balance/chaos */
	var adj: String = null	/* lawful/neutral/chaotic */
	var filecode: String = null	/* file code */
	var allow: Short = 0		/* equivalent ROLE_ mask */
	var value = A_NONE		/* equivalent A_ value */
}
val ROLE_ALIGNS = 3	/* number of permitted player alignments */

/*
 * The following structure contains information about a conduct
 *   - a name (e.g. for a conduct-menu at character creation)
 *   - nouns and adjectives for the highscore
 *   - a flag for 'worth mentioning in the highscore'
 *   - all strings necessary for show_conduct()
 */
/*** Unified structure specifying conduct information ***/
class Conduct {
	const char *name;		/* pacifism/nudism/...	*/
	const char *noun;		/* pacifist/nudist/...	*/
	const char *adj;		/* peaceful/nude/...	*/

	boolean highscore;		/* conduct appears in the highscore */

	const char *prefix;		/* "You "	*/
	const char *presenttxt;		/* "have been "	*/
	const char *pasttxt;		/* "were "	*/
	const char *suffix;		/* "a pacifist"	*/
	const char *failtxt;		/* "pretended to be a pacifist" */

}

/*
 * The following definitions get used to call violated(CONDUCT_XYZ),
 * and to access the information contained in the conducts[] array.
*/

val ROLE_CONDUCTS = 10 	/* number of roleplay conducts */

val FIRST_CONDUCT = 0
val CONDUCT_PACIFISM = 0
val CONDUCT_SADISM = 1
val CONDUCT_ATHEISM = 2
val CONDUCT_NUDISM = 3
val CONDUCT_BLINDFOLDED = 4
val CONDUCT_FOODLESS = 5
val CONDUCT_VEGAN = 6
val CONDUCT_VEGETARIAN = 7
val CONDUCT_ILLITERACY = 8
val CONDUCT_THIEVERY = 9
val LAST_CONDUCT = ROLE_CONDUCTS - 1

/*** Information about the player ***/
class you {
	var ux, uy: xchar = 0
	var dx, dy, dz: schar = 0	/* direction of move (or zap or ... ) */
	var di: schar = 0		/* direction of FF */
	var tx, ty: xchar = 0		/* destination of travel */
	var ux0, uy0: xchar = 0		/* initial position FF */
	var uz = new d_level(); var uz0 = new d_level()	/* your level on this and the previous turn */
	var utolev = new d_level()	/* level monster teleported you to, or uz */
	var utotype: uchar = 0		/* bitmask of goto_level() flags for utolev */
	var umoved = false		/* changed map location (post-move) */
	var last_str_turn = 0	/* 0: none, 1: half turn, 2: full turn */
				/* +: turn right, -: turn left */
	var ulevel = 0		/* 1 to MAXULEV */
	var ulevelmax = 0
	var utrap = 0		/* trap timeout */
	var utraptype = 0	/* defined if utrap nonzero */
	var ufeetfrozen = 0	/* feet frozen, works similarly to utrap */
val TT_BEARTRAP = 0
val TT_PIT = 1
val TT_WEB = 2
val TT_LAVA = 3
val TT_INFLOOR = 4
val TT_SWAMP = 5
val TT_ICE = 6
	val urooms = new Array[uchar](5)	/* rooms (roomno + 3) occupied now */
	val urooms0 = new Array[uchar](5)	/* ditto, for previous position */
	val uentered = new Array[uchar](5)	/* rooms (roomno + 3) entered this turn */
	val ushops = new Array[uchar](5)	/* shop rooms (roomno + 3) occupied now */
	val ushops0 = new Array[uchar](5)	/* ditto, for previous position */
	val ushops_entered = new Array[uchar](5) /* ditto, shops entered this turn */
	val ushops_left = new Array[uchar](5) /* ditto, shops exited this turn */

	var uhunger = 0		/* refd only in eat.c and shk.c */
	var uhs = 0		/* hunger state - see eat.c */

	val uprops = new Array[prop](LAST_PROP+1)

	var umconf = 0
	var usick_cause: String = null
	var usick_type: Byte = 0
val SICK_VOMITABLE = 0x01
val SICK_NONVOMITABLE = 0x02
val SICK_ALL = 0x03

	/* These ranges can never be more than MAX_RANGE (vision.h). */
	var nv_range = 0		/* current night vision range */
	var xray_range = 0		/* current xray vision range */

	/*
	 * These variables are valid globally only when punished and blind.
	 */
val BC_BALL = 0x01	/* bit mask for ball  in 'bc_felt' below */
val BC_CHAIN = 0x02	/* bit mask for chain in 'bc_felt' below */
	var bglyph = 0	/* glyph under the ball */
	var cglyph = 0	/* glyph under the chain */
	var bc_order = 0	/* ball & chain order [see bc_order() in ball.c] */
	var bc_felt = 0	/* mask for ball/chain being felt */

	var umonster = 0			/* hero's "real" monster num */
	var umonnum = 0			/* current monster number */

	var mh, mhmax, mtimedone = 0	/* for polymorph-self */
	val macurr = new attribs()	/* for monster attribs */
	val mamax = new attribs()	/* for monster attribs */
	val ulycn = 0			/* lycanthrope type */

	var ucreamed = 0
	var uswldtim = 0		/* time you have been swallowed */

	var uswallow = false		/* true if swallowed */
	var uinwater = false		/* if you're currently in water (only
					   underwater possible currently) */
	var uundetected = false	/* if you're a hiding monster/piercer */
	var mfemale = false		/* saved human value of flags.female */
	var uinvulnerable = false	/* you're invulnerable (praying) */
	var uburied = false		/* you're buried */
	var uedibility = false		/* blessed food detection; sense unsafe food */
	/* 1 free bit! */

	var udg_cnt = 0			/* how long you have been demigod */
	val uevent = new u_event()		/* certain events have happened */
	val uhave = new u_have()		/* you're carrying special objects */
	val uconduct = new u_conduct()	/* KMH, conduct */
	val roleplay = new u_roleplay()	/* roleplay intrinsics */
	var ulives = 0			/* heaven or hell mode, number of lives */
	val acurr = new attribs()		/* your current attributes (eg. str)*/
	val aexe = new attribs()		/* for gain/loss via "exercise" */
	val abon = new attribs()		/* your bonus attributes (eg. str) */
	val amax = new attribs()		/* your max attributes (eg. str) */
	val atemp = new attribs()		/* used for temporary loss/gain */
	val atime = new attribs()		/* used for loss/gain countdown */
	val ualign = new align()		/* character alignment */
val CONVERT = 2
val A_ORIGINAL = 1
val A_CURRENT = 0
	val ualignbase = new Array[aligntyp](CONVERT)	/* for ualign conversion record */
	var uluck, moreluck: schar = 0		/* luck and luck bonus */
	var luckturn = 0
def Luck = (u.uluck + u.moreluck)
val LUCKADD = 3	/* added value when carrying luck stone */
val LUCKMAX = 10	/* on moonlit nights 11 */
val LUCKMIN = -10
	var uhitinc: schar = 0
	var udaminc: schar = 0
	var uac: schar = 0
	var uspellprot: uchar = 0	/* protection by SPE_PROTECTION */
	var usptime: uchar = 0		/* #moves until uspellprot-- */
	var uspmtime: uchar = 0		/* #moves between uspellprot-- */
	var p,uhpmax = 0
	var n, uenmax = 0		/* magical energy - M. Stephenson */
	var angr = 0			/* if the gods are angry at you */
	var ifts = 0			/* number of artifacts bestowed */
	var lessed, ublesscnt = 0	/* blessing/duration from #pray */
	var gold, ugold0 = 0
	var exp, urexp = 0
	var rscore = 0			/**< the current score */
	var cleansed = 0		/* to record moves when player was cleansed */
	var sleep = 0			/* sleeping; monstermove you last started */
	var nvault = 0
	var ustuck: monst = null
	var usteed: monst = null
	var gallop = 0
	var ideturns = 0
	var ortality = 0		/* how many times you died */
	var rave_arise = 0 		/* you die and become something aside from a ghost */
	var ubirthday: time_t = 0	/* real world time when game began */
	var udeathday: time_t = 0	/* real world time when game ended */

	var weapon_slots = 0		/* unused skill slots */
	var skills_advanced = 0		/* # of advances made so far */
	val skill_record = new Array[xchar](P_SKILL_LIMIT)	/* skill advancements */
	val weapon_skills = new Array[skills](P_NUM_SKILLS)
	var twoweap = false		/* KMH -- Using two-weapon combat */
	var incloud = false		/* used for blindness in stinking clouds */
}	/* end of `struct you' */

def Upolyd: Boolean = (u.umonnum != u.umonster)
}
