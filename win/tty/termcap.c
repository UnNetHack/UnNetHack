/*	SCCS Id: @(#)termcap.c	3.4	2000/07/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#if defined (TTY_GRAPHICS) && !defined(NO_TERMS)

#include "wintty.h"

#include "tcap.h"


#ifdef MICROPORT_286_BUG
#define Tgetstr(key) (tgetstr(key,tbuf))
#else
#define Tgetstr(key) (tgetstr(key,&tbufptr))
#endif /* MICROPORT_286_BUG **/

static char * FDECL(s_atr2str, (int));
static char * FDECL(e_atr2str, (int));

void FDECL(cmov, (int, int));
void FDECL(nocmov, (int, int));
#if defined(TEXTCOLOR) && defined(TERMLIB)
# ifdef OVLB
#  if !defined(UNIX) || !defined(TERMINFO)
static void FDECL(analyze_seq, (char *, int *, int *));
#  endif
static void NDECL(init_hilite);
static void NDECL(kill_hilite);
# endif /* OVLB */
#endif

#ifdef OVLB
	/* (see tcap.h) -- nh_CM, nh_ND, nh_CD, nh_HI,nh_HE, nh_US,nh_UE,
				ul_hack */
struct tc_lcl_data tc_lcl_data = { 0, 0, 0, 0,0, 0,0, FALSE };
#endif /* OVLB */

STATIC_VAR char *HO, *CL, *CE, *UP, *XD, *BC, *SO, *SE, *TI, *TE;
STATIC_VAR char *VS, *VE;
STATIC_VAR char *ME;
STATIC_VAR char *MR;
#if 0
STATIC_VAR char *MB, *MH;
STATIC_VAR char *MD;     /* may already be in use below */
#endif
#ifdef TERMLIB
# ifdef TEXTCOLOR
STATIC_VAR char *MD;
# endif
STATIC_VAR int SG;
#ifdef OVLB
STATIC_OVL char PC = '\0';
#else /* OVLB */
STATIC_DCL char PC;
#endif /* OVLB */
STATIC_VAR char tbuf[512];
#endif

#ifdef TEXTCOLOR
char NEARDATA *hilites[CLR_MAX]; /* terminal escapes for the various colors */
#endif

#ifdef OVLB
static char *KS = (char *)0, *KE = (char *)0;	/* keypad sequences */
static char nullstr[] = "";
#endif /* OVLB */

#if defined(ASCIIGRAPH) && !defined(NO_TERMS)
extern boolean HE_resets_AS;
#endif

#ifndef TERMLIB
STATIC_VAR char tgotobuf[20];
#define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+1, x+1), tgotobuf)
#endif /* TERMLIB */

#ifndef MSDOS

STATIC_DCL void NDECL(init_ttycolor);

boolean colorflag = FALSE;			/* colors are initialized */
int ttycolors[CLR_MAX];

void
init_ttycolor()
{
	if (!colorflag) {
		ttycolors[CLR_RED]		= CLR_RED;
		ttycolors[CLR_GREEN]		= CLR_GREEN;
		ttycolors[CLR_BROWN]		= CLR_BROWN;
		ttycolors[CLR_BLUE]		= CLR_BLUE;
		ttycolors[CLR_MAGENTA]		= CLR_MAGENTA;
		ttycolors[CLR_CYAN]		= CLR_CYAN;
		ttycolors[CLR_GRAY]		= CLR_GRAY;
		if (iflags.wc2_newcolors) {
			ttycolors[CLR_BLACK]	= CLR_BLACK;
			defsyms[S_corr].color	= CLR_BLACK;
			defsyms[S_dnstair].color= CLR_WHITE;
			defsyms[S_upstair].color= CLR_WHITE;
		} else {
			ttycolors[CLR_BLACK]	= CLR_BLUE;
		}
		ttycolors[CLR_ORANGE]		= CLR_ORANGE;
		ttycolors[CLR_BRIGHT_GREEN]	= CLR_BRIGHT_GREEN;
		ttycolors[CLR_YELLOW]		= CLR_YELLOW;
		ttycolors[CLR_BRIGHT_BLUE]	= CLR_BRIGHT_BLUE;
		ttycolors[CLR_BRIGHT_MAGENTA]	= CLR_BRIGHT_MAGENTA;
		ttycolors[CLR_BRIGHT_CYAN]	= CLR_BRIGHT_CYAN;
		ttycolors[CLR_WHITE]		= CLR_WHITE;
	}
}

