/*  SCCS Id: @(#)hack.h 3.4 2001/04/12  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif
#include "lint.h"

/*  For debugging beta code.    */
#ifdef BETA
#define Dpline  pline
#endif

#define TELL        1
#define NOTELL      0
#define ON      1
#define OFF     0
#define BOLT_LIM    8 /* from this distance ranged attacks will be made */
#define MAX_CARR_CAP    1000    /* so that boulders can be heavier */
#define DUMMY { 0 }

/* The UNDEFINED macros are used to initialize variables whose
   initialized value is not relied upon.
   UNDEFINED_VALUE: used to initialize any scalar type except pointers.
   UNDEFINED_VALUES: used to initialize any non scalar type without pointers.
   UNDEFINED_PTR: can be used only on pointer types. */
#define UNDEFINED_VALUE 0
#define UNDEFINED_VALUES { 0 }
#define UNDEFINED_PTR NULL

/* symbolic names for capacity levels */
#define UNENCUMBERED    0
#define SLT_ENCUMBER    1   /* Burdened */
#define MOD_ENCUMBER    2   /* Stressed */
#define HVY_ENCUMBER    3   /* Strained */
#define EXT_ENCUMBER    4   /* Overtaxed */
#define OVERLOADED  5   /* Overloaded */

/* weight increment of heavy iron ball */
#define IRON_BALL_W_INCR 160

/* number of turns it takes for vault guard to show up */
#define VAULT_GUARD_TIME 30

#define SHOP_DOOR_COST 400L /* cost of a destroyed shop door */
#define SHOP_BARS_COST 300L /* cost of iron bars */
#define SHOP_HOLE_COST 200L /* cost of making hole/trapdoor */
#define SHOP_WALL_COST 200L /* cost of destroying a wall */
#define SHOP_WALL_DMG  (10L * ACURRSTR) /* damaging a wall */

/* hunger states - see hu_stat in eat.c */
enum hunger_state_types {
    SATIATED = 0,
    NOT_HUNGRY,
    HUNGRY,
    WEAK,
    FAINTING,
    FAINTED,
    STARVED
};

/* Macros for how a rumor was delivered in outrumor() */
#define BY_ORACLE   0
#define BY_COOKIE   1
#define BY_PAPER    2
#define BY_OTHER    9

/* Macros for why you are no longer riding */
#define DISMOUNT_GENERIC    0
#define DISMOUNT_FELL       1
#define DISMOUNT_THROWN     2
#define DISMOUNT_POLY       3
#define DISMOUNT_ENGULFED   4
#define DISMOUNT_BONES      5
#define DISMOUNT_BYCHOICE   6
#define DISMOUNT_VANISHED   7

/* mgflags for mapglyph() */
#define MG_FLAG_NORMAL     0x00
#define MG_FLAG_NOOVERRIDE 0x01

/* Special returns from mapglyph() */
#define MG_CORPSE   0x01
#define MG_INVIS    0x02
#define MG_DETECT   0x04
#define MG_PET      0x08
#define MG_RIDDEN   0x10
#define MG_STATUE   0x20
#define MG_INVERSE  0x40 /* use inverse video */
#define MG_PEACEFUL 0x80
#define MG_ENGRAVING 0x100

/* sellobj_state() states */
#define SELL_NORMAL (0)
#define SELL_DELIBERATE (1)
#define SELL_DONTSELL   (2)

/* alteration types--keep in synch with costly_alteration(mkobj.c) */
enum cost_alteration_types {
    COST_CANCEL    =  0, /* standard cancellation */
    COST_DRAIN     =  1, /* drain life upon an object */
    COST_UNCHRG    =  2, /* cursed charging */
    COST_UNBLSS    =  3, /* unbless (devalues holy water) */
    COST_UNCURS    =  4, /* uncurse (devalues unholy water) */
    COST_DECHNT    =  5, /* disenchant weapons or armor */
    COST_DEGRD     =  6, /* removal of rustproofing, dulling via engraving */
    COST_DILUTE    =  7, /* potion dilution */
    COST_ERASE     =  8, /* scroll or spellbook blanking */
    COST_BURN      =  9, /* dipped into flaming oil */
    COST_NUTRLZ    = 10, /* neutralized via unicorn horn */
    COST_DSTROY    = 11, /* wand breaking (bill first, useup later) */
    COST_SPLAT     = 12, /* cream pie to own face (ditto) */
    COST_BITE      = 13, /* start eating food */
    COST_OPEN      = 14, /* open tin */
    COST_BRKLCK    = 15, /* break box/chest's lock */
    COST_RUST      = 16, /* rust damage */
    COST_ROT       = 17, /* rotting attack */
    COST_CORRODE   = 18, /* acid damage */
    COST_DISSOLVE  = 19, /* dissolved in acid */
    COST_TRANSFORM = 20, /* transformed in some way */
};

