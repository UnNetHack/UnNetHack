/*  SCCS Id: @(#)drawing.c  3.4 1999/12/02  */
/* Copyright (c) NetHack Development Team 1992.           */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "tcap.h"
/* Relevent header information in rm.h and objclass.h. */

#ifdef C
#undef C
#endif

#ifdef TEXTCOLOR
#define C(n) n
#else
#define C(n)
#endif

#define g_FILLER(symbol) 0

uchar oc_syms[MAXOCLASSES]  = DUMMY;    /* the current object  display symbols */
glyph_t showsyms[MAXPCHARS] = DUMMY;    /* the current feature display symbols */
nhsym primary_syms[SYM_MAX] = DUMMY;    /* primary symbols          */
nhsym rogue_syms[SYM_MAX] = DUMMY;      /* rogue symbols           */
nhsym ov_primary_syms[SYM_MAX] = DUMMY; /* overides via config SYMBOL */
nhsym ov_rogue_syms[SYM_MAX] = DUMMY;   /* overides via config ROGUESYMBOL */
uchar monsyms[MAXMCLASSES]  = DUMMY;    /* the current monster display symbols */
uchar warnsyms[WARNCOUNT]   = DUMMY;    /* the current warning display symbols */

/* Default object class symbols.  See objclass.h. */
const char def_oc_syms[MAXOCLASSES] = {
/* 0*/ '\0',        /* placeholder for the "random class" */
    ILLOBJ_SYM,
    WEAPON_SYM,
    ARMOR_SYM,
    RING_SYM,
/* 5*/ AMULET_SYM,
    TOOL_SYM,
    FOOD_SYM,
    POTION_SYM,
    SCROLL_SYM,
/*10*/ SPBOOK_SYM,
    WAND_SYM,
    GOLD_SYM,
    GEM_SYM,
    ROCK_SYM,
/*15*/ BALL_SYM,
    CHAIN_SYM,
    VENOM_SYM
};

const char invisexplain[] = "remembered, unseen, creature";
const char altinvisexplain[] = "unseen creature"; /* for clairvoyance */

/* Object descriptions.  Used in do_look(). */
const char * const objexplain[] = { /* these match def_oc_syms, above */
/* 0*/ 0,
    "strange object",
    "weapon",
    "suit or piece of armor",
    "ring",
/* 5*/ "amulet",
    "useful item (pick-axe, key, lamp...)",
    "piece of food",
    "potion",
    "scroll",
/*10*/ "spellbook",
    "wand",
    "pile of coins",
    "gem or rock",
    "boulder or statue",
/*15*/ "iron ball",
    "iron chain",
    "splash of venom"
};

/* Object class names.  Used in object_detect(). */
const char * const oclass_names[] = {
/* 0*/ 0,
    "illegal objects",
    "weapons",
    "armor",
    "rings",
/* 5*/ "amulets",
    "tools",
    "food",
    "potions",
    "scrolls",
/*10*/ "spellbooks",
    "wands",
    "coins",
    "rocks",
    "large stones",
/*15*/ "iron balls",
    "chains",
    "venoms"
};

/* Default monster class symbols.  See monsym.h. */
const char def_monsyms[MAXMCLASSES] = {
    '\0',       /* holder */
    DEF_ANT,
    DEF_BLOB,
    DEF_COCKATRICE,
    DEF_DOG,
    DEF_EYE,
    DEF_FELINE,
    DEF_GREMLIN,
    DEF_HUMANOID,
    DEF_IMP,
    DEF_JELLY,      /* 10 */
    DEF_KOBOLD,
    DEF_LEPRECHAUN,
    DEF_MIMIC,
    DEF_NYMPH,
    DEF_ORC,
    DEF_PIERCER,
    DEF_QUADRUPED,
    DEF_RODENT,
    DEF_SPIDER,
    DEF_TRAPPER,        /* 20 */
    DEF_UNICORN,
    DEF_VORTEX,
    DEF_WORM,
    DEF_XAN,
    DEF_LIGHT,
    DEF_ZRUTY,
    DEF_ANGEL,
    DEF_BAT,
    DEF_CENTAUR,
    DEF_DRAGON,     /* 30 */
    DEF_ELEMENTAL,
    DEF_FUNGUS,
    DEF_GNOME,
    DEF_GIANT,
    '\0',
    DEF_JABBERWOCK,
    DEF_KOP,
    DEF_LICH,
    DEF_MUMMY,
    DEF_NAGA,       /* 40 */
    DEF_OGRE,
    DEF_PUDDING,
    DEF_QUANTMECH,
    DEF_RUSTMONST,
    DEF_SNAKE,
    DEF_TROLL,
    DEF_UMBER,
    DEF_VAMPIRE,
    DEF_WRAITH,
    DEF_XORN,       /* 50 */
    DEF_YETI,
    DEF_ZOMBIE,
    DEF_HUMAN,
    DEF_GHOST,
    DEF_GOLEM,
    DEF_DEMON,
    DEF_EEL,
    DEF_LIZARD,
    DEF_WORM_TAIL,
    DEF_MIMIC_DEF,      /* 60 */
    DEF_STATUE,
};

/* The explanations below are also used when the user gives a string
 * for blessed genocide, so no text should wholly contain any later
 * text.  They should also always contain obvious names (eg. cat/feline).
 */
const char * const monexplain[MAXMCLASSES] = {
    0,
    "ant or other insect",  "blob",         "cockatrice",
    "dog or other canine",  "eye or sphere",    "cat or other feline",
    "gremlin",          "humanoid",     "imp or minor demon",
    "jelly",            "kobold",       "leprechaun",
    "mimic",            "nymph",        "orc",
    "piercer",          "quadruped",        "rodent",
    "arachnid or centipede",    "trapper or lurker above", "unicorn or horse",
    "vortex",       "worm", "xan or other mythical/fantastic insect",
    "light",            "zruty",

    "angelic being",        "bat or bird",      "centaur",
    "dragon",           "elemental",        "fungus, mold, or fern",
    "gnome",            "giant humanoid",   0,
    "jabberwock",       "Keystone Kop",     "lich",
    "mummy",            "naga",         "ogre",
    "pudding or ooze",      "quantum mechanic", "rust monster, disenchanter, or disintegrator",
    "snake",            "troll",        "umber hulk or shambling horror",
    "vampire",          "wraith",       "xorn or harmful vegetation",
    "apelike creature",     "zombie",

    "human or elf",     "ghost",        "golem",
    "major demon",      "sea monster",      "lizard",
    "long worm tail",       "mimic",        "statue",
};

