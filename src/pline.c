/*	SCCS Id: @(#)pline.c	3.4	1999/11/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS /* Uses ... */	/* comment line for pre-compiled headers */
#include "hack.h"
#include "epri.h"
#ifdef WIZARD
#include "edog.h"
#include "eshk.h"
#endif

static boolean no_repeat = FALSE;

static char *FDECL(You_buf, (int));

#if defined(DUMP_LOG) && defined(DUMPMSGS)
char msgs[DUMPMSGS][BUFSZ];
int msgs_count[DUMPMSGS];
int lastmsg = -1;
#endif

void
msgpline_add(typ, pattern)
     int typ;
     char *pattern;
{
    struct _plinemsg *tmp = (struct _plinemsg *) alloc(sizeof(struct _plinemsg));
    if (!tmp) return;
    tmp->msgtype = typ;
    tmp->pattern = strdup(pattern);
    tmp->next = pline_msg;
    pline_msg = tmp;
}

void
msgpline_free()
{
    struct _plinemsg *tmp = pline_msg;
    struct _plinemsg *tmp2;
    while (tmp) {
	free(tmp->pattern);
	tmp2 = tmp;
	tmp = tmp->next;
	free(tmp2);
    }
    pline_msg = NULL;
}

int
msgpline_type(msg)
     char *msg;
{
    struct _plinemsg *tmp = pline_msg;
    while (tmp) {
	if (pmatch(tmp->pattern, msg)) return tmp->msgtype;
	tmp = tmp->next;
    }
    return MSGTYP_NORMAL;
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vpline, (const char *, va_list));

void
pline VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vpline(line, VA_ARGS);
	VA_END();
}

char prevmsg[BUFSZ];

# ifdef USE_STDARG
static void
vpline(const char *line, va_list the_args) {
# else
static void
vpline(line, the_args) const char *line; va_list the_args; {
# endif

#else	/* USE_STDARG | USE_VARARG */

#define vpline pline

void
pline VA_DECL(const char *, line)
#endif	/* USE_STDARG | USE_VARARG */

	char pbuf[BUFSZ];
	int typ;
/* Do NOT use VA_START and VA_END in here... see above */

	if (!line || !*line) return;
	if (index(line, '%')) {
	    Vsprintf(pbuf,line,VA_ARGS);
	    line = pbuf;
	}
#if defined(DUMP_LOG) && defined(DUMPMSGS)
	if (DUMPMSGS > 0 && !program_state.gameover) {
		/* count identical messages */
		if (!strncmp(msgs[lastmsg], line, BUFSZ)) {
			msgs_count[lastmsg] += 1;
		} else {
			lastmsg = (lastmsg + 1) % DUMPMSGS;
			strncpy(msgs[lastmsg], line, BUFSZ);
			msgs_count[lastmsg] = 1;
		}
	}
#endif
	typ = msgpline_type(line);
	if (!iflags.window_inited) {
	    raw_print(line);
	    return;
	}
	if (no_repeat && !strcmp(line, toplines))
	    return;
	if (vision_full_recalc) vision_recalc(0);
	if (u.ux) flush_screen(1);		/* %% */
	if (typ == MSGTYP_NOSHOW) return;
	if (typ == MSGTYP_NOREP && !strcmp(line, prevmsg)) return;
	putstr(WIN_MESSAGE, 0, line);
	strncpy(prevmsg, line, BUFSZ);
	if (typ == MSGTYP_STOP) display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */
}

/*VARARGS1*/
void
Norep VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, const char *);
	no_repeat = TRUE;
	vpline(line, VA_ARGS);
	no_repeat = FALSE;
	VA_END();
	return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *
You_buf(siz)
int siz;
{
	if (siz > you_buf_siz) {
		if (you_buf) free((genericptr_t) you_buf);
		you_buf_siz = siz + 10;
		you_buf = (char *) alloc((unsigned) you_buf_siz);
	}
	return you_buf;
}

void
free_youbuf()
{
	if (you_buf) free((genericptr_t) you_buf),  you_buf = (char *)0;
	you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer,prefix,text) \
 Strcpy((pointer = You_buf((int)(strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer,prefix,text) \
 strcat((YouPrefix(pointer, prefix, text), pointer), text)

/*VARARGS1*/
void
You VA_DECL(const char *, line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "You ", line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
Your VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "Your ", line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
You_feel VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "You feel ", line), VA_ARGS);
	VA_END();
}


/*VARARGS1*/
void
You_cant VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "You can't ", line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
pline_The VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "The ", line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
There VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	vpline(YouMessage(tmp, "There ", line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
You_hear VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	if (Underwater)
		YouPrefix(tmp, "You barely hear ", line);
	else if (u.usleep)
		YouPrefix(tmp, "You dream that you hear ", line);
	else
		YouPrefix(tmp, "You hear ", line);
	vpline(strcat(tmp, line), VA_ARGS);
	VA_END();
}

/*VARARGS1*/
void
verbalize VA_DECL(const char *,line)
	char *tmp;
	if (!flags.soundok) return;
	VA_START(line);
	VA_INIT(line, const char *);
	tmp = You_buf((int)strlen(line) + sizeof "\"\"");
	Strcpy(tmp, "\"");
	Strcat(tmp, line);
	Strcat(tmp, "\"");
	vpline(tmp, VA_ARGS);
	VA_END();
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vraw_printf,(const char *,va_list));

void
raw_printf VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vraw_printf(line, VA_ARGS);
	VA_END();
}

