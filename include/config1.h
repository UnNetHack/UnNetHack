/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

/*
 * Windows NT Autodetection
 */
#ifdef WIN32
# undef UNIX
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
