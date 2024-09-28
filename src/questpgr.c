/*  SCCS Id: @(#)questpgr.c 3.4 2000/05/05  */
/*  Copyright 1991, M. Stephenson         */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

/*  quest-specific pager routines. */

#define QTEXT_AREA      FILE_AREA_SHARE
#include "qtext.h"

#define QTEXT_FILE "quest.lua"

/* #define DEBUG */ /* uncomment for debugging */

/* from sp_lev.c, for deliver_splev_message() */
extern char *lev_message;

static const char * intermed(void);
static const char * creatorname(void);
/* sometimes find_qarti(gi.invent), and gi.invent can be null */
static struct obj *find_qarti(struct obj *) NO_NNARGS;
static const char *neminame(void);
static const char *guardname(void);
static const char *homebase(void);
static void qtext_pronoun(char, char);
static void convert_arg(char);
static void convert_line(char *,char *);
static void deliver_by_pline(const char *);
static void deliver_by_window(const char *, int);
static boolean skip_pager(boolean);
static boolean com_pager_core(const char *, const char *, boolean, char **);

static char cvt_buf[64], qu_out_line[128];
/* used by ldrname() and neminame(), then copied into cvt_buf */
static char nambuf[sizeof cvt_buf];

short
quest_info(int typ)
{
    switch (typ) {
    case 0:
        return urole.questarti;
    case MS_LEADER:
        return urole.ldrnum;
    case MS_NEMESIS:
        return urole.neminum;
    case MS_GUARDIAN:
        return urole.guardnum;
    default:
        impossible("quest_info(%d)", typ);
    }
    return 0;
}

/** return your role leader's name */
const char *
ldrname(void)
{
    int i = urole.ldrnum;

    Sprintf(nambuf, "%s%s",
            type_is_pname(&mons[i]) ? "" : "the ",
            mons[i].mname);
    return nambuf;
}

/** return your intermediate target string */
static const char *
intermed(void)
{
    return (urole.intermed);
}

boolean
is_quest_artifact(struct obj *otmp)
{
    return (boolean)(otmp->oartifact == urole.questarti);
}

static struct obj *
find_qarti(struct obj *ochain)
{
    struct obj *otmp, *qarti;

    for (otmp = ochain; otmp; otmp = otmp->nobj) {
        if (is_quest_artifact(otmp)) {
            return otmp;
        }
        if (Has_contents(otmp) && (qarti = find_qarti(otmp->cobj)) != 0) {
            return qarti;
        }
    }
    return (struct obj *) 0;
}

/* check several object chains for the quest artifact to determine
   whether it is present on the current level */
struct obj *
find_quest_artifact(unsigned whichchains)
{
    struct monst *mtmp;
    struct obj *qarti = 0;

    if ((whichchains & (1 << OBJ_INVENT)) != 0) {
        qarti = find_qarti(invent);
    }
    if (!qarti && (whichchains & (1 << OBJ_FLOOR)) != 0) {
        qarti = find_qarti(fobj);
    }
    if (!qarti && (whichchains & (1 << OBJ_MINVENT)) != 0) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) {
                continue;
            }
            if ((qarti = find_qarti(mtmp->minvent)) != 0) {
                break;
            }
        }
    }
    if (!qarti && (whichchains & (1 << OBJ_MIGRATING)) != 0) {
        /* check migrating objects and minvent of migrating monsters */
        for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) {
                continue;
            }
            if ((qarti = find_qarti(mtmp->minvent)) != 0) {
                break;
            }
        }
        if (!qarti) {
            qarti = find_qarti(migrating_objs);
        }
    }
    if (!qarti && (whichchains & (1 << OBJ_BURIED)) != 0) {
        qarti = find_qarti(level.buriedobjlist);
    }

    return qarti;
}

/** return your role nemesis' name */
static const char *
neminame(void)
{
    int i = urole.neminum;

    Sprintf(nambuf, "%s%s",
            type_is_pname(&mons[i]) ? "" : "the ",
            mons[i].mname);
    return nambuf;
}

/** return your role leader's guard monster name */
static const char *
guardname(void)
{
    int i = urole.guardnum;

    return mons[i].mname;
}

/** return your role leader's location */
static const char *
homebase(void)
{
    return urole.homebase;
}

/* replace deity, leader, nemesis, or artifact name with pronoun;
   overwrites cvt_buf[] */
