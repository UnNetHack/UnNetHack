/* Copyright (C) 2001 by Alex Kompel <shurikk@pacbell.net> */
/* NetHack may be freely redistributed.  See license for details. */

#include "winMS.h"
#include "resource.h"
#include "mhsplash.h"
#include "mhmsg.h"
#include "mhfont.h"
#include "patchlevel.h"
#include "dlb.h"

#define LLEN 128

PNHWinApp GetNHApp(void);

BOOL CALLBACK NHSplashWndProc(HWND, UINT, WPARAM, LPARAM);

#define SPLASH_WIDTH		440
#define SPLASH_HEIGHT  322
#define SPLASH_VERSION_X		290
#define SPLASH_VERSION_Y		10
#define SPLASH_OFFSET_X		10
#define SPLASH_OFFSET_Y		10

extern HFONT version_splash_font;

void mswin_display_splash_window (BOOL show_ver)
{
	MSG msg;
	HWND mapWnd;
	int left, top;
	RECT splashrt;
	RECT clientrt;
	RECT controlrt;
	HWND hWnd;
	int buttop;
	int strsize = 0;
	int bufsize = BUFSZ;

	char *buf = malloc(bufsize);
	if (buf == NULL)
	    panic("out of memory");
	buf[0] = '\0';

	hWnd = CreateDialog(GetNHApp()->hApp, MAKEINTRESOURCE(IDD_SPLASH),
	    GetNHApp()->hMainWnd, NHSplashWndProc);
	if( !hWnd ) panic("Cannot create Splash window");
	mswin_init_splashfonts(hWnd);
	GetNHApp()->hPopupWnd = hWnd;
	mapWnd = mswin_hwnd_from_winid(WIN_MAP);
	if( !IsWindow(mapWnd) ) mapWnd = GetNHApp()->hMainWnd;
	/* Get control size */
	GetWindowRect (GetDlgItem(hWnd, IDOK), &controlrt);
	controlrt.right -= controlrt.left;
	controlrt.bottom -= controlrt.top;
	/* Get current client area */
	GetClientRect (hWnd, &clientrt);
	/* Get window size */
	GetWindowRect(hWnd, &splashrt);
	splashrt.right -= splashrt.left;
	splashrt.bottom -= splashrt.top;
	/* Get difference between requested client area and current value */
	splashrt.right += SPLASH_WIDTH + SPLASH_OFFSET_X * 2 - clientrt.right;
	splashrt.bottom += SPLASH_HEIGHT + controlrt.bottom + SPLASH_OFFSET_Y * 3 - clientrt.bottom;
	/* Place the window centered */
	/* On the screen, not on the parent window */
	left = (GetSystemMetrics(SM_CXSCREEN) - splashrt.right) / 2;
	top = (GetSystemMetrics(SM_CYSCREEN) - splashrt.bottom) / 2;
	MoveWindow(hWnd, left, top, splashrt.right, splashrt.bottom, TRUE);
	/* Place the OK control */
	GetClientRect (hWnd, &clientrt);
	MoveWindow (GetDlgItem(hWnd, IDOK),
	    (clientrt.right - clientrt.left - controlrt.right) / 2,
	    clientrt.bottom - controlrt.bottom - SPLASH_OFFSET_Y,
	    controlrt.right, controlrt.bottom, TRUE);
	buttop = clientrt.bottom - controlrt.bottom - SPLASH_OFFSET_Y;
	/* Place the text control */
	GetWindowRect (GetDlgItem(hWnd, IDC_EXTRAINFO), &controlrt);
	controlrt.right -= controlrt.left;
	controlrt.bottom -= controlrt.top;
	GetClientRect (hWnd, &clientrt);
	MoveWindow (GetDlgItem(hWnd, IDC_EXTRAINFO),
	    clientrt.left + SPLASH_OFFSET_X,
	    buttop - controlrt.bottom - SPLASH_OFFSET_Y,
	    clientrt.right - 2 * SPLASH_OFFSET_X, controlrt.bottom, TRUE);
	/* Fill the text control */
	Sprintf(buf, "%s\r\n%s\r\n%s\r\n\r\n", COPYRIGHT_BANNER_A, COPYRIGHT_BANNER_B,
	    COPYRIGHT_BANNER_C);
	strsize = strlen(buf);

	if (show_ver) {
	    /* Show complete version information */
	    dlb *f;

	    getversionstring(buf + strsize);
	    strcat(buf, "\r\n\r\n");
	    strsize = strlen(buf);

	    /* Add compile options */
	    f = dlb_fopen(OPTIONS_USED, RDTMODE);
	    if (f) {
		char line[LLEN + 1];

		while (dlb_fgets(line, LLEN, f)) {
		    size_t len;
		    len = strlen(line);
		    if (len > 0 && line[len - 1] == '\n') {
			line[len - 1] = '\r';
			line[len] = '\n';
			line[len + 1] = '\0';
			len++;
		    }
		    if (strsize + (int)len + 1 > bufsize)
		    {
			bufsize += BUFSZ;
			buf = realloc(buf, bufsize);
			if (buf == NULL)
			    panic("out of memory");
		    }
		    strcat(buf, line);
		    strsize += len;
		}
		(void) dlb_fclose(f);
	    }
	} else {
	    /* Show news, if any */
	    if (iflags.news) {
		FILE *nf;

		iflags.news = 0; /* prevent newgame() from re-displaying news */
		nf = fopen(NEWS, "r");
		if (nf != NULL) {
		    char line[LLEN + 1];

		    while (fgets(line, LLEN, nf)) {
			size_t len;
			len = strlen(line);
			if (len > 0 && line[len - 1] == '\n') {
			    line[len - 1] = '\r';
			    line[len] = '\n';
			    line[len + 1] = '\0';
			    len++;
			}
			if (strsize + (int)len + 1 > bufsize)
			{
			    bufsize += BUFSZ;
			    buf = realloc(buf, bufsize);
			    if (buf == NULL)
				panic("out of memory");
			}
			strcat(buf, line);
			strsize += len;
		    }
		    (void) fclose(nf);
		}
		else
		{
		    strcat(buf, "No news.");
		}
	    }
	}
	SetWindowText(GetDlgItem(hWnd, IDC_EXTRAINFO), buf);
	free(buf);
	ShowWindow(hWnd, SW_SHOW);

    while (IsWindow(hWnd) &&
	    GetMessage(&msg, NULL, 0, 0)!=0 ) {
	    if( !IsDialogMessage(hWnd, &msg) ) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}

	GetNHApp()->hPopupWnd = NULL;
	mswin_destroy_splashfonts();
}

