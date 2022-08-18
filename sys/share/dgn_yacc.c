#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
#ifdef YYPARSE_PARAM_TYPE
#define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
#else
#define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
#endif
#else
#define YYPARSE_DECL() yyparse(void)
#endif /* YYPARSE_PARAM */

extern int YYPARSE_DECL();

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 2 "dgn_comp.y"
/*	SCCS Id: @(#)dgn_comp.c	3.4	1996/06/22	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/*	Copyright (c) 1990 by M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Dungeon Compiler code
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive in its non-standard locale.
 */

#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "config.h"
#include "date.h"
#include "dgn_file.h"

void yyerror(const char *);
void yywarning(const char *);
int yylex();
int yyparse();
int getchain(char *);
int check_dungeon();
int check_branch();
int check_level();
void init_dungeon();
void init_branch();
void init_level();
void output_dgn();

#define Free(ptr)		free((genericptr_t)ptr)

#ifdef AMIGA
# undef	printf
#ifndef	LATTICE
# define    memset(addr,val,len)    setmem(addr,len,val)
#endif
#endif

#define ERR		(-1)

static struct couple couple;
static struct tmpdungeon tmpdungeon[MAXDUNGEON];
static struct tmplevel tmplevel[LEV_LIMIT];
static struct tmpbranch tmpbranch[BRANCH_LIMIT];

static int in_dungeon = 0, n_dgns = -1, n_levs = -1, n_brs = -1;

extern int fatal_error;
extern const char *fname;
extern FILE *yyin, *yyout;	/* from dgn_lex.c */

