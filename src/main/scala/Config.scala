/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Config {

/*
 * Section 1:	Window systems selection.
 */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system.
 */

val DEFAULT_WINDOW_SYS = "curses"

/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, NEWS or PANICLOG removes that
 *		feature from the game; otherwise set the appropriate wizard
 *		name.  LOGFILE, NEWS and PANICLOG refer to files in the
 *		playground.
 */

val WIZARD = "wizard" /* the person allowed to use the -D option */

val LOGFILE = "logfile"	/* larger file for debugging purposes */
val LOGAREA = FILE_AREA_VAR
/* #define XLOGFILE "xlogfile" */ /* even larger logfile */
val NEWS = "news"		/* the file containing the latest hack news */
val NEWS_AREA = FILE_AREA_SHARE
val PANICLOG = "paniclog"	/* log of panic and impossible events */
/* #define LIVELOGFILE "livelog" */ /* live game progress log file */

/* #define LIVELOG_SHOUT */

/*
 *	Data librarian.  Defining DLB places most of the support files into
 *	a tar-like file, thus making a neater installation.  See *conf.h
 *	for detailed configuration.
 */
/* #define DLB */	/* not supported on all platforms */

/*
 * Section 3:	Definitions that may vary with system type.
 */

/*
 * type schar: small signed integers (8 bits suffice)
 */
type schar = Byte

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 */
type uchar = Short // MONOTE No unsigned type on the JVM

/* Type used for outputting DECgraphics and IBMgraphics characters into
 * HTML dumps or for holding unicode codepoints. */
type glypth_t = Int

/* #define STRNCMPI */	/* compiler/library has the strncmpi function */

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

/* I/O */
/* #define SCORE_ON_BOTL */	/* added by Gary Erickson (erickson@ucivax) */

val DOAGAIN = '\001' /* ^A, the "redo" key used in cmd.c and getline.c */

/* #define REALTIME_ON_BOTL */  /* Show elapsed time on bottom line.  Note:
                                 * this breaks savefile compatibility. */

/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

/*# define MENU_COLOR_REGEX*/
/*# define MENU_COLOR_REGEX_POSIX */
/* if MENU_COLOR_REGEX is defined, use regular expressions (regex.h,
 * GNU specific functions by default, POSIX functions with
 * MENU_COLOR_REGEX_POSIX).
 * otherwise use pmatch() to match menu color lines.
 * pmatch() provides basic globbing: '*' and '?' wildcards.
 */

/***
MOTODO: Do we want this kind of logging?
#define DUMP_LOG        /* Dump game end information to a file */
/* #define DUMP_FN "/tmp/%n.nh" */      /* Fixed dumpfile name, if you want
                                         * to prevent definition by users */
#define DUMP_TEXT_LOG   /* Dump game end information in a plain text form */
/*#define DUMP_HTML_LOG*/   /* Dump game end information to a html file */
***/
val DUMPMSGS = 30     /* Number of latest messages in the dump file  */

/* #define WHEREIS_FILE "./whereis/%n.whereis" */ /* Write out player's current location to player.whereis */

/* End of Section 5 */

/*
 * Section 6:  UNCODITIONAL DEFINES
 *
 * These defines must be defined.
 * They come from patches that have been unconditionally incorporated
 * into UnNetHack.
 */
/* #define LIVELOG_BONES_KILLER */ /* Report if a ghost of a former player is
                                    * killed - Patric Mueller (15 Aug 2009) */
/* End of Section 6 */
}
