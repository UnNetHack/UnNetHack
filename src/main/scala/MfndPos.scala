/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object MfndPos {

val ALLOW_TRAPS = 0x00020000L	/* can enter traps */
val ALLOW_U = 0x00040000L	/* can attack you */
val ALLOW_M = 0x00080000L	/* can attack other monsters */
val ALLOW_TM = 0x00100000L	/* can attack tame monsters */
val ALLOW_ALL = (ALLOW_U | ALLOW_M | ALLOW_TM | ALLOW_TRAPS)
val NOTONL = 0x00200000L	/* avoids direct line to player */
val OPENDOOR = 0x00400000L	/* opens closed doors */
val UNLOCKDOOR = 0x00800000L	/* unlocks locked doors */
val BUSTDOOR = 0x01000000L	/* breaks any doors */
val ALLOW_ROCK = 0x02000000L	/* pushes rocks */
val ALLOW_WALL = 0x04000000L	/* walks thru walls */
val ALLOW_DIG = 0x08000000L	/* digs */
val ALLOW_BARS = 0x10000000L	/* may pass thru iron bars */
val ALLOW_SANCT = 0x20000000L	/* enters temples */
val ALLOW_SSM = 0x40000000L	/* ignores scare monster */
val NOGARLIC =0x80000000L	/* hates garlic */

}
