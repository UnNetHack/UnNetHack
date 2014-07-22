/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Global._

object ObjClass {
/* definition of a class of objects */

class objclass {
	var oc_name_idx: Short = 0		/* index of actual name */
	var oc_descr_idx: Short = 0		/* description when name unknown */
	var oc_uname: String = null		/* called by user */
	var oc_name_known = false
	var oc_merge = false	/* merge otherwise equal objects */
	var oc_uses_known = false /* obj->known affects full decription */
				/* otherwise, obj->dknown and obj->bknown */
				/* tell all, and obj->known should always */
				/* be set for proper merging behavior */
	var oc_pre_discovered = false	/* Already known at start of game; */
					/* won't be listed as a discovery. */
	var oc_magic = false	/* inherently magical object */
	var oc_charged = false /* may have +n or (n) charges */
	var oc_unique = false	/* special one-of-a-kind object */
	var oc_nowish = false	/* cannot wish for this object */

	var oc_big = false
def oc_bimanual	= oc_big	/* for weapons & tools used as weapons */
def oc_bulky	= oc_big	/* for armor */
	var oc_tough = false	/* hard gems/rings */

	var oc_dir: Byte = 0
val NODIR = 1	/* for wands/spells: non-directional */
val IMMEDIATE = 2	/*		     directional */
val RAY = 3	/*		     zap beams */

val PIERCE = 1	/* for weapons & tools used as weapons */
val SLASH = 2	/* (latter includes iron ball & chain) */
val WHACK = 0

	/*Bitfield(oc_subtyp,3);*/	/* Now too big for a bitfield... see below */

	var oc_material: Byte = 0
val LIQUID = 1	/* currently only for venom */
val WAX = 2
val VEGGY = 3	/* foodstuffs */
val FLESH = 4	/*   ditto    */
val PAPER = 5
val CLOTH = 6
val LEATHER = 7
val WOOD = 8
val BONE = 9
val DRAGON_HIDE = 10	/* not leather! */
val IRON = 11	/* Fe - includes steel */
val METAL = 12	/* Sn, &c. */
val COPPER = 13	/* Cu - includes brass */
val SILVER = 14	/* Ag */
val GOLD = 15	/* Au */
val PLATINUM = 16	/* Pt */
val MITHRIL = 17
val PLASTIC = 18
val GLASS = 19
val GEMSTONE = 20
val MINERAL = 21

def is_organic(otmp: obj) =	(objects(otmp.otyp).oc_material <= WOOD)
def is_metallic(otmp: obj) =	(objects(otmp.otyp).oc_material >= IRON && 
				 objects(otmp.otyp).oc_material <= MITHRIL)
def is_iron(otmp: obj) =		(objects(otmp.otyp).oc_material == IRON)
def is_silver(otmp: obj) =		(objects(otmp.otyp).oc_material == SILVER)

/* primary damage: fire/rust/--- */
/* is_flammable(otmp: obj) =, is_rottable(otmp) in mkobj.c */
def is_rustprone(otmp: obj) =	(objects(otmp.otyp).oc_material == IRON)

/* secondary damage: rot/acid/acid */
def is_corrodeable(otmp: obj) =	(objects(otmp.otyp).oc_material == COPPER || objects(otmp.otyp).oc_material == IRON)

def is_damageable(otmp: obj) = (is_rustprone(otmp) || is_flammable(otmp) ||
				is_rottable(otmp) || is_corrodeable(otmp))

	var oc_subtyp: schar = _
def oc_skill	= oc_subtyp   /* Skills of weapons, spellbooks, tools, gems */
def oc_armcat	= oc_subtyp   /* for armor */

	var oc_oprop: uchar = _		/* property (invis, &c.) conveyed */
	var oc_class: Char = 0		/* object class */
	var unicode_codepoint: glyph_t = _	/* unicode codepoint for utf8graphics */
	var oc_delay: schar = _		/* delay when using such an object */
	var oc_color: uchar = _		/* color of the object */

	var oc_prob: Short = 0		/* probability, used in mkobj() */
	var oc_weight = 0	/* encumbrance (1 cn = 0.1 lb.) */
	var oc_cost: Short = 0		/* base cost in shops */
/* Check the AD&D rules!  The FIRST is small monster damage. */
/* for weapons, and tools, rocks, and gems useful as weapons */
	var oc_wsdam, oc_wldam: schar = _	/* max small/large monster damage */
	var oc_oc1, oc_oc2: schar = _
def oc_hitbon	= oc_oc1		/* weapons: "to hit" bonus */

def a_ac	=	oc_oc1	/* armor class, used in ARM_BONUS in do.c */
def a_can	=	oc_oc2		/* armor: used in mhitu.c */
def level=	oc_oc2		/* books: spell level */

	var oc_nutrition = 0	/* food value */
}

val ARM_SHIELD = 1	/* needed for special wear function */
val ARM_HELM = 2
val ARM_GLOVES = 3
val ARM_BOOTS = 4
val ARM_CLOAK = 5
val ARM_SHIRT = 6
val ARM_SUIT = 0

class objdescr {
	var oc_name: String = null		/* actual name */
	var oc_descr: String = null		/* description when name unknown */
}

/*
 * All objects have a class. Make sure that all classes have a corresponding
 * symbol below.
 */
val RANDOM_CLASS = 0	/* used for generating random objects */
val ILLOBJ_CLASS = 1
val WEAPON_CLASS = 2
val ARMOR_CLASS = 3
val RING_CLASS = 4
val AMULET_CLASS = 5
val TOOL_CLASS = 6
val FOOD_CLASS = 7
val POTION_CLASS = 8
val SCROLL_CLASS = 9
val SPBOOK_CLASS = 10	/* actually SPELL-book */
val WAND_CLASS = 11
val COIN_CLASS = 12
val GEM_CLASS = 13
val ROCK_CLASS = 14
val BALL_CLASS = 15
val CHAIN_CLASS = 16
val VENOM_CLASS = 17
val MAXOCLASSES = 18

val ALLOW_COUNT	= (MAXOCLASSES+1) /* Can be used in the object class */
val ALL_CLASSES	= (MAXOCLASSES+2) /* input to getobj().		   */
val ALLOW_NONE = (MAXOCLASSES+3) /*				   */

val BURNING_OIL = (MAXOCLASSES+1) /* Can be used as input to explode. */
val MON_EXPLODE	= (MAXOCLASSES+2) /* Exploding monster (e.g. gas spore) */

/* Default definitions of all object-symbols (must match classes above). */

val ILLOBJ_SYM = ']'	/* also used for mimics */
val WEAPON_SYM = ')'
val ARMOR_SYM = '['
val RING_SYM = '='
val AMULET_SYM = '"'
val TOOL_SYM = '('
val FOOD_SYM = '%'
val POTION_SYM = '!'
val SCROLL_SYM = '?'
val SPBOOK_SYM = '+'
val WAND_SYM = '/'
val GOLD_SYM = '$'
val GEM_SYM = '*'
val ROCK_SYM = '`'
val BALL_SYM = '0'
val CHAIN_SYM = '_'
val VENOM_SYM = '.'

class fruit {
	var fname: String = null
	var fid = 0
	var nextf: fruit = null
}
def newfruit() = new fruit()
// MOTODO #define dealloc_fruit(rind) free((genericptr_t) (rind))

def OBJ_NAME(obj: objclass) =  obj_descrr(obj.oc_name_idx).oc_name
def OBJ_DESCR(obj: objclass) = obj_descr(obj.oc_descr_idx).oc_descr
}
