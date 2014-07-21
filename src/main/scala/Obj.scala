/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Global._

object Obj {

// MONOTE: Was a union
class vptrs {
	    var v_nexthere: obj = null	/* floor location lists */
	    var v_ocontainer: obj = null	/* point back to container */
	    var v_ocarry: monst = null	/* point back to carrying monst */
}

class obj {
	var nobj: obj = null
	var v = new vptrs()
def nexthere = v.v_nexthere
def ocontainer = v.v_ocontainer
def ocarry = v.v_ocarry

	var cobj: obj = null	/* contents list for containers */
	var o_id: size_t = _
	var ox,oy: xchar = _
	var otyp: Short = 0		/* object class number */
	var owt = 0
	var quan = 0		/* number of items */

	var spe: schar = _	/* quality of weapon, armor or ring (+ or -)
				   number of charges for wand ( >= -1 )
				   marks your eggs, spinach tins
				   royal coffers for a court ( == 2)
				   tells which fruit a fruit is
				   special for uball and amulet
				   historic and gender for statues
				   real mail, junk mail or hint for scrolls of mail */
val STATUE_HISTORIC = 0x01
val STATUE_MALE = 0x02
val STATUE_FEMALE = 0x04
val MAIL_MAIL = 0
val MAIL_JUNK = 1
val MAIL_HINT = 2
	var oclass: Char = 0	/* object class */
	var invlet: Char = 0	/* designation in inventory */
	var oartifact = 0	/* artifact array index */
	var mysterious_reserved_int = 0	/* so I might add something cool in 3.6.1+ */
	var where: xchar = _	/* where the object thinks it is */
val OBJ_FREE = 0		/* object not attached to anything */
val OBJ_FLOOR = 1		/* object on floor */
val OBJ_CONTAINED = 2		/* object in a container */
val OBJ_INVENT = 3		/* object in the hero's inventory */
val OBJ_MINVENT = 4		/* object in a monster inventory */
val OBJ_MIGRATING = 5		/* object sent off to another level */
val OBJ_BURIED = 6		/* object buried */
val OBJ_ONBILL = 7		/* object on shk bill */
val NOBJ_STATES = 8
	var timed: xchar = _	/* # of fuses (timers) attached to this obj */

	var cursed = false
	var blessed = false
	var unpaid = false	/* on some bill */
	var no_charge = false	/* if shk shouldn't charge for this */
	var known = false	/* exact nature known */
	var dknown = false	/* color or text known */
	var bknown = false	/* blessing or curse known */
	var rknown = false	/* rustproof or not known */

	var oeroded: Byte = 0	/* rusted/burnt weapon/armor */
	var oeroded2: Byte = 0	/* corroded/rotted weapon/armor */
def greatest_erosion(otmp: obj): Int = if(otmp.oeroded > otmp.oeroded2) otmp.oeroded else otmp.oeroded2
val MAX_ERODE = 3
def orotten = oeroded		/* rotten food */
def odiluted = oeroded		/* diluted potions */
def norevive = oeroded2
	var oerodeproof = false /* erodeproof weapon/armor */
	var olocked = false	/* object is locked */
def sokoprize = olocked		/* special flag for sokoban prize */
def orecursive = olocked	/* special flag for preventing recursive calls */
	var obroken = false	/* lock has been broken */
	var otrapped = false	/* container is trapped */
				/* or accidental tripped rolling boulder trap */
def opoisoned = otrapped	/* object (weapon) is coated with poison */

	var recharged: Byte = 0	/* number of times it's been recharged */
	var lamplit = false	/* a light-source -- can be lit */
	var greased = false	/* covered with grease */
	var oattached: Byte = 0	/* obj struct has special attachment */
val OATTACHED_NOTHING = 0
val OATTACHED_MONST = 1	/* monst struct in oextra */
val OATTACHED_M_ID = 2	/* monst id in oextra */
val OATTACHED_UNUSED3 = 3

	var in_use = false	/* for magic items before useup items */
	var bypass = false	/* mark this as an object to be skipped by bhito() */
	var was_thrown = false	/* thrown by the hero since last picked up */
	var odrained = false	/* drained corpse */
	var was_dropped = false /* dropped deliberately by the hero */
	var was_in_starting_inventory = false /* hero started with this item */
	/* 2 free bits */

	var corpsenm = 0	/* type of corpse is mons[corpsenm] */
def leashmon = corpsenm		/* gets m_id of attached pet */
def spestudied = orpsenm	/* # of times a spellbook has been studied */
def fromsink = corpsenm		/* a potion from a sink */

