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
# ifdef __SC__
# define msleep(k) (void) usleep((long)((k)*1000))
# endif
#endif

#endif /* PCCONF_H */
