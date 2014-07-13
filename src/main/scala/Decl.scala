/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Decl {

val WARNCOUNT = 6			/* number of different warning levels */

class dgn_topology {		/* special dungeon levels for speed */
    var	d_oracle_level = new d_level()
    var	d_bigroom_level = new d_level()	/* unused */
    var	d_rogue_level = new d_level()
    var	d_medusa_level = new d_level()
    var	d_stronghold_level = new d_level()
    var	d_valley_level = new d_level()
    var	d_wiz1_level = new d_level()
    var	d_wiz2_level = new d_level()
    var	d_wiz3_level = new d_level()
    var	d_juiblex_level = new d_level()
    var	d_orcus_level = new d_level()
    var	d_baalzebub_level = new d_level()	/* unused */
    var	d_asmodeus_level = new d_level()	/* unused */
    var	d_portal_level = new d_level()		/* only in goto_level() [do.c] */
    var	d_sanctum_level = new d_level()
    var	d_earth_level = new d_level()
    var	d_water_level = new d_level()
    var	d_fire_level = new d_level()
    var	d_air_level = new d_level()
    var	d_astral_level = new d_level()
    var	d_tower_dnum: xchar = 0
    var	d_sokoban_dnum: xchar = 0
    var	d_mines_dnum, d_quest_dnum: xchar = 0
    var	d_mall_dnum: xchar = 0
    var	d_sheol_dnum: xchar = 0
    var	d_qstart_level = new d_level()
    var d_qlocate_level = new d_level()
    var d_nemesis_level = new d_level()
    var	d_knox_level = new d_level()
    var	d_nymph_level = new d_level()
    var	d_mineend_level = new d_level()
    var	d_sokoend_level = new d_level()
    var	d_minetown_level = new d_level()
    var	d_town_level = new d_level()
    var	d_moria_level = new d_level()
}

/* macros for accesing the dungeon levels by their old names */
def oracle_level		 = dungeon_topology.d_oracle_level
def bigroom_level		 = dungeon_topology.d_bigroom_level
def rogue_level		 = dungeon_topology.d_rogue_level
def medusa_level		 = dungeon_topology.d_medusa_level
def stronghold_level	 = dungeon_topology.d_stronghold_level
def valley_level		 = dungeon_topology.d_valley_level
def wiz1_level		 = dungeon_topology.d_wiz1_level
def wiz2_level		 = dungeon_topology.d_wiz2_level
def wiz3_level		 = dungeon_topology.d_wiz3_level
def juiblex_level		 = dungeon_topology.d_juiblex_level
def orcus_level		 = dungeon_topology.d_orcus_level
def baalzebub_level		 = dungeon_topology.d_baalzebub_level
def asmodeus_level		 = dungeon_topology.d_asmodeus_level
def portal_level		 = dungeon_topology.d_portal_level
def sanctum_level		 = dungeon_topology.d_sanctum_level
def earth_level		 = dungeon_topology.d_earth_level
def water_level		 = dungeon_topology.d_water_level
def fire_level		 = dungeon_topology.d_fire_level
def air_level		 = dungeon_topology.d_air_level
def astral_level		 = dungeon_topology.d_astral_level
def tower_dnum		 = dungeon_topology.d_tower_dnum
def sokoban_dnum		 = dungeon_topology.d_sokoban_dnum
def mines_dnum		 = dungeon_topology.d_mines_dnum
def quest_dnum		 = dungeon_topology.d_quest_dnum
def mall_dnum		 = dungeon_topology.d_mall_dnum
def sheol_dnum		 = dungeon_topology.d_sheol_dnum
def qstart_level		 = dungeon_topology.d_qstart_level
def qlocate_level		 = dungeon_topology.d_qlocate_level
def nemesis_level		 = dungeon_topology.d_nemesis_level
def knox_level		 = dungeon_topology.d_knox_level
def nymph_level		 = dungeon_topology.d_nymph_level
def mineend_level            = dungeon_topology.d_mineend_level
def sokoend_level            = dungeon_topology.d_sokoend_level
def minetown_level		 = dungeon_topology.d_minetown_level
def town_level		 = dungeon_topology.d_town_level
def moria_level		 = dungeon_topology.d_moria_level

def xdnstair	 = dnstair.sx
def ydnstair	 = dnstair.sy
def xupstair	 = upstair.sx
def yupstair	 = upstair.sy

def xdnladder	 = dnladder.sx
def ydnladder	 = dnladder.sy
def xupladder	 = upladder.sx
def yupladder	 = upladder.sy

def dunlev_reached(x: d_level) = dungeons(x.dnum).dunlev_ureached

class sinfo {
	var gameover = 0	/* self explanatory? */
	var stopprint = 0	/* inhibit further end of game disclosure */
	var done_hup = 0	/* SIGHUP or moral equivalent received
				 * -- no more screen output */
	var something_worth_saving = 0	/* in case of panic */
	var panicking = 0	/* `panic' is in progress */
	var exiting = 0		/* an exit handler is executing */
	var in_impossible = 0
	var in_paniclog = 0
}

val KILLED_BY_AN = 0
val KILLED_BY = 1
val NO_KILLER_PREFIX = 2
class digging {		/* apply.c, hack.c */
	var effort = 0
	var level = new d_level()
	var pos = new coord()
	var lastdigtime = 0
	var down = false
	var chew = false
	var warned = false
	var quiet = false
} 

val PM_BABY_GRAY_DRAGON = PM_BABY_TATZELWORM
val PM_BABY_SILVER_DRAGON = PM_BABY_AMPHITERE
val PM_BABY_RED_DRAGON = PM_BABY_DRAKEN
val PM_BABY_WHITE_DRAGON = PM_BABY_LINDWORM
val PM_BABY_ORANGE_DRAGON = PM_BABY_SARKANY
val PM_BABY_BLACK_DRAGON = PM_BABY_SIRRUSH
val PM_BABY_BLUE_DRAGON = PM_BABY_LEVIATHAN
val PM_BABY_GREEN_DRAGON = PM_BABY_WYVERN
/*#define PM_BABY_GLOWING_DRAGON   PM_BABY_GLOWING_DRAGON*/
val PM_BABY_YELLOW_DRAGON = PM_BABY_GUIVRE
val PM_GRAY_DRAGON = PM_TATZELWORM
val PM_SILVER_DRAGON = PM_AMPHITERE
val PM_RED_DRAGON = PM_DRAKEN
val PM_WHITE_DRAGON = PM_LINDWORM
val PM_ORANGE_DRAGON = PM_SARKANY
val PM_BLACK_DRAGON = PM_SIRRUSH
val PM_BLUE_DRAGON = PM_LEVIATHAN
val PM_GREEN_DRAGON = PM_WYVERN
/*#define PM_GLOWING_DRAGON   PM_GLOWING_DRAGON*/
val PM_YELLOW_DRAGON = PM_GUIVRE

val GRAY_DRAGON_SCALE_MAIL = MAGIC_DRAGON_SCALE_MAIL
val SILVER_DRAGON_SCALE_MAIL = REFLECTING_DRAGON_SCALE_MAIL
val RED_DRAGON_SCALE_MAIL = FIRE_DRAGON_SCALE_MAIL
val WHITE_DRAGON_SCALE_MAIL = ICE_DRAGON_SCALE_MAIL
val ORANGE_DRAGON_SCALE_MAIL = SLEEP_DRAGON_SCALE_MAIL
val BLACK_DRAGON_SCALE_MAIL = DISINTEGRATION_DRAGON_SCALE_MA
val BLUE_DRAGON_SCALE_MAIL = ELECTRIC_DRAGON_SCALE_MAIL
val GREEN_DRAGON_SCALE_MAIL = POISON_DRAGON_SCALE_MAIL
val GLOWING_DRAGON_SCALE_MAIL = STONE_DRAGON_SCALE_MAIL
val YELLOW_DRAGON_SCALE_MAIL = ACID_DRAGON_SCALE_MAIL

val GRAY_DRAGON_SCALES = MAGIC_DRAGON_SCALES
val SILVER_DRAGON_SCALES = REFLECTING_DRAGON_SCALES
val RED_DRAGON_SCALES = FIRE_DRAGON_SCALES
val WHITE_DRAGON_SCALES = ICE_DRAGON_SCALES
val ORANGE_DRAGON_SCALES = SLEEP_DRAGON_SCALES
val BLACK_DRAGON_SCALES = DISINTEGRATION_DRAGON_SCALES
val BLUE_DRAGON_SCALES = ELECTRIC_DRAGON_SCALES
val GREEN_DRAGON_SCALES = POISON_DRAGON_SCALES
val GLOWING_DRAGON_SCALES = STONE_DRAGON_SCALES
val YELLOW_DRAGON_SCALES = ACID_DRAGON_SCALES


class mvitals {
	var born: uchar = 0
	var died: uchar = 0
	var mvflags: uchar = 0
} 

object c_color_names {
	val c_black = "black";  val c_amber = "amber";  val c_golden = "golden"; 
	val c_light_blue = "light blue"; val c_red = "red";  val c_orange = "orange"; 
	val c_green = "green";  val c_silver = "silver";  val c_blue = "blue"; 
	val c_purple = "purple";  val c_white = "white"
}
def NH_BLACK		 = c_color_names.c_black
def NH_AMBER		 = c_color_names.c_amber
def NH_GOLDEN		 = c_color_names.c_golden
def NH_LIGHT_BLUE	 = c_color_names.c_light_blue
def NH_RED		 = c_color_names.c_red
def NH_ORANGE		 = c_color_names.c_orange
def NH_GREEN		 = c_color_names.c_green
def NH_SILVER		 = c_color_names.c_silver
def NH_BLUE		 = c_color_names.c_blue
def NH_PURPLE		 = c_color_names.c_purple
def NH_WHITE		 = c_color_names.c_white

/* The names of the colors used for gems, etc. */
object c_common_strings {
	val c_nothing_happens = "Nothing happens."; val c_thats_enough_tries = "That's enough tries!"
	val c_silly_thing_to = "That is a silly thing to do %s."; var c_shudder_for_moment = "shudder for a moment."
	val c_something = "something"; var c_Something = "Something"
	val c_You_can_move_again = "You can move again"
	val c_Never_mind = "Never mind."; var c_vision_clears = "vision quickly clears."
	val c_the_your = List("the", "your")
}
def nothing_happens     = c_common_strings.c_nothing_happens
def thats_enough_tries  = c_common_strings.c_thats_enough_tries
def silly_thing_to	    = c_common_strings.c_silly_thing_to
def shudder_for_moment  = c_common_strings.c_shudder_for_moment
def something	    = c_common_strings.c_something
def Something	    = c_common_strings.c_Something
def You_can_move_again  = c_common_strings.c_You_can_move_again
def Never_mind	    = c_common_strings.c_Never_mind
def vision_clears	    = c_common_strings.c_vision_clears
def the_your	    = c_common_strings.c_the_your

/* Monster name articles */
val ARTICLE_NONE = 0
val ARTICLE_THE = 1
val ARTICLE_A = 2
val ARTICLE_YOUR = 3

/* Monster name suppress masks */
val SUPPRESS_IT = 0x01
val SUPPRESS_INVISIBLE = 0x02
val SUPPRESS_HALLUCINATION = 0x04
val SUPPRESS_SADDLE = 0x08
val EXACT_NAME = 0x0F

/*** MOTODO
#ifndef TCAP_H
struct tc_gbl_data {	/* also declared in tcap.h */
    char *tc_AS, *tc_AE;	/* graphics start and end (tty font swapping) */
    int   tc_LI,  tc_CO;	/* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif
***/

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
val HACKPREFIX = 0
val LEVELPREFIX = 1
val SAVEPREFIX = 2
val BONESPREFIX = 3
val DATAPREFIX = 4	/* this one must match hardcoded value in dlb.c */
val SCOREPREFIX = 5
val LOCKPREFIX = 6
val CONFIGPREFIX = 7
val TROUBLEPREFIX = 8
val PREFIX_COUNT = 9
/* used in files.c; xxconf.h can override if needed */
val FQN_MAX_FILENAME = 512

class autopickup_exception {
	var pattern: String = null
	var grab = false
	var next: autopickup_exception = null
}

class _plinemsg {
	var msgtype: xchar = 0
	var pattern: String = null
	var next:_plinemsg = null
}

val MSGTYP_NORMAL = 0
val MSGTYP_NOREP = 1
val MSGTYP_NOSHOW = 2
val MSGTYP_STOP = 3

class u_achieve {
        var get_bell = false        /* You have obtained the bell of 
                                      * opening */
        var get_candelabrum = false /* You have obtained the candelabrum */
        var get_book = false        /* You have obtained the book of 
                                      * the dead */
        var enter_gehennom = false  /* Entered Gehennom (including the 
                                      * Valley) by any means */
        var perform_invocation = false /* You have performed the invocation
                                         * ritual */
        var get_amulet = false      /* You have obtained the amulet
                                      * of Yendor */
        var ascended = false        /* You ascended to demigod[dess]hood.
                                      * Not quite the same as 
                                      * u.uevent.ascended. */
        var get_luckstone = false   /* You obtained the luckstone at the
                                      * end of the mines. */
        var finish_sokoban = false  /* You obtained the sokoban prize. */
        var killed_medusa = false   /* You defeated Medusa. */
}

class realtime_data {
  var realtime: time_t = 0    /* Amount of actual playing time up until the last time
                       * the game was restored. */
  var restoretime: time_t = 0 /* The time that the game was started or restored. */
  var last_displayed_time: time_t = 0 /* Last time displayed on the status line */
}

var afternmv: () => Int = null
var occupation: () => Int = null

/* from xxxmain.c */
val hname = "Mohack" 		/* MONOTE: Was name of the game (argv[0] of main) */
var hackpid = 0		/* current process id */
var locknum = 0		/* max num of simultaneous users */

val bases = new Array[Int](MAXOCLASSES)

var multi = 0
var multi_txt: String = null
var nroom = 0
var nsubroom = 0
var occtime = 0

var x_maze_max, y_maze_max = 0	/* initialized in main, used in mkmaze.c */
var otg_temp = 0			/* used by object_to_glyph() [otg] */

var in_doagain = 0

/*
 *	The following structure will be initialized at startup time with
 *	the level numbers of some "important" things in the game.
 */
val dungeon_topology = new dgn_topology()

val quest_status = new q_score()

val pl_tutorial = new Array[Char](QT_T_MAX-QT_T_FIRST+1)

val smeq = new Array[Int](MAXNROFROOMS+1)
val doorindex = 0

var save_cm: String = null
var killer_format = 0
var killer: String = null
var delayed_kill: String = null
var killer_buf: String = null

var killer_flags = 0L

var nomovemsg: String = null
var nul = new Array[Char](40)			/* contains zeros */
var plname: String = null		/* player name */
var pl_character: String = null
var pl_race = '\0'

var pl_fruit: String = null
var current_fruit = 0
var ffruit: fruit = null

val tune = new Array[Char](6)

var occtxt: String = null
val quitchars = " \r\n\033"
val vowels = "aeiouAEIOU"
val ynchars = "yn"
val ynqchars = "ynq"
val ynaqchars = "ynaq"
val ynNaqchars = "yn#aq"
val yn_number = 0L

val disclosure_options = "iavgc"

val level_info = new Array[linfo](MAXLINFO)

val program_state = new sinfo()

/* 'rogue'-like direction commands (cmd.c) */
val sdir = "hykulnjb><"
val ndir = "47896321><"	/* number pad mode */
val xdir: Array[schar] = Array( -1,-1, 0, 1, 1, 1, 0,-1, 0, 0 )
val ydir: Array[schar] = Array(  0,-1,-1,-1, 0, 1, 1, 1, 0, 0 )
val zdir: Array[schar] = Array(  0, 0, 0, 0, 0, 0, 0, 0, 1,-1 )

var tbx = 0, tby: schar = 0	/* mthrowu: target */

/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
val m_shot = new multishot( 0, 0, STRANGE_OBJECT, FALSE )

val digging = new dig_info()

val dungeons = new Array[dungeon](MAXDUNGEON)	/* ini'ed by init_dungeon() */
val sp_levchn = new s_level()
val upstair = new stairway( 0, 0 ); val dnstair = new stairway( 0, 0 )
val upladder = new stairway( 0, 0 ); val dnladder = new stairway( 0, 0 )
val sstairs = new stairway( 0, 0 )
val updest = new dest_area( 0, 0, 0, 0, 0, 0, 0, 0 )
val dndest = new dest_area( 0, 0, 0, 0, 0, 0, 0, 0 )
val inv_pos = new coord()

var in_mklev = false
var in_mk_rndvault = false
var rndvault_failed = false
var stoned = false	/* done to monsters hit by 'c' */
var unweapon = false
var mrg_to_wielded = false
			 /* weapon picked is merged with wielded one */
var current_wand: obj = null	/* wand currently zapped/applied */

var in_steed_dismounting = false

val bhitpos = new coord()
val doors = new Array[coord](DOORMAX)

val rooms = new Array[mkroom](MAXNROFROOMS+1)*2)
// MOTODO: subrooms uses pointer math to point into rooms array
val subrooms: mkroom = &rooms[MAXNROFROOMS+1]
var upstairs_room, dnstairs_room, sstairs_room: mkroom = null

