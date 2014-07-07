/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object PatchLevel {

/* UnNetHack 5.1.0 */
val GAME_SHORT_NAME = "UNH"
val VERSION_MAJOR = 5
val VERSION_MINOR = 1
/*
 * PATCHLEVEL is updated for each release.
 */
val PATCHLEVEL = 0
/*
 * Incrementing EDITLEVEL can be used to force invalidation of old bones
 * and save files.
 */
val EDITLEVEL = 0

/*
 * UnNetHack follows a more modern approach regarding version numbers:
 * major.minor.patchlevel[-revision]
 *
 * Odd-numbered minor versions are for development releases with no guarantees
 * for save and bones compatibility between different patchlevels, whereas
 * even-numbered minor releases are considered stable releases with save and
 * bones compatibility within the same minor number releases.
 *
 * Releases with increasing revision numbers are always save and bones compatbile
 * and are considered only bugfix releases. Therefore the revision number isn't
 * included into VERSION_COMPATIBILITY.
 */
/* val VERSION_REVISION = 1 */

val COPYRIGHT_BANNER_A = "UnNetHack 5, Copyright 2009-2013"

val COPYRIGHT_BANNER_B = "    Based on NetHack, Copyright 1985-2003 by Stichting Mathematisch"

val COPYRIGHT_BANNER_C = "         Centrum and M. Stephenson.  See license for details."

/*
 * If two or more successive releases have compatible data files, define
 * this with the version number of the oldest such release so that the
 * new release will accept old save and bones files.  The format is
 *	0xMMmmPPeeL
 * 0x = literal prefix "0x", MM = major version, mm = minor version,
 * PP = patch level, ee = edit level, L = literal suffix "L",
 * with all four numbers specified as two hexadecimal digits.
 */
/* val VERSION_COMPATIBILITY = 0x05000000L */	/* 5.0.0-e0 */
}
