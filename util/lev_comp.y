%{
/*	SCCS Id: @(#)lev_yacc.c	3.4	2000/01/17	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"

#define ERR		(-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE	128

#define MAX_NESTED_IFS	20
#define MAX_SWITCH_CASES 20
#define MAX_SWITCH_BREAKS 20

#define New(type)		\
	(type *) memset((genericptr_t)alloc(sizeof(type)), 0, sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

extern void VDECL(lc_error, (const char *, ...));
extern void VDECL(lc_warning, (const char *, ...));
extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);
 extern void FDECL(include_push, (const char *));
extern int NDECL(include_pop);

extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *,CHAR_P));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *, sp_lev *));
extern void FDECL(add_opcode, (sp_lev *, int, genericptr_t));
extern genericptr_t FDECL(get_last_opcode_data1, (sp_lev *, int));
extern genericptr_t FDECL(get_last_opcode_data2, (sp_lev *, int,int));
extern boolean FDECL(check_subrooms, (sp_lev *));
extern boolean FDECL(write_level_file, (char *,sp_lev *));
extern struct opvar *FDECL(set_opvar_int, (struct opvar *, long));
extern void VDECL(add_opvars, (sp_lev *, const char *, ...));

extern struct lc_funcdefs *FDECL(funcdef_new,(long,char *));
extern void FDECL(funcdef_free_all,(struct lc_funcdefs *));
extern struct lc_funcdefs *FDECL(funcdef_defined,(struct lc_funcdefs *,char *, int));

extern struct lc_vardefs *FDECL(vardef_new,(long,char *));
extern void FDECL(vardef_free_all,(struct lc_vardefs *));
extern struct lc_vardefs *FDECL(vardef_defined,(struct lc_vardefs *,char *, int));

extern void FDECL(splev_add_from, (sp_lev *, sp_lev *));

extern void FDECL(check_vardef_type, (struct lc_vardefs *, char *, long));
extern struct lc_vardefs *FDECL(add_vardef_type, (struct lc_vardefs *, char *, long));

extern int FDECL(reverse_jmp_opcode, (int));


struct coord {
	long x;
	long y;
};

sp_lev *splev = NULL;

static struct opvar *if_list[MAX_NESTED_IFS];

static short n_if_list = 0;

unsigned int max_x_map, max_y_map;
int obj_containment = 0;

int in_container_obj = 0;

int in_switch_statement = 0;
static struct opvar *switch_check_jump = NULL;
static struct opvar *switch_default_case = NULL;
static struct opvar *switch_case_list[MAX_SWITCH_CASES];
static long switch_case_value[MAX_SWITCH_CASES];
int n_switch_case_list = 0;
static struct opvar *switch_break_list[MAX_SWITCH_BREAKS];
int n_switch_break_list = 0;


extern struct lc_vardefs *variable_definitions;


static struct lc_funcdefs *function_definitions = NULL;
int in_function_definition = 0;
sp_lev *function_splev_backup = NULL;

extern int fatal_error;
extern int got_errors;
extern int line_number;
extern const char *fname;

%}

%union
{
	long	i;
	char*	map;
	struct {
		long room;
		long wall;
		long door;
	} corpos;
    struct {
	long area;
	long x1;
	long y1;
	long x2;
	long y2;
    } lregn;
    struct {
	long x;
	long y;
    } crd;
    struct {
	long ter;
	long lit;
    } terr;
    struct {
	long height;
	long width;
    } sze;
    struct {
	long die;
	long num;
    } dice;
}


%token	<i> CHAR INTEGER BOOLEAN PERCENT SPERCENT
%token	<i> MINUS_INTEGER PLUS_INTEGER
%token	<i> MAZE_GRID_ID SOLID_FILL_ID MINES_ID
%token	<i> MESSAGE_ID LEVEL_ID LEV_INIT_ID GEOMETRY_ID NOMAP_ID
%token	<i> OBJECT_ID COBJECT_ID MONSTER_ID TRAP_ID DOOR_ID DRAWBRIDGE_ID
%token	<i> MAZEWALK_ID WALLIFY_ID REGION_ID FILLING
%token	<i> ALTAR_ID LADDER_ID STAIR_ID NON_DIGGABLE_ID NON_PASSWALL_ID ROOM_ID
%token	<i> PORTAL_ID TELEPRT_ID BRANCH_ID LEV CHANCE_ID RANDLINE_ID
%token	<i> CORRIDOR_ID GOLD_ID ENGRAVING_ID FOUNTAIN_ID POOL_ID SINK_ID NONE
%token	<i> RAND_CORRIDOR_ID DOOR_STATE LIGHT_STATE CURSE_TYPE ENGRAVING_TYPE
%token	<i> DIRECTION RANDOM_TYPE A_REGISTER
%token	<i> ALIGNMENT LEFT_OR_RIGHT CENTER TOP_OR_BOT ALTAR_TYPE UP_OR_DOWN
%token	<i> SUBROOM_ID NAME_ID FLAGS_ID FLAG_TYPE MON_ATTITUDE MON_ALERTNESS
%token	<i> MON_APPEARANCE ROOMDOOR_ID IF_ID ELSE_ID
%token	<i> SPILL_ID TERRAIN_ID HORIZ_OR_VERT REPLACE_TERRAIN_ID
%token	<i> EXIT_ID SHUFFLE_ID
%token	<i> QUANTITY_ID BURIED_ID LOOP_ID
%token	<i> SWITCH_ID CASE_ID BREAK_ID DEFAULT_ID
%token	<i> ERODED_ID TRAPPED_ID RECHARGED_ID INVIS_ID GREASED_ID
%token	<i> FEMALE_ID CANCELLED_ID REVIVED_ID AVENGE_ID FLEEING_ID BLINDED_ID
%token	<i> PARALYZED_ID STUNNED_ID CONFUSED_ID SEENTRAPS_ID ALL_ID
%token	<i> MON_GENERATION_ID MONTYPE_ID
%token	<i> GRAVE_ID ERODEPROOF_ID
%token	<i> FUNCTION_ID
%token	<i> INCLUDE_ID
%token	<i> SOUNDS_ID MSG_OUTPUT_TYPE
%token	<i> WALLWALK_ID COMPARE_TYPE
%token	<i> rect_ID
%token	<i> ',' ':' '(' ')' '[' ']' '{' '}'
%token	<map> STRING MAP_ID
%token	<map> NQSTRING VARSTRING
%token	<map> VARSTRING_INT VARSTRING_INT_ARRAY
%token	<map> VARSTRING_STRING VARSTRING_STRING_ARRAY
%token	<map> VARSTRING_VAR VARSTRING_VAR_ARRAY
%token	<map> VARSTRING_COORD VARSTRING_COORD_ARRAY
%token	<map> VARSTRING_REGION VARSTRING_REGION_ARRAY
%token	<map> VARSTRING_MAPCHAR VARSTRING_MAPCHAR_ARRAY
%token	<map> VARSTRING_MONST VARSTRING_MONST_ARRAY
%token	<map> VARSTRING_OBJ VARSTRING_OBJ_ARRAY
%token	<dice> DICE;
%type	<i> h_justif v_justif trap_name room_type door_state light_state
%type	<i> alignment altar_type a_register roomfill door_pos
%type	<i> alignment_prfx
%type	<i> door_wall walled secret chance
%type	<i> dir_list map_geometry teleprt_detail
%type	<i> object_infos object_info monster_infos monster_info
%type	<i> levstatements region_detail_end
%type	<i> engraving_type flag_list prefilled
%type	<i> monster
%type	<i> comparestmt encodecoord encoderegion mapchar
%type	<i> seen_trap_mask
%type	<i> mon_gen_list encodemonster encodeobj encodeobj_list
%type	<i> sounds_list integer_list string_list encodecoord_list encoderegion_list mapchar_list encodemonster_list
%type	<i> opt_percent opt_spercent opt_int opt_fillchar
%type	<i> all_integers
%type	<map> string level_def
%type	<map> any_var any_var_array any_var_or_arr
%type	<corpos> corr_spec
%type	<lregn> region lev_region
%type	<crd> room_pos subroom_pos room_align
%type	<sze> room_size
%type	<terr> terrain_type
%left  '+' '-'
%left  '*' '/' '%'
%start	file

%%
file		: header_stmts
		| header_stmts levels
		;

header_stmts	: /* nothing */
		| header_stmt header_stmts
		;

header_stmt	: function_define
		| include_def
		;


include_def	: INCLUDE_ID STRING
		  {
		      include_push( $2 );
		      Free($2);
		  }
		;

levels		: level
		| level levels
		;

level		: level_def flags levstatements
		  {
			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s: %d errors detected for level \"%s\". No output created!\n",
					       fname, fatal_error, $1);
				fatal_error = 0;
				got_errors++;
			} else if (!got_errors) {
				if (!write_level_file($1, splev)) {
				    lc_error("Can't write output file for '%s'!", $1);
				    exit(EXIT_FAILURE);
				}
			}
			Free($1);
			Free(splev);
			splev = NULL;
			vardef_free_all(variable_definitions);
			variable_definitions = NULL;
		  }
		;

