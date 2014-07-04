/*	SCCS Id: @(#)system.h	3.4	2001/12/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined(__cplusplus)

#define E extern

#ifdef AUTOCONF
/*
 * When using GNU autoconf to configure, the switches are output to
 * autoconf.h which should override the defaults set earlier in
 * config.h and *conf.h
 */
#include "autoconf_paths.h"
#include "autoconf.h"
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_DECL_STRNICMP && !HAVE_DECL_STRNCMPI
#define strncmpi(a,b,c) strnicmp(a,b,c)
#endif
#endif

/* some old <sys/types.h> may not define off_t and size_t; if your system is
 * one of these, define them by hand below
 */
#include <sys/types.h>

#if defined(MICRO)
# if !defined(_SIZE_T) && !defined(__size_t) /* __size_t for CSet/2 */
#  define _SIZE_T
typedef unsigned int	size_t;
# endif
#endif	/* MICRO */

#endif /* !__cplusplus */

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if (defined(_MSC_VER) || defined(__SC__) || defined(WIN32))
# define SIG_RET_TYPE void (__cdecl *)(int)
#endif
#ifndef SIG_RET_TYPE
# if defined(NHSTDC) || defined(POSIX_TYPES) || defined(__DECC)
#  define SIG_RET_TYPE void (*)()
# endif
#endif
#ifndef SIG_RET_TYPE
# if defined(SVR3) || defined(SVR4)
	/* SVR3 is defined automatically by some systems */
#  define SIG_RET_TYPE void (*)()
# endif
#endif
#ifndef SIG_RET_TYPE	/* BSD, SIII, SVR2 and earlier, Sun3.5 and earlier */
# define SIG_RET_TYPE int (*)()
#endif

#if !defined(__cplusplus)

#if defined(BSD) || defined(RANDOM)
# ifdef random
# undef random
# endif
# if !defined(__SC__) && !defined(LINUX)
E  long NDECL(random);
# endif
# if (!defined(__FreeBSD__) && !defined(__NetBSD__)) || defined(RANDOM)
E void FDECL(srandom, (unsigned int));
# else
#  if !defined(__FreeBSD__) && !defined(__NetBSD__)
E int FDECL(srandom, (unsigned int));
#  endif
# endif
#else
E long lrand48();
E void srand48();
#endif /* BSD || RANDOM */

#ifdef LEGACY_CODE
#if !defined(BSD)
			/* real BSD wants all these to return int */
# ifndef MICRO
E void FDECL(exit, (int));
# endif /* MICRO */
/* If flex thinks that we're not __STDC__ it declares free() to return
   int and we die.  We must use __STDC__ instead of NHSTDC because
   the former is naturally what flex tests for. */
# if defined(__STDC__) || !defined(FLEX_SCANNER)
E void FDECL(free, (genericptr_t));
# endif
#if !defined(__SC__)
E void FDECL(perror, (const char *));
#endif
#endif
#endif /* LEGACY_CODE */
#ifdef POSIX_TYPES
E void FDECL(qsort, (genericptr_t,size_t,size_t,
		     int(*)(const genericptr,const genericptr)));
#endif

#if !defined(__GNUC__)
/* may already be defined */

E long FDECL(lseek, (int,long,int));
# if defined(POSIX_TYPES)
E int FDECL(write, (int, const void *,unsigned));
# else
#  ifndef __MWERKS__	/* metrowerks defines write via universal headers */
E int FDECL(write, (int,genericptr_t,unsigned));
#  endif
# endif

# ifndef __SC__
E int FDECL(unlink, (const char *));
# endif

#endif /*  __GNUC__ */

#if defined(MICRO)
E int FDECL(close, (int));
E int FDECL(read, (int,genericptr_t,unsigned int));
E int FDECL(open, (const char *,int,...));
E int FDECL(dup2, (int, int));
E int FDECL(setmode, (int,int));
E int NDECL(kbhit);
E int FDECL(chdir, (char *));
E char *FDECL(getcwd, (char *,int));
#endif

/* both old & new versions of Ultrix want these, but real BSD does not */
#ifdef MICRO
E void NDECL(abort);
E void FDECL(_exit, (int));
E int FDECL(system, (const char *));
#endif

#ifdef POSIX_TYPES
/* The POSIX string.h is required to define all the mem* and str* functions */
#include <string.h>
#else
#if defined(SYSV)
# if defined(NHSTDC)
E int FDECL(memcmp, (const void *,const void *,size_t));
E void *FDECL(memcpy, (void *, const void *, size_t));
E void *FDECL(memset, (void *, int, size_t));
# else
#  ifndef memcmp	/* some systems seem to macro these back to b*() */
E int memcmp();
#  endif
#  ifndef memcpy
E char *memcpy();
#  endif
#  ifndef memset
E char *memset();
#  endif
# endif
#else
#endif
#endif /* POSIX_TYPES */

