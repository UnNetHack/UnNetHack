/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/*** Table of all roles ***/
/* According to AD&D, HD for some classes (ex. Wizard) should be smaller
 * (4-sided for wizards).  But this is not AD&D, and using the AD&D
 * rule here produces an unplayable character.  Thus I have used a minimum
 * of an 10-sided hit die for everything.  Another AD&D change: wizards get
 * a minimum strength of 4 since without one you can't teleport or cast
 * spells. --KAA
 *
 * As the wizard has been updated (wizard patch 5 jun '96) their HD can be
 * brought closer into line with AD&D. This forces wizards to use magic more
 * and distance themselves from their attackers. --LSZ
 *
 * With the introduction of races, some hit points and energy
 * has been reallocated for each race.  The values assigned
 * to the roles has been reduced by the amount allocated to
 * humans.  --KMH
 *
 * God names use a leading underscore to flag goddesses.
 */
const struct Role roles[] = {
    {   {"Archeologist", 0}, {
            {"Digger",      0},
            {"Field Worker", 0},
            {"Investigator", 0},
            {"Exhumer",     0},
            {"Excavator",   0},
            {"Spelunker",   0},
            {"Speleologist", 0},
            {"Collector",   0},
            {"Curator",     0}
        },
        "Quetzalcoatl", "Camaxtli", "Huhetotl", /* Central American */
        "Arc", "the College of Archeology", "the Tomb of the Toltec Kings",
        PM_ARCHEOLOGIST, NON_PM, NON_PM,
        PM_LORD_CARNARVON,
        PM_STUDENT,
        PM_SCHLIEMANN,
        ART_ITLACHIAYAQUE,
        MH_HUMAN | MH_DWARF | MH_GNOME | MH_VAMPIRE | ROLE_MALE | ROLE_FEMALE |
         ROLE_LAWFUL | ROLE_NEUTRAL,
        /* Str Int Wis Dex Con Cha */
        {   7, 10, 10,  7,  7,  7 },
        {  20, 20, 20, 10, 20, 10 },
        /* Init   Lower  Higher */
        { 11, 0,  0, 8,  1, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 14,/* Energy */
        10, 5, 0, 2, 10, A_INT, SPE_MAGIC_MAPPING,   -4},
    {   {"Barbarian", 0}, {
            {"Plunderer",   "Plunderess"},
            {"Pillager",    0},
            {"Bandit",      0},
            {"Brigand",     0},
            {"Raider",      0},
            {"Reaver",      0},
            {"Slayer",      0},
            {"Chieftain",   "Chieftainess"},
            {"Conqueror",   "Conqueress"}
        },
        "Mitra", "Crom", "Set", /* Hyborian */
        "Bar", "the Camp of the Duali Tribe", "the Duali Oasis",
        PM_BARBARIAN, NON_PM, NON_PM,
        PM_PELIAS, PM_CHIEFTAIN, PM_THOTH_AMON,
        ART_HEART_OF_AHRIMAN,
        MH_HUMAN|MH_ORC|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
        ROLE_NEUTRAL|ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {  16,  7,  7, 15, 16,  6 },
        {  30,  6,  7, 20, 30,  7 },
        /* Init   Lower  Higher */
        { 14, 0,  0, 10,  2, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 10,/* Energy */
        10, 14, 0, 0,  8, A_INT, SPE_HASTE_SELF,      -4},
    {   {"Caveman", "Cavewoman"}, {
            {"Troglodyte",  0},
            {"Aborigine",   0},
            {"Wanderer",    0},
            {"Vagrant",     0},
            {"Wayfarer",    0},
            {"Roamer",      0},
            {"Nomad",       0},
            {"Rover",       0},
            {"Pioneer",     0}
        },
        "Anu", "_Ishtar", "Anshar", /* Babylonian */
        "Cav", "the Caves of the Ancestors", "the Dragon's Lair",
        PM_CAVEMAN, PM_CAVEWOMAN, NON_PM,
        PM_SHAMAN_KARNOV, PM_NEANDERTHAL, PM_TIAMAT,
        ART_SCEPTRE_OF_MIGHT,
        MH_HUMAN|MH_DWARF|MH_GNOME|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
        ROLE_LAWFUL|ROLE_NEUTRAL,
        /* Str Int Wis Dex Con Cha */
        {  10,  7,  7,  7,  8,  6 },
        {  30,  6,  7, 20, 30,  7 },
        /* Init   Lower  Higher */
        { 14, 0,  0, 8,  2, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 10,/* Energy */
        0, 12, 0, 1,  8, A_INT, SPE_DIG,             -4},
    {   {"Convict", 0}, {
            {"Detainee",     0},
            {"Inmate",   0},
            {"Jail-bird", 0},
            {"Prisoner", 0},
            {"Outlaw",    0},
            {"Crook",   0},
            {"Desperado",     0},
            {"Felon",    0},
            {"Fugitive",  0}
        },
        "Ilmater", "Grumbar", "_Tymora", /* Faerunian */
        "Con", "Castle Waterdeep Dungeon", "the Warden's Level",
        PM_CONVICT, NON_PM, PM_SEWER_RAT,
        PM_ROBERT_THE_LIFER, PM_INMATE, PM_WARDEN_ARIANNA,
        /*PM_GIANT_BEETLE, PM_SOLDIER_ANT, S_RODENT, S_SPIDER,*/
        ART_IRON_BALL_OF_LIBERATION,
        MH_HUMAN|MH_DWARF|MH_GNOME|MH_ORC|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
        ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {  10,  7,  7,  7, 13,  6 },
        {  20, 20, 10, 20, 20, 10 },
        /* Init   Lower  Higher */
        {  8, 0,  0, 8,  0, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 10,/* Energy */
        -10, 5, 0, 2, 10, A_INT, SPE_TELEPORT_AWAY,   -4},
    {   {"Healer", 0}, {
            {"Rhizotomist",    0},
            {"Empiric",        0},
            {"Embalmer",       0},
            {"Dresser",        0},
            {"Medicus ossium", "Medica ossium"},
            {"Herbalist",      0},
            {"Magister",       "Magistra"},
            {"Physician",      0},
            {"Chirurgeon",     0}
        },
        "_Athena", "Hermes", "Poseidon", /* Greek */
        "Hea", "the Temple of Epidaurus", "the Temple of Coeus",
        PM_HEALER, NON_PM, NON_PM,
        PM_HIPPOCRATES, PM_ATTENDANT, PM_CYCLOPS,
        ART_STAFF_OF_AESCULAPIUS,
        MH_HUMAN|MH_GNOME | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL,
        /* Str Int Wis Dex Con Cha */
        {   7,  7, 13,  7, 11, 16 },
        {  15, 20, 20, 15, 25, 5 },
        /* Init   Lower  Higher */
        { 11, 0,  0, 8,  1, 0 },/* Hit points */
        {  1, 4,  0, 1,  0, 2 }, 20,/* Energy */
        10, 3, -3, 2, 10, A_WIS, SPE_CURE_SICKNESS,   -4},
    {   {"Knight", 0}, {
            {"Gallant",     0},
            {"Esquire",     0},
            {"Bachelor",    0},
            {"Sergeant",    0},
            {"Knight",      0},
            {"Banneret",    0},
            {"Chevalier",   "Chevaliere"},
            {"Seignieur",   "Dame"},
            {"Paladin",     0}
        },
        "Lugh", "_Brigit", "Manannan Mac Lir", /* Celtic */
        "Kni", "Camelot Castle", "the Isle of Glass",
        PM_KNIGHT, NON_PM, PM_PONY,
        PM_KING_ARTHUR, PM_PAGE, PM_IXOTH,
        ART_MAGIC_MIRROR_OF_MERLIN,
        MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
        /* Str Int Wis Dex Con Cha */
        {  13,  7, 14,  8, 10, 17 },
        {  30, 15, 15, 10, 20, 10 },
        /* Init   Lower  Higher */
        { 14, 0,  0, 8,  2, 0 },/* Hit points */
        {  1, 4,  0, 1,  0, 2 }, 10,/* Energy */
        10, 8, -2, 0,  9, A_WIS, SPE_TURN_UNDEAD,     -4},
    {   {"Monk", 0}, {
            {"Candidate",         0},
            {"Novice",            0},
            {"Initiate",          0},
            {"Student of Stones", 0},
            {"Student of Waters", 0},
            {"Student of Metals", 0},
            {"Student of Winds",  0},
            {"Student of Fire",   0},
            {"Master",            0}
        },
        "Shan Lai Ching", "Chih Sung-tzu", "Huan Ti", /* Chinese */
        "Mon", "the Monastery of Chan-Sune",
        "the Monastery of the Earth-Lord",
        PM_MONK, NON_PM, NON_PM,
        PM_GRAND_MASTER, PM_ABBOT, PM_MASTER_KAEN,
        ART_EYES_OF_THE_OVERWORLD,
        MH_HUMAN | ROLE_MALE|ROLE_FEMALE |
        ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {  10,  7,  8,  8,  7,  7 },
        {  25, 10, 20, 20, 15, 10 },
        /* Init   Lower  Higher */
        { 12, 0,  0, 8,  1, 0 },/* Hit points */
        {  2, 2,  0, 2,  0, 2 }, 10,/* Energy */
        10, 8, -2, 2, 20, A_WIS, SPE_RESTORE_ABILITY, -4},
    {   {"Priest", "Priestess"}, {
            {"Aspirant",    0},
            {"Acolyte",     0},
            {"Adept",       0},
            {"Priest",      "Priestess"},
            {"Curate",      0},
            {"Canon",       "Canoness"},
            {"Lama",        0},
            {"Patriarch",   "Matriarch"},
            {"High Priest", "High Priestess"}
        },
        0, 0, 0, /* chosen randomly from among the other roles */
        "Pri", "the Great Temple", "the Temple of Nalzok",
        PM_PRIEST, PM_PRIESTESS, NON_PM,
        PM_ARCH_PRIEST, PM_ACOLYTE, PM_NALZOK,
        ART_MITRE_OF_HOLINESS,
        MH_HUMAN|MH_ELF | ROLE_MALE|ROLE_FEMALE |
        ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {   7,  7, 10,  7,  7,  7 },
        {  15, 10, 30, 15, 20, 10 },
        /* Init   Lower  Higher */
        { 12, 0,  0, 8,  1, 0 },/* Hit points */
        {  4, 3,  0, 2,  0, 2 }, 10,/* Energy */
        0, 3, -2, 2, 10, A_WIS, SPE_REMOVE_CURSE,    -4},
    /* Note:  Rogue precedes Ranger so that use of `-R' on the command line
       retains its traditional meaning. */
    {   {"Rogue", 0}, {
            {"Footpad",     0},
            {"Cutpurse",    0},
            {"Rogue",       0},
            {"Pilferer",    0},
            {"Robber",      0},
            {"Burglar",     0},
            {"Filcher",     0},
            {"Magsman",     "Magswoman"},
            {"Thief",       0}
        },
        "Issek", "Mog", "Kos", /* Nehwon */
        "Rog", "the Thieves' Guild Hall", "the Assassins' Guild Hall",
        PM_ROGUE, NON_PM, NON_PM,
        PM_MASTER_OF_THIEVES, PM_THUG, PM_MASTER_ASSASSIN,
        ART_MASTER_KEY_OF_THIEVERY,
        MH_HUMAN|MH_ORC|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
        ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {   7,  7,  7, 10,  7,  6 },
        {  20, 10, 10, 30, 20, 10 },
        /* Init   Lower  Higher */
        { 10, 0,  0, 8,  1, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 11,/* Energy */
        10, 8, 0, 1,  9, A_INT, SPE_DETECT_TREASURE, -4},
    {   {"Ranger", 0}, {
#if 0   /* OBSOLETE */
            {"Edhel",       "Elleth"},
            {"Edhel",       "Elleth"},/* elf-maid */
            {"Ohtar",       "Ohtie"},/* warrior */
            {"Kano",    /* commander (Q.) ['a] */
         "Kanie"},      /* educated guess, until further research- SAC */
            {"Arandur", /* king's servant, minister (Q.) - guess */
         "Aranduriel"},     /* educated guess */
            {"Hir",         "Hiril"},/* lord, lady (S.) ['ir] */
            {"Aredhel",     "Arwen"},/* noble elf, maiden (S.) */
            {"Ernil",       "Elentariel"},/* prince (S.), elf-maiden (Q.) */
            {"Elentar",     "Elentari"},/* Star-king, -queen (Q.) */
            "Solonor Thelandira", "Aerdrie Faenya", "Lolth", /* Elven */
#endif
            {"Tenderfoot",    0},
            {"Lookout",       0},
            {"Trailblazer",   0},
            {"Reconnoiterer", "Reconnoiteress"},
            {"Scout",         0},
            {"Arbalester",    0},/* One skilled at crossbows */
            {"Archer",        0},
            {"Sharpshooter",  0},
            {"Marksman",      "Markswoman"}
        },
        "Mercury", "_Venus", "Mars", /* Roman/planets */
        "Ran", "Orion's camp", "the cave of the wumpus",
        PM_RANGER, NON_PM, NON_PM /* Orion & canis major */,
        PM_ORION, PM_HUNTER, PM_SCORPIUS,
        ART_LONGBOW_OF_DIANA,
        MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC | ROLE_MALE|ROLE_FEMALE |
        ROLE_NEUTRAL|ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {  13, 13, 13,  9, 13,  7 },
        {  30, 10, 10, 20, 20, 10 },
        /* Init   Lower  Higher */
        { 13, 0,  0, 6,  1, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 12,/* Energy */
        10, 9, 2, 1, 10, A_INT, SPE_INVISIBILITY,   -4},
    {   {"Samurai", 0}, {
            {"Hatamoto",    0},/* Banner Knight */
            {"Ronin",       0},/* no allegiance */
            {"Ninja",       "Kunoichi"},/* secret society */
            {"Joshu",       0},/* heads a castle */
            {"Ryoshu",      0},/* has a territory */
            {"Kokushu",     0},/* heads a province */
            {"Daimyo",      0},/* a samurai lord */
            {"Kuge",        0},/* Noble of the Court */
            {"Shogun",      0}
        },               /* supreme commander, warlord */
        "_Amaterasu Omikami", "Raijin", "Susanowo", /* Japanese */
        "Sam", "the Castle of the Taro Clan", "the Shogun's Castle",
        PM_SAMURAI, NON_PM, PM_LITTLE_DOG,
        PM_LORD_SATO, PM_ROSHI, PM_ASHIKAGA_TAKAUJI,
        ART_TSURUGI_OF_MURAMASA,
        MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
        /* Str Int Wis Dex Con Cha */
        {  10,  8,  7, 10, 17,  6 },
        {  30, 10,  8, 30, 14,  8 },
        /* Init   Lower  Higher */
        { 13, 0,  0, 8,  1, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 11,/* Energy */
        10, 10, 0, 0,  8, A_INT, SPE_CLAIRVOYANCE,    -4},
    {   {"Tourist", 0}, {
            {"Rambler",     0},
            {"Sightseer",   0},
            {"Excursionist", 0},
            {"Peregrinator", "Peregrinatrix"},
            {"Traveler",    0},
            {"Journeyer",   0},
            {"Voyager",     0},
            {"Explorer",    0},
            {"Adventurer",  0}
        },
        "Blind Io", "_The Lady", "Offler", /* Discworld */
        "Tou", "Ankh-Morpork", "the Thieves' Guild Hall",
        PM_TOURIST, NON_PM, NON_PM,
        PM_TWOFLOWER, PM_GUIDE, PM_MASTER_OF_THIEVES,
        ART_YENDORIAN_EXPRESS_CARD,
        MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL,
        /* Str Int Wis Dex Con Cha */
        {   7, 10,  6,  7,  7, 10 },
        {  15, 10, 10, 15, 30, 20 },
        /* Init   Lower  Higher */
        {  8, 0,  0, 8,  0, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 14,/* Energy */
        0, 5, 1, 2, 10, A_INT, SPE_CHARM_MONSTER,   -4},
    {   {"Valkyrie", 0}, {
            {"Stripling",   0},
            {"Skirmisher",  0},
            {"Fighter",     0},
            {"Man-at-arms", "Woman-at-arms"},
            {"Warrior",     0},
            {"Swashbuckler", 0},
            {"Hero",        "Heroine"},
            {"Champion",    0},
            {"Lord",        "Lady"}
        },
        "Tyr", "Odin", "Loki", /* Norse */
        "Val",
        "the Shrine of Destiny",
        "the Shattered Bridge",
        PM_VALKYRIE, NON_PM, NON_PM /*PM_WINTER_WOLF_CUB*/,
        PM_NORN, PM_WARRIOR, PM_LORD_SURTUR,
        ART_ORB_OF_FATE,
        MH_HUMAN|MH_DWARF | ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL,
        /* Str Int Wis Dex Con Cha */
        {  10,  7,  7,  7, 10,  7 },
        {  30,  6,  7, 20, 30,  7 },
        /* Init   Lower  Higher */
        { 14, 0,  0, 8,  2, 0 },/* Hit points */
        {  1, 0,  0, 1,  0, 1 }, 10,/* Energy */
        0, 10, -2, 0,  9, A_WIS, SPE_CONE_OF_COLD,    -4},
    {   {"Wizard", 0}, {
            {"Evoker",      0},
            {"Conjurer",    0},
            {"Thaumaturge", 0},
            {"Magician",    0},
            {"Enchanter",   "Enchantress"},
            {"Sorcerer",    "Sorceress"},
            {"Necromancer", 0},
            {"Wizard",      0},
            {"Mage",        0}
        },
        "Ptah", "Thoth", "Anhur", /* Egyptian */
        "Wiz", "the Lonely Tower", "the Tower of Darkness",
        PM_WIZARD, NON_PM, PM_KITTEN,
        PM_NEFERET_THE_GREEN, PM_APPRENTICE, PM_ANARAXIS_THE_BLACK,
        ART_EYE_OF_THE_AETHIOPICA,
        MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
        ROLE_NEUTRAL|ROLE_CHAOTIC,
        /* Str Int Wis Dex Con Cha */
        {   7, 10,  7,  7,  7,  7 },
        {  10, 30, 10, 20, 20, 10 },
        /* Init   Lower  Higher */
        { 10, 0,  0, 8,  1, 0 },/* Hit points */
        {  4, 3,  0, 2,  0, 3 }, 12,/* Energy */
        0, 1, 0, 3, 10, A_INT, SPE_MAGIC_MISSILE,   -4},
    /* Array terminator */
    { { 0 }, { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 } },
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0 }, { 0 }, { 0 }, { 0 },
        0, 0, 0, 0, 0, 0, 0, 0, 0 }
};


/* The player's role, created at runtime from initial
 * choices.  This may be munged in role_init().
 */
struct Role urole =
{   {"Undefined", 0}, { {0, 0}, {0, 0}, {0, 0},
                        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
    "L", "N", "C", "Xxx", "home", "locate",
    NON_PM, NON_PM, NON_PM, NON_PM, NON_PM, NON_PM,
    0, 0,
    /* Str Int Wis Dex Con Cha */
    {   7,  7,  7,  7,  7,  7 },
    {  20, 15, 15, 20, 20, 10 },
    /* Init   Lower  Higher */
    { 10, 0,  0, 8,  1, 0 },    /* Hit points */
    {  2, 0,  0, 2,  0, 3 }, 14, /* Energy */
    0, 10, 0, 0,  4, A_INT, 0, -3};



/* Table of all races */
const struct Race races[] = {
    {   "human", "human", "humanity", "Hum",
        {"man", "woman"},
        PM_HUMAN, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
        MH_HUMAN | ROLE_MALE|ROLE_FEMALE |
        ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
        MH_HUMAN, 0, MH_GNOME|MH_ORC,
        /*    Str     Int Wis Dex Con Cha */
        {      3,      3,  3,  3,  3,  3 },
        { STR18(100), 18, 18, 18, 18, 18 },
        /* Init   Lower  Higher */
        {  2, 0,  0, 2,  1, 0 },/* Hit points */
        {  1, 0,  2, 0,  2, 0 } /* Energy */
    },
    {   "elf", "elven", "elvenkind", "Elf",
        {0, 0},
        PM_ELF, NON_PM, PM_ELF_MUMMY, PM_ELF_ZOMBIE,
        MH_ELF | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
        MH_ELF, MH_ELF, MH_ORC,
        /*  Str    Int Wis Dex Con Cha */
        {    3,     3,  3,  3,  3,  3 },
        {   18,    20, 20, 18, 16, 18 },
        /* Init   Lower  Higher */
        {  1, 0,  0, 1,  1, 0 },/* Hit points */
        {  2, 0,  3, 0,  3, 0 } /* Energy */
    },
    {   "dwarf", "dwarven", "dwarvenkind", "Dwa",
        {0, 0},
        PM_DWARF, NON_PM, PM_DWARF_MUMMY, PM_DWARF_ZOMBIE,
        MH_DWARF | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
        MH_DWARF, MH_DWARF|MH_GNOME, MH_ORC,
        /*    Str     Int Wis Dex Con Cha */
        {      3,      3,  3,  3,  3,  3 },
        { STR18(100), 16, 16, 20, 20, 16 },
        /* Init   Lower  Higher */
        {  4, 0,  0, 3,  2, 0 },/* Hit points */
        {  0, 0,  0, 0,  0, 0 } /* Energy */
    },
    {   "gnome", "gnomish", "gnomehood", "Gno",
        {0, 0},
        PM_GNOME, NON_PM, PM_GNOME_MUMMY, PM_GNOME_ZOMBIE,
        MH_GNOME | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL,
        MH_GNOME, MH_DWARF|MH_GNOME, MH_HUMAN,
        /*  Str    Int Wis Dex Con Cha */
        {    3,     3,  3,  3,  3,  3 },
        {STR18(50), 19, 18, 18, 18, 18 },
        /* Init   Lower  Higher */
        {  1, 0,  0, 1,  0, 0 },/* Hit points */
        {  2, 0,  2, 0,  2, 0 } /* Energy */
    },
    {   "orc", "orcish", "orcdom", "Orc",
        {0, 0},
        PM_ORC, NON_PM, PM_ORC_MUMMY, PM_ORC_ZOMBIE,
        MH_ORC | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
        MH_ORC, 0, MH_HUMAN|MH_ELF|MH_DWARF,
        /*  Str    Int Wis Dex Con Cha */
        {   3,      3,  3,  3,  3,  3 },
        {STR18(50), 16, 16, 18, 18, 16 },
        /* Init   Lower  Higher */
        {  1, 0,  0, 1,  0, 0 },/* Hit points */
        {  1, 0,  1, 0,  1, 0 } /* Energy */
    },
    {   "vampire", "vampiric", "vampirehood", "Vam",
        {0, 0},
        PM_VAMPIRE, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
        MH_VAMPIRE | ROLE_MALE | ROLE_FEMALE | ROLE_NEUTRAL | ROLE_CHAOTIC,
        MH_VAMPIRE, 0, MH_ELF|MH_GNOME|MH_DWARF|MH_ORC,
        /*    Str     Int Wis Dex Con Cha */
        {      3,      3,  3,  3,  3,  3 },
        { STR19(19), 18, 18, 20, 20, 20 },
        /* Init   Lower  Higher */
        {  3, 0,  0, 3,  2, 0 },/* Hit points */
        {  3, 0,  4, 0,  4, 0 } /* Energy */
    },
    /* Array terminator */
    { 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0, { 0 }, { 0 }, { 0 }, { 0 } }
};


/* The player's race, created at runtime from initial
 * choices.  This may be munged in role_init().
 */
struct Race urace =
{   "something", "undefined", "something", "Xxx",
    {0, 0},
    NON_PM, NON_PM, NON_PM, NON_PM,
    0, 0, 0, 0,
    /*    Str     Int Wis Dex Con Cha */
    {      3,      3,  3,  3,  3,  3 },
    { STR18(100), 18, 18, 18, 18, 18 },
    /* Init   Lower  Higher */
    {  2, 0,  0, 2,  1, 0 },    /* Hit points */
    {  1, 0,  2, 0,  2, 0 }     /* Energy */
};


/* Table of all genders */
const struct Gender genders[] = {
    {"male",    "he",   "him",  "his",  "Mal",  ROLE_MALE},
    {"female",  "she",  "her",  "her",  "Fem",  ROLE_FEMALE},
    {"neuter",  "it",   "it",   "its",  "Ntr",  ROLE_NEUTER}
};


/* Table of all alignments */
const struct Align aligns[] = {
    {"law",     "lawful",   "Law",  ROLE_LAWFUL,    A_LAWFUL},
    {"balance", "neutral",  "Neu",  ROLE_NEUTRAL,   A_NEUTRAL},
    {"chaos",   "chaotic",  "Cha",  ROLE_CHAOTIC,   A_CHAOTIC},
    {"evil",    "unaligned",    "Una",  0,      A_NONE}
};


/* Table of roleplay-conducts */

const struct Conduct conducts[] = {
    {   "pacifism", "pacifist",     "peaceful", TRUE,
        "You ", "have been ", "were ", "a pacifist",
        "pretended to be a pacifist"},

    {   "sadism",   "sadist",   "sadistic", TRUE,
        "You ", "have been ", "were ", "a sadist",
        "pretended to be a sadist"},

    {   "atheism",  "atheist",  "atheistic",    TRUE,
        "You ", "have been ", "were ", "an atheist",
        "pretended to be an atheist"},

    {   "nudism",   "nudist",   "nude",     TRUE,
        "You ", "have been ", "were ", "a nudist",
        "pretended to be a nudist"},

    {   "zen",      "zen master",   "blindfolded",  TRUE,
        "You ", "have followed ", "followed ", "the true Path of Zen",
        "left the true Path of Zen"},

    {   "asceticism",   "ascetic",  "hungry",   TRUE,
        "You ", "have gone ", "went ", "without food",
        "pretended to be an ascet"},

    {   "vegan",    "vegan",    "vegan",    TRUE,
        "You ", "have followed ", "followed ", "a strict vegan diet",
        "pretended to be a vegan"},

    {   "vegetarian",   "vegetarian",   "vegetarian",   TRUE,
        "You ", "have been ", "were ", "vegetarian",
        "pretended to be a vegetarian"},

    {   "illiteracy",   "illiterate",   "illiterate",   TRUE,
        "You ", "have been ", "were ", "illiterate",
        "become literate"},

    {   "thievery", "master thief", "tricky",   TRUE,
        "You ", "have been ", "were ", "very tricky",
        "pretended to be a master thief"}
};

/* Filters */
static struct {
    boolean roles[SIZE(roles)];
    short mask;
} rfilter;

static int randrole_filtered(void);
static char * promptsep(char *, int);
static int role_gendercount(int);
static int race_alignmentcount(int);

/* used by str2XXX() */
static char NEARDATA randomstr[] = "random";

boolean
validrole(int rolenum)
{
    return (rolenum >= 0 && rolenum < SIZE(roles)-1);
}

int
randrole(void)
{
    return (rn2(SIZE(roles)-1));
}

static int
randrole_filtered(void)
{
    int i, n = 0, set[SIZE(roles)];

    /* this doesn't rule out impossible combinations but attempts to
       honor all the filter masks */
    for (i = 0; i < SIZE(roles) - 1; ++i) { /* -1: avoid terminating element */
        if (ok_role(i, ROLE_NONE, ROLE_NONE, ROLE_NONE) &&
             ok_race(i, ROLE_RANDOM, ROLE_NONE, ROLE_NONE) &&
             ok_gend(i, ROLE_NONE, ROLE_RANDOM, ROLE_NONE) &&
             ok_align(i, ROLE_NONE, ROLE_NONE, ROLE_RANDOM)) {
            set[n++] = i;
        }
    }
    return n ? set[rn2(n)] : randrole();
}

int
str2role(const char *str)
{
    int i, len;

    /* Is str valid? */
    if (!str || !str[0]) {
        return ROLE_NONE;
    }

    /* Match as much of str as is provided */
    len = strlen(str);
    for (i = 0; roles[i].name.m; i++) {
        /* Does it match the male name? */
        if (!strncmpi(str, roles[i].name.m, len)) {
            return i;
        }
        /* Or the female name? */
        if (roles[i].name.f && !strncmpi(str, roles[i].name.f, len)) {
            return i;
        }
        /* Or the filecode? */
        if (!strcmpi(str, roles[i].filecode)) {
            return i;
        }
    }

    if ((len == 1 && (*str == '*' || *str == '@')) ||
        !strncmpi(str, randomstr, len))
        return ROLE_RANDOM;

    /* Couldn't find anything appropriate */
    return ROLE_NONE;
}

boolean
validrace(int rolenum, int racenum)
{
    /* Assumes validrole */
    return (racenum >= 0 && racenum < SIZE(races) - 1 &&
            (roles[rolenum].allow & races[racenum].allow & ROLE_RACEMASK));
}

int
randrace(int rolenum)
{
    int i, n = 0;

    /* Count the number of valid races */
    for (i = 0; races[i].noun; i++) {
/*      if (roles[rolenum].allow & races[i].allow & ROLE_RACEMASK)*/
        if (validrace(rolenum, i)) {
            n++;
        }
    }

    /* Pick a random race */
    /* Use a factor of 100 in case of bad random number generators */
    if (n) {
        n = rn2(n*100)/100;
    }
    for (i = 0; races[i].noun; i++) {
/*      if (roles[rolenum].allow & races[i].allow & ROLE_RACEMASK) {*/
        if (validrace(rolenum, i)) {
            if (n) {
                n--;
            } else {
                return i;
            }
        }
    }

    /* This role has no permitted races? */
    return (rn2(SIZE(races)-1));
}


int
str2race(const char *str)
{
    int i, len;

    /* Is str valid? */
    if (!str || !str[0]) {
        return ROLE_NONE;
    }

    /* Match as much of str as is provided */
    len = strlen(str);
    for (i = 0; races[i].noun; i++) {
        /* Does it match the noun? */
        if (!strncmpi(str, races[i].noun, len)) {
            return i;
        }
        /* Or the filecode? */
        if (!strcmpi(str, races[i].filecode)) {
            return i;
        }
    }

    if ((len == 1 && (*str == '*' || *str == '@')) ||
        !strncmpi(str, randomstr, len))
        return ROLE_RANDOM;

    /* Couldn't find anything appropriate */
    return ROLE_NONE;
}

boolean
validgend(int rolenum, int racenum, int gendnum)
{
    /* Assumes validrole and validrace */
    return (gendnum >= 0 && gendnum < ROLE_GENDERS &&
            (roles[rolenum].allow & races[racenum].allow &
             genders[gendnum].allow & ROLE_GENDMASK));
}

int
randgend(int rolenum, int racenum)
{
    int i, n = 0;

    /* Count the number of valid genders */
    for (i = 0; i < ROLE_GENDERS; i++) {
/*      if (roles[rolenum].allow & races[racenum].allow &
                genders[i].allow & ROLE_GENDMASK) */
        if (validgend(rolenum, racenum, i)) {
            n++;
        }
    }

    /* Pick a random gender */
    if (n) {
        n = rn2(n);
    }
    for (i = 0; i < ROLE_GENDERS; i++) {
/*      if (roles[rolenum].allow & races[racenum].allow &
                genders[i].allow & ROLE_GENDMASK) {*/
        if (validgend(rolenum, racenum, i)) {
            if (n) {
                n--;
            } else {
                return (i);
            }
        }
    }

    /* This role/race has no permitted genders? */
    return (rn2(ROLE_GENDERS));
}


int
str2gend(const char *str)
{
    int i, len;

    /* Is str valid? */
    if (!str || !str[0]) {
        return ROLE_NONE;
    }

    /* Match as much of str as is provided */
    len = strlen(str);
    for (i = 0; i < ROLE_GENDERS; i++) {
        /* Does it match the adjective? */
        if (!strncmpi(str, genders[i].adj, len)) {
            return i;
        }
        /* Or the filecode? */
        if (!strcmpi(str, genders[i].filecode)) {
            return i;
        }
    }
    if ((len == 1 && (*str == '*' || *str == '@')) ||
        !strncmpi(str, randomstr, len))
        return ROLE_RANDOM;

    /* Couldn't find anything appropriate */
    return ROLE_NONE;
}

boolean
validalign(int rolenum, int racenum, int alignnum)
{
    /* Assumes validrole and validrace */
    return (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
            (roles[rolenum].allow & races[racenum].allow &
             aligns[alignnum].allow & ROLE_ALIGNMASK));
}

int
randalign(int rolenum, int racenum)
{
    int i, n = 0;

    /* Count the number of valid alignments */
    for (i = 0; i < ROLE_ALIGNS; i++) {
        if (roles[rolenum].allow & races[racenum].allow & aligns[i].allow & ROLE_ALIGNMASK) {
            n++;
        }
    }

    /* Pick a random alignment */
    if (n) {
        n = rn2(n);
    }
    for (i = 0; i < ROLE_ALIGNS; i++) {
        if (roles[rolenum].allow & races[racenum].allow & aligns[i].allow & ROLE_ALIGNMASK) {
            if (n) {
                n--;
            } else {
                return i;
            }
        }
    }

    /* This role/race has no permitted alignments? */
    return rn2(ROLE_ALIGNS);
}


int
str2align(const char *str)
{
    int i, len;

    /* Is str valid? */
    if (!str || !str[0]) {
        return ROLE_NONE;
    }

    /* Match as much of str as is provided */
    len = strlen(str);
    for (i = 0; i < ROLE_ALIGNS; i++) {
        /* Does it match the adjective? */
        if (!strncmpi(str, aligns[i].adj, len)) {
            return i;
        }
        /* Or the filecode? */
        if (!strcmpi(str, aligns[i].filecode)) {
            return i;
        }
    }
    if ((len == 1 && (*str == '*' || *str == '@')) ||
        !strncmpi(str, randomstr, len))
        return ROLE_RANDOM;

    /* Couldn't find anything appropriate */
    return ROLE_NONE;
}

/* is rolenum compatible with any racenum/gendnum/alignnum constraints? */
boolean
ok_role(int rolenum, int racenum, int gendnum, int alignnum)
{
    int i;
    short allow;

    if (rolenum >= 0 && rolenum < SIZE(roles) - 1) {
        if (rfilter.roles[rolenum]) {
            return FALSE;
        }
        allow = roles[rolenum].allow;
        if (racenum >= 0 && racenum < SIZE(races) - 1 &&
             !(allow & races[racenum].allow & ROLE_RACEMASK)) {
            return FALSE;
        }
        if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
             !(allow & genders[gendnum].allow & ROLE_GENDMASK)) {
            return FALSE;
        }
        if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
             !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK)) {
            return FALSE;
        }
        return TRUE;
    } else {
        /* random; check whether any selection is possible */
        for (i = 0; i < SIZE(roles) - 1; i++) {
            if (rfilter.roles[i]) {
                continue;
            }
            allow = roles[i].allow;
            if (racenum >= 0 && racenum < SIZE(races) - 1 &&
                 !(allow & races[racenum].allow & ROLE_RACEMASK)) {
                continue;
            }
            if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
                 !(allow & genders[gendnum].allow & ROLE_GENDMASK)) {
                continue;
            }
            if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
                 !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK)) {
                continue;
            }
            return TRUE;
        }
        return FALSE;
    }
}

/* pick a random role subject to any racenum/gendnum/alignnum constraints */
/* If pickhow == PICK_RIGID a role is returned only if there is  */
/* a single possibility */
int
pick_role(int racenum, int gendnum, int alignnum, int pickhow)
{
    int i;
    int roles_ok = 0, set[SIZE(roles)];

    for (i = 0; i < SIZE(roles)-1; i++) {
        if (ok_role(i, racenum, gendnum, alignnum) &&
             ok_race(i, (racenum >= 0) ? racenum : ROLE_RANDOM, gendnum, alignnum) &&
             ok_gend(i, racenum, (gendnum >= 0) ? gendnum : ROLE_RANDOM, alignnum) &&
             ok_race(i, racenum, gendnum, (alignnum >= 0) ? alignnum : ROLE_RANDOM)) {
            set[roles_ok++] = i;
        }
    }
    if (roles_ok == 0 || (roles_ok > 1 && pickhow == PICK_RIGID)) {
        return ROLE_NONE;
    }
    return set[rn2(roles_ok)];
}

/* is racenum compatible with any rolenum/gendnum/alignnum constraints? */
boolean
ok_race(int rolenum, int racenum, int gendnum, int alignnum)
{
    int i;
    short allow;

    if (racenum >= 0 && racenum < SIZE(races) - 1) {
        if (rfilter.mask & races[racenum].selfmask) {
            return FALSE;
        }
        allow = races[racenum].allow;
        if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
             !(allow & roles[rolenum].allow & ROLE_RACEMASK)) {
            return FALSE;
        }
        if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
             !(allow & genders[gendnum].allow & ROLE_GENDMASK)) {
            return FALSE;
        }
        if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
             !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK)) {
            return FALSE;
        }
        return TRUE;
    } else {
        /* random; check whether any selection is possible */
        for (i = 0; i < SIZE(races) - 1; i++) {
            if (rfilter.mask & races[i].selfmask) {
                continue;
            }
            allow = races[i].allow;
            if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
                 !(allow & roles[rolenum].allow & ROLE_RACEMASK)) {
                continue;
            }
            if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
                 !(allow & genders[gendnum].allow & ROLE_GENDMASK)) {
                continue;
            }
            if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
                 !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK)) {
                continue;
            }
            return TRUE;
        }
        return FALSE;
    }
}