/* bitmask flags for xname();
   PFX_THE takes precedence over ARTICLE, NO_PFX takes precedence over both */
#define CXN_NORMAL        0 /* no special handling */
#define CXN_SINGULAR      1 /* override quantity if greather than 1 */
#define CXN_NO_PFX        2 /* suppress "the" from "the Unique Monst */
#define CXN_PFX_THE       4 /* prefix with "the " (unless pname) */
#define CXN_ARTICLE       8 /* include a/an/the prefix */
#define CXN_NOCORPSE     16 /* suppress " corpse" suffix */
#define CXN_UNIDENTIFIED 32 /* force unidentified name */

/* flags for look_here() */
#define LOOKHERE_PICKED_SOME   1
#define LOOKHERE_SKIP_DFEATURE 2

/* game events log */
struct gamelog_line {
    long turn; /* turn when this happened */
    long flags; /* LL_foo flags */
    char *text;
    struct gamelog_line *next;
};

/* inventory counts (slots in tty parlance)
 * a...zA..Z    invlet_basic (52)
 * $a...zA..Z#  2 special additions
 */
enum inventory_counts {
    invlet_basic = 52,
    invlet_gold = 1,
    invlet_overflow = 1,
    invlet_max = invlet_basic + invlet_gold + invlet_overflow,
    /* 2023/11/30 invlet_max is not yet used anywhere */
};

/* getpos() return values */
enum getpos_retval {
    LOOK_TRADITIONAL = 0, /* '.' -- ask about "more info?" */
    LOOK_QUICK       = 1, /* ',' -- skip "more info?" */
    LOOK_ONCE        = 2, /* ';' -- skip and stop looping */
    LOOK_VERBOSE     = 3  /* ':' -- show more info w/o asking */
};

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.  Some parts of the
 * code assume that PANIC separates the deaths from the non-deaths.
 */
#define DIED         0
#define CHOKING      1
#define POISONING    2
#define STARVING     3
#define DROWNING     4
#define BURNING      5
#define DISSOLVED    6
#define CRUSHING     7
#define STONING      8
#define TURNED_SLIME     9
#ifdef WEBB_DISINT
# define GENOCIDED  10
# define DISINTEGRATED  11
# define MAX_SURVIVABLE_DEATH   11
# define PANICKED   12
# define TRICKED    13
# define QUIT       14
# define ESCAPED    15
#ifdef ASTRAL_ESCAPE
#define DEFIED      16
#define ASCENDED    17
#endif
#endif

typedef struct strbuf {
    int    len;
    char * str;
    char   buf[256];
} strbuf_t;

#include "align.h"
#include "dungeon.h"
#include "monsym.h"
#include "mkroom.h"
#include "objclass.h"
#include "youprop.h"
#include "wintype.h"
#include "context.h"
#include "timeout.h"

NEARDATA extern coord bhitpos;  /* place where throw or zap hits or stops */

/* types of calls to bhit() */
enum bhit_call_types {
    ZAPPED_WAND = 0,
    THROWN_WEAPON,
    THROWN_TETHERED_WEAPON,
    KICKED_WEAPON,
    FLASHED_LIGHT,
    INVIS_BEAM
};


/* attack mode for hmon() */
enum hmon_atkmode_types {
    HMON_MELEE   = 0, /* hand-to-hand */
    HMON_THROWN  = 1, /* normal ranged (or spitting while poly'd) */
    HMON_KICKED  = 2, /* alternate ranged */
    HMON_APPLIED = 3, /* polearm, treated as ranged */
    HMON_DRAGGED = 4  /* attached iron ball, pulled into mon */
};

