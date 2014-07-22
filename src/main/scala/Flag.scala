/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Config._
import Coord._
import Decl._
import ObjClass._

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does	   */
/* not require incrementing EDITLEVEL.					   */
/*
 * Persistent flags that are saved and restored with the game.
 *
 */
object Flag {

class flag {
	var autodig = false     /* MRKR: Automatically dig */
	var autoquiver = false	/* Automatically fill quiver */
	var autounlock = false	/* Automatically apply unlocking tool */
	var beginner = false
	var biff = false		/* enable checking for mail */
	var hint = false		/* enable Unclippy the helpful hint daemon */
	var botl = false		/* partially redo status line */
	var botlx = false		/* print an entirely new bottom line */
	var confirm = false	/* confirm before hitting tame monsters */
	var deathdropless = false	/* disable monsters dropping random items */
	var debug = false		/* in debugging mode */
def wizard = flags.debug
	var elberethignore = false /* Elbereth ignores requests for protection */
	var end_own = false	/* list all own scores */
	var explore = false	/* in exploration mode */
	var tutorial = false      /* in tutorial mode */
def discover = flags.explore
	var female = false
	var forcefight = false
	var friday13 = false	/* it's Friday the 13th */
	var help = false		/* look in data file for info about stuff */
	var hitpointbar = false	/* colourful hit point status bar */
	var ignintr = false	/* ignore interrupts */
	var ins_chkpt = false	/* checkpoint as appropriate */
	var invlet_constant = false /* let objects keep their inventory symbol */
	var legacy = false	/* print game entry "story" */
	var lit_corridor = false	/* show a dark corr as lit if it is in sight */
	var made_amulet = false
	var mon_moving = false	/* monsters' turn to move */
	var move = false
	var mv = false
	var bypasses = false	/* bypass flag is set on at least one fobj */
	var nap = false		/* `timed_delay' option for display effects */
	var nopick = false	/* do not pickup objects (as when running) */
	var `null` = false		/* OK to send nulls to the terminal */
	var perm_invent = false	/* keep full inventories up until dismissed */
	var pickup = false	/* whether you pickup or move and look */
	var pickup_dropped = false	/* don't auto-pickup items you dropped */
	var pickup_thrown = false		/* auto-pickup items you threw */

	var pushweapon = false	/* When wielding, push old weapon into second slot */
	var safe_dog = false	/* give complete protection to the dog */
	var showexp = false	/* show experience points */
	var invweight = false
	var showweight = false
	var showscore = false	/* show score */
	var silent = false	/* whether the bell rings or not */
	var sortpack = false	/* sorted inventory */
	var soundok = false	/* ok to tell about sounds heard */
	var sparkle = false	/* show "resisting" special FX (Scott Bigham) */
	var standout = false	/* use standout for --More-- */
	var time = false		/* display elapsed 'time' */
	var tombstone = false	/* print tombstone */
	var toptenwin = false	/* ending list in window instead of stdout */
	var verbose = false	/* max battle info */
	var wounds = false	/* healers can see how badly monsters are damaged */
	var prayconfirm = false	/* confirm before praying */
	var	end_top, end_around = 0	/* describe desired score list */
	var ident = 0 		/* social security number for each monster */
	var moonphase = 0
	var suppress_alert = 0
val NEW_MOON = 0
val FULL_MOON = 4
	var no_of_wizards = 0 /* 0, 1 or 2 (wizard and his shadow) */
	var travel = false	/* find way automatically to u.tx,u.ty */
	var run = 0		/* 0: h (etc), 1: H (etc), 2: fh (etc) */
				/* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
				/* 8: travel */
	var warntype = 0 /* warn_of_mon monster type M2 */
	var warnlevel = 0
	var djinni_count, ghost_count = 0	/* potion effect tuning */
	var pickup_burden = 0		/* maximum burden before prompt */
	var inv_order = new Array[Char](MAXOCLASSES)
	var pickup_types = new Array[Char](MAXOCLASSES)
val NUM_DISCLOSURE_OPTIONS = 5
val DISCLOSE_PROMPT_DEFAULT_YES = 'y'
val DISCLOSE_PROMPT_DEFAULT_NO = 'n'
val DISCLOSE_YES_WITHOUT_PROMPT = '+'
val DISCLOSE_NO_WITHOUT_PROMPT = '-'
	var end_disclose = new Array[Char](NUM_DISCLOSURE_OPTIONS + 1)  /* disclose various info
								upon exit */
	var menu_style: Char = 0	/* User interface style setting */
	var bones = false		/* allow loading bones */
	var perma_hallu = false		/* let the player permanently hallucinate */

