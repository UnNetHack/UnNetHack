/*	SCCS Id: @(#)cmd.c	3.4	2003/02/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "func_tab.h"
/* #define DEBUG */	/* uncomment for debugging */

/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
#if defined(SYSV)
#define NR_OF_EOFS	20
#endif

#define CMD_TRAVEL (char)0x90

#ifdef DEBUG
/*
 * only one "wiz_debug_cmd" routine should be available (in whatever
 * module you are trying to debug) or things are going to get rather
 * hard to link :-)
 */
extern int NDECL(wiz_debug_cmd);
#endif

static int NDECL((*timed_occ_fn));

STATIC_PTR int NDECL(doprev_message);
STATIC_PTR int NDECL(timed_occupation);
STATIC_PTR int NDECL(doextcmd);
STATIC_PTR int NDECL(domonability);
STATIC_PTR int NDECL(dooverview_or_wiz_where);
STATIC_PTR int NDECL(dotravel);
STATIC_PTR int NDECL(doautoexplore);
STATIC_PTR int NDECL(doautofight);
# ifdef WIZARD
int NDECL(wiz_show_rooms);
STATIC_PTR int NDECL(wiz_wish);
STATIC_PTR int NDECL(wiz_identify);
STATIC_PTR int NDECL(wiz_map);
STATIC_PTR int NDECL(wiz_genesis);
STATIC_PTR int NDECL(wiz_panic);
STATIC_PTR int NDECL(wiz_polyself);
STATIC_PTR int NDECL(wiz_level_tele);
STATIC_PTR int NDECL(wiz_level_change);
STATIC_PTR int NDECL(wiz_show_seenv);
STATIC_PTR int NDECL(wiz_show_vision);
STATIC_PTR int NDECL(wiz_mon_polycontrol);
STATIC_PTR int NDECL(wiz_show_wmodes);
STATIC_PTR int NDECL(wiz_mazewalkmap);
extern char SpLev_Map[COLNO][ROWNO];
STATIC_PTR int NDECL(wiz_showkills);	/* showborn patch */
#ifdef SHOW_BORN
extern void FDECL(list_vanquished, (int, BOOLEAN_P, BOOLEAN_P)); /* showborn patch */
#endif /* SHOW_BORN */
#ifdef DEBUG_MIGRATING_MONS
STATIC_PTR int NDECL(wiz_migrate_mons);
#endif
STATIC_DCL void FDECL(count_obj, (struct obj *, long *, long *, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(obj_chain, (winid, const char *, struct obj *, long *, long *));
STATIC_DCL void FDECL(mon_invent_chain, (winid, const char *, struct monst *, long *, long *));
STATIC_DCL void FDECL(mon_chain, (winid, const char *, struct monst *, long *, long *));
STATIC_DCL void FDECL(contained, (winid, const char *, long *, long *));
STATIC_PTR int NDECL(wiz_show_stats);
#  ifdef PORT_DEBUG
STATIC_DCL int NDECL(wiz_port_debug);
#  endif
# else
extern int NDECL(tutorial_redisplay);
# endif
STATIC_PTR int NDECL(enter_explore_mode);
STATIC_PTR int NDECL(doattributes);
STATIC_PTR int NDECL(doconduct); /**/
STATIC_PTR boolean NDECL(minimal_enlightenment);

STATIC_DCL void FDECL(enlght_line, (const char *,const char *,const char *));
STATIC_DCL char *FDECL(enlght_combatinc, (const char *,int,int,char *));
#ifdef UNIX
static void NDECL(end_of_input);
#endif

static const char* readchar_queue="";
static char last_cmd_char='\0';

STATIC_DCL char *NDECL(parse);
STATIC_DCL boolean FDECL(help_dir, (CHAR_P,const char *));

STATIC_PTR int
doprev_message()
{
    return nh_doprev_message();
}

/* Count down by decrementing multi */
STATIC_PTR int
timed_occupation()
{
	(*timed_occ_fn)();
	if (multi > 0)
		multi--;
	return multi > 0;
}

/* If you have moved since initially setting some occupations, they
 * now shouldn't be able to restart.
 *
 * The basic rule is that if you are carrying it, you can continue
 * since it is with you.  If you are acting on something at a distance,
 * your orientation to it must have changed when you moved.
 *
 * The exception to this is taking off items, since they can be taken
 * off in a number of ways in the intervening time, screwing up ordering.
 *
 *	Currently:	Take off all armor.
 *			Picking Locks / Forcing Chests.
 *			Setting traps.
 */
void
reset_occupations()
{
	reset_remarm();
	reset_pick();
	reset_trapset();
}

/* If a time is given, use it to timeout this function, otherwise the
 * function times out by its own means.
 */
void
set_occupation(fn, txt, xtime)
int NDECL((*fn));
const char *txt;
int xtime;
{
	if (xtime) {
		occupation = timed_occupation;
		timed_occ_fn = fn;
	} else
		occupation = fn;
	occtxt = txt;
	occtime = 0;
	return;
}

static char NDECL(popch);

/* Provide a means to redo the last command.  The flag `in_doagain' is set
 * to true while redoing the command.  This flag is tested in commands that
 * require additional input (like `throw' which requires a thing and a
 * direction), and the input prompt is not shown.  Also, while in_doagain is
 * TRUE, no keystrokes can be saved into the saveq.
 */
#define BSIZE 20
static char pushq[BSIZE], saveq[BSIZE];
static int phead, ptail, shead, stail;

static char
popch() {
	/* If occupied, return '\0', letting tgetch know a character should
	 * be read from the keyboard.  If the character read is not the
	 * ABORT character (as checked in pcmain.c), that character will be
	 * pushed back on the pushq.
	 */
	if (occupation) return '\0';
	if (in_doagain) return(char)((shead != stail) ? saveq[stail++] : '\0');
	else		return(char)((phead != ptail) ? pushq[ptail++] : '\0');
}

char
pgetchar() {		/* curtesy of aeb@cwi.nl */
	register int ch;

	if(!(ch = popch()))
		ch = nhgetch();
	return((char)ch);
}

/* A ch == 0 resets the pushq */
void
pushch(ch)
char ch;
{
	if (!ch)
		phead = ptail = 0;
	if (phead < BSIZE)
		pushq[phead++] = ch;
	return;
}

/* A ch == 0 resets the saveq.	Only save keystrokes when not
 * replaying a previous command.
 */
void
savech(ch)
char ch;
{
	if (!in_doagain) {
		if (!ch)
			phead = ptail = shead = stail = 0;
		else if (shead < BSIZE)
			saveq[shead++] = ch;
	}
	return;
}

STATIC_PTR int
doextcmd()	/* here after # - now read a full-word command */
{
	int idx, retval;

	/* keep repeating until we don't run help or quit */
	do {
	    idx = get_ext_cmd();
	    if (idx < 0) return 0;	/* quit */

	    retval = (*extcmdlist[idx].ef_funct)();
	} while (extcmdlist[idx].ef_funct == doextlist);

	return retval;
}

int
doextlist()	/* here after #? - now list all full-word commands */
{
	register const struct ext_func_tab *efp;
	char	 buf[BUFSZ];
	winid datawin;

	datawin = create_nhwindow(NHW_TEXT);
	putstr(datawin, 0, "");
	putstr(datawin, 0, "            Extended Commands List");
	putstr(datawin, 0, "");
	putstr(datawin, 0, "    Press '#', then type:");
	putstr(datawin, 0, "");

	for(efp = extcmdlist; efp->ef_txt; efp++) {
		Sprintf(buf, "    %-15s - %s.", efp->ef_txt, efp->ef_desc);
		putstr(datawin, 0, buf);
	}
	display_nhwindow(datawin, FALSE);
	destroy_nhwindow(datawin);
	return 0;
}

#if defined(TTY_GRAPHICS) || defined(CURSES_GRAPHICS)
#define MAX_EXT_CMD 50		/* Change if we ever have > 50 ext cmds */
/*
 * This is currently used only by the tty port and is
 * controlled via runtime option 'extmenu'
 */
int
extcmd_via_menu()	/* here after # - now show pick-list of possible commands */
{
    const struct ext_func_tab *efp;
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any;
    const struct ext_func_tab *choices[MAX_EXT_CMD];
    char buf[BUFSZ];
    char cbuf[QBUFSZ], prompt[QBUFSZ], fmtstr[20];
    int i, n, nchoices, acount;
    int ret,  biggest;
    int accelerator, prevaccelerator;
    int  matchlevel = 0;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
	    i = n = 0;
	    accelerator = 0;
	    any.a_void = 0;
	    /* populate choices */
	    for(efp = extcmdlist; efp->ef_txt; efp++) {
		if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
			choices[i++] = efp;
			if ((int)strlen(efp->ef_desc) > biggest) {
				biggest = strlen(efp->ef_desc);
				Sprintf(fmtstr,"%%-%ds", biggest + 15);
			}
			if (i >= MAX_EXT_CMD - 2) {
			    warning("Exceeded %d extended commands in doextcmd() menu",
					MAX_EXT_CMD - 2);
			    return 0;
			}
		}
	    }
	    choices[i] = (struct ext_func_tab *)0;
	    nchoices = i;
	    /* if we're down to one, we have our selection so get out of here */
	    if (nchoices == 1) {
		for (i = 0; extcmdlist[i].ef_txt != (char *)0; i++)
			if (!strncmpi(extcmdlist[i].ef_txt, cbuf, matchlevel)) {
				ret = i;
				break;
			}
		break;
	    }

	    /* otherwise... */
	    win = create_nhwindow(NHW_MENU);
	    start_menu(win);
	    prevaccelerator = 0;
	    acount = 0;
	    for(i = 0; choices[i]; ++i) {
		accelerator = choices[i]->ef_txt[matchlevel];
		if (accelerator != prevaccelerator || nchoices < (ROWNO - 3)) {
		    if (acount) {
 			/* flush the extended commands for that letter already in buf */
			Sprintf(buf, fmtstr, prompt);
			any.a_char = prevaccelerator;
			add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, any.a_char, 0,
					ATR_NONE, buf, FALSE);
			acount = 0;
		    }
		}
		prevaccelerator = accelerator;
		if (!acount || nchoices < (ROWNO - 3)) {
		    Sprintf(prompt, "%s [%s]", choices[i]->ef_txt,
				choices[i]->ef_desc);
		} else if (acount == 1) {
		    Sprintf(prompt, "%s or %s", choices[i-1]->ef_txt,
				choices[i]->ef_txt);
		} else {
		    Strcat(prompt," or ");
		    Strcat(prompt, choices[i]->ef_txt);
		}
		++acount;
	    }
	    if (acount) {
		/* flush buf */
		Sprintf(buf, fmtstr, prompt);
		any.a_char = prevaccelerator;
		add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, any.a_char, 0, ATR_NONE, buf, FALSE);
	    }
	    Sprintf(prompt, "Extended Command: %s", cbuf);
	    end_menu(win, prompt);
	    n = select_menu(win, PICK_ONE, &pick_list);
	    destroy_nhwindow(win);
	    if (n==1) {
		if (matchlevel > (QBUFSZ - 2)) {
			free((genericptr_t)pick_list);
#ifdef DEBUG
			warning("Too many characters (%d) entered in extcmd_via_menu()",
				matchlevel);
#endif
			ret = -1;
		} else {
			cbuf[matchlevel++] = pick_list[0].item.a_char;
			cbuf[matchlevel] = '\0';
			free((genericptr_t)pick_list);
		}
	    } else {
		if (matchlevel) {
			ret = 0;
			matchlevel = 0;
		} else
			ret = -1;
	    }
    }
    return ret;
}
#endif

