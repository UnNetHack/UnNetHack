/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object MonFlag {

val MS_SILENT = 0	/* makes no sound */
val MS_BARK = 1		/* if full moon, may howl */
val MS_MEW = 2		/* mews or hisses */
val MS_ROAR = 3		/* roars */
val MS_GROWL = 4	/* growls */
val MS_SQEEK = 5	/* squeaks, as a rodent */
val MS_SQAWK = 6	/* squawks, as a bird */
val MS_HISS = 7		/* hisses */
val MS_BUZZ = 8		/* buzzes (killer bee) */
val MS_GRUNT = 9	/* grunts (or speaks own language) */
val MS_NEIGH = 10	/* neighs, as an equine */
val MS_WAIL = 11	/* wails, as a tortured soul */
val MS_GURGLE = 12	/* gurgles, as liquid or through saliva */
val MS_BURBLE = 13	/* burbles (jabberwock) */
val MS_ANIMAL = 13	/* up to here are animal noises */
val MS_SHRIEK = 15	/* wakes up others */
val MS_BONES = 16	/* rattles bones (skeleton) */
val MS_LAUGH = 17	/* grins, smiles, giggles, and laughs */
val MS_MUMBLE = 18	/* says something or other */
val MS_IMITATE = 19	/* imitates others (leocrotta) */
val MS_ORC = MS_GRUNT	/* intelligent brutes */
val MS_HUMANOID = 20	/* generic traveling companion */
val MS_ARREST = 21	/* "Stop in the name of the law!" (Kops) */
val MS_SOLDIER = 22	/* army and watchmen expressions */
val MS_GUARD = 23	/* "Please drop that gold and follow me." */
val MS_DJINNI = 24	/* "Thank you for freeing me!" */
val MS_NURSE = 25	/* "Take off your shirt, please." */
val MS_SEDUCE = 26	/* "Hello, sailor." (Nymphs) */
val MS_VAMPIRE = 27	/* vampiric seduction, Vlad's exclamations */
val MS_BRIBE = 28	/* asks for money, or berates you */
val MS_CUSS = 29	/* berates (demons) or intimidates (Wiz) */
val MS_RIDER = 30	/* astral level special monsters */
val MS_LEADER = 31	/* your class leader */
val MS_NEMESIS = 32	/* your nemesis */
val MS_GUARDIAN = 33	/* your leader's guards */
val MS_SELL = 34	/* demand payment, complain about shoplifters */
val MS_ORACLE = 35	/* do a consultation */
val MS_PRIEST = 36	/* ask for contribution; do cleansing */
val MS_SPELL = 37	/* spellcaster not matching any of the above */
val MS_WERE = 38	/* lycanthrope in human form */
val MS_BOAST = 39	/* giants */


val MR_FIRE = 0x01	/* resists fire */
val MR_COLD = 0x02	/* resists cold */
val MR_SLEEP = 0x04	/* resists sleep */
val MR_DISINT = 0x08	/* resists disintegration */
val MR_ELEC = 0x10	/* resists electricity */
val MR_POISON = 0x20	/* resists poison */
val MR_ACID = 0x40	/* resists acid */
val MR_STONE = 0x80	/* resists petrification */
/* other resistances: magic, sickness */
/* other conveyances: teleport, teleport control, telepathy */

/* individual resistances */
val MR2_SEE_INVIS = 0x0100	/* see invisible */
val MR2_LEVITATE = 0x0200	/* levitation */
val MR2_WATERWALK = 0x0400	/* water walking */
val MR2_MAGBREATH = 0x0800	/* magical breathing */
val MR2_DISPLACED = 0x1000	/* displaced */
val MR2_STRENGTH = 0x2000	/* gauntlets of power */
val MR2_FUMBLING = 0x4000	/* clumsy */


val M1_FLY = 0x00000001L	/* can fly or float */
val M1_SWIM = 0x00000002L	/* can traverse water */
val M1_AMORPHOUS = 0x00000004L	/* can flow under doors */
val M1_WALLWALK = 0x00000008L	/* can phase thru rock */
val M1_CLING = 0x00000010L	/* can cling to ceiling */
val M1_TUNNEL = 0x00000020L	/* can tunnel thru rock */
val M1_NEEDPICK = 0x00000040L	/* needs pick to tunnel */
val M1_CONCEAL = 0x00000080L	/* hides under objects */
val M1_HIDE = 0x00000100L	/* mimics, blends in with ceiling */
val M1_AMPHIBIOUS = 0x00000200L	/* can survive underwater */
val M1_BREATHLESS = 0x00000400L	/* doesn't need to breathe */
val M1_NOTAKE = 0x00000800L	/* cannot pick up objects */
val M1_NOEYES = 0x00001000L	/* no eyes to gaze into or blind */
val M1_NOHANDS = 0x00002000L	/* no hands to handle things */
val M1_NOLIMBS = 0x00006000L	/* no arms/legs to kick/wear on */
val M1_NOHEAD = 0x00008000L	/* no head to behead */
val M1_MINDLESS = 0x00010000L	/* has no mind--golem, zombie, mold */
val M1_HUMANOID = 0x00020000L	/* has humanoid head/arms/torso */
val M1_ANIMAL = 0x00040000L	/* has animal body */
val M1_SLITHY = 0x00080000L	/* has serpent body */
val M1_UNSOLID = 0x00100000L	/* has no solid or liquid body */
val M1_THICK_HIDE = 0x00200000L	/* has thick hide or scales */
val M1_OVIPAROUS = 0x00400000L	/* can lay eggs */
val M1_REGEN = 0x00800000L	/* regenerates hit points */
val M1_SEE_INVIS = 0x01000000L	/* can see invisible creatures */
val M1_TPORT = 0x02000000L	/* can teleport */
val M1_TPORT_CNTRL = 0x04000000L	/* controls where it teleports to */
val M1_ACID = 0x08000000L	/* acidic to eat */
val M1_POIS = 0x10000000L	/* poisonous to eat */
val M1_CARNIVORE = 0x20000000L	/* eats corpses */
val M1_HERBIVORE = 0x40000000L	/* eats fruits */
val M1_OMNIVORE = 0x60000000L	/* eats both */
val M1_METALLIVORE = 0x80000000L	/* eats metal */

val M2_NOPOLY = 0x00000001L	/* players mayn't poly into one */
val M2_UNDEAD = 0x00000002L	/* is walking dead */
val M2_WERE = 0x00000004L	/* is a lycanthrope */
val M2_HUMAN = 0x00000008L	/* is a human */
val M2_ELF = 0x00000010L	/* is an elf */
val M2_DWARF = 0x00000020L	/* is a dwarf */
val M2_GNOME = 0x00000040L	/* is a gnome */
val M2_ORC = 0x00000080L	/* is an orc */
val M2_VAMPIRE = 0x00000100L	/* is a vampire */
val M2_DEMON = 0x00000200L	/* is a demon */
val M2_MERC = 0x00000400L	/* is a guard or soldier */
val M2_LORD = 0x00000800L	/* is a lord to its kind */
val M2_PRINCE = 0x00001000L	/* is an overlord to its kind */
val M2_MINION = 0x00002000L	/* is a minion of a deity */
val M2_GIANT = 0x00004000L	/* is a giant */

val M2_MALE = 0x00010000L	/* always male */
val M2_FEMALE = 0x00020000L	/* always female */
val M2_NEUTER = 0x00040000L	/* neither male nor female */
val M2_PNAME = 0x00080000L	/* monster name is a proper name */
val M2_HOSTILE = 0x00100000L	/* always starts hostile */
val M2_PEACEFUL = 0x00200000L	/* always starts peaceful */
val M2_DOMESTIC = 0x00400000L	/* can be tamed by feeding */
val M2_WANDER = 0x00800000L	/* wanders randomly */
val M2_STALK = 0x01000000L	/* follows you to other levels */
val M2_NASTY = 0x02000000L	/* extra-nasty monster (more xp) */
val M2_STRONG = 0x04000000L	/* strong (or big) monster */
val M2_ROCKTHROW = 0x08000000L	/* throws boulders */
val M2_GREEDY = 0x10000000L	/* likes gold */
val M2_JEWELS = 0x20000000L	/* likes gems */
val M2_COLLECT = 0x40000000L	/* picks up weapons and food */
val M2_MAGIC = 0x80000000L	/* picks up magic items */

val M3_WANTSAMUL = 0x0001		/* would like to steal the amulet */
val M3_WANTSBELL = 0x0002		/* wants the bell */
val M3_WANTSBOOK = 0x0004		/* wants the book */
val M3_WANTSCAND = 0x0008		/* wants the candelabrum */
val M3_WANTSARTI = 0x0010		/* wants the quest artifact */
val M3_WANTSALL = 0x001f		/* wants any major artifact */
val M3_WAITFORU = 0x0040		/* waits to see you or get attacked */
val M3_CLOSE = 0x0080		/* lets you close unless attacked */

val M3_COVETOUS = 0x001f		/* wants something */
val M3_WAITMASK = 0x00c0		/* waiting... */

/* Infravision is currently implemented for players only */
val M3_INFRAVISION = 0x0100		/* has infravision */
val M3_INFRAVISIBLE = 0x0200		/* visible by infravision */

val M3_GROUPATTACK = 0x0400		/* hits and runs in groups */
val M3_BLINKER = 0x0800		/* alternates between regenerating
					   and attacking. */
val M3_NOREGEN = 0x1000		/* does not naturally regenerate
					   hp. */
val M3_STATIONARY = 0x2000		/* does not move. */

val MZ_TINY = 0			/* < 2' */
val MZ_SMALL = 1		/* 2-4' */
val MZ_MEDIUM = 2		/* 4-7' */
val MZ_HUMAN = MZ_MEDIUM	/* human-sized */
val MZ_LARGE = 3		/* 7-12' */
val MZ_HUGE = 4			/* 12-25' */
val MZ_GIGANTIC = 7		/* off the scale */


/* Monster races -- must stay within ROLE_RACEMASK */
/* Eventually this may become its own field */
val MH_HUMAN = M2_HUMAN
val MH_ELF = M2_ELF
val MH_DWARF = M2_DWARF
val MH_GNOME = M2_GNOME
val MH_ORC = M2_ORC
val MH_VAMPIRE = M2_VAMPIRE


/* for mons[].geno (constant during game) */

val G_UNIQ = 0x4000		/* generated only once */
val G_NOSHEOL = 0x2000		/* not generated in Sheol */
val G_SHEOL = 0x1000		/* generated only in Sheol, unless specified otherwise */
val G_NOHELL = 0x0800		/* not generated in "hell" (Gehennon except Sheol) */
val G_HELL = 0x0400		/* generated only in "hell", unless specified otherwise */
val G_NOGEN = 0x0200		/* generated only specially */
val G_SGROUP = 0x0080		/* appear in small groups normally */
val G_LGROUP = 0x0040		/* appear in large groups normally */
val G_GENO = 0x0020		/* can be genocided */
val G_NOCORPSE = 0x0010		/* no corpse left ever */
val G_FREQ = 0x0007		/* creation frequency mask */

/* for mvitals[].mvflags (variant during game), along with G_NOCORPSE */
val G_KNOWN = 0x0004		/* have been encountered */
val G_GONE = (G_GENOD|G_EXTINCT)
val G_GENOD = 0x0002		/* have been genocided */
val G_EXTINCT = 0x0001		/* have been extinguished as
					   population control */
val MV_KNOWS_EGG = 0x0008		/* player recognizes egg of this
					   monster type */
}
