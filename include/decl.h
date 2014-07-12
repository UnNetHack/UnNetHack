/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int NDECL((*occupation));
E int NDECL((*afternmv));

E const char *hname;
E int hackpid;
#if defined(UNIX)
E int locknum;
#endif

E char SAVEF[];

E int bases[MAXOCLASSES];

E int multi;
E char multi_txt[BUFSZ];
E int nroom;
E int nsubroom;
E int occtime;

#define WARNCOUNT 6			/* number of different warning levels */
E uchar warnsyms[WARNCOUNT];

E int x_maze_max, y_maze_max;
E int otg_temp;

E int in_doagain;

E struct dgn_topology {		/* special dungeon levels for speed */
    d_level	d_oracle_level;
    d_level	d_bigroom_level;	/* unused */
    d_level	d_rogue_level;
    d_level	d_medusa_level;
    d_level	d_stronghold_level;
    d_level	d_valley_level;
    d_level	d_wiz1_level;
    d_level	d_wiz2_level;
    d_level	d_wiz3_level;
    d_level	d_juiblex_level;
    d_level	d_orcus_level;
    d_level	d_baalzebub_level;	/* unused */
    d_level	d_asmodeus_level;	/* unused */
    d_level	d_portal_level;		/* only in goto_level() [do.c] */
    d_level	d_sanctum_level;
    d_level	d_earth_level;
    d_level	d_water_level;
    d_level	d_fire_level;
    d_level	d_air_level;
    d_level	d_astral_level;
    xchar	d_tower_dnum;
    xchar	d_sokoban_dnum;
    xchar	d_mines_dnum, d_quest_dnum;
    xchar	d_mall_dnum;
    xchar	d_sheol_dnum;
    d_level	d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level	d_knox_level;
    d_level	d_nymph_level;
    d_level	d_mineend_level;
    d_level	d_sokoend_level;
    d_level	d_minetown_level;
    d_level	d_town_level;
    d_level	d_moria_level;
} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
#define oracle_level		(dungeon_topology.d_oracle_level)
#define bigroom_level		(dungeon_topology.d_bigroom_level)
#define rogue_level		(dungeon_topology.d_rogue_level)
#define medusa_level		(dungeon_topology.d_medusa_level)
#define stronghold_level	(dungeon_topology.d_stronghold_level)
#define valley_level		(dungeon_topology.d_valley_level)
#define wiz1_level		(dungeon_topology.d_wiz1_level)
#define wiz2_level		(dungeon_topology.d_wiz2_level)
#define wiz3_level		(dungeon_topology.d_wiz3_level)
#define juiblex_level		(dungeon_topology.d_juiblex_level)
#define orcus_level		(dungeon_topology.d_orcus_level)
#define baalzebub_level		(dungeon_topology.d_baalzebub_level)
#define asmodeus_level		(dungeon_topology.d_asmodeus_level)
#define portal_level		(dungeon_topology.d_portal_level)
#define sanctum_level		(dungeon_topology.d_sanctum_level)
#define earth_level		(dungeon_topology.d_earth_level)
#define water_level		(dungeon_topology.d_water_level)
#define fire_level		(dungeon_topology.d_fire_level)
#define air_level		(dungeon_topology.d_air_level)
#define astral_level		(dungeon_topology.d_astral_level)
#define tower_dnum		(dungeon_topology.d_tower_dnum)
#define sokoban_dnum		(dungeon_topology.d_sokoban_dnum)
#define mines_dnum		(dungeon_topology.d_mines_dnum)
#define quest_dnum		(dungeon_topology.d_quest_dnum)
#define mall_dnum		(dungeon_topology.d_mall_dnum)
#define sheol_dnum		(dungeon_topology.d_sheol_dnum)
#define qstart_level		(dungeon_topology.d_qstart_level)
#define qlocate_level		(dungeon_topology.d_qlocate_level)
#define nemesis_level		(dungeon_topology.d_nemesis_level)
#define knox_level		(dungeon_topology.d_knox_level)
#define nymph_level		(dungeon_topology.d_nymph_level)
#define mineend_level           (dungeon_topology.d_mineend_level)
#define sokoend_level           (dungeon_topology.d_sokoend_level)
#define minetown_level		(dungeon_topology.d_minetown_level)
#define town_level		(dungeon_topology.d_town_level)
#define moria_level		(dungeon_topology.d_moria_level)