/* #monster command - use special monster ability while polymorphed */
STATIC_PTR int
domonability()
{
	if (can_breathe(youmonst.data)) return dobreathe();
	else if (attacktype(youmonst.data, AT_SPIT)) return dospit();
	else if (youmonst.data->mlet == S_NYMPH) return doremove();
	else if (attacktype(youmonst.data, AT_GAZE)) return dogaze();
	else if (is_were(youmonst.data)) return dosummon();
	else if (webmaker(youmonst.data)) return dospinweb();
	else if (is_hider(youmonst.data)) return dohide();
	else if (is_mind_flayer(youmonst.data)) return domindblast();
	else if (u.umonnum == PM_GREMLIN) {
	    if(IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		if (split_mon(&youmonst, (struct monst *)0))
		    dryup(u.ux, u.uy, TRUE);
	    } else There("is no fountain here.");
	} else if (is_unicorn(youmonst.data)) {
	    fix_attributes_and_properties((struct obj *)0,0);
	    return 1;
	} else if (youmonst.data->msound == MS_SHRIEK) {
	    You("shriek.");
	    if(u.uburied)
		pline("Unfortunately sound does not carry well through rock.");
	    else aggravate();
	} else if (Upolyd)
		pline("Any special ability you may have is purely reflexive.");
	else You("don't have a special ability in your normal form!");
	return 0;
}

STATIC_PTR int
enter_explore_mode()
{
	if(!discover && !wizard) {
		pline("Beware!  From explore mode there will be no return to normal game.");
		if (paranoid_yn("Do you want to enter explore mode?", iflags.paranoid_quit) == 'y') {
			clear_nhwindow(WIN_MESSAGE);
			You("are now in non-scoring explore mode.");
			discover = TRUE;
		}
		else {
			clear_nhwindow(WIN_MESSAGE);
			pline("Resuming normal game.");
		}
	}
	return 0;
}

/* #vanquished command */
STATIC_PTR int dovanquished()
{
	list_vanquishedonly();
	return 0;
}

STATIC_PTR int
dooverview_or_wiz_where()
{
/*
#ifdef WIZARD
	if (wizard) return wiz_where();
	else
#endif
*/
	dooverview();
	return 0;
}

#ifdef WIZARD

