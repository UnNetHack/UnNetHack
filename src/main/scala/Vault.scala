/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Vault {

val FCSIZ = (ROWNO+COLNO)
class fakecorridor {
	var fx,fy,ftyp: xchar = 0
}

class egd {
	var fcbeg, fcend = 0		/* fcend: first unused pos */
	var vroom = 0			/* room number of the vault */
	var gdx, gdy: xchar = 0		/* goal of guard's walk */
	var ogx, ogy: xchar = 0		/* guard's last position */
	val gdlevel = new d_level()	/* level (& dungeon) guard was created in */
	var warncnt: xchar = 0		/* number of warnings to follow */
	var gddone = false		/* true iff guard has released player */
	var unused: Byte = 0
	val fakecorr = new Array[fakecorridor](FCSIZ)
}

// MOTODO #define EGD(mon)	((struct egd *)&(mon)->mextra[0])
}
