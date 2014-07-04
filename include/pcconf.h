/*	SCCS Id: @(#)pcconf.h	3.4	1995/10/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PCCONF_H
#define PCCONF_H

#define MICRO		/* always define this! */

#define PATHLEN		64	/* maximum pathlength */
#define FILENAME	80	/* maximum filename length (conservative) */
#include "micro.h"		/* contains necessary externs for [os_name].c */


/* ===================================================
 *  The remaining code shouldn't need modification.
 */

#include "system.h"

#ifdef __DJGPP__
#include <unistd.h> /* close(), etc. */
/* lock() in io.h interferes with lock[] in decl.h */
#define lock djlock
#include <io.h>
#undef lock
#include <pc.h> /* kbhit() */
#define PC_LOCKING
#define HOLD_LOCKFILE_OPEN
#define SELF_RECOVER		/* NetHack itself can recover games */
#endif

/* Borland Stuff */
# if defined(__BORLANDC__)
#  if defined(__OVERLAY__) && !defined(VROOMM)
/* __OVERLAY__ is automatically defined by Borland C if overlay option is on */
#define VROOMM		/* Borland's VROOMM overlay system */
#  endif
#  if !defined(STKSIZ)
#define STKSIZ	5*1024	/* Use a default of 5K stack for Borland C	*/
			/* This macro is used in any file that contains */
			/* a main() function.				*/
#  endif
#define PC_LOCKING
# endif

#ifdef PC_LOCKING
#define HLOCK "NHPERM"
#endif

#ifndef index
# define index	strchr
#endif
#ifndef rindex
# define rindex strrchr
#endif

#include <time.h>

#ifdef RANDOM
/* Use the high quality random number routines. */
# define Rand() random()
#else
# define Rand() rand()
#endif

# define FCMASK 0660	/* file creation mask */

#include <fcntl.h>

#ifndef REDO
# undef Getchar
# define Getchar nhgetch
#endif


/* Sanity check, do not modify these blocks. */

/* OVERLAY must be defined with MOVERLAY or VROOMM */
#if (defined(MOVERLAY) || defined(VROOMM))
# ifndef OVERLAY
#  define OVERLAY
# endif
#endif

#if defined(FUNCTION_LEVEL_LINKING)
#define OVERLAY
#define OVL0
#define OVL1
#define OVL2
#define OVL3
#define OVLB
#endif

#if defined(OVERLAY) && !defined(MOVERLAY) && !defined(VROOMM) && !defined(FUNCTION_LEVEL_LINKING)
#define USE_TRAMPOLI
#endif

#ifdef MSC7_WARN	/* define with cl /DMSC7_WARN	*/
#pragma warning(disable:4131)
#endif

#ifdef TIMED_DELAY
# ifdef __DJGPP__
# define msleep(k) (void) usleep((k)*1000)
# endif
# ifdef __BORLANDC__
# define msleep(k) delay(k)
# endif
# ifdef __SC__
# define msleep(k) (void) usleep((long)((k)*1000))
# endif
#endif

#endif /* PCCONF_H */
