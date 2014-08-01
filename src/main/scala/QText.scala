/* Copyright (c) Mike Stephenson 1991.				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object QText { 

val N_HDR = 16		/* Maximum number of categories */
				/* (i.e., num roles + 1) */
val LEN_HDR = 3		/* Maximum length of a category name */

class qtmsg {
	var msgnum = 0
	var delivery: Char = 0
	var offset,
		size = 0
}

val N_MSG = 500		/* arbitrary */

class msghdr {
	var n_msg = 0
	var qt_msg = new Array[qtmsg](N_MSG)
}

class qthdr {
	var n_hdr = 0
	var id: Array[Array[Char]] = Array.ofDim(N_HDR, LEN_HDR)
	var offset = new Array[Int](N_HDR)
};

/* Error message macros */
val CREC_IN_MSG = "Control record encountered during message - line %d\n"
val DUP_MSG = "Duplicate message number at line %d\n"
val END_NOT_IN_MSG = "End record encountered before message - line %d\n"
val TEXT_NOT_IN_MSG = "Text encountered outside message - line %d\n"
val UNREC_CREC = "Unrecognized Control record at line %d\n"
val OUT_OF_HEADERS = "Too many message types (line %d)\nAdjust N_HDR in qtext.h and recompile.\n"
val OUT_OF_MESSAGES = "Too many messages in class (line %d)\nAdjust N_MSG in qtext.h and recompile.\n"


class qtlists {
	var common: qtmsg = null
	var chrole: qtmsg = null
}


/*
 *	Quest message defines.	Used in quest.c to trigger off "realistic"
 *	dialogue to the player.
 */
val QT_FIRSTTIME = 1
val QT_NEXTTIME = 2
val QT_OTHERTIME = 3

val QT_GUARDTALK = 5	/* 5 random things guards say before quest */
val QT_GUARDTALK2 = 10	/* 5 random things guards say after quest */

val QT_FIRSTLEADER = 15
val QT_NEXTLEADER = 16
val QT_OTHERLEADER = 17
val QT_LASTLEADER = 18
val QT_BADLEVEL = 19
val QT_BADALIGN = 20
val QT_ASSIGNQUEST = 21

val QT_ENCOURAGE = 25	/* 1-10 random encouragement messages */

val QT_FIRSTLOCATE = 35
val QT_NEXTLOCATE = 36

val QT_FIRSTGOAL = 40
val QT_NEXTGOAL = 41

val QT_FIRSTNEMESIS = 50
val QT_NEXTNEMESIS = 51
val QT_OTHERNEMESIS = 52
val QT_NEMWANTSIT = 53	/* you somehow got the artifact */

val QT_DISCOURAGE = 60	/* 1-10 random maledictive messages */

val QT_GOTIT = 70

val QT_KILLEDNEM = 80
val QT_OFFEREDIT = 81
val QT_OFFEREDIT2 = 82

val QT_POSTHANKS = 90
val QT_HASAMULET = 91

/*
 *	Message defines for common text used in maledictions.
 */
val COMMON_ID = "-"	/* Common message id value */

val QT_ANGELIC = 10
val QTN_ANGELIC = 10

val QT_DEMONIC = 30
val QTN_DEMONIC = 20

val QT_BANISHED = 60

/*
 *  Tutorial text common to all classes (displayed with check_tutorial_message).
 */

val QT_T_FIRST = 201

/* Introductory messages */
val QT_T_WELCOME = 201 /* at start of game */
val QT_T_CURSOR_NUMPAD = 202 /* upon pressing ; with numpad */
val QT_T_CURSOR_VIKEYS = 203 /* upon pressing ; with vikeys */
/* Looking at monsters */
val QT_T_LOOK_TAME = 210 /* farlooking a pet */
val QT_T_LOOK_HOSTILE = 211 /* farlooking a hostile */
val QT_T_LOOK_PEACEFUL = 212 /* farlooking a peaceful */
val QT_T_LOOK_INVISIBLE = 213 /* adjacent to or farlooked I */
/* Terrain messages; display when seen/adjacent, or farlooked */
val QT_T_DOORS = 230 /* adjacent to or farlooked a door */
val QT_T_CORRIDOR = 231 /* adjacent to or farlooked a corridor */
val QT_T_SECRETDOOR = 232 /* adjacent to a secret door (freebie) */
val QT_T_POOLORMOAT = 233 /* adjacent to or farlooked water */
val QT_T_LAVA = 234 /* adjacent to or farlooked lava */
val QT_T_STAIRS = 235 /* adjacent to or farlooked /downstairs/ */
val QT_T_FOUNTAIN = 236 /* adjacent to or farlooked fountain */
val QT_T_THRONE = 237 /* adjacent to or farlooked throne */
val QT_T_SINK = 238 /* adjacent to or farlooked sink */
val QT_T_GRAVE = 239 /* adjacent to or farlooked grave */
val QT_T_ALTAR = 240 /* adjacent to or farlooked altar */
val QT_T_DRAWBRIDGE = 241 /* adjacent to or farlooked drawbridge */
val QT_T_TRAP = 242 /* adjacent to or farlooked trap */
val QT_T_L1UPSTAIRS = 259 /* farlooked level 1 upstairs */
/* Interface hints and reminders */
val QT_T_FARMOVE_VIKEYS = 260 /* direction key five times in a row */
val QT_T_FARMOVE_NUMPAD = 261 /* ditto */
val QT_T_TRAVEL = 262 /* thirty direction keys in a row */
val QT_T_DIAGONALS_VI = 263 /* twenty orthogonals in a row */
val QT_T_DIAGONALS_NUM = 264 /* twenty orthogonals in a row */
val QT_T_REPEAT_VIKEYS = 265 /* s 5 times in a row */
val QT_T_REPEAT_NUMPAD = 266 /* s 5 times in a row */
val QT_T_CHOOSEITEM = 267 /* explaining [a-zA-Z?*] menus */
val QT_T_MASSUNEQUIP = 268 /* R or T twice in a row */
val QT_T_CALLMONSTER = 269 /* adjacent to two identical monsters */
val QT_T_MULTIDROP = 270 /* d twice in a row */
val QT_T_MASSINVENTORY = 271 /* I twice in a row */
val QT_T_SECONDWIELD = 272 /* w more often than twice per 50 turns */
val QT_T_LOOK_REMINDER = 273 /* no ; within the last 100 turns */
val QT_T_FIRE = 274 /* t three times in a row */
val QT_T_DGN_OVERVIEW = 275 /* dungeon #overview and #annotate */
/* Ambient advice, outside combat */
val QT_T_VIEWTUTORIAL = 280 /* peaceful, turn >= 10 */
val QT_T_CHECK_ITEMS = 281 /* peaceful, turn >= 30 */
val QT_T_OBJECTIVE = 282 /* peaceful, turn >= 60 */
val QT_T_SAVELOAD = 283 /* peaceful, turn >= 100 */
val QT_T_MESSAGERECALL = 284 /* peaceful, turn >= 150 */
/* Ambient advice, during combat; only one advice given per combat */
val QT_T_SPELLS = 290 /* knows 'force bolt' */
val QT_T_THROWNWEAPONS = 291 /* thrown weapon in invent */
val QT_T_PROJECTILES = 292 /* ammo in invent */
val QT_T_ELBERETH = 293 /* combat on dlevel 3+ */
val QT_T_MELEE = 294 /* a general combat ambient */
/* Item types; these appear when the item's picked up, farlooked, or
   the relevant inventory section is viewed */