	/* KMH, role patch -- Variables used during startup.
	 *
	 * If the user wishes to select a role, race, gender, and/or alignment
	 * during startup, the choices should be recorded here.  This
	 * might be specified through command-line options, environmental
	 * variables, a popup dialog box, menus, etc.
	 *
	 * These values are each an index into an array.  They are not
	 * characters or letters, because that limits us to 26 roles.
	 * They are not booleans, because someday someone may need a neuter
	 * gender.  Negative values are used to indicate that the user
	 * hasn't yet specified that particular value.	If you determine
	 * that the user wants a random choice, then you should set an
	 * appropriate random value; if you just left the negative value,
	 * the user would be asked again!
	 *
	 * These variables are stored here because the u structure is
	 * cleared during character initialization, and because the
	 * flags structure is restored for saved games.  Thus, we can
	 * use the same parameters to build the role entry for both
	 * new and restored games.
	 *
	 * These variables should not be referred to after the character
	 * is initialized or restored (specifically, after role_init()
	 * is called).
	 */
	var initrole = 0	/* starting role      (index into roles[])   */
	var initrace = 0	/* starting race      (index into races[])   */
	var initgend = 0	/* starting gender    (index into genders[]) */
	var initalign = 0	/* starting alignment (index into aligns[])  */
	var randomall = 0	/* randomly assign everything not specified */
	var pantheon = 0	/* deity selection for priest character */

	/* --- initial roleplay flags ---
	 * These flags represent the player's conduct/roleplay
	 * intention at character creation.
	 *
	 * First the player can sets some of these at character
	 * creation. (via configuration-file, ..)
	 * Then role_init() may set/prevent certain combinations,
	 * e.g. Monks get the vegetarian flag, vegans should also be
	 * vegetarians, ..
	 * 
	 * After that the initial flags shouldn't be modified.
	 * In u_init() the flags can be used to put some
	 * roleplay-intrinsics into the u structure. Only those
	 * should be modified during gameplay.
	 */
	var ascet = false
	var atheist = false
	var blindfolded = false
	var illiterate = false
	var pacifist = false
	var nudist = false
	var vegan = false
	var vegetarian = false

