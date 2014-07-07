/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define CONFIG_H


/*
 * Section 1:	Operating and window systems selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select BSD, or SYSV in unixconf.h.
 */

#define UNIX		/* delete if no fork(), exec() available */

#include "config1.h"	/* should auto-detect WIN32 */

#ifdef AUTOCONF
# include "autoconf_paths.h"
# include "autoconf.h"
#endif


/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */
/* #define TTY_GRAPHICS */	/* good old tty based graphics */
/* #define CURSES_GRAPHICS */	/* Proper curses interface */
/* #define X11_GRAPHICS */	/* X11 interface */
/* #define MSWIN_GRAPHICS */	/* Windows NT, CE, Graphics */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system (see defines above).
 */

#ifdef MSWIN_GRAPHICS
# ifdef TTY_GRAPHICS
# undef TTY_GRAPHICS
# endif
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "mswin"
# endif
# define HACKDIR "\\unnethack"
#endif

#ifdef CURSES_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "curses"
# endif
#endif

#ifndef DEFAULT_WINDOW_SYS
# define DEFAULT_WINDOW_SYS "tty"
#endif

#ifdef X11_GRAPHICS
/*
 * There are two ways that X11 tiles may be defined.  (1) using a custom
 * format loaded by NetHack code, or (2) using the XPM format loaded by
 * the free XPM library.  The second option allows you to then use other
 * programs to generate tiles files.  For example, the PBMPlus tools
 * would allow:
 *  xpmtoppm <x11tiles.xpm | pnmscale 1.25 | ppmquant 90 >x11tiles_big.xpm
 */
/* # define USE_XPM */		/* Disable if you do not have the XPM library */
# ifdef USE_XPM
#  define GRAPHIC_TOMBSTONE	/* Use graphical tombstone (rip.xpm) */
# endif
#endif


/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, NEWS or PANICLOG removes that
 *		feature from the game; otherwise set the appropriate wizard
 *		name.  LOGFILE, NEWS and PANICLOG refer to files in the
 *		playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
#  define WIZARD  "wizard" /* the person allowed to use the -D option */
#endif

#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define LOGAREA FILE_AREA_VAR
/* #define XLOGFILE "xlogfile" */ /* even larger logfile */
#define NEWS "news"		/* the file containing the latest hack news */
#define NEWS_AREA FILE_AREA_SHARE
#define PANICLOG "paniclog"	/* log of panic and impossible events */
/* #define LIVELOGFILE "livelog" */ /* live game progress log file */

/* #define LIVELOG_SHOUT */

/*
 *	If COMPRESS is defined, it should contain the full path name of your
 *	'compress' program.  Defining INTERNAL_COMP causes NetHack to do
 *	simpler byte-stream compression internally.  Both COMPRESS and
 *	INTERNAL_COMP create smaller bones/level/save files, but require
 *	additional code and time.  Currently, only UNIX fully implements
 *	COMPRESS; other ports should be able to uncompress save files a
 *	la unixmain.c if so inclined.
 *	If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *	as the extension your compressor appends to filenames after
 *	compression.
 */

#ifndef AUTOCONF
#ifdef UNIX
/* path and file name extension for compression program */
/* #define COMPRESS "/usr/bin/compress" */	/* Lempel-Ziv compression */
/* #define COMPRESS_EXTENSION ".Z"	*/	/* compress's extension */
/* An example of one alternative you might want to use: */
#define COMPRESS "/bin/gzip"	/* FSF gzip compression */
#define COMPRESS_EXTENSION ".gz"		/* normal gzip extension */
#endif

#ifndef COMPRESS
# define INTERNAL_COMP	/* control use of NetHack's compression routines */
#endif
#endif

/*
 *	Data librarian.  Defining DLB places most of the support files into
 *	a tar-like file, thus making a neater installation.  See *conf.h
 *	for detailed configuration.
 */
/* #define DLB */	/* not supported on all platforms */

# define CHDIR		/* delete if no chdir() available */

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
# ifndef HACKDIR
#  define HACKDIR "/usr/games/lib/unnethackdir"
# endif

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.	In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE */	/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS 6
 */
#endif /* CHDIR */



/*
 * Section 3:	Definitions that may vary with system type.
 *		For example, both schar and uchar should be short ints on
 *		the AT&T 3B2/3B5/etc. family.
 */

#include "tradstdc.h"

/*
 * type schar: small signed integers (8 bits suffice)
 *
 *	typedef char	schar;
 *
 *	will do when you have signed characters; otherwise use
 *
 *	typedef short int schar;
 */
typedef signed char	schar;

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
typedef unsigned char	uchar;

