/*  SCCS Id: @(#)trap.h 3.4 2000/08/30  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

#ifndef TRAP_H
#define TRAP_H

union vlaunchinfo {
    short v_launch_otyp; /* type of object to be triggered */
    coord v_launch2;     /* secondary launch point (for boulders) */
    uchar v_conjoined;   /* conjoined pit locations */
    short v_tnote;       /* boards: 12 notes */
};

struct trap {
    struct trap *ntrap;
    coordxy tx, ty;
    d_level dst;    /* destination for portals */
    coord launch;
    Bitfield(ttyp, 5);
    Bitfield(tseen, 1);
    Bitfield(once, 1);
    Bitfield(madeby_u, 1); /* So monsters may take offence when you trap
                              them. Recognizing who made the trap isn't
                              completely unreasonable, everybody has
                              their own style.  This flag is also needed
                              when you untrap a monster.  It would be too
                              easy to make a monster peaceful if you could
                              set a trap for it and then untrap it. */
    union vlaunchinfo vl;
#define launch_otyp vl.v_launch_otyp
#define launch2     vl.v_launch2
#define conjoined vl.v_conjoined
#define tnote vl.v_tnote
};

extern struct trap *ftrap;
#define newtrap()   (struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t) (trap))

/* reasons for statue animation */
#define ANIMATE_NORMAL  0
#define ANIMATE_SHATTER 1
#define ANIMATE_SPELL   2

/* reasons for animate_statue's failure */
#define AS_OK        0  /* didn't fail */
#define AS_NO_MON    1  /* makemon failed */
#define AS_MON_IS_UNIQUE 2  /* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
enum trap_types {
    NO_TRAP      =  0,
    ARROW_TRAP,
    DART_TRAP,
    ROCKTRAP,
    SQKY_BOARD,
    BEAR_TRAP,
    LANDMINE,
    ROLLING_BOULDER_TRAP,
    SLP_GAS_TRAP,
    RUST_TRAP,
    FIRE_TRAP,
    PIT,
    SPIKED_PIT,
    HOLE,
    TRAPDOOR,
    TELEP_TRAP,
    LEVEL_TELEP,
    MAGIC_PORTAL,
    WEB,
    STATUE_TRAP,
    MAGIC_TRAP,
    ANTI_MAGIC,
    ICE_TRAP,
    POLY_TRAP,
    VIBRATING_SQUARE,

    TRAPNUM
};

/* some trap-related function return results */
enum {
    Trap_Effect_Finished = 0,
    Trap_Is_Gone = 0,
    Trap_Caught_Mon = 1,
    Trap_Killed_Mon = 2,
    Trap_Moved_Mon = 3, /* new location, or new level */
};

#define is_pit(ttyp) ((ttyp) == PIT || (ttyp) == SPIKED_PIT)
#define is_hole(ttyp)  ((ttyp) == HOLE || (ttyp) == TRAPDOOR)

#endif /* TRAP_H */
