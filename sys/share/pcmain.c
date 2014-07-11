/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c -NT NetHack */

#include "hack.h"
#include "dlb.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include <ctype.h>

#include <sys\stat.h>

#ifdef WIN32
#include "win32api.h"			/* for GetModuleFileName */
#endif

#ifdef OVL0
#define SHARED_DCL
#else
#define SHARED_DCL extern
#endif


SHARED_DCL char orgdir[PATHLEN];	/* also used in pcsys.c, amidos.c */

STATIC_DCL void FDECL(process_options,(int argc,char **argv));
STATIC_DCL void NDECL(nhusage);

#if defined(WIN32)
extern void FDECL(nethack_exit,(int));
#else
#define nethack_exit exit
#endif

#ifdef WIN32
extern boolean getreturn_enabled;	/* from sys/share/pcsys.c */
#endif

#if defined(MSWIN_GRAPHICS)
extern void NDECL(mswin_destroy_reg);
#endif

#ifdef EXEPATH
STATIC_DCL char *FDECL(exepath,(char *));
#endif

#ifdef OVL0
int FDECL(main, (int,char **));
#endif

extern void FDECL(pcmain, (int,char **));

#ifdef OVL0
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
     pcmain(argc,argv);
#ifdef LAN_FEATURES
     init_lan_features();
#endif
     moveloop();
     nethack_exit(EXIT_SUCCESS);
     /*NOTREACHED*/
     return 0;
}
#endif /*MSWIN_GRAPHICS*/
#endif /*OVL0*/
#ifdef OVL1

void
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

	hname = "UnNetHack";      /* used for syntax messages */

	choose_windows(DEFAULT_WINDOW_SYS);

	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == (char *)0)
		error("UnNetHack: current directory path too long");
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) nethack_exit);	/* restore original directory */
# endif

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
	}
	initoptions();

#ifdef NOCWD_ASSUMPTIONS
	if (!validate_prefix_locations(failbuf)) {
		raw_printf("Some invalid directory locations were specified:\n\t%s\n",
				failbuf);
		 nethack_exit(EXIT_FAILURE);
	}
#endif

	if (!hackdir[0])
#if !defined(LATTICE)
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
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = 0;	/* prevent flush_screen() */

	/* chdir shouldn't be called before this point to keep the
	 * code parallel to other ports.
	 */

	init_nhwindows(&argc,argv);
	process_options(argc, argv);

#ifdef WIN32CON
	toggle_mouse_support();	/* must come after process_options */
#endif

	if (!*plname)
		askname();
	plnamesuffix(); 	/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if (wizard) {
		if(!strcmp(plname, WIZARD))
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
	/* I'm not sure what, if anything, is left here, but MFLOPPY has
	 * conflicts with set_lock_and_bones() in files.c.
	 */
	Strcpy(lock,plname);
	Strcat(lock,".99");
	regularize(lock);	/* is this necessary? */
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
	return;
}

STATIC_OVL void
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

STATIC_OVL void 
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
	ADD_USAGE(" [-I] [-i] [-d]");
	if (!iflags.window_inited)
		raw_printf("%s\n",buf1);
	else
		(void) printf("%s\n",buf1);
#undef ADD_USAGE
}

#endif /*OVL1*/
#ifdef OVLB

#ifdef PORT_HELP
# if defined(WIN32)
void
port_help()
{
    /* display port specific help file */
    display_file( PORT_HELP, 1 );
}
# endif /* WIN32 */
#endif /* PORT_HELP */

#ifdef EXEPATH
#define PATH_SEPARATOR '\\'
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
#endif /*OVLB*/
/*pcmain.c*/