static int FDECL(convert_uchars,(char *, uchar *, int));

#ifdef VIDEOSHADES
/*
 * OPTIONS=videocolors:1-2-3-4-5-6-7-8-9-10-11-12-13-14-15
 * Left to right assignments for:
 *	red	green	 brown	blue	magenta	cyan	gray	black
 *	orange	br.green yellow	br.blue	br.mag	br.cyan	white
 */
int assign_videocolors(char *colorvals)
{
	int i,icolor;
	uchar *tmpcolor;

	init_ttycolor();

	i = strlen(colorvals);
	tmpcolor = (uchar *)alloc(i);
	if (convert_uchars(colorvals,tmpcolor,i) < 0) return FALSE;

	icolor = CLR_RED;
	for( i = 0; tmpcolor[i] != 0; ++i) {
	    if (icolor <= CLR_WHITE)
		ttycolors[icolor++] = tmpcolor[i];
	}

	colorflag = TRUE;
	free((genericptr_t)tmpcolor);
	return 1;
}
#endif

static int
convert_uchars(bufp,list,size)
    char *bufp; 	/* current pointer */
    uchar *list;	/* return list */
    int size;
{
    unsigned int num = 0;
    int count = 0;

    list[count] = 0;

    while (1) {
	switch(*bufp) {
	    case ' ':  case '\0':
	    case '\t': case '-':
	    case '\n':
		if (num) {
		    list[count++] = num;
		    list[count] = 0;
		    num = 0;
		}
		if ((count==size) || !*bufp) return count;
		bufp++;
		break;
	    case '#': 
		if (num) {
		    list[count++] = num;
		    list[count] = 0;
		}
		return count;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	    case '8': case '9':
		num = num*10 + (*bufp-'0');
		if (num > 15) return -1;
		bufp++;
		break;
	    default: return -1;
	}
    }
    /*NOTREACHED*/
}
#endif /* !MSDOS */

#ifdef OVLB