int
wiz_show_rooms()
{
    winid win;
    int x,y;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    for (y = 0; y < ROWNO; y++) {
	for (x = 0; x < COLNO; x++) {
	    int rno = levl[x][y].roomno;
	    if (rno == NO_ROOM)
		row[x] = '.';
	    else if (rno == SHARED)
		row[x] = '+';
	    else if (rno == SHARED_PLUS)
		row[x] = '*';
	    else {
		int i = (rno - ROOMOFFSET) % 52;
		row[x] = (i < 26) ? ('a'+i) : ('A'+i-26);
	    }
	}
	row[COLNO] = '\0';
	putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* ^W command - wish for something */
STATIC_PTR int
wiz_wish()	/* Unlimited wishes for debug mode by Paul Polderman */
{
	if (wizard) {
	    boolean save_verbose = flags.verbose;

	    flags.verbose = FALSE;
	    makewish(TRUE);
	    flags.verbose = save_verbose;
	    (void) encumber_msg();
	} else
	    pline("Unavailable command '^W'.");
	return 0;
}

/* ^I command - identify hero's inventory */
STATIC_PTR int
wiz_identify()
{
	if (wizard)	identify_pack(0);
	else		pline("Unavailable command '^I'.");
	return 0;
}

/* ^F command - reveal the level map and any traps on it */
STATIC_PTR int
wiz_map()
{
	if (wizard) {
	    struct trap *t;
	    long save_Hconf = HConfusion,
		 save_Hhallu = HHallucination;

	    HConfusion = HHallucination = 0L;
	    for (t = ftrap; t != 0; t = t->ntrap) {
		t->tseen = 1;
		map_trap(t, TRUE);
	    }
	    do_mapping();
	    HConfusion = save_Hconf;
	    HHallucination = save_Hhallu;
	} else
	    pline("Unavailable command '^F'.");
	return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
STATIC_PTR int
wiz_genesis()
{
	if (wizard)	(void) create_particular();
	else		pline("Unavailable command '^G'.");
	return 0;
}

/* ^V command - level teleport, or tutorial review */
STATIC_PTR int
wiz_level_tele()
{
	if (wizard)	level_tele();
	else if(flags.tutorial)
	    tutorial_redisplay();
	return 0;
}

/* #monpolycontrol command - choose new form for shapechangers, polymorphees */
STATIC_PTR int
wiz_mon_polycontrol()
{
    iflags.mon_polycontrol = !iflags.mon_polycontrol;
    pline("Monster polymorph control is %s.",
	  iflags.mon_polycontrol ? "on" : "off");
    return 0;
}

/* #levelchange command - adjust hero's experience level */
STATIC_PTR int
wiz_level_change()
{
    char buf[BUFSZ];
    int newlevel;
    int ret;

    getlin("To what experience level do you want to be set?", buf);
    (void)mungspaces(buf);
    if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
    else ret = sscanf(buf, "%d", &newlevel);

    if (ret != 1) {
	pline("%s", Never_mind);
	return 0;
    }
    if (newlevel == u.ulevel) {
	You("are already that experienced.");
    } else if (newlevel < u.ulevel) {
	if (u.ulevel == 1) {
	    You("are already as inexperienced as you can get.");
	    return 0;
	}
	if (newlevel < 1) newlevel = 1;
	while (u.ulevel > newlevel)
	    losexp("#levelchange");
    } else {
	if (u.ulevel >= MAXULEV) {
	    You("are already as experienced as you can get.");
	    return 0;
	}
	if (newlevel > MAXULEV) newlevel = MAXULEV;
	while (u.ulevel < newlevel)
	    pluslvl(FALSE);
    }
    u.ulevelmax = u.ulevel;
    return 0;
}

/* #panic command - test program's panic handling */
STATIC_PTR int
wiz_panic()
{
	if (yn("Do you want to call panic() and end your game?") == 'y')
		panic("crash test.");
        return 0;
}

/* #polyself command - change hero's form */
STATIC_PTR int
wiz_polyself()
{
        polyself(TRUE);
        return 0;
}

/* #seenv command */
STATIC_PTR int
wiz_show_seenv()
{
	winid win;
	int x, y, v, startx, stopx, curx;
	char row[COLNO+1];

	win = create_nhwindow(NHW_TEXT);
	/*
	 * Each seenv description takes up 2 characters, so center
	 * the seenv display around the hero.
	 */
	startx = max(1, u.ux-(COLNO/4));
	stopx = min(startx+(COLNO/2), COLNO);
	/* can't have a line exactly 80 chars long */
	if (stopx - startx == COLNO/2) startx++;

	for (y = 0; y < ROWNO; y++) {
	    for (x = startx, curx = 0; x < stopx; x++, curx += 2) {
		if (x == u.ux && y == u.uy) {
		    row[curx] = row[curx+1] = '@';
		} else {
		    v = levl[x][y].seenv & 0xff;
		    if (v == 0)
			row[curx] = row[curx+1] = ' ';
		    else
			Sprintf(&row[curx], "%02x", v);
		}
	    }
	    /* remove trailing spaces */
	    for (x = curx-1; x >= 0; x--)
		if (row[x] != ' ') break;
	    row[x+1] = '\0';

	    putstr(win, 0, row);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

/* #vision command */
STATIC_PTR int
wiz_show_vision()
{
	winid win;
	int x, y, v;
	char row[COLNO+1];

	win = create_nhwindow(NHW_TEXT);
	Sprintf(row, "Flags: 0x%x could see, 0x%x in sight, 0x%x temp lit",
		COULD_SEE, IN_SIGHT, TEMP_LIT);
	putstr(win, 0, row);
	putstr(win, 0, "");
	for (y = 0; y < ROWNO; y++) {
	    for (x = 1; x < COLNO; x++) {
		if (x == u.ux && y == u.uy)
		    row[x] = '@';
		else {
		    v = viz_array[y][x]; /* data access should be hidden */
		    if (v == 0)
			row[x] = ' ';
		    else
			row[x] = '0' + viz_array[y][x];
		}
	    }
	    /* remove trailing spaces */
	    for (x = COLNO-1; x >= 1; x--)
		if (row[x] != ' ') break;
	    row[x+1] = '\0';

	    putstr(win, 0, &row[1]);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

/* #wmode command */
STATIC_PTR int
wiz_show_wmodes()
{
	winid win;
	int x,y;
	char row[COLNO+1];
	struct rm *lev;

	win = create_nhwindow(NHW_TEXT);
	for (y = 0; y < ROWNO; y++) {
	    for (x = 0; x < COLNO; x++) {
		lev = &levl[x][y];
		if (x == u.ux && y == u.uy)
		    row[x] = '@';
		else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
		    row[x] = '0' + (lev->wall_info & WM_MASK);
		else if (lev->typ == CORR)
		    row[x] = '#';
		else if (IS_ROOM(lev->typ) || IS_DOOR(lev->typ))
		    row[x] = '.';
		else
		    row[x] = 'x';
	    }
	    row[COLNO] = '\0';
	    putstr(win, 0, row);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

/* #showkills command */
STATIC_PTR int wiz_showkills()		/* showborn patch */
{
	list_vanquished('y', FALSE, TRUE);
	return 0;
}

#endif /* WIZARD */


/* -enlightenment and conduct- */
static winid en_win;
static const char
	You_[] = "You ",
	are[]  = "are ",  were[]  = "were ",
	have[] = "have ", had[]   = "had ",
	can[]  = "can ",  could[] = "could ";
static const char
	have_been[]  = "have been ",
	have_never[] = "have never ", never[] = "never ";

#define enl_msg(prefix,present,past,suffix) \
			enlght_line(prefix, final ? past : present, suffix)
#define you_are(attr)	enl_msg(You_,are,were,attr)
#define you_have(attr)	enl_msg(You_,have,had,attr)
#define you_can(attr)	enl_msg(You_,can,could,attr)
#define you_have_been(goodthing) enl_msg(You_,have_been,were,goodthing)
#define you_have_never(badthing) enl_msg(You_,have_never,never,badthing)
#define you_have_X(something)	enl_msg(You_,have,(const char *)"",something)

static int want_display = FALSE;
static void
enlght_line(start, middle, end)
const char *start, *middle, *end;
{
	char buf[BUFSZ];

	Sprintf(buf, "%s%s%s.", start, middle, end);
	if (want_display) {
		putstr(en_win, 0, buf);
	}
	dump_list_item(buf);
}

/* format increased damage or chance to hit */
static char *
enlght_combatinc(inctyp, incamt, final, outbuf)
const char *inctyp;
int incamt, final;
char *outbuf;
{
	char numbuf[24];
	const char *modif, *bonus;

	if (final
#ifdef WIZARD
		|| wizard
#endif
	  ) {
	    Sprintf(numbuf, "%s%d",
		    (incamt > 0) ? "+" : "", incamt);
	    modif = (const char *) numbuf;
	} else {
	    int absamt = abs(incamt);

	    if (absamt <= 3) modif = "small";
	    else if (absamt <= 6) modif = "moderate";
	    else if (absamt <= 12) modif = "large";
	    else modif = "huge";
	}
	bonus = (incamt > 0) ? "bonus" : "penalty";
	/* "bonus to hit" vs "damage bonus" */
	if (!strcmp(inctyp, "damage")) {
	    const char *ctmp = inctyp;
	    inctyp = bonus;
	    bonus = ctmp;
	}
	Sprintf(outbuf, "%s %s %s", an(modif), bonus, inctyp);
	return outbuf;
}

void
enlightenment(final, want_disp)
int final;	/* 0 => still in progress; 1 => over, survived; 2 => dead */
boolean want_disp;
{
	int ltmp;
	char buf[BUFSZ];

	want_display = want_disp;

	Sprintf(buf, final ? "Final Attributes:" : "Current Attributes:");
	if (want_display) {
		en_win = create_nhwindow(NHW_MENU);
		putstr(en_win, 0, buf);
		putstr(en_win, 0, "");
	}
	dump_title(buf);
	dump_list_start();

	if (u.uevent.uhand_of_elbereth) {
	    static const char * const hofe_titles[3] = {
				"the Hand of Elbereth",
				"the Envoy of Balance",
				"the Glory of Arioch"
	    };
	    you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1]);
	}

	/* heaven or hell modes */
	if (heaven_or_hell_mode) {
		if (u.ulives > 1) {
			Sprintf(buf, "%ld lives left", u.ulives);
		} else if (u.ulives == 0) {
			Sprintf(buf, "no lives left");
		} else {
			Sprintf(buf, "%ld life left", u.ulives);
		}
		you_have(buf);
	}

	/* note: piousness 3 matches MIN_QUEST_ALIGN (quest.h) */
	if (u.ualign.record >= 20)	you_are("piously aligned");
	else if (u.ualign.record > 13)	you_are("devoutly aligned");
	else if (u.ualign.record > 8)	you_are("fervently aligned");
	else if (u.ualign.record > 3)	you_are("stridently aligned");
	else if (u.ualign.record == 3)	you_are("aligned");
	else if (u.ualign.record > 0)	you_are("haltingly aligned");
	else if (u.ualign.record == 0)	you_are("nominally aligned");
	else if (u.ualign.record >= -3)	you_have("strayed");
	else if (u.ualign.record >= -8)	you_have("sinned");
	else you_have("transgressed");
#ifdef WIZARD
	if (wizard || final) {
		Sprintf(buf, " %d", u.uhunger);
		enl_msg("Hunger level ", "is", "was", buf);
		Sprintf(buf, " %d / %ld", u.ualign.record, ALIGNLIM);
		enl_msg("Your alignment ", "is", "was", buf);
		Sprintf(buf, " %d - %d",
		        min_monster_difficulty(), max_monster_difficulty());
		enl_msg("Monster difficulty range ", "is", "was", buf);
		Sprintf(buf, " %d", level_difficulty());
		enl_msg("Level difficulty ", "is", "was", buf);
	}
#endif

	/*** Resistances to troubles ***/
	if (Fire_resistance) you_are("fire resistant");
	if (Cold_resistance) you_are("cold resistant");
	if (Sleep_resistance) you_are("sleep resistant");
	if (Disint_resistance) you_are("disintegration-resistant");
	if (Shock_resistance) you_are("shock resistant");
	if (Poison_resistance) you_are("poison resistant");
	if (Drain_resistance) you_are("level-drain resistant");
	if (Sick_resistance) you_are("immune to sickness");
	if (Antimagic) you_are("magic-protected");
	if (Acid_resistance) you_are("acid resistant");
	if (Stone_resistance)
		you_are("petrification resistant");
	if (Invulnerable) you_are("invulnerable");
	if (u.uedibility) you_can("recognize detrimental food");

	/*** Troubles ***/
	if (!flags.perma_hallu && Halluc_resistance)
		enl_msg("You resist", "", "ed", " hallucinations");
	if (flags.perma_hallu) you_are("permanently hallucinating");
	if (final) {
		if (!flags.perma_hallu && Hallucination) you_are("hallucinating");
		if (Stunned) you_are("stunned");
		if (Confusion) you_are("confused");
		if (Blinded) you_are("blinded");
		if (Sick) {
			if (u.usick_type & SICK_VOMITABLE)
				you_are("sick from food poisoning");
			if (u.usick_type & SICK_NONVOMITABLE)
				you_are("sick from illness");
		}
		if (Punished) {
			you_are("punished");
		}
	}
	if (Stoned) you_are("turning to stone");
	if (Slimed) you_are("turning into slime");
	if (Strangled) you_are((u.uburied) ? "buried" : "being strangled");
	if (Glib) {
		Sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
		you_have(buf);
	}
	if (Fumbling) enl_msg("You fumble", "", "d", "");
	if (Wounded_legs
	    && !u.usteed
			  ) {
		Sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
		you_have(buf);
	}
#if defined(WIZARD)
	if (Wounded_legs && u.usteed && (wizard || final)) {
	    Strcpy(buf, x_monnam(u.usteed, ARTICLE_YOUR, (char *)0, 
		    SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, FALSE));
	    *buf = highc(*buf);
	    enl_msg(buf, " has", " had", " wounded legs");
	}
#endif
	if (Sleeping) enl_msg("You ", "fall", "fell", " asleep");
	if (Hunger) enl_msg("You hunger", "", "ed", " rapidly");

	/*** Vision and senses ***/
	if (See_invisible) enl_msg(You_, "see", "saw", " invisible");
	if (Blind_telepat) you_are("telepathic");
	if (Warning) you_are("warned");
	if (Warn_of_mon && flags.warntype) {
		Sprintf(buf, "aware of the presence of %s",
			(flags.warntype & M2_ORC) ? "orcs" :
			(flags.warntype & M2_DEMON) ? "demons" :
			(flags.warntype & M2_GIANT) ? "giants" :
			(flags.warntype & M2_WERE) ? "lycanthropes" :
			(flags.warntype & M2_UNDEAD) ? "undead" :
			something);
		you_are(buf);
	} else if (Warn_of_mon && uwep) {
		const char *monster_name = get_warned_of_monster(uwep);
		if (monster_name) {
			Sprintf(buf, "aware of the presence of %s", monster_name);
			you_are(buf);
		}
	}
	if (Undead_warning) you_are("warned of undead");
	if (Searching) you_have("automatic searching");
	if (Clairvoyant) you_are("clairvoyant");
	if (Infravision) you_have("infravision");
	if (Detect_monsters) you_are("sensing the presence of monsters");
	if (u.umconf) you_are("going to confuse monsters");

	/*** Appearance and behavior ***/
	if (Adornment) {
	    int adorn = 0;

	    if(uleft && uleft->otyp == RIN_ADORNMENT) adorn += uleft->spe;
	    if(uright && uright->otyp == RIN_ADORNMENT) adorn += uright->spe;
	    if (adorn < 0)
		you_are("poorly adorned");
	    else
		you_are("adorned");
	}
	if (Invisible) you_are("invisible");
	else if (Invis) you_are("invisible to others");
	/* ordinarily "visible" is redundant; this is a special case for
	   the situation when invisibility would be an expected attribute */
	else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis)
	    you_are("visible");
	if (Displaced) you_are("displaced");
	if (Stealth) you_are("stealthy");
	if (Aggravate_monster) enl_msg("You aggravate", "", "d", " monsters");
	if (Conflict) enl_msg("You cause", "", "d", " conflict");

	/*** Transportation ***/
	if (Jumping) you_can("jump");
	if (Teleportation) you_can("teleport");
	if (Teleport_control) you_have("teleport control");
	if (Lev_at_will) you_are("levitating, at will");
	else if (Levitation) you_are("levitating");	/* without control */
	else if (Flying) you_can("fly");
	if (Wwalking) you_can("walk on water");
	if (Swimming) you_can("swim");
	if (Breathless) you_can("survive without air");
	else if (Amphibious) you_can("breathe water");
	if (Passes_walls) you_can("walk through walls");
	/* If you die while dismounting, u.usteed is still set.  Since several
	 * places in the done() sequence depend on u.usteed, just detect this
	 * special case. */
	if (u.usteed && (final < 2 || strcmp(killer, "riding accident"))) {
	    Sprintf(buf, "riding %s", y_monnam(u.usteed));
	    you_are(buf);
	}
	if (u.uswallow) {
	    Sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
#ifdef WIZARD
	    if (wizard || final) Sprintf(eos(buf), " (%u)", u.uswldtim);
#endif
	    you_are(buf);
	} else if (u.ustuck) {
	    Sprintf(buf, "%s %s",
		    (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
		    a_monnam(u.ustuck));
	    you_are(buf);
	}

	/*** Physical attributes ***/
	if (u.uhitinc)
	    you_have(enlght_combatinc("to hit", u.uhitinc, final, buf));
	if (u.udaminc)
	    you_have(enlght_combatinc("damage", u.udaminc, final, buf));
	if (Slow_digestion) you_have("slower digestion");
	if (Regeneration && can_regenerate()) enl_msg("You regenerate", "", "d", "");
	if (!can_regenerate()) {
	    if (is_elf(youmonst.data))
		you_are("in direct contact with cold iron");
	    else if (is_vampiric(youmonst.data))
		you_are("in direct contact with silver");
	}
	if (u.uspellprot || Protection) {
	    int prot = 0;

	    if(uleft && uleft->otyp == RIN_PROTECTION) prot += uleft->spe;
	    if(uright && uright->otyp == RIN_PROTECTION) prot += uright->spe;
	    if (HProtection & INTRINSIC) prot += u.ublessed;
	    prot += u.uspellprot;

	    if (prot < 0)
		you_are("ineffectively protected");
	    else if (prot > 0)
		you_are("protected");
	}
	if (Protection_from_shape_changers)
		you_are("protected from shape changers");
	if (Polymorph) you_are("polymorphing");
	if (Polymorph_control) you_have("polymorph control");
	if (u.ulycn >= LOW_PM) {
		Strcpy(buf, an(mons[u.ulycn].mname));
		you_are(buf);
	}
	if (Upolyd) {
	    if (u.umonnum == u.ulycn) Strcpy(buf, "in beast form");
	    else Sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
#ifdef WIZARD
	    if (wizard || final) Sprintf(eos(buf), " (%d)", u.mtimedone);
#endif
	    you_are(buf);
	}
	if (Unchanging) you_can("not change from your current form");
	if (Fast) you_are(Very_fast ? "very fast" : "fast");
	if (Reflecting) you_have("reflection");
	if (Free_action) you_have("free action");
	if (Fixed_abil) you_have("fixed abilities");
	if (Lifesaved)
		enl_msg("Your life ", "will be", "would have been", " saved");
	if (u.twoweap) you_are("wielding two weapons at once");

	/*** Miscellany ***/
	if (Luck) {
	    ltmp = abs((int)Luck);
	    Sprintf(buf, "%s%slucky",
		    ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
		    Luck < 0 ? "un" : "");
#ifdef WIZARD
	    if (wizard || final) Sprintf(eos(buf), " (%d)", Luck);
#endif
	    you_are(buf);
	}
#ifdef WIZARD
	 else if (wizard || final) enl_msg("Your luck ", "is", "was", " zero");
#endif
	if (u.moreluck > 0) you_have("extra luck");
	else if (u.moreluck < 0) you_have("reduced luck");
	if (has_luckitem()) {
	    ltmp = stone_luck(FALSE);
	    if (ltmp <= 0)
		enl_msg("Bad luck ", "times", "timed", " out slowly for you");
	    if (ltmp >= 0)
		enl_msg("Good luck ", "times", "timed", " out slowly for you");
	}

	if (u.ugangr) {
	    Sprintf(buf, " %sangry with you",
		    u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
#ifdef WIZARD
	    if (wizard || final) Sprintf(eos(buf), " (%d)", u.ugangr);
#endif
	    enl_msg(u_gname(), " is", " was", buf);
	} else
	    /*
	     * We need to suppress this when the game is over, because death
	     * can change the value calculated by can_pray(), potentially
	     * resulting in a false claim that you could have prayed safely.
	     */
	  if (!final) {
#if 0
	    /* "can [not] safely pray" vs "could [not] have safely prayed" */
	    Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
		    final ? "have " : "", final ? "ed" : "");
#else
	    Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
#ifdef WIZARD
	    if (wizard || final) Sprintf(eos(buf), " (%d)", u.ublesscnt);
#endif
	    you_can(buf);
	}

    {
	const char *p;

	buf[0] = '\0';
	if (final < 2) {    /* still in progress, or quit/escaped/ascended */
	    p = "survived after being killed ";
	    switch (u.umortality) {
	    case 0:  p = !final ? (char *)0 : "survived";  break;
	    case 1:  Strcpy(buf, "once");  break;
	    case 2:  Strcpy(buf, "twice");  break;
	    case 3:  Strcpy(buf, "thrice");  break;
	    default: Sprintf(buf, "%d times", u.umortality);
		     break;
	    }
	} else {		/* game ended in character's death */
	    p = "are dead";
	    switch (u.umortality) {
	    case 0:  warning("dead without dying?");
	    case 1:  break;			/* just "are dead" */
	    default: Sprintf(buf, " (%d%s time!)", u.umortality,
			     ordin(u.umortality));
		     break;
	    }
	}
	if (p) enl_msg(You_, "have been killed ", p, buf);
    }
    dump_list_end();
    dump("", "");

    if (want_display) {
	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
    }
}

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns FALSE if menu cancelled (dismissed with ESC), TRUE otherwise.
 */
STATIC_OVL boolean
minimal_enlightenment()
{
	winid tmpwin;
	menu_item *selected;
	anything any;
	int genidx, n;
	char buf[BUFSZ], buf2[BUFSZ];
	static const char untabbed_fmtstr[] = "%-15s: %-12s";
	static const char untabbed_deity_fmtstr[] = "%-17s%s";
	static const char tabbed_fmtstr[] = "%s:\t%-12s";
	static const char tabbed_deity_fmtstr[] = "%s\t%s";
	static const char *fmtstr;
	static const char *deity_fmtstr;

	fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
	deity_fmtstr = iflags.menu_tab_sep ?
			tabbed_deity_fmtstr : untabbed_deity_fmtstr; 
	any.a_void = 0;
	buf[0] = buf2[0] = '\0';
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Starting", FALSE);

	/* Starting name, race, role, gender */
	Sprintf(buf, fmtstr, "name", plname);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "race", urace.noun);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "role",
		(flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Starting alignment */
	Sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Current name, race, role, gender */
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Current", FALSE);
	Sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	if (Upolyd) {
	    Sprintf(buf, fmtstr, "role (base)",
		(u.mfemale && urole.name.f) ? urole.name.f : urole.name.m);
	    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	} else {
	    Sprintf(buf, fmtstr, "role",
		(flags.female && urole.name.f) ? urole.name.f : urole.name.m);
	    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	}
	/* don't want poly_gender() here; it forces `2' for non-humanoids */
	genidx = is_neuter(youmonst.data) ? 2 : flags.female;
	Sprintf(buf, fmtstr, "gender", genders[genidx].adj);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	if (Upolyd && (int)u.mfemale != genidx) {
	    Sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
	    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	}

	/* Current alignment */
	Sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Current position of hero */
	if (wizard) {
		Sprintf(buf2, "(%2d,%2d)", u.ux, u.uy);
		Sprintf(buf, fmtstr, "position", buf2);
		add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
	}

	/* Deity list */
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Deities", FALSE);
	Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type
		&& u.ualign.type == A_CHAOTIC) ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_CHAOTIC)       ? " (s)" :
	    (u.ualign.type   == A_CHAOTIC)       ? " (c)" : "");
	Sprintf(buf, fmtstr, "Chaotic", buf2);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type
		&& u.ualign.type == A_NEUTRAL) ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_NEUTRAL)       ? " (s)" :
	    (u.ualign.type   == A_NEUTRAL)       ? " (c)" : "");
	Sprintf(buf, fmtstr, "Neutral", buf2);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type &&
		u.ualign.type == A_LAWFUL)  ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_LAWFUL)        ? " (s)" :
	    (u.ualign.type   == A_LAWFUL)        ? " (c)" : "");
	Sprintf(buf, fmtstr, "Lawful", buf2);
	add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

	end_menu(tmpwin, "Base Attributes");
	n = select_menu(tmpwin, PICK_NONE, &selected);
	destroy_nhwindow(tmpwin);
	return (n != -1);
}