const struct symdef def_warnsyms[WARNCOUNT] = {
    {'0', "unknown creature causing you worry", C(CLR_WHITE)},      /* white warning  */
    {'1', "unknown creature causing you concern", C(CLR_RED)},  /* pink warning   */
    {'2', "unknown creature causing you anxiety", C(CLR_RED)},  /* red warning    */
    {'3', "unknown creature causing you disquiet", C(CLR_RED)}, /* ruby warning   */
    {'4', "unknown creature causing you alarm",
     C(CLR_MAGENTA)},                           /* purple warning */
    {'5', "unknown creature causing you dread",
     C(CLR_BRIGHT_MAGENTA)}                     /* black warning  */
};

/*
 *  Default screen symbols with explanations and colors.
 *  Note:  {ibm|dec|mac}_graphics[] arrays also depend on this symbol order.
 */
struct symdef defsyms[MAXPCHARS] = {
       { ' ', "unexplored area", C(NO_COLOR) },   /* stone */
       { '|', "wall",       C(CLR_GRAY) },   /* vwall */
       { '-', "wall",       C(CLR_GRAY) },   /* hwall */
       { '-', "wall",       C(CLR_GRAY) },   /* tlcorn */
       { '-', "wall",       C(CLR_GRAY) },   /* trcorn */
       { '-', "wall",       C(CLR_GRAY) },   /* blcorn */
       { '-', "wall",       C(CLR_GRAY) },   /* brcorn */
       { '-', "wall",       C(CLR_GRAY) },   /* crwall */
       { '-', "wall",       C(CLR_GRAY) },   /* tuwall */
       { '-', "wall",       C(CLR_GRAY) },   /* tdwall */
       { '|', "wall",       C(CLR_GRAY) },    /* tlwall */
       { '|', "wall",       C(CLR_GRAY) },   /* trwall */
       { '.', "doorway",    C(CLR_GRAY) },   /* ndoor */
       { '-', "open door",  C(CLR_BROWN) },  /* vodoor */
       { '|', "open door",  C(CLR_BROWN) },  /* hodoor */
       { '+', "closed door",    C(CLR_BROWN) },  /* vcdoor */
       { '+', "closed door",    C(CLR_BROWN) },  /* hcdoor */
       { '#', "iron bars",  C(HI_METAL) },   /* bars */
       { '#', "tree",       C(CLR_GREEN) },  /* tree */
       { '#', "dead tree",  C(CLR_BLACK) },  /* dead tree */
       { '.', "floor of a room", C(CLR_GRAY) },   /* room */
       { '.', "dark part of a room", C(CLR_BLACK) },  /* dark room */
       { '#', "corridor",   C(CLR_BLACK) },    /* dark corr */
       { '#', "lit corridor",   C(CLR_GRAY) },   /* lit corr (see mapglyph.c) */
       { '<', "staircase up",   C(CLR_WHITE) },   /* upstair */
       { '>', "staircase down", C(CLR_WHITE) },   /* dnstair */
       { '<', "ladder up",  C(CLR_BROWN) },  /* upladder */
       { '>', "ladder down",    C(CLR_BROWN) },  /* dnladder */
       { '_', "altar",      C(CLR_GRAY) },   /* altar */
       { '|', "grave",      C(CLR_GRAY) },   /* grave */
       { '\\', "opulent throne", C(HI_GOLD) },    /* throne */
#ifdef SINKS
       { '#', "sink",       C(CLR_GRAY) },   /* sink */
#else
       { '#', "",       C(CLR_GRAY) },   /* sink */
#endif
       { '{', "fountain",   C(CLR_BLUE) },    /* fountain */
       { '}', "water",      C(CLR_BLUE) },   /* pool */
       { '.', "ice",        C(CLR_CYAN) },   /* ice */
       { '}', "muddy swamp",    C(CLR_GREEN) },  /* muddy swamp, bog */
       { '}', "molten lava",    C(CLR_RED) },    /* lava */
       { '.', "lowered drawbridge", C(CLR_BROWN) },   /* vodbridge */
       { '.', "lowered drawbridge", C(CLR_BROWN) },   /* hodbridge */
       { '#', "raised drawbridge", C(CLR_BROWN) },/* vcdbridge */
       { '#', "raised drawbridge", C(CLR_BROWN) },/* hcdbridge */
       { ' ', "air",        C(CLR_CYAN) },   /* open air */
       { '#', "cloud",      C(CLR_GRAY) },   /* [part of] a cloud */
       { '8', "ice wall",       C(CLR_BRIGHT_CYAN) }, /* ice wall */
       { '8', "crystal ice wall", C(CLR_WHITE) }, /* ice wall */
       { '}', "water",      C(CLR_BLUE) },    /* under water */
       { '^', "arrow trap", C(HI_METAL) },   /* trap */
       { '^', "dart trap",  C(HI_METAL) },   /* trap */
       { '^', "falling rock trap", C(CLR_GRAY) }, /* trap */
       { '^', "squeaky board",  C(CLR_BROWN) },  /* trap */
       { '^', "bear trap",  C(HI_METAL) },   /* trap */
       { '^', "land mine",  C(CLR_RED) },    /* trap */
       { '^', "rolling boulder trap",   C(CLR_GRAY) },   /* trap */
       { '^', "sleeping gas trap", C(HI_ZAP) },   /* trap */
       { '^', "rust trap",  C(CLR_BLUE) },   /* trap */
       { '^', "fire trap",  C(CLR_ORANGE) }, /* trap */
       { '^', "pit",        C(CLR_BLACK) },  /* trap */
       { '^', "spiked pit", C(CLR_BLACK) },  /* trap */
       { '^', "hole",   C(CLR_BROWN) },  /* trap */
       { '^', "trap door",  C(CLR_BROWN) },  /* trap */
       { '^', "teleportation trap", C(CLR_MAGENTA) },    /* trap */
       { '^', "level teleporter", C(CLR_MAGENTA) },  /* trap */
       { '^', "magic portal",   C(CLR_BRIGHT_MAGENTA) }, /* trap */
       { '"', "web",        C(CLR_GRAY) },    /* web */
       { '^', "statue trap",    C(CLR_GRAY) },   /* trap */
       { '^', "magic trap", C(HI_ZAP) }, /* trap */
       { '^', "anti-magic field", C(HI_ZAP) },   /* trap */
       { '^', "ice trap",   C(CLR_WHITE) },  /* trap */
       { '^', "polymorph trap", C(CLR_BRIGHT_GREEN) },   /* trap */
       { '^', "vibrating square", C(CLR_MAGENTA) },    /* "trap" */
       { '|', "",       C(CLR_GRAY) },   /* vbeam */
       { '-', "",       C(CLR_GRAY) },   /* hbeam */
       { '\\', "",      C(CLR_GRAY) },   /* lslant */
       { '/', "",       C(CLR_GRAY) },   /* rslant */
       { '*', "",       C(CLR_WHITE) },   /* dig beam */
       { '!', "",       C(CLR_WHITE) },  /* camera flash beam */
       { ')', "",       C(HI_WOOD) },    /* boomerang open left */
       { '(', "",       C(HI_WOOD) },    /* boomerang open right */
       { '0', "",       C(HI_ZAP) }, /* 4 magic shield symbols */
       { '#', "",       C(HI_ZAP) },
       { '@', "",       C(HI_ZAP) },
       { '*', "",       C(HI_ZAP) },
       { '#', "poison cloud", C(CLR_BRIGHT_GREEN) },   /* part of a cloud */
       { '?', "valid position", C(CLR_BRIGHT_GREEN) }, /*  target position */
       { '/', "",       C(CLR_GREEN) },  /* swallow top left */
       { '-', "",       C(CLR_GREEN) },  /* swallow top center   */
       { '\\', "",      C(CLR_GREEN) },   /* swallow top right    */
       { '|', "",       C(CLR_GREEN) },  /* swallow middle left  */
       { '|', "",       C(CLR_GREEN) },  /* swallow middle right */
       { '\\', "",      C(CLR_GREEN) },  /* swallow bottom left  */
       { '-', "",       C(CLR_GREEN) },  /* swallow bottom center*/
       { '/', "",       C(CLR_GREEN) },  /* swallow bottom right */
       { '/', "",       C(CLR_ORANGE) }, /* explosion top left     */
       { '-', "",       C(CLR_ORANGE) }, /* explosion top center   */
       { '\\', "",      C(CLR_ORANGE) }, /* explosion top right    */
       { '|', "",       C(CLR_ORANGE) }, /* explosion middle left  */
       { ' ', "",       C(CLR_ORANGE) },  /* explosion middle center*/
       { '|', "",       C(CLR_ORANGE) }, /* explosion middle right */
       { '\\', "",      C(CLR_ORANGE) }, /* explosion bottom left  */
       { '-', "",       C(CLR_ORANGE) }, /* explosion bottom center*/
       { '/', "",       C(CLR_ORANGE) }, /* explosion bottom right */
/*
 *  Note: Additions to this array should be reflected in the
 *    {ibm,dec,mac}_graphics[] arrays below.
 */
};