void
tty_startup(wid, hgt)
int *wid, *hgt;
{
	register int i;
#ifdef TERMLIB
	register const char *term;
	register char *tptr;
	char *tbufptr, *pc;
#endif

#ifdef TEXTCOLOR
# ifndef MSDOS
	init_ttycolor();
# endif
#endif

#ifdef TERMLIB

		term = getenv("TERM");

	if (!term)
#endif
#ifndef ANSI_DEFAULT
		error("Can't get TERM.");
#else
	{
#  ifdef MICRO
		get_scr_size();
#   ifdef CLIPPING
		if(CO < COLNO || LI < ROWNO+3)
			setclipped();
#   endif
#  endif
		HO = "\033[H";
/*		nh_CD = "\033[J"; */
		CE = "\033[K";		/* the ANSI termcap */
#  ifndef TERMLIB
		nh_CM = "\033[%d;%dH";
#  else
		nh_CM = "\033[%i%d;%dH";
#  endif
		UP = "\033[A";
		nh_ND = "\033[C";
		XD = "\033[B";
#  ifdef MICRO	/* backspaces are non-destructive */
		BC = "\b";
#  else
		BC = "\033[D";
#  endif
		nh_HI = SO = "\033[1m";
		nh_US = "\033[4m";
		MR = "\033[7m";
		TI = nh_HE = ME = SE = nh_UE = "\033[0m";
		/* strictly, SE should be 2, and nh_UE should be 24,
		   but we can't trust all ANSI emulators to be
		   that complete.  -3. */
#  ifndef MICRO
		AS = "\016";
		AE = "\017";
#  endif
		TE = VS = VE = nullstr;
#  ifdef TEXTCOLOR
		for (i = 0; i < CLR_MAX / 2; i++)
		    if (i != CLR_BLACK) {
			hilites[i|BRIGHT] = (char *) alloc(sizeof("\033[1;3%dm"));
			Sprintf(hilites[i|BRIGHT], "\033[1;3%dm", i);
			if (iflags.wc2_newcolors || (i != CLR_GRAY))
#   ifdef MICRO
			    if (i == CLR_BLUE) hilites[CLR_BLUE] = hilites[CLR_BLUE|BRIGHT];
			    else
#   endif
			    {
				hilites[i] = (char *) alloc(sizeof("\033[0;3%dm"));
				Sprintf(hilites[i], "\033[0;3%dm", i);
			    }
		    }
#  endif
		*wid = CO;
		*hgt = LI;
		CL = "\033[2J";		/* last thing set */
		return;
	}
#endif /* ANSI_DEFAULT */

#ifdef TERMLIB
	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if(!strncmp(term, "5620", 4))
		flags.null = FALSE;	/* this should be a termcap flag */
	if(tgetent(tptr, term) < 1) {
		char buf[BUFSZ];
		(void) strncpy(buf, term,
				(BUFSZ - 1) - (sizeof("Unknown terminal type: .  ")));
		buf[BUFSZ-1] = '\0';
		error("Unknown terminal type: %s.", term);
	}
	if ((pc = Tgetstr("pc")) != 0)
		PC = *pc;

	if(!(BC = Tgetstr("le")))	/* both termcap and terminfo use le */
# ifdef TERMINFO
	    error("Terminal must backspace.");
# else
	    if(!(BC = Tgetstr("bc"))) {	/* termcap also uses bc/bs */
#  ifndef MINIMAL_TERM
		if(!tgetflag("bs"))
			error("Terminal must backspace.");
#  endif
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	    }
# endif

# ifdef MINIMAL_TERM
	HO = (char *)0;
# else
	HO = Tgetstr("ho");
# endif
	/*
	 * LI and CO are set in ioctl.c via a TIOCGWINSZ if available.  If
	 * the kernel has values for either we should use them rather than
	 * the values from TERMCAP ...
	 */
# ifndef MICRO
	if (!CO) CO = tgetnum("co");
	if (!LI) LI = tgetnum("li");
# else
		CO = tgetnum("co");
		LI = tgetnum("li");
		if (!LI || !CO)			/* if we don't override it */
			get_scr_size();
# endif
# ifdef CLIPPING
	if(CO < COLNO || LI < ROWNO+3)
		setclipped();
# endif
	nh_ND = Tgetstr("nd");
	if(tgetflag("os"))
		error("UnNetHack can't have OS.");
	if(tgetflag("ul"))
		ul_hack = TRUE;
	CE = Tgetstr("ce");
	UP = Tgetstr("up");
	/* It seems that xd is no longer supported, and we should use
	   a linefeed instead; unfortunately this requires resetting
	   CRMOD, and many output routines will have to be modified
	   slightly. Let's leave that till the next release. */
	XD = Tgetstr("xd");
/* not:		XD = Tgetstr("do"); */
	if(!(nh_CM = Tgetstr("cm"))) {
	    if(!UP && !HO)
		error("UnNetHack needs CM or UP or HO.");
	    tty_raw_print("Playing UnNetHack on terminals without CM is suspect.");
	    tty_wait_synch();
	}
	SO = Tgetstr("so");
	SE = Tgetstr("se");
	nh_US = Tgetstr("us");
	nh_UE = Tgetstr("ue");
	SG = tgetnum("sg");	/* -1: not fnd; else # of spaces left by so */
	if(!SO || !SE || (SG > 0)) SO = SE = nh_US = nh_UE = nullstr;
	TI = Tgetstr("ti");
	TE = Tgetstr("te");
	VS = VE = nullstr;
# ifdef TERMINFO
	VS = Tgetstr("eA");	/* enable graphics */
# endif
	KS = Tgetstr("ks");	/* keypad start (special mode) */
	KE = Tgetstr("ke");	/* keypad end (ordinary mode [ie, digits]) */
	MR = Tgetstr("mr");	/* reverse */
# if 0
	MB = Tgetstr("mb");	/* blink */
	MD = Tgetstr("md");	/* boldface */
	MH = Tgetstr("mh");	/* dim */
# endif
	ME = Tgetstr("me");	/* turn off all attributes */
	if (!ME || (SE == nullstr)) ME = SE;	/* default to SE value */

	/* Get rid of padding numbers for nh_HI and nh_HE.  Hope they
	 * aren't really needed!!!  nh_HI and nh_HE are outputted to the
	 * pager as a string - so how can you send it NULs???
	 *  -jsb
	 */
	nh_HI = (char *) alloc((unsigned)(strlen(SO)+1));
	nh_HE = (char *) alloc((unsigned)(strlen(ME)+1));
	i = 0;
	while (digit(SO[i])) i++;
	Strcpy(nh_HI, &SO[i]);
	i = 0;
	while (digit(ME[i])) i++;
	Strcpy(nh_HE, &ME[i]);
	AS = Tgetstr("as");
	AE = Tgetstr("ae");
	nh_CD = Tgetstr("cd");
# ifdef TEXTCOLOR
	MD = Tgetstr("md");
# endif
# ifdef TEXTCOLOR
	init_hilite();
# endif
	*wid = CO;
	*hgt = LI;
	if (!(CL = Tgetstr("cl")))	/* last thing set */
		error("UnNetHack needs CL.");
	if ((int)(tbufptr - tbuf) > (int)(sizeof tbuf))
		error("TERMCAP entry too big...\n");
	free((genericptr_t)tptr);
#endif /* TERMLIB */
}