val QT_T_ITEM_GOLD = 300
val QT_T_ITEM_WEAPON = 301
val QT_T_ITEM_ARMOR = 302
val QT_T_ITEM_FOOD = 303
val QT_T_ITEM_SCROLL = 304
val QT_T_ITEM_WAND = 305
val QT_T_ITEM_RING = 306
val QT_T_ITEM_POTION = 307
val QT_T_ITEM_TOOL = 308
val QT_T_ITEM_CONTAINER = 309 /* on pickup, or standover for chests */
val QT_T_ITEM_AMULET = 310
val QT_T_ITEM_GEM = 311
val QT_T_ITEM_STATUE = 312
val QT_T_ITEM_BOOK = 313
val QT_T_CALLITEM = 317 /* call-item prompt */
val QT_T_ARTIFACT = 318 /* obtained an artifact */
val QT_T_RANDAPPEARANCE = 319 /* obtained a rand-appearance item */
/* Events */
val QT_T_LEVELUP = 320 /* became xlevel 2 */
val QT_T_RANKUP = 321 /* became xlevel 3 */
val QT_T_ABILUP = 322 /* str, con, dex, int, wis, or cha increased */
val QT_T_ABILDOWN = 323 /* str, con, dex, int, wis, or cha decreased */
val QT_T_DLEVELCHANGE = 324 /* entered dlvl 2+ */
val QT_T_DAMAGED = 325 /* not at full hp */
val QT_T_PWUSED = 326 /* not at full pw */
val QT_T_PWEMPTY = 327 /* pw too low to cast a spell */
val QT_T_ACIMPROVED = 328 /* AC better than starting value */
val QT_T_GAINEDEXP = 329 /* not at 0 exp */
val QT_T_HUNGER = 330 /* hungry or worse */
val QT_T_SATIATION = 331 /* satiated */
val QT_T_STATUS = 332 /* other status */
val QT_T_ENHANCE = 333 /* can train a skill */
val QT_T_POLYSELF = 334 /* turned into monster form */
val QT_T_ENGRAVING = 335 /* stepped on an engraving */
val QT_T_MAJORTROUBLE = 336 /* in major trouble and #pray will work */
val QT_T_BURDEN = 337 /* became burdened */
val QT_T_EQUIPCURSE = 338 /* BCU-known item is cursed */
val QT_T_MAILSCROLL = 339 /* mail was delivered */
val QT_T_CASTER_ARMOR = 340 /* spellcaster wearing inappropriate armour */
val QT_T_WEAPON_SKILL = 341 /* wielding an unskilled weapon */
val QT_T_ENGULFED = 342 /* engulfed by a monster */
val QT_T_DEATH = 359 /* died; contains x-mode hint */
/* Dungeon areas */
val QT_T_SHOPENTRY = 360 /* entered a shop */
val QT_T_SHOPBUY = 361 /* picked up an item in a shop */
val QT_T_MINES = 362 /* entered the Mines */
val QT_T_SOKOBAN = 363 /* entered Sokoban */
val QT_T_ORACLE = 364 /* upon "welcome to Delphi" message */
/* Leave some room before this to avoid breaking savefile compatibility
   if the list is ever expanded. No tutorial message can have a number
   higher than this. */
val QT_T_MAX = 400
/* Guidebook sections not mentioned in the tutorial: conducts,
   options, riding, autopickup-exceptions, regexp sounds, play by the
   blind, scoring, credits, flavour introduction (although 'legacy'
   handles that in tutorial style), class choice (so far) */
}
