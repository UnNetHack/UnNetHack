/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object MonAttk {

/*	Add new attack types below - ordering affects experience (exper.c).
 *	Attacks > AT_BUTT are worth extra experience.
 */
val AT_ANY = -1	/* fake attack; dmgtype_fromattack wildcard */
val AT_NONE = 0	/* passive monster (ex. acid blob) */
val AT_CLAW = 1	/* claw (punch, hit, etc.) */
val AT_BITE = 2	/* bite */
val AT_KICK = 3	/* kick */
val AT_BUTT = 4	/* head butt (ex. a unicorn) */
val AT_TUCH = 5	/* touches */
val AT_STNG = 6	/* sting */
val AT_HUGS = 7	/* crushing bearhug */
val AT_SPIT = 10	/* spits substance - ranged */
val AT_ENGL = 11	/* engulf (swallow or by a cloud) */
val AT_BREA = 12	/* breath - ranged */
val AT_EXPL = 13	/* explodes - proximity */
val AT_BOOM = 14	/* explodes when killed */
val AT_GAZE = 15	/* gaze - ranged */
val AT_TENT = 16	/* tentacles */
val AT_SCRE = 17	/* scream - sonic attack */

val AT_WEAP = 254	/* uses weapon */
val AT_MAGC = 255	/* uses magic spell(s) */

/*	Add new damage types below.
 *
 *	Note that 1-10 correspond to the types of attack used in buzz().
 *	Please don't disturb the order unless you rewrite the buzz() code.
 */
val AD_ANY = -1	/* fake damage; attacktype_fordmg wildcard */
val AD_PHYS = 0	/* ordinary physical */
val AD_MAGM = 1	/* magic missiles */
val AD_FIRE = 2	/* fire damage */
val AD_COLD = 3	/* frost damage */
val AD_SLEE = 4	/* sleep ray */
val AD_DISN = 5	/* disintegration (death ray) */
val AD_ELEC = 6	/* shock damage */
val AD_DRST = 7	/* drains str (poison) */
val AD_LAVA = 8	/* a jet of molten lava */
val AD_ACID = 9	/* acid damage */
val AD_SPC1 = 10	/* for extension of buzz() */
val AD_SPC2 = 11	/* for extension of buzz() */
val AD_BLND = 12	/* blinds (yellow light) */
val AD_STUN = 13	/* stuns */
val AD_SLOW = 14	/* slows */
val AD_PLYS = 15	/* paralyses */
val AD_DRLI = 16	/* drains life levels (Vampire) */
val AD_DREN = 17	/* drains magic energy */
val AD_LEGS = 18	/* damages legs (xan) */
val AD_STON = 19	/* petrifies (Medusa, cockatrice) */
val AD_STCK = 20	/* sticks to you (mimic) */
val AD_SGLD = 21	/* steals gold (leppie) */
val AD_SITM = 22	/* steals item (nymphs) */
val AD_SEDU = 23	/* seduces & steals multiple items */
val AD_TLPT = 24	/* teleports you (Quantum Mech.) */
val AD_RUST = 25	/* rusts armour (Rust Monster)*/
val AD_CONF = 26	/* confuses (Umber Hulk) */
val AD_DGST = 27	/* digests opponent (trapper, etc.) */
val AD_HEAL = 28	/* heals opponent's wounds (nurse) */
val AD_WRAP = 29	/* special "stick" for eels */
val AD_WERE = 30	/* confers lycanthropy */
val AD_DRDX = 31	/* drains dexterity (quasit) */
val AD_DRCO = 32	/* drains constitution */
val AD_DRIN = 33	/* drains intelligence (mind flayer) */
val AD_DISE = 34	/* confers diseases */
val AD_DCAY = 35	/* decays organics (brown Pudding) */
val AD_SSEX = 36	/* Succubus seduction (extended) */
				/* If no SEDUCE then same as AD_SEDU */
val AD_HALU = 37	/* causes hallucination */
val AD_DETH = 38	/* for Death only */
val AD_PEST = 39	/* for Pestilence only */
val AD_FAMN = 40	/* for Famine only */
val AD_SLIM = 41	/* turns you into green slime */
val AD_ENCH = 42	/* remove enchantment (disenchanter) */
val AD_CORR = 43	/* corrode armor (black pudding) */
val AD_LUCK = 44	/* drain luck (evil eye) */
val AD_FREZ = 45	/* freezing attack (blue slime) */
val AD_HEAD = 46	/* decapitate (vorpal jabberwock) */

val AD_PUNI = 47	/* punisher spells */

val AD_LVLT = 48	/* level teleport (weeping angel) */
val AD_BLNK = 49	/* mental invasion (weeping angel) */

val AD_SPOR = 50	/* generate spore */

val AD_CLRC = 240	/* random clerical spell */
val AD_SPEL = 241	/* random magic spell */
val AD_RBRE = 242	/* random breath weapon */

val AD_SAMU = 252	/* hits, may steal Amulet (Wizard) */
val AD_CURS = 253	/* random curse (ex. gremlin) */


/*
 *  Monster to monster attacks.  When a monster attacks another (mattackm),
 *  any or all of the following can be returned.  See mattackm() for more
 *  details.
 */
val MM_MISS = 0x0	/* aggressor missed */
val MM_HIT = 0x1	/* aggressor hit defender */
val MM_DEF_DIED = 0x2	/* defender died */
val MM_AGR_DIED = 0x4	/* aggressor died */
}