/* note: at present, this routine is not part of the formal window interface */
/* deallocate resources prior to final termination */
void
tty_shutdown()
{
#if defined(TEXTCOLOR) && defined(TERMLIB)
	kill_hilite();
#endif
	/* we don't attempt to clean up individual termcap variables [yet?] */
	return;
}

void
tty_number_pad(state)
int state;
{
	switch (state) {
	    case -1:	/* activate keypad mode (escape sequences) */
		    if (KS && *KS) xputs(KS);
		    break;
	    case  1:	/* activate numeric mode for keypad (digits) */
		    if (KE && *KE) xputs(KE);
		    break;
	    case  0:	/* don't need to do anything--leave terminal as-is */
	    default:
		    break;
	}
}

#ifdef TERMLIB
extern void NDECL((*decgraphics_mode_callback));    /* defined in drawing.c */
static void NDECL(tty_decgraphics_termcap_fixup);

/*
   We call this routine whenever DECgraphics mode is enabled, even if it
   has been previously set, in case the user manages to reset the fonts.
   The actual termcap fixup only needs to be done once, but we can't
   call xputs() from the option setting or graphics assigning routines,
   so this is a convenient hook.
 */
static void
tty_decgraphics_termcap_fixup()
{
	static char ctrlN[]   = "\016";
	static char ctrlO[]   = "\017";
	static char appMode[] = "\033=";
	static char numMode[] = "\033>";

	/* these values are missing from some termcaps */
	if (!AS) AS = ctrlN;	/* ^N (shift-out [graphics font]) */
	if (!AE) AE = ctrlO;	/* ^O (shift-in  [regular font])  */
	if (!KS) KS = appMode;	/* ESC= (application keypad mode) */
	if (!KE) KE = numMode;	/* ESC> (numeric keypad mode)	  */
	/*
	 * Select the line-drawing character set as the alternate font.
	 * Do not select NA ASCII as the primary font since people may
	 * reasonably be using the UK character set.
	 */
	if (iflags.DECgraphics) xputs("\033)0");

#if defined(ASCIIGRAPH) && !defined(NO_TERMS)
	/* some termcaps suffer from the bizarre notion that resetting
	   video attributes should also reset the chosen character set */
    {
	const char *nh_he = nh_HE, *ae = AE;
	int he_limit, ae_length;

	if (digit(*ae)) {	/* skip over delay prefix, if any */
	    do ++ae; while (digit(*ae));
	    if (*ae == '.') { ++ae; if (digit(*ae)) ++ae; }
	    if (*ae == '*') ++ae;
	}
	/* can't use nethack's case-insensitive strstri() here, and some old
	   systems don't have strstr(), so use brute force substring search */
	ae_length = strlen(ae), he_limit = strlen(nh_he);
	while (he_limit >= ae_length) {
	    if (strncmp(nh_he, ae, ae_length) == 0) {
		HE_resets_AS = TRUE;
		break;
	    }
	    ++nh_he, --he_limit;
	}
    }
#endif
}
#endif	/* TERMLIB */

void
tty_start_screen()
{
	xputs(TI);
	xputs(VS);

#ifdef TERMLIB
	if (iflags.DECgraphics) tty_decgraphics_termcap_fixup();
	/* set up callback in case option is not set yet but toggled later */
	decgraphics_mode_callback = tty_decgraphics_termcap_fixup;
#endif
	if (iflags.num_pad) tty_number_pad(1);	/* make keypad send digits */
}

void
tty_end_screen()
{
	clear_screen();
	xputs(VE);
	xputs(TE);
}

/* Cursor movements */

#endif /* OVLB */

