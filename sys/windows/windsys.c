/* NetHack 3.7	windsys.c	$NHDT-Date: 1710949760 2024/03/20 15:49:20 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.95 $ */
/* Copyright (c) NetHack PC Development Team 1993, 1994 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  WIN32 system functions.
 *
 *  Included in both console-based and window-based clients on the windows platform.
 *
 *  Initial Creation: Michael Allison - January 31/93
 *
 */

#ifdef __MINGW32__
#include <windows.h>
#else
#include "win10.h"
#include "winos.h"
#endif

#define NEED_VARARGS
#include "hack.h"
#include <dos.h>
#ifndef __BORLANDC__
#include <direct.h>
#endif
#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif
#include <inttypes.h>

#ifdef WIN32
#include <versionhelpers.h>

/*
 * The following WIN32 API routines are used in this file.
 *
 * GetDiskFreeSpace
 * GetVolumeInformation
 * GetUserName
 * FindFirstFile
 * FindNextFile
 * FindClose
 *
 */

/* runtime cursor display control switch */
boolean win32_cursorblink;

/* globals required within here */
HANDLE ffhandle = (HANDLE) 0;
WIN32_FIND_DATA ffd;
extern int GUILaunched;
boolean getreturn_enabled;
int redirect_stdout;

#ifdef WIN32CON
typedef HWND(WINAPI *GETCONSOLEWINDOW)(void);
#if 0
static HWND GetConsoleHandle(void);
static HWND GetConsoleHwnd(void);
#endif /* 0 */
#endif /* WIN32CON */
#if !defined(TTY_GRAPHICS)
extern void backsp(void);
#endif
int windows_console_custom_nhgetch(void);
extern void safe_routines(void);
int windows_early_options(const char *window_opt);
unsigned long sys_random_seed(void);
#if 0
static int max_filename(void);
#endif

/* The function pointer nt_kbhit contains a kbhit() equivalent
 * which varies depending on which window port is active.
 * For the tty port it is tty_kbhit() [from consoletty.c]
 * For the win32 port it is win32_kbhit() [from winmain.c]
 * It is initialized to point to def_kbhit [in here] for safety.
 */

int def_kbhit(void);
int (*nt_kbhit)(void) = def_kbhit;

#ifndef WIN32CON
/* this is used as a printf() replacement when the window
 * system isn't initialized yet
 */
void msmsg
VA_DECL(const char *, fmt)
{
    VA_START(fmt);
    VA_INIT(fmt, const char *);
    VA_END();
    return;
}
#endif  /* WIN32CON */

char
switchar(void)
{
    /* Could not locate a WIN32 API call for this- MJA */
    return '-';
}

long
freediskspace(char *path)
{
    char tmppath[4];
    DWORD SectorsPerCluster = 0;
    DWORD BytesPerSector = 0;
    DWORD FreeClusters = 0;
    DWORD TotalClusters = 0;

    tmppath[0] = *path;
    tmppath[1] = ':';
    tmppath[2] = '\\';
    tmppath[3] = '\0';
    GetDiskFreeSpace(tmppath, &SectorsPerCluster, &BytesPerSector,
                     &FreeClusters, &TotalClusters);
    return (long) (SectorsPerCluster * BytesPerSector * FreeClusters);
}

/*
 * Functions to get filenames using wildcards
 */
int
findfirst(char *path)
{
    if (ffhandle) {
        FindClose(ffhandle);
        ffhandle = (HANDLE) 0;
    }
    ffhandle = FindFirstFile(path, &ffd);
    return (ffhandle == INVALID_HANDLE_VALUE) ? 0 : 1;
}

int
findnext(void)
{
    return FindNextFile(ffhandle, &ffd) ? 1 : 0;
}

char *
foundfile_buffer(void)
{
    return &ffd.cFileName[0];
}

long
filesize(char *file)
{
    if (findfirst(file)) {
        return ((long) ffd.nFileSizeLow);
    } else
        return -1L;
}

/*
 * Chdrive() changes the default drive.
 */
void
chdrive(char *str)
{
    char *ptr;
    char drive;
    if ((ptr = strchr(str, ':')) != (char *) 0) {
        drive = toupper((uchar) *(ptr - 1));
        (void) _chdrive((drive - 'A') + 1);
    }
}

#if 0
static int
max_filename(void)
{
    DWORD maxflen;
    int status = 0;

    status = GetVolumeInformation((LPTSTR) 0, (LPTSTR) 0, 0, (LPDWORD) 0,
                                  &maxflen, (LPDWORD) 0, (LPTSTR) 0, 0);
    if (status)
        return maxflen;
    else
        return 0;
}
#endif

int
def_kbhit(void)
{
    return 0;
}

/*
 * Strip out troublesome file system characters.
 */