#line 66 "dgn_comp.y"
typedef union
{
	int	i;
	char*	str;
} YYSTYPE;
#line 102 "y.tab.c"
#define INTEGER 257
#define A_DUNGEON 258
#define BRANCH 259
#define CHBRANCH 260
#define LEVEL 261
#define RNDLEVEL 262
#define CHLEVEL 263
#define RNDCHLEVEL 264
#define UP_OR_DOWN 265
#define PROTOFILE 266
#define DESCRIPTION 267
#define DESCRIPTOR 268
#define LEVELDESC 269
#define ALIGNMENT 270
#define LEVALIGN 271
#define ENTRY 272
#define STAIR 273
#define NO_UP 274
#define NO_DOWN 275
#define PORTAL 276
#define STRING 277
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    5,    5,    6,    6,    6,    6,    7,    1,
    1,    8,    8,    8,   12,   13,   15,   15,   14,   10,
   10,   10,   10,   10,   16,   16,   17,   17,   18,   18,
   19,   19,   20,   20,    9,    9,   22,   23,    3,    3,
    3,    3,    3,    2,    2,    4,   21,   11,
};
static const short yylen[] = {                            2,
    0,    1,    1,    2,    1,    1,    1,    1,    6,    0,
    1,    1,    1,    1,    3,    1,    3,    3,    3,    1,
    1,    1,    1,    1,    6,    7,    7,    8,    3,    3,
    7,    8,    8,    9,    1,    1,    7,    8,    0,    1,
    1,    1,    1,    0,    1,    1,    5,    5,
};
static const short yydefred[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    3,    5,    6,    7,    8,
   12,   13,   14,   16,   20,   21,   22,   23,   24,   35,
   36,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    4,    0,    0,    0,    0,    0,
    0,    0,   19,   17,   29,   18,   30,   15,   46,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   11,    9,    0,   40,
   41,   42,   43,    0,    0,    0,    0,    0,    0,    0,
    0,   45,   37,    0,   27,    0,    0,    0,    0,    0,
   38,   28,   33,    0,   48,   47,   34,
};
static const short yydgoto[] = {                         14,
   78,   93,   84,   60,   15,   16,   17,   18,   19,   20,
   68,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   70,   30,   31,
};
static const short yysindex[] = {                      -237,
  -46,  -45,  -44,  -39,  -38,  -30,  -22,  -21,  -20,  -19,
  -18,  -17,  -16,    0, -237,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -262, -234, -233, -232, -230, -229, -228, -227, -217,
 -216, -215, -214, -202,    0, -221,   -7, -219, -221, -221,
 -221, -221,    0,    0,    0,    0,    0,    0,    0,   19,
   20,   21,   -2,   -1, -212, -211, -190, -189, -188, -271,
   19,   20,   20,   27,   28,   29,    0,    0,   30,    0,
    0,    0,    0, -193, -271, -182, -180,   19,   19, -179,
 -178,    0,    0, -193,    0, -177, -176, -175,   42,   43,
    0,    0,    0, -172,    0,    0,    0,
};
static const short yyrindex[] = {                        86,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   87,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   16,    0,    1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   31,    1,   46,    0,    0,    0,    0,
    0,    0,    0,   31,    0,   61,   76,    0,    0,    0,
    0,    0,    0,   91,    0,    0,    0,
};
static const short yygindex[] = {                         0,
    0,   -6,    4,  -43,    0,   75,    0,    0,    0,    0,
  -71,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -62,    0,    0,
};
#define YYTABLESIZE 363
static const short yytable[] = {                         85,
   39,   80,   81,   82,   83,   63,   64,   65,   66,   86,
   87,   32,   33,   34,   46,   10,   97,   98,   35,   36,
    1,    2,    3,    4,    5,    6,    7,   37,    8,    9,
   44,   10,   11,   12,   13,   38,   39,   40,   41,   42,
   43,   44,   47,   48,   49,   25,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   61,   62,   67,   69,
   26,   72,   73,   71,   74,   75,   76,   77,   79,   88,
   89,   92,   90,   91,   95,   31,   96,   99,  100,  102,
  103,  104,  105,  106,  107,    1,    2,  101,   94,   45,
   32,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,    0,   39,
   39,   39,   39,   10,   10,   10,   10,   10,   10,   10,
    0,   10,   10,    0,   10,   10,   10,   10,   44,   44,
   44,   44,   44,   44,   44,    0,   44,   44,    0,   44,
   44,   44,   44,   25,   25,   25,   25,   25,   25,   25,
    0,   25,   25,    0,   25,   25,   25,   25,   26,   26,
   26,   26,   26,   26,   26,    0,   26,   26,    0,   26,
   26,   26,   26,   31,   31,   31,   31,   31,   31,   31,
    0,   31,   31,    0,   31,   31,   31,   31,   32,   32,
   32,   32,   32,   32,   32,    0,   32,   32,    0,   32,
   32,   32,   32,
};
static const short yycheck[] = {                         71,
    0,  273,  274,  275,  276,   49,   50,   51,   52,   72,
   73,   58,   58,   58,  277,    0,   88,   89,   58,   58,
  258,  259,  260,  261,  262,  263,  264,   58,  266,  267,
    0,  269,  270,  271,  272,   58,   58,   58,   58,   58,
   58,   58,  277,  277,  277,    0,  277,  277,  277,  277,
  268,  268,  268,  268,  257,  277,   64,  277,   40,   40,
    0,   64,   64,   43,  277,  277,  257,  257,  257,   43,
   43,  265,   44,   44,  257,    0,  257,  257,  257,  257,
  257,  257,   41,   41,  257,    0,    0,   94,   85,   15,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,
};
#define YYFINAL 14
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 277
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,"'+'","','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,
"'@'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER",
"A_DUNGEON","BRANCH","CHBRANCH","LEVEL","RNDLEVEL","CHLEVEL","RNDCHLEVEL",
"UP_OR_DOWN","PROTOFILE","DESCRIPTION","DESCRIPTOR","LEVELDESC","ALIGNMENT",
"LEVALIGN","ENTRY","STAIR","NO_UP","NO_DOWN","PORTAL","STRING",
};
static const char *yyrule[] = {
"$accept : file",
"file :",
"file : dungeons",
"dungeons : dungeon",
"dungeons : dungeons dungeon",
"dungeon : dungeonline",
"dungeon : dungeondesc",
"dungeon : branches",
"dungeon : levels",
"dungeonline : A_DUNGEON ':' STRING bones_tag rcouple optional_int",
"optional_int :",
"optional_int : INTEGER",
"dungeondesc : entry",
"dungeondesc : descriptions",
"dungeondesc : prototype",
"entry : ENTRY ':' INTEGER",
"descriptions : desc",
"desc : DESCRIPTION ':' DESCRIPTOR",
"desc : ALIGNMENT ':' DESCRIPTOR",
"prototype : PROTOFILE ':' STRING",
"levels : level1",
"levels : level2",
"levels : levdesc",
"levels : chlevel1",
"levels : chlevel2",
"level1 : LEVEL ':' STRING bones_tag '@' acouple",
"level1 : RNDLEVEL ':' STRING bones_tag '@' acouple INTEGER",
"level2 : LEVEL ':' STRING bones_tag '@' acouple INTEGER",
"level2 : RNDLEVEL ':' STRING bones_tag '@' acouple INTEGER INTEGER",
"levdesc : LEVELDESC ':' DESCRIPTOR",
"levdesc : LEVALIGN ':' DESCRIPTOR",
"chlevel1 : CHLEVEL ':' STRING bones_tag STRING '+' rcouple",
"chlevel1 : RNDCHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER",
"chlevel2 : CHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER",
"chlevel2 : RNDCHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER INTEGER",
"branches : branch",
"branches : chbranch",
"branch : BRANCH ':' STRING '@' acouple branch_type direction",
"chbranch : CHBRANCH ':' STRING STRING '+' rcouple branch_type direction",
"branch_type :",
"branch_type : STAIR",
"branch_type : NO_UP",
"branch_type : NO_DOWN",
"branch_type : PORTAL",
"direction :",
"direction : UP_OR_DOWN",
"bones_tag : STRING",
"acouple : '(' INTEGER ',' INTEGER ')'",
"rcouple : '(' INTEGER ',' INTEGER ')'",

};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static unsigned yystacksize;
#line 433 "dgn_comp.y"

