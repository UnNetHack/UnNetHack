/*	SCCS Id: @(#)pcmain.c	3.4	2002/08/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - MSDOS, OS/2, ST, Amiga, and NT NetHack */

#include "hack.h"
#include "dlb.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include <ctype.h>


#ifdef WIN32
#include "win32api.h"			/* for GetModuleFileName */
#endif

#ifdef __DJGPP__
#include <unistd.h>			/* for getcwd() prototype */
#endif

char orgdir[PATHLEN];	/* also used in pcsys.c, amidos.c */

#ifdef TOS
boolean run_from_desktop = TRUE;	/* should we pause before exiting?? */
# ifdef __GNUC__
long _stksize = 16*1024;
# endif
#endif

#ifdef AMIGA
extern int bigscreen;
void  preserve_icon ();
#endif

static void process_options(int argc,char **argv);
static void nhusage();

#if defined(MICRO) || defined(WIN32) || defined(OS2)
extern void nethack_exit(int);
#else
#define nethack_exit exit
#endif

#ifdef WIN32
extern boolean getreturn_enabled;	/* from sys/share/pcsys.c */
#endif

#if defined(MSWIN_GRAPHICS)
extern void mswin_destroy_reg();
#endif

#ifdef EXEPATH
static char *exepath(char *);
#endif

int main(int,char **);

extern int pcmain(int,char **);


#if defined(__BORLANDC__) && !defined(_WIN32)
void  startup ();
unsigned _stklen = STKSIZ;
#endif

/* If the graphics version is built, we don't need a main; it is skipped
 * to help MinGW decide which entry point to choose. If both main and
 * WinMain exist, the resulting executable won't work correctly.
 */
#ifndef MSWIN_GRAPHICS
int
main(argc,argv)
int argc;
char *argv[];
{
     boolean resuming = pcmain(argc,argv);
#ifdef LAN_FEATURES
     init_lan_features();
#endif
     moveloop(resuming);
     nethack_exit(EXIT_SUCCESS);
     /*NOTREACHED*/
     return 0;
}
#endif /*MSWIN_GRAPHICS*/

int
pcmain(argc,argv)
int argc;
char *argv[];
{

	register int fd;
	register char *dir;
#if defined(WIN32)
	char fnamebuf[BUFSZ], encodedfnamebuf[BUFSZ];
#endif
#ifdef NOCWD_ASSUMPTIONS
	char failbuf[BUFSZ];
#endif
    boolean resuming = FALSE; /* assume new game */

#if defined(__BORLANDC__) && !defined(_WIN32)
	startup();
#endif

#ifdef TOS
	long clock_time;
	if (*argv[0]) { 		/* only a CLI can give us argv[0] */
		hname = argv[0];
		run_from_desktop = FALSE;
	} else
#endif
		hname = "UnNetHack";      /* used for syntax messages */

	choose_windows(DEFAULT_WINDOW_SYS);

#if !defined(AMIGA) && !defined(GNUDOS)
	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == (char *)0)
		error("UnNetHack: current directory path too long");
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) nethack_exit);	/* restore original directory */
# endif
#endif /* !AMIGA && !GNUDOS */

	dir = nh_getenv("NETHACKDIR");
	if (dir == (char *)0)
		dir = nh_getenv("HACKDIR");
#ifdef EXEPATH
	if (dir == (char *)0)
		dir = exepath(argv[0]);
#endif
	if (dir != (char *)0) {
		(void) strncpy(hackdir, dir, PATHLEN - 1);
		hackdir[PATHLEN-1] = '\0';
#ifdef NOCWD_ASSUMPTIONS
		{
		    int prefcnt;

		    fqn_prefix[0] = (char *)alloc(strlen(hackdir)+2);
		    Strcpy(fqn_prefix[0], hackdir);
		    append_slash(fqn_prefix[0]);
		    for (prefcnt = 1; prefcnt < PREFIX_COUNT; prefcnt++)
			fqn_prefix[prefcnt] = fqn_prefix[0];
		}
#endif
#ifdef CHDIR
		chdirx (dir, 1);
#endif
	}
#ifdef AMIGA
# ifdef CHDIR
	/*
	 * If we're dealing with workbench, change the directory.  Otherwise
	 * we could get "Insert disk in drive 0" messages. (Must be done
	 * before initoptions())....
	 */
	if(argc == 0)
		chdirx(HACKDIR, 1);