#if defined(MICRO) && !defined(LATTICE)
#  if defined(NHSTDC) || defined(WIN32)
E int  FDECL(memcmp, (const void *, const void *, size_t));
E void *FDECL(memcpy, (void *, const void *, size_t));
E void *FDECL(memset, (void *, int, size_t));
#  else
E int FDECL(memcmp, (char *,char *,unsigned int));
E char *FDECL(memcpy, (char *,char *,unsigned int));
E char *FDECL(memset, (char*,int,int));
#  endif /* NHSTDC */
#endif /* MICRO */

#if defined(SYSV)
E unsigned sleep();
#endif

E char *FDECL(getenv, (const char *));
E char *getlogin();
# ifdef POSIX_TYPES
E pid_t NDECL(getpid);
E uid_t NDECL(getuid);
E gid_t NDECL(getgid);
# else	/*!POSIX_TYPES*/
#  ifndef getpid		/* Borland C defines getpid() as a macro */
E int NDECL(getpid);
#  endif
# endif	/*?POSIX_TYPES*/

/* add more architectures as needed */

/*# string(s).h #*/
#if !defined(_XtIntrinsic_h) && !defined(POSIX_TYPES)
/* <X11/Intrinsic.h> #includes <string[s].h>; so does defining POSIX_TYPES */

E char	*FDECL(strcpy, (char *,const char *));
E char	*FDECL(strncpy, (char *,const char *,size_t));
E char	*FDECL(strcat, (char *,const char *));
E char	*FDECL(strncat, (char *,const char *,size_t));
E char	*FDECL(strpbrk, (const char *,const char *));

#if defined(SYSV) || defined(MICRO)
E char	*FDECL(strchr, (const char *,int));
E char	*FDECL(strrchr, (const char *,int));
#else /* BSD */
E char	*FDECL(index, (const char *,int));
E char	*FDECL(rindex, (const char *,int));
#endif

E int	FDECL(strcmp, (const char *,const char *));
E int	FDECL(strncmp, (const char *,const char *,size_t));
#if defined(MICRO)
E size_t FDECL(strlen, (const char *));
#else
E int	FDECL(strlen, (const char *));
#endif /* MICRO */

#endif	/* !_XtIntrinsic_h_ && !POSIX_TYPES */

/* Old varieties of BSD have char *sprintf().
 * Newer varieties of BSD have int sprintf() but allow for the old char *.
 * Several varieties of SYSV and PC systems also have int sprintf().
 * If your system doesn't agree with this breakdown, you may want to change
 * this declaration, especially if your machine treats the types differently.
 * If your system defines sprintf, et al, in stdio.h, add to the initial
 * #if.
 */
#if defined(__DECC) || defined(WIN32)
#define SPRINTF_PROTO
#endif
#if defined(__GNUC__)
	/* problem with prototype mismatches */
#define SPRINTF_PROTO
#endif
#if defined(__MWERKS__) || defined(__SC__)
	/* Metrowerks already has a prototype for sprintf() */
# define SPRINTF_PROTO
#endif

#ifndef SPRINTF_PROTO
# if defined(POSIX_TYPES) || !defined(BSD)
E  int FDECL(sprintf, (char *,const char *,...));
# else
#  define OLD_SPRINTF
E  char *sprintf();
# endif
#endif
#ifdef SPRINTF_PROTO
# undef SPRINTF_PROTO
#endif

#ifdef NEED_VARARGS
# if defined(USE_STDARG) || defined(USE_VARARGS)
#  if !defined(SVR4) && !defined(apollo)
E int FDECL(vsprintf, (char *, const char *, va_list));
E int FDECL(vfprintf, (FILE *, const char *, va_list));
E int FDECL(vprintf, (const char *, va_list));
#  endif
# else
#  define vprintf	printf
#  define vfprintf	fprintf
#  define vsprintf	sprintf
# endif
#endif /* NEED_VARARGS */


#ifdef MICRO
E int FDECL(tgetent, (const char *,const char *));
E void FDECL(tputs, (const char *,int,int (*)()));
E int FDECL(tgetnum, (const char *));
E int FDECL(tgetflag, (const char *));
E char *FDECL(tgetstr, (const char *,char **));
E char *FDECL(tgoto, (const char *,int,int));
#else
E int FDECL(tgetent, (char *,const char *));
E void FDECL(tputs, (const char *,int,int (*)()));
E int FDECL(tgetnum, (const char *));
E int FDECL(tgetflag, (const char *));
E char *FDECL(tgetstr, (const char *,char **));
E char *FDECL(tgoto, (const char *,int,int));
#endif

#ifdef ALLOC_C
E genericptr_t FDECL(malloc, (size_t));
#endif

/* time functions */

#ifdef LEGACY_CODE
# ifndef LATTICE
E struct tm *FDECL(localtime, (const time_t *));
# endif

# if (defined(BSD) && defined(POSIX_TYPES)) || defined(SYSV) || defined(MICRO)
E time_t FDECL(time, (time_t *));
# else
E long FDECL(time, (time_t *));
# endif
#endif /* LEGACY_CODE */


#ifdef MICRO
# ifdef abs
# undef abs
# endif
E int FDECL(abs, (int));
# ifdef atoi
# undef atoi
# endif
E int FDECL(atoi, (const char *));
#endif

#undef E

#endif /*  !__cplusplus */

#endif /* SYSTEM_H */
