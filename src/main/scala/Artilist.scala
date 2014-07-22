/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Artifact._
import Align._
import MonAttk._
import MonFlag._
import MonSym._
import ONames._
import PerMonst._
import Pm._
import Prop._

/* in artifact.c, set up the actual artifact list structure */
object ArtiList {

def A(name: String, typ: Int, spfx: Int, cspfx: Int, mtype: Long, attk: attack, defn: Int, cary: attack, inv_prop: Int, alignment: aligntyp, role: Int, race: Int, cost: Int): artifact =
    new artifact(typ.toShort, name, spfx, cspfx, mtype, attk, defn, cary, inv_prop.toByte, alignment, race.toShort, role.toShort, cost)

val NO_ATTK = new attack()		/* no attack */
val NO_CARY = new attack()  /* no carry effects */
def CARY(c: Int)	= new attack(0,c,0,0)
def PHYS(a: Int, b: Int) = new attack(0,AD_PHYS,a,b)		/* physical */
def DRLI(a: Int, b: Int) = new attack(0,AD_DRLI,a,b)		/* life drain */
def COLD(a: Int, b: Int) = new attack(0,AD_COLD,a,b)
def FIRE(a: Int, b: Int) = new attack(0,AD_FIRE,a,b)
def ELEC(a: Int, b: Int) = new attack(0,AD_ELEC,a,b)		/* electrical shock */
def STUN(a: Int, b: Int) = new attack(0,AD_STUN,a,b)		/* magical attack */

val artilist: List[artifact] = List(

/* Artifact cost rationale:
 * 1.  The more useful the artifact, the better its cost.
 * 2.  Quest artifacts are highly valued.
 * 3.  Chaotic artifacts are inflated due to scarcity (and balance).
 */


/*  dummy element #0, so that all interesting indices are non-zero */
A("",				STRANGE_OBJECT,
	0, 0, 0, NO_ATTK, SPDF_NONE, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0 ),

A("Excalibur",			LONG_SWORD,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),0,0,
	PHYS(5,10),	SPDF_DRAIN,	NO_CARY,	0, A_LAWFUL, PM_KNIGHT, NON_PM, 4000 ),
A("Luck Blade",			BROADSWORD,
	(SPFX_RESTR|SPFX_LUCK),0,0,
	PHYS(5,6),	SPDF_NONE,	NO_CARY,	0, A_CHAOTIC, PM_CONVICT, NON_PM, 3000 ),
/*
 *	Stormbringer only has a 2 because it can drain a level,
 *	providing 8 more.
 */
A("Stormbringer",		RUNESWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL|SPFX_DRLI), 0, 0,
	DRLI(5,2),	SPDF_DRAIN,	NO_CARY,	0, A_CHAOTIC, NON_PM, NON_PM, 8000 ),

/*
 *	Mjollnir will return to the hand of the wielder when thrown
 *	if the wielder is a Valkyrie wearing Gauntlets of Power.
 */
A("Mjollnir",			WAR_HAMMER,		/* Mjo:llnir */
	(SPFX_RESTR|SPFX_ATTK),  0, 0,
	ELEC(5,24),	SPDF_NONE,	NO_CARY,	0, A_NEUTRAL, PM_VALKYRIE, NON_PM, 4000 ),

A("Cleaver",			BATTLE_AXE,
	SPFX_RESTR, 0, 0,
	PHYS(3,6),	SPDF_NONE,	NO_CARY,	0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500 ),

A("Grimtooth",			ORCISH_DAGGER,
	SPFX_RESTR, 0, 0,
	PHYS(2,6),	SPDF_POISON,	NO_CARY,	0, A_CHAOTIC, PM_ROGUE, PM_ORC, 300 ),
/*
 *	Orcrist and Sting have same alignment as elves.
 */
A("Orcrist",			ELVEN_BROADSWORD,
	SPFX_DFLAG2, 0, M2_ORC,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_CHAOTIC, NON_PM, PM_ELF, 2000 ),

