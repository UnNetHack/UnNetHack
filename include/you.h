/*  SCCS Id: @(#)you.h  3.4 2000/05/21  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

#include "attrib.h"
#include "monst.h"
#ifndef PROP_H
#include "prop.h"       /* (needed here for util/makedefs.c) */
#endif
#include "skills.h"

/*** Substructures ***/

struct RoleName {
    const char  *m; /* name when character is male */
    const char  *f; /* when female; null if same as male */
};

struct RoleAdvance {
    /* "fix" is the fixed amount, "rnd" is the random amount */
    xint16 infix, inrnd; /* at character initialization */
    xint16 lofix, lornd; /* gained per level <  urole.xlev */
    xint16 hifix, hirnd; /* gained per level >= urole.xlev */
};

struct u_have {
    Bitfield(amulet, 1); /* carrying Amulet */
    Bitfield(bell, 1);   /* carrying Bell   */
    Bitfield(book, 1);   /* carrying Book   */
    Bitfield(menorah, 1);    /* carrying Candelabrum */
    Bitfield(questart, 1);   /* carrying the Quest Artifact */
    Bitfield(unused, 3);
};

struct u_event {
    Bitfield(minor_oracle, 1);   /* received at least 1 cheap oracle */
    Bitfield(major_oracle, 1);   /*  "  expensive oracle */
    Bitfield(qcalled, 1);        /* called by Quest leader to do task */
    Bitfield(qexpelled, 1);      /* expelled from the Quest dungeon */
    Bitfield(qcompleted, 1);     /* successfully completed Quest task */
    Bitfield(uheard_tune, 2);    /* 1=know about, 2=heard passtune */
    Bitfield(uopened_dbridge, 1);    /* opened the drawbridge */

    Bitfield(invoked, 1);        /* invoked Gate to the Sanctum level */

    Bitfield(gehennom_entered, 1);          /* entered Gehennom via Valley */
    Bitfield(entered_the_gnomish_mines, 1); /* entered the Gnomish Mines */
    Bitfield(entered_mine_town, 1);         /* entered Minetown */
    Bitfield(entered_sokoban, 1);           /* entered Sokoban */
    Bitfield(entered_the_bigroom, 1);       /* entered the Big Room */
    Bitfield(entered_the_blackmarket, 1);   /* entered the Blackmarket */
    Bitfield(entered_the_town, 1);          /* entered the Town */
    Bitfield(entered_moria, 1);             /* entered Moria */
    Bitfield(entered_the_dragon_caves, 1);  /* entered the Dragon Caves */
    Bitfield(entered_sheol, 1);             /* entered Sheol */
    Bitfield(entered_vlads_tower, 1);       /* entered Vlad's Tower */
    Bitfield(entered_fort_ludios, 1);       /* entered Fort Ludios */

    Bitfield(entered_a_shop, 1);            /* entered a shop */
    Bitfield(entered_a_temple, 1);          /* entered a temple */

    Bitfield(uhand_of_elbereth, 2);  /* became Hand of Elbereth */

    Bitfield(udemigod, 1);       /* killed the wiz */
    Bitfield(uvibrated, 1);      /* stepped on "vibrating square" */
    Bitfield(ascended, 1);       /* has offered the Amulet */
};

#ifdef LIVELOGFILE
# define record_uevent_achievement(message, field) \
{ \
        if (!u.uevent.field) { \
            livelog_printf(LL_ACHIEVE, message); \
            u.uevent.field = TRUE; \
        } \
}
#else
# define record_uevent_achievement(message, field) if (!u.uevent.field) { u.uevent.field = TRUE; }
#endif


/* KMH, conduct --
 * These are voluntary challenges.  Each field denotes the number of
 * times a challenge has been violated.
 */
