/*	SCCS Id: @(#)panic.c	3.4	1994/03/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This code was adapted from the code in end.c to run in a standalone
 *	mode for the makedefs / drg code.
 */

#define NEED_VARARGS
#include "config.h"

/*VARARGS1*/
boolean panicking;
void VDECL(panic, (char *,...));

void
panic VA_DECL(char *,str)
	VA_START(str);
	VA_INIT(str, char *);
	if(panicking++)
#ifdef SYSV
	    (void)
#endif
		abort();    /* avoid loops - this should never happen*/

	(void) fputs(" ERROR:  ", stderr);
	Vfprintf(stderr, str, VA_ARGS);
	(void) fflush(stderr);
#if defined(UNIX)
# ifdef SYSV
		(void)
# endif
		    abort();	/* generate core dump */
#endif
	VA_END();
	exit(EXIT_FAILURE);		/* redundant */
	return;
}

#ifdef ALLOCA_HACK
/*
 * In case bison-generated foo_yacc.c tries to use alloca(); if we don't
 * have it then just use malloc() instead.  This may not work on some
 * systems, but they should either use yacc or get a real alloca routine.
 */
long *alloca(cnt)
unsigned cnt;
{
	return cnt ? alloc(cnt) : (long *)0;
}
#endif

/*panic.c*/