/*
 *	The combination of SPFX_WARN and M2_something on an artifact
 *	will trigger EWarn_of_mon for all monsters that have the appropriate
 *	M2_something flags.  In Sting's case it will trigger EWarn_of_mon
 *	for M2_ORC monsters.
 */
A("Sting",			ELVEN_DAGGER,
	(SPFX_WARN|SPFX_DFLAG2), 0, M2_ORC,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_CHAOTIC, NON_PM, PM_ELF, 800 ),
/*
 *	Magicbane is a bit different!  Its magic fanfare
 *	unbalances victims in addition to doing some damage.
 */
A("Magicbane",			ATHAME,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	STUN(3,4),	SPDF_MAGIC,	NO_CARY,	0, A_NEUTRAL, PM_WIZARD, NON_PM, 3500),

A("Frost Brand",		LONG_SWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	COLD(5,0),	SPDF_COLD,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 3000),

A("Fire Brand",			LONG_SWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	FIRE(5,0),	SPDF_FIRE,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 3000),

A("Dragonbane",			DWARVISH_SPEAR,
	(SPFX_WARN_S|SPFX_RESTR|SPFX_DCLAS), 0, S_DRAGON, PHYS(5,4), 
	(SPDF_FIRE|SPDF_COLD|SPDF_SLEEP|SPDF_DISINT|
	    SPDF_ELEC|SPDF_POISON|SPDF_ACID),
	NO_CARY, 0, A_NONE, NON_PM, NON_PM, 1500),

A("Demonbane",			LONG_SWORD,
	(SPFX_WARN|SPFX_RESTR|SPFX_DFLAG2), 0, M2_DEMON,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 2500),

A("Werebane",			SILVER_SABER,
	(SPFX_WARN|SPFX_RESTR|SPFX_DFLAG2|SPFX_POLYC), 0, M2_WERE,
	PHYS(5,0),	SPDF_WERE,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 1500),

A("Grayswandir",		SILVER_SABER,
	(SPFX_RESTR|SPFX_HALRES), 0, 0,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 8000),

A("Giantslayer",		SLING,
	(SPFX_WARN|SPFX_RESTR|SPFX_DFLAG2|SPFX_STR), 0, M2_GIANT,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_NEUTRAL, NON_PM, NON_PM, 1200),

A("Ogresmasher",		WAR_HAMMER,
	(SPFX_WARN_S|SPFX_RESTR|SPFX_DCLAS|SPFX_CON), 0, S_OGRE,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 1200),

A("Trollsbane",			MORNING_STAR,
	(SPFX_WARN_S|SPFX_RESTR|SPFX_DCLAS|SPFX_REGEN), 0, S_TROLL,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 1200),
/*
 *	Two problems:  1) doesn't let trolls regenerate heads,
 *	2) doesn't give unusual message for 2-headed monsters (but
 *	allowing those at all causes more problems than worth the effort).
 */
A("Vorpal Blade",		LONG_SWORD,
	(SPFX_WARN_S|SPFX_RESTR|SPFX_BEHEAD), 0, S_JABBERWOCK,
	PHYS(5,8),	SPDF_NONE,	NO_CARY,	0, A_NEUTRAL, NON_PM, NON_PM, 4000),
/*
 *	Ah, never shall I forget the cry,
 *		or the shriek that shrieked he,
 *	As I gnashed my teeth, and from my sheath
 *		I drew my Snickersnee!
 *			--Koko, Lord high executioner of Titipu
 *			  (From Sir W.S. Gilbert's "The Mikado")
 */
A("Snickersnee",		KATANA,
	SPFX_RESTR, 0, 0,
	PHYS(0,8),	SPDF_NONE,	NO_CARY,	0, A_LAWFUL, PM_SAMURAI, NON_PM, 1200),

A("Sunsword",			LONG_SWORD,
	(SPFX_RESTR|SPFX_DFLAG2), 0, M2_UNDEAD,
	PHYS(5,0),	SPDF_BLIND,	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 1500),