struct u_conduct {      /* number of times... */
    long unvegetarian;      /* eaten any animal */
    long unvegan;       /* ... or any animal byproduct */
    long food;          /* ... or any comestible */
    long gnostic;       /* used prayer, priest, or altar */
    long weaphit;       /* hit a monster with a weapon */
    long killer;        /* killed a monster yourself */
    long literate;      /* read something (other than BotD) */
    long polypiles;     /* polymorphed an object */
    long polyselfs;     /* transformed yourself */
    long wishes;        /* used a wish */
    long wisharti;      /* wished for an artifact */
    long wishmagic;     /* wished for a magic object */
    long armoruses;     /* put on a piece of armor */
    long unblinded;     /* starting non-blindfolded and removing a blindfold */
    long robbed;        /* killed an artifact-guardian (like an ordinary robber) */
    long elbereths;     /* engraved Elbereth */
    long bones;         /* encountered bones level */
    long non_racial_armor;      /* put on non-racial armor */
    long non_racial_weapons;    /* wielded non-racial weapons */
    long sokoban;           /* how many times violated sokoban "rules" */
    long heaven_or_hell;    /* heaven or hell mode */
    long hell_and_hell;     /* hell and hell mode */
    /* genocides already listed at end of game */
};

#define heaven_or_hell_mode u.uconduct.heaven_or_hell
#define hell_and_hell_mode u.uconduct.hell_and_hell

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
struct u_roleplay {     /* Your character is a/an ... */
    boolean ascet;      /* foodless character */
    boolean atheist;    /* atheist */
    boolean blindfolded;    /* blindfolded character */
    boolean illiterate; /* illiterate character */
    boolean pacifist;   /* pacifist */
    boolean sadist;     /* sadist */
    boolean nudist;     /* nudist */
    boolean vegan;      /* vegan */
    boolean vegetarian; /* vegetarian */
};

/*** Unified structure containing role information ***/
struct Role {
    /*** Strings that name various things ***/
    struct RoleName name;   /* the role's name (from u_init.c) */
    struct RoleName rank[9]; /* names for experience levels (from botl.c) */
    const char *lgod, *ngod, *cgod; /* god names (from pray.c) */
    const char *filecode;   /* abbreviation for use in file names */
    const char *homebase;   /* quest leader's location (from questpgr.c) */
    const char *intermed;   /* quest intermediate goal (from questpgr.c) */

    /*** Indices of important monsters and objects ***/
    short malenum,      /* index (PM_) as a male (botl.c) */
          femalenum,    /* ...or as a female (NON_PM == same) */
          petnum,       /* PM_ of preferred pet (NON_PM == random) */
          ldrnum,       /* PM_ of quest leader (questpgr.c) */
          guardnum,     /* PM_ of quest guardians (questpgr.c) */
          neminum;      /* PM_ of quest nemesis (questpgr.c) */
    short questarti;    /* index (ART_) of quest artifact (questpgr.c) */

    /*** Bitmasks ***/
    short allow;        /* bit mask of allowed variations */
#define ROLE_RACEMASK   0x0ff8      /* allowable races */
#define ROLE_GENDMASK   0xf000      /* allowable genders */
#define ROLE_MALE   0x1000
#define ROLE_FEMALE 0x2000
#define ROLE_NEUTER 0x4000
#define ROLE_ALIGNMASK  AM_MASK     /* allowable alignments */
#define ROLE_LAWFUL AM_LAWFUL
#define ROLE_NEUTRAL    AM_NEUTRAL
#define ROLE_CHAOTIC    AM_CHAOTIC

    /*** Attributes (from attrib.c and exper.c) ***/
    xint16 attrbase[A_MAX];  /* lowest initial attributes */
    xint16 attrdist[A_MAX];  /* distribution of initial attributes */
    struct RoleAdvance hpadv; /* hit point advancement */
    struct RoleAdvance enadv; /* energy advancement */
    xint16 xlev;     /* cutoff experience level */
    xint16 initrecord;   /* initial alignment record */

    /*** Spell statistics (from spell.c) ***/
    int spelbase;       /* base spellcasting penalty */
    int spelheal;       /* penalty (-bonus) for healing spells */
    int spelshld;       /* penalty for wearing any shield */
    int spelarmr;       /* penalty for wearing metal armour */
    int spelstat;       /* which stat (A_) is used */
    int spelspec;       /* spell (SPE_) the class excels at */
    int spelsbon;       /* penalty (-bonus) for that spell */

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
};

