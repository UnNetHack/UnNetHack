/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Prop {
/*** What the properties are ***/
val FIRE_RES = 1
val COLD_RES = 2
val SLEEP_RES = 3
val DISINT_RES = 4
val SHOCK_RES = 5
val POISON_RES = 6
val ACID_RES = 7
val STONE_RES = 8
/* note: for the first eight properties, MR_xxx == (1 << (xxx_RES - 1)) */
val ADORNED = 9
val REGENERATION = 10
val SEARCHING = 11
val SEE_INVIS = 12
val INVIS = 13
val TELEPORT = 14
val TELEPORT_CONTROL = 15
val POLYMORPH = 16
val POLYMORPH_CONTROL = 17
val LEVITATION = 18
val STEALTH = 19
val AGGRAVATE_MONSTER = 20
val CONFLICT = 21
val PROTECTION = 22
val PROT_FROM_SHAPE_CHANGERS = 23
val WARNING = 24
val TELEPAT = 25
val FAST = 26
val STUNNED = 27
val CONFUSION = 28
val SICK = 29
val BLINDED = 30
val SLEEPING = 31
val WOUNDED_LEGS = 32
val STONED = 33
val STRANGLED = 34
val HALLUC = 35
val HALLUC_RES = 36
val FUMBLING = 37
val JUMPING = 38
val WWALKING = 39
val HUNGER = 40
val GLIB = 41
val REFLECTING = 42
val LIFESAVED = 43
val ANTIMAGIC = 44
val DISPLACED = 45
val CLAIRVOYANT = 46
val VOMITING = 47
val ENERGY_REGENERATION = 48
val MAGICAL_BREATHING = 49
val HALF_SPDAM = 50
val HALF_PHDAM = 51
val SICK_RES = 52
val DRAIN_RES = 53
val WARN_UNDEAD = 54
val INVULNERABLE = 55
val FREE_ACTION = 56
val SWIMMING = 57
val SLIMED = 58
val FIXED_ABIL = 59
val FLYING = 60
val UNCHANGING = 61
val PASSES_WALLS = 62
val SLOW_DIGESTION = 63
val INFRAVISION = 64
val WARN_OF_MON = 65
val DETECT_MONSTERS = 66
val LAST_PROP = DETECT_MONSTERS

/*** Where the properties come from ***/
/* Definitions were moved here from obj.h and you.h */
class prop {
	/*** Properties conveyed by objects ***/
	var extrinsic = 0
	/* Armor */
val W_ARM = 0x00000001L /* Body armor */
val W_ARMC = 0x00000002L /* Cloak */
val W_ARMH = 0x00000004L /* Helmet/hat */
val W_ARMS = 0x00000008L /* Shield */
val W_ARMG = 0x00000010L /* Gloves/gauntlets */
val W_ARMF = 0x00000020L /* Footwear */
val W_ARMU = 0x00000040L /* Undershirt */
#	define W_ARMOR	     (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
	/* Weapons and artifacts */
val W_WEP = 0x00000100L /* Wielded weapon */
val W_QUIVER = 0x00000200L /* Quiver for (f)iring ammo */
val W_SWAPWEP = 0x00000400L /* Secondary weapon */
val W_ART = 0x00001000L /* Carrying artifact (not really worn) */
val W_ARTI = 0x00002000L /* Invoked artifact  (not really worn) */
	/* Amulets, rings, tools, and other items */
val W_AMUL = 0x00010000L /* Amulet */
val W_RINGL = 0x00020000L /* Left ring */
val W_RINGR = 0x00040000L /* Right ring */
val W_RING = (W_RINGL | W_RINGR)
val W_TOOL = 0x00080000L /* Eyewear */
val W_SADDLE = 0x00100000L	/* KMH -- For riding monsters */
val W_BALL = 0x00200000L /* Punishment ball */
val W_CHAIN = 0x00400000L /* Punishment chain */

	/*** Property is blocked by an object ***/
	var blocked = 0					/* Same assignments as extrinsic */

	/*** Timeouts, permanent properties, and other flags ***/
	var intrinsic = 0
	/* Timed properties */
val TIMEOUT = 0x00ffffffL /* Up to 16 million turns */
	/* Permanent properties */
val FROMEXPER = 0x01000000L /* Gain/lose with experience, for role */
val FROMRACE = 0x02000000L /* Gain/lose with experience, for race */
val FROMOUTSIDE = 0x04000000L /* By corpses, prayer, thrones, etc. */
val INTRINSIC = (FROMOUTSIDE|FROMRACE|FROMEXPER)
	/* Control flags */
val I_SPECIAL = 0x10000000L /* Property is controllable */
}

/*** Definitions for backwards compatibility ***/
val LEFT_RING = W_RINGL
val RIGHT_RING = W_RINGR
val LEFT_SIDE = LEFT_RING
val RIGHT_SIDE = RIGHT_RING
val BOTH_SIDES = (LEFT_SIDE | RIGHT_SIDE)
val WORN_ARMOR = W_ARM
val WORN_CLOAK = W_ARMC
val WORN_HELMET = W_ARMH
val WORN_SHIELD = W_ARMS
val WORN_GLOVES = W_ARMG
val WORN_BOOTS = W_ARMF
val WORN_AMUL = W_AMUL
val WORN_BLINDF = W_TOOL
val WORN_SHIRT = W_ARMU
}