# endif
	ami_wininit_data();
#endif
	initoptions();

#ifdef NOCWD_ASSUMPTIONS
	if (!validate_prefix_locations(failbuf)) {
		raw_printf("Some invalid directory locations were specified:\n\t%s\n",
				failbuf);
		 nethack_exit(EXIT_FAILURE);
	}
#endif

#if defined(TOS) && defined(TEXTCOLOR)
	if (iflags.BIOS && iflags.use_color)
		set_colors();
#endif
	if (!hackdir[0])
#if !defined(LATTICE) && !defined(AMIGA)
		Strcpy(hackdir, orgdir);
#else
		Strcpy(hackdir, HACKDIR);
#endif
	if(argc > 1) {
	    if (!strncmp(argv[1], "-d", 2) && argv[1][2] != 'e') {
		/* avoid matching "-dec" for DECgraphics; since the man page
		 * says -d directory, hope nobody's using -desomething_else
		 */
		argc--;
		argv++;
		dir = argv[0]+2;
		if(*dir == '=' || *dir == ':') dir++;
		if(!*dir && argc > 1) {
			argc--;
			argv++;
			dir = argv[0];
		}
		if(!*dir)
		    error("Flag -d must be followed by a directory name.");
		Strcpy(hackdir, dir);
	    }
	    if (argc > 1) {

		/*
		 * Now we know the directory containing 'record' and
		 * may do a prscore().
		 */
		if (!strncmp(argv[1], "-s", 2)) {
#if !defined(MSWIN_GRAPHICS)
# if defined(CHDIR) && !defined(NOCWD_ASSUMPTIONS)
			chdirx(hackdir,0);
# endif
			prscore(argc, argv);
#else
			raw_printf("-s is not supported for the Graphical Interface\n");
#endif /*MSWIN_GRAPHICS*/
			nethack_exit(EXIT_SUCCESS);
		}

#ifdef MSWIN_GRAPHICS
		if (!strncmpi(argv[1], "-clearreg", 6)) {	/* clear registry */
			mswin_destroy_reg();
			nethack_exit(EXIT_SUCCESS);
		}
#endif
		/* Don't initialize the window system just to print usage */
		if (!strncmp(argv[1], "-?", 2) || !strncmp(argv[1], "/?", 2)) {
			nhusage();
			nethack_exit(EXIT_SUCCESS);
		}
	    }
	}

	/*
	 * It seems you really want to play.
	 */
#ifdef TOS
	if (comp_times((long)time(&clock_time)))
		error("Your clock is incorrectly set!");
#endif
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = 0;	/* prevent flush_screen() */

	/* chdir shouldn't be called before this point to keep the
	 * code parallel to other ports.
	 */
#if defined(CHDIR) && !defined(NOCWD_ASSUMPTIONS)
	chdirx(hackdir,1);
#endif

#ifdef MSDOS
	process_options(argc, argv);
	init_nhwindows(&argc,argv);
#else
	init_nhwindows(&argc,argv);
	process_options(argc, argv);
#endif

#ifdef WIN32CON
	toggle_mouse_support();	/* must come after process_options */
#endif

#ifdef MFLOPPY
	set_lock_and_bones();
# ifndef AMIGA
	copybones(FROMPERM);
# endif
#endif

	if (!*plname)
		askname();
	plnamesuffix(); 	/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if (wizard) {
# ifdef KR1ED
		if(!strcmp(plname, WIZARD_NAME))
# else
		if(!strcmp(plname, WIZARD))
# endif
			Strcpy(plname, "wizard");
		else {
			wizard = FALSE;
			discover = TRUE;
		}
	}
#endif /* WIZARD */
#if defined(PC_LOCKING)
	/* 3.3.0 added this to support detection of multiple games
	 * under the same plname on the same machine in a windowed
	 * or multitasking environment.
	 *
	 * That allows user confirmation prior to overwriting the
	 * level files of a game in progress.
	 *
	 * Also prevents an aborted game's level files from being
	 * overwritten without confirmation when a user starts up
	 * another game with the same player name.
	 */