static void
qtext_pronoun(char who, char which)
            /* 'd' => deity, 'l' => leader, 'n' => nemesis, 'o' => artifact */
            /* 'h'|'H'|'i'|'I'|'j'|'J' */
{
    const char *pnoun;
    int g;
    char lwhich = lowc(which); /* H,I,J -> h,i,j */

    /*
     * Invalid subject (not d,l,n,o) yields neuter, singular result.
     *
     * For %o, treat all artifacts as neuter; some have plural names,
     * which genders[] doesn't handle; cvt_buf[] already contains name.
     */
    if (who == 'o' &&
         (strstri(cvt_buf, "Eyes ")  ||
          strcmpi(cvt_buf, makesingular(cvt_buf)))) {
        pnoun = (lwhich == 'h') ? "they" :
                (lwhich == 'i') ? "them" :
                (lwhich == 'j') ? "their" : "?";
    } else {
        g = (who == 'd') ? quest_status.godgend :
            (who == 'l') ? quest_status.ldrgend :
            (who == 'n') ? quest_status.nemgend : 2; /* default to neuter */
        pnoun = (lwhich == 'h') ? genders[g].he :
                (lwhich == 'i') ? genders[g].him :
                (lwhich == 'j') ? genders[g].his : "?";
    }
    Strcpy(cvt_buf, pnoun);
    /* capitalize for H,I,J */
    if (lwhich != which) {
        cvt_buf[0] = highc(cvt_buf[0]);
    }
    return;
}

static void
convert_arg(char c)
{
    const char *str;

    switch (c) {

    case 'p':   str = plname;
        break;
    case 'c':   str = (flags.female && urole.name.f) ?
                      urole.name.f : urole.name.m;
        break;
    case 'r':   str = rank_of(u.ulevel, Role_switch, flags.female);
        break;
    case 'R':   str = rank_of(MIN_QUEST_LEVEL, Role_switch,
                              flags.female);
        break;
    case 's':   str = (flags.female) ? "sister" : "brother";
        break;
    case 'S':   str = (flags.female) ? "daughter" : "son";
        break;
    case 'l':   str = ldrname();
        break;
    case 'i':   str = intermed();
        break;
    case 'O':
    case 'o':
                str = the(artiname(urole.questarti));
                if (c == 'O') {
                    /* shorten "the Foo of Bar" to "the Foo"
                       (buffer returned by the() is modifiable) */
                    char *p = strstri(str, " of ");

                    if (p) {
                        *p = '\0';
                    }
                }
                break;
    case 'm':   str = creatorname();
        break;
    case 'n':   str = neminame();
        break;
    case 'g':   str = guardname();
        break;
    case 'G':   str = align_gtitle(u.ualignbase[A_ORIGINAL]);
        break;
    case 'H':   str = homebase();
        break;
    case 'a':   str = align_str(u.ualignbase[A_ORIGINAL]);
        break;
    case 'A':   str = align_str(u.ualign.type);
        break;
    case 'd':   str = align_gname(u.ualignbase[A_ORIGINAL]);
        break;
    case 'D':   str = align_gname(A_LAWFUL);
        break;
    case 'C':   str = "chaotic";
        break;
    case 'N':   str = "neutral";
        break;
    case 'L':   str = "lawful";
        break;
    case 'x':   str = Blind ? "sense" : "see";
        break;
    case 'Z':   str = dungeons[0].dname;
        break;
    case '%':   str = "%";
        break;
    default:   str = "";
        break;
    }
    Strcpy(cvt_buf, str);
}

static void
convert_line(char *in_line, char *out_line)
{
    char *c, *cc;

    cc = out_line;
    for (c = in_line; *c; c++) {
        *cc = 0;
        switch (*c) {
        case '\r':
        case '\n':
            *(++cc) = 0;
            return;

        case '%':
            if (*(c+1)) {
                convert_arg(*(++c));
                switch (*(++c)) {
                /* insert "a"/"an" prefix */
                case 'A': Strcat(cc, An(cvt_buf));
                    cc += strlen(cc);
                    continue; /* for */
                case 'a': Strcat(cc, an(cvt_buf));
                    cc += strlen(cc);
                    continue; /* for */

                /* capitalize */
                case 'C': cvt_buf[0] = highc(cvt_buf[0]);
                    break;

                /* replace name with pronoun;
                   valid for %d, %l, %n, and %o */
                case 'h': /* he/she */
                case 'H': /* He/She */
                case 'i': /* him/her */
                case 'I':
                case 'j': /* his/her */
                case 'J':
                    if (index("dlno", lowc(*(c - 1)))) {
                        qtext_pronoun(*(c - 1), *c);
                    } else {
                        --c; /* default action */
                    }
                    break;

                /* pluralize */
                case 'P': cvt_buf[0] = highc(cvt_buf[0]);
                    /* fall through */
                case 'p': Strcpy(cvt_buf, makeplural(cvt_buf));
                    break;

                /* append possessive suffix */
                case 'S': cvt_buf[0] = highc(cvt_buf[0]);
                    /* fall through */
                case 's': Strcpy(cvt_buf, s_suffix(cvt_buf));
                    break;

                /* strip any "the" prefix */
                case 't': if (!strncmpi(cvt_buf, "the ", 4)) {
                        Strcat(cc, &cvt_buf[4]);
                        cc += strlen(cc);
                        continue; /* for */
                }
                    break;

                default: --c;   /* undo switch increment */
                    break;
                }
                Strcat(cc, cvt_buf);
                cc += strlen(cvt_buf);
                break;
            }   /* else fall through */

        default:
            *cc++ = *c;
            break;
        }
    }
    if (cc > &out_line[BUFSZ-1]) {
        panic("convert_line: overflow");
    }
    *cc = 0;
    return;
}