#undef C

#ifdef ASCIIGRAPH

#ifdef PC9800
void (*ibmgraphics_mode_callback)(void) = 0; /* set in tty_start_screen() */
#endif /* PC9800 */

#ifdef CURSES_GRAPHICS
void (*cursesgraphics_mode_callback)(void) = 0;
#endif

static glyph_t ibm_graphics[MAXPCHARS] = {
/* 0*/ g_FILLER(S_stone),
    0xb3,   /* S_vwall: meta-3, vertical rule */
    0xc4,   /* S_hwall: meta-D, horizontal rule */
    0xda,   /* S_tlcorn:    meta-Z, top left corner */
    0xbf,   /* S_trcorn:    meta-?, top right corner */
    0xc0,   /* S_blcorn:    meta-@, bottom left */
    0xd9,   /* S_brcorn:    meta-Y, bottom right */
    0xc5,   /* S_crwall:    meta-E, cross */
    0xc1,   /* S_tuwall:    meta-A, T up */
    0xc2,   /* S_tdwall:    meta-B, T down */
/*10*/ 0xb4,    /* S_tlwall:    meta-4, T left */
    0xc3,   /* S_trwall:    meta-C, T right */
    0xfa,   /* S_ndoor: meta-z, centered dot */
    0xfe,   /* S_vodoor:    meta-~, small centered square */
    0xfe,   /* S_hodoor:    meta-~, small centered square */
    g_FILLER(S_vcdoor),
    g_FILLER(S_hcdoor),
    240,    /* S_bars:  equivalence symbol */
    241,    /* S_tree:  plus or minus symbol */
    241,    /* S_deadtree:  plus or minus symbol */
    0xfa,   /* S_room:  meta-z, centered dot */
    0xfa,   /* S_darkroom:  meta-z, centered dot */
/*20*/ 0xb0,    /* S_corr:  meta-0, light shading */
    0xb1,   /* S_litcorr:   meta-1, medium shading */
    g_FILLER(S_upstair),
    g_FILLER(S_dnstair),
    g_FILLER(S_upladder),
    g_FILLER(S_dnladder),
    g_FILLER(S_altar),
    g_FILLER(S_grave),
    g_FILLER(S_throne),
    g_FILLER(S_sink),
/*30*/ 0xf4,    /* S_fountain:  meta-t, integral top half */
    0xf7,   /* S_pool:  meta-w, approx. equals */
    0xfa,   /* S_ice:   meta-z, centered dot */
    0xf7,   /* S_bog:   meta-w, approx. equals */
    0xf7,   /* S_lava:  meta-w, approx. equals */
    0xfa,   /* S_vodbridge: meta-z, centered dot */
    0xfa,   /* S_hodbridge: meta-z, centered dot */
    g_FILLER(S_vcdbridge),
    g_FILLER(S_hcdbridge),
    g_FILLER(S_air),
    g_FILLER(S_cloud),
    0xb2,   /* S_icewall:   solid block */
    0xb2,   /* S_crystalicewall:    solid block */
/*40*/ 0xf7,    /* S_water: meta-w, approx. equals */
    g_FILLER(S_arrow_trap),
    g_FILLER(S_dart_trap),
    g_FILLER(S_falling_rock_trap),
    g_FILLER(S_squeaky_board),
    g_FILLER(S_bear_trap),
    g_FILLER(S_land_mine),
    g_FILLER(S_rolling_boulder_trap),
/*50*/ g_FILLER(S_sleeping_gas_trap),
    g_FILLER(S_rust_trap),
    g_FILLER(S_fire_trap),
    g_FILLER(S_pit),
    g_FILLER(S_spiked_pit),
    g_FILLER(S_hole),
    g_FILLER(S_trap_door),
    g_FILLER(S_teleportation_trap),
    g_FILLER(S_level_teleporter),
    g_FILLER(S_magic_portal),
/*60*/ g_FILLER(S_web),
    g_FILLER(S_statue_trap),
    g_FILLER(S_magic_trap),
    g_FILLER(S_anti_magic_trap),
    g_FILLER(S_ice_trap),
    g_FILLER(S_polymorph_trap),
    g_FILLER(S_vibrating_square),
    0xb3,   /* S_vbeam: meta-3, vertical rule */
    0xc4,   /* S_hbeam: meta-D, horizontal rule */
    g_FILLER(S_lslant),
    g_FILLER(S_rslant),
    g_FILLER(S_poisoncloud),
    g_FILLER(S_goodpos),
/*70*/ g_FILLER(S_digbeam),
    g_FILLER(S_flashbeam),
    g_FILLER(S_boomleft),
    g_FILLER(S_boomright),
    g_FILLER(S_ss1),
    g_FILLER(S_ss2),
    g_FILLER(S_ss3),
    g_FILLER(S_ss4),
    g_FILLER(S_sw_tl),
    g_FILLER(S_sw_tc),
/*80*/ g_FILLER(S_sw_tr),
    0xb3,   /* S_sw_ml: meta-3, vertical rule */
    0xb3,   /* S_sw_mr: meta-3, vertical rule */
    g_FILLER(S_sw_bl),
    g_FILLER(S_sw_bc),
    g_FILLER(S_sw_br),
    g_FILLER(S_explode1),
    g_FILLER(S_explode2),
    g_FILLER(S_explode3),
    0xb3,   /* S_explode4:  meta-3, vertical rule */
/*90*/ g_FILLER(S_explode5),
    0xb3,   /* S_explode6:  meta-3, vertical rule */
    g_FILLER(S_explode7),
    g_FILLER(S_explode8),
    g_FILLER(S_explode9)
};
#endif  /* ASCIIGRAPH */