	var record_achieve_special: xchar = _

	var oeaten = 0	/* nutrition left in food, if partly eaten */
	var age = 0		/* creation date */

	var onamelth: uchar = _	/* length of name (following oxlth) */
	var oxlth: Short = 0	/* length of following data */
	/* in order to prevent alignment problems oextra should
	   be (or follow) a long int */
	var owornmask = 0
	var oextra: Object = null /* used for name of ordinary objects - length
				   is flexible; amount for tmp gold objects */
}

// MOTODO #define newobj(xl)	(struct obj *)alloc((unsigned)(xl) + sizeof(struct obj))
def ONAME(otmp: obj) = otmp.oextra.toString

/* Weapons and weapon-tools */
/* KMH -- now based on skill categories.  Formerly:
 *	#define is_sword(otmp)	(otmp.oclass == WEAPON_CLASS && \
 *			 objects(otmp.otyp).oc_wepcat == WEP_SWORD)
 *	#define is_blade(otmp)	(otmp.oclass == WEAPON_CLASS && \
 *			 (objects(otmp.otyp).oc_wepcat == WEP_BLADE || \
 *			  objects(otmp.otyp).oc_wepcat == WEP_SWORD))
 *	#define is_weptool(o)	(o.oclass == TOOL_CLASS && \
 *			 objects[o.otyp].oc_weptool)
 *	#define is_multigen(otyp) (otyp <= SHURIKEN)
 *	#define is_poisonable(otyp) (otyp <= BEC_DE_CORBIN)
 */
def is_blade(otmp: obj) = (otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= P_DAGGER && 
			 objects(otmp.otyp).oc_skill <= P_SABER)
def is_axe(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			 otmp.oclass == TOOL_CLASS) && 
			 objects(otmp.otyp).oc_skill == P_AXE)
def is_pick(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			 otmp.oclass == TOOL_CLASS) && 
			 objects(otmp.otyp).oc_skill == P_PICK_AXE)
def is_crystal_pick(otmp: obj) = (otmp.otyp == CRYSTAL_PICK)
def is_sword(otmp: obj) =	(otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= P_SHORT_SWORD && 
			 objects(otmp.otyp).oc_skill <= P_SABER)
def is_pole(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			otmp.oclass == TOOL_CLASS) && 
			 (objects(otmp.otyp).oc_skill == P_POLEARMS || 
			 objects(otmp.otyp).oc_skill == P_LANCE))
def is_spear(otmp: obj) =	(otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= P_SPEAR && 
			 objects(otmp.otyp).oc_skill <= P_JAVELIN)
def is_launcher(otmp: obj) =	(otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= P_BOW && 
			 objects(otmp.otyp).oc_skill <= P_CROSSBOW)
def is_ammo(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			 otmp.oclass == GEM_CLASS) && 
			 objects(otmp.otyp).oc_skill >= -P_CROSSBOW && 
			 objects(otmp.otyp).oc_skill <= -P_BOW)
def ammo_and_launcher(otmp: obj ,ltmp: obj) =
			 (is_ammo(otmp) && (ltmp != null) && 
			 objects(otmp.otyp).oc_skill == -objects(ltmp.otyp).oc_skill)
def is_missile(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			 otmp.oclass == TOOL_CLASS) && 
			 objects(otmp.otyp).oc_skill >= -P_BOOMERANG && 
			 objects(otmp.otyp).oc_skill <= -P_DART)
def is_weptool(o: obj) = 	(o.oclass == TOOL_CLASS && 
			 objects(o.otyp).oc_skill != P_NONE)
def bimanual(otmp: obj) =	((otmp.oclass == WEAPON_CLASS || 
			 otmp.oclass == TOOL_CLASS) && 
			 objects(otmp.otyp).oc_bimanual)
def is_multigen(otmp: obj) =	(otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= -P_SHURIKEN && 
			 objects(otmp.otyp).oc_skill <= -P_BOW)
def is_poisonable(otmp: obj) =	(otmp.oclass == WEAPON_CLASS && 
			 objects(otmp.otyp).oc_skill >= -P_SHURIKEN && 
			 objects(otmp.otyp).oc_skill <= -P_BOW)
def uslinging()	= (uwep && objects(uwep.otyp).oc_skill == P_SLING)