int
do_naming(typ)
int typ;
{
    winid win;
    anything any;
    menu_item *pick_list = NULL;
    int n;
    register struct obj *obj;
    char allowall[2];
    static const char callable[] = {
	SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
	GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0 };

    if (!typ) {
      any.a_void = 0;
      win = create_nhwindow(NHW_MENU);
      start_menu(win);

      /* the accelerator keys are chosen to be compatible with NAO and
       * where possible similar to AceHack's keys. That explains the
       * choice for the group accelerators. */
      any.a_int = 1;
      add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 'a', 'C', ATR_NONE, "Name a monster", MENU_UNSELECTED);

      any.a_int = 2;
      add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 'b', 'y', ATR_NONE, "Name an individual item", MENU_UNSELECTED);

      any.a_int = 3;
      add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 'c', 'n', ATR_NONE, "Name all items of a certain type", MENU_UNSELECTED);

      any.a_int = 4;
      add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 'd', 'f', ATR_NONE, "Annotate the current level", MENU_UNSELECTED);

      any.a_int = 0;
      add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 'q', ATR_NONE, "", MENU_UNSELECTED);

      end_menu(win, "What do you wish to name?");
      n = select_menu(win, PICK_ONE, &pick_list);
      destroy_nhwindow(win);

      if (pick_list) {
	n = (pick_list[0].item.a_int - 1);
	free((genericptr_t) pick_list);
      } else return 0;
    } else {
      n = (typ - 1);
    }
    switch (n) {
    default: break;
    case 0: do_mname(); break;
	/* cases 1 & 2 duplicated from ddocall() */
    case 1:
	allowall[0] = ALL_CLASSES; allowall[1] = '\0';
	obj = getobj(allowall, "name");
	if(obj) do_oname(obj);
	break;
    case 2:
	obj = getobj(callable, "call");
	if (obj) {
	    /* behave as if examining it in inventory;
                           this might set dknown if it was picked up
                           while blind and the hero can now see */
	    (void) xname(obj);

	    if (!obj->dknown) {
		You("would never recognize another one.");
		return 0;
	    }
	    docall(obj);
	}
	break;
    case 3:
	donamelevel();
	break;
    }
    return 0;
}

int
do_naming_mname()
{
  if (iflags.vanilla_ui_behavior) return do_naming(1);
  return do_naming(0);
}

int
do_naming_ddocall()
{
  return do_naming(0);
}

