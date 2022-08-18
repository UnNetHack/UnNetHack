/*  SCCS Id: @(#)monst.h    3.4 1999/01/04  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONST_H
#define MONST_H

#ifndef MEXTRA_H
#include "mextra.h"
#endif

/* The weapon_check flag is used two ways:
 * 1) When calling mon_wield_item, is 2-6 depending on what is desired.
 * 2) Between calls to mon_wield_item, is 0 or 1 depending on whether or not
 *    the weapon is known by the monster to be cursed (so it shouldn't bother
 *    trying for another weapon).
 * I originally planned to also use 0 if the monster already had its best
 * weapon, to avoid the overhead of a call to mon_wield_item, but it turns out
 * that there are enough situations which might make a monster change its
 * weapon that this is impractical.  --KAA
 */
# define NO_WEAPON_WANTED 0
# define NEED_WEAPON 1
# define NEED_RANGED_WEAPON 2
# define NEED_HTH_WEAPON 3
# define NEED_PICK_AXE 4
# define NEED_AXE 5
# define NEED_PICK_OR_AXE 6

/* The following flags are used for the second argument to display_minventory
 * in invent.c:
 *
 * PICK_NONE, PICK_ONE, PICK_ANY (wintype.h)  0, 1, 2
 * MINV_NOLET  If set, don't display inventory letters on monster's inventory.
 * MINV_ALL    If set, display all items in monster's inventory, otherwise
 *	       just display wielded weapons and worn items.
 */
#define MINV_PICKMASK 0x03 /* 1|2 */
#define MINV_NOLET    0x04
#define MINV_ALL      0x08

#ifndef ALIGN_H
#include "align.h"
#endif

/* monster appearance types */
enum m_ap_types {
    M_AP_NOTHING   = 0, /* mappearance unused--monster appears as itself */
    M_AP_FURNITURE,     /* stairs, a door, an altar, etc. */
    M_AP_OBJECT,        /* an object */
    M_AP_MONSTER,       /* a monster; mostly used for cloned Wizard */
};

#define MON_FLOOR        0x00
#define MON_OFFMAP       0x01
#define MON_DETACH       0x02
#define MON_MIGRATING    0x04
#define MON_LIMBO        0x08
#define MON_BUBBLEMOVE   0x10
#define MON_ENDGAME_FREE 0x20
#define MON_ENDGAME_MIGR 0x40
#define MON_OBLITERATE   0x80
#define MSTATE_MASK      0xFF

#define M_AP_TYPMASK  0x7
#define M_AP_F_DKNOWN 0x8
#define U_AP_TYPE (youmonst.m_ap_type & M_AP_TYPMASK)
#define U_AP_FLAG (youmonst.m_ap_type & ~M_AP_TYPMASK)
#define M_AP_TYPE(m) ((m)->m_ap_type & M_AP_TYPMASK)
#define M_AP_FLAG(m) ((m)->m_ap_type & ~M_AP_TYPMASK)

struct monst {
    struct monst *nmon;
    struct permonst *data;
    size_t m_id;
    short mnum;           /* permanent monster index number */
    short cham;           /* if shapeshifter, orig mons[] idx goes here */
    short movement;       /* movement points (derived from permonst definition and added effects */
    uchar m_lev;          /* adjusted difficulty level of monster */
    aligntyp malign;      /* alignment of this monster, relative to the
                             player (positive = good to kill) */
    coordxy mx, my;
    coordxy mux, muy;       /* where the monster thinks you are */
#define MTSZ    4
    coord mtrack[MTSZ];   /* monster track */
    int mhp, mhpmax;
    unsigned mappearance; /* for undetected mimics and the wiz */
    uchar m_ap_type;      /* what mappearance is describing, m_ap_types */

    schar mtame;                /* level of tameness, implies peaceful */
    unsigned short mextrinsics; /* low 8 correspond to mresists */
    int mspec_used;             /* monster's special ability attack timeout */

    int mtrapped;         /* trapped in a pit, web or bear trap */
    int mfeetfrozen;      /* monster's feet are frozen so it can't
                             move (0 = not frozen) */
    int mrevived;         /* has been revived from the dead */

    Bitfield(female, 1);     /* is female */
    Bitfield(minvis, 1);     /* currently invisible */
    Bitfield(invis_blkd, 1); /* invisibility blocked */
    Bitfield(perminvis, 1);  /* intrinsic minvis value */
    Bitfield(mcan, 1);       /* has been cancelled */
    Bitfield(mburied, 1);    /* has been buried */
#define mtemplit mburied     /* since buried isn't implemented, use bit for
                              * monsters shown by transient light source;
                              * only valid during bhit() execution        */
    Bitfield(mundetected, 1);/* not seen in present hiding place;
                              * implies one of M1_CONCEAL or M1_HIDE,
                              * but not mimic (that is, snake, spider,
                              * trapper, piercer, eel) */

    Bitfield(mspeed, 2);    /* current speed */
    Bitfield(permspeed, 2); /* intrinsic mspeed value */
    Bitfield(mcloned, 1);   /* has been cloned from another */
    Bitfield(mavenge, 1);   /* did something to deserve retaliation */
    Bitfield(mflee, 1);     /* fleeing */

    Bitfield(mfleetim, 7);   /* timeout for mflee */
    Bitfield(mcansee, 1);    /* cansee 1, temp.blinded 0, blind 0 */
    Bitfield(mblinded, 7);   /* cansee 0, temp.blinded n, blind 0 */

