/* Copyright (c) Steve Linhart, Eric Raymond, 1989. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import C2Scala._
import Global._

object Color {

/*
 * The color scheme used is tailored for an IBM PC.  It consists of the
 * standard 8 colors, folowed by their bright counterparts.  There are
 * exceptions, these are listed below.	Bright black doesn't mean very
 * much, so it is used as the "default" foreground color of the screen.
 */

val CLR_BLACK = 8 // MONOTE: WIN32CON has this defined as 0
val CLR_RED = 1
val CLR_GREEN = 2
val CLR_BROWN = 3 /* on IBM, low-intensity yellow is brown */
val CLR_BLUE = 4
val CLR_MAGENTA = 5
val CLR_CYAN = 6
val CLR_GRAY = 7 /* low-intensity white */
val NO_COLOR = 0 // MONOTE: WIN32CON has this defined as 8
val CLR_ORANGE = 9
val CLR_BRIGHT_GREEN = 10
val CLR_YELLOW = 11
val CLR_BRIGHT_BLUE = 12
val CLR_BRIGHT_MAGENTA = 13
val CLR_BRIGHT_CYAN = 14
val CLR_WHITE = 15
val CLR_MAX = 16
val CLR_UNDEFINED = CLR_MAX

/* The "half-way" point for tty based color systems.  This is used in */
/* the tty color setup code.  (IMHO, it should be removed - dean).    */
val BRIGHT = 8

/* these can be configured */
val HI_OBJ = CLR_MAGENTA
val HI_METAL = CLR_CYAN
val HI_COPPER = CLR_YELLOW
val HI_SILVER = CLR_GRAY
val HI_MITHRIL = HI_SILVER
val HI_GOLD = CLR_YELLOW
val HI_LEATHER = CLR_BROWN
val HI_CLOTH = CLR_BROWN
val HI_ORGANIC = CLR_BROWN
val HI_WOOD = CLR_BROWN
val HI_PAPER = CLR_WHITE
val HI_GLASS = CLR_BRIGHT_CYAN
val HI_MINERAL = CLR_GRAY
val DRAGON_SILVER = CLR_BRIGHT_CYAN
val HI_ZAP = CLR_BRIGHT_BLUE

class menucoloring {
    var `match`: regex_t = null
    var color, attr = 0
    var next: menucoloring = null
}

class color_option {
    var color = 0
    var attr_bits = 0
}

val STATCLR_TYPE_PERCENT = 0
val STATCLR_TYPE_NUMBER_LT = 1
val STATCLR_TYPE_NUMBER_GT = 2
val STATCLR_TYPE_NUMBER_EQ = 3

class percent_color_option {
	var statclrtype: xchar = _
	var percentage = 0
	var color_option = new color_option()
	var next: percent_color_option = null
}

class text_color_option {
	var text: String = null
	var color_option = new color_option()
	var next: text_color_option = null
}
}