char *
string_subst(char *str)
{
    convert_line(str, qu_out_line);
    return qu_out_line;
}

static void
deliver_by_pline(const char *str)
{
    char in_line[BUFSZ], out_line[BUFSZ];
    const char *msgp = str, *msgend = eos((char *) str);

    while (msgp < msgend) {
        /* copynchars() will stop at newline if it finds one */
        copynchars(in_line, msgp, (int) sizeof in_line - 1);
        msgp += strlen(in_line) + 1;

        convert_line(in_line, out_line);
        pline("%s", out_line);
    }
}

static void
deliver_by_window(const char *msg, int how)
{
    char in_line[BUFSZ], out_line[BUFSZ];
    const char *msgp = msg, *msgend = eos((char *) msg);
    winid datawin = create_nhwindow(how);

    while (msgp < msgend) {
        /* copynchars() will stop at newline if it finds one */
        copynchars(in_line, msgp, (int) sizeof in_line - 1);
        msgp += strlen(in_line) + 1;

        convert_line(in_line, out_line);
        putstr(datawin, 0, out_line);
    }

    display_nhwindow(datawin, TRUE);
    destroy_nhwindow(datawin);
}

static boolean
skip_pager(boolean common UNUSED)
{
    /* WIZKIT: suppress plot feedback if starting with quest artifact */
    if (program_state.wizkit_wishing) {
        return TRUE;
    }
    return FALSE;
}

char *
qt_com_firstline(const char *msgid)
{
    char *msgbuf = NULL;
    com_pager_core("common", msgid, TRUE, &msgbuf);
    return msgbuf;
}

