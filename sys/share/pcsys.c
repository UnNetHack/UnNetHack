/*	SCCS Id: @(#)pcsys.c	3.4	2002/01/22		  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  System related functions for MSDOS, OS/2, TOS, and Windows NT
 */

#define NEED_VARARGS
#include "hack.h"
#include "wintty.h"

#include <ctype.h>
#include <fcntl.h>
#include <process.h>


#if defined(MICRO) || defined(WIN32)
void FDECL(nethack_exit,(int));
#else
#define nethack_exit exit
#endif
static void NDECL(msexit);


#ifdef MOVERLAY
extern void __far __cdecl _movepause( void );
extern void __far __cdecl _moveresume( void );
extern unsigned short __far __cdecl _movefpause;
extern unsigned short __far __cdecl _movefpaused;
#define     __MOVE_PAUSE_DISK	  2   /* Represents the executable file */
#define     __MOVE_PAUSE_CACHE	  4   /* Represents the cache memory */
#endif /* MOVERLAY */

#ifdef WIN32CON
extern int GUILaunched;    /* from nttty.c */
#endif

#if defined(MICRO) || defined(WIN32)

void
flushout()
{
	(void) fflush(stdout);
	return;
}

static const char *COMSPEC =
"COMSPEC";

#define getcomspec() nh_getenv(COMSPEC)

# ifdef SHELL
int
dosh()
{
	extern char orgdir[];
	char *comspec;
	int spawnstat;
	if ((comspec = getcomspec())) {
		suspend_nhwindows("To return to NetHack, enter \"exit\" at the system prompt.\n");
#  ifndef NOCWD_ASSUMPTIONS
		chdirx(orgdir, 0);
#  endif
#   ifdef MOVERLAY
       /* Free the cache memory used by overlays, close .exe */
	_movefpause |= __MOVE_PAUSE_DISK;
	_movefpause |= __MOVE_PAUSE_CACHE;
	_movepause();
#   endif
		spawnstat = spawnl(P_WAIT, comspec, comspec, (char *)0);
#   ifdef MOVERLAY
		 _moveresume();
#   endif

		if ( spawnstat < 0) {
			raw_printf("Can't spawn \"%s\"!", comspec);
			getreturn("to continue");
		}
#  ifndef NOCWD_ASSUMPTIONS
		chdirx(hackdir, 0);
#  endif
		get_scr_size(); /* maybe the screen mode changed (TH) */
		resume_nhwindows();
	} else
		pline("Can't find %s.",COMSPEC);
	return 0;
}
# endif /* SHELL */


#endif /* MICRO */

/*
 * Add a backslash to any name not ending in /, \ or :	 There must
 * be room for the \
 */
void
append_slash(name)
char *name;
{
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
		*++ptr = '\\';
		*++ptr = '\0';
	}
	return;
}

#ifdef WIN32
boolean getreturn_enabled;
#endif

void
getreturn(str)
const char *str;
{
#ifdef WIN32
	if (!getreturn_enabled) return;
#endif
	msmsg("Hit <Enter> %s.", str);
	while (Getchar() != '\n') ;
	return;
}

#ifndef WIN32CON
void
msmsg VA_DECL(const char *, fmt)
	VA_START(fmt);
	VA_INIT(fmt, const char *);
	Vprintf(fmt, VA_ARGS);
	flushout();
	VA_END();
	return;
}
#endif

/*
 * Follow the PATH, trying to fopen the file.
 */
#define PATHSEP ';'

FILE *
fopenp(name, mode)
const char *name, *mode;
{
	char buf[BUFSIZ], *bp, *pp, lastch = 0;
	FILE *fp;

	/* Try the default directory first.  Then look along PATH.
	 */
	(void) strncpy(buf, name, BUFSIZ - 1);
	buf[BUFSIZ-1] = '\0';
	if ((fp = fopen(buf, mode)))
		return fp;
	else {
		int ccnt = 0;
		pp = getenv("PATH");
		while (pp && *pp) {
			bp = buf;
			while (*pp && *pp != PATHSEP) {
				lastch = *bp++ = *pp++;
				ccnt++;
			}
			if (lastch != '\\' && lastch != '/') {
				*bp++ = '\\';
				ccnt++;
			}
			(void) strncpy(bp, name, (BUFSIZ - ccnt) - 2);
			bp[BUFSIZ - ccnt - 1] = '\0';
			if ((fp = fopen(buf, mode)))
				return fp;
			if (*pp)
				pp++;
		}
	}
	return (FILE *)0;
}

#if defined(MICRO) || defined(WIN32)
void nethack_exit(code)
int code;
{
	msexit();
	exit(code);
}

/* Chdir back to original directory
 */

static void msexit()
{
#ifdef CHDIR
	extern char orgdir[];
#endif

	flushout();
# ifndef WIN32
	enable_ctrlP(); 	/* in case this wasn't done */
# endif
#if defined(CHDIR) && !defined(NOCWD_ASSUMPTIONS)
	chdir(orgdir);		/* chdir, not chdirx */
	chdrive(orgdir);
#endif
#ifdef WIN32CON
	/* Only if we started from the GUI, not the command prompt,
	 * we need to get one last return, so the score board does
	 * not vanish instantly after being created.
	 * GUILaunched is defined and set in nttty.c.
	 */
	synch_cursor();
	if (GUILaunched) getreturn("to end");
	synch_cursor();
#endif
	return;
}
#endif /* MICRO || WIN32 */
