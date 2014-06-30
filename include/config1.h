/*	SCCS Id: @(#)config1.h	3.4	1999/12/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

/*
 * MS DOS - compilers
 *
 * Microsoft C auto-defines MSDOS,
 * Borland C   auto-defines __MSDOS__,
 * DJGPP       auto-defines MSDOS.
 */

/* #define MSDOS */	/* use if not defined by compiler or cases below */

#ifdef __MSDOS__	/* for Borland C */
# ifndef MSDOS
# define MSDOS
# endif
#endif

#ifdef __TURBOC__
# define __MSC		/* increase Borland C compatibility in libraries */
#endif

#ifdef MSDOS
# undef UNIX
#endif

/*
 * Mac Stuff.
 */
#ifdef macintosh	/*	Auto-defined symbol for MPW compilers (sc and mrc) */
# define MAC
#endif

#ifdef THINK_C		/* Think C auto-defined symbol */
# define MAC
# define NEED_VARARGS
#endif

#ifdef __MWERKS__	/* defined by Metrowerks' Codewarrior compiler */
# ifndef __BEOS__	/* BeOS */
#  define MAC
# endif
# define NEED_VARARGS
# define USE_STDARG
#endif

#if defined(MAC) || defined(__BEOS__)
# define DLB
# undef UNIX
#endif

#ifdef __BEOS__
# define NEED_VARARGS
#endif


/*
 * Amiga setup.
 */
#ifdef AZTEC_C	/* Manx auto-defines this */
# ifdef MCH_AMIGA	/* Manx auto-defines this for AMIGA */
#  ifndef AMIGA
#define AMIGA		/* define for Commodore-Amiga */
#  endif		/* (SAS/C auto-defines AMIGA) */
#define AZTEC_50	/* define for version 5.0 of manx */
# endif
#endif
#ifdef __SASC_60
# define NEARDATA __near /* put some data close */
#else
# ifdef _DCC
# define NEARDATA __near /* put some data close */
# else
# define NEARDATA
# endif
#endif
#ifdef AMIGA
# define NEED_VARARGS
# undef UNIX
# define DLB
# define HACKDIR "NetHack:"
# define NO_MACRO_CPATH
#endif

/*
 * Windows NT Autodetection
 */
#ifdef _WIN32_WCE
#define WIN_CE
# ifndef WIN32
# define WIN32
# endif
#endif

#ifdef WIN32
# undef UNIX
# undef MSDOS
# define NHSTDC
# define USE_STDARG
# define NEED_VARARGS

# define STRNCMPI
# define STRCMPI

#endif


#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
# define _GNU_SOURCE
#endif

#endif	/* CONFIG1_H */
