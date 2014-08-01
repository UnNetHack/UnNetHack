/* Copyright (c) Izchak Miller, 1989.				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import Coord._
import Config._
import Dungeon._

object EPri {

class epri {
	var shralign = A_NONE		/* alignment of priest's shrine */
					/* leave as first field to match emin */
	var shroom: schar = 0		/* index in rooms */
	var shrpos = new coord()	/* position of shrine */
	var shrlevel = new d_level()	/* level (& dungeon) of shrine */
};

// MOTODO #define EPRI(mon)	((struct epri *)&(mon)->mextra[0])

/* A priest without ispriest is a roaming priest without a shrine, so
 * the fields (except shralign, which becomes only the priest alignment)
 * are available for reuse.
 */
// MOTODO #define renegade shroom
}