#ifdef OVL0
/* Note to OVLx tinkerers.  The placement of this overlay controls the location
   of the function xputc().  This function is not currently in trampoli.[ch]
   files for what is deemed to be performance reasons.  If this define is moved
   and or xputc() is taken out of the ROOT overlay, then action must be taken
   in trampoli.[ch]. */

void
nocmov(x, y)
int x,y;
{
	if ((int) ttyDisplay->cury > y) {
		if(UP) {
			while ((int) ttyDisplay->cury > y) {	/* Go up. */
				xputs(UP);
				ttyDisplay->cury--;
			}
		} else if(nh_CM) {
			cmov(x, y);
		} else if(HO) {
			home();
			tty_curs(BASE_WINDOW, x+1, y);
		} /* else impossible("..."); */
	} else if ((int) ttyDisplay->cury < y) {
		if(XD) {
			while((int) ttyDisplay->cury < y) {
				xputs(XD);
				ttyDisplay->cury++;
			}
		} else if(nh_CM) {
			cmov(x, y);
		} else {
			while((int) ttyDisplay->cury < y) {
				xputc('\n');
				ttyDisplay->curx = 0;
				ttyDisplay->cury++;
			}
		}
	}
	if ((int) ttyDisplay->curx < x) {		/* Go to the right. */
		if(!nh_ND) cmov(x, y); else	/* bah */
			/* should instead print what is there already */
		while ((int) ttyDisplay->curx < x) {
			xputs(nh_ND);
			ttyDisplay->curx++;
		}
	} else if ((int) ttyDisplay->curx > x) {
		while ((int) ttyDisplay->curx > x) {	/* Go to the left. */
			xputs(BC);
			ttyDisplay->curx--;
		}
	}
}

void
cmov(x, y)
register int x, y;
{
	xputs(tgoto(nh_CM, x, y));
	ttyDisplay->cury = y;
	ttyDisplay->curx = x;
}

/* See note at OVLx ifdef above.   xputc() is a special function. */
void
xputc(c)
#if defined(apollo)
int c;
#else
char c;
#endif
{
	(void) putchar(c);
}

void
xputs(s)
const char *s;
{
# ifndef TERMLIB
	(void) fputs(s, stdout);
# else
#  if defined(NHSTDC)
	tputs(s, 1, (int (*)())xputc);
#  else
	tputs(s, 1, xputc);
#  endif
# endif
}

void
cl_end()
{
	if(CE)
		xputs(CE);
	else {	/* no-CE fix - free after Harold Rynes */
		/* this looks terrible, especially on a slow terminal
		   but is better than nothing */
		register int cx = ttyDisplay->curx+1;

		while(cx < CO) {
			xputc(' ');
			cx++;
		}
		tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
	}
}

#endif /* OVL0 */
#ifdef OVLB

void
clear_screen()
{
	/* note: if CL is null, then termcap initialization failed,
		so don't attempt screen-oriented I/O during final cleanup.
	 */
	if (CL) {
		xputs(CL);
		home();
	}
}

#endif /* OVLB */
#ifdef OVL0

void
home()
{
	if(HO)
		xputs(HO);
	else if(nh_CM)
		xputs(tgoto(nh_CM, 0, 0));
	else
		tty_curs(BASE_WINDOW, 1, 0);	/* using UP ... */
	ttyDisplay->curx = ttyDisplay->cury = 0;
}

void
standoutbeg()
{
	if(SO) xputs(SO);
}

void
standoutend()
{
	if(SE) xputs(SE);
}

#if 0	/* if you need one of these, uncomment it (here and in extern.h) */
void
revbeg()
{
	if(MR) xputs(MR);
}

void
boldbeg()
{
	if(MD) xputs(MD);
}

void
blinkbeg()
{
	if(MB) xputs(MB);
}

void
dimbeg()
/* not in most termcap entries */
{
	if(MH) xputs(MH);
}

void
m_end()
{
	if(ME) xputs(ME);
}
#endif

#endif /* OVL0 */
#ifdef OVLB

void
backsp()
{
	xputs(BC);
}

void
tty_nhbell()
{
	if (flags.silent) return;
	(void) putchar('\007');		/* curx does not change */
	(void) fflush(stdout);
}

#endif /* OVLB */
#ifdef OVL0

#ifdef ASCIIGRAPH
void
graph_on() {
	if (AS) xputs(AS);
}