    Bitfield(mcanmove, 1);   /* paralysis, similar to mblinded */
    Bitfield(mfrozen, 7);

    Bitfield(msleeping, 1);  /* asleep until woken */
    Bitfield(mstun, 1);      /* stunned (off balance) */
    Bitfield(mconf, 1);      /* confused */
    Bitfield(mpeaceful, 1);  /* does not attack unprovoked */
    Bitfield(mleashed, 1);   /* monster is on a leash */
    Bitfield(isshk, 1);      /* is shopkeeper */
    Bitfield(isminion, 1);   /* is a minion */

    Bitfield(isgd, 1);   /* is guard */
    Bitfield(ispriest, 1);   /* is a priest */
    Bitfield(iswiz, 1);  /* is the Wizard of Yendor */
    Bitfield(wormno, 5); /* at most 31 worms on any level */

#define MAX_NUM_WORMS   32  /* should be 2^(wormno bitfield size) */

    unsigned long mstrategy; /* for monsters with mflag3: current strategy */
#ifdef NHSTDC
#define STRAT_APPEARMSG 0x80000000UL
#else
#define STRAT_APPEARMSG 0x80000000L
#endif
#define STRAT_ARRIVE    0x40000000L /* just arrived on current level */
#define STRAT_WAITFORU  0x20000000L
#define STRAT_CLOSE     0x10000000L
#define STRAT_WAITMASK  (STRAT_CLOSE | STRAT_WAITFORU)
#define STRAT_HEAL      0x08000000L
#define STRAT_GROUND    0x04000000L
#define STRAT_MONSTR    0x02000000L
#define STRAT_PLAYER    0x01000000L
#define STRAT_NONE      0x00000000L
#define STRAT_STRATMASK 0x0f000000L
#define STRAT_XMASK     0x00ff0000L
#define STRAT_YMASK     0x0000ff00L
#define STRAT_GOAL      0x000000ffL
#define STRAT_GOALX(s)  ((coordxy)((s & STRAT_XMASK) >> 16))
#define STRAT_GOALY(s)  ((coordxy)((s & STRAT_YMASK) >> 8))

    long mtrapseen;     /* bitmap of traps we've been trapped in */
    long mlstmv;        /* for catching up with lost time */
    long mspare1;
#define mstate mspare1  /* only for debug exam right now, not code flow */
    struct obj *minvent;   /* mon's inventory */
    struct obj *mw;        /* mon's weapon */
    long misc_worn_check;  /* mon's wornmask */
    xint16 weapon_check;   /* flag for whether to try switching weapons */

    long mgrlastattack;    /* turn when a grouper last attacked.
                              at certain points, the attackers decide
                              to get back to attacking all at once. */

#ifdef LIVELOG_BONES_KILLER
    char former_rank[25];   /* for bones' ghost rank in the former life */
#endif

    int meating;            /* monster is eating timeout */
    struct mextra *mextra;  /* point to mextra struct */
};

#define newmonst() (struct monst *) alloc(sizeof (struct monst))

/* these are in mspeed */
#define MSLOW 1     /* slow monster */
#define MFAST 2     /* speeded monster */

#define MON_WEP(mon)    ((mon)->mw)
#define MON_NOWEP(mon)  ((mon)->mw = (struct obj *)0)

#define DEADMONSTER(mon)    ((mon)->mhp < 1)
#define is_starting_pet(mon) ((mon)->m_id == flags.startingpet_mid)
#define is_vampshifter(mon)                                      \
    ((mon)->cham == PM_VAMPIRE || (mon)->cham == PM_VAMPIRE_LORD \
     || (mon)->cham == PM_VLAD_THE_IMPALER)
#define vampshifted(mon) (is_vampshifter((mon)) && !is_vampire((mon)->data))

/* monsters which cannot be displaced: priests, shopkeepers, vault guards,
   Oracle, quest leader */
#define mundisplaceable(mon) ((mon)->ispriest                    \
                              || (mon)->isshk                    \
                              || (mon)->isgd                     \
                              || (mon)->data == &mons[PM_ORACLE] \
                              || (mon)->m_id == quest_status.leader_m_id)

/* mimic appearances that block vision/light */
#define is_lightblocker_mappear(mon)                       \
    ((mon->data == &mons[PM_GIANT_TURTLE]) ||              \
    (is_obj_mappear(mon, BOULDER)                          \
     || (M_AP_TYPE(mon) == M_AP_FURNITURE                  \
         && ((mon)->mappearance == S_hcdoor                \
             || (mon)->mappearance == S_vcdoor             \
             || (mon)->mappearance < S_ndoor /* = walls */ \
             || (mon)->mappearance == S_tree))))
#define is_door_mappear(mon) (M_AP_TYPE(mon) == M_AP_FURNITURE   \
                              && ((mon)->mappearance == S_hcdoor \
                                  || (mon)->mappearance == S_vcdoor))
#define is_obj_mappear(mon,otyp) (M_AP_TYPE(mon) == M_AP_OBJECT \
                                  && (mon)->mappearance == (otyp))

/* Returns true if monster is trapped in place but can still attack.
 * (mon evaluated twice) */
#define IS_TRAPPED(mon) ((mon)->mtrapped || (mon)->mfeetfrozen)

#endif /* MONST_H */
