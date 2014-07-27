/* Copyright (c) David Cohrs, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object WinProcs {
class window_procs {
	var name: String = null
	var wincap = 0L		/* window port capability options supported */
	var wincap2 = 0L	/* additional window port capability options supported */
	// MOTODO: Pick types for following:
	// def win_init_nhwindows: (Int *, char **) => Unit
    def win_player_selection: () => Unit
    def win_askname: () => Unit
    def win_get_nh_event: () => Unit
    def win_exit_nhwindows: (String) => Unit
    def win_suspend_nhwindows: (String) => Unit
    def win_resume_nhwindows: () => Unit
    def win_create_nhwindow: (Int) => winid
    def win_clear_nhwindow: (winid) => Unit
    def win_display_nhwindow: (winid, Boolean) => Unit
    def win_destroy_nhwindow: (winid) => Unit
    def win_curs: (winid,Int,Int) => Unit
    def win_putstr: (winid, Int, String) => Unit
    def win_display_file: (String, String, Boolean) => Unit
    def win_start_menu: (winid) => Unit
    def win_add_menu: (winid,Int,Int,Object,
		Char,Char,Int,String, Boolean) => Unit
    def win_end_menu: (winid, String) => Unit
	// MOTODO: Pick types for following:
	// def win_select_menu: (winid, Int, MENU_ITEM_P **) => Int
    def win_message_menu: (Char,Int,String) => Char
    def win_update_inventory: () => Unit
    def win_mark_synch: () => Unit
    def win_wait_synch: () => Unit
    def win_cliparound: (Int, Int) => Unit
    def win_print_glyph: (winid,xchar,xchar,Int) => Unit
    def win_raw_print: (String) => Unit
    def win_raw_print_bold: (String) => Unit
    def win_nhgetch: () => Int
	// MOTODO: Pick types for following:
	// def win_nh_poskey: (Int *, Int *, Int *) => Int
    def win_nhbell: () => Unit
    def win_doprev_message: () => Int
    def win_yn_function: (String, String, Char) => Char
    def win_getlin: (String, StringBuilder) => Unit
    def win_get_ext_cmd: () => Int
    def win_number_pad: (Int) => Unit
    def win_delay_output: () => Unit

    /* other defs that really should go away (they're tty specific) */
    def win_start_screen: () => Unit
    def win_end_screen: () => Unit

    def win_outrip: (winid,Int) => Unit
    def win_preference_update: (String) => Unit
};

/*
 * If you wish to only support one window system and not use procedure
 * pointers, add the appropriate #ifdef below.
 */

def init_nhwindows = windowprocs.win_init_nhwindows
def player_selection = windowprocs.win_player_selection
def askname = windowprocs.win_askname
def get_nh_event = windowprocs.win_get_nh_event
def exit_nhwindows = windowprocs.win_exit_nhwindows
def suspend_nhwindows = windowprocs.win_suspend_nhwindows
def resume_nhwindows = windowprocs.win_resume_nhwindows
def create_nhwindow = windowprocs.win_create_nhwindow
def clear_nhwindow = windowprocs.win_clear_nhwindow
def display_nhwindow = windowprocs.win_display_nhwindow
def destroy_nhwindow = windowprocs.win_destroy_nhwindow
def curs = windowprocs.win_curs
def putstr = windowprocs.win_putstr
def display_file = windowprocs.win_display_file
def display_file_area(area: String,file: String,complain: Boolean) = display_file(area,file,complain)
def start_menu = windowprocs.win_start_menu
def add_menu = windowprocs.win_add_menu
def end_menu = windowprocs.win_end_menu
def select_menu = windowprocs.win_select_menu
def message_menu = windowprocs.win_message_menu
def update_inventory = windowprocs.win_update_inventory
def mark_synch = windowprocs.win_mark_synch
def wait_synch = windowprocs.win_wait_synch
def cliparound = windowprocs.win_cliparound
def print_glyph = windowprocs.win_print_glyph
def raw_print = windowprocs.win_raw_print
def raw_print_bold = windowprocs.win_raw_print_bold
def nhgetch = windowprocs.win_nhgetch
def nh_poskey = windowprocs.win_nh_poskey
def nhbell = windowprocs.win_nhbell
def nh_doprev_message = windowprocs.win_doprev_message
def getlin = windowprocs.win_getlin
def get_ext_cmd = windowprocs.win_get_ext_cmd
def number_pad = windowprocs.win_number_pad
def delay_output = windowprocs.win_delay_output

/* 3.4.2: There is a real yn_function() in the core now, which does
 *        some buffer length validation on the parameters prior to
 *        invoking the window port routine. yn_function() is in cmd.c
 */
/* def yn_function = windowprocs.win_yn_function) */

/* other defs that really should go away (they're tty specific) */
def start_screen = windowprocs.win_start_screen
def end_screen = windowprocs.win_end_screen

def outrip = windowprocs.win_outrip
def preference_update = windowprocs.win_preference_update