level_def	: LEVEL_ID ':' string
		  {
		      struct lc_funcdefs *f;
			if (index($3, '.'))
			    lc_error("Invalid dot ('.') in level name '%s'.", $3);
			if ((int) strlen($3) > 8)
			    lc_error("Level names limited to 8 characters ('%s').", $3);
			f = function_definitions;
			while (f) {
			    f->n_called = 0;
			    f = f->next;
			}
			splev = (sp_lev *)alloc(sizeof(sp_lev));
			splev->n_opcodes = 0;
			splev->opcodes = NULL;

			vardef_free_all(variable_definitions);
			variable_definitions = NULL;

			$$ = $3;
		  }
		;

lev_init	: LEV_INIT_ID ':' SOLID_FILL_ID ',' terrain_type
		  {
		      long filling = $5.ter;
		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
		      add_opvars(splev, "iiiiiiiio", LVLINIT_SOLIDFILL,filling,0,(long)$5.lit, 0,0,0,0, SPO_INITLEVEL);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| LEV_INIT_ID ':' MAZE_GRID_ID ',' CHAR
		  {
		      long filling = what_map_char((char) $5);
		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
		      add_opvars(splev, "iiiiiiiio", LVLINIT_MAZEGRID,filling,0,0, 0,0,0,0, SPO_INITLEVEL);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| LEV_INIT_ID ':' MINES_ID ',' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled opt_fillchar
		  {
		      long fg = what_map_char((char) $5);
		      long bg = what_map_char((char) $7);
		      long smoothed = $9;
		      long joined = $11;
		      long lit = $13;
		      long walled = $15;
		      long filling = $16;
		      if (fg == INVALID_TYPE || fg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid foreground type.");
		      if (bg == INVALID_TYPE || bg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid background type.");
		      if (joined && fg != CORR && fg != ROOM)
			  lc_error("INIT_MAP: Invalid foreground type for joined map.");

		      if (filling == INVALID_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");

		      add_opvars(splev, "iiiiiiiio", LVLINIT_MINES,filling,walled,lit, joined,smoothed,bg,fg, SPO_INITLEVEL);
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
		;

opt_fillchar	: /* nothing */
		  {
		      $$ = -1;
		  }
		| ',' CHAR
		  {
		      $$ = what_map_char((char) $2);
		  }
		;


walled		: BOOLEAN
		| RANDOM_TYPE
		;

flags		: /* nothing */
		  {
		      add_opvars(splev, "io", 0, SPO_LEVEL_FLAGS);
		  }
		| FLAGS_ID ':' flag_list
		  {
		      add_opvars(splev, "io", $3, SPO_LEVEL_FLAGS);
		  }
		;

flag_list	: FLAG_TYPE ',' flag_list
		  {
		      $$ = ($1 | $3);
		  }
		| FLAG_TYPE
		  {
		      $$ = $1;
		  }
		;

levstatements	: /* nothing */
		  {
		      $$ = 0;
		  }
		| levstatement levstatements
		  {
		      $$ = 1 + $2;
		  }
		;

levstatement 	: message
		| lev_init
		| altar_detail
		| grave_detail
		| mon_generation
		| sounds_detail
		| branch_region
		| corridor
		| variable_define
		| shuffle_detail
		| diggable_detail
		| door_detail
		| wallwalk_detail
		| drawbridge_detail
		| engraving_detail
		| fountain_detail
		| gold_detail
		| switchstatement
		| loopstatement
		| ifstatement
		| exitstatement
		| function_define
		| function_call
		| ladder_detail
		| map_definition
		| mazewalk_detail
		| monster_detail
		| object_detail
		| passwall_detail
		| pool_detail
		| portal_region
		| random_corridors
		| region_detail
		| room_def
		| subroom_def
		| sink_detail
		| terrain_detail
		| replace_terrain_detail
		| spill_detail
		| randline_detail
		| stair_detail
		| stair_region
		| teleprt_region
		| trap_detail
		| wallify_detail
		;

any_var_array	: VARSTRING_INT_ARRAY
		| VARSTRING_STRING_ARRAY
		| VARSTRING_VAR_ARRAY
		| VARSTRING_COORD_ARRAY
		| VARSTRING_REGION_ARRAY
		| VARSTRING_MAPCHAR_ARRAY
		| VARSTRING_MONST_ARRAY
		| VARSTRING_OBJ_ARRAY
		;

any_var		: VARSTRING_INT
		| VARSTRING_STRING
		| VARSTRING_VAR
		| VARSTRING_COORD
		| VARSTRING_REGION
		| VARSTRING_MAPCHAR
		| VARSTRING_MONST
		| VARSTRING_OBJ
		;

any_var_or_arr	: any_var_array
		| any_var
		| VARSTRING
		;

shuffle_detail	: SHUFFLE_ID ':' any_var_array
		  {
		      struct lc_vardefs *vd;
		      if ((vd = vardef_defined(variable_definitions, $3, 1))) {
			  if (!(vd->var_type & SPOVAR_ARRAY))
			      lc_error("Trying to shuffle non-array variable '%s'", $3);
		      } else lc_error("Trying to shuffle undefined variable '%s'", $3);
		      add_opvars(splev, "so", $3, SPO_SHUFFLE_ARRAY);
		      Free($3);
		  }
		;

variable_define	: any_var_or_arr '=' math_expr
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_INT);
		      add_opvars(splev, "iso", 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' STRING
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_STRING);
		      add_opvars(splev, "siso", $3, 0, $1, SPO_VAR_INIT);
		      Free($1);
		      Free($3);
		  }
		| any_var_or_arr '=' any_var_or_arr
		  {
		      struct lc_vardefs *vd1, *vd2;
		      if (!strcmp($1, $3)) lc_error("Trying to set variable '%s' to value of itself", $1);
		      vd2 = vardef_defined(variable_definitions, $3, 1);
		      if (!vd2) {
			  lc_error("Trying to use an undefined variable '%s'", $3);
		      } else {
			  if ((vd1 = vardef_defined(variable_definitions, $1, 1))) {
			      if (vd1->var_type != vd2->var_type)
				  lc_error("Trying to redefine variable '%s' as different type", $1);
			  } else {
			      vd1 = vardef_new(vd2->var_type, $1);
			      vd1->next = variable_definitions;
			      variable_definitions = vd1;
			  }
		      }
		      add_opvars(splev, "siso", $3, -1, $1, SPO_VAR_INIT);
		      Free($1);
		      Free($3);
		  }
		| any_var_or_arr '=' TERRAIN_ID ':' mapchar
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_MAPCHAR);
		      add_opvars(splev, "miso", (long)$5, 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' MONSTER_ID ':' encodemonster
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_MONST);
		      add_opvars(splev, "Miso", (long)$5, 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' OBJECT_ID ':' encodeobj
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_OBJ);
		      add_opvars(splev, "Oiso", (long)$5, 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' encodecoord
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_COORD);
		      add_opvars(splev, "ciso", (long)$3, 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' encoderegion
		  {
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_REGION);
		      add_opvars(splev, "riso", (long)$3, 0, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' '{' integer_list '}'
		  {
		      long n_items = $4;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_INT|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' '{' encodecoord_list '}'
		  {
		      long n_items = $4;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_COORD|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' '{' encoderegion_list '}'
		  {
		      long n_items = $4;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_REGION|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' TERRAIN_ID ':' '{' mapchar_list '}'
		  {
		      long n_items = $6;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_MAPCHAR|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' MONSTER_ID ':' '{' encodemonster_list '}'
		  {
		      long n_items = $6;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_MONST|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' OBJECT_ID ':' '{' encodeobj_list '}'
		  {
		      long n_items = $6;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_OBJ|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		| any_var_or_arr '=' '{' string_list '}'
		  {
		      long n_items = $4;
		      variable_definitions = add_vardef_type(variable_definitions, $1, SPOVAR_STRING|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, $1, SPO_VAR_INIT);
		      Free($1);
		  }
		;

encodeobj_list	: encodeobj
		  {
		      add_opvars(splev, "O", $1);
		      $$ = 1;
		  }
		| encodeobj_list ',' encodeobj
		  {
		      add_opvars(splev, "O", $3);
		      $$ = 1 + $1;
		  }
		;

encodemonster_list	: encodemonster
		  {
		      add_opvars(splev, "M", $1);
		      $$ = 1;
		  }
		| encodemonster_list ',' encodemonster
		  {
		      add_opvars(splev, "M", $3);
		      $$ = 1 + $1;
		  }
		;

mapchar_list	: mapchar
		  {
		      add_opvars(splev, "m", $1);
		      $$ = 1;
		  }
		| mapchar_list ',' mapchar
		  {
		      add_opvars(splev, "m", $3);
		      $$ = 1 + $1;
		  }
		;

encoderegion_list	: encoderegion
		  {
		      add_opvars(splev, "r", $1);
		      $$ = 1;
		  }
		| encoderegion_list ',' encoderegion
		  {
		      add_opvars(splev, "r", $3);
		      $$ = 1 + $1;
		  }
		;

encodecoord_list	: encodecoord
		  {
		      add_opvars(splev, "c", $1);
		      $$ = 1;
		  }
		| encodecoord_list ',' encodecoord
		  {
		      add_opvars(splev, "c", $3);
		      $$ = 1 + $1;
		  }
		;

integer_list	: math_expr
		  {
		      $$ = 1;
		  }
		| integer_list ',' math_expr
		  {
		      $$ = 1 + $1;
		  }
		;

string_list	: STRING
		  {
		      add_opvars(splev, "s", $1);
		      Free($1);
		      $$ = 1;
		  }
		| string_list ',' STRING
		  {
		      add_opvars(splev, "s", $3);
		      Free($1);
		      $$ = 1 + $1;
		  }
		;

function_define	: FUNCTION_ID NQSTRING '(' ')'
		  {
		      struct lc_funcdefs *funcdef;

		      if (in_function_definition)
			  lc_error("Recursively defined functions not allowed (function %s).", $2);

		      in_function_definition++;

		      if (funcdef_defined(function_definitions, $2, 1))
			  lc_error("Function '%s' already defined once.", $2);

		      funcdef = funcdef_new(-1, $2);
		      funcdef->next = function_definitions;
		      function_definitions = funcdef;
		      function_splev_backup = splev;
		      splev = &(funcdef->code);
		      Free($2);
		  }
		'{' levstatements '}'
		  {
		      add_opvars(splev, "io", 0, SPO_RETURN);
		      splev = function_splev_backup;
		      in_function_definition--;
		  }
		;

function_call	: NQSTRING '(' ')'
		  {
		      struct lc_funcdefs *tmpfunc;
		      tmpfunc = funcdef_defined(function_definitions, $1, 1);
		      if (tmpfunc) {
			  long l;
			  if (!(tmpfunc->n_called)) {
			      /* we haven't called the function yet, so insert it in the code */
			      struct opvar *jmp = New(struct opvar);
			      set_opvar_int(jmp, splev->n_opcodes+1);
			      add_opcode(splev, SPO_PUSH, jmp);
			      add_opcode(splev, SPO_JMP, NULL); /* we must jump past it first, then CALL it, due to RETURN. */

			      tmpfunc->addr = splev->n_opcodes;
			      splev_add_from(splev, &(tmpfunc->code));
			      set_opvar_int(jmp, splev->n_opcodes - jmp->vardata.l);
			  }
			  l = tmpfunc->addr - splev->n_opcodes - 2;
			  add_opvars(splev, "iio", 0, l, SPO_CALL);
			  tmpfunc->n_called++;
		      } else {
			  lc_error("Function '%s' not defined.", $1);
		      }
		      Free($1);
		  }
		;

exitstatement	: EXIT_ID
		  {
		      add_opcode(splev, SPO_EXIT, NULL);
		  }
		;

opt_percent	: /* nothing */
		  {
		      $$ = 100;
		  }
		| PERCENT
		  {
		      if ($1 < 0 || $1 > 100) lc_error("Unexpected percentile chance '%li%%'", $1);
		      $$ = $1;
		  }
		;

opt_spercent	: /* nothing */
		  {
		      $$ = 100;
		  }
		| ',' SPERCENT
		  {
		      if ($2 < 0 || $2 > 100) lc_error("Unexpected percentile chance '%li%%'", $2);
		      $$ = $2;
		  }
		;

comparestmt     : PERCENT
                  {
		      /* val > rn2(100) */
		      add_opvars(splev, "iio", (long)$1, 100, SPO_RN2);
		      $$ = SPO_JG;
                  }
		| '[' math_expr_var COMPARE_TYPE math_expr_var ']'
                  {
		      $$ = $3;
                  }
		;

switchstatement	: SWITCH_ID '[' integer_or_var ']'
		  {
		      struct opvar *chkjmp;
		      if (in_switch_statement > 0)
			  lc_error("Cannot nest switch-statements.");

		      in_switch_statement++;

		      n_switch_case_list = 0;
		      n_switch_break_list = 0;
		      switch_default_case = NULL;

		      add_opvars(splev, "o", SPO_RN2);

		      chkjmp = New(struct opvar);
		      set_opvar_int(chkjmp, splev->n_opcodes+1);
		      switch_check_jump = chkjmp;
		      add_opcode(splev, SPO_PUSH, chkjmp);
		      add_opcode(splev, SPO_JMP, NULL);
		  }
		'{' switchcases '}'
		  {
		      struct opvar *endjump = New(struct opvar);
		      int i;

		      set_opvar_int(endjump, splev->n_opcodes+1);

		      add_opcode(splev, SPO_PUSH, endjump);
		      add_opcode(splev, SPO_JMP, NULL);

		      set_opvar_int(switch_check_jump, splev->n_opcodes - switch_check_jump->vardata.l);

		      for (i = 0; i < n_switch_case_list; i++) {
			  add_opvars(splev, "oio", SPO_COPY, switch_case_value[i], SPO_CMP);
			  set_opvar_int(switch_case_list[i], switch_case_list[i]->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_case_list[i]);
			  add_opcode(splev, SPO_JE, NULL);
		      }

		      if (switch_default_case) {
			  set_opvar_int(switch_default_case, switch_default_case->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_default_case);
			  add_opcode(splev, SPO_JMP, NULL);
		      }

		      set_opvar_int(endjump, splev->n_opcodes - endjump->vardata.l);

		      for (i = 0; i < n_switch_break_list; i++) {
			  set_opvar_int(switch_break_list[i], splev->n_opcodes - switch_break_list[i]->vardata.l);
		      }

		      add_opcode(splev, SPO_POP, NULL); /* get rid of the value in stack */
		      in_switch_statement--;


		  }
		;

switchcases	: /* nothing */
		| switchcase switchcases
		;

switchcase	: CASE_ID all_integers ':'
		  {
		      if (n_switch_case_list < MAX_SWITCH_CASES) {
			  struct opvar *tmppush = New(struct opvar);
			  set_opvar_int(tmppush, splev->n_opcodes);
			  switch_case_value[n_switch_case_list] = $2;
			  switch_case_list[n_switch_case_list++] = tmppush;
		      } else lc_error("Too many cases in a switch.");
		  }
		breakstatements
		  {
		  }
		| DEFAULT_ID ':'
		  {
		      struct opvar *tmppush = New(struct opvar);

		      if (switch_default_case)
			  lc_error("Switch default case already used.");

		      set_opvar_int(tmppush, splev->n_opcodes);
		      switch_default_case = tmppush;
		  }
		breakstatements
		  {
		  }
		;

breakstatements	: /* nothing */
		| breakstatement breakstatements
		;

breakstatement	: BREAK_ID
		  {
		      struct opvar *tmppush = New(struct opvar);
		      set_opvar_int(tmppush, splev->n_opcodes);
		      if (n_switch_break_list >= MAX_SWITCH_BREAKS)
			  lc_error("Too many BREAKs inside single SWITCH");
		      switch_break_list[n_switch_break_list++] = tmppush;

		      add_opcode(splev, SPO_PUSH, tmppush);
		      add_opcode(splev, SPO_JMP, NULL);
		  }
		| levstatement
		  {
		  }
		;

loopstatement	: LOOP_ID '[' integer_or_var ']'
		  {
		      struct opvar *tmppush = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("LOOP: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      set_opvar_int(tmppush, splev->n_opcodes);
		      if_list[n_if_list++] = tmppush;

		      add_opvars(splev, "o", SPO_DEC);
		  }
		 '{' levstatements '}'
		  {
		      struct opvar *tmppush;

		      add_opvars(splev, "oio", SPO_COPY, 0, SPO_CMP);

		      tmppush = (struct opvar *) if_list[--n_if_list];
		      set_opvar_int(tmppush, tmppush->vardata.l - splev->n_opcodes-1);
		      add_opcode(splev, SPO_PUSH, tmppush);
		      add_opcode(splev, SPO_JG, NULL);
		      add_opcode(splev, SPO_POP, NULL); /* get rid of the count value in stack */
		  }
		;

ifstatement 	: IF_ID comparestmt
		  {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("IF: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, splev->n_opcodes+1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(splev, SPO_PUSH, tmppush2);

		      add_opcode(splev, reverse_jmp_opcode( $2 ), NULL);
		  }
		 if_ending
		  {
		     /* do nothing */
		  }
		;

if_ending	: '{' levstatements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?!  No start address?");
		  }
		| '{' levstatements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush = New(struct opvar);
			  struct opvar *tmppush2;

			  set_opvar_int(tmppush, splev->n_opcodes+1);
			  add_opcode(splev, SPO_PUSH, tmppush);

			  add_opcode(splev, SPO_JMP, NULL);

			  tmppush2 = (struct opvar *) if_list[--n_if_list];

			  set_opvar_int(tmppush2, splev->n_opcodes - tmppush2->vardata.l);
			  if_list[n_if_list++] = tmppush;
		      } else lc_error("IF: Huh?!  No else-part address?");
		  }
		 ELSE_ID '{' levstatements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?! No end address?");
		  }
		;

message		: MESSAGE_ID ':' string_expr
		  {
		      add_opvars(splev, "o", SPO_MESSAGE);
		  }
		;

wallwalk_detail	: WALLWALK_ID ':' coord_or_var ',' mapchar_or_var opt_spercent
		  {
		      add_opvars(splev, "mio", SP_MAPCHAR_PACK(ROOM,-2), $6, SPO_WALLWALK);
		  }
		| WALLWALK_ID ':' coord_or_var ',' mapchar_or_var ',' mapchar_or_var opt_spercent
		  {
		      add_opvars(splev, "io", $8, SPO_WALLWALK);
		  }
		;

random_corridors: RAND_CORRIDOR_ID
		  {
		      add_opvars(splev, "iiiiiio", -1,  0, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
		| RAND_CORRIDOR_ID ':' all_integers
		  {
		      add_opvars(splev, "iiiiiio", -1, $3, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
		| RAND_CORRIDOR_ID ':' RANDOM_TYPE
		  {
		      add_opvars(splev, "iiiiiio", -1, -1, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
		;

corridor	: CORRIDOR_ID ':' corr_spec ',' corr_spec
		  {
		      add_opvars(splev, "iiiiiio",
				 $3.room, $3.door, $3.wall,
				 $5.room, $5.door, $5.wall,
				 SPO_CORRIDOR);
		  }
		| CORRIDOR_ID ':' corr_spec ',' all_integers
		  {
		      add_opvars(splev, "iiiiiio",
				 $3.room, $3.door, $3.wall,
				 -1, -1, (long)$5,
				 SPO_CORRIDOR);
		  }
		;

corr_spec	: '(' INTEGER ',' DIRECTION ',' door_pos ')'
		  {
			$$.room = $2;
			$$.wall = $4;
			$$.door = $6;
		  }
		;

room_begin      : room_type opt_percent ',' light_state
                  {
		      if (($2 < 100) && ($1 == OROOM))
			  lc_error("Only typed rooms can have a chance.");
		      else {
			  add_opvars(splev, "iii", (long)$1, (long)$2, (long)$4);
		      }
                  }
                ;

subroom_def	: SUBROOM_ID ':' room_begin ',' subroom_pos ',' room_size roomfill
		  {
		      add_opvars(splev, "iiiiiiio", (long)$8, ERR, ERR,
				 $5.x, $5.y, $7.width, $7.height, SPO_SUBROOM);
		  }
		  '{' levstatements '}'
		  {
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
		;

room_def	: ROOM_ID ':' room_begin ',' room_pos ',' room_align ',' room_size roomfill
		  {
		      add_opvars(splev, "iiiiiiio", (long)$10,
				 $7.x, $7.y, $5.x, $5.y,
				 $9.width, $9.height, SPO_ROOM);
		  }
		  '{' levstatements '}'
		  {
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
		;

roomfill	: /* nothing */
		  {
			$$ = 1;
		  }
		| ',' BOOLEAN
		  {
			$$ = $2;
		  }
		;

room_pos	: '(' INTEGER ',' INTEGER ')'
		  {
			if ( $2 < 1 || $2 > 5 ||
			    $4 < 1 || $4 > 5 ) {
			    lc_error("Room positions should be between 1-5: (%li,%li)!", $2, $4);
			} else {
			    $$.x = $2;
			    $$.y = $4;
			}
		  }
		| RANDOM_TYPE
		  {
			$$.x = $$.y = ERR;
		  }
		;

subroom_pos	: '(' INTEGER ',' INTEGER ')'
		  {
			if ( $2 < 0 || $4 < 0) {
			    lc_error("Invalid subroom position (%li,%li)!", $2, $4);
			} else {
			    $$.x = $2;
			    $$.y = $4;
			}
		  }
		| RANDOM_TYPE
		  {
			$$.x = $$.y = ERR;
		  }
		;

room_align	: '(' h_justif ',' v_justif ')'
		  {
		      $$.x = $2;
		      $$.y = $4;
		  }
		| RANDOM_TYPE
		  {
		      $$.x = $$.y = ERR;
		  }
		;

room_size	: '(' INTEGER ',' INTEGER ')'
		  {
			$$.width = $2;
			$$.height = $4;
		  }
		| RANDOM_TYPE
		  {
			$$.height = $$.width = ERR;
		  }
		;

door_detail	: ROOMDOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos
		  {
			/* ERR means random here */
			if ($7 == ERR && $9 != ERR) {
			    lc_error("If the door wall is random, so must be its pos!");
			} else {
			    add_opvars(splev, "iiiio", (long)$9, (long)$5, (long)$3, (long)$7, SPO_ROOM_DOOR);
			}
		  }
		| DOOR_ID ':' door_state ',' coord_or_var
		  {
		      add_opvars(splev, "io", (long)$3, SPO_DOOR);
		  }
		;

secret		: BOOLEAN
		| RANDOM_TYPE
		;

door_wall	: dir_list
		| RANDOM_TYPE
		;

dir_list	: DIRECTION
		  {
		      $$ = $1;
		  }
		| DIRECTION '|' dir_list
		  {
		      $$ = ($1 | $3);
		  }
		;

door_pos	: INTEGER
		| RANDOM_TYPE
		;

map_definition	: NOMAP_ID
		  {
		      add_opvars(splev, "ciisiio", 0, 0, 1, (char *)0, 0, 0, SPO_MAP);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| map_geometry roomfill MAP_ID
		  {
		      add_opvars(splev, "cii", ((long)($1 % 10) & 0xff) + (((long)($1 / 10) & 0xff) << 16), 1, (long)$2);
		      scan_map($3, splev);
		      Free($3);
		  }
		| GEOMETRY_ID ':' coord_or_var roomfill MAP_ID
		  {
		      add_opvars(splev, "ii", 2, (long)$4);
		      scan_map($5, splev);
		      Free($5);
		  }
		;

map_geometry	: GEOMETRY_ID ':' h_justif ',' v_justif
		  {
			$$ = $3 + ($5 * 10);
		  }
		;

h_justif	: LEFT_OR_RIGHT
		| CENTER
		;

v_justif	: TOP_OR_BOT
		| CENTER
		;

sounds_detail	: SOUNDS_ID ':' integer_or_var ',' sounds_list
		  {
		      long n_sounds = $5;
		      add_opvars(splev, "io", n_sounds, SPO_LEVEL_SOUNDS);
		  }
		;

sounds_list	: lvl_sound_part
		  {
		      $$ = 1;
		  }
		| lvl_sound_part ',' sounds_list
		  {
		      $$ = 1 + $3;
		  }
		;

lvl_sound_part	: '(' MSG_OUTPUT_TYPE ',' string_expr ')'
		  {
		      add_opvars(splev, "i", (long)$2);
		  }
		;

mon_generation	: MON_GENERATION_ID ':' SPERCENT ',' mon_gen_list
		  {
		      long chance = $3;
		      long total_mons = $5;
		      if (chance < 0) chance = 0;
		      else if (chance > 100) chance = 100;

		      if (total_mons < 1) lc_error("Monster generation: zero monsters defined?");
		      add_opvars(splev, "iio", chance, total_mons, SPO_MON_GENERATION);
		  }
		;

mon_gen_list	: mon_gen_part
		  {
		      $$ = 1;
		  }
		| mon_gen_part ',' mon_gen_list
		  {
		      $$ = 1 + $3;
		  }
		;

mon_gen_part	: '(' integer_or_var ',' monster ')'
		  {
		      long token = $4;
		      if (token == ERR) lc_error("Monster generation: Invalid monster symbol");
		      add_opvars(splev, "ii", token, 1);
		  }
		| '(' integer_or_var ',' string ')'
		  {
		      long token;
		      token = get_monster_id($4, (char)0);
		      if (token == ERR) lc_error("Monster generation: Invalid monster name");
		      add_opvars(splev, "ii", token, 0);
		  }
		;

monster_detail	: MONSTER_ID chance ':' monster_desc
		  {
		      add_opvars(splev, "io", 0, SPO_MONSTER);

		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("Conditional creation of monster, but no jump point marker.");
		      }
		  }
		| MONSTER_ID chance ':' monster_desc
		  {
		      add_opvars(splev, "io", 1, SPO_MONSTER);
		      $<i>$ = $2;
		      in_container_obj++;
		  }
		'{' levstatements '}'
		 {
		     in_container_obj--;
		     add_opvars(splev, "o", SPO_END_MONINVENT);
		     if ( 1 == $<i>5 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional creation of monster, but no jump point marker.");
		     }
		 }
		;

monster_desc	: monster_or_var ',' coord_or_var monster_infos
		  {
		      /* nothing */
		  }
		;

monster_infos	: /* nothing */
		  {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_M_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      $$ = 0x0000;
		  }
		| monster_infos ',' monster_info
		  {
		      if (( $1 & $3 ))
			  lc_error("MONSTER extra info defined twice.");
		      $$ = ( $1 | $3 );
		  }
		;

monster_info	: string_expr
		  {
		      add_opvars(splev, "i", SP_M_V_NAME);
		      $$ = 0x0001;
		  }
		| MON_ATTITUDE
		  {
		      add_opvars(splev, "ii", (long)$<i>1, SP_M_V_PEACEFUL);
		      $$ = 0x0002;
		  }
		| MON_ALERTNESS
		  {
		      add_opvars(splev, "ii", (long)$<i>1, SP_M_V_ASLEEP);
		      $$ = 0x0004;
		  }
		| alignment_prfx
		  {
		      add_opvars(splev, "ii", (long)$1, SP_M_V_ALIGN);
		      $$ = 0x0008;
		  }
		| MON_APPEARANCE string_expr
		  {
		      add_opvars(splev, "ii", (long)$<i>1, SP_M_V_APPEAR);
		      $$ = 0x0010;
		  }
		| FEMALE_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_FEMALE);
		      $$ = 0x0020;
		  }
		| INVIS_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_INVIS);
		      $$ = 0x0040;
		  }
		| CANCELLED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_CANCELLED);
		      $$ = 0x0080;
		  }
		| REVIVED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_REVIVED);
		      $$ = 0x0100;
		  }
		| AVENGE_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_AVENGE);
		      $$ = 0x0200;
		  }
		| FLEEING_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_M_V_FLEEING);
		      $$ = 0x0400;
		  }
		| BLINDED_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_M_V_BLINDED);
		      $$ = 0x0800;
		  }
		| PARALYZED_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_M_V_PARALYZED);
		      $$ = 0x1000;
		  }
		| STUNNED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_STUNNED);
		      $$ = 0x2000;
		  }
		| CONFUSED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_M_V_CONFUSED);
		      $$ = 0x4000;
		  }
		| SEENTRAPS_ID ':' seen_trap_mask
		  {
		      add_opvars(splev, "ii", (long)$3, SP_M_V_SEENTRAPS);
		      $$ = 0x8000;
		  }
		;

