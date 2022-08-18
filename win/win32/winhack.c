/* Copyright (C) 2001 by Alex Kompel <shurikk@pacbell.net> */
/* NetHack may be freely redistributed.  See license for details. */

// winhack.cpp : Defines the entry point for the application.
//

#include <process.h>
#include "winMS.h"
#include "hack.h"
#include "dlb.h"
#include "resource.h"
#include "mhmain.h"
#include "mhmap.h"

#if !defined(__BORLANDC__) && !defined(__MINGW32__)
#include <shlwapi.h>
#else /* Borland redefines "boolean" in shlwapi.h so just use the little bit we need */
typedef struct _DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

//
// The caller should always GetProcAddress("DllGetVersion"), not
// implicitly link to it.
//

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

#endif

#define SHARED_DCL

/* Minimal common control library version
Version     _WIN_32IE   Platform/IE
=======     =========   ===========
4.00        0x0200      Microsoft(r) Windows 95/Windows NT 4.0
4.70        0x0300      Microsoft(r) Internet Explorer 3.x
4.71        0x0400      Microsoft(r) Internet Explorer 4.0
4.72        0x0401      Microsoft(r) Internet Explorer 4.01
...and probably going on infinitely...
*/
#define MIN_COMCTLMAJOR 4
#define MIN_COMCTLMINOR 71
#define INSTALL_NOTES "http://www.nethack.org/v340/ports/download-win.html#cc"
/*#define COMCTL_URL "http://www.microsoft.com/msdownload/ieplatform/ie/comctrlx86.asp"*/

extern void nethack_exit(int);
static TCHAR* _get_cmd_arg(TCHAR* pCmdLine);
static HRESULT GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor);


// Global Variables:
NHWinApp _nethack_app;

#ifdef __BORLANDC__
#define _stricmp(s1,s2)     stricmp(s1,s2)
#define _strdup(s1)         strdup(s1)
#endif

// Foward declarations of functions included in this code module:
extern void pcmain(int,char **);
static void __cdecl mswin_moveloop(void *);

#define MAX_CMDLINE_PARAM 255

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	INITCOMMONCONTROLSEX InitCtrls;
	int argc;
	char* argv[MAX_CMDLINE_PARAM];
	size_t len;
	TCHAR *p;
	TCHAR wbuf[BUFSZ];
	char buf[BUFSZ];
    DWORD major, minor;
    boolean resuming = FALSE; /* assume new game */

	/* ensure that we don't access violate on a panic() */
	windowprocs.win_raw_print = mswin_raw_print;
	windowprocs.win_raw_print_bold = mswin_raw_print_bold;

	/* init applicatio structure */
	_nethack_app.hApp = hInstance;
	_nethack_app.hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_NETHACKW);
	_nethack_app.hMainWnd = NULL;
	_nethack_app.hPopupWnd = NULL;
	_nethack_app.bmpTiles = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TILES));
	if( _nethack_app.bmpTiles==NULL ) panic("cannot load tiles bitmap");
	_nethack_app.bmpPetMark = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_PETMARK));
	if( _nethack_app.bmpPetMark==NULL ) panic("cannot load pet mark bitmap");
	_nethack_app.bmpRip = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RIP));
	if ( _nethack_app.bmpRip == NULL ) panic("cannot load rip bitmap");
	_nethack_app.bmpSplash = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
	if ( _nethack_app.bmpSplash == NULL ) panic("cannot load splash bitmap");
	_nethack_app.bmpMapTiles = _nethack_app.bmpTiles;
	_nethack_app.mapTile_X = TILE_X;
	_nethack_app.mapTile_Y = TILE_Y;
	_nethack_app.mapTilesPerLine = TILES_PER_LINE;

	_nethack_app.bNoHScroll = FALSE;
	_nethack_app.bNoVScroll = FALSE;
	_nethack_app.saved_text = strdup("");

    // init controls
    if (FAILED(GetComCtlVersion(&major, &minor)))
    {
        char buf[TBUFSZ];
        Sprintf(buf, "Cannot load common control library.\n%s\n%s",
              "For further information, refer to the installation notes at",
		INSTALL_NOTES);
        panic(buf);
    }
    if (major < MIN_COMCTLMAJOR
        || (major == MIN_COMCTLMAJOR && minor < MIN_COMCTLMINOR))
    {
        char buf[TBUFSZ];
        Sprintf(buf, "Common control library is outdated.\n%s %d.%d\n%s\n%s",
              "NetHack requires at least version ",
	      MIN_COMCTLMAJOR, MIN_COMCTLMINOR,
	      "For further information, refer to the installation notes at",
		INSTALL_NOTES);
        panic(buf);
    }
	ZeroMemory(&InitCtrls, sizeof(InitCtrls));
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	/* get command line parameters */
	p = _get_cmd_arg(GetCommandLine());
	p = _get_cmd_arg(NULL); /* skip first paramter - command name */
	for( argc = 1; p && argc<MAX_CMDLINE_PARAM; argc++ ) {
		len = _tcslen(p);
		if( len>0 ) {
			argv[argc] = _strdup( NH_W2A(p, buf, BUFSZ) );
		} else {
			argv[argc] = "";
		}
		p = _get_cmd_arg(NULL);
	}
	GetModuleFileName(NULL, wbuf, BUFSZ);
	argv[0] = _strdup(NH_W2A(wbuf, buf, BUFSZ));

    if (argc == 2) {
	    TCHAR *savefile = strdup(argv[1]);
	    TCHAR *plname;
        for (p = savefile; *p && *p != '-'; p++)
            ;
        if (*p) {
            /* we found a '-' */
            plname = p + 1;
            for (p = plname; *p && *p != '.'; p++)
                ;
            if (*p) {
                if (strcmp(p + 1, "NetHack-saved-game") == 0) {
                    *p = '\0';
                    argv[1] = "-u";
                    argv[2] = _strdup(plname);
                    argc = 3;
                }
            }
        }
        free(savefile);
    }
	pcmain(argc,argv);

	moveloop(resuming);

	return 0;
}