# if defined(WIN32)
	/* Obtain the name of the logged on user and incorporate
	 * it into the name. */
	Sprintf(fnamebuf, "%s-%s", get_username(0), plname);
	(void)fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-.",
				'%', fnamebuf, encodedfnamebuf, BUFSZ);
	Sprintf(lock, "%s",encodedfnamebuf);
	/* regularize(lock); */ /* we encode now, rather than substitute */
# else
	Strcpy(lock,plname);
	regularize(lock);
# endif
	getlock();
#else   /* What follows is !PC_LOCKING */
# ifdef AMIGA /* We'll put the bones & levels in the user specified directory -jhsa */
	Strcat(lock,plname);
	Strcat(lock,".99");
# else
#  ifndef MFLOPPY
	/* I'm not sure what, if anything, is left here, but MFLOPPY has
	 * conflicts with set_lock_and_bones() in files.c.
	 */
	Strcpy(lock,plname);
	Strcat(lock,".99");
	regularize(lock);	/* is this necessary? */
				/* not compatible with full path a la AMIGA */
#  endif
# endif
#endif	/* PC_LOCKING */

	/* Set up level 0 file to keep the game state.
	 */
	fd = create_levelfile(0, (char *)0);
	if (fd < 0) {
		raw_print("Cannot create lock file");
	} else {
#ifdef WIN32
		hackpid = GetCurrentProcessId();
#else
		hackpid = 1;
#endif
		write(fd, (genericptr_t) &hackpid, sizeof(hackpid));
		close(fd);
	}
#ifdef MFLOPPY
	level_info[0].where = ACTIVE;
#endif

	/*
	 * Initialisation of the boundaries of the mazes
	 * Both boundaries have to be even.
	 */

	x_maze_max = COLNO-1;
	if (x_maze_max % 2)
		x_maze_max--;
	y_maze_max = ROWNO-1;
	if (y_maze_max % 2)
		y_maze_max--;

	/*
	 *  Initialize the vision system.  This must be before mklev() on a
	 *  new game or before a level restore on a saved game.
	 */
	vision_init();

	dlb_init();

	display_gamewindows();
#ifdef WIN32
	getreturn_enabled = TRUE;
#endif

	if ((fd = restore_saved_game()) >= 0) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
		if(iflags.news){
		    display_file_area(NEWS_AREA, NEWS, FALSE);
		    iflags.news = FALSE;
		}
#endif
		pline("Restoring save file...");
		mark_synch();	/* flush output */

		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = TRUE;
#endif
		check_special_room(FALSE);
		if (discover)
			You("are in non-scoring discovery mode.");

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n'){
				(void) delete_savefile();
			}
		}

		flags.move = 0;
        resuming = TRUE;
	} else {
not_recovered:
		player_selection();
		newgame();
		if (discover)
			You("are in non-scoring discovery mode.");

		flags.move = 0;
		set_wear();
		(void) pickup(1);
		read_engr_at(u.ux,u.uy);
	}

#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif
#ifdef OS2
	gettty(); /* somehow ctrl-P gets turned back on during startup ... */
#endif
	return resuming;
}

static void
process_options(argc, argv)
int argc;
char *argv[];
{
	int i;


	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
		case 'a':
			if (argv[0][2]) {
			    if ((i = str2align(&argv[0][2])) >= 0)
			    	flags.initalign = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2align(argv[0])) >= 0)
			    	flags.initalign = i;
			}
			break;
		case 'D':
#ifdef WIZARD
			/* If they don't have a valid wizard name, it'll be
			 * changed to discover later.  Cannot check for
			 * validity of the name right now--it might have a
			 * character class suffix, for instance.
			 */
			wizard = TRUE;
			break;
#endif
		case 'X':
			discover = TRUE;
			break;
#ifdef NEWS
		case 'n':
			iflags.news = FALSE;
			break;