seen_trap_mask	: STRING
		  {
		      int token = get_trap_type($1);
		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", $1);
		      $$ = (1L << (token - 1));
		  }
		| ALL_ID
		  {
		      $$ = (long) ~0;
		  }
		| STRING '|' seen_trap_mask
		  {
		      int token = get_trap_type($1);
		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", $1);

		      if ((1L << (token - 1)) & $3)
			  lc_error("Monster seen_traps, trap '%s' listed twice.", $1);

		      $$ = ((1L << (token - 1)) | $3);
		  }
		;

object_detail	: OBJECT_ID chance ':' object_desc
		  {
		      long cnt = 0;
		      if (in_container_obj) cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", cnt, SPO_OBJECT);
		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("conditional creation of obj, but no jump point marker.");
		      }
		  }
		| COBJECT_ID chance ':' object_desc
		  {
		      long cnt = SP_OBJ_CONTAINER;
		      if (in_container_obj) cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", cnt, SPO_OBJECT);
		      $<i>$ = $2;
		      in_container_obj++;
		  }
		'{' levstatements '}'
		 {
		     in_container_obj--;
		     add_opcode(splev, SPO_POP_CONTAINER, NULL);

		     if ( 1 == $<i>5 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional creation of obj, but no jump point marker.");
		     }
		 }
		;