/* pick a random race subject to any rolenum/gendnum/alignnum constraints */
/* If pickhow == PICK_RIGID a race is returned only if there is  */
/* a single possibility */
int
pick_race(int rolenum, int gendnum, int alignnum, int pickhow)
{
    int i;
    int races_ok = 0;

    for (i = 0; i < SIZE(races)-1; i++) {
        if (ok_race(rolenum, i, gendnum, alignnum)) {
            races_ok++;
        }
    }
    if (races_ok == 0 || (races_ok > 1 && pickhow == PICK_RIGID)) {
        return ROLE_NONE;
    }
    races_ok = rn2(races_ok);
    for (i = 0; i < SIZE(races)-1; i++) {
        if (ok_race(rolenum, i, gendnum, alignnum)) {
            if (races_ok == 0) {
                return i;
            } else {
                races_ok--;
            }
        }
    }
    return ROLE_NONE;
}

/* is gendnum compatible with any rolenum/racenum/alignnum constraints? */
/* gender and alignment are not comparable (and also not constrainable) */
boolean
ok_gend(int rolenum, int racenum, int gendnum, int alignnum UNUSED)
{
    int i;
    short allow;

    if (gendnum >= 0 && gendnum < ROLE_GENDERS) {
        if (rfilter.mask & genders[gendnum].allow) {
            return FALSE;
        }
        allow = genders[gendnum].allow;
        if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
             !(allow & roles[rolenum].allow & ROLE_GENDMASK)) {
            return FALSE;
        }
        if (racenum >= 0 && racenum < SIZE(races) - 1 &&
             !(allow & races[racenum].allow & ROLE_GENDMASK)) {
            return FALSE;
        }
        return TRUE;
    } else {
        /* random; check whether any selection is possible */
        for (i = 0; i < ROLE_GENDERS; i++) {
            if (rfilter.mask & genders[i].allow) {
                continue;
            }
            allow = genders[i].allow;
            if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
                 !(allow & roles[rolenum].allow & ROLE_GENDMASK)) {
                continue;
            }
            if (racenum >= 0 && racenum < SIZE(races) - 1 &&
                 !(allow & races[racenum].allow & ROLE_GENDMASK)) {
                continue;
            }
            return TRUE;
        }
        return FALSE;
    }
}