void
graph_off() {
	if (AE) xputs(AE);
}
#endif

#endif /* OVL0 */
#ifdef OVL1

#if !defined(MICRO)
static const short tmspc10[] = {		/* from termcap */
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5
};
#endif

/* delay 50 ms */
void
tty_delay_output()
{
#if defined(MICRO)
	register int i;
#endif
#ifdef TIMED_DELAY
	if (flags.nap) {
		(void) fflush(stdout);
		msleep(50);		/* sleep for 50 milliseconds */
		return;
	}
#endif
#if defined(MICRO)
	/* simulate the delay with "cursor here" */
	for (i = 0; i < 3; i++) {
		cmov(ttyDisplay->curx, ttyDisplay->cury);
		(void) fflush(stdout);
	}
#else /* MICRO */
	/* BUG: if the padding character is visible, as it is on the 5620
	   then this looks terrible. */
	if(flags.null)
# ifdef TERMINFO
		/* cbosgd!cbcephus!pds for SYS V R2 */
#  ifdef NHSTDC
		tputs("$<50>", 1, (int (*)())xputc);
#  else
		tputs("$<50>", 1, xputc);
#  endif
# else
#  if defined(NHSTDC)
		tputs("50", 1, (int (*)())xputc);
#  else
		tputs("50", 1, xputc);
#  endif
# endif

	else if(ospeed > 0 && ospeed < SIZE(tmspc10) && nh_CM) {
		/* delay by sending cm(here) an appropriate number of times */
		register int cmlen = strlen(tgoto(nh_CM, ttyDisplay->curx,
							ttyDisplay->cury));
		register int i = 500 + tmspc10[ospeed]/2;

		while(i > 0) {
			cmov((int)ttyDisplay->curx, (int)ttyDisplay->cury);
			i -= cmlen*tmspc10[ospeed];
		}
	}
#endif /* MICRO */
}

#endif /* OVL1 */
#ifdef OVLB

void
cl_eos()			/* free after Robert Viduya */
{				/* must only be called with curx = 1 */

	if(nh_CD)
		xputs(nh_CD);
	else {
		register int cy = ttyDisplay->cury+1;
		while(cy <= LI-2) {
			cl_end();
			xputc('\n');
			cy++;
		}
		cl_end();
		tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
	}
}

#if defined(TEXTCOLOR) && defined(TERMLIB)
# if defined(UNIX) && defined(TERMINFO)
/*
 * Sets up color highlighting, using terminfo(4) escape sequences.
 *
 * Having never seen a terminfo system without curses, we assume this
 * inclusion is safe.  On systems with color terminfo, it should define
 * the 8 COLOR_FOOs, and avoid us having to guess whether this particular
 * terminfo uses BGR or RGB for its indexes.
 *
 * If we don't get the definitions, then guess.  Original color terminfos
 * used BGR for the original Sf (setf, Standard foreground) codes, but
 * there was a near-total lack of user documentation, so some subsequent
 * terminfos, such as early Linux ncurses and SCO UNIX, used RGB.  Possibly
 * as a result of the confusion, AF (setaf, ANSI Foreground) codes were
 * introduced, but this caused yet more confusion.  Later Linux ncurses
 * have BGR Sf, RGB AF, and RGB COLOR_FOO, which appears to be the SVR4
 * standard.  We could switch the colors around when using Sf with ncurses,
 * which would help things on later ncurses and hurt things on early ncurses.
 * We'll try just preferring AF and hoping it always agrees with COLOR_FOO,
 * and falling back to Sf if AF isn't defined.
 *
 * In any case, treat black specially so we don't try to display black
 * characters on the assumed black background.
 */

	/* `curses' is aptly named; various versions don't like these
	    macros used elsewhere within nethack; fortunately they're
	    not needed beyond this point, so we don't need to worry
	    about reconstructing them after the header file inclusion. */
#undef delay_output
#undef TRUE
#undef FALSE
#define m_move curses_m_move	/* Some curses.h decl m_move(), not used here */

#include <curses.h>

#ifndef HAVE_TPARM
extern char *tparm();
#endif