/* Armor */
def is_shield(otmp: obj) = (otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_SHIELD)
def is_helmet(otmp: obj) = (otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_HELM)
def is_boots(otmp: obj) =	(otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_BOOTS)
def is_gloves(otmp: obj) = (otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_GLOVES)
def is_cloak(otmp: obj) =	(otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_CLOAK)
def is_shirt(otmp: obj) =	(otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_SHIRT)
def is_suit(otmp: obj) =	(otmp.oclass == ARMOR_CLASS && 
			 objects(otmp.otyp).oc_armcat == ARM_SUIT)

/* http://nethackwiki.com/wiki/Unofficial_conduct#Racial */
def is_elven_armor(otmp: obj) =	(otmp.otyp == ELVEN_LEATHER_HELM
				|| otmp.otyp == ELVEN_MITHRIL_COAT
				|| otmp.otyp == ELVEN_CLOAK
				|| otmp.otyp == ELVEN_SHIELD
				|| otmp.otyp == ELVEN_BOOTS)
def is_orcish_armor(otmp: obj) =	(otmp.otyp == ORCISH_HELM
				|| otmp.otyp == ORCISH_CHAIN_MAIL
				|| otmp.otyp == ORCISH_RING_MAIL
				|| otmp.otyp == ORCISH_CLOAK
				|| otmp.otyp == URUK_HAI_SHIELD
				|| otmp.otyp == ORCISH_SHIELD)
def is_dwarvish_armor(otmp: obj) =	(otmp.otyp == DWARVISH_IRON_HELM
				|| otmp.otyp == DWARVISH_MITHRIL_COAT
				|| otmp.otyp == DWARVISH_CLOAK
				|| otmp.otyp == DWARVISH_ROUNDSHIELD)
def is_gnomish_armor(otmp: obj) =	(FALSE)

				
/* Eggs and other food */
val MAX_EGG_HATCH_TIME = 200	/* longest an egg can remain unhatched */
def stale_egg(egg: obj) = ((monstermoves - egg.age) > (2*MAX_EGG_HATCH_TIME))
def ofood(o: obj) = (o.otyp == CORPSE || o.otyp == EGG || o.otyp == TIN)
def polyfodder(obj: obj) = (ofood(obj) && 
			 pm_to_cham(obj.corpsenm) != CHAM_ORDINARY)
def mlevelgain(obj: obj) = (ofood(obj) && obj.corpsenm == PM_WRAITH)
def mhealup(obj: obj) = (ofood(obj) && obj.corpsenm == PM_NURSE)
def drainlevel(corpse: obj) = (mons(corpse.corpsenm).cnutrit*4/5)

/* Containers */
def carried(o: obj) = 	(o.where == OBJ_INVENT)
def mcarried(o: obj) =	(o.where == OBJ_MINVENT)
def Has_contents(o: obj) = (/* (Is_container(o) || o.otyp == STATUE) && */ 
			 o.cobj != null)
def Is_container(o: obj) = (o.otyp >= LARGE_BOX && o.otyp <= BAG_OF_TRICKS)
def Is_box(otmp: obj) =	(otmp.otyp == LARGE_BOX || otmp.otyp == CHEST || 
			 otmp.otyp == IRON_SAFE)
def Is_mbag(otmp: obj) =	(otmp.otyp == BAG_OF_HOLDING || 
			 otmp.otyp == BAG_OF_TRICKS)

def Is_sokoprize(otmp: obj) =	(otmp.sokoprize && !Is_box(otmp))

/* dragon gear */
def Is_dragon_scales(idx: Int) =	(idx >= GRAY_DRAGON_SCALES && 
				 idx <= CHROMATIC_DRAGON_SCALES)
def Is_dragon_mail(idx: Int) =	(idx >= GRAY_DRAGON_SCALE_MAIL && 
				 idx <= CHROMATIC_DRAGON_SCALE_MAIL)
def Is_dragon_armor(idx: Int) =	(Is_dragon_scales(idx) || Is_dragon_mail(idx))
def Is_glowing_dragon_armor(idx: Int) =	(Is_dragon_armor(idx) && 
					 (OBJ_DESCR(objects[idx])) && 
					 (!strncmp(OBJ_DESCR(objects[idx]), "glowing ", 5)))