/* Type used for outputting DECgraphics and IBMgraphics characters into
 * HTML dumps or for holding unicode codepoints. */
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#ifdef UINT32_MAX
typedef uint32_t glyph_t;
#else
/* Fallback that should work on most systems */
typedef long glyph_t;
#endif

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */

/* #define STRNCMPI */	/* compiler/library has the strncmpi function */

#define EXOTIC_PETS      /* Rob Ellwood  June 2002 */

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

#ifndef AUTOCONF

/* I/O */
#define EXP_ON_BOTL	/* Show experience on bottom line */
/* #define SCORE_ON_BOTL */	/* added by Gary Erickson (erickson@ucivax) */
#endif /* AUTOCONF */

# define DOAGAIN '\001' /* ^A, the "redo" key used in cmd.c and getline.c */

/* #define REALTIME_ON_BOTL */  /* Show elapsed time on bottom line.  Note:
                                 * this breaks savefile compatibility. */

/* The options in this section require the extended logfile support */
#ifdef XLOGFILE
#define RECORD_CONDUCT  /* Record conducts kept in logfile */
#define RECORD_TURNS    /* Record turns elapsed in logfile */
#define RECORD_ACHIEVE  /* Record certain notable achievements in the
                         * logfile.  Note: this breaks savefile compatibility
                         * due to the addition of the u_achieve struct. */
#define RECORD_REALTIME /* Record the amount of actual playing time (in
                         * seconds) in the record file.  Note: this breaks
                         * savefile compatibility. */
#define RECORD_START_END_TIME /* Record to-the-second starting and ending
                               * times; stored as 32-bit values obtained
                               * from time(2) (seconds since the Epoch.) */
#define RECORD_GENDER0   /* Record initial gender in logfile */
#define RECORD_ALIGN0   /* Record initial alignment in logfile */
#endif

/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

#ifndef AUTOCONF

#define RANDOMIZED_PLANES /* Elemental Planes order is randomized - Patric Mueller (4 Jan 2009) */
#define BLACKMARKET	/* Massimo Campostrini (campo@sunthpi3.difi.unipi.it) */


#if defined(TTY_GRAPHICS) || defined(MSWIN_GRAPHICS) || \
 defined(CURSES_GRAPHICS)
# define MENU_COLOR
/*# define MENU_COLOR_REGEX*/
/*# define MENU_COLOR_REGEX_POSIX */
/* if MENU_COLOR_REGEX is defined, use regular expressions (regex.h,
 * GNU specific functions by default, POSIX functions with
 * MENU_COLOR_REGEX_POSIX).
 * otherwise use pmatch() to match menu color lines.
 * pmatch() provides basic globbing: '*' and '?' wildcards.
 */
#endif

/*#define GOLDOBJ */	/* Gold is kept on obj chains - Helge Hafting */
/*#define AUTOPICKUP_EXCEPTIONS */ /* exceptions to autopickup */
#define DUMP_LOG        /* Dump game end information to a file */
/* #define DUMP_FN "/tmp/%n.nh" */      /* Fixed dumpfile name, if you want
                                         * to prevent definition by users */
#define DUMP_TEXT_LOG   /* Dump game end information in a plain text form */
/*#define DUMP_HTML_LOG*/   /* Dump game end information to a html file */
#define DUMPMSGS 30     /* Number of latest messages in the dump file  */

#define AUTO_OPEN	/* open doors by walking into them - Stefano Busti */

/* #define WHEREIS_FILE "./whereis/%n.whereis" */ /* Write out player's current location to player.whereis */

#endif /* AUTOCONF */

#ifdef TTY_GRAPHICS
# define WIN_EDGE	/* windows aligned left&top */
# define VIDEOSHADES    /* Slash'Em like colors */
#endif

#define ADVENT_CALENDAR

/* End of Section 5 */

/*
 * Section 6:  UNCODITIONAL DEFINES
 *
 * These defines must be defined.
 * They come from patches that have been unconditionally incorporated
 * into UnNetHack.
 */
#define WEBB_DISINT /* Disintegrator - Nicholas Webb */
#define ASTRAL_ESCAPE /* Allow escape from Astral plane (with the Amulet) - CWC */
/* #define LIVELOG_BONES_KILLER */ /* Report if a ghost of a former player is
                                    * killed - Patric Mueller (15 Aug 2009) */
#define ADJSPLIT /* splittable #adjust - Sam Dennis, conditionalized by Jukka Lahtinen */
#define TUTORIAL_MODE /* Alex Smith */
#define ELBERETH_CONDUCT /* Track the number of times the player engraves Elbereth. - Ray Kulhanek */
#define SHOW_WEIGHT
/* End of Section 6 */

#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H */