/*
 *	The artifacts for the quest dungeon, all self-willed.
 */

A("Itlachiayaque",		SHIELD_OF_REFLECTION,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_ESP|SPFX_HSPDAM), 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_FIRE),
	SMOKE_CLOUD,	A_LAWFUL, PM_ARCHEOLOGIST, NON_PM, 3000),

A("The Heart of Ahriman",	LUCKSTONE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_DISPL|SPFX_EREGEN), 0,
	/* this stone does double damage if used as a projectile weapon */
	PHYS(5,0),	SPDF_NONE,	NO_CARY,
	LEVITATION,	A_NEUTRAL, PM_BARBARIAN, NON_PM, 2500),

A("The Sceptre of Might",	MACE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DALIGN), 0, 0,
	PHYS(5,0),	SPDF_NONE,	CARY(AD_MAGM),
	CONFLICT,	A_LAWFUL, PM_CAVEMAN, NON_PM, 2500),

A("The Iron Ball of Liberation", HEAVY_IRON_BALL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),
		(SPFX_DISPL|SPFX_SEARCH|SPFX_WARN), 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_MAGM),
	PHASING,	A_NEUTRAL, PM_PRISONER, NON_PM, 5000),

A("The Staff of Aesculapius",	QUARTERSTAFF,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_DRLI|SPFX_REGEN), 0,0,
	DRLI(0,0),	SPDF_DRAIN,	NO_CARY,
	HEALING,	A_NEUTRAL, PM_HEALER, NON_PM, 5000),

A("The Magic Mirror of Merlin", MIRROR,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK), SPFX_ESP, 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_MAGM),
	0,		A_LAWFUL, PM_KNIGHT, NON_PM, 1500),

A("The Eyes of the Overworld",	LENSES,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_XRAY), 0, 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_MAGM),
	ENLIGHTENING,	A_NEUTRAL,	 PM_MONK, NON_PM, 2500),

A("The Mitre of Holiness",	HELM_OF_BRILLIANCE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DFLAG2|SPFX_INTEL), 0, M2_UNDEAD,
	NO_ATTK,	SPDF_NONE,	CARY(AD_FIRE),
	ENERGY_BOOST,	A_LAWFUL, PM_PRIEST, NON_PM, 2000),

A("The Longbow of Diana", BOW,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_REFLECT), SPFX_ESP, 0,
	PHYS(5,0),	SPDF_NONE,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM, 4000),

A("The Master Key of Thievery", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK),
		(SPFX_WARN|SPFX_TCTRL|SPFX_HPHDAM), 0,
	NO_ATTK,	SPDF_NONE,	NO_CARY,
	UNTRAP,		A_CHAOTIC, PM_ROGUE, NON_PM, 3500),

A("The Tsurugi of Muramasa",	TSURUGI,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_BEHEAD|SPFX_LUCK), 0, 0,
	PHYS(0,8),	SPDF_NONE,	NO_CARY,
	0,		A_LAWFUL, PM_SAMURAI, NON_PM, 4500),

A("The Platinum Yendorian Express Card", CREDIT_CARD,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN),
		(SPFX_ESP|SPFX_HSPDAM), 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_MAGM),
	CHARGE_OBJ,	A_NEUTRAL, PM_TOURIST, NON_PM, 7000),

A("The Orb of Fate",		CRYSTAL_BALL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),
		(SPFX_WARN|SPFX_HSPDAM|SPFX_HPHDAM), 0,
	NO_ATTK,	SPDF_NONE,	NO_CARY,
	LEV_TELE,	A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500),

A("The Eye of the Aethiopica",	AMULET_OF_ESP,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_EREGEN|SPFX_HSPDAM), 0,
	NO_ATTK,	SPDF_NONE,	CARY(AD_MAGM),
	CREATE_PORTAL,	A_NEUTRAL, PM_WIZARD, NON_PM, 4000),

/*
 *  terminator; otyp must be zero
 */
A(null, 0, 0, 0, 0, NO_ATTK, SPDF_NONE, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0)
)
}