struct launchplace {
    struct obj *obj;
    coordxy x, y;
};

struct restore_info {
    const char *name;
    int mread_flags;
};

enum restore_stages {
    REST_GSTATE = 1,    /* restoring current level and game state */
    REST_LEVELS = 2,    /* restoring remainder of dungeon */
};

/* structure for 'program_state'; not saved and restored */
struct sinfo {
    int gameover;               /* self explanatory? */
    int stopprint;              /* inhibit further end of game disclosure */
#ifdef HANGUPHANDLING
    volatile int done_hup;      /* SIGHUP or moral equivalent received
                                 * -- no more screen output */
    int preserve_locks;         /* don't remove level files prior to exit */
#endif
    int something_worth_saving; /* in case of panic */
    int panicking;              /* `panic' is in progress */
    int exiting;                /* an exit handler is executing */
    int saving;                 /* creating a save file */
    int restoring;              /* reloading a save file */
    int in_moveloop;            /* normal gameplay in progress */
    int in_impossible;          /* reportig a warning */
    int in_docrt;               /* in docrt(): redrawing the whole screen */
    int in_self_recover;        /* processsing orphaned level files */
    int in_checkpoint;          /* saving insurance checkpoint */
    int in_parseoptions;        /* in parseoptions */
    int in_role_selection;      /* role/race/&c selection menus in progress */
    int in_getlin;              /* inside interface getlin routine */
    int config_error_ready;     /* config_error_add is ready, available */
    int beyond_savefile_load;   /* set when past savefile loading */
#ifdef PANICLOG
    int in_paniclog;            /* writing a panicloc entry */
#endif
    int wizkit_wishing;         /* starting wizard mode game w/ WIZKIT file */
    /* input_state:  used in the core for the 'altmeta' option to process ESC;
       used in the curses interface to avoid arrow keys when user is doing
       something other than entering a command or direction and in the Qt
       interface to suppress menu commands in similar conditions;
       readchar() alrways resets it to 'otherInp' prior to returning */
    int input_state; /* whether next key pressed will be entering a command */
#ifdef TTY_GRAPHICS
    /* resize_pending only matters when handling a SIGWINCH signal for tty;
       getting_char is used along with that and also separately for UNIX;
       we minimize #if conditionals for them to avoid unnecessary clutter */
    volatile int resize_pending; /* set by signal handler */
    volatile int getting_char;  /* referenced during signal handling */
#endif
};

/* Flags for controlling uptodate */
#define UTD_CHECKSIZES                 0x01
#define UTD_CHECKFIELDCOUNTS           0x02
#define UTD_SKIP_SANITY1               0x04
#define UTD_SKIP_SAVEFILEINFO          0x08
#define UTD_WITHOUT_WAITSYNCH_PERFILE  0x10

/* NetHack ftypes */
#define NHF_LEVELFILE       1
#define NHF_SAVEFILE        2
#define NHF_BONESFILE       3
/* modes */
#define READING  0x0
#define COUNTING 0x1
#define WRITING  0x2
#define FREEING  0x4
#define MAX_BMASK 4
/* operations of the various saveXXXchn & co. routines */
#define perform_bwrite(nhfp) ((nhfp)->mode & (COUNTING | WRITING))
#define release_data(nhfp) ((nhfp)->mode & FREEING)

/* Content types for fieldlevel files */
struct fieldlevel_content {
    boolean deflt;        /* individual fields */
    boolean binary;       /* binary rather than text */
    boolean json;         /* JSON */
};

typedef struct {
    int fd;               /* for traditional structlevel binary writes */
    int mode;             /* holds READING, WRITING, or FREEING modes  */
    int ftype;            /* NHF_LEVELFILE, NHF_SAVEFILE, or NHF_BONESFILE */
    int fnidx;            /* index of procs for fieldlevel saves */
    long count;           /* holds current line count for default style file,
                             field count for binary style */
    boolean structlevel;  /* traditional structure binary saves */
    boolean fieldlevel;   /* fieldlevel saves saves each field individually */
    boolean addinfo;      /* if set, some additional context info from core */
    boolean eof;          /* place to mark eof reached */
    boolean bendian;      /* set to true if executing on big-endian machine */
    FILE *fpdef;          /* file pointer for fieldlevel default style */
    FILE *fpdefmap;       /* file pointer mapfile for def format */
    FILE *fplog;          /* file pointer logfile */
    FILE *fpdebug;        /* file pointer debug info */
    struct fieldlevel_content style;
} NHFILE;