# ifdef USE_STDARG
static void
vraw_printf(const char *line, va_list the_args) {
# else
static void
vraw_printf(line, the_args) const char *line; va_list the_args; {
# endif

#else  /* USE_STDARG | USE_VARARG */

void
raw_printf VA_DECL(const char *, line)
#endif
/* Do NOT use VA_START and VA_END in here... see above */

	if(!index(line, '%'))
	    raw_print(line);
	else {
	    char pbuf[BUFSZ];
	    Vsprintf(pbuf,line,VA_ARGS);
	    raw_print(pbuf);
	}
}


/*VARARGS1*/
void
impossible VA_DECL(const char *, s)
	char pbuf[2*BUFSZ];
	VA_START(s);
	VA_INIT(s, const char *);
	if (program_state.in_impossible) {
		panic("impossible called impossible");
	}

	program_state.in_impossible = 1;
	Vsprintf(pbuf, s, VA_ARGS);
	pbuf[BUFSZ-1] = '\0'; /* sanity */
	paniclog("impossible", pbuf);
	pline("%s", pbuf);
	pline("Program in disorder; you probably should S)ave and restart the process.");
	program_state.in_impossible = 0;
	VA_END();
}

void
warning VA_DECL(const char *, s)
	char pbuf[2*BUFSZ];
	VA_START(s);
	VA_INIT(s, const char *);
	Vsprintf(pbuf, s, VA_ARGS);
	pbuf[BUFSZ-1] = '\0'; /* sanity */
	paniclog("warning", pbuf);
	pline("Warning: %s\n", pbuf);
	VA_END();
}

const char *
align_str(alignment)
    aligntyp alignment;
{
    switch ((int)alignment) {
	case A_CHAOTIC: return "chaotic";
	case A_NEUTRAL: return "neutral";
	case A_LAWFUL:	return "lawful";
	case A_NONE:	return "unaligned";
    }
    return "unknown";
}

void
mstatusline(mtmp)
register struct monst *mtmp;
{
	aligntyp alignment;
	char info[BUFSZ], monnambuf[BUFSZ];

	if (mtmp->ispriest || mtmp->data == &mons[PM_ALIGNED_PRIEST]
				|| mtmp->data == &mons[PM_ANGEL])
		alignment = EPRI(mtmp)->shralign;
	else
		alignment = mtmp->data->maligntyp;
	if (alignment == A_NONE) {
	    alignment = A_NONE;
	  } else if (alignment > 0) {
	    alignment = A_LAWFUL;
	  } else if (alignment < 0) {
	    alignment = A_CHAOTIC;
	  } else {
	    alignment = A_NEUTRAL;
	  }

	info[0] = 0;
	if (mtmp->mtame) {	  Strcat(info, ", tame");
#ifdef WIZARD
	    if (wizard) {
		Sprintf(eos(info), " (%d", mtmp->mtame);
		if (!mtmp->isminion)
		    Sprintf(eos(info), "; hungry %ld; apport %d",
			EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
		Strcat(info, ")");
	    }
#endif
	}
	else if (mtmp->mpeaceful) Strcat(info, ", peaceful");
	if (mtmp->meating)	  Strcat(info, ", eating");
	if (mtmp->mcan)		  Strcat(info, ", cancelled");
	if (mtmp->mconf)	  Strcat(info, ", confused");
	if (mtmp->mblinded || !mtmp->mcansee)
				  Strcat(info, ", blind");
	if (mtmp->mstun)	  Strcat(info, ", stunned");
	if (mtmp->msleeping)	  Strcat(info, ", asleep");
#if 0	/* unfortunately mfrozen covers temporary sleep and being busy
	   (donning armor, for instance) as well as paralysis */
	else if (mtmp->mfrozen)	  Strcat(info, ", paralyzed");
#else
	else if (mtmp->mfrozen || !mtmp->mcanmove)
				  Strcat(info, ", can't move");
#endif
				  /* [arbitrary reason why it isn't moving] */
	else if (mtmp->mstrategy & STRAT_WAITMASK)
				  Strcat(info, ", meditating");
	else if (mtmp->mflee)	  Strcat(info, ", scared");
	if (mtmp->mtrapped)	  Strcat(info, ", trapped");
	if (mtmp->mfeetfrozen)	  Strcat(info, ", stuck in ice");
	if (mtmp->mspeed)	  Strcat(info,
					mtmp->mspeed == MFAST ? ", fast" :
					mtmp->mspeed == MSLOW ? ", slow" :
					", ???? speed");
	if (mtmp->mundetected)	  Strcat(info, ", concealed");
	if (mtmp->minvis)	  Strcat(info, ", invisible");
	if (mtmp == u.ustuck)	  Strcat(info,
			(sticks(youmonst.data)) ? ", held by you" :
				u.uswallow ? (is_animal(u.ustuck->data) ?
				", swallowed you" :
				", engulfed you") :
				", holding you");
	if (mtmp == u.usteed)	  Strcat(info, ", carrying you");
#ifdef WIZARD
	if (wizard &&
	    mtmp->isshk && ESHK(mtmp)->cheapskate) {
		Strcat(info, ", cheapskate");
	}
#endif

	/* avoid "Status of the invisible newt ..., invisible" */
	/* and unlike a normal mon_nam, use "saddled" even if it has a name */
	Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_THE, (char *)0,
	    (SUPPRESS_IT|SUPPRESS_INVISIBLE), FALSE));

	pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.",
		monnambuf,
		align_str(alignment),
		mtmp->m_lev,
		mtmp->mhp,
		mtmp->mhpmax,
		find_mac(mtmp),
		info);

	/* Heisenberg's code */
	if (mtmp->data == &mons[PM_QUANTUM_MECHANIC]) {
	    if (canspotmon(mtmp))
		pline("Having determined %s's speed, you are unable to know its location.",
			mon_nam(mtmp));
	    (void) rloc(mtmp, FALSE);
	} else if (mtmp->data == &mons[PM_CTHULHU]) {
	    pline("There are some things incapable of being understood!");
	    make_confused(HConfusion + rnd(20), FALSE);
	}
}