STATIC_PTR int
doattributes()
{
	if (!minimal_enlightenment())
		return 0;
	if (wizard || discover)
		enlightenment(0, TRUE);
	return 0;
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
STATIC_PTR int
doconduct()
{
	show_conduct(0, TRUE);
	return 0;
}

void
show_conduct(final, want_disp)
int final;
boolean want_disp;
{
	char buf[BUFSZ];
	int ngenocided;
	int cdt;

	want_display = want_disp;

	/* Create the conduct window */
	Sprintf(buf, "Voluntary challenges:");
	if (want_display) {
		en_win = create_nhwindow(NHW_MENU);
		putstr(en_win, 0, buf);
		putstr(en_win, 0, "");
	}
	dump_title(buf);
	dump_list_start();

	if (heaven_or_hell_mode) {
		if (hell_and_hell_mode) {
			you_have_been("following the path of Hell and Hell");
		} else {
			you_have_been("following the path of Heaven or Hell");
		}
	}

	if (flags.deathdropless)
		you_have_been("ignoring all death drops");

	/* list all major conducts */

	for(cdt=FIRST_CONDUCT; cdt<=LAST_CONDUCT; cdt++){
	    if(successful_cdt(cdt)){
		if (!superfluous_cdt(cdt))
		    enl_msg(conducts[cdt].prefix, 	/* "You "	*/
			conducts[cdt].presenttxt,	/* "have been"	*/
			conducts[cdt].pasttxt,		/* "were"	*/
			conducts[cdt].suffix);		/* "a pacifist"	*/
	    } else if(intended_cdt(cdt)){
		you_have_X(conducts[cdt].failtxt);	/* "pretended to be a pacifist" */
	    }
	}

	if (failed_cdt(CONDUCT_PACIFISM) || failed_cdt(CONDUCT_SADISM)){
	    if (u.uconduct.killer == 0){
		you_have_never("killed a creature");
	    } else {
		Sprintf(buf, "killed %ld creature%s", u.uconduct.killer,
			plur(u.uconduct.killer));
		you_have_X(buf);
	    }
	}

	/* now list the remaining statistical details */

	if (!u.uconduct.weaphit)
	    you_have_never("hit with a wielded weapon");
	else if (wizard || final) {
	    Sprintf(buf, "used a wielded weapon %ld time%s",
		    u.uconduct.weaphit, plur(u.uconduct.weaphit));
	    you_have_X(buf);
	}

#ifdef WIZARD
	if ((wizard || final) && u.uconduct.literate){
	    Sprintf(buf, "read items or engraved %ld time%s",
		    u.uconduct.literate, plur(u.uconduct.literate));
	    you_have_X(buf);
	}
	if ((wizard || final) && u.uconduct.armoruses) {
	    Sprintf(buf, "put on armor %ld time%s",
		  u.uconduct.armoruses, plur(u.uconduct.armoruses));
	    you_have_X(buf);
	}
#endif
	if (!u.uconduct.non_racial_armor &&
	    /* only show when armor was worn at all */
	    u.uconduct.armoruses > 0) {
	    Sprintf(buf, "wearing only %s armor", urace.adj);
	    you_have_been(buf);
	}

	ngenocided = num_genocides();
	if (ngenocided == 0) {
	    you_have_never("genocided any monsters");
	} else {
	    Sprintf(buf, "genocided %d type%s of monster%s",
		    ngenocided, plur(ngenocided), plur(ngenocided));
	    you_have_X(buf);
	}

	if (!u.uconduct.polypiles)
	    you_have_never("polymorphed an object");
#ifdef WIZARD
	else if (wizard || final) {
	    Sprintf(buf, "polymorphed %ld item%s",
		    u.uconduct.polypiles, plur(u.uconduct.polypiles));
	    you_have_X(buf);
	}
#endif

	if (!u.uconduct.polyselfs)
	    you_have_never("changed form");
#ifdef WIZARD
	else if (wizard || final) {
	    Sprintf(buf, "changed form %ld time%s",
		    u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
	    you_have_X(buf);
	}
#endif

	if (!u.uconduct.wishes)
	    you_have_X("used no wishes");
	else {
	    Sprintf(buf, "used %ld wish%s",
		    u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
	    you_have_X(buf);

	    if (u.uconduct.wishmagic) {
		    Sprintf(buf, "used %ld wish%s for magical items",
				    u.uconduct.wishmagic, (u.uconduct.wishmagic > 1L) ? "es" : "");
		    you_have_X(buf);
	    }

	    if (!u.uconduct.wisharti)
		enl_msg(You_, "have not wished", "did not wish",
			" for any artifacts");
	}

#ifdef ELBERETH_CONDUCT
	/* no point displaying the conduct if Elbereth doesn't do anything */
	if (flags.elberethignore) {
	    you_have_been("ignored by Elbereth");
	} else {
	    if (u.uconduct.elbereths) {
		Sprintf(buf, "engraved Elbereth %ld time%s",
			u.uconduct.elbereths, plur(u.uconduct.elbereths));
		you_have_X(buf);
	    } else {
		you_have_never("engraved Elbereth");
	    }
	}
#endif /* ELBERETH_CONDUCT */

	if ((wizard || final) && !u.uconduct.bones) {
	    you_have_never("encountered a bones level");
	} else if (wizard || final) {
	    Sprintf(buf, "encountered %ld bones level%s",
		    u.uconduct.bones, plur(u.uconduct.bones));
	    you_have_X(buf);
	}

#ifdef RECORD_ACHIEVE
	if ((wizard || final) && !u.uconduct.sokoban) {
		you_have_never("used any Sokoban shortcuts");
	} else if (wizard || final) {
		Sprintf(buf, "used Sokoban shortcuts %ld time%s",
			u.uconduct.sokoban, plur(u.uconduct.sokoban));
		you_have_X(buf);
	}
#endif

	dump_list_end();
	dump("", "");

	/* Pop up the window and wait for a key */
	if (want_display) {
		display_nhwindow(en_win, TRUE);
		destroy_nhwindow(en_win);
	}
}

#ifndef M
# ifndef NHSTDC
#  define M(c)		(0x80 | (c))
# else
#  define M(c)		((c) - 128)
# endif /* NHSTDC */
#endif
#ifndef C
#define C(c)		(0x1f & (c))
#endif

static const struct func_tab cmdlist[] = {
	{C('d'), FALSE, dokick, NULL}, /* "D" is for door!...?  Msg is in dokick.c */
	{C('e'), TRUE, doengrave_elbereth, NULL},
#ifdef WIZARD
	{C('f'), TRUE, wiz_map, NULL},
	{C('g'), TRUE, wiz_genesis, NULL},
	{C('i'), TRUE, wiz_identify, NULL},
#endif
	{C('l'), TRUE, doredraw, NULL}, /* if number_pad is set */
	{C('n'), TRUE, donamelevel, NULL}, /* if number_pad is set */
	{C('o'), TRUE, dooverview_or_wiz_where, NULL}, /* depending on wizard status */
	{C('p'), TRUE, doprev_message, NULL},
	{C('r'), TRUE, doredraw, NULL},
	{C('t'), TRUE, dotele, NULL},
#ifdef WIZARD
	{C('v'), TRUE, wiz_level_tele, NULL},
	{C('w'), TRUE, wiz_wish, NULL},
#else
	{C('v'), TRUE, tutorial_redisplay, NULL},
#endif
	{C('x'), TRUE, doattributes, NULL},
#ifdef SUSPEND
	{C('z'), TRUE, dosuspend, NULL},
#endif
	{'a', FALSE, doapply, NULL},
	{'A', FALSE, doddoremarm, NULL},
	{M('a'), TRUE, doorganize, NULL},
/*	'b', 'B' : go sw */
	{'c', FALSE, doclose, NULL},
	{'C', TRUE, do_naming_mname, NULL},
	{M('c'), TRUE, dotalk, NULL},
	{'d', FALSE, dodrop, NULL},
	{'D', FALSE, doddrop, NULL},
	{M('d'), FALSE, dodip, NULL},
	{'e', FALSE, doeat, NULL},
	{'E', FALSE, doengrave, NULL},
	{M('e'), TRUE, enhance_weapon_skill, NULL},
	{'f', FALSE, dofire, NULL},
/*	'F' : fight (one time) */
	{M('f'), FALSE, doforce, NULL},
/*	'g', 'G' : multiple go */
/*	'h', 'H' : go west */
	{'h', TRUE, dohelp, NULL}, /* if number_pad is set */
	{'i', TRUE, ddoinv, NULL},
	{'I', TRUE, dotypeinv, NULL},		/* Robert Viduya */
	{M('i'), TRUE, doinvoke, NULL},
/*	'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
	{'j', FALSE, dojump, NULL}, /* if number_pad is on */
	{M('j'), FALSE, dojump, NULL},
	{'k', FALSE, dokick, NULL}, /* if number_pad is on */
	{'l', FALSE, doloot, NULL}, /* if number_pad is on */
	{M('l'), FALSE, doloot, NULL},
/*	'n' prefixes a count if number_pad is on */
	{M('m'), TRUE, domonability, NULL},
	{'N', TRUE, ddocall, NULL}, /* if number_pad is on */
	{M('n'), TRUE, ddocall, NULL},
	{M('N'), TRUE, ddocall, NULL},
	{'o', FALSE, doopen, NULL},
	{'O', TRUE, doset, NULL},
	{M('o'), FALSE, dosacrifice, NULL},
	{'p', FALSE, dopay, NULL},
	{'P', FALSE, doputon, NULL},
	{M('p'), TRUE, dopray, NULL},
	{'q', FALSE, dodrink, NULL},
	{'Q', FALSE, dowieldquiver, NULL},
	{M('q'), TRUE, done2, NULL},
	{'r', FALSE, doread, NULL},
	{'R', FALSE, doremring, NULL},
	{M('r'), FALSE, dorub, NULL},
	{'s', TRUE, dosearch, "searching"},
	{'S', TRUE, dosave, NULL},
	{M('s'), FALSE, dosit, NULL},
	{'t', FALSE, dothrow, NULL},
	{'T', FALSE, dotakeoff, NULL},
	{M('t'), TRUE, doturn, NULL},
/*	'u', 'U' : go ne */
	{'u', FALSE, dountrap, NULL}, /* if number_pad is on */
	{M('u'), FALSE, dountrap, NULL},
	{'v', TRUE, doautoexplore, NULL},
	{'V', TRUE, doversion, NULL},
	{M('v'), TRUE, doextversion, NULL},
	{'w', FALSE, dowield, NULL},
	{'W', FALSE, dowear, NULL},
	{M('w'), FALSE, dowipe, NULL},
	{'x', FALSE, doswapweapon, NULL},
	{'X', FALSE, dotwoweapon, NULL},
	{M('x'), TRUE, enter_explore_mode, NULL},
/*	'y', 'Y' : go nw */
	{'z', FALSE, dozap, NULL},
	{'Z', TRUE, docast, NULL},
	{'<', FALSE, doup, NULL},
	{'>', FALSE, dodown, NULL},
	{'/', TRUE, dowhatis, NULL},
	{'&', TRUE, dowhatdoes, NULL},
	{'?', TRUE, dohelp, NULL},
	{M('?'), TRUE, doextlist, NULL},
#ifdef SHELL
	{'!', TRUE, dosh, NULL},
#endif
	{'.', TRUE, donull, "waiting"},
	{' ', TRUE, donull, "waiting"},
	{',', FALSE, dopickup, NULL},
	{':', TRUE, dolook, NULL},
	{';', TRUE, doquickwhatis, NULL},
	{'^', TRUE, doidtrap, NULL},
	{'\\', TRUE, dodiscovered, NULL},		/* Robert Viduya */
	{'@', TRUE, dotogglepickup, NULL},
	{M('2'), FALSE, dotwoweapon, NULL},
	{WEAPON_SYM,  TRUE, doprwep, NULL},
	{ARMOR_SYM,  TRUE, doprarm, NULL},
	{RING_SYM,  TRUE, doprring, NULL},
	{AMULET_SYM, TRUE, dopramulet, NULL},
	{TOOL_SYM, TRUE, doprtool, NULL},
	{'*', TRUE, doprinuse, NULL},	/* inventory of all equipment in use */
	{GOLD_SYM, TRUE, doprgold, NULL},
	{SPBOOK_SYM, TRUE, dovspell, NULL},			/* Mike Stephenson */
	{'#', TRUE, doextcmd, NULL},
	{'_', TRUE, dotravel, NULL},
	{'\'', TRUE, doautofight, NULL},
	{0,0,0,0}
};

struct ext_func_tab extcmdlist[] = {
	{"adjust", "adjust inventory letters", doorganize, TRUE},
	{"annotate", "name current level", donamelevel, TRUE},
	{"chat", "talk to someone", dotalk, TRUE},	/* converse? */
	{"conduct", "list which challenges you have adhered to", doconduct, TRUE},
	{"dip", "dip an object into something", dodip, FALSE},
	{"enhance", "advance or check weapons skills", enhance_weapon_skill,
							TRUE},
	{"force", "force a lock", doforce, FALSE},
	{"invoke", "invoke an object's powers", doinvoke, TRUE},
	{"jump", "jump to a location", dojump, FALSE},
	{"loot", "loot a box on the floor", doloot, FALSE},
	{"monster", "use a monster's special ability", domonability, TRUE},
	{"name", "name an item or type of object", do_naming_ddocall, TRUE},
	{"offer", "offer a sacrifice to the gods", dosacrifice, FALSE},
	{"overview", "show an overview of the dungeon", dooverview, TRUE},
	{"pray", "pray to the gods for help", dopray, TRUE},
	{"quit", "exit without saving current game", done2, TRUE},
	{"ride", "ride (or stop riding) a monster", doride, FALSE},
	{"rub", "rub a lamp or a stone", dorub, FALSE},
#ifdef DUMP_LOG
	{"screenshot", "output current map to a html file", dump_screenshot, FALSE},
#endif
#ifdef LIVELOG_SHOUT
	{"shout", "shout something", doshout, FALSE},
#endif
	{"sit", "sit down", dosit, FALSE},
	{"tip", "empty a container of its contents", dotip, FALSE},
	{"turn", "turn undead", doturn, TRUE},
	{"twoweapon", "toggle two-weapon combat", dotwoweapon, FALSE},
	{"untrap", "untrap something", dountrap, FALSE},
	{"vanquished", "show list of monsters killed", dovanquished, TRUE},
	{"version", "list compile time options for this version of UnNetHack",
		doextversion, TRUE},
	{"wipe", "wipe off your face", dowipe, FALSE},
	{"xplore", "enter the explore mode", enter_explore_mode, TRUE},
	{"?", "get this list of extended commands", doextlist, TRUE},
#if defined(WIZARD)
	/*
	 * There must be a blank entry here for every entry in the table
	 * below.
	 */
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef DEBUG_MIGRATING_MONS
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef PORT_DEBUG
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE}, /* showkills (showborn patch) */
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef DEBUG
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE}	/* sentinel */
};

#if defined(WIZARD)
static const struct ext_func_tab debug_extcmdlist[] = {
	{"levelchange", "change experience level", wiz_level_change, TRUE},
	{"lightsources", "show mobile light sources", wiz_light_sources, TRUE},
	{"mazewalkmap", "show MAZEWALK paths", wiz_mazewalkmap, TRUE},
#ifdef DEBUG_MIGRATING_MONS
	{"migratemons", "migrate n random monsters", wiz_migrate_mons, TRUE},
#endif
	{"monpolycontrol", "control monster polymorphs", wiz_mon_polycontrol, TRUE},
	{"panic", "test panic routine (fatal to game)", wiz_panic, TRUE},
	{"polyself", "polymorph self", wiz_polyself, TRUE},
#ifdef PORT_DEBUG
	{"portdebug", "wizard port debug command", wiz_port_debug, TRUE},
#endif
	{"rooms", "show room numbers", wiz_show_rooms, TRUE},
	{"seenv", "show seen vectors", wiz_show_seenv, TRUE},
	{"showkills", "show list of monsters killed", wiz_showkills, TRUE},
	{"stats", "show memory statistics", wiz_show_stats, TRUE},
	{"timeout", "look at timeout queue", wiz_timeout_queue, TRUE},
	{"vision", "show vision array", wiz_show_vision, TRUE},
#ifdef DEBUG
	{"wizdebug", "wizard debug command", wiz_debug_cmd, TRUE},
#endif
	{"wmode", "show wall modes", wiz_show_wmodes, TRUE},
	{(char *)0, (char *)0, donull, TRUE}
};

/*
 * Insert debug commands into the extended command list.  This function
 * assumes that the last entry will be the help entry.
 *
 * You must add entries in ext_func_tab every time you add one to the
 * debug_extcmdlist().
 */
void
add_debug_extended_commands()
{
	int i, j, k, n;

	/* count the # of help entries */
	for (n = 0; extcmdlist[n].ef_txt[0] != '?'; n++)
	    ;

	for (i = 0; debug_extcmdlist[i].ef_txt; i++) {
	    for (j = 0; j < n; j++)
		if (strcmp(debug_extcmdlist[i].ef_txt, extcmdlist[j].ef_txt) < 0) break;

	    /* insert i'th debug entry into extcmdlist[j], pushing down  */
	    for (k = n; k >= j; --k)
		extcmdlist[k+1] = extcmdlist[k];
	    extcmdlist[j] = debug_extcmdlist[i];
	    n++;	/* now an extra entry */
	}
}


static const char template[] = "%-18s %4ld  %6ld";
static const char count_str[] = "                   count  bytes";
static const char separator[] = "------------------ -----  ------";

STATIC_OVL void
count_obj(chain, total_count, total_size, top, recurse)
	struct obj *chain;
	long *total_count;
	long *total_size;
	boolean top;
	boolean recurse;
{
	long count, size;
	struct obj *obj;

	for (count = size = 0, obj = chain; obj; obj = obj->nobj) {
	    if (top) {
		count++;
		size += sizeof(struct obj) + obj->oxlth + obj->onamelth;
	    }
	    if (recurse && obj->cobj)
		count_obj(obj->cobj, total_count, total_size, TRUE, TRUE);
	}
	*total_count += count;
	*total_size += size;
}

STATIC_OVL void
obj_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct obj *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;

	count_obj(chain, &count, &size, TRUE, FALSE);
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
mon_invent_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct monst *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;
	struct monst *mon;

	for (mon = chain; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, TRUE, FALSE);
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
contained(win, src, total_count, total_size)
	winid win;
	const char *src;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;
	struct monst *mon;

	count_obj(invent, &count, &size, FALSE, TRUE);
	count_obj(fobj, &count, &size, FALSE, TRUE);
	count_obj(level.buriedobjlist, &count, &size, FALSE, TRUE);
	count_obj(migrating_objs, &count, &size, FALSE, TRUE);
	/* DEADMONSTER check not required in this loop since they have no inventory */
	for (mon = fmon; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, FALSE, TRUE);
	for (mon = migrating_mons; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, FALSE, TRUE);

	*total_count += count; *total_size += size;

	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
mon_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct monst *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count, size;
	struct monst *mon;

	for (count = size = 0, mon = chain; mon; mon = mon->nmon) {
	    count++;
	    size += sizeof(struct monst) + mon->mxlth + mon->mnamelth;
	}
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

static int
wiz_mazewalkmap()
{
	winid win;
	int x, y;
	char row[COLNO+1];

	win = create_nhwindow(NHW_TEXT);

	for (y = 0; y < ROWNO; y++) {
	    for (x = 0; x < COLNO; x++)
		row[x] = SpLev_Map[x][y] ? '1' : '.';
	    if (y == u.uy)
		row[u.ux] = '@';
	    row[x] = '\0';
	    putstr(win, 0, row);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}


/*
 * Display memory usage of all monsters and objects on the level.
 */
static int
wiz_show_stats()
{
	char buf[BUFSZ];
	winid win;
	long total_obj_size = 0, total_obj_count = 0;
	long total_mon_size = 0, total_mon_count = 0;

	win = create_nhwindow(NHW_TEXT);
	putstr(win, 0, "Current memory statistics:");
	putstr(win, 0, "");
	Sprintf(buf, "Objects, size %d", (int) sizeof(struct obj));
	putstr(win, 0, buf);
	putstr(win, 0, "");
	putstr(win, 0, count_str);

	obj_chain(win, "invent", invent, &total_obj_count, &total_obj_size);
	obj_chain(win, "fobj", fobj, &total_obj_count, &total_obj_size);
	obj_chain(win, "buried", level.buriedobjlist,
				&total_obj_count, &total_obj_size);
	obj_chain(win, "migrating obj", migrating_objs,
				&total_obj_count, &total_obj_size);
	mon_invent_chain(win, "minvent", fmon,
				&total_obj_count,&total_obj_size);
	mon_invent_chain(win, "migrating minvent", migrating_mons,
				&total_obj_count, &total_obj_size);

	contained(win, "contained",
				&total_obj_count, &total_obj_size);

	putstr(win, 0, separator);
	Sprintf(buf, template, "Total", total_obj_count, total_obj_size);
	putstr(win, 0, buf);

	putstr(win, 0, "");
	putstr(win, 0, "");
	Sprintf(buf, "Monsters, size %d", (int) sizeof(struct monst));
	putstr(win, 0, buf);
	putstr(win, 0, "");

	mon_chain(win, "fmon", fmon,
				&total_mon_count, &total_mon_size);
	mon_chain(win, "migrating", migrating_mons,
				&total_mon_count, &total_mon_size);

	putstr(win, 0, separator);
	Sprintf(buf, template, "Total", total_mon_count, total_mon_size);
	putstr(win, 0, buf);

	display_nhwindow(win, FALSE);
	destroy_nhwindow(win);
	return 0;
}

void
sanity_check()
{
	obj_sanity_check();
	timer_sanity_check();
}

#ifdef DEBUG_MIGRATING_MONS
static int
wiz_migrate_mons()
{
	int mcount = 0;
	char inbuf[BUFSZ];
	struct permonst *ptr;
	struct monst *mtmp;
	d_level tolevel;
	getlin("How many random monsters to migrate? [0]", inbuf);
	if (*inbuf == '\033') return 0;
	mcount = atoi(inbuf);
	if (mcount < 0 || mcount > (COLNO * ROWNO) || Is_botlevel(&u.uz))
		return 0;
	while (mcount > 0) {
		if (Is_stronghold(&u.uz))
		    assign_level(&tolevel, &valley_level);
		else
		    get_level(&tolevel, depth(&u.uz) + 1);
		ptr = rndmonst();
		mtmp = makemon(ptr, 0, 0, NO_MM_FLAGS);
		if (mtmp) migrate_to_level(mtmp, ledger_no(&tolevel),
				MIGR_RANDOM, (coord *)0);
		mcount--;
	}
	return 0;
}
#endif

#endif /* WIZARD */

#define unctrl(c)	((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c)	(0x7f & (c))


void
rhack(cmd)
register char *cmd;
{
	boolean do_walk, do_rush, prefix_seen, bad_command,
		firsttime = (cmd == 0);

	iflags.menu_requested = FALSE;
	if (firsttime) {
		flags.nopick = 0;
		cmd = parse();
	}
	if (*cmd == '\033') {
		flags.move = FALSE;
		return;
	}
	if (*cmd == DOAGAIN && !in_doagain && saveq[0]) {
		in_doagain = TRUE;
		stail = 0;
		rhack((char *)0);	/* read and execute command */
		in_doagain = FALSE;
		return;
	}
	/* Special case of *cmd == ' ' handled better below */
	if(!*cmd || *cmd == (char)0377)
	{
		nhbell();
		flags.move = FALSE;
		return;		/* probably we just had an interrupt */
	}
	if (iflags.num_pad && iflags.num_pad_mode == 1) {
		/* This handles very old inconsistent DOS/Windows behaviour
		 * in a new way: earlier, the keyboard handler mapped these,
		 * which caused counts to be strange when entered from the
		 * number pad. Now do not map them until here. 
		 */
		switch (*cmd) {
		    case '5':       *cmd = 'g'; break;
		    case M('5'):    *cmd = 'G'; break;
		    case M('0'):    *cmd = 'I'; break;
        	}
        }
	/* handle most movement commands */
	do_walk = do_rush = prefix_seen = FALSE;
	flags.travel = iflags.travel1 = 0;
	switch (*cmd) {
	 case 'g':  if (movecmd(cmd[1])) {
			flags.run = 2;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '5':  if (!iflags.num_pad) break;	/* else FALLTHRU */
	 case 'G':  if (movecmd(lowc(cmd[1]))) {
			flags.run = 3;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '-':  if (!iflags.num_pad) break;	/* else FALLTHRU */
	/* Effects of movement commands and invisible monsters:
	 * m: always move onto space (even if 'I' remembered)
	 * F: always attack space (even if 'I' not remembered)
	 * normal movement: attack if 'I', move otherwise
	 */
	 case 'F':  if (movecmd(cmd[1])) {
			flags.forcefight = 1;
			do_walk = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case 'm':  if (movecmd(cmd[1]) || u.dz) {
			flags.run = 0;
			flags.nopick = 1;
			if (!u.dz) do_walk = TRUE;
			else cmd[0] = cmd[1];	/* "m<" or "m>" */
		    } else
			prefix_seen = TRUE;
		    break;
	 case 'M':  if (movecmd(lowc(cmd[1]))) {
			flags.run = 1;
			flags.nopick = 1;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '0':  if (!iflags.num_pad) break;
		    (void)ddoinv(); /* a convenience borrowed from the PC */
		    flags.move = FALSE;
		    multi = 0;
		    return;
	 case CMD_TRAVEL:
		    if (iflags.travelcmd) {
			    flags.travel = 1;
			    iflags.travel1 = 1;
			    flags.run = 8;
			    flags.nopick = !iflags.autoexplore;
			    do_rush = TRUE;
			    break;
		    }
		    /*FALLTHRU*/
	 default:   if (movecmd(*cmd)) {	/* ordinary movement */
			flags.run = 0;	/* only matters here if it was 8 */
			do_walk = TRUE;
		    } else if (movecmd(iflags.num_pad ?
				       unmeta(*cmd) : lowc(*cmd))) {
			flags.run = 1;
			do_rush = TRUE;
		    } else if (movecmd(unctrl(*cmd))) {
			flags.run = 3;
			do_rush = TRUE;
		    }
		    break;
	}

	/* some special prefix handling */
	/* overload 'm' prefix for ',' to mean "request a menu" */
	if (prefix_seen && cmd[1] == ',') {
		iflags.menu_requested = TRUE;
		++cmd;
	}

	if (do_walk) {
	    if (multi) flags.mv = TRUE;
	    check_tutorial_command('m');
	    domove();
	    flags.forcefight = 0;
	    return;
	} else if (do_rush) {
	    if (firsttime) {
		if (!multi) multi = max(COLNO,ROWNO);
		u.last_str_turn = 0;
	    }
	    flags.mv = TRUE;
	    check_tutorial_command('G');
	    domove();
	    return;
	} else if (prefix_seen && cmd[1] == '\033') {	/* <prefix><escape> */
	    /* don't report "unknown command" for change of heart... */
	    bad_command = FALSE;
	} else if (*cmd == ' ' && !iflags.rest_on_space) {
	    bad_command = TRUE;		/* skip cmdlist[] loop */

	/* handle all other commands */
	} else {
	    register const struct func_tab *tlist;
	    int res, NDECL((*func));
#ifdef QWERTZ
	    unsigned char cmdchar = *cmd & 0xff;
#endif

	    for (tlist = cmdlist; tlist->f_char; tlist++) {
#ifdef QWERTZ
		if(C(cmdchar)==C('y') && iflags.qwertz_layout)
			cmdchar+='z'-'y';
		if (cmdchar != (tlist->f_char & 0xff)) continue;
#else
		if ((*cmd & 0xff) != (tlist->f_char & 0xff)) continue;
#endif
		check_tutorial_command(*cmd & 0xff);

		if (u.uburied && !tlist->can_if_buried) {
		    You_cant("do that while you are buried!");
		    res = 0;
		} else {
		    /* we discard 'const' because some compilers seem to have
		       trouble with the pointer passed to set_occupation() */
		    func = ((struct func_tab *)tlist)->f_funct;
		    if (tlist->f_text && !occupation && multi)
			set_occupation(func, tlist->f_text, multi);
		    /* remember pressed character */
		    last_cmd_char = *cmd;
		    res = (*func)();		/* perform the command */
		}
		if (!res) {
		    flags.move = FALSE;
		    multi = 0;
		}
		return;
	    }
	    /* if we reach here, cmd wasn't found in cmdlist[] */
	    bad_command = TRUE;
	}

	if (bad_command) {
	    char expcmd[10];
	    register char *cp = expcmd;

	    while (*cmd && (int)(cp - expcmd) < (int)(sizeof expcmd - 3)) {
		if (*cmd >= 040 && *cmd < 0177) {
		    *cp++ = *cmd++;
		} else if (*cmd & 0200) {
		    *cp++ = 'M';
		    *cp++ = '-';
		    *cp++ = *cmd++ &= ~0200;
		} else {
		    *cp++ = '^';
		    *cp++ = *cmd++ ^ 0100;
		}
	    }
	    *cp = '\0';
	    if (!prefix_seen || !iflags.cmdassist ||
		!help_dir(0, "Invalid direction key!"))
		Norep("Unknown command '%s'.", expcmd);
	}
	/* didn't move */
	flags.move = FALSE;
	multi = 0;
	return;
}

int
xytod(x, y)	/* convert an x,y pair into a direction code */
schar x, y;
{
	register int dd;

	for(dd = 0; dd < 8; dd++)
	    if(x == xdir[dd] && y == ydir[dd]) return dd;

	return -1;
}

void
dtoxy(cc,dd)	/* convert a direction code into an x,y pair */
coord *cc;
register int dd;
{
	cc->x = xdir[dd];
	cc->y = ydir[dd];
	return;
}

int
movecmd(sym)	/* also sets u.dz, but returns false for <> */
char sym;
{
	register const char *dp;
	register const char *sdp;
	if(iflags.num_pad) sdp = ndir; else sdp = sdir;	/* DICE workaround */

	u.dz = 0;
	if(!(dp = index(sdp, sym))) return 0;
	u.dx = xdir[dp-sdp];
	u.dy = ydir[dp-sdp];
	u.dz = zdir[dp-sdp];
	if (u.dx && u.dy && u.umonnum == PM_GRID_BUG) {
		u.dx = u.dy = 0;
		return 0;
	}
	return !u.dz;
}

/*
 * uses getdir() but unlike getdir() it specifically
 * produces coordinates using the direction from getdir()
 * and verifies that those coordinates are ok.
 *
 * If the call to getdir() returns 0, Never_mind is displayed.
 * If the resulting coordinates are not okay, emsg is displayed.
 *
 * Returns non-zero if coordinates in cc are valid.
 */
int get_adjacent_loc(prompt,emsg,x,y,cc)
const char *prompt, *emsg;
xchar x,y;
coord *cc;
{
	xchar new_x, new_y;
	if (!getdir(prompt)) {
		pline("%s", Never_mind);
		return 0;
	}
	new_x = x + u.dx;
	new_y = y + u.dy;
	if (cc && isok(new_x,new_y)) {
		cc->x = new_x;
		cc->y = new_y;
	} else {
		if (emsg) pline("%s", emsg);
		return 0;
	}
	return 1;
}

int
getdir(s)
const char *s;
{
	char dirsym;
	/* saved direction of the previous call of getdir() */
	static char saved_dirsym = '\0';

	if(in_doagain || *readchar_queue)
	    dirsym = readchar();
	else
	    dirsym = yn_function ((s && *s != '^') ? s : "In what direction?",
					(char *)0, '\0');
	savech(dirsym);
	if (dirsym == last_cmd_char) {
		/* in here dirsym is not representing a direction
		 * but the same sym used before for calling the
		 * current cmd */
		movecmd(saved_dirsym);
		dirsym = saved_dirsym;
	} else if(dirsym == '.' || dirsym == 's')
		u.dx = u.dy = u.dz = 0;
	else if(!movecmd(dirsym) && !u.dz) {
		boolean did_help = FALSE;
		if(!index(quitchars, dirsym)) {
		    if (iflags.cmdassist) {
			did_help = help_dir((s && *s == '^') ? dirsym : 0,
					    "Invalid direction key!");
		    }
		    if (!did_help) pline("What a strange direction!");
		}
		return 0;
	}
	saved_dirsym = dirsym;
	if(!u.dz && (Stunned || (Confusion && !rn2(5)))) confdir();
	return 1;
}

STATIC_OVL boolean
help_dir(sym, msg)
char sym;
const char *msg;
{
	char ctrl;
	winid win;
	static const char wiz_only_list[] = "EFGIOVW";
	char buf[BUFSZ], buf2[BUFSZ], *expl;

	win = create_nhwindow(NHW_TEXT);
	if (!win) return FALSE;
	if (msg) {
		Sprintf(buf, "cmdassist: %s", msg);
		putstr(win, 0, buf);
		putstr(win, 0, "");
	}
	if (letter(sym)) { 
	    sym = highc(sym);
	    ctrl = (sym - 'A') + 1;
	    if ((expl = dowhatdoes_core(ctrl, buf2))
		&& (!index(wiz_only_list, sym)
#ifdef WIZARD
		    || wizard
#endif
	                     )) {
		Sprintf(buf, "Are you trying to use ^%c%s?", sym,
			index(wiz_only_list, sym) ? "" :
			" as specified in the Guidebook");
		putstr(win, 0, buf);
		putstr(win, 0, "");
		putstr(win, 0, expl);
		putstr(win, 0, "");
		putstr(win, 0, "To use that command, you press");
		Sprintf(buf,
			"the <Ctrl> key, and the <%c> key at the same time.", sym);
		putstr(win, 0, buf);
		putstr(win, 0, "");
	    }
	}
	if (iflags.num_pad && u.umonnum == PM_GRID_BUG) {
	    putstr(win, 0, "Valid direction keys in your current form (with number_pad on) are:");
	    putstr(win, 0, "             8   ");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "          4- . -6");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "             2   ");
	} else if (u.umonnum == PM_GRID_BUG) {
	    putstr(win, 0, "Valid direction keys in your current form are:");
	    putstr(win, 0, "             k   ");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "          h- . -l");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "             j   ");
	} else if (iflags.num_pad) {
	    putstr(win, 0, "Valid direction keys (with number_pad on) are:");
	    putstr(win, 0, "          7  8  9");
	    putstr(win, 0, "           \\ | / ");
	    putstr(win, 0, "          4- . -6");
	    putstr(win, 0, "           / | \\ ");
	    putstr(win, 0, "          1  2  3");
	} else {
	    putstr(win, 0, "Valid direction keys are:");
	    putstr(win, 0, "          y  k  u");
	    putstr(win, 0, "           \\ | / ");
	    putstr(win, 0, "          h- . -l");
	    putstr(win, 0, "           / | \\ ");
	    putstr(win, 0, "          b  j  n");
	};
	putstr(win, 0, "");
	putstr(win, 0, "          <  up");
	putstr(win, 0, "          >  down");
	putstr(win, 0, "          .  direct at yourself");
	putstr(win, 0, "");
	putstr(win, 0, "(Suppress this message with !cmdassist in config file.)");
	display_nhwindow(win, FALSE);
	destroy_nhwindow(win);
	return TRUE;
}

void
confdir()
{
	register int x = (u.umonnum == PM_GRID_BUG) ? 2*rn2(4) : rn2(8);
	u.dx = xdir[x];
	u.dy = ydir[x];
	return;
}

int
isok(x,y)
register int x, y;
{
	/* x corresponds to curx, so x==1 is the first column. Ach. %% */
	return x >= 1 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1;
}

static int last_multi;

/*
 * convert a MAP window position into a movecmd
 */
const char *
click_to_cmd(x, y, mod)
    int x, y, mod;
{
    int dir;
    static char cmd[4];
    cmd[1]=0;

    x -= u.ux;
    y -= u.uy;

    if (iflags.travelcmd) {
        iflags.autoexplore = FALSE;

        if (abs(x) <= 1 && abs(y) <= 1 ) {
            x = sgn(x), y = sgn(y);
        } else {
            u.tx = u.ux+x;
            u.ty = u.uy+y;
            cmd[0] = CMD_TRAVEL;
            return cmd;
        }

        if(x == 0 && y == 0) {
            /* here */
            if(IS_FOUNTAIN(levl[u.ux][u.uy].typ) || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0]=mod == CLICK_1 ? 'q' : M('d');
                return cmd;
            } else if(IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0]=M('s');
                return cmd;
            } else if((u.ux == xupstair && u.uy == yupstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up)
                      || (u.ux == xupladder && u.uy == yupladder)) {
                return "<";
            } else if((u.ux == xdnstair && u.uy == ydnstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)
                      || (u.ux == xdnladder && u.uy == ydnladder)) {
                return ">";
            } else if(OBJ_AT(u.ux, u.uy)) {
                cmd[0] = Is_container(level.objects[u.ux][u.uy]) ? M('l') : ',';
                return cmd;
            } else {
                return "."; /* just rest */
            }
        }

        /* directional commands */

        dir = xytod(x, y);

	if (!m_at(u.ux+x, u.uy+y) && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
            cmd[2] = 0;
            if (IS_DOOR(levl[u.ux+x][u.uy+y].typ)) {
                /* slight assistance to the player: choose kick/open for them */
                if (levl[u.ux+x][u.uy+y].doormask & D_LOCKED) {
                    cmd[0] = C('d');
                    return cmd;
                }
                if (levl[u.ux+x][u.uy+y].doormask & D_CLOSED) {
                    cmd[0] = 'o';
                    return cmd;
                }
            }
            if (levl[u.ux+x][u.uy+y].typ <= SCORR) {
                cmd[0] = 's';
                cmd[1] = 0;
                return cmd;
            }
        }
    } else {
        /* convert without using floating point, allowing sloppy clicking */
        if(x > 2*abs(y))
            x = 1, y = 0;
        else if(y > 2*abs(x))
            x = 0, y = 1;
        else if(x < -2*abs(y))
            x = -1, y = 0;
        else if(y < -2*abs(x))
            x = 0, y = -1;
        else
            x = sgn(x), y = sgn(y);

        if(x == 0 && y == 0)	/* map click on player to "rest" command */
            return ".";

        dir = xytod(x, y);
    }

    /* move, attack, etc. */
    cmd[1] = 0;
    if(mod == CLICK_1) {
	cmd[0] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
    } else {
	cmd[0] = (iflags.num_pad ? M(ndir[dir]) :
		(sdir[dir] - 'a' + 'A')); /* run command */
    }

    return cmd;
}