#  ifdef COLOR_BLACK	/* trust include file */
#ifndef VIDEOSHADES
#undef COLOR_BLACK
#endif
#  else
#ifdef VIDEOSHADES
#define COLOR_BLACK   0
#endif
#define COLOR_BLUE    1
#define COLOR_GREEN   2
#define COLOR_CYAN    3
#define COLOR_RED     4
#define COLOR_MAGENTA 5
#define COLOR_YELLOW  6
#define COLOR_WHITE   7
#  endif
#ifndef VIDEOSHADES
#define COLOR_BLACK COLOR_BLUE
#endif

const int ti_map[8] = {
	COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE };

static void
init_hilite()
{
	register int c;
	char *setf, *scratch;

	for (c = 0; c < SIZE(hilites); c++)
		hilites[c] = nh_HI;
	hilites[CLR_GRAY] = hilites[NO_COLOR] = (char *)0;

	if (tgetnum("Co") < 8
	    || ((setf = tgetstr("AF", (char **)0)) == (char *)0
		 && (setf = tgetstr("Sf", (char **)0)) == (char *)0))
		return;

	for (c = 0; c < CLR_MAX / 2; c++) {
	    scratch = tparm(setf, ti_map[c]);
	    if (iflags.wc2_newcolors || (c != CLR_GRAY)) {
		hilites[c] = (char *) alloc(strlen(scratch) + 1);
		Strcpy(hilites[c], scratch);
	    }
	    if (c != CLR_BLACK) {
		hilites[c|BRIGHT] = (char*) alloc(strlen(scratch)+strlen(MD)+1);
		Strcpy(hilites[c|BRIGHT], MD);
		Strcat(hilites[c|BRIGHT], scratch);
	    }

	}
	if (!iflags.wc2_newcolors)
		hilites[CLR_BLACK] = hilites[CLR_BLUE];
}

# else /* UNIX && TERMINFO */

/* find the foreground and background colors set by nh_HI or nh_HE */
static void
analyze_seq (str, fg, bg)
char *str;
int *fg, *bg;
{
	register int c, code;
	int len;

#   ifdef MICRO
	*fg = CLR_GRAY; *bg = CLR_BLACK;
#   else
	*fg = *bg = NO_COLOR;
#   endif

	c = (str[0] == '\233') ? 1 : 2;	 /* index of char beyond esc prefix */
	len = strlen(str) - 1;		 /* length excluding attrib suffix */
	if ((c != 1 && (str[0] != '\033' || str[1] != '[')) ||
	    (len - c) < 1 || str[len] != 'm')
		return;

	while (c < len) {
	    if ((code = atoi(&str[c])) == 0) { /* reset */
		/* this also catches errors */
#   ifdef MICRO
		*fg = CLR_GRAY; *bg = CLR_BLACK;
#   else
		*fg = *bg = NO_COLOR;
#   endif
	    } else if (code == 1) { /* bold */
		*fg |= BRIGHT;
#   if 0
	/* I doubt we'll ever resort to using blinking characters,
	   unless we want a pulsing glow for something.  But, in case
	   we do... - 3. */
	    } else if (code == 5) { /* blinking */
		*fg |= BLINK;
	    } else if (code == 25) { /* stop blinking */
		*fg &= ~BLINK;
#   endif
	    } else if (code == 7 || code == 27) { /* reverse */
		code = *fg & ~BRIGHT;
		*fg = *bg | (*fg & BRIGHT);
		*bg = code;
	    } else if (code >= 30 && code <= 37) { /* hi_foreground RGB */
		*fg = code - 30;
	    } else if (code >= 40 && code <= 47) { /* hi_background RGB */
		*bg = code - 40;
	    }
	    while (digit(str[++c]));
	    c++;
	}
}

/*
 * Sets up highlighting sequences, using ANSI escape sequences (highlight code
 * found in print.c).  The nh_HI and nh_HE sequences (usually from SO) are
 * scanned to find foreground and background colors.
 */