object_desc	: object_or_var object_infos
		  {
		      if (( $2 & 0x4000) && in_container_obj) lc_error("Object cannot have a coord when contained.");
		      else if (!( $2 & 0x4000) && !in_container_obj) lc_error("Object needs a coord when not contained.");
		  }
		;

object_infos	: /* nothing */
		  {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_O_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      $$ = 0x00;
		  }
		| object_infos ',' object_info
		  {
		      if (( $1 & $3 ))
			  lc_error("OBJECT extra info defined twice.");
		      $$ = ( $1 | $3 );
		  }
		;

object_info	: CURSE_TYPE
		  {
		      add_opvars(splev, "ii", (long)$1, SP_O_V_CURSE);
		      $$ = 0x0001;
		  }
		| MONTYPE_ID ':' monster_or_var
		  {
		      add_opvars(splev, "i", SP_O_V_CORPSENM);
		      $$ = 0x0002;
		  }
		| all_ints_push
		  {
		      add_opvars(splev, "i", SP_O_V_SPE);
		      $$ = 0x0004;
		  }
		| NAME_ID ':' string_expr
		  {
		      add_opvars(splev, "i", SP_O_V_NAME);
		      $$ = 0x0008;
		  }
		| QUANTITY_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_O_V_QUAN);
		      $$ = 0x0010;
		  }
		| BURIED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_O_V_BURIED);
		      $$ = 0x0020;
		  }
		| LIGHT_STATE
		  {
		      add_opvars(splev, "ii", (long)$1, SP_O_V_LIT);
		      $$ = 0x0040;
		  }
		| ERODED_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_O_V_ERODED);
		      $$ = 0x0080;
		  }
		| ERODEPROOF_ID
		  {
		      add_opvars(splev, "ii", -1, SP_O_V_ERODED);
		      $$ = 0x0080;
		  }
		| DOOR_STATE
		  {
		      if ($1 == D_LOCKED) {
			  add_opvars(splev, "ii", 1, SP_O_V_LOCKED);
			  $$ = 0x0100;
		      } else if ($1 == D_BROKEN) {
			  add_opvars(splev, "ii", 1, SP_O_V_BROKEN);
			  $$ = 0x0200;
		      } else
			  lc_error("OBJECT state can only be locked or broken.");
		  }
		| TRAPPED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_O_V_TRAPPED);
		      $$ = 0x0400;
		  }
		| RECHARGED_ID ':' integer_or_var
		  {
		      add_opvars(splev, "i", SP_O_V_RECHARGED);
		      $$ = 0x0800;
		  }
		| INVIS_ID
		  {
		      add_opvars(splev, "ii", 1, SP_O_V_INVIS);
		      $$ = 0x1000;
		  }
		| GREASED_ID
		  {
		      add_opvars(splev, "ii", 1, SP_O_V_GREASED);
		      $$ = 0x2000;
		  }
		| coord_or_var
		  {
		      add_opvars(splev, "i", SP_O_V_COORD);
		      $$ = 0x4000;
		  }
		;

