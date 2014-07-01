/*	SCCS Id: @(#)recover.c	3.4	1999/10/23	*/
/*	Copyright (c) Janet Walz, 1992.				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Utility for reconstructing NetHack save file from a set of individual
 *  level files.  Requires that the `checkpoint' option be enabled at the
 *  time NetHack creates those level files.
 */
#include "config.h"
#if !defined(O_WRONLY) && !defined(LSC) && !defined(AZTEC_C)
#include <fcntl.h>
#endif
#ifdef WIN32
#include <errno.h>
#include "win32api.h"
#endif

int FDECL(restore_savefile, (char *, const char *));
static void FDECL(set_levelfile_name, (int));
static int FDECL(open_levelfile, (int, const char *));
static int NDECL(create_savefile);
void FDECL(copy_bytes, (int,int));

#define Fprintf	(void)fprintf

#define Close	(void)close

#ifdef UNIX
#define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
#  ifdef WIN32
#define SAVESIZE	(PL_NSIZ + 40)  /* username-player.NetHack-saved-game */
#  else
#define SAVESIZE	FILENAME	/* from pcconf.h */
#  endif
#endif

#if defined(EXEPATH)
char *FDECL(exepath, (char *));
#endif

#if defined(__BORLANDC__) && !defined(_WIN32)
extern unsigned _stklen = STKSIZ;
#endif
char savename[SAVESIZE]; /* holds relative path of save file from playground */

#ifndef NO_MAIN
int
main(argc, argv)
int argc;
char *argv[];
{
	int argno;
	const char *dir = (char *)0;

	if (!dir) dir = getenv("NETHACKDIR");
	if (!dir) dir = getenv("HACKDIR");
#ifdef FILE_AREAS
	if (!dir) dir = FILE_AREA_LEVL;
#endif
#if defined(EXEPATH)
	if (!dir) dir = exepath(argv[0]);
#endif
	if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-"))) {
	    Fprintf(stderr,
		"Usage: %s [ -d directory ] base1 [ base2 ... ]\n", argv[0]);
#if defined(WIN32)
	    if (dir) {
	    	Fprintf(stderr, "\t(Unless you override it with -d, recover will look \n");
	    	Fprintf(stderr, "\t in the %s directory on your system)\n", dir);
	    }
#endif
	    exit(EXIT_FAILURE);
	}

	argno = 1;
	if (!strncmp(argv[argno], "-d", 2)) {
		dir = argv[argno]+2;
		if (*dir == '=' || *dir == ':') dir++;
		if (!*dir && argc > argno) {
			argno++;
			dir = argv[argno];
		}
		if (!*dir) {
		    Fprintf(stderr,
			"%s: flag -d must be followed by a directory name.\n",
			argv[0]);
		    exit(EXIT_FAILURE);
		}
		argno++;
	}
#if defined(SECURE)
	if (dir
# ifdef HACKDIR
		&& strcmp(dir, HACKDIR)
# endif
		) {
		(void) setgid(getgid());
		(void) setuid(getuid());
	}
#endif	/* SECURE */

#ifdef HACKDIR
	if (!dir) dir = HACKDIR;
#endif

	if (dir && chdir((char *) dir) < 0) {
		Fprintf(stderr, "%s: cannot chdir to %s.\n", argv[0], dir);
		exit(EXIT_FAILURE);
	}

	while (argc > argno) {
		if (restore_savefile(argv[argno], dir) == 0)
		    Fprintf(stderr, "recovered \"%s\" to %s\n",
			    argv[argno], savename);
		argno++;
	}
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}
#endif

static char lock[256];

static void
set_levelfile_name(lev)
int lev;
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = lock + strlen(lock);
	(void) sprintf(tf, ".%d", lev);
}

static int
open_levelfile(lev, directory)
int lev;
const char *directory;
{
	int fd;
	char levelfile[BUFSIZ];

	set_levelfile_name(lev);
	if (directory) {
		snprintf(levelfile, BUFSIZ, "%s/%s", directory, lock);
	} else {
		strcpy(levelfile, lock);
	}
#if defined(MICRO) || defined(WIN32)
	fd = open(levelfile, O_RDONLY | O_BINARY);
#else
	fd = open(levelfile, O_RDONLY, 0);
#endif
	return fd;
}