#include "rect.h"
#include "region.h"
#include "trap.h"
#include "flag.h"
#include "decl.h"
#include "rm.h"
#include "selvar.h"
#include "display.h"
#include "vision.h"
#include "engrave.h"
#include "botl.h"

/* Symbol offsets */
#define SYM_NOTHING 0
#define SYM_OFF_P (0)
#define SYM_OFF_O (SYM_OFF_P + MAXPCHARS)
#define SYM_OFF_M (SYM_OFF_O + MAXOCLASSES)
#define SYM_OFF_W (SYM_OFF_M + MAXMCLASSES)
#define SYM_OFF_X (SYM_OFF_W + WARNCOUNT)
#define SYM_MAX (SYM_OFF_X + MAXOTHER)

#define def_monsyms_explain(i) monexplain[i]
#define def_objsyms_explain(i) objexplain[i]
#define def_oc_syms_name(i)    oclass_names[i]

#include "extern.h"
#include "winprocs.h"
#include "sys.h"

#ifdef USE_TRAMPOLI
#include "wintty.h"
#undef WINTTY_H
#include "trampoli.h"
#undef EXTERN_H
#include "extern.h"
#endif /* USE_TRAMPOLI */

#define NO_SPELL    0

typedef uint32_t mmflags_nht;     /* makemon MM_ flags */

/* flags to control makemon() */
/* goodpos() uses some plus has some of its own */
#define NO_MM_FLAGS     0x00000L /* use this rather than plain 0 */
#define NO_MINVENT      0x00001L /* suppress minvent when creating mon */
#define MM_NOWAIT       0x00002L /* don't set STRAT_WAITMASK flags */
#define MM_NOCOUNTBIRTH 0x00004L /* don't increment born count (for revival) */
#define MM_IGNOREWATER  0x00008L /* ignore water when positioning */
#define MM_ADJACENTOK   0x00010L /* acceptable to use adjacent coordinates */
#define MM_ANGRY        0x00020L /* monster is created angry */
#define MM_NONAME       0x00040L /* monster is not christened */
#define MM_EGD          0x00080L /* add egd structure */
#define MM_EPRI         0x00100L /* add epri structure */
#define MM_ESHK         0x00200L /* add eshk structure */
#define MM_EMIN         0x00400L /* add emin structure */
#define MM_EDOG         0x00800L /* add edog structure */
#define MM_ASLEEP       0x01000L /* monsters should be generated asleep */
#define MM_NOGRP        0x02000L /* suppress creation of monster groups */
#define MM_NOTAIL       0x04000L /* if a long worm, don't give it a tail */
#define MM_NOMSG       0x040000L /* no appear message */
/* if more MM_ flag masks are added, skip or renumber the GP_ one(s) */
#define GP_ALLOW_XY    0x080000L /* [actually used by enexto() to decide
                                  * whether to make extra call to goodpos()] */
#define GP_ALLOW_U     0x100000L /* don't reject hero's location */
#define GP_CHECKSCARY  0x200000L /* check monster for onscary() */

/* flags for make_corpse() and mkcorpstat(); 0..7 are recorded in obj->spe */
#define CORPSTAT_NONE     0x00
#define CORPSTAT_GENDER   0x03 /* 0x01 | 0x02 */
#define CORPSTAT_HISTORIC 0x04 /* historic statue; not used for corpse */
#define CORPSTAT_SPE_VAL  0x07 /* 0x03 | 0x04 */
#define CORPSTAT_INIT     0x08 /* pass init flag to mkcorpstat */
#define CORPSTAT_BURIED   0x10 /* bury the corpse or statue */
/* note: gender flags have different values from those used for monsters
   so that 0 can be unspecified/random instead of male */
#define CORPSTAT_RANDOM 0
#define CORPSTAT_FEMALE 1
#define CORPSTAT_MALE   2
#define CORPSTAT_NEUTER 3

/* flags for decide_to_shift() */
#define SHIFT_SEENMSG 0x01 /* put out a message if in sight */
#define SHIFT_MSG 0x02     /* always put out a message */