trap_detail	: TRAP_ID chance ':' trap_name ',' coord_or_var
		  {
		      add_opvars(splev, "io", (long)$4, SPO_TRAP);
		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("Conditional creation of trap, but no jump point marker.");
		      }
		  }
		;

drawbridge_detail: DRAWBRIDGE_ID ':' coord_or_var ',' DIRECTION ',' door_state
		   {
		       long d, state = 0;
		       /* convert dir from a DIRECTION to a DB_DIR */
		       d = $5;
		       switch(d) {
		       case W_NORTH: d = DB_NORTH; break;
		       case W_SOUTH: d = DB_SOUTH; break;
		       case W_EAST:  d = DB_EAST;  break;
		       case W_WEST:  d = DB_WEST;  break;
		       default:
			   lc_error("Invalid drawbridge direction.");
			   break;
		       }

		       if ( $7 == D_ISOPEN )
			   state = 1;
		       else if ( $7 == D_CLOSED )
			   state = 0;
		       else
			   lc_error("A drawbridge can only be open or closed!");
		       add_opvars(splev, "iio", state, d, SPO_DRAWBRIDGE);
		   }
		;

mazewalk_detail : MAZEWALK_ID ':' coord_or_var ',' DIRECTION
		  {
		      add_opvars(splev, "iiio",
				 (long)$5, 1, 0, SPO_MAZEWALK);
		  }
		| MAZEWALK_ID ':' coord_or_var ',' DIRECTION ',' BOOLEAN opt_fillchar
		  {
		      add_opvars(splev, "iiio",
				 (long)$5, (long)$<i>7, (long)$8, SPO_MAZEWALK);
		  }
		;