void
ustatusline()
{
	char info[BUFSZ];

	info[0] = '\0';
	if (Sick) {
		Strcat(info, ", dying from");
		if (u.usick_type & SICK_VOMITABLE)
			Strcat(info, " food poisoning");
		if (u.usick_type & SICK_NONVOMITABLE) {
			if (u.usick_type & SICK_VOMITABLE)
				Strcat(info, " and");
			Strcat(info, " illness");
		}
	}
	if (Stoned)		Strcat(info, ", solidifying");
	if (Slimed)		Strcat(info, ", becoming slimy");
	if (Strangled)		Strcat(info, ", being strangled");
	if (Vomiting)		Strcat(info, ", nauseated"); /* !"nauseous" */
	if (Confusion)		Strcat(info, ", confused");
	if (Blind) {
	    Strcat(info, ", blind");
	    if (u.ucreamed) {
		if ((long)u.ucreamed < Blinded || Blindfolded
						|| !haseyes(youmonst.data))
		    Strcat(info, ", cover");
		Strcat(info, "ed by sticky goop");
	    }	/* note: "goop" == "glop"; variation is intentional */
	}
	if (Stunned)		Strcat(info, ", stunned");
	if (!u.usteed)
	if (Wounded_legs) {
	    const char *what = body_part(LEG);
	    if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
		what = makeplural(what);
				Sprintf(eos(info), ", injured %s", what);
	}
	if (Glib)		Sprintf(eos(info), ", slippery %s",
					makeplural(body_part(HAND)));
	if (u.utrap)		Strcat(info, ", trapped");
	if (u.ufeetfrozen)	Strcat(info, ", stuck in ice");
	if (Fast)		Strcat(info, Very_fast ?
						", very fast" : ", fast");
	if (u.uundetected)	Strcat(info, ", concealed");
	if (Invis)		Strcat(info, ", invisible");
	if (u.ustuck) {
	    if (sticks(youmonst.data))
		Strcat(info, ", holding ");
	    else
		Strcat(info, ", held by ");
	    Strcat(info, mon_nam(u.ustuck));
	}

	pline("Status of %s (%s%s):  Level %d  HP %d(%d)  AC %d%s.",
		plname,
		    (u.ualign.record >= 20) ? "piously " :
		    (u.ualign.record > 13) ? "devoutly " :
		    (u.ualign.record > 8) ? "fervently " :
		    (u.ualign.record > 3) ? "stridently " :
		    (u.ualign.record == 3) ? "" :
		    (u.ualign.record >= 1) ? "haltingly " :
		    (u.ualign.record == 0) ? "nominally " :
					    "insufficiently ",
		align_str(u.ualign.type),
		Upolyd ? mons[u.umonnum].mlevel : u.ulevel,
		Upolyd ? u.mh : u.uhp,
		Upolyd ? u.mhmax : u.uhpmax,
		u.uac,
		info);
}

void
self_invis_message()
{
	pline("%s %s.",
	    Hallucination ? "Far out, man!  You" : "Gee!  All of a sudden, you",
	    See_invisible ? "can see right through yourself" :
		"can't see yourself");
}

/*pline.c*/