E stairway dnstair, upstair;		/* stairs up and down */
#define xdnstair	(dnstair.sx)
#define ydnstair	(dnstair.sy)
#define xupstair	(upstair.sx)
#define yupstair	(upstair.sy)

E stairway dnladder, upladder;		/* ladders up and down */
#define xdnladder	(dnladder.sx)
#define ydnladder	(dnladder.sy)
#define xupladder	(upladder.sx)
#define yupladder	(upladder.sy)

E stairway sstairs;

E dest_area updest, dndest;	/* level-change destination areas */

E coord inv_pos;
E dungeon dungeons[];
E s_level *sp_levchn;
#define dunlev_reached(x)	(dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
E struct q_score quest_status;

#include "qtext.h"
E char pl_tutorial[QT_T_MAX-QT_T_FIRST+1];

E char pl_character[PL_CSIZ];
E char pl_race;		/* character's race */

E char pl_fruit[PL_FSIZ];
E int current_fruit;
E struct fruit *ffruit;

E char tune[6];

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
E struct linfo level_info[MAXLINFO];

E struct sinfo {
	int gameover;		/* self explanatory? */
	int stopprint;		/* inhibit further end of game disclosure */
#if defined(UNIX) || defined(WIN32)
	int done_hup;		/* SIGHUP or moral equivalent received
				 * -- no more screen output */
#endif
	int something_worth_saving;	/* in case of panic */
	int panicking;		/* `panic' is in progress */
#if defined(WIN32)
	int exiting;		/* an exit handler is executing */
#endif
	int in_impossible;
#ifdef PANICLOG
	int in_paniclog;
#endif
} program_state;

E boolean restoring;

E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char ynNaqchars[];
E long yn_number;

E const char disclosure_options[];

E int smeq[];
E int doorindex;
E char *save_cm;
#define KILLED_BY_AN	 0
#define KILLED_BY	 1
#define NO_KILLER_PREFIX 2
E int killer_format;
E const char *killer;
E const char *delayed_killer;
#ifdef GOLDOBJ
E long done_money;
#endif
E char killer_buf[BUFSZ];

E long killer_flags;

#ifdef DUMP_LOG
E char dump_fn[];		/* dumpfile name (dump patch) */
#endif
#ifdef WHEREIS_FILE 
E char whereis_real_path[255];
#endif
E const char *configfile;
E char plname[PL_NSIZ];
E char dogname[];
E char catname[];
E char horsename[];
#ifdef EXOTIC_PETS
E char monkeyname[];
E char wolfname[];
E char crocodilename[];
#endif
E char ratname[];
E char preferred_pet;
E const char *occtxt;			/* defined when occupation != NULL */
E const char *nomovemsg;
E const char nul[];
E char lock[];

#ifdef QWERTZ
E const char qykbd_dir[], qzkbd_dir[], ndir[];
E char const *sdir;
#else
E const char sdir[], ndir[];
#endif
E const schar xdir[], ydir[], zdir[];

E schar tbx, tby;		/* set in mthrowu.c */

E struct multishot { int n, i; short o; boolean s; } m_shot;

E struct dig_info {		/* apply.c, hack.c */
	int	effort;
	d_level level;
	coord	pos;
	long lastdigtime;
	boolean down, chew, warned, quiet;
} digging;

E long moves, monstermoves;
E long wailmsg;

E boolean in_mklev;
E boolean in_mk_rndvault;
E boolean rndvault_failed;
E boolean stoned;
E boolean unweapon;
E boolean mrg_to_wielded;
E struct obj *current_wand;

E boolean in_steed_dismounting;

E const int shield_static[];

#include "spell.h"
E struct spell spl_book[];	/* sized in decl.c */

#include "color.h"
E const int zapcolors[];

E const char def_oc_syms[MAXOCLASSES];	/* default class symbols */
E uchar oc_syms[MAXOCLASSES];		/* current class symbols */
E const char def_monsyms[MAXMCLASSES];	/* default class symbols */
E uchar monsyms[MAXMCLASSES];		/* current class symbols */

#include "obj.h"
E struct obj *invent,
	*uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
	*uarmu,				/* under-wear, so to speak */
	*uskin, *uamul, *uleft, *uright, *ublindf,
	*uwep, *uswapwep, *uquiver;

E struct obj *uchain;		/* defined only when punished */
E struct obj *uball;
E struct obj *migrating_objs;
E struct obj *billobjs;
E struct obj zeroobj;		/* init'd and defined in decl.c */

