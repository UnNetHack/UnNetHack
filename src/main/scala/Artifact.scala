/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align.A_NONE
import Config._
import PerMonst._

object Artifact {

val SPFX_NONE = 0x0000000L	/* no special effects, just a bonus */
val SPFX_NOGEN = 0x0000001L	/* item is special, bequeathed by gods */
val SPFX_RESTR = 0x0000002L	/* item is restricted - can't be named */
val SPFX_INTEL = 0x0000004L	/* item is self-willed - intelligent */
val SPFX_SPEAK = 0x0000008L	/* item can speak (not implemented) */
val SPFX_SEEK = 0x0000010L	/* item helps you search for things */
val SPFX_WARN = 0x0000020L	/* item warns you of danger */
val SPFX_ATTK = 0x0000040L	/* item has a special attack (attk) */
val SPFX_DEFN = 0x0000080L	/* item has a special defence (defn) */
val SPFX_DRLI = 0x0000100L	/* drains a level from monsters */
val SPFX_SEARCH = 0x0000200L	/* helps searching */
val SPFX_BEHEAD = 0x0000400L	/* beheads monsters */
val SPFX_HALRES = 0x0000800L	/* blocks hallucinations */
val SPFX_ESP = 0x0001000L	/* ESP (like amulet of ESP) */
val SPFX_DISPL = 0x0002000L	/* Displacement */
val SPFX_REGEN = 0x0004000L	/* Regeneration */
val SPFX_EREGEN = 0x0008000L	/* Energy Regeneration */
val SPFX_HSPDAM = 0x0010000L	/* 1/2 spell damage (on player) in combat */
val SPFX_HPHDAM = 0x0020000L	/* 1/2 physical damage (on player) in combat */
val SPFX_TCTRL = 0x0040000L	/* Teleportation Control */
val SPFX_LUCK = 0x0080000L	/* Increase Luck (like Luckstone) */
val SPFX_DMONS = 0x0100000L	/* attack bonus on one monster type */
val SPFX_DCLAS = 0x0200000L	/* attack bonus on monsters w/ symbol mtype */
val SPFX_DFLAG1 = 0x0400000L	/* attack bonus on monsters w/ mflags1 flag */
val SPFX_DFLAG2 = 0x0800000L	/* attack bonus on monsters w/ mflags2 flag */
val SPFX_DALIGN = 0x1000000L	/* attack bonus on non-aligned monsters  */
val SPFX_DBONUS = 0x1F00000L	/* attack bonus mask */
val SPFX_XRAY = 0x2000000L	/* gives X-RAY vision to player */
val SPFX_REFLECT = 0x4000000L /* Reflection */
val SPFX_WARN_S = 0x8000000L	/* item warns you of danger of S_MONSTER */
val SPFX_STR = 0x10000000L   /* item bestows STR 18(**) */
val SPFX_CON = 0x20000000L   /* item bestows CON 25 */
val SPFX_POLYC = 0x40000000L	 /* items grants polymorph control */

val SPDF_NONE = 0x00000000L /* No special effects */
val SPDF_MAGIC = 0x00000001L /* Magic resistance */
val SPDF_FIRE = 0x00000002L /* Fire defense */
val SPDF_COLD = 0x00000004L /* Cold defense */
val SPDF_SLEEP = 0x00000008L /* Sleep defense */
val SPDF_DISINT = 0x00000010L /* Disintegration defense */
val SPDF_ELEC = 0x00000020L /* Shock defense */
val SPDF_POISON = 0x00000040L /* Poison defense */
val SPDF_ACID = 0x00000080L /* Acid defense */
val SPDF_BLIND = 0x00000100L /* Blinding resistance */
val SPDF_WERE = 0x00000200L /* Lycanthropy resistance */
val SPDF_DRAIN = 0x00000400L /* Drain level defense */
val SPDF_CONFUSE = 0x00000800L /* Confusion */
val SPDF_STUN = 0x00001000L /* Stunning */


class artifact {
	var otyp: Short = 0
	var name: String = null
	var spfx = 0L	/* special effect from wielding/wearing */
	var cspfx = 0L	/* special effect just from carrying obj */
	var mtype = 0L	/* monster type, symbol, or flag */
	var attk = new attack()
	var defn = 0L
	var cary = new attack()
	var inv_prop: uchar = 0	/* property obtained by invoking artifact */
	var alignment  = A_NONE	/* alignment of bequeathing gods */
	var role: Short = 0	/* character role associated with */
	var race: Short = 0	/* character race associated with */
	var cost = 0	/* price when sold to hero (default 100 x base cost) */
}

/* invoked properties with special powers */
def TAMING		= LAST_PROP+1
def HEALING		= LAST_PROP+2
def ENERGY_BOOST	= LAST_PROP+3
def UNTRAP		= LAST_PROP+4
def CHARGE_OBJ	= LAST_PROP+5
def LEV_TELE	= LAST_PROP+6
def CREATE_PORTAL	= LAST_PROP+7
def ENLIGHTENING	= LAST_PROP+8
def CREATE_AMMO	= LAST_PROP+9
def PHASING		= LAST_PROP+10
def SMOKE_CLOUD	= LAST_PROP+11
}