extern const struct Role roles[];   /* table of available roles */
extern struct Role urole;
#define Role_if(X)  (urole.malenum == (X))
#define Role_switch (urole.malenum)

/* used during initialization for race, gender, and alignment
   as well as for character class */
#define ROLE_NONE   (-1)
#define ROLE_RANDOM (-2)

/*** Unified structure specifying race information ***/

struct Race {
    /*** Strings that name various things ***/
    const char *noun;   /* noun ("human", "elf") */
    const char *adj;    /* adjective ("human", "elven") */
    const char *coll;   /* collective ("humanity", "elvenkind") */
    const char *filecode;   /* code for filenames */
    struct RoleName individual; /* individual as a noun ("man", "elf") */

    /*** Indices of important monsters and objects ***/
    short malenum,      /* PM_ as a male monster */
          femalenum,    /* ...or as a female (NON_PM == same) */
          mummynum,     /* PM_ as a mummy */
          zombienum;    /* PM_ as a zombie */

    /*** Bitmasks ***/
    short allow;        /* bit mask of allowed variations */
    short selfmask,     /* your own race's bit mask */
          lovemask,     /* bit mask of always peaceful */
          hatemask;     /* bit mask of always hostile */

    /*** Attributes ***/
    xint16 attrmin[A_MAX];   /* minimum allowable attribute */
    xint16 attrmax[A_MAX];   /* maximum allowable attribute */
    struct RoleAdvance hpadv; /* hit point advancement */
    struct RoleAdvance enadv; /* energy advancement */
#if 0   /* DEFERRED */
    int nv_range;       /* night vision range */
    int xray_range;     /* X-ray vision range */
#endif

    /*** Properties in variable-length arrays ***/
    /* intrinsics (see attrib.c) */

    /*** Don't forget to add... ***/
    /* quest leader, guardians, nemesis (monst.c) */
    /* quest dungeon definition (dat/Xyz.dat) */
    /* quest text (dat/quest.txt) */
    /* dictionary entries (dat/data.bas) */
};

extern const struct Race races[];   /* Table of available races */
extern struct Race urace;
#define Race_if(X)  (urace.malenum == (X))
#define Race_switch (urace.malenum)

/*** Unified structure specifying gender information ***/
struct Gender {
    const char *adj;    /* male/female/neuter */
    const char *he;     /* he/she/it */
    const char *him;    /* him/her/it */
    const char *his;    /* his/her/its */
    const char *filecode;   /* file code */
    short allow;        /* equivalent ROLE_ mask */
};
#define ROLE_GENDERS    2   /* number of permitted player genders */
/* increment to 3 if you allow neuter roles */

extern const struct Gender genders[];   /* table of available genders */
/* pronouns for the hero */
#define uhe()      (genders[flags.female ? 1 : 0].he)
#define uhim()     (genders[flags.female ? 1 : 0].him)
#define uhis()     (genders[flags.female ? 1 : 0].his)
/* corresponding pronouns for monsters; yields "it" when mtmp can't be seen */
#define mhe(mtmp)  (genders[pronoun_gender(mtmp, FALSE)].he)
#define mhim(mtmp) (genders[pronoun_gender(mtmp, FALSE)].him)
#define mhis(mtmp) (genders[pronoun_gender(mtmp, FALSE)].his)
/* override "it" if reason is lack of visibility rather than neuter species */
#define noit_mhe(mtmp)  (genders[pronoun_gender(mtmp, TRUE)].he)
#define noit_mhim(mtmp) (genders[pronoun_gender(mtmp, TRUE)].him)
#define noit_mhis(mtmp) (genders[pronoun_gender(mtmp, TRUE)].his)

/*** Unified structure specifying alignment information ***/
struct Align {
    const char *noun;   /* law/balance/chaos */
    const char *adj;    /* lawful/neutral/chaotic */
    const char *filecode;   /* file code */
    short allow;        /* equivalent ROLE_ mask */
    aligntyp value;     /* equivalent A_ value */
};
#define ROLE_ALIGNS 3   /* number of permitted player alignments */

extern const struct Align aligns[]; /* table of available alignments */

