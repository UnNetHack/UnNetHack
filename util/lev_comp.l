%{
/*	SCCS Id: @(#)lev_lex.c	3.4	2002/03/27	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define LEV_LEX_C

#include "hack.h"
#include "lev_comp.h"
#include "sp_lev.h"

/* Most of these don't exist in flex, yywrap is macro and
 * yyunput is properly declared in flex.skel.
 */
#if !defined(FLEX_SCANNER) && !defined(FLEXHACK_SCANNER)
int FDECL(yyback, (int *,int));
int NDECL(yylook);
int NDECL(yyinput);
int NDECL(yywrap);
int NDECL(yylex);
	/* Traditional lexes let yyunput() and yyoutput() default to int;
	 * newer ones may declare them as void since they don't return
	 * values.  For even more fun, the lex supplied as part of the
	 * newer unbundled compiler for SunOS 4.x adds the void declarations
	 * (under __STDC__ or _cplusplus ifdefs -- otherwise they remain
	 * int) while the bundled lex and the one with the older unbundled
	 * compiler do not.  To detect this, we need help from outside --
	 * sys/unix/Makefile.utl.
	 *
	 * Digital UNIX is difficult and still has int in spite of all
	 * other signs.
	 */
# if defined(NeXT) || defined(SVR4) || defined(_AIX32)
#  define VOIDYYPUT
# endif
# if !defined(VOIDYYPUT) && defined(POSIX_TYPES)
#  if !defined(BOS) && !defined(HISX) && !defined(_M_UNIX) && !defined(VMS)
#   define VOIDYYPUT
#  endif
# endif
# if !defined(VOIDYYPUT) && defined(WEIRD_LEX)
#  if defined(SUNOS4) && defined(__STDC__) && (WEIRD_LEX > 1)
#   define VOIDYYPUT
#  endif
# endif
# if defined(VOIDYYPUT) && defined(__osf__)
#  undef VOIDYYPUT
# endif
# ifdef VOIDYYPUT
void FDECL(yyunput, (int));
void FDECL(yyoutput, (int));
# else
int FDECL(yyunput, (int));
int FDECL(yyoutput, (int));
# endif
#endif	/* !FLEX_SCANNER && !FLEXHACK_SCANNER */

#ifdef FLEX_SCANNER
#define YY_MALLOC_DECL \
	      genericptr_t FDECL(malloc, (size_t)); \
	      genericptr_t FDECL(realloc, (genericptr_t,size_t));
#endif

void FDECL(init_yyin, (FILE *));
void FDECL(init_yyout, (FILE *));

void FDECL(include_push, (const char *));
int NDECL(include_pop);

long NDECL(handle_varstring_check);

extern struct lc_vardefs *FDECL(vardef_defined,(struct lc_vardefs *,char *, int));

extern struct lc_vardefs *variable_definitions;

/*
 * This doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison).
 */
extern YYSTYPE yylval;

int line_number = 1, colon_line_number = 1;
static char map[4096];
static int map_cnt = 0;

int doing_included_file = 0;
YY_BUFFER_STATE include_stack[5];
int include_line[5];
char *include_file[5];

FILE *orig_yyin = NULL;

%}
%e 1500
%p 5000
%n 700
%s MAPC
%%
<MAPC>ENDMAP	{
		  BEGIN(INITIAL);
		  yylval.map = (char *) alloc(map_cnt + 1);
		  (void) strncpy(yylval.map, map, map_cnt);
		  yylval.map[map_cnt] = 0;
		  map_cnt = 0;
		  return MAP_ID;
		}
<MAPC>[-|}{+xABCISHKPLWTtF\\#. 0123456789]*\r?\n	{
		  int len = yyleng;
		  /* convert \r\n to \n */
		  if (len >= 2 && yytext[len - 2] == '\r') len -= 1;
		  line_number++;
		  (void) strncpy(map + map_cnt, yytext, len);
		  map_cnt += len;
		  map[map_cnt - 1] = '\n';
		  map[map_cnt] = '\0';
		}