var head_engr: engr = null

var level: dlevel_t = null		/* level map */
var ftrap: ftrap = null
var youmonst = new monst()
var upermonst = new permonst()
var flags = new flag()
var iflags = new instance_flags(0
var u = new you()

var invent: obj = null
var uwep: obj = null; var uarm: obj = null
var uswapwep: obj = null
var uquiver: obj = null /* quiver */
var uarmu: obj = null; /* under-wear, so to speak */
var uskin: obj = null; /* dragon armor, if a dragon */
var uarmc: obj = null; var uarmh: obj = null
var uarms: obj = null; var uarmg: obj = null
var uarmf: obj = null; var uamul: obj = null
var uright: obj = null
var uleft: obj = null
var ublindf: obj = null
var uchain: obj = null
var uball: obj = null

/*
 *  This must be the same order as used for buzz() in zap.c.
 */
val zapcolors: Array[Int] = Array(
    HI_ZAP,		/* 0 - missile */
    CLR_ORANGE,		/* 1 - fire */
    CLR_WHITE,		/* 2 - frost */
    HI_ZAP,		/* 3 - sleep */
    CLR_BLACK,		/* 4 - death */
    CLR_WHITE,		/* 5 - lightning */
    CLR_YELLOW,		/* 6 - poison gas */
    CLR_RED,		/* 7 - lava */
    CLR_GREEN,		/* 8 - acid */
)

val shield_static: Array[Int] = Array(
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,	/* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
)

val spl_book = new Array[Spell](MAXSPELL + 1)

var moves = 1L
var monstermoves = 1L
	 /* These diverge when player is Fast */
var wailmsg = 0L

/* objects that are moving to another dungeon level */
var migrating_objs: obj = null
/* objects not yet paid for */
var billobjs: obj = null

/* used to zero all elements of a struct obj */
var zeroobj = new obj()

/* originally from dog.c */
var dogname: String = null
var catname[P: String = null
var horsename[: String = null

var monkeyname: String = null
var wolfname: String = null
var crocodilename: String = null
/* ...and 'preferred_pet', just below, can now be 'e'. */
var ratname: String = null

var preferred_pet: Char = 0	/* '\0', 'c', 'd', 'n' (none) */
/* monsters that went down/up together with @ */
var mydogs: monst = null
/* monsters that are moving to another dungeon level */
var migrating_mons: monst = null

val mvitals = new Array[mvital](NUMMONS)

/* originally from end.c */
#ifdef DUMP_LOG
#ifdef DUMP_FN
char dump_fn[] = DUMP_FN;
#else
char dump_fn[PL_PSIZ] = DUMMY;
#endif
#endif /* DUMP_LOG */

var c_obj_colors = List(
	"black",		/* CLR_BLACK */
	"red",			/* CLR_RED */
	"green",		/* CLR_GREEN */
	"brown",		/* CLR_BROWN */
	"blue",			/* CLR_BLUE */
	"magenta",		/* CLR_MAGENTA */
	"cyan",			/* CLR_CYAN */
	"gray",			/* CLR_GRAY */
	"transparent",		/* no_color */
	"orange",		/* CLR_ORANGE */
	"bright green",		/* CLR_BRIGHT_GREEN */
	"yellow",		/* CLR_YELLOW */
	"bright blue",		/* CLR_BRIGHT_BLUE */
	"bright magenta",	/* CLR_BRIGHT_MAGENTA */
	"bright cyan",		/* CLR_BRIGHT_CYAN */
	"white",		/* CLR_WHITE */
)

var menu_colorings: menucoloring = null

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
val materialnm = List(
	"mysterious", "liquid", "wax", "organic", "flesh",
	"paper", "cloth", "leather", "wooden", "bone", "dragonhide",
	"iron", "metal", "copper", "silver", "gold", "platinum", "mithril",
	"plastic", "glass", "gemstone", "stone"
)

/* Vision */
var vision_full_recalc = false
char	 **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
var WIN_MESSAGE = WIN_ERR, WIN_STATUS = WIN_ERR
var WIN_MAP = WIN_ERR, WIN_INVEN = WIN_ERR
var toplines = new Array[Char](TBUFSZ)
/* Windowing stuff that's really tty oriented, but present for all ports */
val tc_gbl_data = new tc_gbl_data()	/* AS,AE, LI,CO */

val fqn_prefix = new Array[String](PREFIX_COUNT)

val fqn_prefix_names = List( "hackdir", "leveldir", "savedir",
					"bonesdir", "datadir", "scoredir",
					"lockdir", "configdir", "troubledir" )

val achieve = new u_achieve()

val realtime_data = new realtime_data()

val pline_msg: _plinemsg = null

/* FIXME: The curses windowport requires this stupid hack, in the
   case where a game is in progress and the user is asked if he
   wants to destroy old game.
   Without this, curses tries to show the yn() question with pline()
   ...but the message window isn't up yet.
 */
val curses_stupid_hack = true
}
