/* Copyright 1994, Dean Luick					  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Timeout {
/* generic timeout function */
type timeout_proc = (Object,Int) => Unit

  /* kind of timer */
  val TIMER_LEVEL = 0	/* event specific to level */
  val TIMER_GLOBAL = 1	/* event follows current play */
  val TIMER_OBJECT = 2	/* event follows a object */
  val TIMER_MONSTER = 3	/* event follows a monster */

  /* save/restore timer ranges */
  val RANGE_LEVEL = 0		/* save/restore timers staying on level */
  val RANGE_GLOBAL = 1		/* save/restore timers following global play */

  /*
   * Timeout functions.  Add a define here, then put it in the table
   * in timeout.c.  "One more level of indirection will fix everything."
   */
  val ROT_ORGANIC = 0	/* for buried organics */
  val ROT_CORPSE = 1
  val REVIVE_MON = 2
  val BURN_OBJECT = 3
  val HATCH_EGG = 4
  val FIG_TRANSFORM = 5
  val NUM_TIME_FUNCS = 6

  /* used in timeout.c */
  class timer_element {
    var	next: timer_element = null		/* next item in chain */
    var timeout = 0		/* when we time out */
    var tid = 0		/* timer ID */
    var kind: Short = 0			/* kind of use */
    var func_index: Short = 0		/* what to call when we time out */
    var arg: genericptr_t = null		/* pointer to timeout argument */
    var needs_fixup = false	/* does arg need to be patched? */
  }
}