static void
init_hilite()
{
	register int c;

	int backg, foreg, hi_backg, hi_foreg;

	for (c = 0; c < SIZE(hilites); c++)
	    hilites[c] = nh_HI;
	hilites[CLR_GRAY] = hilites[NO_COLOR] = (char *)0;

	analyze_seq(nh_HI, &hi_foreg, &hi_backg);
	analyze_seq(nh_HE, &foreg, &backg);

	for (c = 0; c < SIZE(hilites); c++)
	    /* avoid invisibility */
	    if ((backg & ~BRIGHT) != c) {
#   ifdef MICRO
		if (c == CLR_BLUE) continue;
#   endif
		if (c == foreg)
		    hilites[c] = (char *)0;
		else if (c != hi_foreg || backg != hi_backg) {
		    hilites[c] = (char *) alloc(sizeof("\033[%d;3%d;4%dm"));
		    Sprintf(hilites[c], "\033[%d", !!(c & BRIGHT));
		    if ((c | BRIGHT) != (foreg | BRIGHT))
			Sprintf(eos(hilites[c]), ";3%d", c & ~BRIGHT);
		    if (backg != CLR_BLACK)
			Sprintf(eos(hilites[c]), ";4%d", backg & ~BRIGHT);
		    Strcat(hilites[c], "m");
		}
	    }

#   ifdef MICRO
	/* brighten low-visibility colors */
	hilites[CLR_BLUE] = hilites[CLR_BLUE|BRIGHT];
#   endif
}
# endif /* UNIX */

static void
kill_hilite()
{
	register int c;

	for (c = 0; c < CLR_MAX / 2; c++) {
		if ((!iflags.wc2_newcolors) &&
		    (c == CLR_BLUE || c == CLR_GRAY)) continue;

		if (hilites[c|BRIGHT] == hilites[c])  hilites[c|BRIGHT] = 0;
		if (hilites[c] && (hilites[c] != nh_HI))
			free((genericptr_t) hilites[c]),  hilites[c] = 0;
		if (hilites[c|BRIGHT] && (hilites[c|BRIGHT] != nh_HI))
			free((genericptr_t) hilites[c|BRIGHT]),  hilites[c|BRIGHT] = 0;
	}
	return;
}
#endif /* TEXTCOLOR */


static char nulstr[] = "";

static char *
s_atr2str(n)
int n;
{
    switch (n) {
	    case ATR_ULINE:
		    if(nh_US) return nh_US;
	    case ATR_BOLD:
	    case ATR_BLINK:
#if defined(TERMLIB) && defined(TEXTCOLOR)
		    if (MD) return MD;
#endif
		    return nh_HI;
	    case ATR_INVERSE:
		    return MR;
    }
    return nulstr;
}

static char *
e_atr2str(n)
int n;
{
    switch (n) {
	    case ATR_ULINE:
		    if(nh_UE) return nh_UE;
	    case ATR_BOLD:
	    case ATR_BLINK:
		    return nh_HE;
	    case ATR_INVERSE:
		    return ME;
    }
    return nulstr;
}


void
term_start_attr(attr)
int attr;
{
	if (attr) {
		xputs(s_atr2str(attr));
	}
}


void
term_end_attr(attr)
int attr;
{
	if(attr) {
		xputs(e_atr2str(attr));
	}
}


void
term_start_raw_bold()
{
	xputs(nh_HI);
}


void
term_end_raw_bold()
{
	xputs(nh_HE);
}


#ifdef TEXTCOLOR

void
term_end_color()
{
	xputs(nh_HE);
}


void
term_start_color(color)
int color;
{
	if (iflags.wc2_newcolors)
		xputs(hilites[ttycolors[color]]);
	else
		xputs(hilites[color]);
}


int
has_color(color)
int color;
{
#ifdef X11_GRAPHICS
	/* XXX has_color() should be added to windowprocs */
	if (windowprocs.name != NULL &&
	    !strcmpi(windowprocs.name, "X11")) return TRUE;
#endif
#ifdef LISP_GRAPHICS
	/* XXX has_color() should be added to windowprocs */
	if (windowprocs.name != NULL &&
	    !strcmpi(windowprocs.name, "lisp")) return TRUE;
#endif
#ifdef QT_GRAPHICS
	/* XXX has_color() should be added to windowprocs */
	if (windowprocs.name != NULL &&
	    !strcmpi(windowprocs.name, "Qt")) return TRUE;
#endif
#ifdef AMII_GRAPHICS
	/* hilites[] not used */
	return iflags.use_color;
#endif
#ifdef CURSES_GRAPHICS
    /* XXX has_color() should be added to windowprocs */
    /* iflags.wc_color is set to false and the option disabled if the
     terminal cannot display color */
    if (windowprocs.name != NULL &&
     !strcmpi(windowprocs.name, "curses")) return iflags.wc_color;
#endif
	return hilites[color] != (char *)0;
}

#endif /* TEXTCOLOR */

#endif /* OVLB */

#endif /* TTY_GRAPHICS */

/*termcap.c*/
