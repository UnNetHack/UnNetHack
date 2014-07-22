/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import Config._
import PerMonst._
import Prop._

object Artifact {

val SPFX_NONE = 0x0000000	/* no special effects, just a bonus */
val SPFX_NOGEN = 0x0000001	/* item is special, bequeathed by gods */
val SPFX_RESTR = 0x0000002	/* item is restricted - can't be named */
val SPFX_INTEL = 0x0000004	/* item is self-willed - intelligent */
val SPFX_SPEAK = 0x0000008	/* item can speak (not implemented) */
val SPFX_SEEK = 0x0000010	/* item helps you search for things */
val SPFX_WARN = 0x0000020	/* item warns you of danger */
val SPFX_ATTK = 0x0000040	/* item has a special attack (attk) */
val SPFX_DEFN = 0x0000080	/* item has a special defence (defn) */
val SPFX_DRLI = 0x0000100	/* drains a level from monsters */
val SPFX_SEARCH = 0x0000200	/* helps searching */
val SPFX_BEHEAD = 0x0000400	/* beheads monsters */
val SPFX_HALRES = 0x0000800	/* blocks hallucinations */
val SPFX_ESP = 0x0001000	/* ESP (like amulet of ESP) */
val SPFX_DISPL = 0x0002000	/* Displacement */
val SPFX_REGEN = 0x0004000	/* Regeneration */
val SPFX_EREGEN = 0x0008000	/* Energy Regeneration */
val SPFX_HSPDAM = 0x0010000	/* 1/2 spell damage (on player) in combat */
val SPFX_HPHDAM = 0x0020000	/* 1/2 physical damage (on player) in combat */
val SPFX_TCTRL = 0x0040000	/* Teleportation Control */
val SPFX_LUCK = 0x0080000	/* Increase Luck (like Luckstone) */
val SPFX_DMONS = 0x0100000	/* attack bonus on one monster type */
val SPFX_DCLAS = 0x0200000	/* attack bonus on monsters w/ symbol mtype */
val SPFX_DFLAG1 = 0x0400000	/* attack bonus on monsters w/ mflags1 flag */
val SPFX_DFLAG2 = 0x0800000	/* attack bonus on monsters w/ mflags2 flag */
val SPFX_DALIGN = 0x1000000	/* attack bonus on non-aligned monsters  */
val SPFX_DBONUS = 0x1F00000	/* attack bonus mask */
val SPFX_XRAY = 0x2000000	/* gives X-RAY vision to player */
val SPFX_REFLECT = 0x4000000 /* Reflection */
val SPFX_WARN_S = 0x8000000	/* item warns you of danger of S_MONSTER */
val SPFX_STR = 0x10000000   /* item bestows STR 18(**) */
val SPFX_CON = 0x20000000   /* item bestows CON 25 */
val SPFX_POLYC = 0x40000000	 /* items grants polymorph control */

val SPDF_NONE = 0x00000000 /* No special effects */
val SPDF_MAGIC = 0x00000001 /* Magic resistance */
val SPDF_FIRE = 0x00000002 /* Fire defense */
val SPDF_COLD = 0x00000004 /* Cold defense */
val SPDF_SLEEP = 0x00000008 /* Sleep defense */
val SPDF_DISINT = 0x00000010 /* Disintegration defense */
val SPDF_ELEC = 0x00000020 /* Shock defense */
val SPDF_POISON = 0x00000040 /* Poison defense */
val SPDF_ACID = 0x00000080 /* Acid defense */
val SPDF_BLIND = 0x00000100 /* Blinding resistance */
val SPDF_WERE = 0x00000200 /* Lycanthropy resistance */
val SPDF_DRAIN = 0x00000400 /* Drain level defense */
val SPDF_CONFUSE = 0x00000800 /* Confusion */
val SPDF_STUN = 0x00001000 /* Stunning */

class artifact(
	var otyp: Short,
	var name: String,
	var spfx: Int,	/* special effect from wielding/wearing */
	var cspfx: Int,	/* special effect just from carrying obj */
	var mtype: Long,	/* monster type, symbol, or flag */
	var attk: attack,
	var defn: Int,
	var cary: attack,
	var inv_prop: uchar,	/* property obtained by invoking artifact */
	var alignment: aligntyp,	/* alignment of bequeathing gods */
	var role: Short,	/* character role associated with */
	var race: Short,	/* character race associated with */
	var cost: Int) {	/* price when sold to hero (default 100 x base cost) */
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