wallify_detail	: WALLIFY_ID
		  {
		      add_opvars(splev, "ro", SP_REGION_PACK(-1,-1,-1,-1), SPO_WALLIFY);
		  }
		| WALLIFY_ID ':' region_or_var
		  {
		      add_opvars(splev, "o", SPO_WALLIFY);
		  }
		;

ladder_detail	: LADDER_ID ':' coord_or_var ',' UP_OR_DOWN
		  {
		      add_opvars(splev, "io", (long)$<i>5, SPO_LADDER);
		  }
		;

stair_detail	: STAIR_ID ':' coord_or_var ',' UP_OR_DOWN
		  {
		      add_opvars(splev, "io", (long)$<i>5, SPO_STAIR);
		  }
		;

stair_region	: STAIR_ID ':' lev_region ',' lev_region ',' UP_OR_DOWN
		  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 $3.x1, $3.y1, $3.x2, $3.y2, $3.area,
				 $5.x1, $5.y1, $5.x2, $5.y2, $5.area,
				 (long)(($7) ? LR_UPSTAIR : LR_DOWNSTAIR),
				 0, (char *)0, SPO_LEVREGION);
		  }
		;

portal_region	: PORTAL_ID ':' lev_region ',' lev_region ',' string
		  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 $3.x1, $3.y1, $3.x2, $3.y2, $3.area,
				 $5.x1, $5.y1, $5.x2, $5.y2, $5.area,
				 LR_PORTAL, 0, $7, SPO_LEVREGION);
		      Free($7);
		  }
		;

teleprt_region	: TELEPRT_ID ':' lev_region ',' lev_region teleprt_detail
		  {
		      long rtype;
		      switch($6) {
		      case -1: rtype = LR_TELE; break;
		      case  0: rtype = LR_DOWNTELE; break;
		      case  1: rtype = LR_UPTELE; break;
		      }
		      add_opvars(splev, "iiiii iiiii iiso",
				 $3.x1, $3.y1, $3.x2, $3.y2, $3.area,
				 $5.x1, $5.y1, $5.x2, $5.y2, $5.area,
				 rtype, 0, (char *)0, SPO_LEVREGION);
		  }
		;

branch_region	: BRANCH_ID ':' lev_region ',' lev_region
		  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 $3.x1, $3.y1, $3.x2, $3.y2, $3.area,
				 $5.x1, $5.y1, $5.x2, $5.y2, $5.area,
				 (long)LR_BRANCH, 0, (char *)0, SPO_LEVREGION);
		  }
		;

teleprt_detail	: /* empty */
		  {
			$$ = -1;
		  }
		| ',' UP_OR_DOWN
		  {
			$$ = $2;
		  }
		;

fountain_detail : FOUNTAIN_ID ':' coord_or_var
		  {
		      add_opvars(splev, "o", SPO_FOUNTAIN);
		  }
		;

sink_detail : SINK_ID ':' coord_or_var
		  {
		      add_opvars(splev, "o", SPO_SINK);
		  }
		;

pool_detail : POOL_ID ':' coord_or_var
		  {
		      add_opvars(splev, "o", SPO_POOL);
		  }
		;

terrain_type	: CHAR
		  {
		      $$.lit = -2;
		      $$.ter = what_map_char((char) $<i>1);
		  }
		| '(' CHAR ',' light_state ')'
		  {
		      $$.lit = $4;
		      $$.ter = what_map_char((char) $<i>2);
		  }
		;

