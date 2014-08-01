/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined(__cplusplus)

#define E extern

/* some old <sys/types.h> may not define off_t and size_t; if your system is
 * one of these, define them by hand below
 */
#include <sys/types.h>

#endif /* !__cplusplus */

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if (defined(_MSC_VER) || defined(__SC__) || defined(WIN32))
# define SIG_RET_TYPE void (__cdecl *)(int)
#endif
#ifndef SIG_RET_TYPE
#  define SIG_RET_TYPE void (*)()
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
E int FDECL(write, (int,genericptr_t,unsigned));
# endif

# ifndef __SC__
E int FDECL(unlink, (const char *));
# endif

#endif /*  __GNUC__ */

/* both old & new versions of Ultrix want these, but real BSD does not */

#ifdef POSIX_TYPES
/* The POSIX string.h is required to define all the mem* and str* functions */
#include <string.h>
#else
#if defined(SYSV)
E int FDECL(memcmp, (const void *,const void *,size_t));
E void *FDECL(memcpy, (void *, const void *, size_t));
E void *FDECL(memset, (void *, int, size_t));
#else
#endif
#endif /* POSIX_TYPES */

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

#if defined(SYSV)
E char	*FDECL(strchr, (const char *,int));
E char	*FDECL(strrchr, (const char *,int));
#else /* BSD */
E char	*FDECL(index, (const char *,int));
E char	*FDECL(rindex, (const char *,int));
#endif

E int	FDECL(strcmp, (const char *,const char *));
E int	FDECL(strncmp, (const char *,const char *,size_t));
E int	FDECL(strlen, (const char *));

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
#  if !defined(SVR4)
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


E int FDECL(tgetent, (char *,const char *));
E void FDECL(tputs, (const char *,int,int (*)()));
E int FDECL(tgetnum, (const char *));
E int FDECL(tgetflag, (const char *));
E char *FDECL(tgetstr, (const char *,char **));
E char *FDECL(tgoto, (const char *,int,int));

#ifdef ALLOC_C
E genericptr_t FDECL(malloc, (size_t));
#endif

/* time functions */

#undef E

#endif /*  !__cplusplus */

#endif /* SYSTEM_H */