#include "engrave.h"
E struct engr *head_engr;

#include "you.h"
E struct you u;

#include "onames.h"
#include "pm.h"

#define PM_BABY_GRAY_DRAGON   PM_BABY_TATZELWORM
#define PM_BABY_SILVER_DRAGON PM_BABY_AMPHITERE
#define PM_BABY_RED_DRAGON    PM_BABY_DRAKEN
#define PM_BABY_WHITE_DRAGON  PM_BABY_LINDWORM
#define PM_BABY_ORANGE_DRAGON PM_BABY_SARKANY
#define PM_BABY_BLACK_DRAGON  PM_BABY_SIRRUSH
#define PM_BABY_BLUE_DRAGON   PM_BABY_LEVIATHAN
#define PM_BABY_GREEN_DRAGON  PM_BABY_WYVERN
/*#define PM_BABY_GLOWING_DRAGON   PM_BABY_GLOWING_DRAGON*/
#define PM_BABY_YELLOW_DRAGON PM_BABY_GUIVRE
#define PM_GRAY_DRAGON        PM_TATZELWORM
#define PM_SILVER_DRAGON      PM_AMPHITERE
#define PM_RED_DRAGON         PM_DRAKEN
#define PM_WHITE_DRAGON       PM_LINDWORM
#define PM_ORANGE_DRAGON      PM_SARKANY
#define PM_BLACK_DRAGON       PM_SIRRUSH
#define PM_BLUE_DRAGON        PM_LEVIATHAN
#define PM_GREEN_DRAGON       PM_WYVERN
/*#define PM_GLOWING_DRAGON   PM_GLOWING_DRAGON*/
#define PM_YELLOW_DRAGON      PM_GUIVRE

#define GRAY_DRAGON_SCALE_MAIL       MAGIC_DRAGON_SCALE_MAIL
#define SILVER_DRAGON_SCALE_MAIL     REFLECTING_DRAGON_SCALE_MAIL
#define RED_DRAGON_SCALE_MAIL        FIRE_DRAGON_SCALE_MAIL
#define WHITE_DRAGON_SCALE_MAIL      ICE_DRAGON_SCALE_MAIL
#define ORANGE_DRAGON_SCALE_MAIL     SLEEP_DRAGON_SCALE_MAIL
#define BLACK_DRAGON_SCALE_MAIL      DISINTEGRATION_DRAGON_SCALE_MA
#define BLUE_DRAGON_SCALE_MAIL       ELECTRIC_DRAGON_SCALE_MAIL
#define GREEN_DRAGON_SCALE_MAIL      POISON_DRAGON_SCALE_MAIL
#define GLOWING_DRAGON_SCALE_MAIL    STONE_DRAGON_SCALE_MAIL
#define YELLOW_DRAGON_SCALE_MAIL     ACID_DRAGON_SCALE_MAIL

#define GRAY_DRAGON_SCALES       MAGIC_DRAGON_SCALES
#define SILVER_DRAGON_SCALES     REFLECTING_DRAGON_SCALES
#define RED_DRAGON_SCALES        FIRE_DRAGON_SCALES
#define WHITE_DRAGON_SCALES      ICE_DRAGON_SCALES
#define ORANGE_DRAGON_SCALES     SLEEP_DRAGON_SCALES
#define BLACK_DRAGON_SCALES      DISINTEGRATION_DRAGON_SCALES
#define BLUE_DRAGON_SCALES       ELECTRIC_DRAGON_SCALES
#define GREEN_DRAGON_SCALES      POISON_DRAGON_SCALES
#define GLOWING_DRAGON_SCALES    STONE_DRAGON_SCALES
#define YELLOW_DRAGON_SCALES     ACID_DRAGON_SCALES


E struct monst youmonst;	/* init'd and defined in decl.c */
E struct monst *mydogs, *migrating_mons;

E struct permonst upermonst;	/* init'd in decl.c, 
					 * defined in polyself.c 
					 */

E struct mvitals {
	uchar	born;
	uchar	died;
	uchar	mvflags;
} mvitals[NUMMONS];