replace_terrain_detail : REPLACE_TERRAIN_ID ':' region_or_var ',' mapchar_or_var ',' mapchar_or_var ',' SPERCENT
		  {
		      long chance;

		      chance = $9;
		      if (chance < 0) chance = 0;
		      else if (chance > 100) chance = 100;
		      add_opvars(splev, "io", chance, SPO_REPLACETERRAIN);
		  }
		;

terrain_detail : TERRAIN_ID chance ':' coord_or_var ',' mapchar_or_var
		 {
		     add_opvars(splev, "io", 0, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional terrain modification, but no jump point marker.");
		     }
		 }
	       |
	         TERRAIN_ID chance ':' coord_or_var ',' HORIZ_OR_VERT ',' INTEGER ',' mapchar_or_var
		 {
		     long areatyp, x2,y2;

		     areatyp = $<i>6; /* 1 or 2 */
		     if (areatyp == 1) {
			 x2 = $8;
			 y2 = -1;
		     } else {
			 x2 = -1;
			 y2 = $8;
		     }

		     add_opvars(splev, "cio",	SP_COORD_PACK(x2,y2),
				areatyp, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional terrain modification, but no jump point marker.");
		     }
		 }
	       |
	         TERRAIN_ID chance ':' rect_ID region_or_var ',' FILLING ',' mapchar_or_var
		 {
		     long areatyp = (3 + $<i>7); /* 3 or 4*/
		     add_opvars(splev, "io", areatyp, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional terrain modification, but no jump point marker.");
		     }
		 }
	       ;

randline_detail : RANDLINE_ID ':' coord_or_var ',' coord_or_var ',' mapchar_or_var ',' INTEGER opt_int
		  {
		      add_opvars(splev, "iio", (long)$9, (long)$10, SPO_RANDLINE);
		  }

opt_int		: /* empty */
		  {
			$$ = 0;
		  }
		| ',' INTEGER
		  {
			$$ = $2;
		  }
		;

spill_detail : SPILL_ID ':' coord_or_var ',' terrain_type ',' DIRECTION ',' INTEGER
		{
		    long c, typ;

		    typ = $5.ter;
		    if (typ == INVALID_TYPE || typ >= MAX_TYPE) {
			lc_error("SPILL: Invalid map character!");
		    }

		    c = $9;
		    if (c < 1) lc_error("SPILL: Invalid count '%li'!", c);

		    add_opvars(splev, "iiiio", typ, (long)$7, c, (long)$5.lit, SPO_SPILL);
		}
		;

diggable_detail : NON_DIGGABLE_ID ':' region_or_var
		  {
		     add_opvars(splev, "o", SPO_NON_DIGGABLE);
		  }
		;

passwall_detail : NON_PASSWALL_ID ':' region_or_var
		  {
		     add_opvars(splev, "o", SPO_NON_PASSWALL);
		  }
		;

region_detail	: REGION_ID ':' region_or_var ',' light_state ',' room_type prefilled
		  {
		      long rt, irr;
		      rt = $7;
		      if (( $8 ) & 1) rt += MAXRTYPE+1;
		      irr = ((( $8 ) & 2) != 0);
		      add_opvars(splev, "iiio",
				 (long)$5, rt, irr, SPO_REGION);
		      $<i>$ = (irr || ($8 & 1) || rt != OROOM);
		  }
		  region_detail_end
		  {
		      if ( $<i>9 ) {
			  add_opcode(splev, SPO_ENDROOM, NULL);
		      } else if ( $<i>10 )
			  lc_error("Cannot use lev statements in non-permanent REGION");
		  }
		;

region_detail_end : /* nothing */
		  {
		      $$ = 0;
		  }
		| '{' levstatements '}'
		  {
		      $$ = $2;
		  }
		;

altar_detail	: ALTAR_ID ':' coord_or_var ',' alignment ',' altar_type
		  {
		      add_opvars(splev, "iio", (long)$7, (long)$5, SPO_ALTAR);
		  }
		;

grave_detail	: GRAVE_ID ':' coord_or_var ',' string_expr
		  {
		      add_opvars(splev, "io", 2, SPO_GRAVE);
		  }
		| GRAVE_ID ':' coord_or_var ',' RANDOM_TYPE
		  {
		      add_opvars(splev, "sio",
				 (char *)0, 1, SPO_GRAVE);
		  }
		| GRAVE_ID ':' coord_or_var
		  {
		      add_opvars(splev, "sio",
				 (char *)0, 0, SPO_GRAVE);
		  }
		;

gold_detail	: GOLD_ID ':' math_expr ',' coord_or_var
		  {
		      add_opvars(splev, "o", SPO_GOLD);
		  }
		;

engraving_detail: ENGRAVING_ID ':' coord_or_var ',' engraving_type ',' string_expr
		  {
		      add_opvars(splev, "io",
				 (long)$5, SPO_ENGRAVING);
		  }
		;

trap_name	: string
		  {
			int token = get_trap_type($1);
			if (token == ERR)
			    lc_error("Unknown trap type '%s'!", $1);
			$$ = token;
			Free($1);
		  }
		| RANDOM_TYPE
		;

room_type	: string
		  {
			int token = get_room_type($1);
			if (token == ERR) {
			    lc_warning("Unknown room type \"%s\"!  Making ordinary room...", $1);
				$$ = OROOM;
			} else
				$$ = token;
			Free($1);
		  }
		| RANDOM_TYPE
		;

prefilled	: /* empty */
		  {
			$$ = 0;
		  }
		| ',' FILLING
		  {
			$$ = $2;
		  }
		| ',' FILLING ',' BOOLEAN
		  {
			$$ = $2 + ($4 << 1);
		  }
		;

door_state	: DOOR_STATE
		| RANDOM_TYPE
		;

light_state	: LIGHT_STATE
		| RANDOM_TYPE
		;

alignment	: ALIGNMENT
		| a_register
		| RANDOM_TYPE
		  {
			$$ = - MAX_REGISTERS - 1;
		  }
		;

alignment_prfx	: ALIGNMENT
		| a_register
		| A_REGISTER ':' RANDOM_TYPE
		  {
			$$ = - MAX_REGISTERS - 1;
		  }
		;

altar_type	: ALTAR_TYPE
		| RANDOM_TYPE
		;

a_register	: A_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= 3 )
				lc_error("Register Index overflow!");
			else
				$$ = - $3 - 1;
		  }
		;

monster		: CHAR
		  {
			if (check_monster_char((char) $1))
				$$ = $1 ;
			else {
			    lc_error("Unknown monster class '%c'!", $1);
			    $$ = ERR;
			}
		  }
		;

string_or_var	: STRING
		  {
		      add_opvars(splev, "s", $1);
		      Free($1);
		  }
		| VARSTRING_STRING
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_STRING);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_STRING_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_STRING|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;


integer_or_var	: math_expr_var
		  {
		      /* nothing */
		  }
		;

coord_or_var	: encodecoord
		  {
		      add_opvars(splev, "c", $1);
		  }
		| VARSTRING_COORD
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_COORD);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_COORD_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_COORD|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;

encodecoord	: '(' INTEGER ',' INTEGER ')'
		  {
		      if ($2 < 0 || $4 < 0 || $2 >= COLNO || $4 >= ROWNO)
			  lc_error("Coordinates (%li,%li) out of map range!", $2, $4);
		      $$ = SP_COORD_PACK($2, $4);
		  }
		| RANDOM_TYPE
		  {
		      $$ = SP_COORD_PACK(-1,-1);
		  }
		;

region_or_var	: encoderegion
		  {
		      add_opvars(splev, "r", $1);
		  }
		| VARSTRING_REGION
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_REGION);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_REGION_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_REGION|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;

encoderegion	: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
		      if ( $2 > $6 || $4 > $8 )
			  lc_error("Region start > end: (%li,%li,%li,%li)!", $2, $4, $6, $8);

		      $$ = SP_REGION_PACK($2, $4, $6, $8);
		  }
		;

