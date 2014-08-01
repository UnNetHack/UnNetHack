/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define TRADSTDC_H

/*
 * ANSI X3J11 detection.
 * Makes substitutes for compatibility with the old C standard.
 */

/* Decide how to handle variable parameter lists:
 * USE_STDARG means use the ANSI <stdarg.h> facilities (only ANSI compilers
 * should do this, and only if the library supports it).
 * USE_VARARGS means use the <varargs.h> facilities.  Again, this should only
 * be done if the library supports it.	ANSI is *not* required for this.
 * Otherwise, the kludgy old methods are used.
 * The defaults are USE_STDARG for ANSI compilers.
 */

/* #define USE_VARARGS */	/* use <varargs.h> instead of <stdarg.h> */

# if !defined(USE_VARARGS) || !defined(USE_STDARG)
#   define USE_STDARG
# endif

#ifdef NEED_VARARGS		/* only define these if necessary */
#ifdef USE_STDARG
#include <stdarg.h>
# define VA_DECL(typ1,var1)	(typ1 var1, ...) { va_list the_args;
# define VA_DECL2(typ1,var1,typ2,var2)	\
	(typ1 var1, typ2 var2, ...) { va_list the_args;
# define VA_INIT(var1,typ1)
# define VA_NEXT(var1,typ1)	var1 = va_arg(the_args, typ1)
# define VA_ARGS		the_args
# define VA_START(x)		va_start(the_args, x)
# define VA_END()		va_end(the_args)
#else
# ifdef USE_VARARGS
#include <varargs.h>
#  define VA_DECL(typ1,var1)	(va_alist) va_dcl {\
		va_list the_args; typ1 var1;
#  define VA_DECL2(typ1,var1,typ2,var2) (va_alist) va_dcl {\
		va_list the_args; typ1 var1; typ2 var2;
#  define VA_ARGS		the_args
#  define VA_START(x)		va_start(the_args)
#  define VA_INIT(var1,typ1)	var1 = va_arg(the_args, typ1)
#  define VA_NEXT(var1,typ1)	var1 = va_arg(the_args,typ1)
#  define VA_END()		va_end(the_args)
# else
#   define VA_ARGS	arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
#   define VA_DECL(typ1,var1)  (var1,VA_ARGS) typ1 var1; \
	char *arg1,*arg2,*arg3,*arg4,*arg5,*arg6,*arg7,*arg8,*arg9; {
#   define VA_DECL2(typ1,var1,typ2,var2)  (var1,var2,VA_ARGS) \
	typ1 var1; typ2 var2;\
	char *arg1,*arg2,*arg3,*arg4,*arg5,*arg6,*arg7,*arg8,*arg9; {
#   define VA_START(x)
#   define VA_INIT(var1,typ1)
#   define VA_END()
# endif
#endif
#endif /* NEED_VARARGS */

/*
 * Used for robust ANSI parameter forward declarations:
 * int VDECL(sprintf, (char *, const char *, ...));
 *
 * NDECL() is used for functions with zero arguments;
 * FDECL() is used for functions with a fixed number of arguments;
 * VDECL() is used for functions with a variable number of arguments.
 * Separate macros are needed because ANSI will mix old-style declarations
 * with prototypes, except in the case of varargs, and the OVERLAY-specific
 * trampoli.* mechanism conflicts with the ANSI <<f(void)>> syntax.
 */

# define NDECL(f)	f(void) /* overridden later if USE_TRAMPOLI set */

# define FDECL(f,p)	f p

# if defined(USE_STDARG)
#  define VDECL(f,p)	f p
# else
#  define VDECL(f,p)	f()
# endif

/* generic pointer, always a macro; genericptr_t is usually a typedef */
# define genericptr	void *


#ifndef genericptr_t
typedef genericptr genericptr_t;	/* (void *) or (char *) */
#endif

/*
 * Allow gcc2 to check parameters of printf-like calls with -Wformat;
 * append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
# if __GNUC__ >= 2
#define PRINTF_F(f,v) __attribute__ ((format (printf, f, v)))
# endif
#endif
#ifndef PRINTF_F
#define PRINTF_F(f,v)
#endif

#endif /* TRADSTDC_H */