#endif
		case 'u':
			if(argv[0][2])
			  (void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  (void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				raw_print("Player name expected after -u");
			break;
#ifndef AMIGA
		case 'I':
		case 'i':
			if (!strncmpi(argv[0]+1, "IBM", 3))
				switch_graphics(IBM_GRAPHICS);
			break;
	    /*	case 'D': */
		case 'd':
			if (!strncmpi(argv[0]+1, "DEC", 3))
				switch_graphics(DEC_GRAPHICS);
			break;
#endif
		case 'g':
			if (argv[0][2]) {
			    if ((i = str2gend(&argv[0][2])) >= 0)
			    	flags.initgend = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2gend(argv[0])) >= 0)
			    	flags.initgend = i;
			}
			break;
		case 'p': /* profession (role) */
			if (argv[0][2]) {
			    if ((i = str2role(&argv[0][2])) >= 0)
			    	flags.initrole = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2role(argv[0])) >= 0)
			    	flags.initrole = i;
			}
			break;
		case 'r': /* race */
			if (argv[0][2]) {
			    if ((i = str2race(&argv[0][2])) >= 0)
			    	flags.initrace = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2race(argv[0])) >= 0)
			    	flags.initrace = i;
			}
			break;
#ifdef MFLOPPY
# ifndef AMIGA
		/* Player doesn't want to use a RAM disk
		 */
		case 'R':
			ramdisk = FALSE;
			break;
# endif
#endif
#ifdef AMIGA
			/* interlaced and non-interlaced screens */
		case 'L':
			bigscreen = 1;
			break;
		case 'l':
			bigscreen = -1;
			break;
#endif
		case '@':
			flags.randomall = 1;
			break;
		default:
			if ((i = str2role(&argv[0][1])) >= 0) {
			    flags.initrole = i;
				break;
			} else raw_printf("\nUnknown switch: %s", argv[0]);
			/* FALL THROUGH */
		case '?':
			nhusage();
			nethack_exit(EXIT_SUCCESS);
		}
	}
}

static void
nhusage()
{
	char buf1[BUFSZ], buf2[BUFSZ], *bufptr;

	buf1[0] = '\0';
	bufptr = buf1;

#define ADD_USAGE(s)	if ((strlen(buf1) + strlen(s)) < (BUFSZ - 1)) Strcat(bufptr, s);

	/* -role still works for those cases which aren't already taken, but
	 * is deprecated and will not be listed here.
	 */
	(void) Sprintf(buf2,
"\nUsage:\n%s [-d dir] -s [-r race] [-p profession] [maxrank] [name]...\n       or",
		hname);
	ADD_USAGE(buf2);

	(void) Sprintf(buf2,
	 "\n%s [-d dir] [-u name] [-r race] [-p profession] [-[DX]]",
		hname);
	ADD_USAGE(buf2);
#ifdef NEWS
	ADD_USAGE(" [-n]");
#endif
#ifndef AMIGA
	ADD_USAGE(" [-I] [-i] [-d]");
#endif
#ifdef MFLOPPY
# ifndef AMIGA
	ADD_USAGE(" [-R]");
# endif
#endif
#ifdef AMIGA
	ADD_USAGE(" [-[lL]]");
#endif
	if (!iflags.window_inited)
		raw_printf("%s\n",buf1);
	else
		(void) printf("%s\n",buf1);
#undef ADD_USAGE
}

#ifdef CHDIR
void
chdirx(dir, wr)
char *dir;
boolean wr;
{
# ifdef AMIGA
	static char thisdir[] = "";
# else
	static char thisdir[] = ".";
# endif
	if(dir && chdir(dir) < 0) {
		error("Cannot chdir to %s.", dir);
	}

# ifndef AMIGA
	/* Change the default drive as well.
	 */
	chdrive(dir);
# endif

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) check_recordfile(dir ? dir : thisdir);
}
#endif /* CHDIR */

#ifdef PORT_HELP
# if defined(MSDOS) || defined(WIN32)
void
port_help()
{
    /* display port specific help file */
    display_file( PORT_HELP, 1 );
}
# endif /* MSDOS || WIN32 */
#endif /* PORT_HELP */

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
# ifndef WIN32
	Strcpy (tmp, str);
# else
	#ifdef UNICODE
	{
		TCHAR wbuf[BUFSZ];
		GetModuleFileName((HANDLE)0, wbuf, BUFSZ);
		WideCharToMultiByte(CP_ACP, 0, wbuf, -1, tmp, bsize, NULL, NULL);
	}
	#else
		*(tmp + GetModuleFileName((HANDLE)0, tmp, bsize)) = '\0';
	#endif
# endif
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */
/*pcmain.c*/