/* pick a random gender subject to any rolenum/racenum/alignnum constraints */
/* gender and alignment are not comparable (and also not constrainable) */
/* If pickhow == PICK_RIGID a gender is returned only if there is  */
/* a single possibility */
int
pick_gend(int rolenum, int racenum, int alignnum, int pickhow)
{
    int i;
    int gends_ok = 0;

    for (i = 0; i < ROLE_GENDERS; i++) {
        if (ok_gend(rolenum, racenum, i, alignnum)) {
            gends_ok++;
        }
    }
    if (gends_ok == 0 || (gends_ok > 1 && pickhow == PICK_RIGID)) {
        return ROLE_NONE;
    }
    gends_ok = rn2(gends_ok);
    for (i = 0; i < ROLE_GENDERS; i++) {
        if (ok_gend(rolenum, racenum, i, alignnum)) {
            if (gends_ok == 0) {
                return i;
            } else {
                gends_ok--;
            }
        }
    }
    return ROLE_NONE;
}

/* is alignnum compatible with any rolenum/racenum/gendnum constraints? */
/* alignment and gender are not comparable (and also not constrainable) */
boolean
ok_align(int rolenum, int racenum, int gendnum UNUSED, int alignnum)
{
    int i;
    short allow;

    if (alignnum >= 0 && alignnum < ROLE_ALIGNS) {
        if (rfilter.mask & aligns[alignnum].allow) {
            return FALSE;
        }
        allow = aligns[alignnum].allow;
        if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
             !(allow & roles[rolenum].allow & ROLE_ALIGNMASK)) {
            return FALSE;
        }
        if (racenum >= 0 && racenum < SIZE(races) - 1 &&
             !(allow & races[racenum].allow & ROLE_ALIGNMASK)) {
            return FALSE;
        }
        return TRUE;
    } else {
        /* random; check whether any selection is possible */
        for (i = 0; i < ROLE_ALIGNS; i++) {
            if (rfilter.mask & aligns[i].allow) {
                return FALSE;
            }
            allow = aligns[i].allow;
            if (rolenum >= 0 && rolenum < SIZE(roles) - 1 &&
                 !(allow & roles[rolenum].allow & ROLE_ALIGNMASK)) {
                continue;
            }
            if (racenum >= 0 && racenum < SIZE(races) - 1 &&
                 !(allow & races[racenum].allow & ROLE_ALIGNMASK)) {
                continue;
            }
            return TRUE;
        }
        return FALSE;
    }
}