mapchar_or_var	: mapchar
		  {
		      add_opvars(splev, "m", $1);
		  }
		| VARSTRING_MAPCHAR
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_MAPCHAR);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_MAPCHAR_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_MAPCHAR|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;

mapchar		: CHAR
		  {
		      if (what_map_char((char) $1) != INVALID_TYPE)
			  $$ = SP_MAPCHAR_PACK(what_map_char((char) $1), -2);
		      else {
			  lc_error("Unknown map char type '%c'!", $1);
			  $$ = SP_MAPCHAR_PACK(STONE, -2);
		      }
		  }
		| '(' CHAR ',' light_state ')'
		  {
		      if (what_map_char((char) $2) != INVALID_TYPE)
			  $$ = SP_MAPCHAR_PACK(what_map_char((char) $2), $4);
		      else {
			  lc_error("Unknown map char type '%c'!", $2);
			  $$ = SP_MAPCHAR_PACK(STONE, $4);
		      }
		  }
		;

monster_or_var	: encodemonster
		  {
		      add_opvars(splev, "M", $1);
		  }
		| VARSTRING_MONST
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_MONST);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_MONST_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_MONST|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;

encodemonster	: STRING
		  {
		      long m = get_monster_id($1, (char)0);
		      if (m == ERR) {
			  lc_error("Unknown monster \"%s\"!", $1);
			  $$ == -1;
		      } else
			  $$ = SP_MONST_PACK(m, def_monsyms[(int)mons[m].mlet]);
		  }
		| CHAR
		  {
			if (check_monster_char((char) $1))
			    $$ = SP_MONST_PACK(-1, $1);
			else {
			    lc_error("Unknown monster class '%c'!", $1);
			    $$ = -1;
			}
		  }
		| '(' CHAR ',' STRING ')'
		  {
		      long m = get_monster_id($4, (char) $2);
		      if (m == ERR) {
			  lc_error("Unknown monster ('%c', \"%s\")!", $2, $4);
			  $$ == -1;
		      } else
			  $$ = SP_MONST_PACK(m, $2);
		  }
		| RANDOM_TYPE
		  {
		      $$ = -1;
		  }
		;

object_or_var	: encodeobj
		  {
		      add_opvars(splev, "O", $1);
		  }
		| VARSTRING_OBJ
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_OBJ);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_OBJ_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_OBJ|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		;

encodeobj	: STRING
		  {
		      long m = get_object_id($1, (char)0);
		      if (m == ERR) {
			  lc_error("Unknown object \"%s\"!", $1);
			  $$ == -1;
		      } else
			  $$ = SP_OBJ_PACK(m, 1); /* obj class != 0 to force generation of a specific item */

		  }
		| CHAR
		  {
			if (check_object_char((char) $1))
			    $$ = SP_OBJ_PACK(-1, $1);
			else {
			    lc_error("Unknown object class '%c'!", $1);
			    $$ = -1;
			}
		  }
		| '(' CHAR ',' STRING ')'
		  {
		      long m = get_object_id($4, (char) $2);
		      if (m == ERR) {
			  lc_error("Unknown object ('%c', \"%s\")!", $2, $4);
			  $$ == -1;
		      } else
			  $$ = SP_OBJ_PACK(m, $2);
		  }
		| RANDOM_TYPE
		  {
		      $$ = -1;
		  }
		;


string_expr	: string_or_var                 { }
		| string_expr '.' string_or_var
		  {
		      add_opvars(splev, "o", SPO_MATH_ADD);
		  }
		;

math_expr_var	: INTEGER                       { add_opvars(splev, "i", $1 ); }
		| dice				{ }
		| '(' MINUS_INTEGER ')'         { add_opvars(splev, "i", $2 ); }
		| VARSTRING_INT
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_INT);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| VARSTRING_INT_ARRAY '[' math_expr ']'
		  {
		      check_vardef_type(variable_definitions, $1, SPOVAR_INT|SPOVAR_ARRAY);
		      add_opvars(splev, "v", $1);
		      Free($1);
		  }
		| math_expr_var '+' math_expr_var       { add_opvars(splev, "o", SPO_MATH_ADD); }
		| math_expr_var '-' math_expr_var       { add_opvars(splev, "o", SPO_MATH_SUB); }
		| math_expr_var '*' math_expr_var       { add_opvars(splev, "o", SPO_MATH_MUL); }
		| math_expr_var '/' math_expr_var       { add_opvars(splev, "o", SPO_MATH_DIV); }
		| math_expr_var '%' math_expr_var       { add_opvars(splev, "o", SPO_MATH_MOD); }
		| '(' math_expr ')'             { }
		;

math_expr	: INTEGER                       { add_opvars(splev, "i", $1 ); }
		| dice				{ }
		| '(' MINUS_INTEGER ')'         { add_opvars(splev, "i", $2 ); }
		| math_expr '+' math_expr       { add_opvars(splev, "o", SPO_MATH_ADD); }
		| math_expr '-' math_expr       { add_opvars(splev, "o", SPO_MATH_SUB); }
		| math_expr '*' math_expr       { add_opvars(splev, "o", SPO_MATH_MUL); }
		| math_expr '/' math_expr       { add_opvars(splev, "o", SPO_MATH_DIV); }
		| math_expr '%' math_expr       { add_opvars(splev, "o", SPO_MATH_MOD); }
		| '(' math_expr ')'             { }
		;

dice		: DICE
		  {
		      add_opvars(splev, "iio", $1.num, $1.die, SPO_DICE);
		  }
		;

all_integers	: MINUS_INTEGER
		| PLUS_INTEGER
		| INTEGER
		;

all_ints_push	: MINUS_INTEGER
		  {
		      add_opvars(splev, "i", $1 );
		  }
		| PLUS_INTEGER
		  {
		      add_opvars(splev, "i", $1 );
		  }
		| INTEGER
		  {
		      add_opvars(splev, "i", $1 );
		  }
		| dice
		  {
		      /* nothing */
		  }
		;

string		: STRING
		;

chance		: /* empty */
		  {
		      /* by default we just do it, unconditionally. */
		      $$ = 0;
		  }
		| comparestmt
		  {
		      /* otherwise we generate an IF-statement */
		      struct opvar *tmppush2 = New(struct opvar);
		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("Comparison: Too deeply nested IFs.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      add_opcode(splev, SPO_CMP, NULL);
		      set_opvar_int(tmppush2, splev->n_opcodes+1);
		      if_list[n_if_list++] = tmppush2;
		      add_opcode(splev, SPO_PUSH, tmppush2);
		      add_opcode(splev, reverse_jmp_opcode( $1 ), NULL);
		      $$ = 1;
		  }
		;

engraving_type	: ENGRAVING_TYPE
		| RANDOM_TYPE
		;

lev_region	: region
		  {
			$$ = $1;
		  }
		| LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
			if ($3 <= 0 || $3 >= COLNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (x1)!", $3, $5, $7, $9);
			else if ($5 < 0 || $5 >= ROWNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (y1)!", $3, $5, $7, $9);
			else if ($7 <= 0 || $7 >= COLNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (x2)!", $3, $5, $7, $9);
			else if ($9 < 0 || $9 >= ROWNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (y2)!", $3, $5, $7, $9);
			$$.x1 = $3;
			$$.y1 = $5;
			$$.x2 = $7;
			$$.y2 = $9;
			$$.area = 1;
		  }
		;

region		: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ($2 < 0 || $2 > (int)max_x_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (x1)!", $2, $4, $6, $8);
			else if ($4 < 0 || $4 > (int)max_y_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (y1)!", $2, $4, $6, $8);
			else if ($6 < 0 || $6 > (int)max_x_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (x2)!", $2, $4, $6, $8);
			else if ($8 < 0 || $8 > (int)max_y_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (y2)!", $2, $4, $6, $8);
			$$.area = 0;
			$$.x1 = $2;
			$$.y1 = $4;
			$$.x2 = $6;
			$$.y2 = $8;
		  }
		;


%%

/*lev_comp.y*/
