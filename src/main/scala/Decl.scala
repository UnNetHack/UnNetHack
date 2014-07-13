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

class c_color_names {
    const char	*const c_black, *const c_amber, *const c_golden,
		*const c_light_blue,*const c_red, *const c_orange,
		*const c_green, *const c_silver, *const c_blue,
		*const c_purple, *const c_white;
} c_color_names;
#define NH_BLACK		c_color_names.c_black
#define NH_AMBER		c_color_names.c_amber
#define NH_GOLDEN		c_color_names.c_golden
#define NH_LIGHT_BLUE		c_color_names.c_light_blue
#define NH_RED			c_color_names.c_red
#define NH_ORANGE		c_color_names.c_orange
#define NH_GREEN		c_color_names.c_green
#define NH_SILVER		c_color_names.c_silver
#define NH_BLUE			c_color_names.c_blue
#define NH_PURPLE		c_color_names.c_purple
#define NH_WHITE		c_color_names.c_white

/* The names of the colors used for gems, etc. */
class c_common_strings {
	var c_nothing_happens: String = null; var c_thats_enough_tries: String = null,
	var c_silly_thing_to: String = null; var c_shudder_for_moment: String = null,
	var c_something: String = null; var c_Something: String = null,
	var c_You_can_move_again: String = null;
	var c_Never_mind: String = null; var c_vision_clears: String = null,
	var c_the_your = new Array[String](2)
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

}