/* pick a random alignment subject to any rolenum/racenum/gendnum constraints */
/* alignment and gender are not comparable (and also not constrainable) */
/* If pickhow == PICK_RIGID an alignment is returned only if there is  */
/* a single possibility */
int
pick_align(int rolenum, int racenum, int gendnum, int pickhow)
{
    int i;
    int aligns_ok = 0;

    for (i = 0; i < ROLE_ALIGNS; i++) {
        if (ok_align(rolenum, racenum, gendnum, i)) {
            aligns_ok++;
        }
    }
    if (aligns_ok == 0 || (aligns_ok > 1 && pickhow == PICK_RIGID)) {
        return ROLE_NONE;
    }
    aligns_ok = rn2(aligns_ok);
    for (i = 0; i < ROLE_ALIGNS; i++) {
        if (ok_align(rolenum, racenum, gendnum, i)) {
            if (aligns_ok == 0) {
                return i;
            } else {
                aligns_ok--;
            }
        }
    }
    return ROLE_NONE;
}

void
rigid_role_checks(void)
{
    int tmp;

    /* Some roles are limited to a single race, alignment, or gender and
     * calling this routine prior to XXX_player_selection() will help
     * prevent an extraneous prompt that actually doesn't allow
     * you to choose anything further. Note the use of PICK_RIGID which
     * causes the pick_XX() routine to return a value only if there is one
     * single possible selection, otherwise it returns ROLE_NONE.
     *
     */
    if (flags.initrole == ROLE_RANDOM) {
        /* If the role was explicitly specified as ROLE_RANDOM
         * via -uXXXX-@ or OPTIONS=role:random then choose the role
         * in here to narrow down later choices.
         */
        flags.initrole = pick_role(flags.initrace, flags.initgend,
                                   flags.initalign, PICK_RANDOM);
        if (flags.initrole < 0) {
            flags.initrole = randrole_filtered();
        }
    }
    if (flags.initrace == ROLE_RANDOM &&
         (tmp = pick_race(flags.initrole, flags.initgend,
                          flags.initalign, PICK_RANDOM)) != ROLE_NONE) {
        flags.initrace = tmp;
    }
    if (flags.initalign == ROLE_RANDOM &&
         (tmp = pick_align(flags.initrole, flags.initrace,
                           flags.initgend, PICK_RANDOM)) != ROLE_NONE) {
        flags.initalign = tmp;
    }
    if (flags.initgend == ROLE_RANDOM &&
         (tmp = pick_gend(flags.initrole, flags.initrace,
                          flags.initalign, PICK_RANDOM)) != ROLE_NONE) {
        flags.initgend = tmp;
    }

    if (flags.initrole != ROLE_NONE) {
        if (flags.initrace == ROLE_NONE) {
            flags.initrace = pick_race(flags.initrole, flags.initgend,
                                       flags.initalign, PICK_RIGID);
        }
        if (flags.initalign == ROLE_NONE) {
            flags.initalign = pick_align(flags.initrole, flags.initrace,
                                         flags.initgend, PICK_RIGID);
        }
        if (flags.initgend == ROLE_NONE) {
            flags.initgend = pick_gend(flags.initrole, flags.initrace,
                                       flags.initalign, PICK_RIGID);
        }
    }
}