	/* Heaven or hell modes */
	var heaven_or_hell = false /* player and monsters have max 1 HP and player has 3 lives */
	var hell_and_hell = false /* like heaven_or_hell but only player has max 1 HP */
}

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

class instance_flags {
	var cbreak = false	/* in cbreak mode, rogue format */
	var cursesgraphics = false /* Use portable curses extended characters */
	var DECgraphics = false	/* use DEC VT-xxx extended character set */
	var echo = false		/* 1 to echo characters */
	var IBMgraphics = false	/* use IBM extended character set */
	var UTF8graphics = false	/* use UTF-8 characters */
	var msg_history = 0	/* hint: # of top lines to save */
	var num_pad = false	/* use numbers for movement commands */
	var news = false		/* print news */
	var window_inited = false /* true if init_nhwindows() completed */
	var vision_inited = false /* true if vision is ready */
	var menu_tab_sep = false	/* Use tabs to separate option menu fields */
	var menu_requested = false /* Flag for overloaded use of 'm' prefix
				  * on some non-move commands */
	var num_pad_mode: uchar = _
	var menu_headings = 0	/* ATR for menu headings */
	var purge_monsters = 0	/* # of dead monsters still on fmon list */
	var opt_booldup: Array[Int] = null	/* for duplication of boolean opts in config file */
	var opt_compdup: Array[Int] = null	/* for duplication of compound opts in config file */
	var bouldersym: uchar = _	/* symbol for boulder display */
	var autoexplore = false	/* in autoexplore */
	var travel1 = false	/* first travel step */
	val travelcc = new coord()	/* coordinates for travel_cache */
	var simplemail = false	/* simple mail format $NAME:$MESSAGE */
	var sanity_check = false	/* run sanity checks */
	var mon_polycontrol = false	/* debug: control monster polymorphs */
	var prevmsg_window: Char = 0	/* type of old message window to use */
	var extmenu = false	/* extended commands use menu interface */
	var use_menu_color = false	/* use color in menus; only if wc_color */
	var win_edge = false	/* are the menus aligned left&top */
	var use_status_colors = false /* use color in status line; only if wc_color */
	var sortloot: Char = 0		/* sort items to loot alphabetically */
	var show_born = false	/* show numbers of created monsters */
	var showdmg = false	/* show damage */
	/* only set when PARANOID is defined */
	var paranoid_hit = false	/* Ask for 'yes' when hitting peacefuls */
	var paranoid_quit = false	/* Ask for 'yes' when quitting */
	var paranoid_remove = false /* Always show menu for 'T' and 'R' */
	var paranoid_trap = false /* Ask for 'yes' before walking into known traps */
	var paranoid_lava = false /* Ask for 'yes' before walking into lava */
	var paranoid_water = false /* Ask for 'yes' before walking into water */
	var quiver_fired = false	/* quiver with f command */
/*
 * Window capability support.
 */
	var wc_color = false		/* use color graphics                  */
	var wc_hilite_pet = false		/* hilight pets                        */
	var wc_ascii_map = false		/* show map using traditional ascii    */
	var wc_tiled_map = false		/* show map using tiles                */
	var wc_tile_width = 0		/* tile width                          */
	var wc_tile_height = 0		/* tile height                         */
	var wc_tile_file: String = null		/* name of tile file;overrides default */
	var wc_inverse = false		/* use inverse video for some things   */
	var wc_align_status = 0	/*  status win at top|bot|right|left   */
	var wc_align_message = 0	/* message win at top|bot|right|left   */
	var wc_vary_msgcount = 0	/* show more old messages at a time    */
	var wc_foregrnd_menu: String = null	/* points to foregrnd color name for menu win   */
	var wc_backgrnd_menu: String = null	/* points to backgrnd color name for menu win   */
	var wc_foregrnd_message: String = null	/* points to foregrnd color name for msg win    */
	var wc_backgrnd_message: String = null	/* points to backgrnd color name for msg win    */
	var wc_foregrnd_status: String = null	/* points to foregrnd color name for status win */
	var wc_backgrnd_status: String = null	/* points to backgrnd color name for status win */
	var wc_foregrnd_text: String = null	/* points to foregrnd color name for text win   */
	var wc_backgrnd_text: String = null	/* points to backgrnd color name for text win   */
	var wc_font_map: String = null		/* points to font name for the map win */
	var wc_font_message: String = null	/* points to font name for message win */
	var wc_font_status: String = null	/* points to font name for status win  */
	var wc_font_menu: String = null		/* points to font name for menu win    */
	var wc_font_text: String = null		/* points to font name for text win    */
	var wc_fontsiz_map = 0		/* font size for the map win           */
	var wc_fontsiz_message = 0	/* font size for the message window    */
	var wc_fontsiz_status = 0	/* font size for the status window     */
	var wc_fontsiz_menu = 0		/* font size for the menu window       */
	var wc_fontsiz_text = 0		/* font size for text windows          */
	var wc_scroll_amount = 0	/* scroll this amount at scroll_margin */
	var wc_scroll_margin = 0	/* scroll map when this far from
						the edge */
	var wc_map_mode = 0		/* specify map viewing options, mostly
						for backward compatibility */
	var wc_player_selection = 0	/* method of choosing character */
	var wc_splash_screen = false	/* display an opening splash screen or not */
	var wc_popup_dialog = false	/* put queries in pop up dialogs instead of
				   		in the message window */
	var wc_eight_bit_input = false	/* allow eight bit input               */
	var wc_mouse_support = false	/* allow mouse support */
	var wc2_fullscreen = false	/* run fullscreen */
	var wc2_softkeyboard = false	/* use software keyboard */
	var wc2_wraptext = false	/* wrap text */
	var wc2_term_cols = 0		/* terminal width, in characters */
	var wc2_term_rows = 0		/* terminal height, in characters */
	var wc2_windowborders = 0	/* display borders on NetHack windows */
	var wc2_petattr = 0		/* points to text attributes for pet */
	var wc2_guicolor = false	/* allow colors in GUI (outside map) */
	var wc2_newcolors = false	/* try to use slashem like colors including
					 * dark-gray to represent black object */

	var hp_notify = false
	var hp_notify_fmt: String = null
	var nameempty: String = null	/* what to automatically name known empty wands */
	var show_buc = false	/* always show BUC status */
	var cmdassist = false	/* provide detailed assistance for some commands */
	var obsolete = false	/* obsolete options can point at this, it isn't used */
	var rest_on_space = false /* space means rest */
	/* Items which belong in flags, but are here to allow save compatibility */
	var lootabc = false	/* use "a/b/c" rather than "o/i/b" when looting */
	var showrace = false	/* show hero glyph by race rather than by role */
	var travelcmd = false	/* allow travel command */
	var show_dgn_name = false /* show dungeon names instead of Dlvl: on bottom line */
	var runmode = 0	/* update screen display during run moves */
	var pilesize = 0	/* how many items to list automatically */
	var autopickup_exceptions = new Array[autopickup_exception](2)
val AP_LEAVE = 0
val AP_GRAB = 1
	var autoopen = false	/* open doors by walking into them */
	var dark_room = false	/* show shadows in lit rooms */
	var vanilla_ui_behavior = false	/* fall back to vanilla behavior */
	var show_annotation = false	/* level annotation when entering level */
	var statuslines = 0	/* number of status lines */
}

/*
 * Old deprecated names
 */
/*** MOTODO May not be needed, they're deprecated and the scope is a pain
def eight_bit_tty = wc_eight_bit_input
def use_color = wc_color
def hilite_pet = wc_hilite_pet
def use_inverse = wc_inverse
***/

/* runmode options */
val RUN_TPORT = 0	/* don't update display until movement stops */
val RUN_LEAP = 1	/* update display every 7 steps */
val RUN_STEP = 2	/* update display every single step */
val RUN_CRAWL = 3	/* walk w/ extra delay after each update */
}
