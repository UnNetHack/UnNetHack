/* Copyright 1986, M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Global.xchar

object Spell {

class spell {
    var sp_id: Short = 0			/* spell id (== object.otyp) */
    var sp_lev: xchar = _	/* power level */
    var sp_know = 0		/* knowlege of spell */
}

/* levels of memory destruction with a scroll of amnesia */
val ALL_MAP = 0x1
val ALL_SPELLS = 0x2

def decrnknow(spell: spell) = spl_book(spell).sp_know = spl_book(spell) - 1
def spellid(spell: spell) = spl_book(spell).sp_id
def spellknow(spell: spell) = spl_book(spell).sp_know
}