/* m_poisongas_ok() return values */
#define M_POISONGAS_BAD   0 /* poison gas is bad */
#define M_POISONGAS_MINOR 1 /* poison gas is ok, maybe causes coughing */
#define M_POISONGAS_OK    2 /* ignores poison gas completely */

/* flags for deliver_obj_to_mon */
#define DF_NONE     0x00
#define DF_RANDOM   0x01
#define DF_ALL      0x04

/* special mhpmax value when loading bones monster to flag as extinct or genocided */
#define DEFUNCT_MONSTER (-100)

/* macro form of adjustments of physical damage based on Half_physical_damage.
 * Can be used on-the-fly with the 1st parameter to losehp() if you don't
 * need to retain the dmg value beyond that call scope.
 * Take care to ensure it doesn't get used more than once in other instances.
 */
#define Maybe_Half_Phys(dmg) \
        ((Half_physical_damage) ? (((dmg) + 1) / 2) : (dmg))

/* flags for special ggetobj status returns */
#define ALL_FINISHED      0x01  /* called routine already finished the job */

/* flags to control query_objlist() */
#define BY_NEXTHERE        0x1 /* follow objlist by nexthere field */
#define AUTOSELECT_SINGLE  0x2 /* if only 1 object, don't ask */
#define USE_INVLET         0x4 /* use object's invlet */
#define INVORDER_SORT      0x8 /* sort objects by packorder */
#define SIGNAL_NOMENU     0x10 /* return -1 rather than 0 if none allowed */
#define SIGNAL_ESCAPE     0x20 /* return -2 rather than 0 for ESC */
#define FEEL_COCKATRICE   0x40 /* engage cockatrice checks and react */
#define INCLUDE_HERO      0x80 /* show hero among engulfer's inventory */

/* Flags to control query_category() */
/* BY_NEXTHERE used by query_category() too, so skip 0x01 */
#define UNPAID_TYPES 0x02
#define GOLD_TYPES   0x04
#define WORN_TYPES   0x08
#define ALL_TYPES    0x10
#define BILLED_TYPES 0x20
#define CHOOSE_ALL   0x40
#define BUC_BLESSED  0x80
#define BUC_CURSED   0x100
#define BUC_UNCURSED 0x200
#define BUC_UNKNOWN  0x400
#define BUC_ALLBKNOWN (BUC_BLESSED|BUC_CURSED|BUC_UNCURSED)
#define BUCX_TYPES (BUC_ALLBKNOWN | BUC_UNKNOWN)
#define UNIDENTIFIED_TYPES 0x800
#define RECENTLY_PICKED_UP 0x1000
#define ALL_TYPES_SELECTED -2

/* Flags to control find_mid() */
#define FM_FMON        0x01 /* search the fmon chain */
#define FM_MIGRATE     0x02 /* search the migrating monster chain */
#define FM_MYDOGS      0x04 /* search mydogs */
#define FM_EVERYWHERE  (FM_FMON | FM_MIGRATE | FM_MYDOGS)

/* Flags to control pick_[race,role,gend,align] routines in role.c */
#define PICK_RANDOM 0
#define PICK_RIGID  1

/* Flags to control dotrap() and mintrap() in trap.c */
#define NO_TRAP_FLAGS 0x00U
#define FORCETRAP     0x01 /* triggering not left to chance */
#define NOWEBMSG      0x02 /* suppress stumble into web message */
#define FORCEBUNGLE   0x04 /* adjustments appropriate for bungling */
#define RECURSIVETRAP 0x08 /* trap changed into another type this same turn */
#define TOOKPLUNGE    0x10 /* used '>' to enter pit below you */
#define VIASITTING    0x20 /* #sit while at trap location (affects message) */
#define FAILEDUNTRAP  0x40 /* trap activated by failed untrap attempt */

/* Flags to control test_move in hack.c */
#define DO_MOVE     0   /* really doing the move */
#define TEST_MOVE   1   /* test a normal move (move there next) */
#define TEST_TRAV   2   /* test a future travel location */
#define TEST_TRAP   3   /* check if a future travel location is a trap */