#ifdef TERMLIB
void (*decgraphics_mode_callback)(void) = 0;  /* set in tty_start_screen() */

static glyph_t dec_graphics[MAXPCHARS] = {
/* 0*/ g_FILLER(S_stone),
    0xf8,   /* S_vwall: meta-x, vertical rule */
    0xf1,   /* S_hwall: meta-q, horizontal rule */
    0xec,   /* S_tlcorn:    meta-l, top left corner */
    0xeb,   /* S_trcorn:    meta-k, top right corner */
    0xed,   /* S_blcorn:    meta-m, bottom left */
    0xea,   /* S_brcorn:    meta-j, bottom right */
    0xee,   /* S_crwall:    meta-n, cross */
    0xf6,   /* S_tuwall:    meta-v, T up */
    0xf7,   /* S_tdwall:    meta-w, T down */
/*10*/ 0xf5,    /* S_tlwall:    meta-u, T left */
    0xf4,   /* S_trwall:    meta-t, T right */
    0xfe,   /* S_ndoor: meta-~, centered dot */
    0xe1,   /* S_vodoor:    meta-a, solid block */
    0xe1,   /* S_hodoor:    meta-a, solid block */
    g_FILLER(S_vcdoor),
    g_FILLER(S_hcdoor),
    0xfb,   /* S_bars:  meta-{, small pi */
    0xe7,   /* S_tree:  meta-g, plus-or-minus */
    0xe7,   /* S_deadtree:  meta-g, plus-or-minus */
    0xfe,   /* S_room:  meta-~, centered dot */
    0xfe,   /* S_darkroom:  meta-~, centered dot */
/*20*/ g_FILLER(S_corr),
    g_FILLER(S_litcorr),
    g_FILLER(S_upstair),
    g_FILLER(S_dnstair),
    0xf9,   /* S_upladder:  meta-y, greater-than-or-equals */
    0xfa,   /* S_dnladder:  meta-z, less-than-or-equals */
    g_FILLER(S_altar),  /* 0xc3, \E)3: meta-C, dagger */
    g_FILLER(S_grave),
    g_FILLER(S_throne),
    g_FILLER(S_sink),
/*30*/ g_FILLER(S_fountain),    /* 0xdb, \E)3: meta-[, integral top half */
    0xe0,   /* S_pool:  meta-\, diamond */
    0xfe,   /* S_ice:   meta-~, centered dot */
    0xe0,   /* S_bog:   meta-\, diamond */
    0xe0,   /* S_lava:  meta-\, diamond */
    0xfe,   /* S_vodbridge: meta-~, centered dot */
    0xfe,   /* S_hodbridge: meta-~, centered dot */
    g_FILLER(S_vcdbridge),
    g_FILLER(S_hcdbridge),
    g_FILLER(S_air),
    g_FILLER(S_cloud),
    0xe1,   /* S_icewall,       solid block */
    0xe1,   /* S_crystalicewall,    solid block */
/*40*/ 0xe0,    /* S_water: meta-\, diamond */
    g_FILLER(S_arrow_trap),
    g_FILLER(S_dart_trap),
    g_FILLER(S_falling_rock_trap),
    g_FILLER(S_squeaky_board),
    g_FILLER(S_bear_trap),
    g_FILLER(S_land_mine),
    g_FILLER(S_rolling_boulder_trap),
/*50*/ g_FILLER(S_sleeping_gas_trap),
    g_FILLER(S_rust_trap),
    g_FILLER(S_fire_trap),
    g_FILLER(S_pit),
    g_FILLER(S_spiked_pit),
    g_FILLER(S_hole),
    g_FILLER(S_trap_door),
    g_FILLER(S_teleportation_trap),
    g_FILLER(S_level_teleporter),
    g_FILLER(S_magic_portal),
/*60*/ g_FILLER(S_web),     /* 0xbd, \E)3: meta-=, int'l currency */
    g_FILLER(S_statue_trap),
    g_FILLER(S_magic_trap),
    g_FILLER(S_anti_magic_trap),
    g_FILLER(S_ice_trap),
    g_FILLER(S_polymorph_trap),
    g_FILLER(S_vibrating_square),
    0xf8,   /* S_vbeam: meta-x, vertical rule */
    0xf1,   /* S_hbeam: meta-q, horizontal rule */
    g_FILLER(S_lslant),
    g_FILLER(S_rslant),
    g_FILLER(S_poisoncloud),
    g_FILLER(S_goodpos),
/*70*/ g_FILLER(S_digbeam),
    g_FILLER(S_flashbeam),
    g_FILLER(S_boomleft),
    g_FILLER(S_boomright),
    g_FILLER(S_ss1),
    g_FILLER(S_ss2),
    g_FILLER(S_ss3),
    g_FILLER(S_ss4),
    g_FILLER(S_sw_tl),
    0xef,   /* S_sw_tc: meta-o, high horizontal line */
/*80*/ g_FILLER(S_sw_tr),
    0xf8,   /* S_sw_ml: meta-x, vertical rule */
    0xf8,   /* S_sw_mr: meta-x, vertical rule */
    g_FILLER(S_sw_bl),
    0xf3,   /* S_sw_bc: meta-s, low horizontal line */
    g_FILLER(S_sw_br),
    g_FILLER(S_explode1),
    0xef,   /* S_explode2:  meta-o, high horizontal line */
    g_FILLER(S_explode3),
    0xf8,   /* S_explode4:  meta-x, vertical rule */
/*90*/ g_FILLER(S_explode5),
    0xf8,   /* S_explode6:  meta-x, vertical rule */
    g_FILLER(S_explode7),
    0xf3,   /* S_explode8:  meta-s, low horizontal line */
    g_FILLER(S_explode9)
};
#endif  /* TERMLIB */