STATIC_OVL char *
parse()
{
#ifdef LINT	/* static char in_line[COLNO]; */
	char in_line[COLNO];
#else
	static char in_line[COLNO];
#endif
	register int foo;
	boolean prezero = FALSE;

	multi = 0;
	flags.move = 1;
	flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

	if (!iflags.num_pad || (foo = readchar()) == 'n')
	    for (;;) {
		foo = readchar();
		if (foo >= '0' && foo <= '9') {
		    multi = 10 * multi + foo - '0';
		    if (multi < 0 || multi >= LARGEST_INT) multi = LARGEST_INT;
		    if (multi > 9) {
			clear_nhwindow(WIN_MESSAGE);
			Sprintf(in_line, "Count: %d", multi);
			pline("%s", in_line);
			mark_synch();
		    }
		    last_multi = multi;
		    if (!multi && foo == '0') prezero = TRUE;
		} else break;	/* not a digit */
	    }

	if (foo == '\033') {   /* esc cancels count (TH) */
	    clear_nhwindow(WIN_MESSAGE);
	    multi = last_multi = 0;
	} else if (foo == DOAGAIN || in_doagain) {
	    multi = last_multi;
	} else {
	    last_multi = multi;
	    savech(0);	/* reset input queue */
	    savech((char)foo);
	}

	if (multi) {
	    multi--;
	    save_cm = in_line;
	} else {
	    save_cm = (char *)0;
	}
	in_line[0] = foo;
	in_line[1] = '\0';
	if (foo == 'g' || foo == 'G' || foo == 'm' || foo == 'M' ||
	    foo == 'F' || (iflags.num_pad && (foo == '5' || foo == '-'))) {
	    foo = readchar();
	    savech((char)foo);
	    in_line[1] = foo;
	    in_line[2] = 0;
	}
	clear_nhwindow(WIN_MESSAGE);
	if (prezero) in_line[0] = '\033';
	return(in_line);
}

