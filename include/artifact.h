/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x0000000L	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x0000001L	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x0000002L	/* item is restricted - can't be named */
#define SPFX_INTEL  0x0000004L	/* item is self-willed - intelligent */
#define SPFX_SPEAK  0x0000008L	/* item can speak (not implemented) */
#define SPFX_SEEK   0x0000010L	/* item helps you search for things */
#define SPFX_WARN   0x0000020L	/* item warns you of danger */
#define SPFX_ATTK   0x0000040L	/* item has a special attack (attk) */
#define SPFX_DEFN   0x0000080L	/* item has a special defence (defn) */
#define SPFX_DRLI   0x0000100L	/* drains a level from monsters */
#define SPFX_SEARCH 0x0000200L	/* helps searching */
#define SPFX_BEHEAD 0x0000400L	/* beheads monsters */
#define SPFX_HALRES 0x0000800L	/* blocks hallucinations */
#define SPFX_ESP    0x0001000L	/* ESP (like amulet of ESP) */
#define SPFX_DISPL  0x0002000L	/* Displacement */
#define SPFX_REGEN  0x0004000L	/* Regeneration */
#define SPFX_EREGEN 0x0008000L	/* Energy Regeneration */
#define SPFX_HSPDAM 0x0010000L	/* 1/2 spell damage (on player) in combat */
#define SPFX_HPHDAM 0x0020000L	/* 1/2 physical damage (on player) in combat */
#define SPFX_TCTRL  0x0040000L	/* Teleportation Control */
#define SPFX_LUCK   0x0080000L	/* Increase Luck (like Luckstone) */
#define SPFX_DMONS  0x0100000L	/* attack bonus on one monster type */
#define SPFX_DCLAS  0x0200000L	/* attack bonus on monsters w/ symbol mtype */
#define SPFX_DFLAG1 0x0400000L	/* attack bonus on monsters w/ mflags1 flag */
#define SPFX_DFLAG2 0x0800000L	/* attack bonus on monsters w/ mflags2 flag */
#define SPFX_DALIGN 0x1000000L	/* attack bonus on non-aligned monsters  */
#define SPFX_DBONUS 0x1F00000L	/* attack bonus mask */
#define SPFX_XRAY   0x2000000L	/* gives X-RAY vision to player */
#define SPFX_REFLECT 0x4000000L /* Reflection */
#define SPFX_WARN_S 0x8000000L	/* item warns you of danger of S_MONSTER */
#define SPFX_STR    0x10000000L   /* item bestows STR 18(**) */
#define SPFX_CON    0x20000000L   /* item bestows CON 25 */
#define SPFX_POLYC  0x40000000L	 /* items grants polymorph control */

#define SPDF_NONE      0x00000000L /* No special effects */
#define SPDF_MAGIC     0x00000001L /* Magic resistance */
#define SPDF_FIRE      0x00000002L /* Fire defense */
#define SPDF_COLD      0x00000004L /* Cold defense */
#define SPDF_SLEEP     0x00000008L /* Sleep defense */
#define SPDF_DISINT    0x00000010L /* Disintegration defense */
#define SPDF_ELEC      0x00000020L /* Shock defense */
#define SPDF_POISON    0x00000040L /* Poison defense */
#define SPDF_ACID      0x00000080L /* Acid defense */
#define SPDF_BLIND     0x00000100L /* Blinding resistance */
#define SPDF_WERE      0x00000200L /* Lycanthropy resistance */
#define SPDF_DRAIN     0x00000400L /* Drain level defense */
#define SPDF_CONFUSE   0x00000800L /* Confusion */
#define SPDF_STUN      0x00001000L /* Stunning */



struct artifact {
	short	    otyp;
	const char  *name;
	unsigned long spfx;	/* special effect from wielding/wearing */
	unsigned long cspfx;	/* special effect just from carrying obj */
	unsigned long mtype;	/* monster type, symbol, or flag */
	struct attack attk;
	unsigned long defn;
	struct attack cary;
	uchar	    inv_prop;	/* property obtained by invoking artifact */
	aligntyp    alignment;	/* alignment of bequeathing gods */
	short	    role;	/* character role associated with */
	short	    race;	/* character race associated with */
	long        cost;	/* price when sold to hero (default 100 x base cost) */
};

/* invoked properties with special powers */
#define TAMING		(LAST_PROP+1)
#define HEALING		(LAST_PROP+2)
#define ENERGY_BOOST	(LAST_PROP+3)
#define UNTRAP		(LAST_PROP+4)
#define CHARGE_OBJ	(LAST_PROP+5)
#define LEV_TELE	(LAST_PROP+6)
#define CREATE_PORTAL	(LAST_PROP+7)
#define ENLIGHTENING	(LAST_PROP+8)
#define CREATE_AMMO	(LAST_PROP+9)
#define PHASING		(LAST_PROP+10)
#define SMOKE_CLOUD	(LAST_PROP+11)

#endif /* ARTIFACT_H */