#ifdef MAC_GRAPHICS_ENV
static glyph_t mac_graphics[MAXPCHARS] = {
/* 0*/ g_FILLER(S_stone),
    0xba,   /* S_vwall */
    0xcd,   /* S_hwall */
    0xc9,   /* S_tlcorn */
    0xbb,   /* S_trcorn */
    0xc8,   /* S_blcorn */
    0xbc,   /* S_brcorn */
    0xce,   /* S_crwall */
    0xca,   /* S_tuwall */
    0xcb,   /* S_tdwall */
/*10*/ 0xb9,    /* S_tlwall */
    0xcc,   /* S_trwall */
    0xb0,   /* S_ndoor */
    0xee,   /* S_vodoor */
    0xee,   /* S_hodoor */
    0xef,   /* S_vcdoor */
    0xef,   /* S_hcdoor */
    0xf0,   /* S_bars:  equivalency symbol */
    0xf1,   /* S_tree:  plus-or-minus */
    0xf1,   /* S_deadtree:  plus-or-minus */
    g_FILLER(S_Room),
    g_FILLER(S_stone), /* S_darkroom */
/*20*/ 0xB0,    /* S_corr */
    g_FILLER(S_litcorr),
    g_FILLER(S_upstair),
    g_FILLER(S_dnstair),
    g_FILLER(S_upladder),
    g_FILLER(S_dnladder),
    g_FILLER(S_altar),
    0xef,   /* S_grave: same as open door */
    g_FILLER(S_throne),
    g_FILLER(S_sink),
/*30*/ g_FILLER(S_fountain),
    0xe0,   /* S_pool */
    g_FILLER(S_ice),
    g_FILLER(S_bog),
    g_FILLER(S_lava),
    g_FILLER(S_vodbridge),
    g_FILLER(S_hodbridge),
    g_FILLER(S_vcdbridge),
    g_FILLER(S_hcdbridge),
    g_FILLER(S_air),
    g_FILLER(S_cloud),
    g_FILLER(S_icewall),
    g_FILLER(S_crystalicewall),
/*40*/ g_FILLER(S_water),
    g_FILLER(S_arrow_trap),
    g_FILLER(S_dart_trap),
    g_FILLER(S_falling_rock_trap),
    g_FILLER(S_squeaky_board),
    g_FILLER(S_bear_trap),
    g_FILLER(S_land_mine),
    g_FILLER(S_rolling_boulder_trap),
/*50*/ g_FILLER(S_sleeping_gas_trap),
    g_FILLER(S_rust_trap),
    g_FILLER(S_fire_trap),
    g_FILLER(S_pit),
    g_FILLER(S_spiked_pit),
    g_FILLER(S_hole),
    g_FILLER(S_trap_door),
    g_FILLER(S_teleportation_trap),
    g_FILLER(S_level_teleporter),
    g_FILLER(S_magic_portal),
/*60*/ g_FILLER(S_web),
    g_FILLER(S_statue_trap),
    g_FILLER(S_magic_trap),
    g_FILLER(S_anti_magic_trap),
    g_FILLER(S_ice_trap),
    g_FILLER(S_polymorph_trap),
    g_FILLER(S_vibrating_square),
    g_FILLER(S_vbeam),
    g_FILLER(S_hbeam),
    g_FILLER(S_lslant),
    g_FILLER(S_rslant),
    g_FILLER(S_poisoncloud),
    g_FILLER(S_goodpos),
/*70*/ g_FILLER(S_digbeam),
    g_FILLER(S_flashbeam),
    g_FILLER(S_boomleft),
    g_FILLER(S_boomright),
    g_FILLER(S_ss1),
    g_FILLER(S_ss2),
    g_FILLER(S_ss3),
    g_FILLER(S_ss4),
    g_FILLER(S_sw_tl),
    g_FILLER(S_sw_tc),
/*80*/ g_FILLER(S_sw_tr),
    g_FILLER(S_sw_ml),
    g_FILLER(S_sw_mr),
    g_FILLER(S_sw_bl),
    g_FILLER(S_sw_bc),
    g_FILLER(S_sw_br),
    g_FILLER(S_explode1),
    g_FILLER(S_explode2),
    g_FILLER(S_explode3),
    g_FILLER(S_explode4),
/*90*/ g_FILLER(S_explode5),
    g_FILLER(S_explode6),
    g_FILLER(S_explode7),
    g_FILLER(S_explode8),
    g_FILLER(S_explode9)
};
#endif  /* MAC_GRAPHICS_ENV */

