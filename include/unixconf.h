/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef UNIX
#ifndef UNIXCONF_H
#define UNIXCONF_H

/*
 * Some include files are in a different place under SYSV
 *	BSD		   SYSV
 * <sys/time.h>		<time.h>
 * <sgtty.h>		<termio.h>
 *
 * Some routines are called differently
 * index		strchr
 * rindex		strrchr
 *
 */

/* define exactly one of the following four choices */
/* #define BSD 1 */	/* define for 4.n/Free/Open/Net BSD  */
			/* also for relatives like older versions of Linux */
#define SYSV		/* define for System V, Solaris 2.x, newer versions */
			/* of Linux */

/* define any of the following that are appropriate */
#define SVR4		/* use in addition to SYSV for System V Release 4 */
			/* including Solaris 2+ */
#define NETWORK		/* if running on a networked system */
			/* e.g. Suns sharing a playground through NFS */
/* #define LINUX */	/* Another Unix clone */
/* #define CYGWIN32 */	/* Unix on Win32 -- use with case sensitive defines */

#define TERMINFO	/* uses terminfo rather than termcap */
			/* Should be defined for most SYSV, SVR4 (including
			 * Solaris 2+), HPUX, and Linux systems.  In
			 * particular, it should NOT be defined for the UNIXPC
			 * unless you remove the use of the shared library in
			 * the Makefile */
#define POSIX_JOB_CONTROL /* use System V / Solaris 2.x / POSIX job control */
			/* (e.g., VSUSP) */
#define POSIX_TYPES	/* use POSIX types for system calls and termios */
			/* Define for many recent OS releases, including
			 * those with specific defines (since types are
			 * changing toward the standard from earlier chaos).
			 * For example, platforms using the GNU libraries,
			 * Linux, Solaris 2.x
			 */

/* #define RANDOM */		/* if neither random/srandom nor lrand48/srand48
				   is available from your system */

/*
 * The next two defines are intended mainly for the Andrew File System,
 * which does not allow hard links.  If NO_FILE_LINKS is defined, lock files
 * will be created in LOCKDIR using open() instead of in the playground using
 * link().
 *		Ralf Brown, 7/26/89 (from v2.3 hack of 10/10/88)
 */

/* #define NO_FILE_LINKS */	/* if no hard links */
#ifdef NO_FILE_LINKS
# define LOCKDIR FILE_AREA_VAR			/* where to put locks */
#endif

/*
 * If you want the static parts of your playground on a read-only file
 * system, define VAR_PLAYGROUND to be where the variable parts are kept.
 */
/* #define VAR_PLAYGROUND "/var/lib/games/nethack" */

/*
 * Define PORT_HELP to be the name of the port-specfic help file.
 * This file is found in HACKDIR.
 * Normally, you shouldn't need to change this.
 * There is currently no port-specific help for Unix systems.
 */
/* #define PORT_HELP "Unixhelp" */

#ifdef TTY_GRAPHICS
/*
 * To enable the `timed_delay' option for using a timer rather than extra
 * screen output when pausing for display effect.  Requires that `msleep'
 * function be available (with time argument specified in milliseconds).
 * Various output devices can produce wildly varying delays when the
 * "extra output" method is used, but not all systems provide access to
 * a fine-grained timer.
 */
/* #define TIMED_DELAY */	/* usleep() */
#endif

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives.  If you also define DEF_MAILREADER then this will
 * be the default mail reader, and can be overridden by the environment
 * variable MAILREADER; otherwise an internal pager will be used.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */

#define MAIL			/* Deliver mail during the game */

/* NO_MAILREADER is for kerberos authenticating filesystems where it is
 * essentially impossible to securely exec child processes, like mail
 * readers, when the game is running under a special token.
 *
 *	       dan
 */

/* #define NO_MAILREADER */	/* have mail daemon just tell player of mail */

#ifdef	MAIL
# if defined(BSD)
#   if defined(__FreeBSD__) || defined(__OpenBSD__)
#define DEF_MAILREADER	"/usr/bin/mail"
#   else
#define DEF_MAILREADER	"/usr/ucb/Mail"
#   endif
#else
# if defined(SYSV) && !defined(LINUX)
#define DEF_MAILREADER	"/usr/bin/mailx"
# else
#define DEF_MAILREADER	"/bin/mail"
# endif
#endif

#endif	/* MAIL */

#define FCMASK	0660	/* file creation mask */

/* fcntl(2) is a POSIX-portable call for manipulating file descriptors.
 * Comment out the USE_FCNTL if for some reason you have a strange
 * os/filesystem combination for which fcntl(2) does not work. */
#ifdef HAVE_FCNTL
# define USE_FCNTL
#endif

/*
 * The remainder of the file should not need to be changed.
 */

#if defined(LINUX)
# ifndef POSIX_TYPES
#  define POSIX_TYPES
# endif
# ifndef POSIX_JOB_CONTROL
#  define POSIX_JOB_CONTROL
# endif
#endif

/*
 * BSD systems are normally the only ones that can suspend processes.
 * Suspending NetHack processes cleanly should be easy to add to other systems
 * that have SIGTSTP in the Berkeley sense.  Currently the only such systems
 * known to work are HPUX; other systems will probably require
 * tweaks to unixtty.c and ioctl.c.
 *
 * POSIX defines a slightly different type of job control, which should be
 * equivalent for NetHack's purposes.  POSIX_JOB_CONTROL should work on
 * various recent SYSV versions (with possibly tweaks to unixtty.c again).
 */
#ifndef POSIX_JOB_CONTROL
# if defined(BSD)
#  define BSD_JOB_CONTROL
# else
#  if defined(SVR4)
#   define POSIX_JOB_CONTROL
#  endif
# endif
#endif
#if defined(BSD_JOB_CONTROL) || defined(POSIX_JOB_CONTROL) || defined(AUX)
#define SUSPEND		/* let ^Z suspend the game */
#endif

#define HLOCK	"perm"	/* an empty file used for locking purposes */
#define HLOCK_AREA	FILE_AREA_VAR

#define tgetch getchar

#if defined(BSD)
#define memcpy(d, s, n)		bcopy(s, d, n)
#define memcmp(s1, s2, n)	bcmp(s2, s1, n)
#else	/* therefore SYSV */
# ifndef index	/* some systems seem to do this for you */
#define index	strchr
# endif
# ifndef rindex
#define rindex	strrchr
# endif
#endif

/* Use the high quality random number routines. */
#if defined(BSD) || defined(LINUX) || defined(CYGWIN32) || defined(RANDOM)
#define Rand()	random()
#else
#define Rand()	lrand48()
#endif

#ifdef TIMED_DELAY
# if defined(LINUX) || defined(BSD)
# define msleep(k) usleep((k)*1000)
# endif
#endif

#endif /* UNIXCONF_H */
#endif /* UNIX */