/*** some utility macros ***/
#define yn(query) yn_function(query, ynchars, 'n')
#define ynq(query) yn_function(query, ynqchars, 'q')
#define ynaq(query) yn_function(query, ynaqchars, 'y')
#define nyaq(query) yn_function(query, ynaqchars, 'n')
#define nyNaq(query) yn_function(query, ynNaqchars, 'n')
#define ynNaq(query) yn_function(query, ynNaqchars, 'y')

/* Macros for scatter */
#define VIS_EFFECTS 0x01    /* display visual effects */
#define MAY_HITMON  0x02    /* objects may hit monsters */
#define MAY_HITYOU  0x04    /* objects may hit you */
#define MAY_HIT     (MAY_HITMON|MAY_HITYOU)
#define MAY_DESTROY 0x08    /* objects may be destroyed at random */
#define MAY_FRACTURE    0x10    /* boulders & statues may fracture */

/* Macros for launching objects */
#define ROLL        0x01    /* the object is rolling */
#define FLING       0x02    /* the object is flying thru the air */
#define LAUNCH_UNSEEN   0x40    /* hero neither caused nor saw it */
#define LAUNCH_KNOWN    0x80    /* the hero caused this by explicit action */

/* Macros for explosion types */
#define EXPL_DARK   0
#define EXPL_NOXIOUS    1
#define EXPL_MUDDY  2
#define EXPL_WET    3
#define EXPL_MAGICAL    4
#define EXPL_FIERY  5
#define EXPL_FROSTY 6
#define EXPL_MAX    7

/* flags for xkilled() [note: meaning of first bit used to be reversed,
   1 to give message and 0 to suppress] */
#define XKILL_GIVEMSG   0
#define XKILL_NOMSG     1
#define XKILL_NOCORPSE  2
#define XKILL_NOCONDUCT 4

/* pline_flags; mask values for custompline()'s first argument */
/* #define PLINE_ORDINARY 0 */
#define PLINE_NOREPEAT   1
#define OVERRIDE_MSGTYPE 2
#define SUPPRESS_HISTORY 4
#define URGENT_MESSAGE   8

/* Lua callback functions */
enum nhcore_calls {
    NHCORE_START_NEW_GAME = 0,
    NHCORE_RESTORE_OLD_GAME,
    NHCORE_MOVELOOP_TURN,
    NHCORE_GAME_EXIT,

    NUM_NHCORE_CALLS
};

/* Macros for messages referring to hands, eyes, feet, etc... */
#define ARM 0
#define EYE 1
#define FACE 2
#define FINGER 3
#define FINGERTIP 4
#define FOOT 5
#define HAND 6
#define HANDED 7
#define HEAD 8
#define LEG 9
#define LIGHT_HEADED 10
#define NECK 11
#define SPINE 12
#define TOE 13
#define HAIR 14
#define BLOOD 15
#define LUNG 16
#define NOSE 17
#define STOMACH 18

/* rloc() flags */
#define RLOC_NONE    0x00
#define RLOC_ERR     0x01 /* allow impossible() if no rloc */
#define RLOC_MSG     0x02 /* show vanish/appear msg */
#define RLOC_NOMSG   0x04 /* prevent appear msg, even for STRAT_APPEARMSG */

/* indices for some special tin types */
#define ROTTEN_TIN     0
#define HOMEMADE_TIN   1
#define SPINACH_TIN  (-1)
#define RANDOM_TIN   (-2)
#define HEALTHY_TIN  (-3)

/* Some misc definitions */
#define POTION_OCCUPANT_CHANCE(n) (13 + 2 * (n))
#define WAND_BACKFIRE_CHANCE 30
#define BALL_IN_MON (u.uswallow && uball && uball->where == OBJ_FREE)
#define CHAIN_IN_MON (u.uswallow && uchain && uchain->where == OBJ_FREE)
#define NODIAG(monnum) ((monnum) == PM_GRID_BUG)

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_PARTIAL     2
#define MENU_FULL    3

#define MENU_SELECTED   TRUE
#define MENU_UNSELECTED FALSE

#define MENU_DEFCNT 1