#ifdef UTF8_GLYPHS
/* Probably best to only use characters from this list
 * http://en.wikipedia.org/wiki/WGL4 */
static glyph_t utf8_graphics[MAXPCHARS] = {
/* 0*/ g_FILLER(S_stone),
    0x2502, /* S_vwall: BOX DRAWINGS LIGHT VERTICAL */
    0x2500, /* S_hwall: BOX DRAWINGS LIGHT HORIZONTAL */
    0x250c, /* S_tlcorn:    BOX DRAWINGS LIGHT DOWN AND RIGHT */
    0x2510, /* S_trcorn:    BOX DRAWINGS LIGHT DOWN AND LEFT */
    0x2514, /* S_blcorn:    BOX DRAWINGS LIGHT UP AND RIGHT */
    0x2518, /* S_brcorn:    BOX DRAWINGS LIGHT UP AND LEFT */
    0x253c, /* S_crwall:    BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
    0x2534, /* S_tuwall:    BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    0x252c, /* S_tdwall:    BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    0x2524, /* S_tlwall:    BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    0x251c, /* S_trwall:    BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    0x00b7, /* S_ndoor: MIDDLE DOT */
    0x2592, /* S_vodoor:    MEDIUM SHADE */
    0x2592, /* S_hodoor:    MEDIUM SHADE */
    g_FILLER(S_vcdoor),
    g_FILLER(S_hcdoor),
    0x2261, /* S_bars:  IDENTICAL TO */
    0x03a8, /* S_tree:  GREEK CAPITAL LETTER PSI */
    0x03a8, /* S_deadtree:  GREEK CAPITAL LETTER PSI */
    0x00b7, /* S_room:  MIDDLE DOT */
    0x00b7, /* S_darkroom:  MIDDLE DOT */
    g_FILLER(S_corr),
    g_FILLER(S_litcorr),
    g_FILLER(S_upstair),
    g_FILLER(S_dnstair),
    0x2264, /* S_upladder:  LESS-THAN OR EQUAL TO */
    0x2265, /* S_dnladder:  GREATER-THAN OR EQUAL TO */
    0x03A9, /* S_altar: GREEK CAPITAL LETTER OMEGA */
    0x2020, /* S_grave: DAGGER */
    g_FILLER(S_throne),
    g_FILLER(S_sink),
    0x00b6, /* S_fountain:  PILCROW SIGN */
    0x224b, /* S_pool:  TRIPLE TILDE */
    0x00b7, /* S_ice:   MIDDLE DOT */
    0x224b, /* S_bog:   TRIPLE TILDE */
    0x224b, /* S_lava:  TRIPLE TILDE */
    0x00b7, /* S_vodbridge: MIDDLE DOT */
    0x00b7, /* S_hodbridge: MIDDLE DOT */
    g_FILLER(S_vcdbridge),
    g_FILLER(S_hcdbridge),
    g_FILLER(S_air),
    g_FILLER(S_cloud),
    0x2591, /* S_icewall:   LIGHT SHADE */
    0x2591, /* S_crystalicewall:    LIGHT SHADE */
    0x2248, /* S_water: ALMOST EQUAL TO */
    g_FILLER(S_arrow_trap),
    g_FILLER(S_dart_trap),
    g_FILLER(S_falling_rock_trap),
    g_FILLER(S_squeaky_board),
    g_FILLER(S_bear_trap),
    g_FILLER(S_land_mine),
    g_FILLER(S_rolling_boulder_trap),
    g_FILLER(S_sleeping_gas_trap),
    g_FILLER(S_rust_trap),
    g_FILLER(S_fire_trap),
    g_FILLER(S_pit),
    g_FILLER(S_spiked_pit),
    g_FILLER(S_hole),
    g_FILLER(S_trap_door),
    g_FILLER(S_teleportation_trap),
    g_FILLER(S_level_teleporter),
    g_FILLER(S_magic_portal),
    0x00A4, /* S_web:   CURRENCY SIGN */
    g_FILLER(S_statue_trap),
    g_FILLER(S_magic_trap),
    g_FILLER(S_anti_magic_trap),
    g_FILLER(S_ice_trap),
    g_FILLER(S_polymorph_trap),
    g_FILLER(S_vibrating_square),
    0x2502, /* S_vbeam: BOX DRAWINGS LIGHT VERTICAL */
    0x2500, /* S_hbeam: BOX DRAWINGS LIGHT HORIZONTAL */
    g_FILLER(S_lslant),
    g_FILLER(S_rslant),
    g_FILLER(S_poisoncloud),
    g_FILLER(S_goodpos),
    g_FILLER(S_digbeam),
    g_FILLER(S_flashbeam),
    g_FILLER(S_boomleft),
    g_FILLER(S_boomright),
    g_FILLER(S_ss1),
    g_FILLER(S_ss2),
    g_FILLER(S_ss3),
    g_FILLER(S_ss4),
    g_FILLER(S_sw_tl),
    0x2594, /* S_sw_tc: UPPER ONE EIGHTH BLOCK */
    g_FILLER(S_sw_tr),
    0x258f, /* S_sw_ml: LEFT ONE EIGHTH BLOCK */
    0x2595, /* S_sw_mr: RIGHT ONE EIGHTH BLOCK */
    g_FILLER(S_sw_bl),
    0x2581, /* S_sw_bc: LOWER ONE EIGHTH BLOCK */
    g_FILLER(S_sw_br),
    g_FILLER(S_explode1),
    0x2594, /* S_explode2:  UPPER ONE EIGHTH BLOCK */
    g_FILLER(S_explode3),
    0x258f, /* S_explode4:  LEFT ONE EIGHTH BLOCK */
    g_FILLER(S_explode5),
    0x2595, /* S_explode6:  RIGHT ONE EIGHTH BLOCK */
    g_FILLER(S_explode7),
    0x2581, /* S_explode8:  LOWER ONE EIGHTH BLOCK */
    g_FILLER(S_explode9)
};
#endif