/*
 * WINCAP
 * Window port preference capability bits.
 * Some day this might be better in its own wincap.h file.
 */
val WC_COLOR = 0x01L		/* 01 Port can display things in color       */
val WC_HILITE_PET = 0x02L		/* 02 supports hilite pet                    */
val WC_ASCII_MAP = 0x04L		/* 03 supports an ascii map                  */
val WC_TILED_MAP = 0x08L		/* 04 supports a tiled map                   */
val WC_TILE_WIDTH = 0x20L		/* 06 prefer this width of tile              */
val WC_TILE_HEIGHT = 0x40L		/* 07 prefer this height of tile             */
val WC_TILE_FILE = 0x80L		/* 08 alternative tile file name             */
val WC_INVERSE = 0x100L		/* 09 Port supports inverse video            */
val WC_ALIGN_MESSAGE = 0x200L		/* 10 supports message alignmt top|b|l|r     */
val WC_ALIGN_STATUS = 0x400L		/* 11 supports status alignmt top|b|l|r      */
val WC_VARY_MSGCOUNT = 0x800L		/* 12 supports varying message window        */
val WC_FONT_MAP = 0x1000L	/* 13 supports specification of map win font */
val WC_FONT_MESSAGE = 0x2000L	/* 14 supports specification of msg win font */
val WC_FONT_STATUS = 0x4000L	/* 15 supports specification of sts win font */
val WC_FONT_MENU = 0x8000L	/* 16 supports specification of mnu win font */
val WC_FONT_TEXT = 0x10000L	/* 17 supports specification of txt win font */
val WC_FONTSIZ_MAP = 0x20000L	/* 18 supports specification of map win font */
val WC_FONTSIZ_MESSAGE = 0x40000L	/* 19 supports specification of msg win font */
val WC_FONTSIZ_STATUS = 0x80000L	/* 20 supports specification of sts win font */
val WC_FONTSIZ_MENU = 0x100000L	/* 21 supports specification of mnu win font */
val WC_FONTSIZ_TEXT = 0x200000L	/* 22 supports specification of txt win font */
val WC_SCROLL_MARGIN = 0x400000L	/* 23 supports setting scroll margin for map */
val WC_SPLASH_SCREEN = 0x800000L	/* 24 supports display of splash screen      */
val WC_POPUP_DIALOG = 0x1000000L	/* 25 supports queries in pop dialogs        */
val WC_SCROLL_AMOUNT = 0x2000000L	/* 26 scroll this amount at scroll margin    */
val WC_EIGHT_BIT_IN = 0x4000000L	/* 27 8-bit character input                  */
val WC_PERM_INVENT = 0x8000000L	/* 28 8-bit character input                  */
val WC_MAP_MODE = 0x10000000L	/* 29 map_mode option                        */
val WC_WINDOWCOLORS = 0x20000000L	/* 30 background color for message window    */
val WC_PLAYER_SELECTION = 0x40000000L /* 31 background color for message window    */
val WC_MOUSE_SUPPORT = 0x80000000L	/* 32 mouse support                          */
					/* no free bits */

val WC2_FULLSCREEN = 0x01L	/* 01 display full screen                    */
val WC2_SOFTKEYBOARD = 0x02L	/* 02 software keyboard                      */
val WC2_WRAPTEXT = 0x04L	/* 03 wrap long lines of text                */
val WC2_TERM_COLS = 0x08L	/* 04 supports setting terminal width        */
val WC2_TERM_ROWS = 0x10L	/* 05 supports setting terminal height       */
val WC2_WINDOWBORDERS = 0x20L	/* 06 display borders for NetHack windows    */
val WC2_PETATTR = 0x40L	/* 07 attributes for highlight_pet */
val WC2_GUICOLOR = 0x80L	/* 08 attributes for highlight_pet */
val WC2_NEWCOLORS = 0x100L	/* 09 try to use slashem colors              */
					/* 24 free bits */

val ALIGN_LEFT = 1
val ALIGN_RIGHT = 2
val ALIGN_TOP = 3
val ALIGN_BOTTOM = 4

/* player_selection */
val VIA_DIALOG = 0
val VIA_PROMPTS = 1

/* map_mode settings - deprecated */
val MAP_MODE_TILES = 0
val MAP_MODE_ASCII4x6 = 1
val MAP_MODE_ASCII6x8 = 2
val MAP_MODE_ASCII8x8 = 3
val MAP_MODE_ASCII16x8 = 4
val MAP_MODE_ASCII7x12 = 5
val MAP_MODE_ASCII8x12 = 6
val MAP_MODE_ASCII16x12 = 7
val MAP_MODE_ASCII12x16 = 8
val MAP_MODE_ASCII10x18 = 9
val MAP_MODE_ASCII_FIT_TO_SCREEN = 10
val MAP_MODE_TILES_FIT_TO_SCREEN = 11

class wc_Opt(val wc_name: String, val wc_bit: Long) {
}
}