def Dragon_scales_to_mail(idx: Int) = (idx - GRAY_DRAGON_SCALES + GRAY_DRAGON_SCALE_MAIL)
def Dragon_mail_to_scales(idx: Int) = (idx - GRAY_DRAGON_SCALE_MAIL + GRAY_DRAGON_SCALES)
/*** MOTODO
def Dragon_scales_to_pm(obj: obj) = &mons[PM_GRAY_DRAGON + obj.otyp 
				       - GRAY_DRAGON_SCALES]
def Dragon_mail_to_pm(obj: obj) =	&mons[PM_GRAY_DRAGON + obj.otyp 
				      - GRAY_DRAGON_SCALE_MAIL]
***/
def Dragon_to_scales(pm: Int) = (GRAY_DRAGON_SCALES + (pm - mons))

/* http://nethackwiki.com/wiki/Unofficial_conduct#Racial */
/* Elven gear */
def is_elven_weapon(otmp: obj) =	(otmp.otyp == ELVEN_ARROW
				|| otmp.otyp == ELVEN_SPEAR
				|| otmp.otyp == ELVEN_DAGGER
				|| otmp.otyp == ELVEN_SHORT_SWORD
				|| otmp.otyp == ELVEN_BROADSWORD
				|| otmp.otyp == ELVEN_BOW)
def is_elven_obj(otmp: obj) =	(is_elven_armor(otmp) || is_elven_weapon(otmp))

/* Orcish gear */
def is_orcish_obj(otmp: obj) =	(is_orcish_armor(otmp)
				|| otmp.otyp == ORCISH_ARROW
				|| otmp.otyp == ORCISH_SPEAR
				|| otmp.otyp == ORCISH_DAGGER
				|| otmp.otyp == ORCISH_SHORT_SWORD
				|| otmp.otyp == ORCISH_BOW)

/* Dwarvish gear */
def is_dwarvish_obj(otmp: obj) =	(is_dwarvish_armor(otmp)
				|| otmp.otyp == DWARVISH_SPEAR
				|| otmp.otyp == DWARVISH_SHORT_SWORD
				|| otmp.otyp == DWARVISH_MATTOCK)

/* Gnomish gear */
def is_gnomish_obj(otmp: obj) =	(is_gnomish_armor(otmp))

/* Light sources */
def Is_candle(otmp: obj) = (otmp.otyp == TALLOW_CANDLE || 
			 otmp.otyp == WAX_CANDLE)
val MAX_OIL_IN_FLASK = 400	/* maximum amount of oil in a potion of oil */

/* MAGIC_LAMP intentionally excluded below */
/* age field of this is relative age rather than absolute */
def age_is_relative(otmp: obj) = (otmp.otyp == BRASS_LANTERN
				|| otmp.otyp == OIL_LAMP
				|| otmp.otyp == CANDELABRUM_OF_INVOCATION
				|| otmp.otyp == TALLOW_CANDLE
				|| otmp.otyp == WAX_CANDLE
				|| otmp.otyp == POT_OIL)
/* object can be ignited */
def ignitable(otmp: obj) = (otmp.otyp == BRASS_LANTERN
				|| otmp.otyp == OIL_LAMP
				|| otmp.otyp == CANDELABRUM_OF_INVOCATION
				|| otmp.otyp == TALLOW_CANDLE
				|| otmp.otyp == WAX_CANDLE
				|| otmp.otyp == POT_OIL)

/* unique items */
def is_unique(obj: obj) = 	(obj.otyp == AMULET_OF_YENDOR
				|| obj.otyp == BELL_OF_OPENING
				|| obj.otyp == CANDELABRUM_OF_INVOCATION
				|| obj.otyp == SPE_BOOK_OF_THE_DEAD)

/* special stones */
def is_graystone(obj: obj) =	(obj.otyp == LUCKSTONE || 
				 obj.otyp == LOADSTONE || 
				 obj.otyp == FLINT     || 
				 obj.otyp == TOUCHSTONE)

/* misc */
def is_flimsy(otmp: obj) = (objects(otmp.otyp).oc_material <= LEATHER || 
				 otmp.otyp == RUBBER_HOSE)
def oresist_disintegration(obj: obj) = 
		(objects(obj.otyp).oc_oprop == DISINT_RES || 
		 obj_resists(obj, 5, 50) || is_quest_artifact(obj) )
/***
MOTODO: Was a macro that modifies (i) in place
def weight_dmg(i: Int) = { i = if(i<=100) 1 else i/100; 
  i = rnd(i);
  if(i > 6) i = 6;
}
***/

/* helpers, simple enough to be macros */
def is_plural(o: obj) = (o.quan > 1 || o.oartifact == ART_EYES_OF_THE_OVERWORLD)

/* Flags for get_obj_location(). */
val CONTAINED_TOO = 0x1
val BURIED_TOO = 0x2
}