#ifdef PC9800
void (*ascgraphics_mode_callback)(void) = 0;   /* set in tty_start_screen() */
#endif

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in detect.c invent.c,
 * options.c, pickup.c, sp_lev.c, and lev_main.c.
 */
int
def_char_to_objclass(ch)
char ch;
{
    int i;
    for (i = 1; i < MAXOCLASSES; i++)
        if (ch == def_oc_syms[i]) break;
    return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 */
int
def_char_to_monclass(ch)
char ch;
{
    int i;
    for (i = 1; i < MAXMCLASSES; i++)
        if (def_monsyms[i] == ch) break;
    return i;
}

void
assign_graphics(graph_chars, glth, maxlen, offset)
register glyph_t *graph_chars;
int glth, maxlen, offset;
{
    register int i;

    for (i = 0; i < maxlen; i++)
        showsyms[i+offset] = (((i < glth) && graph_chars[i]) ?
                              graph_chars[i] : defsyms[i+offset].sym);
}

void
switch_graphics(gr_set_flag)
int gr_set_flag;
{
    iflags.IBMgraphics = FALSE;
    iflags.DECgraphics = FALSE;
#ifdef CURSES_GRAPHICS
    iflags.cursesgraphics = FALSE;
#endif
    iflags.UTF8graphics = FALSE;

    switch (gr_set_flag) {
    default:
    case ASCII_GRAPHICS:
        assign_graphics((glyph_t *)0, 0, MAXPCHARS, 0);
#ifdef PC9800
        if (ascgraphics_mode_callback) (*ascgraphics_mode_callback)();
#endif
        break;
#ifdef ASCIIGRAPH
    case IBM_GRAPHICS:
/*
 * Use the nice IBM Extended ASCII line-drawing characters (codepage 437).
 *
 * OS/2 defaults to a multilingual character set (codepage 850, corresponding
 * to the ISO 8859 character set.  We should probably do a VioSetCp() call to
 * set the codepage to 437.
 */
        iflags.IBMgraphics = TRUE;
        iflags.DECgraphics = FALSE;
#ifdef CURSES_GRAPHICS
        iflags.cursesgraphics = FALSE;
#endif
        assign_graphics(ibm_graphics, SIZE(ibm_graphics), MAXPCHARS, 0);
#ifdef PC9800
        if (ibmgraphics_mode_callback) (*ibmgraphics_mode_callback)();
#endif
        break;
#endif /* ASCIIGRAPH */
#ifdef TERMLIB
    case DEC_GRAPHICS:
/*
 * Use the VT100 line drawing character set.
 */
        iflags.DECgraphics = TRUE;
        iflags.IBMgraphics = FALSE;
#ifdef CURSES_GRAPHICS
        iflags.cursesgraphics = FALSE;
#endif
        assign_graphics(dec_graphics, SIZE(dec_graphics), MAXPCHARS, 0);
        if (decgraphics_mode_callback) (*decgraphics_mode_callback)();
        break;
#endif /* TERMLIB */
#ifdef MAC_GRAPHICS_ENV
    case MAC_GRAPHICS:
        assign_graphics(mac_graphics, SIZE(mac_graphics), MAXPCHARS, 0);
        break;
#endif
#ifdef CURSES_GRAPHICS
    case CURS_GRAPHICS:
        assign_graphics((glyph_t *)0, 0, MAXPCHARS, 0);
        iflags.cursesgraphics = TRUE;
        iflags.IBMgraphics = FALSE;
        iflags.DECgraphics = FALSE;
        break;
#endif
#ifdef UTF8_GLYPHS
    case UTF8_GRAPHICS:
        assign_graphics(utf8_graphics, SIZE(utf8_graphics), MAXPCHARS, 0);
        iflags.UTF8graphics = TRUE;
        break;
#endif
    }
    return;
}

/** Change the UTF8graphics symbol at position with codepoint "value". */
void
assign_utf8graphics_symbol(position, value)
int position;
glyph_t value;
{
#ifdef UTF8_GLYPHS
    if (position < MAXPCHARS) {
        utf8_graphics[position] = value;
        /* need to update showsym */
        if (iflags.UTF8graphics) {
            switch_graphics(UTF8_GRAPHICS);
        }
    }
#endif
}

/*
 * saved display symbols for objects & features.
 */
static uchar save_oc_syms[MAXOCLASSES]  = DUMMY;
static glyph_t save_showsyms[MAXPCHARS] = DUMMY;
static uchar save_monsyms[MAXPCHARS]    = DUMMY;

void
save_syms()
{
    (void) memcpy((genericptr_t)save_showsyms,
                  (genericptr_t)showsyms, sizeof showsyms);
    (void) memcpy((genericptr_t)save_oc_syms,
                  (genericptr_t)oc_syms, sizeof oc_syms);
    (void) memcpy((genericptr_t)save_monsyms,
                  (genericptr_t)monsyms, sizeof monsyms);
}

void
restore_syms()
{
    (void) memcpy((genericptr_t)showsyms,
                  (genericptr_t)save_showsyms, sizeof showsyms);
    (void) memcpy((genericptr_t)oc_syms,
                  (genericptr_t)save_oc_syms, sizeof oc_syms);
    (void) memcpy((genericptr_t)monsyms,
                  (genericptr_t)save_monsyms, sizeof monsyms);
}

#ifdef REINCARNATION

static const glyph_t r_oc_syms[MAXOCLASSES] = {
/* 0*/ '\0',
    ILLOBJ_SYM,
    WEAPON_SYM,
    ']',            /* armor */
    RING_SYM,
/* 5*/ ',',             /* amulet */
    TOOL_SYM,
    ':',            /* food */
    POTION_SYM,
    SCROLL_SYM,
/*10*/ SPBOOK_SYM,
    WAND_SYM,
    GEM_SYM,        /* gold -- yes it's the same as gems */
    GEM_SYM,
    ROCK_SYM,
/*15*/ BALL_SYM,
    CHAIN_SYM,
    VENOM_SYM
};

# ifdef ASCIIGRAPH
/* Rogue level graphics.  Under IBM graphics mode, use the symbols that were
 * used for Rogue on the IBM PC.  Unfortunately, this can't be completely
 * done because some of these are control characters--armor and rings under
 * DOS, and a whole bunch of them under Linux.  Use the TTY Rogue characters
 * for those cases.
 */
static const uchar IBM_r_oc_syms[MAXOCLASSES] = {   /* a la EPYX Rogue */
/* 0*/ '\0',
    ILLOBJ_SYM,
#  if defined(MSDOS) || defined(OS2) || ( defined(WIN32) && !defined(MSWIN_GRAPHICS) )
    0x18,           /* weapon: up arrow */
/*  0x0a, */ ARMOR_SYM,  /* armor:  Vert rect with o */
/*  0x09, */ RING_SYM,   /* ring:   circle with arrow */
/* 5*/ 0x0c,            /* amulet: "female" symbol */
    TOOL_SYM,
    0x05,           /* food:   club (as in cards) */
    0xad,           /* potion: upside down '!' */
    0x0e,           /* scroll: musical note */
/*10*/ SPBOOK_SYM,
    0xe7,           /* wand:   greek tau */
    0x0f,           /* gold:   yes it's the same as gems */
    0x0f,           /* gems:   fancy '*' */
#  else
    ')',            /* weapon  */
    ARMOR_SYM,      /* armor */
    RING_SYM,       /* ring */
/* 5*/ ',',             /* amulet  */
    TOOL_SYM,
    ':',            /* food    */
    0xad,           /* potion: upside down '!' */
    SCROLL_SYM,     /* scroll  */
/*10*/ SPBOOK_SYM,
    0xe7,           /* wand:   greek tau */
    GEM_SYM,        /* gold:   yes it's the same as gems */
    GEM_SYM,        /* gems    */
#  endif
    ROCK_SYM,
/*15*/ BALL_SYM,
    CHAIN_SYM,
    VENOM_SYM
};
# endif /* ASCIIGRAPH */

void
assign_rogue_graphics(is_rlevel)
boolean is_rlevel;
{
    /* Adjust graphics display characters on Rogue levels */

    if (is_rlevel) {
        register int i;

        save_syms();

        /* Use a loop: char != uchar on some machines. */
        for (i = 0; i < MAXMCLASSES; i++)
            monsyms[i] = def_monsyms[i];
# if defined(ASCIIGRAPH) && !defined(MSWIN_GRAPHICS)
        if (iflags.IBMgraphics
#  if defined(USE_TILES) && defined(MSDOS)
            && !iflags.grmode
#  endif
            )
            monsyms[S_HUMAN] = 0x01; /* smiley face */
# endif
        for (i = 0; i < MAXPCHARS; i++)
            showsyms[i] = defsyms[i].sym;

/*
 * Some day if these rogue showsyms get much more extensive than this,
 * we may want to create r_showsyms, and IBM_r_showsyms arrays to hold
 * all of this info and to simply initialize it via a for() loop like r_oc_syms.
 */

# ifdef ASCIIGRAPH
        if (!iflags.IBMgraphics
#  if defined(USE_TILES) && defined(MSDOS)
            || iflags.grmode
#  endif
            ) {
# endif
        showsyms[S_vodoor]  = showsyms[S_hodoor]  = showsyms[S_ndoor] = '+';
        showsyms[S_upstair] = showsyms[S_dnstair] = '%';
# ifdef ASCIIGRAPH
    } else {
        /* a la EPYX Rogue */
        showsyms[S_vwall]   = 0xba; /* all walls now use    */
        showsyms[S_hwall]   = 0xcd; /* double line graphics */
        showsyms[S_tlcorn]  = 0xc9;
        showsyms[S_trcorn]  = 0xbb;
        showsyms[S_blcorn]  = 0xc8;
        showsyms[S_brcorn]  = 0xbc;
        showsyms[S_crwall]  = 0xce;
        showsyms[S_tuwall]  = 0xca;
        showsyms[S_tdwall]  = 0xcb;
        showsyms[S_tlwall]  = 0xb9;
        showsyms[S_trwall]  = 0xcc;
        showsyms[S_ndoor]   = 0xce;
        showsyms[S_vodoor]  = 0xce;
        showsyms[S_hodoor]  = 0xce;
        showsyms[S_room]    = 0xfa; /* centered dot */
        showsyms[S_corr]    = 0xb1;
        showsyms[S_litcorr] = 0xb2;
        showsyms[S_upstair] = 0xf0; /* Greek Xi */
        showsyms[S_dnstair] = 0xf0;
#ifndef MSWIN_GRAPHICS
#endif
    }
#endif /* ASCIIGRAPH */

    } else {
        restore_syms();
    }
}
#endif /* REINCARNATION */

void
assign_moria_graphics(is_moria)
boolean is_moria;
{
    /* Adjust graphics display characters on Moria levels */

    if (is_moria) {
        save_syms();

        showsyms[S_vwall]   = '#';
        showsyms[S_hwall]   = '#';
        showsyms[S_tlcorn]  = '#';
        showsyms[S_trcorn]  = '#';
        showsyms[S_blcorn]  = '#';
        showsyms[S_brcorn]  = '#';
        showsyms[S_crwall]  = '#';
        showsyms[S_tuwall]  = '#';
        showsyms[S_tdwall]  = '#';
        showsyms[S_tlwall]  = '#';
        showsyms[S_trwall]  = '#';

        showsyms[S_corr]    = '#';
        showsyms[S_litcorr] = '#';
    } else {
        restore_syms();
    }
}

/*drawing.c*/
