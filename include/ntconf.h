/* Copyright (c) NetHack PC Development Team 1993, 1994.  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NTCONF_H
#define NTCONF_H

/* #define SHELL */	/* nt use of pcsys routines caused a hang */

#define RANDOM		/* have Berkeley random(3) */

#define EXEPATH			/* Allow .exe location to be used as HACKDIR */
#define TRADITIONAL_GLYPHMAP	/* Store glyph mappings at level change time */
#ifdef WIN32CON
#define LAN_FEATURES		/* Include code for lan-aware features. Untested in 3.4.0*/
#endif

#define PC_LOCKING		/* Prevent overwrites of aborted or in-progress games */
				/* without first receiving confirmation. */

#define HOLD_LOCKFILE_OPEN	/* Keep an exclusive lock on the .0 file */

#define SELF_RECOVER		/* Allow the game itself to recover from an aborted game */

//#define USER_SOUNDS
/*
 * -----------------------------------------------------------------
 *  The remaining code shouldn't need modification.
 * -----------------------------------------------------------------
 */

#define NO_TERMS
#define ASCIIGRAPH

#ifdef OPTIONS_USED
#undef OPTIONS_USED
#endif
#ifdef MSWIN_GRAPHICS
#define OPTIONS_USED	"guioptions"
#else
#define OPTIONS_USED	"options"
#endif
#define OPTIONS_FILE OPTIONS_USED

#define PORT_HELP	"porthelp"

#ifdef WIN32CON
#define PORT_DEBUG	/* include ability to debug international keyboard issues */
#endif

/* Stuff to help the user with some common, yet significant errors */
#define INTERJECT_PANIC		0
#define INTERJECTION_TYPES	(INTERJECT_PANIC + 1)
extern void FDECL(interject_assistance, (int,int,genericptr_t,genericptr_t));
extern void FDECL(interject, (int));

/* The following is needed for prototypes of certain functions */
#if defined(_MSC_VER)
#include <process.h>	/* Provides prototypes of exit(), spawn()      */
#endif

#include <string.h>	/* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#define strncmpi(a,b,c) strnicmp(a,b,c)
#endif

#include <sys/types.h>
#include <stdlib.h>

#define PATHLEN		BUFSZ /* maximum pathlength */
#define FILENAME	BUFSZ /* maximum filename length (conservative) */

#if defined(_MAX_PATH) && defined(_MAX_FNAME)
# if (_MAX_PATH < BUFSZ) && (_MAX_FNAME < BUFSZ)
#undef PATHLEN
#undef FILENAME
#define PATHLEN		_MAX_PATH
#define FILENAME	_MAX_FNAME
# endif
#endif


#define NO_SIGNAL
#define index	strchr
#define rindex	strrchr
#include <time.h>
#define USE_STDARG
#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand()	random()
#else
#define Rand()	rand()
#endif

#define FCMASK	0660	/* file creation mask */
#define regularize	nt_regularize
#define HLOCK "NHPERM"

#ifndef M
#define M(c)		((char) (0x80 | (c)))
/* #define M(c)		((c) - 128) */
#endif

#ifndef C
#define C(c)		(0x1f & (c))
#endif

#if defined(DLB)
#define FILENAME_CMP  stricmp		      /* case insensitive */
#endif

/* this was part of the MICRO stuff in the past */
extern const char *alllevels, *allbones;
extern char hackdir[];
#define ABORT C('a')
#define getuid() 1
#define getlogin() ((char *)0)
extern void NDECL(win32_abort);
#ifdef WIN32CON
extern void FDECL(nttty_preference_update, (const char *));
extern void NDECL(toggle_mouse_support);
extern void FDECL(map_subkeyvalue, (char *));
extern void NDECL(load_keyboard_handler);
#endif

#include <fcntl.h>
#include <io.h>
#include <direct.h>
#ifndef CURSES_GRAPHICS
# include <conio.h>
#endif
#undef kbhit		/* Use our special NT kbhit */
#define kbhit (*nt_kbhit)

#ifdef LAN_FEATURES
#define MAX_LAN_USERNAME 20
#define LAN_RO_PLAYGROUND	/* not implemented in 3.3.0 */
#define LAN_SHARED_BONES	/* not implemented in 3.3.0 */
#include "nhlan.h"
#endif

#ifndef alloca
#define ALLOCA_HACK	/* used in util/panic.c */
#endif

#ifdef _MSC_VER
#pragma warning(disable:4761)	/* integral size mismatch in arg; conv supp*/
#ifdef YYPREFIX
#pragma warning(disable:4102)	/* unreferenced label */
#endif
#endif

extern int FDECL(set_win32_option, (const char *, const char *));
#ifdef WIN32CON
#define LEFTBUTTON  FROM_LEFT_1ST_BUTTON_PRESSED
#define RIGHTBUTTON RIGHTMOST_BUTTON_PRESSED
#define MIDBUTTON   FROM_LEFT_2ND_BUTTON_PRESSED
#define MOUSEMASK (LEFTBUTTON | RIGHTBUTTON | MIDBUTTON)
#endif /* WIN32CON */

#endif /* NTCONF_H */