void
init_dungeon()
{
	if(++n_dgns > MAXDUNGEON) {
	    (void) fprintf(stderr, "FATAL - Too many dungeons (limit: %d).\n",
		    MAXDUNGEON);
	    (void) fprintf(stderr, "To increase the limit edit MAXDUNGEON in global.h\n");
	    exit(EXIT_FAILURE);
	}

	in_dungeon = 1;
	tmpdungeon[n_dgns].lev.base = 0;
	tmpdungeon[n_dgns].lev.rand = 0;
	tmpdungeon[n_dgns].chance = 100;
	Strcpy(tmpdungeon[n_dgns].name, "");
	Strcpy(tmpdungeon[n_dgns].protoname, "");
	tmpdungeon[n_dgns].flags = 0;
	tmpdungeon[n_dgns].levels = 0;
	tmpdungeon[n_dgns].branches = 0;
	tmpdungeon[n_dgns].entry_lev = 0;
}

void
init_level()
{
	if(++n_levs > LEV_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(EXIT_FAILURE);
	}
	tmplevel[n_levs].lev.base = 0;
	tmplevel[n_levs].lev.rand = 0;
	tmplevel[n_levs].chance = 100;
	tmplevel[n_levs].rndlevs = 0;
	tmplevel[n_levs].flags = 0;
	Strcpy(tmplevel[n_levs].name, "");
	tmplevel[n_levs].chain = -1;
}

void
init_branch()
{
	if(++n_brs > BRANCH_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(EXIT_FAILURE);
	}
	tmpbranch[n_brs].lev.base = 0;
	tmpbranch[n_brs].lev.rand = 0;
	Strcpy(tmpbranch[n_brs].name, "");
	tmpbranch[n_brs].chain = -1;
}

int
getchain(s)
	char	*s;
{
	int i;

	if(strlen(s)) {

	    for(i = n_levs - tmpdungeon[n_dgns].levels + 1; i <= n_levs; i++)
		if(!strcmp(tmplevel[i].name, s)) return i;

	    yyerror("Can't locate the specified chain level.");
	    return(-2);
	}
	return(-1);
}

/*
 *	Consistancy checking routines:
 *
 *	- A dungeon must have a unique name.
 *	- A dungeon must have a originating "branch" command
 *	  (except, of course, for the first dungeon).
 *	- A dungeon must have a proper depth (at least (1, 0)).
 */

int
check_dungeon()
{
	int i;

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpdungeon[n_dgns].name)) {
		yyerror("Duplicate dungeon name.");
		return(0);
	    }

	if(n_dgns)
	  for(i = 0; i < n_brs - tmpdungeon[n_dgns].branches; i++) {
	    if(!strcmp(tmpbranch[i].name, tmpdungeon[n_dgns].name)) break;

	    if(i >= n_brs - tmpdungeon[n_dgns].branches) {
		yyerror("Dungeon cannot be reached.");
		return(0);
	    }
	  }

	if(tmpdungeon[n_dgns].lev.base <= 0 ||
	   tmpdungeon[n_dgns].lev.rand < 0) {
		yyerror("Invalid dungeon depth specified.");
		return(0);
	}
	return(1);	/* OK */
}