enum utraptypes {
    TT_BEARTRAP   = 0,
    TT_PIT,
    TT_WEB,
    TT_LAVA,
    TT_INFLOOR,
    TT_BURIEDBALL,
    TT_SWAMP,
    TT_ICE,
};

/*
 * The following structure contains information about a conduct
 *   - a name (e.g. for a conduct-menu at character creation)
 *   - nouns and adjectives for the highscore
 *   - a flag for 'worth mentioning in the highscore'
 *   - all strings necessary for show_conduct()
 */
/*** Unified structure specifying conduct information ***/
struct Conduct {
    const char *name;       /* pacifism/nudism/...  */
    const char *noun;       /* pacifist/nudist/...  */
    const char *adj;        /* peaceful/nude/...    */

    boolean highscore;      /* conduct appears in the highscore */

    const char *prefix;     /* "You "   */
    const char *presenttxt;     /* "have been " */
    const char *pasttxt;        /* "were "  */
    const char *suffix;     /* "a pacifist" */
    const char *failtxt;        /* "pretended to be a pacifist" */

};

extern const struct Conduct conducts[];/* table of available roleplay conducts*/

/*
 * The following definitions get used to call violated(CONDUCT_XYZ),
 * and to access the information contained in the conducts[] array.
 */

#define ROLE_CONDUCTS       10  /* number of roleplay conducts */

#define FIRST_CONDUCT       0
#define CONDUCT_PACIFISM    0
#define CONDUCT_SADISM      1
#define CONDUCT_ATHEISM     2
#define CONDUCT_NUDISM      3
#define CONDUCT_BLINDFOLDED 4
#define CONDUCT_FOODLESS    5
#define CONDUCT_VEGAN       6
#define CONDUCT_VEGETARIAN  7
#define CONDUCT_ILLITERACY  8
#define CONDUCT_THIEVERY    9
#define LAST_CONDUCT    ROLE_CONDUCTS - 1

/*** Information about the player ***/
struct you {
    coordxy ux, uy;
    schar dx, dy, dz;   /* direction of move (or zap or ... ) */
    schar di;       /* direction of FF */
    coordxy tx, ty;       /* destination of travel */
    coordxy ux0, uy0;     /* initial position FF */
    d_level uz, uz0;    /* your level on this and the previous turn */
    d_level utolev;     /* level monster teleported you to, or uz */
    uchar utotype;      /* bitmask of goto_level() flags for utolev */
    boolean umoved;     /* changed map location (post-move) */
    int last_str_turn;  /* 0: none, 1: half turn, 2: full turn */
    /* +: turn right, -: turn left */
    int ulevel;     /* 1 to MAXULEV */
    int ulevelmax;
    unsigned utrap;     /* trap timeout */
    unsigned utraptype; /* defined if utrap nonzero */
    unsigned ufeetfrozen;   /* feet frozen, works similarly to utrap */
    char urooms[5];     /* rooms (roomno + 3) occupied now */
    char urooms0[5];    /* ditto, for previous position */
    char uentered[5];       /* rooms (roomno + 3) entered this turn */
    char ushops[5];     /* shop rooms (roomno + 3) occupied now */
    char ushops0[5];    /* ditto, for previous position */
    char ushops_entered[5];    /* ditto, shops entered this turn */
    char ushops_left[5];    /* ditto, shops exited this turn */

    int uhunger;    /* refd only in eat.c and shk.c */
    unsigned uhs;       /* hunger state - see eat.c */

    struct prop uprops[LAST_PROP+1];

    unsigned umconf;
    char usick_cause[PL_PSIZ+20]; /* sizeof "unicorn horn named "+1 */
    Bitfield(usick_type, 2);
#define SICK_VOMITABLE 0x01
#define SICK_NONVOMITABLE 0x02
#define SICK_ALL 0x03

    /* These ranges can never be more than MAX_RANGE (vision.h). */
    int nv_range;       /* current night vision range */
    int xray_range;     /* current xray vision range */

    /*
     * These variables are valid globally only when punished and blind.
     */
#define BC_BALL  0x01   /* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02   /* bit mask for chain in 'bc_felt' below */
    int bglyph; /* glyph under the ball */
    int cglyph; /* glyph under the chain */
    int bc_order;   /* ball & chain order [see bc_order() in ball.c] */
    int bc_felt;    /* mask for ball/chain being felt */