BOOL CALLBACK NHSplashWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (message)
	{
	case WM_INITDIALOG:
	    /* set text control font */
		hdc = GetDC(hWnd);
		SendMessage(hWnd, WM_SETFONT,
			(WPARAM)mswin_get_font(NHW_TEXT, ATR_NONE, hdc, FALSE), 0);
		ReleaseDC(hWnd, hdc);

		SetFocus(GetDlgItem(hWnd, IDOK));
	return FALSE;

	case WM_PAINT:
	{
		char VersionString[BUFSZ];
		RECT rt;
		HDC hdcBitmap;
		HANDLE OldBitmap;
		HANDLE OldFont;
		PAINTSTRUCT ps;

		hdc = BeginPaint (hWnd, &ps);
		/* Show splash graphic */

		hdcBitmap = CreateCompatibleDC(hdc);
		SetBkMode (hdc, OPAQUE);
		OldBitmap = SelectObject(hdcBitmap, GetNHApp()->bmpSplash);
		nhapply_image_transparent(hdc, SPLASH_OFFSET_X, SPLASH_OFFSET_Y,
		    SPLASH_WIDTH, SPLASH_HEIGHT,
		    hdcBitmap, 0, 0, SPLASH_WIDTH, SPLASH_HEIGHT,
		    TILE_BK_COLOR);

		SelectObject (hdcBitmap, OldBitmap);
		DeleteDC (hdcBitmap);

		SetBkMode (hdc, TRANSPARENT);
		/* Print version number */

		SetTextColor (hdc, RGB(0, 0, 0));
		rt.right = rt.left = SPLASH_VERSION_X;
		rt.bottom = rt.top = SPLASH_VERSION_Y;
		Sprintf (VersionString, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR,
		    PATCHLEVEL);
		OldFont = SelectObject(hdc, version_splash_font);
		DrawText (hdc, VersionString, strlen(VersionString), &rt,
		    DT_LEFT | DT_NOPREFIX | DT_CALCRECT);
		DrawText (hdc, VersionString, strlen(VersionString), &rt,
		    DT_LEFT | DT_NOPREFIX);

		EndPaint (hWnd, &ps);
	}
	break;

	case WM_COMMAND:
	switch (LOWORD(wParam))
        {
		case IDOK:
			mswin_window_mark_dead(mswin_winid_from_handle(hWnd));
			if( GetNHApp()->hMainWnd==hWnd )
				GetNHApp()->hMainWnd=NULL;
			DestroyWindow(hWnd);
			SetFocus(GetNHApp()->hMainWnd);
			return TRUE;
		}
	break;
	}
	return FALSE;
}