boolean
setrolefilter(const char *bufp)
{
    int i;
    boolean reslt = TRUE;

    if ((i = str2role(bufp)) != ROLE_NONE && i != ROLE_RANDOM) {
        rfilter.roles[i] = TRUE;
    } else if ((i = str2race(bufp)) != ROLE_NONE && i != ROLE_RANDOM) {
        rfilter.mask |= races[i].selfmask;
    } else if ((i = str2gend(bufp)) != ROLE_NONE && i != ROLE_RANDOM) {
        rfilter.mask |= genders[i].allow;
    } else if ((i = str2align(bufp)) != ROLE_NONE && i != ROLE_RANDOM) {
        rfilter.mask |= aligns[i].allow;
    } else {
        reslt = FALSE;
    }
    return reslt;
}

boolean
gotrolefilter(void)
{
    int i;

    if (rfilter.mask) {
        return TRUE;
    }
    for (i = 0; i < SIZE(roles); ++i) {
        if (rfilter.roles[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

void
clearrolefilter(void)
{
    int i;

    for (i = 0; i < SIZE(roles); ++i) {
        rfilter.roles[i] = FALSE;
    }
    rfilter.mask = 0;
}

#define BP_ALIGN    0
#define BP_GEND     1
#define BP_RACE     2
#define BP_ROLE     3
#define NUM_BP      4

static char pa[NUM_BP], post_attribs;

static char *
promptsep(char *buf, int num_post_attribs)
{
    const char *conj = "and ";

    if (num_post_attribs > 1
        && post_attribs < num_post_attribs && post_attribs > 1)
        Strcat(buf, ",");
    Strcat(buf, " ");
    --post_attribs;
    if (!post_attribs && num_post_attribs > 1) {
        Strcat(buf, conj);
    }
    return buf;
}

static int
role_gendercount(int rolenum)
{
    int gendcount = 0;
    if (validrole(rolenum)) {
        if (roles[rolenum].allow & ROLE_MALE) {
            ++gendcount;
        }
        if (roles[rolenum].allow & ROLE_FEMALE) {
            ++gendcount;
        }
        if (roles[rolenum].allow & ROLE_NEUTER) {
            ++gendcount;
        }
    }
    return gendcount;
}

static int
race_alignmentcount(int racenum)
{
    int aligncount = 0;
    if (racenum != ROLE_NONE && racenum != ROLE_RANDOM) {
        if (races[racenum].allow & ROLE_CHAOTIC) {
            ++aligncount;
        }
        if (races[racenum].allow & ROLE_LAWFUL) {
            ++aligncount;
        }
        if (races[racenum].allow & ROLE_NEUTRAL) {
            ++aligncount;
        }
    }
    return aligncount;
}

char *
root_plselection_prompt(char *suppliedbuf, int buflen, int rolenum, int racenum, int gendnum, int alignnum)
{
    int k, gendercount = 0, aligncount = 0;
    char buf[BUFSZ];
    static char err_ret[] = " character's";
    boolean donefirst = FALSE;

    if (!suppliedbuf || buflen < 1) {
        return err_ret;
    }

    /* initialize these static variables each time this is called */
    post_attribs = 0;
    for (k=0; k < NUM_BP; ++k) {
        pa[k] = 0;
    }
    buf[0] = '\0';
    *suppliedbuf = '\0';

    /* How many alignments are allowed for the desired race? */
    if (racenum != ROLE_NONE && racenum != ROLE_RANDOM) {
        aligncount = race_alignmentcount(racenum);
    }

    if (alignnum != ROLE_NONE &&
         alignnum != ROLE_RANDOM &&
         ok_align(rolenum, racenum, gendnum, alignnum)) {
        /* if race specified, and multiple choice of alignments for it */
        if ((racenum >= 0) && (aligncount > 1)) {
            if (donefirst) {
                Strcat(buf, " ");
            }
            Strcat(buf, aligns[alignnum].adj);
            donefirst = TRUE;
        } else {
            if (donefirst) {
                Strcat(buf, " ");
            }
            Strcat(buf, aligns[alignnum].adj);
            donefirst = TRUE;
        }
    } else {
        /* in case we got here by failing the ok_align() test */
        if (alignnum != ROLE_RANDOM) {
            alignnum = ROLE_NONE;
        }
        /* if alignment not specified, but race is specified
            and only one choice of alignment for that race then
            don't include it in the later list */
        if ((((racenum != ROLE_NONE && racenum != ROLE_RANDOM) &&
              ok_race(rolenum, racenum, gendnum, alignnum))
             && (aligncount > 1))
            || (racenum == ROLE_NONE || racenum == ROLE_RANDOM)) {
            pa[BP_ALIGN] = 1;
            post_attribs++;
        }
    }
    /* <your lawful> */

    /* How many genders are allowed for the desired role? */
    if (validrole(rolenum)) {
        gendercount = role_gendercount(rolenum);
    }

    if (gendnum != ROLE_NONE && gendnum != ROLE_RANDOM) {
        if (validrole(rolenum)) {
            /* if role specified, and multiple choice of genders for it,
               and name of role itself does not distinguish gender */
            if ((rolenum != ROLE_NONE) && (gendercount > 1)
                && !roles[rolenum].name.f) {
                if (donefirst) {
                    Strcat(buf, " ");
                }
                Strcat(buf, genders[gendnum].adj);
                donefirst = TRUE;
            }
        } else {
            if (donefirst) {
                Strcat(buf, " ");
            }
            Strcat(buf, genders[gendnum].adj);
            donefirst = TRUE;
        }
    } else {
        /* if gender not specified, but role is specified
            and only one choice of gender then
            don't include it in the later list */
        if ((validrole(rolenum) && (gendercount > 1)) || !validrole(rolenum)) {
            pa[BP_GEND] = 1;
            post_attribs++;
        }
    }
    /* <your lawful female> */

    if (racenum != ROLE_NONE && racenum != ROLE_RANDOM) {
        if (validrole(rolenum) && ok_race(rolenum, racenum, gendnum, alignnum)) {
            if (donefirst) {
                Strcat(buf, " ");
            }
            Strcat(buf, (rolenum == ROLE_NONE) ?
                   races[racenum].noun :
                   races[racenum].adj);
            donefirst = TRUE;
        } else if (!validrole(rolenum)) {
            if (donefirst) {
                Strcat(buf, " ");
            }
            Strcat(buf, races[racenum].noun);
            donefirst = TRUE;
        } else {
            pa[BP_RACE] = 1;
            post_attribs++;
        }
    } else {
        pa[BP_RACE] = 1;
        post_attribs++;
    }
    /* <your lawful female gnomish> || <your lawful female gnome> */

    if (validrole(rolenum)) {
        if (donefirst) {
            Strcat(buf, " ");
        }
        if (gendnum != ROLE_NONE) {
            if (gendnum == 1  && roles[rolenum].name.f) {
                Strcat(buf, roles[rolenum].name.f);
            } else {
                Strcat(buf, roles[rolenum].name.m);
            }
        } else {
            if (roles[rolenum].name.f) {
                Strcat(buf, roles[rolenum].name.m);
                Strcat(buf, "/");
                Strcat(buf, roles[rolenum].name.f);
            } else {
                Strcat(buf, roles[rolenum].name.m);
            }
        }
        donefirst = TRUE;
    } else if (rolenum == ROLE_NONE) {
        pa[BP_ROLE] = 1;
        post_attribs++;
    }

    if ((racenum == ROLE_NONE || racenum == ROLE_RANDOM) && !validrole(rolenum)) {
        if (donefirst) {
            Strcat(buf, " ");
        }
        Strcat(buf, "character");
        donefirst = TRUE;
    }
    /* <your lawful female gnomish cavewoman> || <your lawful female gnome>
     *    || <your lawful female character>
     */
    if (buflen > (int) (strlen(buf) + 1)) {
        Strcpy(suppliedbuf, buf);
        return suppliedbuf;
    } else {
        return err_ret;
    }
}

char *
build_plselection_prompt(char *buf, int buflen, int rolenum, int racenum, int gendnum, int alignnum)
{
    const char *defprompt = "Shall I pick a character for you? [yntq] ";
    int num_post_attribs = 0;
    char tmpbuf[BUFSZ];

    if (buflen < QBUFSZ) {
        return (char *)defprompt;
    }

    Strcpy(tmpbuf, "Shall I pick ");
    if (racenum != ROLE_NONE || validrole(rolenum)) {
        Strcat(tmpbuf, "your ");
    } else {
        Strcat(tmpbuf, "a ");
    }
    /* <your> */

    (void)  root_plselection_prompt(eos(tmpbuf), buflen - strlen(tmpbuf),
                                    rolenum, racenum, gendnum, alignnum);
    Sprintf(buf, "%s", s_suffix(tmpbuf));

    /* buf should now be:
     * < your lawful female gnomish cavewoman's> || <your lawful female gnome's>
     *    || <your lawful female character's>
     *
     * Now append the post attributes to it
     */

    num_post_attribs = post_attribs;
    if (!num_post_attribs) {
        /* some constraints might have been mutually exclusive, in which case
           some prompting that would have been omitted is needed after all */
        if (flags.initrole == ROLE_NONE && !pa[BP_ROLE]) {
            pa[BP_ROLE] = ++post_attribs;
        }
        if (flags.initrace == ROLE_NONE && !pa[BP_RACE]) {
            pa[BP_RACE] = ++post_attribs;
        }
        if (flags.initalign == ROLE_NONE && !pa[BP_ALIGN]) {
            pa[BP_ALIGN] = ++post_attribs;
        }
        if (flags.initgend == ROLE_NONE && !pa[BP_GEND]) {
            pa[BP_GEND] = ++post_attribs;
        }
        num_post_attribs = post_attribs;
    }
    if (num_post_attribs) {
        if (pa[BP_RACE]) {
            (void) promptsep(eos(buf), num_post_attribs);
            Strcat(buf, "race");
        }
        if (pa[BP_ROLE]) {
            (void) promptsep(eos(buf), num_post_attribs);
            Strcat(buf, "role");
        }
        if (pa[BP_GEND]) {
            (void) promptsep(eos(buf), num_post_attribs);
            Strcat(buf, "gender");
        }
        if (pa[BP_ALIGN]) {
            (void) promptsep(eos(buf), num_post_attribs);
            Strcat(buf, "alignment");
        }
    }
    Strcat(buf, " for you? [yntq] ");
    return buf;
}

#undef BP_ALIGN
#undef BP_GEND
#undef BP_RACE
#undef BP_ROLE
#undef NUM_BP

void
plnamesuffix(void)
{
    char *sptr, *eptr;
    int i;

    /* Look for tokens delimited by '-' */
    if ((eptr = index(plname, '-')) != (char *) 0) {
        *eptr++ = '\0';
    }
    while (eptr) {
        /* Isolate the next token */
        sptr = eptr;
        if ((eptr = index(sptr, '-')) != (char *)0) {
            *eptr++ = '\0';
        }

        /* Try to match it to something */
        if ((i = str2role(sptr)) != ROLE_NONE) {
            flags.initrole = i;
        } else if ((i = str2race(sptr)) != ROLE_NONE) {
            flags.initrace = i;
        } else if ((i = str2gend(sptr)) != ROLE_NONE) {
            flags.initgend = i;
        } else if ((i = str2align(sptr)) != ROLE_NONE) {
            flags.initalign = i;
        }
    }
    if (!plname[0]) {
        askname();
        plnamesuffix();
    }

    /* commas in the plname confuse the record file, convert to spaces */
    for (sptr = plname; *sptr; sptr++) {
        if (*sptr == ',') {
            *sptr = ' ';
        }
    }
}

/* show current settings for name, role, race, gender, and alignment
   in the specified window */
void
role_selection_prolog(int which, winid where)
{
    static const char choosing[] = " choosing now";
    static const char not_yet[] = " not yet specified";
    static const char rand_choice[] = " random";
    char buf[BUFSZ];
    int r, c, g, a, allowmask;

    r = flags.initrole;
    c = flags.initrace;
    g = flags.initgend;
    a = flags.initalign;
    if (r >= 0) {
        allowmask = roles[r].allow;
        if ((allowmask & ROLE_RACEMASK) == MH_HUMAN) {
            c = 0; /* races[human] */
        } else if (c >= 0 && !(allowmask & ROLE_RACEMASK & races[c].allow)) {
            c = ROLE_RANDOM;
        }
        if ((allowmask & ROLE_GENDMASK) == ROLE_MALE) {
            g = 0; /* role forces male (hypothetical) */
        } else if ((allowmask & ROLE_GENDMASK) == ROLE_FEMALE) {
            g = 1; /* role forces female (valkyrie) */
        }
        if ((allowmask & ROLE_ALIGNMASK) == AM_LAWFUL) {
            a = 0; /* aligns[lawful] */
        } else if ((allowmask & ROLE_ALIGNMASK) == AM_NEUTRAL) {
            a = 1; /* aligns[neutral] */
        } else if ((allowmask & ROLE_ALIGNMASK) == AM_CHAOTIC) {
            a = 2; /* alings[chaotic] */
        }
    }
    if (c >= 0) {
        allowmask = races[c].allow;
        if ((allowmask & ROLE_ALIGNMASK) == AM_LAWFUL) {
            a = 0; /* aligns[lawful] */
        } else if ((allowmask & ROLE_ALIGNMASK) == AM_NEUTRAL) {
            a = 1; /* aligns[neutral] */
        } else if ((allowmask & ROLE_ALIGNMASK) == AM_CHAOTIC) {
            a = 2; /* alings[chaotic] */
        }
        /* [c never forces gender] */
    }
    /* [g and a don't constrain anything sufficiently
       to narrow something done to a single choice] */

    Sprintf(buf, "%12s ", "name:");
    Strcat(buf, (which == RS_NAME) ? choosing : !*plname ? not_yet : plname);
    putstr(where, 0, buf);
    Sprintf(buf, "%12s ", "role:");
    Strcat(buf, (which == RS_ROLE) ? choosing :
                (r == ROLE_NONE) ? not_yet :
                (r == ROLE_RANDOM) ? rand_choice : roles[r].name.m);
    if (r >= 0 && roles[r].name.f) {
        /* distinct female name [caveman/cavewoman, priest/priestess] */
        if (g == 1) {
            /* female specified; replace male role name with female one */
            Sprintf(index(buf, ':'), ": %s", roles[r].name.f);
        } else if (g < 0) {
            /* gender unspecified; append slash and female role name */
            Sprintf(eos(buf), "/%s", roles[r].name.f);
        }
    }
    putstr(where, 0, buf);
    Sprintf(buf, "%12s ", "race:");
    Strcat(buf, (which == RS_RACE) ? choosing :
                (c == ROLE_NONE) ? not_yet :
                (c == ROLE_RANDOM) ? rand_choice : races[c].noun);
    putstr(where, 0, buf);
    Sprintf(buf, "%12s ", "gender:");
    Strcat(buf, (which == RS_GENDER) ? choosing :
                (g == ROLE_NONE) ? not_yet :
                (g == ROLE_RANDOM) ? rand_choice : genders[g].adj);
    putstr(where, 0, buf);
    Sprintf(buf, "%12s ", "alignment:");
    Strcat(buf, (which == RS_ALGNMNT) ? choosing :
                (a == ROLE_NONE) ? not_yet :
                (a == ROLE_RANDOM) ? rand_choice : aligns[a].adj);
    putstr(where, 0, buf);
}

/* add a "pick alignment first"-type entry to the specified menu */
void
role_menu_extra(int which, winid where, boolean preselect)
{
    static NEARDATA const char RS_menu_let[] = {
        '=',  /* name */
        '?',  /* role */
        '/',  /* race */
        '\"', /* gender */
        '[',  /* alignment */
    };
    anything any;
    char buf[BUFSZ];
    const char *what = 0, *constrainer = 0, *forcedvalue = 0;
    int f = 0, r, c, g, a, i, allowmask;

    r = flags.initrole;
    c = flags.initrace;
    switch (which) {
    case RS_NAME:
        what = "name";
        break;
    case RS_ROLE:
        what = "role";
        f = r;
        for (i = 0; i < SIZE(roles); ++i) {
            if (i != f && !rfilter.roles[i]) {
                break;
            }
        }
        if (i == SIZE(roles)) {
            constrainer = "filter";
            forcedvalue = "role";
        }
        break;
    case RS_RACE:
        what = "race";
        f = flags.initrace;
        c = ROLE_NONE; /* override player's setting */
        if (r >= 0) {
            allowmask = roles[r].allow & ROLE_RACEMASK;
            if (allowmask == MH_HUMAN) {
                c = 0; /* races[human] */
            }
            if (c >= 0) {
                constrainer = "role";
                forcedvalue = races[c].noun;
            } else if (f >= 0 && (allowmask & ~rfilter.mask) == races[f].selfmask) {
                /* if there is only one race choice available due to user
                   options disallowing others, race menu entry is disabled */
                constrainer = "filter";
                forcedvalue = "race";
            }
        }
        break;
    case RS_GENDER:
        what = "gender";
        f = flags.initgend;
        g = ROLE_NONE;
        if (r >= 0) {
            allowmask = roles[r].allow & ROLE_GENDMASK;
            if (allowmask == ROLE_MALE) {
                g = 0; /* genders[male] */
            } else if (allowmask == ROLE_FEMALE) {
                g = 1; /* genders[female] */
            }
            if (g >= 0) {
                constrainer = "role";
                forcedvalue = genders[g].adj;
            } else if (f >= 0 && (allowmask & ~rfilter.mask) == genders[f].allow) {
                /* if there is only one gender choice available due to user
                   options disallowing other, gender menu entry is disabled */
                constrainer = "filter";
                forcedvalue = "gender";
            }
        }
        break;
    case RS_ALGNMNT:
        what = "alignment";
        f = flags.initalign;
        a = ROLE_NONE;
        if (r >= 0) {
            allowmask = roles[r].allow & ROLE_ALIGNMASK;
            if (allowmask == AM_LAWFUL) {
                a = 0; /* aligns[lawful] */
            } else if (allowmask == AM_NEUTRAL) {
                a = 1; /* aligns[neutral] */
            } else if (allowmask == AM_CHAOTIC) {
                a = 2; /* aligns[chaotic] */
            }
            if (a >= 0) {
                constrainer = "role";
            }
        }
        if (c >= 0 && !constrainer) {
            allowmask = races[c].allow & ROLE_ALIGNMASK;
            if (allowmask == AM_LAWFUL) {
                a = 0; /* aligns[lawful] */
            } else if (allowmask == AM_NEUTRAL) {
                a = 1; /* aligns[neutral] */
            } else if (allowmask == AM_CHAOTIC) {
                a = 2; /* aligns[chaotic] */
            }
            if (a >= 0) {
                constrainer = "race";
            }
        }
        if (f >= 0 && !constrainer &&
             (ROLE_ALIGNMASK & ~rfilter.mask) == aligns[f].allow) {
            /* if there is only one alignment choice available due to user
               options disallowing others, algn menu entry is disabled */
            constrainer = "filter";
            forcedvalue = "alignment";
        }
        if (a >= 0) {
            forcedvalue = aligns[a].adj;
        }
        break;
    }

    any = zeroany; /* zero out all bits */
    if (constrainer) {
        any.a_int = 0;
        /* use four spaces of padding to fake a grayed out menu choice */
        Sprintf(buf, "%4s%s forces %s", "", constrainer, forcedvalue);
        add_menu(where, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf,
                 MENU_UNSELECTED);
    } else if (what) {
        any.a_int = RS_menu_arg(which);
        Sprintf(buf, "Pick%s %s first", (f >= 0) ? " another" : "", what);
        add_menu(where, NO_GLYPH, MENU_DEFCNT, &any, RS_menu_let[which], 0, ATR_NONE, buf,
                 MENU_UNSELECTED);
    } else if (which == RS_filter) {
        any.a_int = RS_menu_arg(RS_filter);
        add_menu(where, NO_GLYPH, MENU_DEFCNT, &any, '~', 0, ATR_NONE,
                 "Reset role/race/&c filtering", MENU_UNSELECTED);
    } else if (which == ROLE_RANDOM) {
        any.a_int = ROLE_RANDOM;
        add_menu(where, NO_GLYPH, MENU_DEFCNT, &any, '*', 0, ATR_NONE, "Random",
                 preselect ? MENU_SELECTED : MENU_UNSELECTED);
    } else if (which == ROLE_NONE) {
        any.a_int = ROLE_NONE;
        add_menu(where, NO_GLYPH, MENU_DEFCNT, &any, 'q', 0, ATR_NONE, "Quit",
                 preselect ? MENU_SELECTED : MENU_UNSELECTED);
    } else {
        impossible("role_menu_extra: bad arg (%d)", which);
    }
}

/*
 *  Special setup modifications here:
 *
 *  Unfortunately, this is going to have to be done
 *  on each newgame or restore, because you lose the permonst mods
 *  across a save/restore.  :-)
 *
 *  1 - The Rogue Leader is the Tourist Nemesis.
 *  2 - Priests start with a random alignment - convert the leader and
 *      guardians here.
 *  3 - Priests also get their set of deities from a randomly chosen role.
 *  4 - [obsolete] Elves can have one of two different leaders,
 *      but can't work it out here because it requires hacking the
 *      level file data (see sp_lev.c).
 *
 * This code also replaces quest_init().
 */
void
role_init(void)
{
    int alignmnt;
    struct permonst *pm;

    /* Strip the role letter out of the player name.
     * This is included for backwards compatibility.
     */
    plnamesuffix();

    /* Check for a valid role.  Try flags.initrole first. */
    if (!validrole(flags.initrole)) {
        /* Try the player letter second */
        if ((flags.initrole = str2role(pl_character)) < 0) {
            /* None specified; pick a random role */
            flags.initrole = randrole();
        }
    }

    /* We now have a valid role index.  Copy the role name back. */
    /* This should become OBSOLETE */
    Strcpy(pl_character, roles[flags.initrole].name.m);
    pl_character[PL_CSIZ-1] = '\0';

    /* Check for a valid race */
    if (!validrace(flags.initrole, flags.initrace)) {
        flags.initrace = randrace(flags.initrole);
    }

    /* Check for a valid gender.  If new game, check both initgend
     * and female.  On restore, assume flags.female is correct. */
    if (flags.pantheon == -1) {
        /* new game */
        if (!validgend(flags.initrole, flags.initrace, flags.female)) {
            flags.female = !flags.female;
        }
    }
    if (!validgend(flags.initrole, flags.initrace, flags.initgend)) {
        /* Note that there is no way to check for an unspecified gender. */
        flags.initgend = flags.female;
    }

    /* Check for a valid alignment */
    if (!validalign(flags.initrole, flags.initrace, flags.initalign)) {
        /* Pick a random alignment */
        flags.initalign = randalign(flags.initrole, flags.initrace);
    }
    alignmnt = aligns[flags.initalign].value;

    /* Initialize urole and urace */
    urole = roles[flags.initrole];
    urace = races[flags.initrace];

    /* Fix up the quest leader */
    if (urole.ldrnum != NON_PM) {
        mons[urole.ldrnum].msound = MS_LEADER;
        mons[urole.ldrnum].mflags2 |= (M2_PEACEFUL);
        mons[urole.ldrnum].mflags3 |= M3_CLOSE;
        mons[urole.ldrnum].maligntyp = alignmnt * 3;
        /* if gender is random, we choose it now instead of waiting
           until the leader monster is created */
        pm = &mons[urole.ldrnum];
        quest_status.ldrgend = is_neuter(pm) ? 2 :
                               is_female(pm) ? 1 :
                               is_male(pm)   ? 0 : (rn2(100) < 50);
    }

    /* Fix up the quest guardians */
    if (urole.guardnum != NON_PM) {
        mons[urole.guardnum].mflags2 |= (M2_PEACEFUL);
        mons[urole.guardnum].maligntyp = alignmnt * 3;
    }

    /* Fix up the quest nemesis */
    if (urole.neminum != NON_PM) {
        mons[urole.neminum].msound = MS_NEMESIS;
        mons[urole.neminum].mflags2 &= ~(M2_PEACEFUL);
        mons[urole.neminum].mflags2 |= (M2_NASTY|M2_STALK|M2_HOSTILE);
        mons[urole.neminum].mflags3 |= M3_WANTSARTI | M3_WAITFORU;
        /* if gender is random, we choose it now instead of waiting
           until the nemesis monster is created */
        pm = &mons[urole.neminum];
        quest_status.nemgend = is_neuter(pm) ? 2 :
                               is_female(pm) ? 1 :
                               is_male(pm)   ? 0 : (rn2(100) < 50);
    }

    /* Determine a valid Pantheon. This is here to ensure the same RNG for
     * all Roles, including the Priest. */
    int pantheon = randrole();
    while (!roles[pantheon].lgod) {
        pantheon = randrole();
    }
    /* Fix up the god names */
    if (flags.pantheon == -1) {
        /* new game */
        flags.pantheon = flags.initrole; /* use own gods */
        if (!roles[flags.pantheon].lgod) {
            /* if they're missing */
            flags.pantheon = pantheon;
        }
    }
    if (!urole.lgod) {
        urole.lgod = roles[flags.pantheon].lgod;
        urole.ngod = roles[flags.pantheon].ngod;
        urole.cgod = roles[flags.pantheon].cgod;
    }
    /* 0 or 1; no gods are neuter, nor is gender randomized */
    quest_status.godgend = !strcmpi(align_gtitle(alignmnt), "goddess");

    /* Fix up infravision */
    if (mons[urace.malenum].mflags3 & M3_INFRAVISION) {
        /* although an infravision intrinsic is possible, infravision
         * is purely a property of the physical race.  This means that we
         * must put the infravision flag in the player's current race
         * (either that or have separate permonst entries for
         * elven/non-elven members of each class).  The side effect is that
         * all NPCs of that class will have (probably bogus) infravision,
         * but since infravision has no effect for NPCs anyway we can
         * ignore this.
         */
        mons[urole.malenum].mflags3 |= M3_INFRAVISION;
        if (urole.femalenum != NON_PM) {
            mons[urole.femalenum].mflags3 |= M3_INFRAVISION;
        }
    }

    /* Fix up initial roleplay flags */
    if (Role_if(PM_MONK)) {
        flags.vegetarian = TRUE;
    }
    flags.vegan |= flags.ascet;
    flags.vegetarian |= flags.vegan;

    if (Role_if(PM_CAVEMAN)) {
        flags.confused_reading = TRUE;
    }

    /* Artifacts are fixed in hack_artifacts() */

    /* Success! */
    return;
}

const char *
Hello(struct monst *mtmp)
{
    switch (Role_switch) {
    case PM_KNIGHT:
        return ("Salutations"); /* Olde English */
    case PM_MONK:
        return ("Namaste"); /* Sanskrit */
    case PM_SAMURAI:
        return (mtmp && mtmp->data == &mons[PM_SHOPKEEPER] ?
                "Irasshaimase" : "Konnichi wa"); /* Japanese */
    case PM_TOURIST:
        return ("Aloha"); /* Hawaiian */
    case PM_VALKYRIE:
        return (
#ifdef MAIL
            mtmp && mtmp->data == &mons[PM_MAIL_DAEMON] ? "Hallo" :
#endif
            "Velkommen"); /* Norse */
    default:
        return ("Hello");
    }
}

const char *
Goodbye(void)
{
    switch (Role_switch) {
    case PM_KNIGHT:
        return ("Fare thee well");  /* Olde English */
    case PM_MONK:
        return ("Punardarsanaya");  /* Sanskrit */
    case PM_SAMURAI:
        return ("Sayonara");        /* Japanese */
    case PM_TOURIST:
        return ("Aloha");           /* Hawaiian */
    case PM_VALKYRIE:
        return ("Farvel");          /* Norse */
    default:
        return ("Goodbye");
    }
}

/* A function to break a specific roleplay-conduct */
void
violated(int cdt)
{
    switch (cdt) {
    case CONDUCT_PACIFISM:
        u.uconduct.killer++;
        if (u.roleplay.pacifist) {
            You_feel("violent!");
            if (yn("Do you want to quit?") == 'y') {
                killer.format = NO_KILLER_PREFIX;
                Strcpy(killer.name, "quit after an act of violence");
                done(QUIT);
            }
            if (u.uconduct.killer >= 10) {
                u.roleplay.pacifist = FALSE;
            }
        }
        break;

    case CONDUCT_NUDISM:
        u.uconduct.armoruses++;
        if (u.roleplay.nudist) {
            You("realize you were nude.");
            makemon(&mons[PM_COBRA], u.ux, u.uy, NO_MM_FLAGS);
            mksobj_at(APPLE, u.ux, u.uy, FALSE, FALSE);
            u.roleplay.nudist = FALSE;
        }
        break;

    case CONDUCT_BLINDFOLDED:
        u.uconduct.unblinded++;
        if (u.roleplay.blindfolded) {
            pline("The Spirit of Zen leaves your body.");
            makemon(mkclass(S_ZOMBIE, 0), u.ux, u.uy, NO_MM_FLAGS); /* Z */
            makemon(mkclass(S_EYE, 0), u.ux, u.uy, NO_MM_FLAGS); /* e */
            makemon(mkclass(S_NYMPH, 0), u.ux, u.uy, NO_MM_FLAGS); /* n */
            u.roleplay.blindfolded = FALSE;
        }
        break;

    case CONDUCT_VEGETARIAN:    /* replaces violated_vegetarian() */
        if (u.roleplay.vegetarian) {
            You_feel("guilty.");
        }
        if (Role_if(PM_MONK)) {
            adjalign(-1);
        }
        u.uconduct.unvegetarian++;
        u.uconduct.unvegan++;
        u.uconduct.food++;
        if (u.uconduct.unvegetarian >= 30) {
            u.roleplay.vegetarian = FALSE;
        }
        if (u.uconduct.unvegan >= 20) {
            u.roleplay.vegan = FALSE;
        }
        if (u.uconduct.food >= 10) {
            u.roleplay.ascet = FALSE;
        }
        break;

    case CONDUCT_VEGAN:
        if (u.roleplay.vegan) {
            You_feel("a bit guilty.");
        }
        u.uconduct.unvegan++;
        u.uconduct.food++;
        if (u.uconduct.unvegan >= 20) {
            u.roleplay.vegan = FALSE;
        }
        if (u.uconduct.food >= 10) {
            u.roleplay.ascet = FALSE;
        }
        break;

    case CONDUCT_FOODLESS:
        if (u.roleplay.ascet) {
            You_feel("a little bit guilty.");
        }
        u.uconduct.food++;
        if (u.uconduct.food >= 10) {
            u.roleplay.ascet = FALSE;
        }
        break;

    case CONDUCT_ILLITERACY:
        u.uconduct.literate++;
        if (u.roleplay.illiterate) {
            /* should be impossible */
            pline("Literatally literature for literate illiterates!");
            exercise(A_WIS, TRUE);
        }
        break;

    case CONDUCT_THIEVERY:
        u.uconduct.robbed++;
        if (Role_if(PM_ROGUE)) {
            You_feel("like an ordinary robber.");
        }
        break;

    default:
        impossible("violated: unknown conduct");

    }
    return;
}

/* a function to check whether a specific conduct has been broken
 * return FALSE if broken
 */
boolean
successful_cdt(int cdt)
{
    if ((cdt == CONDUCT_PACIFISM) && !u.uconduct.killer &&
        !num_genocides() && (u.uconduct.weaphit<=100))
        return TRUE;
    if ((cdt == CONDUCT_SADISM) && !u.uconduct.killer &&
        (num_genocides() || (u.uconduct.weaphit>100)))
        return TRUE;
    if ((cdt == CONDUCT_ATHEISM) && !u.uconduct.gnostic) {
        return TRUE;
    }
    if ((cdt == CONDUCT_NUDISM) && !u.uconduct.armoruses) {
        return TRUE;
    }
    if ((cdt == CONDUCT_BLINDFOLDED) && !u.uconduct.unblinded) {
        return TRUE;
    }
    if ((cdt == CONDUCT_VEGETARIAN) && !u.uconduct.unvegetarian) {
        return TRUE;
    }
    if ((cdt == CONDUCT_VEGAN) && !u.uconduct.unvegan) {
        return TRUE;
    }
    if ((cdt == CONDUCT_FOODLESS) && !u.uconduct.food) {
        return TRUE;
    }
    if ((cdt == CONDUCT_ILLITERACY) && !u.uconduct.literate) {
        return TRUE;
    }
    if ((cdt == CONDUCT_THIEVERY) && !u.uconduct.robbed) {
        return TRUE;
    }

    return FALSE;
}

/* a function to check whether a specific conduct *
 *  was selected at character creation        */
boolean
intended_cdt(int cdt)
{
    if ((cdt == CONDUCT_PACIFISM) && flags.pacifist) {
        return TRUE;
    }
    if ((cdt == CONDUCT_ATHEISM) && flags.atheist) {
        return TRUE;
    }
    if ((cdt == CONDUCT_NUDISM) && flags.nudist) {
        return TRUE;
    }
    if ((cdt == CONDUCT_BLINDFOLDED) && flags.blindfolded) {
        return TRUE;
    }
    if ((cdt == CONDUCT_FOODLESS) && flags.ascet) {
        return TRUE;
    }
    if ((cdt == CONDUCT_VEGAN) && flags.vegan) {
        return TRUE;
    }
    if ((cdt == CONDUCT_VEGETARIAN) && flags.vegetarian) {
        return TRUE;
    }
    if ((cdt == CONDUCT_ILLITERACY) && flags.illiterate) {
        return TRUE;
    }

    return FALSE;
}

/* a function to check whether it's superflous to list that conduct */
boolean
superfluous_cdt(int cdt)
{
    if ((cdt == CONDUCT_VEGAN) && successful_cdt(CONDUCT_FOODLESS)) {
        return TRUE;
    }
    if ((cdt == CONDUCT_VEGETARIAN) && successful_cdt(CONDUCT_VEGAN)) {
        return TRUE;
    }
    if ((cdt == CONDUCT_THIEVERY) && !u.uevent.invoked) {
        return TRUE;
    }

    return FALSE;
}

/* tell if you failed a selected conduct */
boolean
failed_cdt(int cdt)
{
    return (intended_cdt(cdt) && !successful_cdt(cdt));
}

/* Dialog code and variables when starting the game */
const char *str_tutorial_prompt = "New? Press T to enter a tutorial.";
const char *str_conduct_tracking_prompt = "Press C for selecting conduct tracking.";

boolean
show_conduct_selection_dialog(void)
{
    int i;
    anything any;
    winid win = create_nhwindow(NHW_MENU);

    start_menu(win);

    int pick_cnt, pick_idx, opt_idx;
    menu_item *conduct_category_pick = (menu_item *)0;

    static const char *conduct_names[] = {
        "ascet", "atheist", "blindfolded", "hallucinating",
        "illiterate", "nudist", "pacifist", "vegan",
        "vegetarian", "death dropless",
        "Elberethless",
        "Heaven or Hell", "Quit"
    };
#define NUM_CONDUCT_OPTIONS SIZE(conduct_names)
    static boolean *conduct_bools[NUM_CONDUCT_OPTIONS];
    int idx = 0;
    conduct_bools[idx++] = &flags.ascet;
    conduct_bools[idx++] = &flags.atheist;
    conduct_bools[idx++] = &flags.blindfolded;
    conduct_bools[idx++] = &flags.perma_hallu;
    conduct_bools[idx++] = &flags.illiterate;
    conduct_bools[idx++] = &flags.nudist;
    conduct_bools[idx++] = &flags.pacifist;
    conduct_bools[idx++] = &flags.vegan;
    conduct_bools[idx++] = &flags.vegetarian;
    conduct_bools[idx++] = &flags.deathdropless;
    conduct_bools[idx++] = &flags.elberethignore;
    conduct_bools[idx++] = &flags.heaven_or_hell;
    conduct_bools[idx++] = 0;

    int conduct_settings[NUM_CONDUCT_OPTIONS];

    winid tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    for (i = 0; i < NUM_CONDUCT_OPTIONS; i++) {
        any.a_int = i + 1;
        /* use uppercase character if previous option has the same
         * starting character */
        char selection_char = conduct_names[i][0];
        if (i > 0 && conduct_names[i-1][0] == conduct_names[i][0]) {
            selection_char = highc(selection_char);
        }
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, selection_char, 0,
                 ATR_NONE, conduct_names[i],
                 !conduct_bools[i] ? MENU_UNSELECTED :
                 (*conduct_bools[i] ? MENU_SELECTED : MENU_UNSELECTED));
        conduct_settings[i] = 0;
    }
    end_menu(tmpwin, "Change which conduct settings:");

    if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &conduct_category_pick)) > 0) {
        for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
            opt_idx = conduct_category_pick[pick_idx].item.a_int - 1;
            conduct_settings[opt_idx] = 1;
        }
        free((genericptr_t)conduct_category_pick);
        conduct_category_pick = (menu_item *)0;
    }
    destroy_nhwindow(tmpwin);
    /* has Quit been selected? */
    if (conduct_settings[NUM_CONDUCT_OPTIONS-1]) {
        return FALSE;
    }

    idx = 0;
    flags.ascet          = conduct_settings[idx++];
    flags.atheist        = conduct_settings[idx++];
    flags.blindfolded    = conduct_settings[idx++];
    flags.perma_hallu    = conduct_settings[idx++];
    flags.illiterate     = conduct_settings[idx++];
    flags.nudist         = conduct_settings[idx++];
    flags.pacifist       = conduct_settings[idx++];
    flags.vegan          = conduct_settings[idx++];
    flags.vegetarian     = conduct_settings[idx++];
    flags.deathdropless  = conduct_settings[idx++];
    flags.elberethignore = conduct_settings[idx++];
    flags.heaven_or_hell = conduct_settings[idx++];

    return TRUE;
}

/* role.c */
