/* Copyright (c) Mike Stephenson, Izchak Miller  1991.		  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Decl.moves
import Config._

object Align {

type aligntyp = schar /* basic alignment type */

/** alignment and record */
class align { 
  var aligntype: aligntyp = A_NONE // MONOTE: Was "type", but that is a reserved word.  Remove this when conversion done.
  var record = 0
}

/* bounds for "record" -- respect initial alignments of 10 */
def ALIGNLIM = (10L + (moves / 200L))

val A_NONE: aligntyp = -128	/* the value range of type */

val A_CHAOTIC: aligntyp = -1
val A_NEUTRAL: aligntyp = 0
val A_LAWFUL: aligntyp = 1

val A_COALIGNED: aligntyp = 1
val A_OPALIGNED: aligntyp = -1

val AM_NONE = 0
val AM_CHAOTIC = 1
val AM_NEUTRAL = 2
val AM_LAWFUL = 4

val AM_MASK = 7

val AM_SPLEV_CO = 3
val AM_SPLEV_NONCO = 7

// MOTODO def Amask2align(x: Int) = (aligntyp) if (!x) A_NONE else if (x == AM_LAWFUL)  A_LAWFUL else ((int)x) - 2))
def Align2amask(x: aligntyp) = if (x == A_NONE) AM_NONE else if (x == A_LAWFUL) AM_LAWFUL else (x + 2)
}