    int umonster;           /* hero's "real" monster num */
    int umonnum;            /* current monster number */

    int mh, mhmax, mtimedone;   /* for polymorph-self */
    struct attribs macurr,      /* for monster attribs */
                   mamax; /* for monster attribs */
    int ulycn;          /* lycanthrope type */

    unsigned ucreamed;
    unsigned uswldtim;      /* time you have been swallowed */

    Bitfield(uswallow, 1);       /* true if swallowed */
    Bitfield(uinwater, 1);       /* if you're currently in water (only
                                    underwater possible currently) */
    Bitfield(uundetected, 1);    /* if you're a hiding monster/piercer */
    Bitfield(mfemale, 1);        /* saved human value of flags.female */
    Bitfield(uinvulnerable, 1);  /* you're invulnerable (praying) */
    Bitfield(uburied, 1);        /* you're buried */
    Bitfield(uedibility, 1);     /* blessed food detection; sense unsafe food */
    /* 1 free bit! */

    unsigned udg_cnt;       /* how long you have been demigod */
    struct u_event uevent;      /* certain events have happened */
    struct u_have uhave;        /* you're carrying special objects */
    struct u_conduct uconduct;  /* KMH, conduct */
    struct u_roleplay roleplay; /* roleplay intrinsics */
    long ulives;            /* heaven or hell mode, number of lives */
    struct attribs acurr,       /* your current attributes (eg. str)*/
                   aexe, /* for gain/loss via "exercise" */
                   abon, /* your bonus attributes (eg. str) */
                   amax, /* your max attributes (eg. str) */
                   atemp, /* used for temporary loss/gain */
                   atime; /* used for loss/gain countdown */
    align ualign;           /* character alignment */
#define CONVERT     2
#define A_ORIGINAL  1
#define A_CURRENT   0
    aligntyp ualignbase[CONVERT];   /* for ualign conversion record */
    schar uluck, moreluck;      /* luck and luck bonus */
    int luckturn;
#define Luck    (u.uluck + u.moreluck)
#define LUCKADD     3   /* added value when carrying luck stone */
#define LUCKMAX     10  /* on moonlit nights 11 */
#define LUCKMIN     (-10)
    schar uhitinc;
    schar udaminc;
    schar uac;
    uchar uspellprot;        /* protection by SPE_PROTECTION */
    uchar usptime;           /* #moves until uspellprot-- */
    uchar uspmtime;          /* #moves between uspellprot-- */
    int uhp, uhpmax;
    int uen, uenmax;         /* magical energy - M. Stephenson */
    xint16 uhpinc[MAXULEV];  /* increases to uhpmax for each level gain */
    xint16 ueninc[MAXULEV];  /* increases to uenmax for each level gain */
    int ugangr;              /* if the gods are angry at you */
    int ugifts;              /* number of artifacts bestowed */
    int ublessed, ublesscnt; /* blessing/duration from #pray */
    long umoney0;
    long uexp, urexp;
    long urscore;            /**< the current score */
    long ucleansed;          /* to record moves when player was cleansed */
    long usleep;             /* sleeping; monstermove you last started */
    int uinvault;
    struct monst *ustuck;

    struct monst *usteed;
    long ugallop;
    int urideturns;

    int umortality;         /* how many times you died */
    int ugrave_arise;       /* you die and become something aside from a ghost */
    time_t ubirthday;       /* real world time when game began */
    time_t udeathday;       /* real world time when game ended */

    int weapon_slots;       /* unused skill slots */
    int skills_advanced;    /* # of advances made so far */
    xint16 skill_record[P_SKILL_LIMIT]; /* skill advancements */
    struct skills weapon_skills[P_NUM_SKILLS];
    boolean twoweap;        /* KMH -- Using two-weapon combat */
    boolean incloud;        /* used for blindness in stinking clouds */

};  /* end of `struct you' */

#define Upolyd (u.umonnum != u.umonster)

#endif  /* YOU_H */