static boolean
com_pager_core(
    const char *section,
    const char *msgid,
    boolean showerror,
    char **rawtext)
{
    static const char *const howtoput[] = {
        "pline", "window", "text", "menu", "default", NULL
    };
    static const int howtoput2i[] = { 1, 2, 2, 3, 0, 0 };
    int output;
    lua_State *L;
    char *text = NULL, *synopsis = NULL, *fallback_msgid = NULL;
    boolean res = FALSE;
    nhl_sandbox_info sbi = {NHL_SB_SAFE, 1*1024*1024, 0, 1*1024*1024};

    if (skip_pager(TRUE)) {
        return FALSE;
    }

    L = nhl_init(&sbi);
    if (!L) {
        if (showerror) {
            impossible("com_pager: nhl_init() failed");
        }
        goto compagerdone;
    }

    if (!nhl_loadlua(L, NH_DATAAREA, QTEXT_FILE)) {
        if (showerror) {
            impossible("com_pager: %s not found.", QTEXT_FILE);
        }
        goto compagerdone;
    }

    lua_settop(L, 0);
    lua_getglobal(L, "questtext");
    if (!lua_istable(L, -1)) {
        if (showerror) {
            impossible("com_pager: questtext in %s is not a lua table", QTEXT_FILE);
        }
        goto compagerdone;
    }

    lua_getfield(L, -1, section);
    if (!lua_istable(L, -1)) {
        if (showerror) {
            impossible("com_pager: questtext[%s] in %s is not a lua table", section, QTEXT_FILE);
        }
        goto compagerdone;
    }

 tryagain:
    lua_getfield(L, -1, fallback_msgid ? fallback_msgid : msgid);
    if (!lua_istable(L, -1)) {
        if (!fallback_msgid) {
            /* Do we have questtxt[msg_fallbacks][<msgid>]? */
            lua_getfield(L, -3, "msg_fallbacks");
            if (lua_istable(L, -1)) {
                fallback_msgid = get_table_str_opt(L, msgid, NULL);
                lua_pop(L, 2);
                if (fallback_msgid) {
                    goto tryagain;
                }
            }
        }
        if (showerror) {
            if (!fallback_msgid) {
                impossible("com_pager: questtext[%s][%s] in %s is not a lua table",
                           section, msgid, QTEXT_FILE);
            } else {
                impossible("com_pager: questtext[%s][%s] and [][%s] in %s are not lua tables",
                           section, msgid, fallback_msgid, QTEXT_FILE);
            }
        }
        goto compagerdone;
    }

    text = get_table_str_opt(L, "text", NULL);
    if (rawtext) {
        /* only copy first line */
        for (int i = 0; i < strlen(text); i++) {
            if (text[i] == '\n') {
                text[i] = '\0';
                break;
            }
        }
        *rawtext = dupstr(text);
        res = TRUE;
        goto compagerdone;
    }
    synopsis = get_table_str_opt(L, "synopsis", NULL);
    output = howtoput2i[get_table_option(L, "output", "default", howtoput)];

    if (!text) {
        int nelems;

        lua_len(L, -1);
        nelems = (int) lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (nelems < 2) {
            if (showerror) {
                impossible("com_pager: questtext[%s][%s] in %s is not an array of strings",
                           section, fallback_msgid ? fallback_msgid : msgid,
                           QTEXT_FILE);
            }
            goto compagerdone;
        }
        nelems = rn2(nelems) + 1;
        lua_pushinteger(L, nelems);
        lua_gettable(L, -2);
        text = dupstr(luaL_checkstring(L, -1));
    }

    /* switch from by_pline to by_window if line has multiple segments or
       is unreasonably long (the latter ought to checked after formatting
       conversions rather than before...) */
    if (output == 0 && (strchr(text, '\n') || strlen(text) >= BUFSZ - 1)) {
        output = 2;

        /*
         * FIXME:  should update quest.lua to include proper synopsis line
         * for any item subject to having its delivery converted to by_window.
         */
        if (!synopsis) {
            char tmpbuf[BUFSZ];

            Sprintf(tmpbuf, "[%.*s]", BUFSZ - 1 - 2, text);
            /* change every newline character to a space */
            (void) strNsubst(tmpbuf, "\n", " ", 0);
            synopsis = dupstr(tmpbuf);
        }
    }

    if (output == 0 || output == 1) {
        deliver_by_pline(text);
    } else {
        deliver_by_window(text, (output == 3) ? NHW_MENU : NHW_TEXT);
    }

    if (synopsis) {
        char in_line[BUFSZ], out_line[BUFSZ];

#if 0   /* not yet -- brackets need to be removed from quest.lua */
        Sprintf(in_line, "[%.*s]",
                (int) (sizeof in_line - sizeof "[]"), synopsis);
#else
        Strcpy(in_line, synopsis);
#endif
        convert_line(in_line, out_line);
        /* bypass message delivery but be available for ^P recall */
#ifdef NEXT_VERSION
        putmsghistory(out_line, FALSE);
#endif
    }
    res = TRUE;

 compagerdone:
    if (text) {
        free((genericptr_t) text);
    }
    if (synopsis) {
        free((genericptr_t) synopsis);
    }
    if (fallback_msgid) {
        free((genericptr_t) fallback_msgid);
    }
    nhl_done(L);
    return res;
}

void
com_pager(const char *msgid)
{
    (void) com_pager_core("common", msgid, TRUE, (char **) 0);
}

void
qt_pager(const char *msgid)
{
    if (!com_pager_core(urole.filecode, msgid, FALSE, (char **) 0)) {
        (void) com_pager_core("common", msgid, TRUE, (char **) 0);
    }
}


/** special levels can include a custom arrival message; display it */
void
deliver_splev_message(void)
{
    /* there's no provision for delivering via window instead of pline */
    if (lev_message) {
        deliver_by_pline(lev_message);

        free((genericptr_t) lev_message);
        lev_message = NULL;
    }
}

/** The names of creator deities from different cultures. */
static const char *creator_names[] = {
    "Marduk", /* Babylonian */
    "Apsu", /* Babylonian */
    "Aeon", /* Greek */
    "Gaia", /* Greek */
    "Khronos", /* Greek */
    "Atum", /* Egyptian */
    "Khepri", /* Egyptian */
    "Kamui", /* Ainu */
    "Mbombo", /* Bakuba */
    "Unkulunkulu", /* Zulu */
    "Vishvakarman", /* Vedic */
    "Brahma", /* Hindu */
    "Coatlique", /* Aztec */
    "Viracocha", /* Inca */
    "Tepeu", /* Maya */
    "Pangu", /* Chinese */
    "Bulaing", /* Australian */
    "Ahura Mazda", /* Zoroastrian */
    "Demiourgos", /* Platon */
};

/** Return the name of the creator deity.
 * The name stays the same for the running game. */
static const char *
creatorname(void)
{
    if (pirateday()) {
        return "the FSM";
    } else if (discordian_holiday()) {
        return (game_seed % 2) ? "Discordia" : "Eris";
    } else {
        int index = game_seed % SIZE(creator_names);
        return creator_names[index];
    }
}

/*questpgr.c*/