/*
 *	- A level must have a unique level name.
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabilistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_level()
{
	int i;

	if(!in_dungeon) {
		yyerror("Level defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_levs; i++)
	    if(!strcmp(tmplevel[i].name, tmplevel[n_levs].name)) {
		yyerror("Duplicate level name.");
		return(0);
	    }

	if(tmplevel[i].chain == -2) {
		yyerror("Invaild level chain reference.");
		return(0);
	} else if(tmplevel[i].chain != -1) {	/* there is a chain */
	    /* KMH -- tmplevel[tmpbranch[i].chain].chance was in error */
	    if(tmplevel[tmplevel[i].chain].chance != 100) {
		yyerror("Level cannot chain from a probabilistic level.");
		return(0);
	    } else if(tmplevel[i].chain == n_levs) {
		yyerror("A level cannot chain to itself!");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	- A branch may not branch backwards - to avoid branch loops.
 *	- A branch name must be unique.
 *	  (ie. You can only have one entry point to each dungeon).
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabilistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_branch()
{
	int i;

	if(!in_dungeon) {
		yyerror("Branch defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpbranch[n_brs].name)) {

		yyerror("Reverse branching not allowed.");
		return(0);
	    }

	if(tmpbranch[i].chain == -2) {

		yyerror("Invaild branch chain reference.");
		return(0);
	} else if(tmpbranch[i].chain != -1) {	/* it is chained */

	    if(tmplevel[tmpbranch[i].chain].chance != 100) {
		yyerror("Branch cannot chain from a probabilistic level.");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	Output the dungon definition into a file.
 *
 *	The file will have the following format:
 *
 *	[ nethack version ID ]
 *	[ number of dungeons ]
 *	[ first dungeon struct ]
 *	[ levels for the first dungeon ]
 *	  ...
 *	[ branches for the first dungeon ]
 *	  ...
 *	[ second dungeon struct ]
 *	  ...
 */

void
output_dgn()
{
	int	nd, cl = 0, nl = 0,
		    cb = 0, nb = 0;
	static struct version_info version_data = {
			VERSION_NUMBER, VERSION_FEATURES,
			VERSION_SANITY1, VERSION_SANITY2
	};

	if(++n_dgns <= 0) {
	    yyerror("FATAL - no dungeons were defined.");
	    exit(EXIT_FAILURE);
	}

	if (fwrite((char *)&version_data, sizeof version_data, 1, yyout) != 1) {
	    yyerror("FATAL - output failure.");
	    exit(EXIT_FAILURE);
	}

	(void) fwrite((char *)&n_dgns, sizeof(int), 1, yyout);
	for (nd = 0; nd < n_dgns; nd++) {
	    (void) fwrite((char *)&tmpdungeon[nd], sizeof(struct tmpdungeon),
							1, yyout);

	    nl += tmpdungeon[nd].levels;
	    for(; cl < nl; cl++)
		(void) fwrite((char *)&tmplevel[cl], sizeof(struct tmplevel),
							1, yyout);

	    nb += tmpdungeon[nd].branches;
	    for(; cb < nb; cb++)
		(void) fwrite((char *)&tmpbranch[cb], sizeof(struct tmpbranch),
							1, yyout);
	}
	/* apparently necessary for Think C 5.x, otherwise harmless */
	(void) fflush(yyout);
}

/*dgn_comp.y*/
#line 619 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    yystate = 0;
    *yyssp = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yyssp = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yyssp = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 2:
#line 83 "dgn_comp.y"
	{
			output_dgn();
		  }
break;
case 9:
#line 99 "dgn_comp.y"
	{
			init_dungeon();
			Strcpy(tmpdungeon[n_dgns].name, yyvsp[-3].str);
			tmpdungeon[n_dgns].boneschar = (char)yyvsp[-2].i;
			tmpdungeon[n_dgns].lev.base = couple.base;
			tmpdungeon[n_dgns].lev.rand = couple.rand;
			tmpdungeon[n_dgns].chance = yyvsp[0].i;
			Free(yyvsp[-3].str);
		  }
break;
case 10:
#line 111 "dgn_comp.y"
	{
			yyval.i = 0;
		  }
break;
case 11:
#line 115 "dgn_comp.y"
	{
			yyval.i = yyvsp[0].i;
		  }
break;
case 15:
#line 126 "dgn_comp.y"
	{
			tmpdungeon[n_dgns].entry_lev = yyvsp[0].i;
		  }
break;
case 17:
#line 135 "dgn_comp.y"
	{
			if(yyvsp[0].i <= TOWN || yyvsp[0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yyvsp[0].i ;
		  }
break;
case 18:
#line 142 "dgn_comp.y"
	{
			if(yyvsp[0].i && yyvsp[0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yyvsp[0].i ;
		  }
break;
case 19:
#line 151 "dgn_comp.y"
	{
			Strcpy(tmpdungeon[n_dgns].protoname, yyvsp[0].str);
			Free(yyvsp[0].str);
		  }
break;
case 25:
#line 165 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-3].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-2].i;
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-3].str);
		  }
break;
case 26:
#line 175 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-3].i;
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-4].str);
		  }
break;
case 27:
#line 188 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-3].i;
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-4].str);
		  }