^[ \t]*#.*\n		{ line_number++; }
:		{ colon_line_number = line_number; return ':'; }
MESSAGE		return MESSAGE_ID;
NOMAP		return NOMAP_ID;
LEVEL		return LEVEL_ID;
INIT_MAP	return LEV_INIT_ID;
mazegrid	return MAZE_GRID_ID;
solidfill	return SOLID_FILL_ID;
mines		return MINES_ID;
FLAGS		return FLAGS_ID;
GEOMETRY	return GEOMETRY_ID;
^MAP\r?\n		{ BEGIN(MAPC); line_number++; }
OBJECT		return OBJECT_ID;
CONTAINER	return COBJECT_ID;
MONSTER		return MONSTER_ID;
TRAP		return TRAP_ID;
DOOR		return DOOR_ID;
ROOMDOOR	return ROOMDOOR_ID;
DRAWBRIDGE	return DRAWBRIDGE_ID;
MAZEWALK	return MAZEWALK_ID;
WALLIFY		return WALLIFY_ID;
REGION		return REGION_ID;
ALTAR		return ALTAR_ID;
LADDER		return LADDER_ID;
STAIR		return STAIR_ID;
PORTAL		return PORTAL_ID;
TELEPORT_REGION	return TELEPRT_ID;
BRANCH		return BRANCH_ID;
FOUNTAIN	return FOUNTAIN_ID;
SINK		return SINK_ID;
POOL		return POOL_ID;
WALLWALK	return WALLWALK_ID;
NON_DIGGABLE	return NON_DIGGABLE_ID;
NON_PASSWALL	return NON_PASSWALL_ID;
IF		return IF_ID;
ELSE		return ELSE_ID;
EXIT		return EXIT_ID;
ROOM		return ROOM_ID;
SUBROOM		return SUBROOM_ID;
RANDOM_CORRIDORS	return RAND_CORRIDOR_ID;
CORRIDOR	return CORRIDOR_ID;
SPILL	  return SPILL_ID;
TERRAIN		return TERRAIN_ID;
REPLACE_TERRAIN	return REPLACE_TERRAIN_ID;
MON_GENERATION	return MON_GENERATION_ID;
GOLD		return GOLD_ID;
GRAVE		return GRAVE_ID;
ENGRAVING	return ENGRAVING_ID;
NAME		return NAME_ID;
CHANCE		return CHANCE_ID;
LOOP		return LOOP_ID;
SWITCH		return SWITCH_ID;
CASE		return CASE_ID;
BREAK		return BREAK_ID;
DEFAULT		return DEFAULT_ID;
FUNCTION	return FUNCTION_ID;
INCLUDE		return INCLUDE_ID;
SOUNDS		return SOUNDS_ID;
SHUFFLE		return SHUFFLE_ID;
montype		return MONTYPE_ID;
selection	return selection_ID;
rect		return rect_ID;
fillrect	return fillrect_ID;
line		return line_ID;
randline	return randline_ID;
grow		return grow_ID;
floodfill	return flood_ID;
rndcoord	return rndcoord_ID;
circle		return circle_ID;
ellipse		return ellipse_ID;
filter		return filter_ID;
hear		{ yylval.i=LVLSND_HEARD;  return MSG_OUTPUT_TYPE; }
pline		{ yylval.i=LVLSND_PLINED; return MSG_OUTPUT_TYPE; }
verbal		{ yylval.i=LVLSND_VERBAL; return MSG_OUTPUT_TYPE; }
feel		{ yylval.i=LVLSND_FELT;   return MSG_OUTPUT_TYPE; }
levregion	return LEV;
quantity	return QUANTITY_ID;
buried		return BURIED_ID;
eroded		return ERODED_ID;
erodeproof	return ERODEPROOF_ID;
trapped		return TRAPPED_ID;
recharged	return RECHARGED_ID;
invisible	return INVIS_ID;
greased		return GREASED_ID;
female		return FEMALE_ID;
cancelled	return CANCELLED_ID;
revived		return REVIVED_ID;
avenge		return AVENGE_ID;
fleeing		return FLEEING_ID;
blinded		return BLINDED_ID;
paralyzed	return PARALYZED_ID;
stunned		return STUNNED_ID;
confused	return CONFUSED_ID;
seen_traps	return SEENTRAPS_ID;
all		return ALL_ID;
horizontal	{ yylval.i=1; return HORIZ_OR_VERT; }
vertical	{ yylval.i=2; return HORIZ_OR_VERT; }
open		{ yylval.i=D_ISOPEN; return DOOR_STATE; }
closed		{ yylval.i=D_CLOSED; return DOOR_STATE; }
locked		{ yylval.i=D_LOCKED; return DOOR_STATE; }
nodoor		{ yylval.i=D_NODOOR; return DOOR_STATE; }
broken		{ yylval.i=D_BROKEN; return DOOR_STATE; }
secret		{ yylval.i=D_SECRET; return DOOR_STATE; }
north		{ yylval.i=W_NORTH; return DIRECTION; }
east		{ yylval.i=W_EAST; return DIRECTION; }
south		{ yylval.i=W_SOUTH; return DIRECTION; }
west		{ yylval.i=W_WEST; return DIRECTION; }
random		{ yylval.i = -1; return RANDOM_TYPE; }
none		{ yylval.i = -2; return NONE; }
align		return A_REGISTER;
left		{ yylval.i=1; return LEFT_OR_RIGHT; }
half-left	{ yylval.i=2; return LEFT_OR_RIGHT; }
center		{ yylval.i=3; return CENTER; }
half-right	{ yylval.i=4; return LEFT_OR_RIGHT; }
right		{ yylval.i=5; return LEFT_OR_RIGHT; }
top		{ yylval.i=1; return TOP_OR_BOT; }
bottom		{ yylval.i=5; return TOP_OR_BOT; }
lit		{ yylval.i=1; return LIGHT_STATE; }
unlit		{ yylval.i=0; return LIGHT_STATE; }
filled		{ yylval.i=0; return FILLING; }
unfilled	{ yylval.i=1; return FILLING; }
noalign		{ yylval.i= AM_NONE; return ALIGNMENT; }
law		{ yylval.i= AM_LAWFUL; return ALIGNMENT; }
neutral		{ yylval.i= AM_NEUTRAL; return ALIGNMENT; }
chaos		{ yylval.i= AM_CHAOTIC; return ALIGNMENT; }
coaligned	{ yylval.i= AM_SPLEV_CO; return ALIGNMENT; }
noncoaligned	{ yylval.i= AM_SPLEV_NONCO; return ALIGNMENT; }
peaceful	{ yylval.i=1; return MON_ATTITUDE; }
hostile		{ yylval.i=0; return MON_ATTITUDE; }
asleep		{ yylval.i=1; return MON_ALERTNESS; }
awake		{ yylval.i=0; return MON_ALERTNESS; }
m_feature	{ yylval.i= M_AP_FURNITURE; return MON_APPEARANCE; }
m_monster	{ yylval.i= M_AP_MONSTER;   return MON_APPEARANCE; }
m_object	{ yylval.i= M_AP_OBJECT;    return MON_APPEARANCE; }
sanctum		{ yylval.i=2; return ALTAR_TYPE; }
shrine		{ yylval.i=1; return ALTAR_TYPE; }
altar		{ yylval.i=0; return ALTAR_TYPE; }
up		{ yylval.i=1; return UP_OR_DOWN; }
down		{ yylval.i=0; return UP_OR_DOWN; }
false		{ yylval.i=0; return BOOLEAN; }
true		{ yylval.i=1; return BOOLEAN; }
dust		{ yylval.i=DUST; return ENGRAVING_TYPE; }
engrave		{ yylval.i=ENGRAVE; return ENGRAVING_TYPE; }
burn		{ yylval.i=BURN; return ENGRAVING_TYPE; }
mark		{ yylval.i=MARK; return ENGRAVING_TYPE; }
blood		{ yylval.i=ENGR_BLOOD; return ENGRAVING_TYPE; }
blessed		{ yylval.i=1; return CURSE_TYPE; }
uncursed	{ yylval.i=2; return CURSE_TYPE; }
cursed		{ yylval.i=3; return CURSE_TYPE; }
noteleport	{ yylval.i=NOTELEPORT; return FLAG_TYPE; }
hardfloor	{ yylval.i=HARDFLOOR; return FLAG_TYPE; }
nommap		{ yylval.i=NOMMAP; return FLAG_TYPE; }
arboreal	{ yylval.i=ARBOREAL; return FLAG_TYPE; }	/* KMH */
shortsighted	{ yylval.i=SHORTSIGHTED; return FLAG_TYPE; }
noflipx		{ yylval.i=NOFLIPX; return FLAG_TYPE; }
noflipy		{ yylval.i=NOFLIPY; return FLAG_TYPE; }
noflip		{ yylval.i=(NOFLIPX + NOFLIPY); return FLAG_TYPE; }
mazelevel	{ yylval.i=MAZELEVEL; return FLAG_TYPE; }
premapped	{ yylval.i=PREMAPPED; return FLAG_TYPE; }
shroud		{ yylval.i=SHROUD; return FLAG_TYPE; }
stormy		{ yylval.i=STORMY; return FLAG_TYPE; }
graveyard	{ yylval.i=GRAVEYARD; return FLAG_TYPE; }
[0-9]+d[0-9]+	{ char *p = strchr(yytext, 'd');
		  if (p) {
			*p = '\0';
			p++;
			yylval.dice.num=atoi(yytext);
			yylval.dice.die=atoi(p);
		  } else { yylval.dice.num = yylval.dice.die = 1; }
		  return DICE;
		}