E struct c_color_names {
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
E const char *c_obj_colors[];

E struct c_common_strings {
    const char	*const c_nothing_happens, *const c_thats_enough_tries,
		*const c_silly_thing_to, *const c_shudder_for_moment,
		*const c_something, *const c_Something,
		*const c_You_can_move_again,
		*const c_Never_mind, *c_vision_clears,
		*const c_the_your[2];
} c_common_strings;
#define nothing_happens    c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to	   c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something	   c_common_strings.c_something
#define Something	   c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again
#define Never_mind	   c_common_strings.c_Never_mind
#define vision_clears	   c_common_strings.c_vision_clears
#define the_your	   c_common_strings.c_the_your

/* material strings */
E const char *materialnm[];

/* Monster name articles */
#define ARTICLE_NONE	0
#define ARTICLE_THE	1
#define ARTICLE_A	2
#define ARTICLE_YOUR	3

/* Monster name suppress masks */
#define SUPPRESS_IT		0x01
#define SUPPRESS_INVISIBLE	0x02
#define SUPPRESS_HALLUCINATION  0x04
#define SUPPRESS_SADDLE		0x08
#define EXACT_NAME		0x0F

/* Vision */
E boolean vision_full_recalc;	/* TRUE if need vision recalc */
E char **viz_array;		/* could see/in sight row pointers */

/* Window system stuff */
E winid WIN_MESSAGE, WIN_STATUS;
E winid WIN_MAP, WIN_INVEN;
E char toplines[];
#ifndef TCAP_H
E struct tc_gbl_data {	/* also declared in tcap.h */
    char *tc_AS, *tc_AE;	/* graphics start and end (tty font swapping) */
    int   tc_LI,  tc_CO;	/* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

/* xxxexplain[] is in drawing.c */
E const char * const monexplain[], invisexplain[], * const objexplain[], * const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX	0
#define LEVELPREFIX	1
#define SAVEPREFIX	2
#define BONESPREFIX	3
#define DATAPREFIX	4	/* this one must match hardcoded value in dlb.c */
#define SCOREPREFIX	5
#define LOCKPREFIX	6
#define CONFIGPREFIX	7
#define TROUBLEPREFIX	8
#define PREFIX_COUNT	9
/* used in files.c; xxconf.h can override if needed */
# ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
# endif

#if defined(NOCWD_ASSUMPTIONS) || defined(VAR_PLAYGROUND)
/* the bare-bones stuff is unconditional above to simplify coding; for
 * ports that actually use prefixes, add some more localized things
 */
#define PREFIXES_IN_USE
#endif

E char *fqn_prefix[PREFIX_COUNT];
#ifdef PREFIXES_IN_USE
E char *fqn_prefix_names[PREFIX_COUNT];
#endif

struct autopickup_exception {
	char *pattern;
	boolean grab;
	struct autopickup_exception *next;
};

struct _plinemsg {
	xchar msgtype;
	char *pattern;
	struct _plinemsg *next;
};

E struct _plinemsg *pline_msg;

#define MSGTYP_NORMAL	0
#define MSGTYP_NOREP	1
#define MSGTYP_NOSHOW	2
#define MSGTYP_STOP	3

struct u_achieve {
        Bitfield(get_bell,1);        /* You have obtained the bell of 
                                      * opening */
        Bitfield(get_candelabrum,1); /* You have obtained the candelabrum */
        Bitfield(get_book,1);        /* You have obtained the book of 
                                      * the dead */
        Bitfield(enter_gehennom,1);  /* Entered Gehennom (including the 
                                      * Valley) by any means */
        Bitfield(perform_invocation,1); /* You have performed the invocation
                                         * ritual */
        Bitfield(get_amulet,1);      /* You have obtained the amulet
                                      * of Yendor */
        Bitfield(ascended,1);        /* You ascended to demigod[dess]hood.
                                      * Not quite the same as 
                                      * u.uevent.ascended. */
        Bitfield(get_luckstone,1);   /* You obtained the luckstone at the
                                      * end of the mines. */
        Bitfield(finish_sokoban,1);  /* You obtained the sokoban prize. */
        Bitfield(killed_medusa,1);   /* You defeated Medusa. */
};

E struct u_achieve achieve;

E struct realtime_data {
  time_t realtime;    /* Amount of actual playing time up until the last time
                       * the game was restored. */
  time_t restoretime; /* The time that the game was started or restored. */
  time_t last_displayed_time; /* Last time displayed on the status line */
} realtime_data;

#ifdef SIMPLE_MAIL
E int mailckfreq;
#endif

#ifdef USE_MERSENNE_TWISTER
#include <gsl/gsl_rng.h>
E gsl_rng *rngstate;
#endif



E boolean curses_stupid_hack;

#undef E

#endif /* DECL_H */
