/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct trobj {
    short trotyp;
    schar trspe;
    char trclass;
    Bitfield(trquan, 6);
    Bitfield(trbless, 2);
};

static void ini_inv(struct trobj *);
static void knows_object(int);
static void knows_class(char);
static boolean restricted_spell_discipline(int);

#define UNDEF_TYP   0
#define UNDEF_SPE   '\177'
#define UNDEF_BLESS 2

/*
 *  Initial inventory for the various roles.
 */

static struct trobj Archeologist[] = {
    /* if adventure has a name...  idea from tan@uvm-gen */
    { BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
    { LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
    { PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
    { TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
    { TOUCHSTONE, 0, GEM_CLASS, 1, 0 },
    { SACK, 0, TOOL_CLASS, 1, 0 },

    { 0, 0, 0, 0, 0 }
};
static struct trobj Barbarian[] = {
#define B_MAJOR 0   /* two-handed sword or battle-axe  */
#define B_MINOR 1   /* matched with axe or short sword */
    { TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Cave_man[] = {
#define C_AMMO  2
    { CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { SLING, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
    { FLINT, 0, GEM_CLASS, 15, UNDEF_BLESS },   /* quan is variable */
    { ROCK, 0, GEM_CLASS, 3, 0 },           /* yields 18..33 */
    { LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Convict[] = {
    { ROCK, 0, GEM_CLASS, 1, 0 },
    { STRIPED_SHIRT, 0, ARMOR_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Healer[] = {
    { SCALPEL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
    { STETHOSCOPE, 0, TOOL_CLASS, 1, 0 },
    { POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
    { POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
    { WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
    /* always blessed, so it's guaranteed readable */
    { SPE_HEALING, 0, SPBOOK_CLASS, 1, 1 },
    { SPE_EXTRA_HEALING, 0, SPBOOK_CLASS, 1, 1 },
    { SPE_STONE_TO_FLESH, 0, SPBOOK_CLASS, 1, 1 },
    { APPLE, 0, FOOD_CLASS, 5, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { RING_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { APPLE, 0, FOOD_CLASS, 10, 0 },
    { CARROT, 0, FOOD_CLASS, 10, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Monk[] = {
#define M_BOOK      2
    { LEATHER_GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
    { ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
    { POT_HEALING, 0, POTION_CLASS, 3, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
    { APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
    { ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
    /* Yes, we know fortune cookies aren't really from China.  They were
     * invented by George Jung in Los Angeles, California, USA in 1916.
     */
    { FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Priest[] = {
#define PRI_MACE    0
    { MACE, 1, WEAPON_CLASS, 1, 1 },
    { ROBE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { POT_WATER, 0, POTION_CLASS, 4, 1 },   /* holy water */
    { CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 0 },
    { SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 0 },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Ranger[] = {
#define RAN_BOW         1
#define RAN_TWO_ARROWS  2
#define RAN_ZERO_ARROWS 3
    { DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
    { ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
    { CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
    { CRAM_RATION, 0, FOOD_CLASS, 4, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Rogue[] = {
#define R_DAGGERS   1
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { DAGGER, 0, WEAPON_CLASS, 10, 0 }, /* quan is variable */
    { LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
    { POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
    { LOCK_PICK, 0, TOOL_CLASS, 1, 0 },
    { SACK, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Samurai[] = {
#define S_ARROWS    3
    { KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS }, /* wakizashi */
    { YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS }, /* variable quan */
    { SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Tourist[] = {
#define T_DARTS     0
    { DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS }, /* quan is variable */
    { UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 10, 0 },
    { POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS },
    { SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 4, UNDEF_BLESS },
    { HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
    { CREDIT_CARD, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Valkyrie[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
    { DAGGER, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
    { SMALL_SHIELD, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Wizard[] = {
#define W_MULTSTART 2
#define W_MULTEND   6
    { QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },
    { CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
    { SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 1 },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0 }
};

/*
 *  Optional extra inventory items.
 */

static struct trobj Tinopener[] = {
    { TIN_OPENER, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Magicmarker[] = {
    { MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Lamp[] = {
    { OIL_LAMP, 1, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Blindfold[] = {
    { BLINDFOLD, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Instrument[] = {
    { WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Xtra_food[] = {
    { UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Leash[] = {
    { LEASH, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Towel[] = {
    { TOWEL, 0, TOOL_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
static struct trobj Wishing[] = {
    { WAN_WISHING, 3, WAND_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};

static struct trobj Money[] = {
    { GOLD_PIECE, 0, COIN_CLASS, 1, 0 },
    { 0, 0, 0, 0, 0 }
};

/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
static struct inv_sub {
    short race_pm, item_otyp, subs_otyp;
} inv_subs[] = {
    { PM_ELF, DAGGER, ELVEN_DAGGER },
    { PM_ELF, SPEAR, ELVEN_SPEAR },
    { PM_ELF, SHORT_SWORD, ELVEN_SHORT_SWORD },
    { PM_ELF, BOW, ELVEN_BOW },
    { PM_ELF, ARROW, ELVEN_ARROW },
    { PM_ELF, HELMET, ELVEN_LEATHER_HELM },
    /* { PM_ELF, SMALL_SHIELD, ELVEN_SHIELD }, */
    { PM_ELF, CLOAK_OF_DISPLACEMENT, ELVEN_CLOAK },
    { PM_ELF, CRAM_RATION, LEMBAS_WAFER },
    { PM_ORC, DAGGER, ORCISH_DAGGER },
    { PM_ORC, SPEAR, ORCISH_SPEAR },
    { PM_ORC, SHORT_SWORD, ORCISH_SHORT_SWORD },
    { PM_ORC, BOW, ORCISH_BOW },
    { PM_ORC, ARROW, ORCISH_ARROW },
    { PM_ORC, HELMET, ORCISH_HELM },
    { PM_ORC, SMALL_SHIELD, ORCISH_SHIELD },
    { PM_ORC, RING_MAIL, ORCISH_RING_MAIL },
    { PM_ORC, CHAIN_MAIL, ORCISH_CHAIN_MAIL },
    { PM_ORC, CRAM_RATION, TRIPE_RATION },
    { PM_ORC, LEMBAS_WAFER, TRIPE_RATION },
    { PM_DWARF, SPEAR, DWARVISH_SPEAR },
    { PM_DWARF, SHORT_SWORD, DWARVISH_SHORT_SWORD },
    { PM_DWARF, HELMET, DWARVISH_IRON_HELM },
    /* { PM_DWARF, SMALL_SHIELD, DWARVISH_ROUNDSHIELD }, */
    /* { PM_DWARF, PICK_AXE, DWARVISH_MATTOCK }, */
    { PM_DWARF, LEMBAS_WAFER, CRAM_RATION },
    { PM_GNOME, BOW, CROSSBOW },
    { PM_GNOME, ARROW, CROSSBOW_BOLT },
    { PM_VAMPIRE, POT_FRUIT_JUICE, POT_BLOOD },
    { PM_VAMPIRE, FOOD_RATION, POT_VAMPIRE_BLOOD },
    { NON_PM, STRANGE_OBJECT, STRANGE_OBJECT }
};

static const struct def_skill Skill_A[] = {
    { P_DAGGER, P_BASIC },
    { P_KNIFE,  P_BASIC },
    { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },
    { P_SABER, P_EXPERT },
    { P_CLUB, P_SKILLED },
    { P_QUARTERSTAFF, P_SKILLED },
    { P_SLING, P_SKILLED },
    { P_DART, P_BASIC },
    { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },
    { P_UNICORN_HORN, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT},
    { P_MATTER_SPELL, P_BASIC},
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_B[] = {
    { P_DAGGER, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_SKILLED },
    { P_SABER, P_BASIC },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_BOW, P_BASIC },
    { P_ATTACK_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_BASIC }, /* special spell is haste self */
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_C[] = {
    { P_DAGGER, P_BASIC },
    { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },
    { P_PICK_AXE, P_BASIC },
    { P_CLUB, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_SKILLED },
    { P_HAMMER, P_SKILLED },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_SKILLED },
    { P_BOW, P_SKILLED },
    { P_SLING, P_EXPERT },
    { P_ATTACK_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },
    { P_UNICORN_HORN, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Con[] = {
    { P_DAGGER, P_SKILLED },
    { P_KNIFE,  P_EXPERT },
    { P_HAMMER, P_SKILLED },
    { P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },
    { P_MACE, P_BASIC },
    { P_DART, P_SKILLED },
    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_SKILLED },
    { P_SLING, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};

static const struct def_skill Skill_H[] = {
    { P_DAGGER, P_SKILLED },
    { P_KNIFE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },
    { P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_UNICORN_HORN, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_K[] = {
    { P_DAGGER, P_BASIC },
    { P_KNIFE, P_BASIC },
    { P_AXE, P_SKILLED },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_SKILLED },
    { P_SCIMITAR, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_ATTACK_SPELL, P_SKILLED },
    { P_HEALING_SPELL, P_SKILLED },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_RIDING, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mon[] = {
    { P_QUARTERSTAFF, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_CROSSBOW, P_BASIC },
    { P_SHURIKEN, P_BASIC },
    { P_ATTACK_SPELL, P_BASIC },
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_BASIC },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_P[] = {
    { P_CLUB, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },
    { P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },
    { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },
    { P_DART, P_BASIC },
    { P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },
    { P_UNICORN_HORN, P_SKILLED },
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_R[] = {
    { P_DAGGER, P_EXPERT },
    { P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ran[] = {
    { P_DAGGER, P_EXPERT },
    { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_BASIC },
    { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_SKILLED },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_BASIC },
    { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ESCAPE_SPELL, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_S[] = {
    { P_DAGGER, P_BASIC },
    { P_KNIFE,  P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },
    { P_FLAIL, P_SKILLED },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },
    { P_LANCE, P_SKILLED },
    { P_BOW, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },
    { P_ATTACK_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_BASIC }, /* special spell is clairvoyance */
    { P_CLERIC_SPELL, P_SKILLED },
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_T[] = {
    { P_DAGGER, P_EXPERT },
    { P_KNIFE,  P_SKILLED },
    { P_AXE, P_BASIC },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_BASIC },
    { P_LONG_SWORD, P_BASIC },
    { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },
    { P_SABER, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },
    { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },
    { P_WHIP, P_BASIC },
    { P_UNICORN_HORN, P_SKILLED },
    { P_DIVINATION_SPELL, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};

static const struct def_skill Skill_V[] = {
    { P_DAGGER, P_EXPERT },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_SKILLED },
    { P_SLING, P_BASIC },
    { P_ATTACK_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_BASIC },
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_W[] = {
    { P_DAGGER, P_EXPERT },
    { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_BASIC },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },
    { P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },
    { P_ATTACK_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_SKILLED },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};


static void
knows_object(int obj)
{
    discover_object(obj, TRUE, FALSE);
    objects[obj].oc_pre_discovered = 1; /* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
   like all gems except the loadstone and luckstone. */
static void
knows_class(char sym)
{
    struct obj odummy, *o;
    int ct;

    odummy = zeroobj;
    odummy.oclass = sym;
    o = &odummy; /* for use in various obj.h macros */

    /*
     * Note:  the exceptions here can be bypassed if necessary by
     *        calling knows_object() directly.  So an elven ranger,
     *        for example, knows all elven weapons despite the bow,
     *        arrow, and spear limitation below.
     */

    for (ct = bases[(uchar) sym]; ct < bases[(uchar) sym + 1]; ct++) {
        /* not flagged as magic but shouldn't be pre-discovered */
        if (ct == CORNUTHAUM || ct == DUNCE_CAP) {
            continue;
        }
        if (sym == WEAPON_CLASS) {
            odummy.otyp = ct; /* update 'o' */
            /* arbitrary: only knights and samurai recognize polearms */
            if ((!Role_if(PM_KNIGHT) && !Role_if(PM_SAMURAI)) && is_pole(o)) {
                continue;
            }
            /* rangers know all launchers (bows, &c), ammo (arrows, &c),
               and spears regardless of race/species, but not other weapons */
            if (Role_if(PM_RANGER) &&
                (!is_launcher(o) && !is_ammo(o) && !is_spear(o))) {
                continue;
            }
            /* rogues know daggers, regardless of racial variations */
            if (Role_if(PM_ROGUE) && (objects[o->otyp].oc_skill != P_DAGGER)) {
                continue;
            }
        }

        if (objects[ct].oc_class == sym && !objects[ct].oc_magic &&
             !Is_dragon_armor(ct)) {
            knows_object(ct);
        }
    }
}

void
u_init(void)
{
    int i;
    struct permonst* shambler = &mons[PM_SHAMBLING_HORROR];
    struct attack* attkptr;

    flags.female = flags.initgend;
    flags.beginner = 1;

    /* zero u, including pointer values --
     * necessary when aborting from a failed restore */
    (void) memset((genericptr_t)&u, 0, sizeof(u));
    u.ustuck = (struct monst *)0;
    memset(&u.ubirthday, 0, sizeof(u.ubirthday));

#if 0   /* documentation of more zero values as desirable */
    u.usick_cause[0] = 0;
    u.uluck  = u.moreluck = 0;
    uarmu = 0;
    uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
    uwep = uball = uchain = uleft = uright = 0;
    uswapwep = uquiver = 0;
    u.twoweap = 0;
    u.ublessed = 0;             /* not worthy yet */
    u.ugangr   = 0;             /* gods not angry */
    u.ugifts   = 0;             /* no divine gifts bestowed */
    u.uevent.uhand_of_elbereth = 0;
    u.uevent.uheard_tune = 0;
    u.uevent.uopened_dbridge = 0;
    u.uevent.udemigod = 0;      /* not a demi-god yet... */
    u.udg_cnt = 0;
    u.mh = u.mhmax = u.mtimedone = 0;
    u.uz.dnum = u.uz0.dnum = 0;
    u.utotype = 0;
#endif  /* 0 */

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

    u.ulevel = 0;   /* set up some of the initial attributes */
    u.uhp = u.uhpmax = newhp();
    u.uen = u.uenmax = newpw();
    u.uspellprot = 0;
    adjabil(0, 1);
    u.ulevel = u.ulevelmax = 1;

    /*
     * u.roleplay should be treated similar to gender and alignment
     *   - it gets set at character creation
     *   - it's hard to change in-game
     *     (e.g. a special NPC could teach literacy somewhere)
     * the initialisation has to be in front of food, alignment
     * and inventory.
     */
    u.roleplay.ascet    = flags.ascet;
    u.roleplay.atheist  = flags.atheist;
    u.roleplay.blindfolded  = flags.blindfolded;
    u.roleplay.illiterate   = flags.illiterate;
    u.roleplay.pacifist = flags.pacifist;
    u.roleplay.nudist   = flags.nudist;
    u.roleplay.vegan    = flags.vegan;
    u.roleplay.vegetarian   = flags.vegetarian;

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

    if (marathon_mode) {
        u.uhp = u.uhpmax = 999;
    }

    init_uhunger();
    for (i = 0; i <= MAXSPELL; i++) {
        spl_book[i].sp_id = NO_SPELL;
    }
    u.ublesscnt = 300;          /* no prayers just yet */
    u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type =
        aligns[flags.initalign].value;
    u.ulycn = NON_PM;

    u.ubirthday = current_epoch();

    /*
     *  For now, everyone starts out with a night vision range of 1 and
     *  their xray range disabled.
     */
    u.nv_range   =  1;
    u.xray_range = -1;
    u.unblind_telepat_range = -1;

    /*** Role-specific initializations ***/
    switch (Role_switch) {
    /* rn2(100) > 50 necessary for some choices because some
     * random number generators are bad enough to seriously
     * skew the results if we use rn2(2)...  --KAA
     */
    case PM_ARCHEOLOGIST:
        ini_inv(Archeologist);
        if (!rn2(10)) {
            ini_inv(Tinopener);
        } else if (!rn2(4)) {
            ini_inv(Lamp);
        } else if (!rn2(10)) {
            ini_inv(Magicmarker);
        }
        knows_object(SACK);
        knows_object(TOUCHSTONE);
        skill_init(Skill_A);
        break;

    case PM_BARBARIAN:
        if (rn2(100) >= 50) {   /* see above comment */
            Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
            Barbarian[B_MINOR].trotyp = SHORT_SWORD;
        }
        ini_inv(Barbarian);
        if (!rn2(6)) {
            ini_inv(Lamp);
        }
        knows_class(WEAPON_CLASS); /* excluding polearms */
        knows_class(ARMOR_CLASS);
        skill_init(Skill_B);
        break;

    case PM_CAVEMAN:
        Cave_man[C_AMMO].trquan = rn1(11, 10);  /* 10..20 */
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
        u.umoney0 = rn1(1000, 1001);
        ini_inv(Healer);
        if (!rn2(25)) {
            ini_inv(Lamp);
        }
        knows_object(POT_FULL_HEALING);
        skill_init(Skill_H);
        break;

    case PM_KNIGHT:
        ini_inv(Knight);
        knows_class(WEAPON_CLASS); /* all weapons */
        knows_class(ARMOR_CLASS);
        /* give knights chess-like mobility--idea from wooledge@..cwru.edu */
        HJumping |= FROMOUTSIDE;
        skill_init(Skill_K);
        break;

    case PM_MONK: {
        static short M_spell[] = { SPE_HEALING, SPE_PROTECTION, SPE_CONFUSE_MONSTER };

        Monk[M_BOOK].trotyp = M_spell[rn2(90) / 30]; /* [0..2] */
        ini_inv(Monk);
        if (!rn2(5)) {
            ini_inv(Magicmarker);
        } else if (!rn2(10)) {
            ini_inv(Lamp);
        }
        knows_class(ARMOR_CLASS);
        /* sufficiently martial-arts oriented item to ignore language issue */
        knows_object(SHURIKEN);
        skill_init(Skill_Mon);
        break;
    }

    case PM_PRIEST:
        if (Race_switch == PM_ELF) {
            Priest[PRI_MACE].trotyp = CLUB;
        }
        ini_inv(Priest);
        if (!rn2(10)) {
            ini_inv(Magicmarker);
        } else if (!rn2(10)) {
            ini_inv(Lamp);
        }
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
        knows_class(WEAPON_CLASS); /* bows, arrows, spears only */
        skill_init(Skill_Ran);
        break;

    case PM_ROGUE:
        Rogue[R_DAGGERS].trquan = rn1(10, 6);
        u.umoney0 = 0;
        ini_inv(Rogue);
        if (!rn2(5)) {
            ini_inv(Blindfold);
        }
        knows_object(SACK);
        knows_class(WEAPON_CLASS); /* daggers only */
        skill_init(Skill_R);
        break;

    case PM_SAMURAI:
        Samurai[S_ARROWS].trquan = rn1(20, 26);
        ini_inv(Samurai);
        if (!rn2(5)) {
            ini_inv(Blindfold);
        }
        knows_class(WEAPON_CLASS); /* all weapons */
        knows_class(ARMOR_CLASS);
        skill_init(Skill_S);
        break;

    case PM_TOURIST:
        Tourist[T_DARTS].trquan = rn1(43, 21); /* 63 darts ought to be enough for anybody. */
        u.umoney0 = rnd(1000);
        ini_inv(Tourist);
        if (!rn2(25)) {
            ini_inv(Tinopener);
        } else if (!rn2(25)) {
            ini_inv(Leash);
        } else if (!rn2(25) && !towelday()) {
            ini_inv(Towel);
        } else if (!rn2(25)) {
            ini_inv(Magicmarker);
        }
        skill_init(Skill_T);
        break;

    case PM_VALKYRIE:
        ini_inv(Valkyrie);
        if (!rn2(6)) {
            ini_inv(Lamp);
        }
        knows_class(WEAPON_CLASS); /* excludes polearms */
        knows_class(ARMOR_CLASS);
        skill_init(Skill_V);
        break;

    case PM_WIZARD:
        ini_inv(Wizard);
        if (!rn2(5)) {
            ini_inv(Magicmarker);
        }
        if (!rn2(5)) {
            ini_inv(Blindfold);
        }
        skill_init(Skill_W);
        break;

    default:    /* impossible */
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
        if (!Role_if(PM_WIZARD)) {
            if (!Role_if(PM_CONVICT)) {
                ini_inv(Xtra_food);
            }
        }
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

    default:    /* impossible */
        warning("u_init: unknown race %d", Race_switch);
        break;
    }

    /* Towel Day: In Memoriam Douglas Adams */
    if (towelday()) {
        ini_inv(Towel);
    }

    /*** Conduct specific initialisation ***/

    if (u.roleplay.blindfolded) {
        if (!ublindf) {
            ini_inv(Blindfold);
        }
    } else {
        violated(CONDUCT_BLINDFOLDED);
    }
    if (u.roleplay.atheist) {
        u.ugangr++;
    }

    if (discover) {
        ini_inv(Wishing);
    }

#ifdef WIZARD
    if (wizard) {
        read_wizkit();
    }
#endif

    if (u.umoney0) {
        ini_inv(Money);
    }
    u.umoney0 += hidden_gold(); /* in case sack has gold in it */

    find_ac();          /* get initial ac value */
    init_attr(75);          /* init attribute values */
    max_rank_sz();          /* set max str size for class ranks */
/*
 *  Do we really need this?
 */
    for (i = 0; i < A_MAX; i++) {
        if (!rn2(20)) {
            int xd = rn2(7) - 2; /* biased variation */
            (void) adjattrib(i, xd, TRUE);
            if (ABASE(i) < AMAX(i)) {
                AMAX(i) = ABASE(i);
            }
        }
    }

    /* make sure you can carry all you have - especially for Tourists */
    while (inv_weight() > 0) {
        if (adjattrib(A_STR, 1, TRUE)) {
            continue;
        }
        if (adjattrib(A_CON, 1, TRUE)) {
            continue;
        }
        /* only get here when didn't boost strength or constitution */
        break;
    }

    /* what a horrible night to have a curse */
    shambler->mlevel += rnd(12)-3;              /* shuffle level */
    shambler->mmove = rn2(10)+9;                /* slow to very fast */
    shambler->ac = rn2(21)-10;              /* any AC */
    shambler->mr = rn2(5)*25;               /* varying amounts of MR */
    shambler->maligntyp = rn2(21)-10;           /* any alignment */
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
        attkptr->damn = 2;              /* we're almost sure to get this wrong first time */
        attkptr->damd = 10;             /* either too high or too low */
    }
    shambler->msize = rn2(MZ_GIGANTIC+1);           /* any size */
    shambler->cwt = 20;                 /* fortunately moot as it's flagged NOCORPSE */
    shambler->cnutrit = 20;                 /* see above */
    shambler->msound = rn2(MS_HUMANOID);            /* any but the specials */
    shambler->mresists = 0;
    for (i = 0; i < rnd(6); i++) {
        shambler->mresists |= (1 << rn2(8));        /* physical resistances... */
    }
    for (i = 0; i < rnd(5); i++) {
        shambler->mresists |= (0x100 << rn2(7));    /* 'different' resistances, even clumsy */
    }
    shambler->mconveys = 0;                 /* flagged NOCORPSE */
    /*
     * now time for the random flags.  this will likely produce
     * a number of complete trainwreck monsters at first, but
     * every so often something will dial up nasty stuff
     */
    shambler->mflags1 = 0;
    for (i = 0; i < rnd(17); i++) {
        shambler->mflags1 |= (1 << rn2(31));        /* trainwreck this way :D */
    }
    shambler->mflags1 &= ~M1_UNSOLID;           /* no ghosts */
    shambler->mflags1 &= ~M1_WALLWALK;          /* no wall-walkers */

    shambler->mflags2 = M2_NOPOLY | M2_HOSTILE;     /* Don't let the player be one of these yet. */
    for (i = 0; i < rnd(17); i++) {
        shambler->mflags2 |= (1 << rn2(31));
    }
    shambler->mflags2 &= ~M2_MERC;              /* no guards */
    shambler->mflags2 &= ~M2_PEACEFUL;          /* no peacefuls */
    shambler->mflags2 &= ~M2_WERE;              /* no lycanthropes */
    shambler->mflags2 &= ~M2_PNAME;             /* not a proper name */

    /* If we have at least one spell, force starting Pw to be enough,
       so hero can cast the level 1 spell they should have */
    if (num_spells() && (u.uenmax < SPELL_LEV_PW(1))) {
        u.uen = u.uenmax = u.ueninc[u.ulevel] = SPELL_LEV_PW(1);
    }
}

/* skills aren't initialized, so we use the role-specific skill lists */
static boolean
restricted_spell_discipline(int otyp)
{
    const struct def_skill *skills;
    int this_skill = spell_skilltype(otyp);

    switch (Role_switch) {
    case PM_ARCHEOLOGIST:  skills = Skill_A; break;
    case PM_BARBARIAN:     skills = Skill_B; break;
    case PM_CAVEMAN:       skills = Skill_C; break;
    case PM_CONVICT:       skills = Skill_Con; break;
    case PM_HEALER:        skills = Skill_H; break;
    case PM_KNIGHT:        skills = Skill_K; break;
    case PM_MONK:      skills = Skill_Mon; break;
    case PM_PRIEST:        skills = Skill_P; break;
    case PM_RANGER:        skills = Skill_Ran; break;
    case PM_ROGUE:     skills = Skill_R; break;
    case PM_SAMURAI:       skills = Skill_S; break;
    case PM_TOURIST:       skills = Skill_T; break;
    case PM_VALKYRIE:      skills = Skill_V; break;
    case PM_WIZARD:        skills = Skill_W; break;
    default:           skills = 0; break;   /* lint suppression */
    }

    while (skills && skills->skill != P_NONE) {
        if (skills->skill == this_skill) {
            return FALSE;
        }
        ++skills;
    }
    return TRUE;
}

static void
ini_inv(struct trobj *trop)
{
    struct obj *obj;
    int otyp, i;
    boolean got_sp1 = FALSE; /* got a level 1 spellbook? */

    while (trop->trclass) {
        otyp = (int) trop->trotyp;
        if (otyp != UNDEF_TYP) {
            obj = mksobj(otyp, TRUE, FALSE);
        } else { /* UNDEF_TYP */
            static NEARDATA short nocreate = STRANGE_OBJECT;
            static NEARDATA short nocreate2 = STRANGE_OBJECT;
            static NEARDATA short nocreate3 = STRANGE_OBJECT;
            static NEARDATA short nocreate4 = STRANGE_OBJECT;
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
                   /* orcs start with poison resistance */
                   || (otyp == RIN_POISON_RESISTANCE && Race_if(PM_ORC))
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
                       (objects[otyp].oc_level > (got_sp1 ? 3 : 1) ||
                        restricted_spell_discipline(otyp)))) {
                dealloc_obj(obj);
                obj = mkobj(trop->trclass, FALSE);
                otyp = obj->otyp;
            }

            /* Don't start with +0 or negative rings */
            if (objects[otyp].oc_charged && obj->spe <= 0) {
                obj->spe = rne(3);
            }

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

            /* First spellbook should be level 1 - did we get it? */
            if (obj->oclass == SPBOOK_CLASS && objects[obj->otyp].oc_level == 1) {
                got_sp1 = TRUE;
            }
        }

        if (urace.malenum != PM_HUMAN) {
            /* substitute race-specific items; this used to be in
               the 'if (otyp != UNDEF_TYP) { }' block above, but then
               substitutions didn't occur for randomly generated items
               (particularly food) which have racial substitutes */
            for (i = 0; inv_subs[i].race_pm != NON_PM; ++i) {
                if (inv_subs[i].race_pm == urace.malenum &&
                    otyp == inv_subs[i].item_otyp) {
                    debug_pline("ini_inv: substituting %s for %s%s",
                                OBJ_NAME(objects[inv_subs[i].subs_otyp]),
                                (trop->trotyp == UNDEF_TYP) ? "random " : "",
                                OBJ_NAME(objects[otyp]));
                    otyp = obj->otyp = inv_subs[i].subs_otyp;
                    obj->oclass = objects[otyp].oc_class;
                    break;
                }
            }
        }

        if (trop->trclass == COIN_CLASS) {
            /* no "blessed" or "identified" money */
            obj->quan = u.umoney0;
        } else {
        obj->dknown = obj->bknown = obj->rknown = 1;
        if (objects[otyp].oc_uses_known) {
            obj->known = 1;
        }
        obj->cursed = 0;
        if (obj->opoisoned && u.ualign.type != A_CHAOTIC) {
            obj->opoisoned = 0;
        }
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
        if (trop->trspe != UNDEF_SPE) {
            obj->spe = trop->trspe;
        }
        if (trop->trbless != UNDEF_BLESS) {
            obj->blessed = trop->trbless;
        }
    }
        /* defined after setting otyp+quan + blessedness */
        obj->owt = weight(obj);
        obj = addinv(obj);

        /* Make the type known if necessary */
        if (OBJ_DESCR(objects[otyp]) && obj->known) {
            discover_object(otyp, TRUE, FALSE);
        }
        if (otyp == OIL_LAMP) {
            discover_object(POT_OIL, TRUE, FALSE);
        }

        if ((obj->oclass == ARMOR_CLASS && !u.roleplay.nudist)) {
            if (is_shield(obj) && !uarms && !(uwep && bimanual(uwep))) {
                setworn(obj, W_ARMS);
                /* 3.6.2: this used to unset uswapwep if it was set, but
                   wearing a shield doesn't prevent having an alternate
                   weapon ready to swap with the primary; just make sure we
                   aren't two-weaponing (academic; no one starts that way) */
                u.twoweap = FALSE;
            } else if (is_helmet(obj) && !uarmh) {
                setworn(obj, W_ARMH);
            } else if (is_gloves(obj) && !uarmg) {
                setworn(obj, W_ARMG);
            } else if (is_shirt(obj) && !uarmu) {
                setworn(obj, W_ARMU);
            } else if (is_cloak(obj) && !uarmc) {
                setworn(obj, W_ARMC);
            } else if (is_boots(obj) && !uarmf) {
                setworn(obj, W_ARMF);
            } else if (is_suit(obj) && !uarm) {
                setworn(obj, W_ARM);
            }
        }

        if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
            otyp == TIN_OPENER || otyp == FLINT || otyp == ROCK) {
            if (is_ammo(obj) || is_missile(obj)) {
                if (!uquiver) {
                    setuqwep(obj);
                }
            } else if (!uwep && !u.roleplay.pacifist && (!uarms || !bimanual(obj))) {
                setuwep(obj);
            } else if (!uswapwep) {
                setuswapwep(obj);
            }
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

        if (--trop->trquan) {
            continue; /* make a similar object */
        }
        trop++;
    }
}

/*u_init.c*/