/* flags for mktrap() */
#define MKTRAP_NOFLAGS       0x0U
#define MKTRAP_SEEN          0x1U /* trap is seen */
#define MKTRAP_MAZEFLAG      0x2U /* choose random coords instead of room */
#define MKTRAP_NOSPIDERONWEB 0x4U /* web will not generate a spider */
#define MKTRAP_NOVICTIM      0x8U /* no victim corpse or items on it */

/* extended command return values */
#define ECMD_OK     0x00 /* cmd done successfully */
#define ECMD_TIME   0x01 /* cmd took time, uses up a turn */
#define ECMD_CANCEL 0x02 /* cmd canceled by user */
#define ECMD_FAIL   0x04 /* cmd failed to finish, maybe with a yafm */

/* flags for newcham() */
#define NO_NC_FLAGS          0U
#define NC_SHOW_MSG          0x01U
#define NC_VIA_WAND_OR_SPELL 0x02U

/* pick a random entry from array */
#define ROLL_FROM(array) array[rn2(SIZE(array))]
/* array with terminator variation */
/* #define ROLL_FROMT(array) array[rn2(SIZE(array) - 1)] */

/*
 * option setting restrictions
 */
enum optset_restrictions {
    set_in_sysconf = 0, /* system config file option only */

    SET_IN_FILE    = 1, /* config file option only */
    set_in_config  = 1, /* config file option only */

    SET_VIA_PROG   = 2, /* may be set via extern program, not seen in game */
    set_viaprog    = 2, /* may be set via extern program, not seen in game */

    DISP_IN_GAME   = 3, /* may be set via extern program, displayed in game */
    set_gameview   = 3, /* may be set via extern program, displayed in game */

    SET_IN_GAME    = 4, /* may be set via extern program or set in the game */
    set_in_game    = 4, /* may be set via extern program or set in the game */

    set_wizonly    = 5, /* may be set set in the game if wizmode */

    set_hidden     = 6  /* placeholder for prefixed entries, never show it  */
};
#define SET__IS_VALUE_VALID(s) ((s < set_in_sysconf) || (s > set_wizonly))

#define FEATURE_NOTICE_VER(major, minor, patch) (((unsigned long)major << 24) | \
                                                 ((unsigned long)minor << 16) | \
                                                 ((unsigned long)patch << 8) | \
                                                 ((unsigned long)0))

#define FEATURE_NOTICE_VER_MAJ    (flags.suppress_alert >> 24)
#define FEATURE_NOTICE_VER_MIN    (((unsigned long)(0x0000000000FF0000L & flags.suppress_alert)) >> 16)
#define FEATURE_NOTICE_VER_PATCH  (((unsigned long)(0x000000000000FF00L & flags.suppress_alert)) >>  8)

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#define plur(x) (((x) == 1) ? "" : "s")

#define ARM_BONUS(obj)  (objects[(obj)->otyp].a_ac + (obj)->spe \
                         /* extra AC point for racial armor */ \
                         + (is_racial_armor(obj) ? 1 : 0) \
                         - min((int)greatest_erosion(obj), objects[(obj)->otyp].a_ac))

#define makeknown(x) discover_object((x), TRUE, TRUE)
#define distu(xx, yy) dist2((int)(xx), (int)(yy), (int) u.ux, (int) u.uy)
#define mdistu(mon) \
        dist2((int) (mon)->mx, (int) (mon)->my, (int) u.ux, (int) u.uy)
#define onlineu(xx, yy) online2((int)(xx), (int)(yy), (int) u.ux, (int) u.uy)

#define rn1(x, y) (rn2(x) + (y))

/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC)    ((AC) >= 0 ? (AC) : -rnd(-(AC)))

#if defined(MICRO) && !defined(__DJGPP__)
#define getuid() 1
#define getlogin() ((char *)0)
#endif /* MICRO */

/* The function argument to qsort() requires a particular
 * calling convention under WINCE which is not the default
 * in that environment.
 */
#if defined(WIN_CE)
# define CFDECLSPEC __cdecl
#else
# define CFDECLSPEC
#endif

#ifdef DEBUG
# ifdef WIZARD
#define debug_pline  if (wizard) pline
# else
#define debug_pline  pline
# endif
#else
# define debug_pline if (0) pline
#endif

#define DEVTEAM_EMAIL "bhaak@gmx.net"
#define DEVTEAM_URL "https://github.com/unnethack/unnethack"

#endif /* HACK_H */