PNHWinApp GetNHApp()
{
	return &_nethack_app;
}

TCHAR* _get_cmd_arg(TCHAR* pCmdLine)
{
        static TCHAR* pArgs = NULL;
        TCHAR  *pRetArg;
        BOOL   bQuoted;

        if( !pCmdLine && !pArgs ) return NULL;
        if( !pArgs ) pArgs = pCmdLine;

        /* skip whitespace */
        for(pRetArg = pArgs; *pRetArg && _istspace(*pRetArg); pRetArg = CharNext(pRetArg));
		if( !*pRetArg ) {
			pArgs = NULL;
			return NULL;
		}

        /* check for quote */
        if( *pRetArg==TEXT('"') ) {
                bQuoted = TRUE;
                pRetArg = CharNext(pRetArg);
				pArgs = _tcschr(pRetArg, TEXT('"'));
	    } else {
			/* skip to whitespace */
			for(pArgs = pRetArg; *pArgs && !_istspace(*pArgs); pArgs = CharNext(pArgs));
		}

		if( pArgs && *pArgs ) {
			TCHAR* p;
			p = pArgs;
			pArgs = CharNext(pArgs);
			*p = (TCHAR)0;
		} else {
			pArgs = NULL;
		}

		return pRetArg;
}

/* Get the version of the Common Control library on this machine.
   Copied from the Microsoft SDK
 */
HRESULT GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	HINSTANCE   hComCtl;
	HRESULT           hr = S_OK;
	DLLGETVERSIONPROC pDllGetVersion;

	if(IsBadWritePtr(pdwMajor, sizeof(DWORD)) ||
	   IsBadWritePtr(pdwMinor, sizeof(DWORD)))
		return E_INVALIDARG;
	//load the DLL
	hComCtl = LoadLibrary(TEXT("comctl32.dll"));
	if (!hComCtl) return E_FAIL;

	/*
	You must get this function explicitly because earlier versions of the DLL
	don't implement this function. That makes the lack of implementation of the
	function a version marker in itself.
	*/
	pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hComCtl, TEXT("DllGetVersion"));
	if(pDllGetVersion) {
		DLLVERSIONINFO    dvi;
		ZeroMemory(&dvi, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);
		hr = (*pDllGetVersion)(&dvi);
		if(SUCCEEDED(hr)) {
			*pdwMajor = dvi.dwMajorVersion;
			*pdwMinor = dvi.dwMinorVersion;
                } else {
			hr = E_FAIL;
                }
        } else {
		/*
		If GetProcAddress failed, then the DLL is a version previous to the one
		shipped with IE 3.x.
		*/
		*pdwMajor = 4;
		*pdwMinor = 0;
	}
	FreeLibrary(hComCtl);
	return hr;
}


