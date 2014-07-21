/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Skills {

/* Much of this code was taken from you.h.  It is now
 * in a separate file so it can be included in objects.c.
 */


/* Code to denote that no skill is applicable */
val P_NONE = 0

/* Weapon Skills -- Stephen White
 * Order matters and are used in macros.
 * Positive values denote hand-to-hand weapons or launchers.
 * Negative values denote ammunition or missiles.
 * Update weapon.c if you ammend any skills.
 * Also used for oc_subtyp.
 */
val P_DAGGER = 1
val P_KNIFE = 2
val P_AXE = 3
val P_PICK_AXE = 4
val P_SHORT_SWORD = 5
val P_BROAD_SWORD = 6
val P_LONG_SWORD = 7
val P_TWO_HANDED_SWORD = 8
val P_SCIMITAR = 9
val P_SABER = 10
val P_CLUB = 11	/* Heavy-shafted bludgeon */
val P_MACE = 12	
val P_MORNING_STAR = 13	/* Spiked bludgeon */
val P_FLAIL = 14	/* Two pieces hinged or chained together */
val P_HAMMER = 15	/* Heavy head on the end */
val P_QUARTERSTAFF = 16	/* Long-shafted bludgeon */
val P_POLEARMS = 17
val P_SPEAR = 18
val P_JAVELIN = 19
val P_TRIDENT = 20
val P_LANCE = 21
val P_BOW = 22
val P_SLING = 23
val P_CROSSBOW = 24
val P_DART = 25
val P_SHURIKEN = 26
val P_BOOMERANG = 27
val P_WHIP = 28
val P_UNICORN_HORN = 29	/* last weapon */
val P_FIRST_WEAPON = P_DAGGER
val P_LAST_WEAPON = P_UNICORN_HORN

/* Spell Skills added by Larry Stewart-Zerba */
val P_ATTACK_SPELL = 30
val P_HEALING_SPELL = 31
val P_DIVINATION_SPELL = 32
val P_ENCHANTMENT_SPELL = 33
val P_CLERIC_SPELL = 34
val P_ESCAPE_SPELL = 35
val P_MATTER_SPELL = 36
val P_FIRST_SPELL = P_ATTACK_SPELL
val P_LAST_SPELL = P_MATTER_SPELL

/* Other types of combat */
val P_BARE_HANDED_COMBAT = 37
val P_MARTIAL_ARTS = P_BARE_HANDED_COMBAT	/* Role distinguishes */
val P_TWO_WEAPON_COMBAT = 38	/* Finally implemented */
val P_RIDING = 39	/* How well you control your steed */
val P_LAST_H_TO_H = P_RIDING
val P_FIRST_H_TO_H = P_BARE_HANDED_COMBAT

val P_NUM_SKILLS = (P_LAST_H_TO_H+1)

/* These roles qualify for a martial arts bonus */
def martial_bonus() = (Role_if(PM_SAMURAI) || Role_if(PM_MONK))


/*
 * These are the standard weapon skill levels.  It is important that
 * the lowest "valid" skill be be 1.  The code calculates the
 * previous amount to practice by calling  practice_needed_to_advance()
 * with the current skill-1.  To work out for the UNSKILLED case,
 * a value of 0 needed.
 */
val P_ISRESTRICTED = 0
val P_UNSKILLED = 1
val P_BASIC = 2
val P_SKILLED = 3
val P_EXPERT = 4
val P_MASTER = 5	/* Unarmed combat/martial arts only */
val P_GRAND_MASTER = 6	/* Unarmed combat/martial arts only */

/**
 * To reach the specified skill level:
 * P_UNSKILLED       0
 * P_BASIC         100
 * P_SKILLED       200
 * P_EXPERT        400
 * P_MASTER        800
 * P_GRAND_MASTER 1600
 */
def practice_needed_to_advance(level: Int) = ((1 << (level-1))*100)

/* The hero's skill in various weapons. */
class skills {
	var skill: xchar = _
	var max_skill: xchar = _
	var advance = 0
}

def P_SKILL(typ: Int)		= (u.weapon_skills(typ).skill)
def P_MAX_SKILL(typ: Int)	= (u.weapon_skills(typ).max_skill)
def P_ADVANCE(typ: Int)		= (u.weapon_skills(typ).advance)
def P_RESTRICTED(typ: Int)	= (u.weapon_skills(typ).skill == P_ISRESTRICTED)

val P_SKILL_LIMIT = 60	/* Max number of skill advancements */

/* Initial skill matrix structure; used in u_init.c and weapon.c */
class def_skill {
	var skill: xchar = _
	var skmax: xchar = _
}
}