static int
create_savefile()
{
	int fd;
	char savefile[BUFSIZ];
#ifdef FILE_AREAS
	snprintf(savefile, BUFSIZ, "%s/%s", FILE_AREA_SAVE, savename);
#else
	strcpy(savefile, savename);
#endif

#if defined(MICRO) || defined(WIN32)
	fd = open(savefile, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
	fd = creat(savefile, FCMASK);
#endif
	return fd;
}

void
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) {
			Fprintf(stderr, "file copy failed!\n");
			exit(EXIT_FAILURE);
		}
	} while (nfrom == BUFSIZ);
}

int
restore_savefile(basename, directory)
char *basename;
const char *directory;
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;
	struct version_info version_data;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	(void) strcpy(lock, basename);
	gfd = open_levelfile(0, directory);
	if (gfd < 0) {
#if defined(WIN32) && !defined(WIN_CE)
 	    if(errno == EACCES) {
	  	Fprintf(stderr,
			"\nThere are files from a game in progress under your name.");
		Fprintf(stderr,"\nThe files are locked or inaccessible.");
		Fprintf(stderr,"\nPerhaps the other game is still running?\n");
	    } else
	  	Fprintf(stderr,
			"\nTrouble accessing level 0 (errno = %d).\n", errno);
#endif
	    Fprintf(stderr, "Cannot open level 0 for %s.\n", basename);
	    return(-1);
	}
	if (read(gfd, (genericptr_t) &hpid, sizeof hpid) != sizeof hpid) {
	    Fprintf(stderr, "%s\n%s%s%s\n",
	     "Checkpoint data incompletely written or subsequently clobbered;",
		    "recovery for \"", basename, "\" impossible.");
	    Close(gfd);
	    return(-1);
	}
	if (read(gfd, (genericptr_t) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    Fprintf(stderr,
	    "Checkpointing was not in effect for %s -- recovery impossible.\n",
		    basename);
	    Close(gfd);
	    return(-1);
	}
	if ((read(gfd, (genericptr_t) savename, sizeof savename)
		!= sizeof savename) ||
	    (read(gfd, (genericptr_t) &version_data, sizeof version_data)
		!= sizeof version_data)) {
	    Fprintf(stderr, "Error reading %s -- can't recover.\n", lock);
	    Close(gfd);
	    return(-1);
	}

	/* save file should contain:
	 *	version info
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	sfd = create_savefile();
	if (sfd < 0) {
	    Fprintf(stderr, "Cannot create savefile %s.\n", savename);
	    Close(gfd);
	    return(-1);
	}

	lfd = open_levelfile(savelev, directory);
	if (lfd < 0) {
	    Fprintf(stderr, "Cannot open level of save for %s.\n", basename);
	    Close(gfd);
	    Close(sfd);
	    return(-1);
	}

	if (write(sfd, (genericptr_t) &version_data, sizeof version_data)
		!= sizeof version_data) {
	    Fprintf(stderr, "Error writing %s; recovery failed.\n", savename);
	    Close(gfd);
	    Close(sfd);
	    return(-1);
	}

	copy_bytes(lfd, sfd);
	Close(lfd);
	(void) unlink(lock);

	copy_bytes(gfd, sfd);
	Close(gfd);
	set_levelfile_name(0);
	(void) unlink(lock);

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev, directory);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (genericptr_t) &levc, sizeof(levc));
				copy_bytes(lfd, sfd);
				Close(lfd);
				(void) unlink(lock);
			}
		}
	}

	Close(sfd);

#if 0 /* OBSOLETE, HackWB is no longer in use */
#endif
	return(0);
}

#ifdef EXEPATH
# ifdef __DJGPP__
#define PATH_SEPARATOR '/'
# else
#define PATH_SEPARATOR '\\'
# endif

#define EXEPATHBUFSZ 256
char exepathbuf[EXEPATHBUFSZ];

char *exepath(str)
char *str;
{
	char *tmp, *tmp2;
	int bsize;

	if (!str) return (char *)0;
	bsize = EXEPATHBUFSZ;
	tmp = exepathbuf;
#if !defined(WIN32)
	strcpy (tmp, str);
#else
	*(tmp + GetModuleFileName((HANDLE)0, tmp, bsize)) = '\0';
#endif
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */

/*recover.c*/