\[\ *[0-9]+\%\ *\] { yylval.i = atoi(yytext + 1); return PERCENT; }
-[0-9]+		{ yylval.i=atoi(yytext); return MINUS_INTEGER; }
\+[0-9]+	{ yylval.i=atoi(yytext); return PLUS_INTEGER; }
[0-9]+\%	{ yylval.i = atoi(yytext); return SPERCENT; }
[0-9]+		{ yylval.i=atoi(yytext); return INTEGER; }
\"[^"]*\"	{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  Strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
\$[a-zA-Z_][a-zA-Z_0-9]*	{ return handle_varstring_check(); }
[a-zA-Z_][a-zA-Z_0-9]*	{ yylval.map = (char *) alloc(strlen(yytext)+1);
		  Strcpy(yylval.map, yytext);
		  return NQSTRING; }
"=="		{ yylval.i = SPO_JE; return COMPARE_TYPE; }
"!="		{ yylval.i = SPO_JNE; return COMPARE_TYPE; }
"<>"		{ yylval.i = SPO_JNE; return COMPARE_TYPE; }
"<="		{ yylval.i = SPO_JLE; return COMPARE_TYPE; }
">="		{ yylval.i = SPO_JGE; return COMPARE_TYPE; }
"<"		{ yylval.i = SPO_JL; return COMPARE_TYPE; }
">"		{ yylval.i = SPO_JG; return COMPARE_TYPE; }
\r?\n		{ line_number++; }
[ \t]+		;
'\\.'		{ yylval.i = yytext[2]; return CHAR; }
'.'		{ yylval.i = yytext[1]; return CHAR; }
.		{ return yytext[0]; }
%%
#ifdef	AMIGA
long *alloc(n)
	unsigned n;
{
	return ((long *)malloc (n));
}
#endif

