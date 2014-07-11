/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Pm._

object PerMonst {

/*	This structure covers all attack forms.
 *	aatyp is the gross attack type (eg. claw, bite, breath, ...)
 *	adtyp is the damage type (eg. physical, fire, cold, spell, ...)
 *	damn is the number of hit dice of damage from the attack.
 *	damd is the number of sides on each die.
 *
 *	Some attacks can do no points of damage.  Additionally, some can
 *	have special effects *and* do damage as well.  If damn and damd
 *	are set, they may have a special meaning.  For example, if set
 *	for a blinding attack, they determine the amount of time blinded.
 */

class attack(var aatyp: uchar, var adtyp: uchar, var damn: uchar, var damd: uchar) {
}

/*	Max # of attacks for any given monster.
 */

val NATTK = 6

/*	Weight of a human body
 */

val WT_HUMAN = 1450

class permonst {
	var 	mname: String = null			/* full name */
	var	mlet = '\0'				/* symbol */
	var	unicode_codepoint: glyph_t = null	/* unicode codepoint for utf8graphics */
	var	mlevel: schar = 0			/* base monster level */
	var	mmove: schar = 0			/* move speed */
	var	ac: schar = 0				/* (base) armor class */
	var	mr: schar = 0				/* (base) magic resistance */
	var	maligntyp: aligntyp = A_NONE		/* basic monster alignment */
	var	geno = 0				/* creation/geno mask value */
	var	mattk = new Array[attack](NATTK)	/* attacks matrix */
	var	cwt = 0					/* weight of corpse */
	var	cnutrit = 0				/* its nutritional value */
	var	pxlth: Short = 0			/* length of extension */
	var	msound: uchar = 0			/* noise it makes (6 bits) */
	var	msize: uchar = 0			/* physical size (3 bits) */
	var	mresists: uchar = 0			/* resistances */
	var	mconveys: uchar = 0			/* conveyed by eating */
	var	mflags1 = 0				/* boolean bitflags */
	var	mflags2 = 0				/* more boolean bitflags */
	var	mflags3	= 0 				/* yet more boolean bitflags */
	var	mcolor: uchar = 0			/* color to use */
}

val VERY_SLOW = 3
val SLOW_SPEED = 9
val NORMAL_SPEED = 12 /* movement rates */
val FAST_SPEED = 15
val VERY_FAST = 24

val NON_PM = PM_PLAYERMON		/* "not a monster" */
val LOW_PM = (NON_PM+1)		/* first monster in mons[] */
val SPECIAL_PM = PM_LONG_WORM_TAIL	/* [normal] < ~ < [special] */
	/* mons[SPECIAL_PM] through mons[NUMMONS-1], inclusive, are
	   never generated randomly and cannot be polymorphed into */
}
