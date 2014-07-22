/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Hack._
import ONames._

object UInit {

class trobj(trotyp: Short, trspe: schar, trclass: Char, trquan: Byte, trbless: Byte) {
	this(trotyp: Int, trspe: Int, trclass: Int, trquan: Int, trbless: Int) = this(trotyp.toShort, trspe.toByte, trclass.toByte, trquan.toByte, trbless.toByte)
}

val UNDEF_TYP = 0
val UNDEF_SPE = '\177'
val UNDEF_BLESS = 2

/*
 *	Initial inventory for the various roles.
 */

var Archeologist: List[trobj] = List(
	/* if adventure has a name...  idea from tan@uvm-gen */
	new trobj(BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(FOOD_RATION, 0, FOOD_CLASS, 3, 0),
	new trobj(PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS),
	new trobj(TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS),
	new trobj(TOUCHSTONE, 0, GEM_CLASS, 1, 0),
	new trobj(SACK, 0, TOOL_CLASS, 1, 0),

	new trobj(0, 0, 0, 0, 0)
)

val B_MAJOR = 0	/* two-handed sword or battle-axe  */
val B_MINOR = 1	/* matched with axe or short sword */
val Barbarian: List[trobj] = List(
	new trobj(TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(FOOD_RATION, 0, FOOD_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)

val C_AMMO = 2
val Cave_man: List[trobj] = List(
	new trobj(CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(SLING, 2, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(FLINT, 0, GEM_CLASS, 15, UNDEF_BLESS),	/* quan is variable */
	new trobj(ROCK, 0, GEM_CLASS, 3, 0),			/* yields 18..33 */
	new trobj(LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(0, 0, 0, 0, 0)
)
val Convict: List[trobj] = List(
	new trobj(ROCK, 0, GEM_CLASS, 1, 0),
	new trobj(STRIPED_SHIRT, 0, ARMOR_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Healer: List[trobj] = List(
	new trobj(SCALPEL, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(STETHOSCOPE, 0, TOOL_CLASS, 1, 0),
	new trobj(POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS),
	new trobj(POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS),
	new trobj(WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS),
	/* always blessed, so it's guaranteed readable */
	new trobj(SPE_HEALING, 0, SPBOOK_CLASS, 1, 1),
	new trobj(SPE_EXTRA_HEALING, 0, SPBOOK_CLASS, 1, 1),
	new trobj(SPE_STONE_TO_FLESH, 0, SPBOOK_CLASS, 1, 1),
	new trobj(APPLE, 0, FOOD_CLASS, 5, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Knight: List[trobj] = List(
	new trobj(LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(LANCE, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(RING_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(APPLE, 0, FOOD_CLASS, 10, 0),
	new trobj(CARROT, 0, FOOD_CLASS, 10, 0),
	new trobj(0, 0, 0, 0, 0)
)
val M_BOOK = 2
val Monk: List[trobj] = List(
	new trobj(LEATHER_GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1),
	new trobj(UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS),
	new trobj(POT_HEALING, 0, POTION_CLASS, 3, UNDEF_BLESS),
	new trobj(FOOD_RATION, 0, FOOD_CLASS, 3, 0),
	new trobj(APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS),
	new trobj(ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS),
	/* Yes, we know fortune cookies aren't really from China.  They were
	 * invented by George Jung in Los Angeles, California, USA in 1916.
	 */
	new trobj(FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS),
	new trobj(0, 0, 0, 0, 0)
)
val PRI_MACE = 0
val Priest: List[trobj] = List(
	new trobj(MACE, 1, WEAPON_CLASS, 1, 1),
	new trobj(ROBE, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(POT_WATER, 0, POTION_CLASS, 4, 1),	/* holy water */
	new trobj(CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 0),
	new trobj(SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 0),
	new trobj(UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS),
	new trobj(0, 0, 0, 0, 0)
)
val RAN_BOW = 1
val RAN_TWO_ARROWS = 2
val RAN_ZERO_ARROWS = 3
val Ranger: List[trobj] = List(
	new trobj(DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS),
	new trobj(ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS),
	new trobj(CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(CRAM_RATION, 0, FOOD_CLASS, 4, 0),
	new trobj(0, 0, 0, 0, 0)
)
val R_DAGGERS = 1
val Rogue: List[trobj] = List(
	new trobj(SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(DAGGER, 0, WEAPON_CLASS, 10, 0),	/* quan is variable */
	new trobj(LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(POT_SICKNESS, 0, POTION_CLASS, 1, 0),
	new trobj(LOCK_PICK, 9, TOOL_CLASS, 1, 0),
	new trobj(SACK, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val S_ARROWS = 3
val Samurai: List[trobj] = List(
	new trobj(KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS), /* wakizashi */
	new trobj(YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS), /* variable quan */
	new trobj(SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(0, 0, 0, 0, 0)
)
val T_DARTS = 0
val Tourist: List[trobj] = List(
	new trobj(DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS),	/* quan is variable */
	new trobj(UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 10, 0),
	new trobj(POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS),
	new trobj(SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 4, UNDEF_BLESS),
	new trobj(HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0),
	new trobj(CREDIT_CARD, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Valkyrie: List[trobj] = List(
	new trobj(LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(DAGGER, 0, WEAPON_CLASS, 1, UNDEF_BLESS),
	new trobj(SMALL_SHIELD, 3, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(FOOD_RATION, 0, FOOD_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val W_MULTSTART = 2
val W_MULTEND = 6
val Wizard: List[trobj] = List(
	new trobj(QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1),
	new trobj(CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, UNDEF_BLESS),
	new trobj(UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS),
	new trobj(UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS),
	new trobj(UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS),
	new trobj(UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS),
	new trobj(SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 1),
	new trobj(UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, UNDEF_BLESS),
	new trobj(0, 0, 0, 0, 0)
)

/*
 *	Optional extra inventory items.
 */

val Tinopener: List[trobj] = List(
	new trobj(TIN_OPENER, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Magicmarker: List[trobj] = List(
	new trobj(MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Lamp: List[trobj] = List(
	new trobj(OIL_LAMP, 1, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Blindfold: List[trobj] = List(
	new trobj(BLINDFOLD, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Instrument: List[trobj] = List(
	new trobj(WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Xtra_food: List[trobj] = List(
	new trobj(UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Leash: List[trobj] = List(
	new trobj(LEASH, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Towel: List[trobj] = List(
	new trobj(TOWEL, 0, TOOL_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)
val Wishing: List[trobj] = List(
	new trobj(WAN_WISHING, 3, WAND_CLASS, 1, 0),
	new trobj(0, 0, 0, 0, 0)
)

/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
class inv_sub(race_pm: Short, item_otyp: Short, subs_otyp: Short) {
}

private val inv_subs: List[inv_sub] = List(
	new inv_sub(PM_ELF, DAGGER, ELVEN_DAGGER),
	new inv_sub(PM_ELF, SPEAR, ELVEN_SPEAR),
	new inv_sub(PM_ELF, SHORT_SWORD, ELVEN_SHORT_SWORD),
	new inv_sub(PM_ELF, BOW, ELVEN_BOW),
	new inv_sub(PM_ELF, ARROW, ELVEN_ARROW),
	new inv_sub(PM_ELF, HELMET, ELVEN_LEATHER_HELM),
	/* new inv_sub(PM_ELF, SMALL_SHIELD, ELVEN_SHIELD) */
	new inv_sub(PM_ELF, CLOAK_OF_DISPLACEMENT, ELVEN_CLOAK),
	new inv_sub(PM_ELF, CRAM_RATION, LEMBAS_WAFER),
	new inv_sub(PM_ORC, DAGGER, ORCISH_DAGGER),
	new inv_sub(PM_ORC, SPEAR, ORCISH_SPEAR),
	new inv_sub(PM_ORC, SHORT_SWORD, ORCISH_SHORT_SWORD),
	new inv_sub(PM_ORC, BOW, ORCISH_BOW),
	new inv_sub(PM_ORC, ARROW, ORCISH_ARROW),
	new inv_sub(PM_ORC, HELMET, ORCISH_HELM),
	new inv_sub(PM_ORC, SMALL_SHIELD, ORCISH_SHIELD),
	new inv_sub(PM_ORC, RING_MAIL, ORCISH_RING_MAIL),
	new inv_sub(PM_ORC, CHAIN_MAIL, ORCISH_CHAIN_MAIL),
	new inv_sub(PM_DWARF, SPEAR, DWARVISH_SPEAR),
	new inv_sub(PM_DWARF, SHORT_SWORD, DWARVISH_SHORT_SWORD),
	new inv_sub(PM_DWARF, HELMET, DWARVISH_IRON_HELM),
	/* new inv_sub(PM_DWARF, SMALL_SHIELD, DWARVISH_ROUNDSHIELD) */
	/* new inv_sub(PM_DWARF, PICK_AXE, DWARVISH_MATTOCK) */
	new inv_sub(PM_GNOME, BOW, CROSSBOW),
	new inv_sub(PM_GNOME, ARROW, CROSSBOW_BOLT),
	new inv_sub(PM_VAMPIRE, POT_FRUIT_JUICE, POT_BLOOD),
	new inv_sub(PM_VAMPIRE, FOOD_RATION, POT_VAMPIRE_BLOOD),
	new inv_sub(NON_PM, STRANGE_OBJECT, STRANGE_OBJECT)
)

val Skill_A: List[def_skill] = List(
    new def_skill(P_DAGGER, P_BASIC),		new def_skill(P_KNIFE,  P_BASIC),
    new def_skill(P_PICK_AXE, P_EXPERT),		new def_skill(P_SHORT_SWORD, P_BASIC),
    new def_skill(P_SCIMITAR, P_SKILLED),		new def_skill(P_SABER, P_EXPERT),
    new def_skill(P_CLUB, P_SKILLED),		new def_skill(P_QUARTERSTAFF, P_SKILLED),
    new def_skill(P_SLING, P_SKILLED),		new def_skill(P_DART, P_BASIC),
    new def_skill(P_BOOMERANG, P_EXPERT),		new def_skill(P_WHIP, P_EXPERT),
    new def_skill(P_UNICORN_HORN, P_SKILLED),
    new def_skill(P_ATTACK_SPELL, P_BASIC),	new def_skill(P_HEALING_SPELL, P_BASIC),
    new def_skill(P_DIVINATION_SPELL, P_EXPERT),	new def_skill(P_MATTER_SPELL, P_BASIC),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_TWO_WEAPON_COMBAT, P_BASIC),
    new def_skill(P_BARE_HANDED_COMBAT, P_EXPERT),
    new def_skill(P_NONE, 0)
)

val Skill_B: List[def_skill] = List(
    new def_skill(P_DAGGER, P_BASIC),		new def_skill(P_AXE, P_EXPERT),
    new def_skill(P_PICK_AXE, P_SKILLED),	new def_skill(P_SHORT_SWORD, P_EXPERT),
    new def_skill(P_BROAD_SWORD, P_SKILLED),	new def_skill(P_LONG_SWORD, P_SKILLED),
    new def_skill(P_TWO_HANDED_SWORD, P_EXPERT),	new def_skill(P_SCIMITAR, P_SKILLED),
    new def_skill(P_SABER, P_BASIC),		new def_skill(P_CLUB, P_SKILLED),
    new def_skill(P_MACE, P_SKILLED),		new def_skill(P_MORNING_STAR, P_SKILLED),
    new def_skill(P_FLAIL, P_BASIC),		new def_skill(P_HAMMER, P_EXPERT),
    new def_skill(P_QUARTERSTAFF, P_BASIC),	new def_skill(P_SPEAR, P_SKILLED),
    new def_skill(P_TRIDENT, P_SKILLED),		new def_skill(P_BOW, P_BASIC),
    new def_skill(P_ATTACK_SPELL, P_SKILLED),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_TWO_WEAPON_COMBAT, P_BASIC),
    new def_skill(P_BARE_HANDED_COMBAT, P_MASTER),
    new def_skill(P_NONE, 0)
)

val Skill_C: List[def_skill] = List(
    new def_skill(P_DAGGER, P_BASIC),		new def_skill(P_KNIFE,  P_SKILLED),
    new def_skill(P_AXE, P_SKILLED),		new def_skill(P_PICK_AXE, P_BASIC),
    new def_skill(P_CLUB, P_EXPERT),		new def_skill(P_MACE, P_EXPERT),
    new def_skill(P_MORNING_STAR, P_BASIC),	new def_skill(P_FLAIL, P_SKILLED),
    new def_skill(P_HAMMER, P_SKILLED),		new def_skill(P_QUARTERSTAFF, P_EXPERT),
    new def_skill(P_POLEARMS, P_SKILLED),		new def_skill(P_SPEAR, P_EXPERT),
    new def_skill(P_JAVELIN, P_SKILLED),		new def_skill(P_TRIDENT, P_SKILLED),
    new def_skill(P_BOW, P_SKILLED),		new def_skill(P_SLING, P_EXPERT),
    new def_skill(P_ATTACK_SPELL, P_BASIC),	new def_skill(P_MATTER_SPELL, P_SKILLED),
    new def_skill(P_BOOMERANG, P_EXPERT),		new def_skill(P_UNICORN_HORN, P_BASIC),
    new def_skill(P_BARE_HANDED_COMBAT, P_MASTER),
    new def_skill(P_NONE, 0)
)

val Skill_Con: List[def_skill] = List(
    new def_skill(P_DAGGER, P_SKILLED),		new def_skill(P_KNIFE,  P_EXPERT),
    new def_skill(P_HAMMER, P_SKILLED),		new def_skill(P_PICK_AXE, P_EXPERT),
    new def_skill(P_CLUB, P_EXPERT),		    new def_skill(P_MACE, P_BASIC),
    new def_skill(P_DART, P_SKILLED),		    new def_skill(P_FLAIL, P_EXPERT),
    new def_skill(P_SHORT_SWORD, P_BASIC),		new def_skill(P_SLING, P_SKILLED),
    new def_skill(P_ATTACK_SPELL, P_BASIC),	new def_skill(P_ESCAPE_SPELL, P_EXPERT),
    new def_skill(P_TWO_WEAPON_COMBAT, P_SKILLED),
    new def_skill(P_BARE_HANDED_COMBAT, P_SKILLED),
    new def_skill(P_NONE, 0)
)

val Skill_H: List[def_skill] = List(
    new def_skill(P_DAGGER, P_SKILLED),		new def_skill(P_KNIFE, P_EXPERT),
    new def_skill(P_SHORT_SWORD, P_SKILLED),	new def_skill(P_SCIMITAR, P_BASIC),
    new def_skill(P_SABER, P_BASIC),		new def_skill(P_CLUB, P_SKILLED),
    new def_skill(P_MACE, P_BASIC),		new def_skill(P_QUARTERSTAFF, P_EXPERT),
    new def_skill(P_POLEARMS, P_BASIC),		new def_skill(P_SPEAR, P_BASIC),
    new def_skill(P_JAVELIN, P_BASIC),		new def_skill(P_TRIDENT, P_BASIC),
    new def_skill(P_SLING, P_SKILLED),		new def_skill(P_DART, P_EXPERT),
    new def_skill(P_SHURIKEN, P_SKILLED),		new def_skill(P_UNICORN_HORN, P_EXPERT),
    new def_skill(P_HEALING_SPELL, P_EXPERT),
    new def_skill(P_BARE_HANDED_COMBAT, P_BASIC),
    new def_skill(P_NONE, 0)
)

val Skill_K: List[def_skill] = List(
    new def_skill(P_DAGGER, P_BASIC),		new def_skill(P_KNIFE, P_BASIC),
    new def_skill(P_AXE, P_SKILLED),		new def_skill(P_PICK_AXE, P_BASIC),
    new def_skill(P_SHORT_SWORD, P_SKILLED),	new def_skill(P_BROAD_SWORD, P_SKILLED),
    new def_skill(P_LONG_SWORD, P_EXPERT),	new def_skill(P_TWO_HANDED_SWORD, P_SKILLED),
    new def_skill(P_SCIMITAR, P_BASIC),		new def_skill(P_SABER, P_SKILLED),
    new def_skill(P_CLUB, P_BASIC),		new def_skill(P_MACE, P_SKILLED),
    new def_skill(P_MORNING_STAR, P_SKILLED),	new def_skill(P_FLAIL, P_BASIC),
    new def_skill(P_HAMMER, P_BASIC),		new def_skill(P_POLEARMS, P_SKILLED),
    new def_skill(P_SPEAR, P_SKILLED),		new def_skill(P_JAVELIN, P_SKILLED),
    new def_skill(P_TRIDENT, P_BASIC),		new def_skill(P_LANCE, P_EXPERT),
    new def_skill(P_BOW, P_BASIC),			new def_skill(P_CROSSBOW, P_SKILLED),
    new def_skill(P_ATTACK_SPELL, P_SKILLED),	new def_skill(P_HEALING_SPELL, P_SKILLED),
    new def_skill(P_CLERIC_SPELL, P_SKILLED),
    new def_skill(P_RIDING, P_EXPERT),
    new def_skill(P_TWO_WEAPON_COMBAT, P_SKILLED),
    new def_skill(P_BARE_HANDED_COMBAT, P_EXPERT),
    new def_skill(P_NONE, 0)
)

val Skill_Mon: List[def_skill] = List(
    new def_skill(P_QUARTERSTAFF, P_BASIC),    new def_skill(P_SPEAR, P_BASIC),
    new def_skill(P_JAVELIN, P_BASIC),		    new def_skill(P_CROSSBOW, P_BASIC),
    new def_skill(P_SHURIKEN, P_BASIC),
    new def_skill(P_ATTACK_SPELL, P_BASIC),    new def_skill(P_HEALING_SPELL, P_EXPERT),
    new def_skill(P_DIVINATION_SPELL, P_BASIC),new def_skill(P_ENCHANTMENT_SPELL, P_BASIC),
    new def_skill(P_CLERIC_SPELL, P_SKILLED),  new def_skill(P_ESCAPE_SPELL, P_BASIC),
    new def_skill(P_MATTER_SPELL, P_BASIC),
    new def_skill(P_MARTIAL_ARTS, P_GRAND_MASTER),
    new def_skill(P_NONE, 0)
)

val Skill_P: List[def_skill] = List(
    new def_skill(P_CLUB, P_EXPERT),		new def_skill(P_MACE, P_EXPERT),
    new def_skill(P_MORNING_STAR, P_EXPERT),	new def_skill(P_FLAIL, P_EXPERT),
    new def_skill(P_HAMMER, P_EXPERT),		new def_skill(P_QUARTERSTAFF, P_EXPERT),
    new def_skill(P_POLEARMS, P_SKILLED),		new def_skill(P_SPEAR, P_SKILLED),
    new def_skill(P_JAVELIN, P_SKILLED),		new def_skill(P_TRIDENT, P_SKILLED),
    new def_skill(P_LANCE, P_BASIC),		new def_skill(P_BOW, P_BASIC),
    new def_skill(P_SLING, P_BASIC),		new def_skill(P_CROSSBOW, P_BASIC),
    new def_skill(P_DART, P_BASIC),		new def_skill(P_SHURIKEN, P_BASIC),
    new def_skill(P_BOOMERANG, P_BASIC),		new def_skill(P_UNICORN_HORN, P_SKILLED),
    new def_skill(P_HEALING_SPELL, P_EXPERT),	new def_skill(P_DIVINATION_SPELL, P_EXPERT),
    new def_skill(P_CLERIC_SPELL, P_EXPERT),
    new def_skill(P_BARE_HANDED_COMBAT, P_BASIC),
    new def_skill(P_NONE, 0)
)

val Skill_R: List[def_skill] = List(
    new def_skill(P_DAGGER, P_EXPERT),		new def_skill(P_KNIFE,  P_EXPERT),
    new def_skill(P_SHORT_SWORD, P_EXPERT),	new def_skill(P_BROAD_SWORD, P_SKILLED),
    new def_skill(P_LONG_SWORD, P_SKILLED),	new def_skill(P_TWO_HANDED_SWORD, P_BASIC),
    new def_skill(P_SCIMITAR, P_SKILLED),		new def_skill(P_SABER, P_SKILLED),
    new def_skill(P_CLUB, P_SKILLED),		new def_skill(P_MACE, P_SKILLED),
    new def_skill(P_MORNING_STAR, P_BASIC),	new def_skill(P_FLAIL, P_BASIC),
    new def_skill(P_HAMMER, P_BASIC),		new def_skill(P_POLEARMS, P_BASIC),
    new def_skill(P_SPEAR, P_BASIC),		new def_skill(P_CROSSBOW, P_EXPERT),
    new def_skill(P_DART, P_EXPERT),		new def_skill(P_SHURIKEN, P_SKILLED),
    new def_skill(P_DIVINATION_SPELL, P_SKILLED),	new def_skill(P_ESCAPE_SPELL, P_SKILLED),
    new def_skill(P_MATTER_SPELL, P_SKILLED),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_TWO_WEAPON_COMBAT, P_EXPERT),
    new def_skill(P_BARE_HANDED_COMBAT, P_EXPERT),
    new def_skill(P_NONE, 0)
)

val Skill_Ran: List[def_skill] = List(
    new def_skill(P_DAGGER, P_EXPERT),		 new def_skill(P_KNIFE,  P_SKILLED),
    new def_skill(P_AXE, P_SKILLED),	 new def_skill(P_PICK_AXE, P_BASIC),
    new def_skill(P_SHORT_SWORD, P_BASIC),	 new def_skill(P_MORNING_STAR, P_BASIC),
    new def_skill(P_FLAIL, P_SKILLED),	 new def_skill(P_HAMMER, P_BASIC),
    new def_skill(P_QUARTERSTAFF, P_BASIC), new def_skill(P_POLEARMS, P_SKILLED),
    new def_skill(P_SPEAR, P_SKILLED),	 new def_skill(P_JAVELIN, P_EXPERT),
    new def_skill(P_TRIDENT, P_BASIC),	 new def_skill(P_BOW, P_EXPERT),
    new def_skill(P_SLING, P_EXPERT),	 new def_skill(P_CROSSBOW, P_EXPERT),
    new def_skill(P_DART, P_EXPERT),	 new def_skill(P_SHURIKEN, P_SKILLED),
    new def_skill(P_BOOMERANG, P_EXPERT),	 new def_skill(P_WHIP, P_BASIC),
    new def_skill(P_HEALING_SPELL, P_BASIC),
    new def_skill(P_DIVINATION_SPELL, P_EXPERT),
    new def_skill(P_ESCAPE_SPELL, P_BASIC),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_BARE_HANDED_COMBAT, P_BASIC),
    new def_skill(P_NONE, 0)
)

val Skill_S: List[def_skill] = List(
    new def_skill(P_DAGGER, P_BASIC),		new def_skill(P_KNIFE,  P_SKILLED),
    new def_skill(P_SHORT_SWORD, P_EXPERT),	new def_skill(P_BROAD_SWORD, P_SKILLED),
    new def_skill(P_LONG_SWORD, P_EXPERT),		new def_skill(P_TWO_HANDED_SWORD, P_EXPERT),
    new def_skill(P_SCIMITAR, P_BASIC),		new def_skill(P_SABER, P_BASIC),
    new def_skill(P_FLAIL, P_SKILLED),		new def_skill(P_QUARTERSTAFF, P_BASIC),
    new def_skill(P_POLEARMS, P_SKILLED),		new def_skill(P_SPEAR, P_BASIC),
    new def_skill(P_JAVELIN, P_BASIC),		new def_skill(P_LANCE, P_SKILLED),
    new def_skill(P_BOW, P_EXPERT),		new def_skill(P_SHURIKEN, P_EXPERT),
    new def_skill(P_ATTACK_SPELL, P_SKILLED),	new def_skill(P_CLERIC_SPELL, P_SKILLED),
    new def_skill(P_RIDING, P_SKILLED),
    new def_skill(P_TWO_WEAPON_COMBAT, P_EXPERT),
    new def_skill(P_MARTIAL_ARTS, P_MASTER),
    new def_skill(P_NONE, 0)
)

val Skill_T: List[def_skill] = List(
    new def_skill(P_DAGGER, P_EXPERT),		new def_skill(P_KNIFE,  P_SKILLED),
    new def_skill(P_AXE, P_BASIC),			new def_skill(P_PICK_AXE, P_BASIC),
    new def_skill(P_SHORT_SWORD, P_EXPERT),	new def_skill(P_BROAD_SWORD, P_BASIC),
    new def_skill(P_LONG_SWORD, P_BASIC),		new def_skill(P_TWO_HANDED_SWORD, P_BASIC),
    new def_skill(P_SCIMITAR, P_SKILLED),		new def_skill(P_SABER, P_SKILLED),
    new def_skill(P_MACE, P_BASIC),		new def_skill(P_MORNING_STAR, P_BASIC),
    new def_skill(P_FLAIL, P_BASIC),		new def_skill(P_HAMMER, P_BASIC),
    new def_skill(P_QUARTERSTAFF, P_BASIC),	new def_skill(P_POLEARMS, P_BASIC),
    new def_skill(P_SPEAR, P_BASIC),		new def_skill(P_JAVELIN, P_BASIC),
    new def_skill(P_TRIDENT, P_BASIC),		new def_skill(P_LANCE, P_BASIC),
    new def_skill(P_BOW, P_BASIC),			new def_skill(P_SLING, P_BASIC),
    new def_skill(P_CROSSBOW, P_BASIC),		new def_skill(P_DART, P_EXPERT),
    new def_skill(P_SHURIKEN, P_BASIC),		new def_skill(P_BOOMERANG, P_BASIC),
    new def_skill(P_WHIP, P_BASIC),		new def_skill(P_UNICORN_HORN, P_SKILLED),
    new def_skill(P_DIVINATION_SPELL, P_BASIC),	new def_skill(P_ENCHANTMENT_SPELL, P_BASIC),
    new def_skill(P_ESCAPE_SPELL, P_SKILLED),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_TWO_WEAPON_COMBAT, P_SKILLED),
    new def_skill(P_BARE_HANDED_COMBAT, P_SKILLED),
    new def_skill(P_NONE, 0)
)

val Skill_V: List[def_skill] = List(
    new def_skill(P_DAGGER, P_EXPERT),		new def_skill(P_AXE, P_EXPERT),
    new def_skill(P_PICK_AXE, P_SKILLED),		new def_skill(P_SHORT_SWORD, P_SKILLED),
    new def_skill(P_BROAD_SWORD, P_SKILLED),	new def_skill(P_LONG_SWORD, P_EXPERT),
    new def_skill(P_TWO_HANDED_SWORD, P_EXPERT),	new def_skill(P_SCIMITAR, P_BASIC),
    new def_skill(P_SABER, P_BASIC),		new def_skill(P_HAMMER, P_EXPERT),
    new def_skill(P_QUARTERSTAFF, P_BASIC),	new def_skill(P_POLEARMS, P_SKILLED),
    new def_skill(P_SPEAR, P_SKILLED),		new def_skill(P_JAVELIN, P_BASIC),
    new def_skill(P_TRIDENT, P_BASIC),		new def_skill(P_LANCE, P_SKILLED),
    new def_skill(P_SLING, P_BASIC),
    new def_skill(P_ATTACK_SPELL, P_BASIC),	new def_skill(P_ESCAPE_SPELL, P_BASIC),
    new def_skill(P_RIDING, P_SKILLED),
    new def_skill(P_TWO_WEAPON_COMBAT, P_SKILLED),
    new def_skill(P_BARE_HANDED_COMBAT, P_EXPERT),
    new def_skill(P_NONE, 0)
)

val Skill_W: List[def_skill] = List(
    new def_skill(P_DAGGER, P_EXPERT),		new def_skill(P_KNIFE,  P_SKILLED),
    new def_skill(P_AXE, P_SKILLED),		new def_skill(P_SHORT_SWORD, P_BASIC),
    new def_skill(P_CLUB, P_SKILLED),		new def_skill(P_MACE, P_BASIC),
    new def_skill(P_QUARTERSTAFF, P_EXPERT),	new def_skill(P_POLEARMS, P_SKILLED),
    new def_skill(P_SPEAR, P_BASIC),		new def_skill(P_JAVELIN, P_BASIC),
    new def_skill(P_TRIDENT, P_BASIC),		new def_skill(P_SLING, P_SKILLED),
    new def_skill(P_DART, P_EXPERT),		new def_skill(P_SHURIKEN, P_BASIC),
    new def_skill(P_ATTACK_SPELL, P_EXPERT),	new def_skill(P_HEALING_SPELL, P_SKILLED),
    new def_skill(P_DIVINATION_SPELL, P_EXPERT),	new def_skill(P_ENCHANTMENT_SPELL, P_SKILLED),
    new def_skill(P_CLERIC_SPELL, P_SKILLED),	new def_skill(P_ESCAPE_SPELL, P_EXPERT),
    new def_skill(P_MATTER_SPELL, P_EXPERT),
    new def_skill(P_RIDING, P_BASIC),
    new def_skill(P_BARE_HANDED_COMBAT, P_BASIC),
    new def_skill(P_NONE, 0)
)

private def 
knows_object(obj: Int): Unit = 
{
	discover_object(obj,TRUE,FALSE)
	objects(obj).oc_pre_discovered = 1	/* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
 * like all gems except the loadstone and luckstone.
 */
private def 
knows_class(sym: Char): Unit =
{
	for (ct <- 1 until NUM_OBJECTS)
		if (objects(ct).oc_class == sym && !objects(ct).oc_magic &&
		    !Is_dragon_armor(ct))
			knows_object(ct);
}

def
u_init(): Unit =
{
/*** MOTODO: Convert
	var i = 0
	struct permonst* shambler = &mons[PM_SHAMBLING_HORROR];
	struct attack* attkptr;

	flags.female = flags.initgend;
	flags.beginner = 1;

	/* zero u, including pointer values --
	 * necessary when aborting from a failed restore */
	(void) memset((genericptr_t)&u, 0, sizeof(u));
	u.ustuck = (struct monst *)0;

	u.uz.dlevel = 1;
	u.uz0.dlevel = 0;
	u.utolev = u.uz;

	u.umoved = FALSE;
	u.umortality = 0;
	u.ugrave_arise = NON_PM;

	u.umonnum = u.umonster = (flags.female &&
			urole.femalenum != NON_PM) ? urole.femalenum :
			urole.malenum;
	init_uasmon();

	u.ulevel = 0;	/* set up some of the initial attributes */
	u.uhp = u.uhpmax = newhp();
	u.uenmax = urole.enadv.infix + urace.enadv.infix;
	if (urole.enadv.inrnd > 0)
	    u.uenmax += rnd(urole.enadv.inrnd);
	if (urace.enadv.inrnd > 0)
	    u.uenmax += rnd(urace.enadv.inrnd);
	u.uen = u.uenmax;
	u.uspellprot = 0;
	adjabil(0,1);
	u.ulevel = u.ulevelmax = 1;

	/*
	 * u.roleplay should be treated similar to gender and alignment
	 *   - it gets set at character creation
	 *   - it's hard to change in-game
	 *     (e.g. a special NPC could teach literacy somewhere)
	 * the initialisation has to be in front of food, alignment
	 * and inventory.
	 */
	u.roleplay.ascet	= flags.ascet;
	u.roleplay.atheist	= flags.atheist;
	u.roleplay.blindfolded	= flags.blindfolded;
	u.roleplay.illiterate	= flags.illiterate;
	u.roleplay.pacifist	= flags.pacifist;
	u.roleplay.nudist	= flags.nudist;
	u.roleplay.vegan	= flags.vegan;
	u.roleplay.vegetarian	= flags.vegetarian;

	if (flags.heaven_or_hell) {
		heaven_or_hell_mode = TRUE;
	}
	if (flags.hell_and_hell) {
		heaven_or_hell_mode = TRUE;
		hell_and_hell_mode = TRUE;
	}

	if (heaven_or_hell_mode) {
		u.ulives = 3;
		u.uhpmax = 1;
		u.uhp = 1;
		HWarning = TRUE;
	} else {
		u.ulives = 0;
	}

	init_uhunger();
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;
	u.ublesscnt = 300;			/* no prayers just yet */
	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type =
			aligns[flags.initalign].value;
	u.ulycn = NON_PM;

#if defined(BSD) && !defined(POSIX_TYPES)
	(void) time((long *)&u.ubirthday);
#else
	(void) time(&u.ubirthday);
#endif

	/*
	 *  For now, everyone starts out with a night vision range of 1 and
	 *  their xray range disabled.
	 */
	u.nv_range   =  1;
	u.xray_range = -1;


	/*** Role-specific initializations ***/
	switch (Role_switch) {
	/* rn2(100) > 50 necessary for some choices because some
	 * random number generators are bad enough to seriously
	 * skew the results if we use rn2(2)...  --KAA
	 */
	case PM_ARCHEOLOGIST:
		ini_inv(Archeologist);
		if(!rn2(10)) ini_inv(Tinopener);
		else if(!rn2(4)) ini_inv(Lamp);
		else if(!rn2(10)) ini_inv(Magicmarker);
		knows_object(SACK);
		knows_object(TOUCHSTONE);
		skill_init(Skill_A);
		break;
	case PM_BARBARIAN:
		if (rn2(100) >= 50) {	/* see above comment */
		    Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
		    Barbarian[B_MINOR].trotyp = SHORT_SWORD;
		}
		ini_inv(Barbarian);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_B);
		break;
	case PM_CAVEMAN:
		Cave_man[C_AMMO].trquan = rn1(11, 10);	/* 10..20 */
		ini_inv(Cave_man);
		skill_init(Skill_C);
		break;
	case PM_CONVICT:
        ini_inv(Convict);
        knows_object(SKELETON_KEY);
        knows_object(GRAPPLING_HOOK);
        skill_init(Skill_Con);
        u.uhunger = 200;  /* On the verge of hungry */
    	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
        u.ualign.type = A_CHAOTIC; /* Override racial alignment */
        urace.hatemask |= urace.lovemask;   /* Hated by the race's allies */
        urace.lovemask = 0; /* Convicts are pariahs of their race */
        break;
	case PM_HEALER:
		u.ugold = u.ugold0 = rn1(1000, 1001);
		ini_inv(Healer);
		if(!rn2(25)) ini_inv(Lamp);
		knows_object(POT_FULL_HEALING);
		skill_init(Skill_H);
		break;
	case PM_KNIGHT:
		ini_inv(Knight);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		/* give knights chess-like mobility
		 * -- idea from wooledge@skybridge.scl.cwru.edu */
		HJumping |= FROMOUTSIDE;
		skill_init(Skill_K);
		break;
	case PM_MONK:
		switch (rn2(90) / 30) {
		case 0: Monk[M_BOOK].trotyp = SPE_HEALING; break;
		case 1: Monk[M_BOOK].trotyp = SPE_PROTECTION; break;
		case 2: Monk[M_BOOK].trotyp = SPE_SLEEP; break;
		}
		ini_inv(Monk);
		if(!rn2(5)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Mon);
		break;
	case PM_PRIEST:
		if (Race_switch == PM_ELF) Priest[PRI_MACE].trotyp = CLUB;
		ini_inv(Priest);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_object(POT_WATER);
		skill_init(Skill_P);
		/* KMH, conduct --
		 * Some may claim that this isn't agnostic, since they
		 * are literally "priests" and they have holy water.
		 * But we don't count it as such.  Purists can always
		 * avoid playing priests and/or confirm another player's
		 * role in their YAAP.
		 */
		break;
	case PM_RANGER:
		Ranger[RAN_TWO_ARROWS].trquan = rn1(10, 50);
		Ranger[RAN_ZERO_ARROWS].trquan = rn1(10, 30);
		ini_inv(Ranger);
		skill_init(Skill_Ran);
		break;
	case PM_ROGUE:
		Rogue[R_DAGGERS].trquan = rn1(10, 6);
		u.ugold = u.ugold0 = 0;
		ini_inv(Rogue);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_object(SACK);
		skill_init(Skill_R);
		break;
	case PM_SAMURAI:
		Samurai[S_ARROWS].trquan = rn1(20, 26);
		ini_inv(Samurai);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_S);
		break;
	case PM_TOURIST:
		Tourist[T_DARTS].trquan = rn1(43, 21); /* 63 darts ought to be enough for anybody. */
		u.ugold = u.ugold0 = rnd(1000);
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		else if(!rn2(25)) ini_inv(Leash);
		else if(!rn2(25) && !towelday()) ini_inv(Towel);
		else if(!rn2(25)) ini_inv(Magicmarker);
		skill_init(Skill_T);
		break;
	case PM_VALKYRIE:
		ini_inv(Valkyrie);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_V);
		break;
	case PM_WIZARD:
		ini_inv(Wizard);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_W);
		break;

	default:	/* impossible */
		warning("u_init: unknown role %d", Role_switch);
		break;
	}

	/* Add knowledge of life saving amulets for heaven or hell mode. */
	if (heaven_or_hell_mode) {
		knows_object(AMULET_OF_LIFE_SAVING);
	}

	/*** Race-specific initializations ***/
	switch (Race_switch) {
	case PM_HUMAN:
	    /* Nothing special */
	    break;

	case PM_ELF:
	    /*
	     * Elves are people of music and song, or they are warriors.
	     * Non-warriors get an instrument.  We use a kludge to
	     * get only non-magic instruments.
	     */
	    if (Role_if(PM_PRIEST) || Role_if(PM_WIZARD)) {
		static int trotyp[] = {
		    WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
		    BELL, BUGLE, LEATHER_DRUM
		};
		Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Instrument);
	    }

	    /* Elves can recognize all elvish objects */
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
	    break;

	case PM_DWARF:
	    /* Dwarves can recognize all dwarvish objects */
	    knows_object(DWARVISH_SPEAR);
	    knows_object(DWARVISH_SHORT_SWORD);
	    knows_object(DWARVISH_MATTOCK);
	    knows_object(DWARVISH_IRON_HELM);
	    knows_object(DWARVISH_MITHRIL_COAT);
	    knows_object(DWARVISH_CLOAK);
	    knows_object(DWARVISH_ROUNDSHIELD);
	    break;

	case PM_GNOME:
	    break;

	case PM_ORC:
	    /* compensate for generally inferior equipment */
	    if (!Role_if(PM_WIZARD))
        if (!Role_if(PM_CONVICT))
		ini_inv(Xtra_food);
	    /* Orcs can recognize all orcish objects */
	    knows_object(ORCISH_SHORT_SWORD);
	    knows_object(ORCISH_ARROW);
	    knows_object(ORCISH_BOW);
	    knows_object(ORCISH_SPEAR);
	    knows_object(ORCISH_DAGGER);
	    knows_object(ORCISH_CHAIN_MAIL);
	    knows_object(ORCISH_RING_MAIL);
	    knows_object(ORCISH_HELM);
	    knows_object(ORCISH_SHIELD);
	    knows_object(URUK_HAI_SHIELD);
	    knows_object(ORCISH_CLOAK);
	    break;

	case PM_VAMPIRE:
	    /* Vampires start off with gods not as pleased, luck penalty */
	    adjalign(-5); 
	    change_luck(-1);
	    break;

	default:	/* impossible */
		warning("u_init: unknown race %d", Race_switch);
		break;
	}

	/* Towel Day: In Memoriam Douglas Adams */
	if (towelday())
		ini_inv(Towel);

	/*** Conduct specific initialisation ***/

	if (u.roleplay.blindfolded) {
		if(!ublindf) ini_inv(Blindfold);
	} else {
		violated(CONDUCT_BLINDFOLDED);
	}
	if (u.roleplay.atheist) 
		u.ugangr++;

	if (discover)
		ini_inv(Wishing);

#ifdef WIZARD
	if (wizard)
		read_wizkit();
#endif

	u.ugold0 += hidden_gold();	/* in case sack has gold in it */

	find_ac();			/* get initial ac value */
	init_attr(75);			/* init attribute values */
	max_rank_sz();			/* set max str size for class ranks */
/*
 *	Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;	/* biased variation */
		(void) adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }

	/* make sure you can carry all you have - especially for Tourists */
	while (inv_weight() > 0) {
		if (adjattrib(A_STR, 1, TRUE)) continue;
		if (adjattrib(A_CON, 1, TRUE)) continue;
		/* only get here when didn't boost strength or constitution */
		break;
	}

	/* what a horrible night to have a curse */
	shambler->mlevel += rnd(12)-3;				/* shuffle level */
	shambler->mmove = rn2(10)+9;				/* slow to very fast */
	shambler->ac = rn2(21)-10;				/* any AC */
	shambler->mr = rn2(5)*25;				/* varying amounts of MR */
	shambler->maligntyp = rn2(21)-10;			/* any alignment */
	/* attacks...?  */
	for (i = 0; i < rnd(4); i++) {
		attkptr = &shambler->mattk[i];
		/* restrict it to certain types of attacks */
		attkptr->aatyp = 0;
		while (attkptr->aatyp == 0 || attkptr->aatyp == AT_ENGL || attkptr->aatyp == AT_SPIT ||
					attkptr->aatyp == AT_BREA || attkptr->aatyp == AT_EXPL || 
					attkptr->aatyp == AT_BOOM || attkptr->aatyp == AT_GAZE) {
			attkptr->aatyp = rn2(AT_SCRE);
		}
		attkptr->adtyp = 0;
		while (attkptr->adtyp == 0 || attkptr->adtyp == AD_DETH || attkptr->adtyp == AD_TLPT ||
					attkptr->adtyp == AD_DGST || attkptr->adtyp == AD_SLIM ||
					attkptr->adtyp == AD_ENCH || attkptr->adtyp == AD_DISN ||
					attkptr->adtyp == AD_PEST || attkptr->adtyp == AD_FAMN ||
					attkptr->adtyp == AD_SPC1 || attkptr->adtyp == AD_SPC2) {
			attkptr->adtyp = rn2(AD_FREZ);
		}
		attkptr->damn = 2;				/* we're almost sure to get this wrong first time */
		attkptr->damd = 10;				/* either too high or too low */
	}
	shambler->msize = rn2(MZ_GIGANTIC+1);			/* any size */
	shambler->cwt = 20;					/* fortunately moot as it's flagged NOCORPSE */
	shambler->cnutrit = 20;					/* see above */
	shambler->msound = rn2(MS_HUMANOID);			/* any but the specials */
	shambler->mresists = 0;
	for (i = 0; i < rnd(6); i++) {
		shambler->mresists |= (1 << rn2(8));		/* physical resistances... */
	}
	for (i = 0; i < rnd(5); i++) {
		shambler->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
	}
	shambler->mconveys = 0;					/* flagged NOCORPSE */
	/*
	 * now time for the random flags.  this will likely produce
	 * a number of complete trainwreck monsters at first, but
	 * every so often something will dial up nasty stuff
	 */
	shambler->mflags1 = 0;
	for (i = 0; i < rnd(17); i++) {
		shambler->mflags1 |= (1 << rn2(33));		/* trainwreck this way :D */
	}
	shambler->mflags1 &= ~M1_UNSOLID;			/* no ghosts */
	shambler->mflags1 &= ~M1_WALLWALK;			/* no wall-walkers */

	shambler->mflags2 = M2_NOPOLY | M2_HOSTILE;		/* Don't let the player be one of these yet. */
	for (i = 0; i < rnd(17); i++) {
		shambler->mflags2 |= (1 << rn2(31));
	}
	shambler->mflags2 &= ~M2_MERC;				/* no guards */
	shambler->mflags2 &= ~M2_PEACEFUL;			/* no peacefuls */
	shambler->mflags2 &= ~M2_WERE;				/* no lycanthropes */
	shambler->mflags2 &= ~M2_PNAME;				/* not a proper name */
***/
}

/* skills aren't initialized, so we use the role-specific skill lists */
private def 
restricted_spell_discipline(otyp: Int): Boolean = 
{
	var skills: def_skill = null
	var this_skill = spell_skilltype(otyp)
/*** MOTODO: Convert
    switch (Role_switch) {
     case PM_ARCHEOLOGIST:	skills = Skill_A; break;
     case PM_BARBARIAN:		skills = Skill_B; break;
     case PM_CAVEMAN:		skills = Skill_C; break;
     case PM_CONVICT:		skills = Skill_Con; break;
     case PM_HEALER:		skills = Skill_H; break;
     case PM_KNIGHT:		skills = Skill_K; break;
     case PM_MONK:		skills = Skill_Mon; break;
     case PM_PRIEST:		skills = Skill_P; break;
     case PM_RANGER:		skills = Skill_Ran; break;
     case PM_ROGUE:		skills = Skill_R; break;
     case PM_SAMURAI:		skills = Skill_S; break;
     case PM_TOURIST:		skills = Skill_T; break;
     case PM_VALKYRIE:		skills = Skill_V; break;
     case PM_WIZARD:		skills = Skill_W; break;
     default:			skills = 0; break;	/* lint suppression */
    }

    while (skills.skill != P_NONE) {
	if (skills.skill == this_skill) return false
	++skills;
    }
***/
	true
}

private def 
ini_inv(trop: trobj): Unit =
{
/*** MOTODO: Convert
	struct obj *obj;
	int otyp, i;

	while (trop->trclass) {
		if (trop->trotyp != UNDEF_TYP) {
			otyp = (int)trop->trotyp;
			if (urace.malenum != PM_HUMAN) {
			    /* substitute specific items for generic ones */
			    for (i = 0; inv_subs[i].race_pm != NON_PM; ++i)
				if (inv_subs[i].race_pm == urace.malenum &&
					otyp == inv_subs[i].item_otyp) {
				    otyp = inv_subs[i].subs_otyp;
				    break;
				}
			}
			obj = mksobj(otyp, TRUE, FALSE);
		} else {	/* UNDEF_TYP */
			static short nocreate = STRANGE_OBJECT;
			static short nocreate2 = STRANGE_OBJECT;
			static short nocreate3 = STRANGE_OBJECT;
			static short nocreate4 = STRANGE_OBJECT;
		/*
		 * For random objects, do not create certain overly powerful
		 * items: wand of wishing, ring of levitation, or the
		 * polymorph/polymorph control combination.  Specific objects,
		 * i.e. the discovery wishing, are still OK.
		 * Also, don't get a couple of really useless items.  (Note:
		 * punishment isn't "useless".  Some players who start out with
		 * one will immediately read it and use the iron ball as a
		 * weapon.)
		 */
			obj = mkobj(trop->trclass, FALSE);
			otyp = obj->otyp;
			while (otyp == WAN_WISHING
				|| otyp == nocreate
				|| otyp == nocreate2
				|| otyp == nocreate3
				|| otyp == nocreate4
				|| otyp == RIN_LEVITATION
				/* 'useless' items */
				|| otyp == POT_HALLUCINATION
				|| otyp == POT_ACID
				|| otyp == SCR_FLOOD
				|| otyp == SCR_FIRE
				|| otyp == SCR_BLANK_PAPER
				|| otyp == SPE_BLANK_PAPER
				|| otyp == RIN_AGGRAVATE_MONSTER
				|| otyp == RIN_HUNGER
				|| otyp == WAN_NOTHING
				/* Monks don't use weapons */
				|| (otyp == SCR_ENCHANT_WEAPON &&
				    Role_if(PM_MONK))
				/* wizard patch -- they already have one */
				|| (otyp == SPE_FORCE_BOLT &&
				    Role_if(PM_WIZARD))
				/* powerful spells are either useless to
				   low level players or unbalancing; also
				   spells in restricted skill categories */
				|| (obj->oclass == SPBOOK_CLASS &&
				    (objects[otyp].oc_level > 3 ||
				    restricted_spell_discipline(otyp)))
							) {
				dealloc_obj(obj);
				obj = mkobj(trop->trclass, FALSE);
				otyp = obj->otyp;
			}

			/* Don't start with +0 or negative rings */
			if (objects[otyp].oc_charged && obj->spe <= 0)
				obj->spe = rne(3);

			/* Heavily relies on the fact that 1) we create wands
			 * before rings, 2) that we create rings before
			 * spellbooks, and that 3) not more than 1 object of a
			 * particular symbol is to be prohibited.  (For more
			 * objects, we need more nocreate variables...)
			 */
			switch (otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
			    case POT_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
				nocreate2 = SPE_POLYMORPH;
				nocreate3 = POT_POLYMORPH;
			}
			/* Don't have 2 of the same ring or spellbook */
			if (obj->oclass == RING_CLASS ||
			    obj->oclass == SPBOOK_CLASS)
				nocreate4 = otyp;
		}

			obj->dknown = obj->bknown = obj->rknown = 1;
			if (objects[otyp].oc_uses_known) obj->known = 1;
			obj->cursed = 0;
			if (obj->opoisoned && u.ualign.type != A_CHAOTIC)
			    obj->opoisoned = 0;
			if (obj->oclass == WEAPON_CLASS ||
				obj->oclass == TOOL_CLASS) {
			    obj->quan = (long) trop->trquan;
			    trop->trquan = 1;
			} else if (obj->oclass == GEM_CLASS &&
				is_graystone(obj) && obj->otyp != FLINT) {
			    obj->quan = 1L;
			}
            if (obj->otyp == STRIPED_SHIRT ) {
                obj->cursed = TRUE;
            }
			if (trop->trspe != UNDEF_SPE)
			    obj->spe = trop->trspe;
			if (trop->trbless != UNDEF_BLESS)
			    obj->blessed = trop->trbless;
		/* defined after setting otyp+quan + blessedness */
		obj->owt = weight(obj);
		obj = addinv(obj);

		/* Make the type known if necessary */
		if (OBJ_DESCR(objects[otyp]) && obj->known)
			discover_object(otyp, TRUE, FALSE);
		if (otyp == OIL_LAMP)
			discover_object(POT_OIL, TRUE, FALSE);

		if((obj->oclass == ARMOR_CLASS && !u.roleplay.nudist)){
			if (is_shield(obj) && !uarms) {
				setworn(obj, W_ARMS);
				if (uswapwep) setuswapwep((struct obj *) 0);
			} else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
			else if (is_shirt(obj) && !uarmu)
				setworn(obj, W_ARMU);
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (is_suit(obj) && !uarm)
				setworn(obj, W_ARM);
		}

		if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
			otyp == TIN_OPENER || otyp == FLINT || otyp == ROCK) {
		    if (is_ammo(obj) || is_missile(obj)) {
			if (!uquiver) setuqwep(obj);
		    } else if ((!uwep) && !u.roleplay.pacifist) setuwep(obj);
		    else if (!uswapwep) setuswapwep(obj);
		}
		if (obj->oclass == SPBOOK_CLASS &&
				obj->otyp != SPE_BLANK_PAPER)
		    initialspell(obj);

		if ((obj->otyp == BLINDFOLD) && u.roleplay.blindfolded) {
			/* Zen ascension attempt */
			obj->cursed = TRUE;
			setworn(obj, W_TOOL);
		}

		/* mark object as start inventory object */
		if (obj) {
			obj->was_in_starting_inventory = TRUE;
		}

		if(--trop->trquan) continue;	/* make a similar object */
		trop++;
	}
***/
}
}