/* routine to switch to another input file; needed for flex */
void init_yyin( input_f )
FILE *input_f;
{
#if defined(FLEX_SCANNER) || defined(FLEXHACK_SCANNER)
	if (yyin)
	    yyrestart(input_f);
	else
#endif
	    yyin = input_f;
	    if (!orig_yyin) orig_yyin = yyin;
}
/* analogous routine (for completeness) */
void init_yyout( output_f )
FILE *output_f;
{
	yyout = output_f;
}

void
include_push(fname)
const char *fname;
{
  if (doing_included_file >= 5)
    yyerror("Too deep includes");
  else {
    FILE *fin;
    fin = fopen(fname, "r");
    if (!fin) {
       yyerror("Cannot open file for including.");
    } else {
       include_stack[doing_included_file] = YY_CURRENT_BUFFER;
       include_line[doing_included_file] = line_number;
       include_file[doing_included_file] = strdup(fname);
       doing_included_file++;
       yyin = fin;
       yy_switch_to_buffer(yy_create_buffer(fin, YY_BUF_SIZE));
       BEGIN(INITIAL);
       yyparse();
    }
  }
}

int
include_pop()
{
  if (--doing_included_file < 0) {
    doing_included_file = 0;
    init_yyin(orig_yyin);
    yy_switch_to_buffer(yy_create_buffer(orig_yyin, YY_BUF_SIZE));
  } else {
    if (include_file[doing_included_file])
       free(include_file[doing_included_file]);
    yy_switch_to_buffer(include_stack[doing_included_file]);
    line_number = include_line[doing_included_file];
  }
  return 0;
}

/*
 * Stub needed for lex interface.
 */
int
yywrap()
{
    if (doing_included_file >= 0) include_pop();
    return 1;
}

long
handle_varstring_check()
{
  struct lc_vardefs *vd;
  yylval.map = (char *) alloc(strlen(yytext)+1);
  Strcpy(yylval.map, yytext);
  if ((vd = vardef_defined(variable_definitions, yytext, 1))) {
     long l = vd->var_type;
     long a = ((l & SPOVAR_ARRAY) == SPOVAR_ARRAY);
     l = (l & ~SPOVAR_ARRAY);
     if (l == SPOVAR_INT) return (a ? VARSTRING_INT_ARRAY : VARSTRING_INT);
     if (l == SPOVAR_STRING) return (a ? VARSTRING_STRING_ARRAY : VARSTRING_STRING);
     if (l == SPOVAR_VARIABLE) return (a ? VARSTRING_VAR_ARRAY : VARSTRING_VAR);
     if (l == SPOVAR_COORD) return (a ? VARSTRING_COORD_ARRAY : VARSTRING_COORD);
     if (l == SPOVAR_REGION) return (a ? VARSTRING_REGION_ARRAY : VARSTRING_REGION);
     if (l == SPOVAR_MAPCHAR) return (a ? VARSTRING_MAPCHAR_ARRAY : VARSTRING_MAPCHAR);
     if (l == SPOVAR_MONST) return (a ? VARSTRING_MONST_ARRAY : VARSTRING_MONST);
     if (l == SPOVAR_OBJ) return (a ? VARSTRING_OBJ_ARRAY : VARSTRING_OBJ);
     if (l == SPOVAR_SEL) return (a ? VARSTRING_SEL_ARRAY : VARSTRING_SEL);
  }
  return VARSTRING;
}

/*lev_comp.l*/
