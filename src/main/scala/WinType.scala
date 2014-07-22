/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._

object WinType {

type winid = Int		/* a window identifier */

/* generic parameter - must not be any larger than a pointer */
/*** MONOTE: Was a union ***/
class anything {
    var a_void, a_obj: Object = null
    var a_int = 0
    var a_char: Char = 0
    var a_schar: schar = _
    /* add types as needed */
}
type ANY_P = anything /* avoid typedef in prototypes */
			/* (buggy old Ultrix compiler) */

/* menu return list */
class menu_item {
    var item: anything = null		/* identifier */
    var count = 0			/* count */
} 
type MENU_ITEM_P = menu_item

/* select_menu() "how" argument types */
val PICK_NONE = 0	/* user picks nothing (display only) */
val PICK_ONE = 1	/* only pick one */
val PICK_ANY = 2	/* can pick any amount */

/* window types */
/* any additional port specific types should be defined in win*.h */
val NHW_MESSAGE = 1
val NHW_STATUS = 2
val NHW_MAP = 3
val NHW_MENU = 4
val NHW_TEXT = 5

/* attribute types for putstr; the same as the ANSI value, for convenience */
val ATR_NONE = 0
val ATR_BOLD = 1
val ATR_DIM = 2
val ATR_ULINE = 4
val ATR_BLINK = 5
val ATR_INVERSE = 7
val ATR_UNDEFINED = 8

/* nh_poskey() modifier types */
val CLICK_1 = 1
val CLICK_2 = 2

/* invalid winid */
val WIN_ERR: winid = -1 

/* menu window keyboard commands (may be mapped) */
val MENU_FIRST_PAGE = '^'
val MENU_LAST_PAGE = '|'
val MENU_NEXT_PAGE = '>'
val MENU_PREVIOUS_PAGE = '<'
val MENU_SELECT_ALL = '.'
val MENU_UNSELECT_ALL = '-'
val MENU_INVERT_ALL = '@'
val MENU_SELECT_PAGE = ','
val MENU_UNSELECT_PAGE = '\\'
val MENU_INVERT_PAGE = '~'
val MENU_SEARCH = ':'
}