void nt_regularize(char* s) /* normalize file name */
{
    unsigned char *lp;

    for (lp = (unsigned char *) s; *lp; lp++)
        if (*lp == '?' || *lp == '"' || *lp == '\\' || *lp == '/'
            || *lp == '>' || *lp == '<' || *lp == '*' || *lp == '|'
            || *lp == ':' || (*lp > 127))
            *lp = '_';
}

/*
 * This is used in nhlan.c to implement some of the LAN_FEATURES.
 */
char *get_username(lan_username_size)
int *lan_username_size;
{
	static TCHAR username_buffer[BUFSZ];
	unsigned int status;
	DWORD i = BUFSZ - 1;

	/* i gets updated with actual size */
	status = GetUserName(username_buffer, &i);
	if (status) username_buffer[i] = '\0';
	else Strcpy(username_buffer, "NetHack");
	if (lan_username_size) *lan_username_size = strlen(username_buffer);
	return username_buffer;
}

# if 0
char *getxxx()
{
char     szFullPath[MAX_PATH] = "";
HMODULE  hInst = NULL;  	/* NULL gets the filename of this module */

GetModuleFileName(hInst, szFullPath, sizeof(szFullPath));
return &szFullPath[0];
}
# endif

#ifndef WIN32CON
/* fatal error */
/*VARARGS1*/
void
error
VA_DECL(const char *,s)
{
    char buf[BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);
    /* error() may get called before tty is initialized */
    if (iflags.window_inited) end_screen();
    if (!strncmpi(windowprocs.name, "tty", 3)) {
        buf[0] = '\n';
        (void) vsprintf(&buf[1], s, VA_ARGS);
        Strcat(buf, "\n");
        msmsg(buf);
    } else {
        (void) vsprintf(buf, s, VA_ARGS);
        Strcat(buf, "\n");
        raw_printf(buf);
    }
    exit(EXIT_FAILURE);
    VA_END();
}
#endif

void Delay(int ms)
{
	(void)Sleep(ms);
}

#ifdef WIN32CON
extern void backsp();
#endif

void win32_abort()
{
#ifdef WIZARD
   	if (wizard) {
# ifdef WIN32CON
	    int c, ci, ct;

   	    if (!iflags.window_inited)
		c = 'n';
		ct = 0;
		msmsg("Execute debug breakpoint wizard?");
		while ((ci=nhgetch()) != '\n') {
		    if (ct > 0) {
			backsp();       /* \b is visible on NT */
			(void) putchar(' ');
			backsp();
			ct = 0;
			c = 'n';
		    }
		    if (ci == 'y' || ci == 'n' || ci == 'Y' || ci == 'N') {
		    	ct = 1;
		        c = ci;
		        msmsg("%c",c);
		    }
		}
		if (c == 'y')
			DebugBreak();
# endif
	}
#endif
	abort();
}

static char interjection_buf[INTERJECTION_TYPES][1024];
static int interjection[INTERJECTION_TYPES];

void
interject_assistance(num, interjection_type, ptr1, ptr2)
int num;
int interjection_type;
genericptr_t ptr1;
genericptr_t ptr2;
{
	switch(num) {
	    case 1: {
		char *panicmsg = (char *)ptr1;
		char *datadir =  (char *)ptr2;
		char *tempdir = nh_getenv("TEMP");
		interjection_type = INTERJECT_PANIC;
		interjection[INTERJECT_PANIC] = 1;
		/*
		 * ptr1 = the panic message about to be delivered.
		 * ptr2 = the directory prefix of the dungeon file
		 *        that failed to open.
		 * Check to see if datadir matches tempdir or a
		 * common windows temp location. If it does, inform
		 * the user that they are probably trying to run the
		 * game from within their unzip utility, so the required
		 * files really don't exist at the location. Instruct
		 * them to unpack them first.
		 */
		if (panicmsg && datadir) {
		    if (!strncmpi(datadir, "C:\\WINDOWS\\TEMP", 15) ||
			    strstri(datadir, "TEMP")   ||
			    (tempdir && strstri(datadir, tempdir))) {
			(void)strncpy(interjection_buf[INTERJECT_PANIC],
			"\nOne common cause of this error is attempting to execute\n"
			"the game by double-clicking on it while it is displayed\n"
			"inside an unzip utility.\n\n"
			"You have to unzip the contents of the zip file into a\n"
			"folder on your system, and then run \"NetHack.exe\" or \n"
			"\"NetHackW.exe\" from there.\n\n"
			"If that is not the situation, you are encouraged to\n"
			"report the error as shown above.\n\n", 1023);
		    }
		}
	    }
	    break;
	}
}

void
interject(interjection_type)
int interjection_type;
{
	if (interjection_type >= 0 && interjection_type < INTERJECTION_TYPES)
		msmsg(interjection_buf[interjection_type]);
}
#endif /* WIN32 */

/*winnt.c*/