#ifdef UNIX
static
void
end_of_input()
{
#ifndef NOSAVEONHANGUP
	if (!program_state.done_hup++ && program_state.something_worth_saving)
	    (void) dosave0();
#endif
	exit_nhwindows((char *)0);
	clearlocks();
	terminate(EXIT_SUCCESS);
}
#endif

char
readchar()
{
	register int sym;
	int x = u.ux, y = u.uy, mod = 0;

	if ( *readchar_queue )
	    sym = *readchar_queue++;
	else
	    sym = in_doagain ? Getchar() : nh_poskey(&x, &y, &mod);

#ifdef UNIX
# ifdef NR_OF_EOFS
	if (sym == EOF) {
	    register int cnt = NR_OF_EOFS;
	  /*
	   * Some SYSV systems seem to return EOFs for various reasons
	   * (?like when one hits break or for interrupted systemcalls?),
	   * and we must see several before we quit.
	   */
	    do {
		clearerr(stdin);	/* omit if clearerr is undefined */
		sym = Getchar();
	    } while (--cnt && sym == EOF);
	}
# endif /* NR_OF_EOFS */
	if (sym == EOF)
	    end_of_input();
#endif /* UNIX */

	if(sym == 0) {
	    /* click event */
	    readchar_queue = click_to_cmd(x, y, mod);
	    sym = *readchar_queue++;
	}
	return((char) sym);
}

