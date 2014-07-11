/* Copyright 1988, Mike Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

/*	attrib.h - Header file for character class processing. */
object Attrib {

val A_STR = 0
val A_INT = 1
val A_WIS = 2
val A_DEX = 3
val A_CON = 4
val A_CHA = 5

val A_MAX = 6	/* used in rn2() selection of attrib */

def ABASE(x: Int) =	(u.acurr.a(x))
def ABON(x: Int) =	(u.abon.a(x))
def AEXE(x: Int) =	(u.aexe.a(x))
def ACURR(x: Int) =	(acurr(x))
def ACURRSTR = 	(acurrstr())
/* should be: */
/* def ACURR(x: Int) = (ABON(x) + ATEMP(x) + (Upolyd  ? MBASE(x) : ABASE(x)) */
def MCURR(x: Int) =	(u.macurr.a(x))
def AMAX(x: Int) =	(u.amax.a(x))
def MMAX(x: Int) =	(u.mamax.a(x))

def ATEMP(x: Int) =	(u.atemp.a(x))
def ATIME(x: Int) =	(u.atime.a(x))

/* KMH -- Conveniences when dealing with strength constants */
def STR18(x: Int) = (18+(x))	/* 18/xx */
def STR19(x: Int) = (100+(x))	/* For 19 and above */

class attribs {
	var a = new Array[schar](A_MAX)
}

// MOTODO #define ATTRMAX(x) ((x == A_STR && Upolyd && strongmonst(youmonst.data)) ? STR18(100) : urace.attrmax(x))
// MOTODO #define ATTRMIN(x) (urace.attrmin(x))

}