break;
case 28:
#line 199 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-4].i;
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[-1].i;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-5].str);
		  }
break;
case 29:
#line 213 "dgn_comp.y"
	{
			if(yyvsp[0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmplevel[n_levs].flags |= yyvsp[0].i ;
		  }
break;
case 30:
#line 220 "dgn_comp.y"
	{
			if(yyvsp[0].i && yyvsp[0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmplevel[n_levs].flags |= yyvsp[0].i ;
		  }
break;
case 31:
#line 229 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-3].i;
			tmplevel[n_levs].chain = getchain(yyvsp[-2].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-4].str);
			Free(yyvsp[-2].str);
		  }
break;
case 32:
#line 242 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-4].i;
			tmplevel[n_levs].chain = getchain(yyvsp[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-5].str);
			Free(yyvsp[-3].str);
		  }
break;
case 33:
#line 258 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-4].i;
			tmplevel[n_levs].chain = getchain(yyvsp[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-5].str);
			Free(yyvsp[-3].str);
		  }
break;
case 34:
#line 272 "dgn_comp.y"
	{
			init_level();
			Strcpy(tmplevel[n_levs].name, yyvsp[-6].str);
			tmplevel[n_levs].boneschar = (char)yyvsp[-5].i;
			tmplevel[n_levs].chain = getchain(yyvsp[-4].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[-1].i;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free(yyvsp[-6].str);
			Free(yyvsp[-4].str);
		  }
break;
case 37:
#line 293 "dgn_comp.y"
	{
			init_branch();
			Strcpy(tmpbranch[n_brs].name, yyvsp[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yyvsp[-1].i;
			tmpbranch[n_brs].up = yyvsp[0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
			Free(yyvsp[-4].str);
		  }
break;
case 38:
#line 307 "dgn_comp.y"
	{
			init_branch();
			Strcpy(tmpbranch[n_brs].name, yyvsp[-5].str);
			tmpbranch[n_brs].chain = getchain(yyvsp[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yyvsp[-1].i;
			tmpbranch[n_brs].up = yyvsp[0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
			Free(yyvsp[-5].str);
			Free(yyvsp[-4].str);
		  }
break;
case 39:
#line 323 "dgn_comp.y"
	{
			yyval.i = TBR_STAIR;	/* two way stair */
		  }
break;
case 40:
#line 327 "dgn_comp.y"
	{
			yyval.i = TBR_STAIR;	/* two way stair */
		  }
break;
case 41:
#line 331 "dgn_comp.y"
	{
			yyval.i = TBR_NO_UP;	/* no up staircase */
		  }
break;
case 42:
#line 335 "dgn_comp.y"
	{
			yyval.i = TBR_NO_DOWN;	/* no down staircase */
		  }
break;
case 43:
#line 339 "dgn_comp.y"
	{
			yyval.i = TBR_PORTAL;	/* portal connection */
		  }
break;
case 44:
#line 345 "dgn_comp.y"
	{
			yyval.i = 0;	/* defaults to down */
		  }
break;
case 45:
#line 349 "dgn_comp.y"
	{
			yyval.i = yyvsp[0].i;
		  }
break;
case 46:
#line 355 "dgn_comp.y"
	{
			char *p = yyvsp[0].str;
			if (strlen(p) != 1) {
			    if (strcmp(p, "none") != 0)
		   yyerror("Bones marker must be a single char, or \"none\"!");
			    *p = '\0';
			}
			yyval.i = *p;
			Free(p);
		  }
break;
case 47:
#line 385 "dgn_comp.y"
	{
			if (yyvsp[-3].i < -MAXLEVEL || yyvsp[-3].i > MAXLEVEL) {
			    yyerror("Abs base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else if (yyvsp[-1].i < -1 ||
				((yyvsp[-3].i < 0) ? (MAXLEVEL + yyvsp[-3].i + yyvsp[-1].i + 1) > MAXLEVEL :
					(yyvsp[-3].i + yyvsp[-1].i) > MAXLEVEL)) {
			    yyerror("Abs range out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yyvsp[-3].i;
			    couple.rand = yyvsp[-1].i;
			}
		  }
break;
case 48:
#line 422 "dgn_comp.y"
	{
			if (yyvsp[-3].i < -MAXLEVEL || yyvsp[-3].i > MAXLEVEL) {
			    yyerror("Rel base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yyvsp[-3].i;
			    couple.rand = yyvsp[-1].i;
			}
		  }
break;
#line 1113 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = (short) yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