STATIC_PTR int
dotravel()
{
	/* Keyboard travel command */
	static char cmd[2];
	coord cc;

	if (!iflags.travelcmd) return 0;
	cmd[1]=0;
	cc.x = iflags.travelcc.x;
	cc.y = iflags.travelcc.y;
	if (cc.x == -1 && cc.y == -1) {
	    /* No cached destination, start attempt from current position */
	    cc.x = u.ux;
	    cc.y = u.uy;
	}
	pline("Where do you want to travel to?");
	if (getpos(&cc, TRUE, "the desired destination") < 0) {
		/* user pressed ESC */
		return 0;
	}
	iflags.travelcc.x = u.tx = cc.x;
	iflags.travelcc.y = u.ty = cc.y;
	iflags.autoexplore = FALSE;
	cmd[0] = CMD_TRAVEL;
	readchar_queue = cmd;
	return 0;
}

STATIC_PTR int
doautoexplore()
{
	static char cmd[2];
	iflags.autoexplore = TRUE;
	cmd[0] = CMD_TRAVEL;
	readchar_queue = cmd;
	return 0;
}

/** Chooses a suitable monster and fights it. */
STATIC_PTR int
doautofight()
{
	int i,j;
	struct monst *mtmp;
	/* TODO: - selection strategy when facing multiple monsters
	 *       - fire ammunition when wielding a launcher
	 *       - fire quivered objects when monsters are in range?
	 */

	/* check surrounding squares for monster to attack */
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			if (!isok(u.ux+i, u.uy+j)) continue;
			mtmp = m_at(u.ux+i, u.uy+j);
			if (mtmp && canspotmon(mtmp) && !is_safepet(mtmp)) {
				if (attack(mtmp)) return 1;
				break;
			}
		}
	}

	return 0;
}

#ifdef PORT_DEBUG
# ifdef WIN32CON
extern void NDECL(win32con_debug_keystrokes);
extern void NDECL(win32con_handler_info);
# endif

int
wiz_port_debug()
{
	int n, k;
	winid win;
	anything any;
	int item = 'a';
	int num_menu_selections;
	struct menu_selection_struct {
		char *menutext;
		void NDECL((*fn));
	} menu_selections[] = {
#ifdef WIN32CON
		{"test win32 keystrokes", win32con_debug_keystrokes},
		{"show keystroke handler information", win32con_handler_info},
#endif
		{(char *)0, (void NDECL((*)))0}		/* array terminator */
	};

	num_menu_selections = SIZE(menu_selections) - 1;
	if (num_menu_selections > 0) {
		menu_item *pick_list;
		win = create_nhwindow(NHW_MENU);
		start_menu(win);
		for (k=0; k < num_menu_selections; ++k) {
			any.a_int = k+1;
			add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, item++, 0, ATR_NONE,
				menu_selections[k].menutext, MENU_UNSELECTED);
		}
		end_menu(win, "Which port debugging feature?");
		n = select_menu(win, PICK_ONE, &pick_list);
		destroy_nhwindow(win);
		if (n > 0) {
			n = pick_list[0].item.a_int - 1;
			free((genericptr_t) pick_list);
			/* execute the function */
			(*menu_selections[n].fn)();
		}
	} else
		pline("No port-specific debug capability defined.");
	return 0;
}
# endif /*PORT_DEBUG*/

/*
 *   Parameter validator for generic yes/no function to prevent
 *   the core from sending too long a prompt string to the
 *   window port causing a buffer overflow there.
 */
char
yn_function(query,resp, def)
const char *query,*resp;
char def;
{
	char qbuf[QBUFSZ];
	unsigned truncspot, reduction = sizeof(" [N]  ?") + 1;

	if (resp) reduction += strlen(resp) + sizeof(" () ");
	if (strlen(query) < (QBUFSZ - reduction))
		return (*windowprocs.win_yn_function)(query, resp, def);
	paniclog("Query truncated: ", query);
	reduction += sizeof("...");
	truncspot = QBUFSZ - reduction;
	(void) strncpy(qbuf, query, (int)truncspot);
	qbuf[truncspot] = '\0';
	Strcat(qbuf,"...");
	return (*windowprocs.win_yn_function)(qbuf, resp, def);
}

/**
 * Asks the player a yes/no question if paranoid is true.
 * @return 'y' or 'n'
 */
char
paranoid_yn(query,paranoid)
const char *query;
boolean paranoid;
{
	if (paranoid) {
		char buf[BUFSZ];
		char query_yesno[2*BUFSZ];
		/* put [yes/no] between question and question mark? */
		Sprintf(query_yesno, "%s [yes/no]", query);
		getlin (query_yesno, buf);
		(void) lcase (buf);
		return (!(strcmp (buf, "yes"))) ? 'y' : 'n';
	} else {
		return yn(query);
	}
}

/**
 * Tracks how many times the player "cheated" in Sokoban.
 *
 * Stops tracking after the player found the sokoban prize.
 */
void
sokoban_trickster()
{
#ifdef RECORD_ACHIEVE
	if (!achieve.finish_sokoban) {
		/* not yet found the sokoban prize */
		u.uconduct.sokoban += 1;
	}
#endif
}

/*cmd.c*/
