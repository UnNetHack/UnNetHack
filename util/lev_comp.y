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

#define New(type)		\
	(type *) memset((genericptr_t)alloc(sizeof(type)), 0, sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

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
extern struct opvar *FDECL(set_opvar_str, (struct opvar *, char *));
extern void VDECL(add_opvars, (sp_lev *, const char *, ...));

static struct reg {
	int x1, y1;
	int x2, y2;
}		current_region;

static struct coord {
	int x;
	int y;
}		current_coord, current_align;

static struct size {
	int height;
	int width;
}		current_size;

sp_lev splev;

static char olist[MAX_REGISTERS], mlist[MAX_REGISTERS];
static struct coord plist[MAX_REGISTERS];
static struct opvar *if_list[MAX_NESTED_IFS];

static short n_olist = 0, n_mlist = 0, n_plist = 0, n_if_list = 0;
static short on_olist = 0, on_mlist = 0, on_plist = 0;

static long lev_flags;

unsigned int max_x_map, max_y_map;
int obj_containment = 0;

extern int fatal_error;
extern const char *fname;

%}

%union
{
	int	i;
	char*	map;
	struct {
		xchar room;
		xchar wall;
		xchar door;
	} corpos;
}


%token	<i> CHAR INTEGER BOOLEAN PERCENT SPERCENT
%token	<i> MAZE_GRID_ID SOLID_FILL_ID MINES_ID
%token	<i> MESSAGE_ID LEVEL_ID LEV_INIT_ID GEOMETRY_ID NOMAP_ID
%token	<i> OBJECT_ID COBJECT_ID MONSTER_ID TRAP_ID DOOR_ID DRAWBRIDGE_ID
%token	<i> MAZEWALK_ID WALLIFY_ID REGION_ID FILLING
%token	<i> RANDOM_OBJECTS_ID RANDOM_MONSTERS_ID RANDOM_PLACES_ID
%token	<i> ALTAR_ID LADDER_ID STAIR_ID NON_DIGGABLE_ID NON_PASSWALL_ID ROOM_ID
%token	<i> PORTAL_ID TELEPRT_ID BRANCH_ID LEV CHANCE_ID RANDLINE_ID
%token	<i> CORRIDOR_ID GOLD_ID ENGRAVING_ID FOUNTAIN_ID POOL_ID SINK_ID NONE
%token	<i> RAND_CORRIDOR_ID DOOR_STATE LIGHT_STATE CURSE_TYPE ENGRAVING_TYPE
%token	<i> DIRECTION RANDOM_TYPE O_REGISTER M_REGISTER P_REGISTER A_REGISTER
%token	<i> ALIGNMENT LEFT_OR_RIGHT CENTER TOP_OR_BOT ALTAR_TYPE UP_OR_DOWN
%token	<i> SUBROOM_ID NAME_ID FLAGS_ID FLAG_TYPE MON_ATTITUDE MON_ALERTNESS
%token	<i> MON_APPEARANCE ROOMDOOR_ID IF_ID ELSE_ID
%token	<i> SPILL_ID TERRAIN_ID HORIZ_OR_VERT REPLACE_TERRAIN_ID
%token	<i> EXIT_ID
%token	<i> ',' ':' '(' ')' '[' ']' '{' '}'
%token	<map> STRING MAP_ID
%type	<i> h_justif v_justif trap_name room_type door_state light_state
%type	<i> alignment altar_type a_register roomfill door_pos
%type	<i> door_wall walled secret amount chance
%type	<i> dir_list
%type	<i> engraving_type flags flag_list prefilled lev_region lev_init
%type	<i> monster monster_c m_register object object_c o_register
%type	<i> comparestmt
%type	<map> string level_def m_name o_name
%type	<corpos> corr_spec
%start	file

%%
file		: /* nothing */
		| levels
		;

levels		: level
		| level levels
		;

level		: level_def flags lev_init levstatements
		  {
			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
			        splev.init_lev.init_style = (xchar) $3;
				splev.init_lev.flags = (long) $2;
				if (!write_level_file($1, &splev)) {
				    yyerror("Can't write output file!!");
				    exit(EXIT_FAILURE);
				}
			}
			Free($1);
		  }
		;

level_def	: LEVEL_ID ':' string
		  {
			if (index($3, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen($3) > 8)
			    yyerror("Level names limited to 8 characters.");
			n_plist = n_mlist = n_olist = 0;
			$$ = $3;
		  }
		;

lev_init	: /* nothing */
		  {
			$$ = LVLINIT_NONE;
		  }
		| LEV_INIT_ID ':' SOLID_FILL_ID ',' CHAR
		  {
		      splev.init_lev.filling = what_map_char((char) $5);
		      if (splev.init_lev.filling == INVALID_TYPE ||
			  splev.init_lev.filling >= MAX_TYPE)
			    yyerror("INIT_MAP: Invalid fill char type.");
		      $$ = LVLINIT_SOLIDFILL;
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| LEV_INIT_ID ':' MAZE_GRID_ID ',' CHAR
		  {
		      splev.init_lev.filling = what_map_char((char) $5);
		      if (splev.init_lev.filling == INVALID_TYPE ||
			  splev.init_lev.filling >= MAX_TYPE)
			    yyerror("INIT_MAP: Invalid fill char type.");
		      $$ = LVLINIT_MAZEGRID;
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| LEV_INIT_ID ':' MINES_ID ',' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled opt_fillchar
		  {
			splev.init_lev.fg = what_map_char((char) $5);
			if (splev.init_lev.fg == INVALID_TYPE ||
			  splev.init_lev.filling >= MAX_TYPE)
			    yyerror("INIT_MAP: Invalid foreground type.");
			splev.init_lev.bg = what_map_char((char) $7);
			if (splev.init_lev.bg == INVALID_TYPE ||
			  splev.init_lev.filling >= MAX_TYPE)
			    yyerror("INIT_MAP: Invalid background type.");
			splev.init_lev.smoothed = $9;
			splev.init_lev.joined = $11;
			if (splev.init_lev.joined &&
			    splev.init_lev.fg != CORR && splev.init_lev.fg != ROOM)
			    yyerror("INIT_MAP: Invalid foreground type for joined map.");
			splev.init_lev.lit = $13;
			splev.init_lev.walled = $15;

			splev.init_lev.filling = $<i>16;
			if (splev.init_lev.filling == INVALID_TYPE)
			    yyerror("INIT_MAP: Invalid fill char type.");

			$$ = LVLINIT_MINES;
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
		;

opt_fillchar	: /* nothing */
		  {
		      $<i>$ = -1;
		  }
		| ',' CHAR
		  {
		      $<i>$ = what_map_char((char) $2);
		  }
		;


walled		: BOOLEAN
		| RANDOM_TYPE
		;

flags		: /* nothing */
		  {
			$$ = 0;
		  }
		| FLAGS_ID ':' flag_list
		  {
			$$ = lev_flags;
			lev_flags = 0;	/* clear for next user */
		  }
		;

flag_list	: FLAG_TYPE ',' flag_list
		  {
			lev_flags |= $1;
		  }
		| FLAG_TYPE
		  {
			lev_flags |= $1;
		  }
		;

levstatements	: /* nothing */
		| levstatement levstatements
		;

levstatement 	: message
		| altar_detail
		| branch_region
		| corridor
		| diggable_detail
		| door_detail
		| drawbridge_detail
		| engraving_detail
		| fountain_detail
		| gold_detail
		| ifstatement
		| exitstatement
		| init_reg
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
		| room_chance
		| room_name
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

exitstatement	: EXIT_ID
		  {
		      add_opcode(&splev, SPO_EXIT, NULL);
		  }
		;


comparestmt     : PERCENT
                  {
		      /* val > rn2(100) */
		      add_opvars(&splev, "ioi", 100, SPO_RN2, $1);
		      $$ = SPO_JGE; /* TODO: shouldn't this be SPO_JG? */
                  }
		;

ifstatement 	: IF_ID comparestmt
		  {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  yyerror("IF: Too deeply nested IFs.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(&splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, -1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(&splev, SPO_PUSH, tmppush2);

		      add_opcode(&splev, $2, NULL);
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
			  set_opvar_int(tmppush, splev.init_lev.n_opcodes-1);
		      } else yyerror("IF: Huh?!  No start address?");
		  }
		| '{' levstatements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush = New(struct opvar);
			  struct opvar *tmppush2;

			  set_opvar_int(tmppush, -1);
			  add_opcode(&splev, SPO_PUSH, tmppush);

			  add_opcode(&splev, SPO_JMP, NULL);

			  tmppush2 = (struct opvar *) if_list[--n_if_list];

			  set_opvar_int(tmppush2, splev.init_lev.n_opcodes-1);
			  if_list[n_if_list++] = tmppush;
		      } else yyerror("IF: Huh?!  No else-part address?");
		  }
		 ELSE_ID '{' levstatements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev.init_lev.n_opcodes-1);
		      } else yyerror("IF: Huh?! No end address?");
		  }
		;

message		: MESSAGE_ID ':' STRING
		  {
		      if (strlen($3) > 254)
			  yyerror("Message string > 255 characters.");
		      else {
			  add_opvars(&splev, "so", $3, SPO_MESSAGE);
		      }
		  }
		;

cobj_ifstatement : IF_ID '[' comparestmt ']'
		  {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  yyerror("IF: Too deeply nested IFs.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(&splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, -1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(&splev, SPO_PUSH, tmppush2);

		      add_opcode(&splev, $3, NULL);
		  }
		 cobj_if_ending
		  {
		     /* do nothing */
		  }
		;

cobj_if_ending	: '{' cobj_statements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev.init_lev.n_opcodes-1);
		      } else yyerror("IF: Huh?!  No start address?");
		  }
		| '{' cobj_statements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush = New(struct opvar);
			  struct opvar *tmppush2;

			  set_opvar_int(tmppush, -1);
			  add_opcode(&splev, SPO_PUSH, tmppush);

			  add_opcode(&splev, SPO_JMP, NULL);

			  tmppush2 = (struct opvar *) if_list[--n_if_list];

			  set_opvar_int(tmppush2, splev.init_lev.n_opcodes-1);
			  if_list[n_if_list++] = tmppush;
		      } else yyerror("IF: Huh?!  No else-part address?");
		  }
		 ELSE_ID '{' cobj_statements '}'
		  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev.init_lev.n_opcodes-1);
		      } else yyerror("IF: Huh?! No end address?");
		  }
		;



random_corridors: RAND_CORRIDOR_ID
		  {
		      add_opvars(&splev, "iiiiiio", -1, -1, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
		;

corridor	: CORRIDOR_ID ':' corr_spec ',' corr_spec
		  {
		      add_opvars(&splev, "iiiiiio",
				 $3.room, $3.door, $3.wall,
				 $5.room, $5.door, $5.wall,
				 SPO_CORRIDOR);
		  }
		| CORRIDOR_ID ':' corr_spec ',' INTEGER
		  {
		      add_opvars(&splev, "iiiiiio",
				 $3.room, $3.door, $3.wall,
				 -1, -1, $5,
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

room_begin      : room_type chance ',' light_state
                  {
		      if (($2 == 1) && ($1 == OROOM))
			  yyerror("Only typed rooms can have a chance.");
		      else {
			  add_opvars(&splev, "iii", $1, $2, $4);
		      }
                  }
                ;

subroom_def	: SUBROOM_ID ':' room_begin ',' subroom_pos ',' room_size roomfill
		  {
		      add_opvars(&splev, "iiiiiiio", $8, ERR, ERR,
				 current_coord.x, current_coord.y,
				 current_size.width, current_size.height,
				 SPO_SUBROOM);
		  }
		  '{' levstatements '}'
		  {
		      add_opcode(&splev, SPO_ENDROOM, NULL);
		  }
		;

room_def	: ROOM_ID ':' room_begin ',' room_pos ',' room_align ',' room_size roomfill
		  {
		      add_opvars(&splev, "iiiiiiio", $10,
				 current_align.x, current_align.y,
				 current_coord.x, current_coord.y,
				 current_size.width, current_size.height,
				 SPO_ROOM);
		  }
		  '{' levstatements '}'
		  {
		      add_opcode(&splev, SPO_ENDROOM, NULL);
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
			    yyerror("Room position should be between 1 & 5!");
			} else {
			    current_coord.x = $2;
			    current_coord.y = $4;
			}
		  }
		| RANDOM_TYPE
		  {
			current_coord.x = current_coord.y = ERR;
		  }
		;

subroom_pos	: '(' INTEGER ',' INTEGER ')'
		  {
			if ( $2 < 0 || $4 < 0) {
			    yyerror("Invalid subroom position !");
			} else {
			    current_coord.x = $2;
			    current_coord.y = $4;
			}
		  }
		| RANDOM_TYPE
		  {
			current_coord.x = current_coord.y = ERR;
		  }
		;

room_align	: '(' h_justif ',' v_justif ')'
		  {
			current_align.x = $2;
			current_align.y = $4;
		  }
		| RANDOM_TYPE
		  {
			current_align.x = current_align.y = ERR;
		  }
		;

room_size	: '(' INTEGER ',' INTEGER ')'
		  {
			current_size.width = $2;
			current_size.height = $4;
		  }
		| RANDOM_TYPE
		  {
			current_size.height = current_size.width = ERR;
		  }
		;

room_name	: NAME_ID ':' string
		  {
		      yyerror("NAME for rooms is not used anymore.");
		      Free($3);
		  }
		;

room_chance	: CHANCE_ID ':' INTEGER
		   {
		       yyerror("CHANCE for rooms is not used anymore.");
		   }
		;

door_detail	: ROOMDOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos
		  {
			/* ERR means random here */
			if ($7 == ERR && $9 != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			} else {
			    add_opvars(&splev, "iiiio", $9, $5, $3, $7, SPO_ROOM_DOOR);
			}
		  }
		| DOOR_ID ':' door_state ',' coordinate
		  {
		      add_opvars(&splev, "iiio",
				 current_coord.x, current_coord.y,
				 $<i>3, SPO_DOOR);
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
		      add_opvars(&splev, "iiiisiio", 0, 1, 1, 1, (char *)0, 0, 0, SPO_MAP);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
		| map_geometry roomfill MAP_ID
		  {
		      add_opvars(&splev, "iiii", 1, $2, ($<i>1 % 10), ($<i>1 / 10));
		      scan_map($3, &splev);
		      Free($3);
		  }
		| GEOMETRY_ID ':' coordinate roomfill MAP_ID
		  {
		      add_opvars(&splev, "iiii", 2, $4, current_coord.x, current_coord.y);
		      scan_map($5, &splev);
		      Free($5);
		  }
		;

map_geometry	: GEOMETRY_ID ':' h_justif ',' v_justif
		  {
			$<i>$ = $<i>3 + ($<i>5 * 10);
		  }
		;

h_justif	: LEFT_OR_RIGHT
		| CENTER
		;

v_justif	: TOP_OR_BOT
		| CENTER
		;

init_reg	: RANDOM_OBJECTS_ID ':' object_list
		  {
		     char *tmp_olist;

		     tmp_olist = (char *) alloc(n_olist+1);
		     (void) memcpy((genericptr_t)tmp_olist,
				   (genericptr_t)olist, n_olist);
		     tmp_olist[n_olist] = 0;
		     add_opvars(&splev, "so", tmp_olist, SPO_RANDOM_OBJECTS);

		     on_olist = n_olist;
		     n_olist = 0;
		  }
		| RANDOM_PLACES_ID ':' place_list
		  {
		     char *tmp_plist;
		     int i;

		     tmp_plist = (char *) alloc(n_plist*2+1);

		     for (i=0; i<n_plist; i++) {
			tmp_plist[i*2] = plist[i].x+1;
			tmp_plist[i*2+1] = plist[i].y+1;
		     }
		     tmp_plist[n_plist*2] = 0;
		     add_opvars(&splev, "so", tmp_plist, SPO_RANDOM_PLACES);

		     on_plist = n_plist;
		     n_plist = 0;
		  }
		| RANDOM_MONSTERS_ID ':' monster_list
		  {
		     char *tmp_mlist;

		     tmp_mlist = (char *) alloc(n_mlist+1);
		     (void) memcpy((genericptr_t)tmp_mlist,
				   (genericptr_t)mlist, n_mlist);
		     tmp_mlist[n_mlist] = 0;
		     add_opvars(&splev, "so", tmp_mlist, SPO_RANDOM_MONSTERS);
		     on_mlist = n_mlist;
		     n_mlist = 0;
		  }
		;

object_list	: object
		  {
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = $<i>1;
			else
			    yyerror("Object list too long!");
		  }
		| object ',' object_list
		  {
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = $<i>1;
			else
			    yyerror("Object list too long!");
		  }
		;

monster_list	: monster
		  {
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = $<i>1;
			else
			    yyerror("Monster list too long!");
		  }
		| monster ',' monster_list
		  {
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = $<i>1;
			else
			    yyerror("Monster list too long!");
		  }
		;

place_list	: place
		  {
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
		| place
		  {
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
		 ',' place_list
		;

monster_detail	: MONSTER_ID chance ':' monster_desc
		  {
		      add_opcode(&splev, SPO_MONSTER, NULL);

		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			  } else yyerror("conditional creation of monster, but no jump point marker.");
		      }
		  }
		;

monster_desc	: monster_c ',' m_name ',' coordinate monster_infos
		  {
		      int token = NON_PM;
		      if ($3) {
			  token = get_monster_id($3, (char) $<i>1);
			  if (token == ERR) {
			      yywarning("Invalid monster name!  Making random monster.");
			      token = NON_PM;
			  }
			  Free($3);
		      }
		      add_opvars(&splev, "iiii", current_coord.x, current_coord.y, $<i>1, token);
		  }
		;

monster_infos	: /* nothing */
		  {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_M_V_END);
		      add_opcode(&splev, SPO_PUSH, stopit);
		      $<i>$ = 0x00;
		  }
		| monster_infos monster_info
		  {
		      if (( $<i>1 & $<i>2 ))
			  yyerror("MONSTER extra info already used.");
		      $<i>$ = ( $<i>1 | $<i>2 );
		  }
		;

monster_info	: ',' string
		  {
		      add_opvars(&splev, "si", $2, SP_M_V_NAME);
		      $<i>$ = 0x01;
		  }
		| ',' MON_ATTITUDE
		  {
		      add_opvars(&splev, "ii", $<i>2, SP_M_V_PEACEFUL);
		      $<i>$ = 0x02;
		  }
		| ',' MON_ALERTNESS
		  {
		      add_opvars(&splev, "ii", $<i>2, SP_M_V_ASLEEP);
		      $<i>$ = 0x04;
		  }
		| ',' alignment
		  {
		      add_opvars(&splev, "ii", $<i>2, SP_M_V_ALIGN);
		      $<i>$ = 0x08;
		  }
		| ',' MON_APPEARANCE string
		  {
		      add_opvars(&splev, "sii", $3, $<i>2, SP_M_V_APPEAR);
		      $<i>$ = 0x10;
		  }
		;

cobj_statements	: /* nothing */
		  {
		  }
		| cobj_statement cobj_statements
		;

cobj_statement  : cobj_detail
		| cobj_ifstatement
		;

cobj_detail	: OBJECT_ID chance ':' cobj_desc
		  {
		      add_opvars(&splev, "io", SP_OBJ_CONTENT, SPO_OBJECT);
		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			  } else yyerror("conditional creation of obj, but no jump point marker.");
		      }
		  }
		| COBJECT_ID chance ':' cobj_desc
		  {
		      add_opvars(&splev, "io", SP_OBJ_CONTENT|SP_OBJ_CONTAINER, SPO_OBJECT);
		      $<i>$ = $2;
		  }
		'{' cobj_statements '}'
		  {
		      add_opcode(&splev, SPO_POP_CONTAINER, NULL);

		      if ( 1 == $<i>5 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			  } else yyerror("conditional creation of obj, but no jump point marker.");
		      }
		  }
		;

object_detail	: OBJECT_ID chance ':' object_desc
		  {
		      add_opvars(&splev, "io", 0, SPO_OBJECT); /* 0 == not container, nor contents of one. */
		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			  } else yyerror("conditional creation of obj, but no jump point marker.");
		      }
		  }
		| COBJECT_ID chance ':' object_desc
		  {
		      add_opvars(&splev, "io", SP_OBJ_CONTAINER, SPO_OBJECT);
		      $<i>$ = $2;
		  }
		'{' cobj_statements '}'
		 {
		     add_opcode(&splev, SPO_POP_CONTAINER, NULL);

		     if ( 1 == $<i>5 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			 } else yyerror("conditional creation of obj, but no jump point marker.");
		     }
		 }
		;

cobj_desc	: object_c ',' o_name object_infos
		  {
		      int token = -1;
		      if ($3) {
			  token = get_object_id($3, $<i>1);
			  if (token == ERR) {
			      yywarning("Illegal object name!  Making random object.");
			      token = -1;
			  }
			  Free($3);
		      }
		      add_opvars(&splev, "iiii", -1, -1, $<i>1, token);
		  }
		;

object_desc	: object_c ',' o_name ',' coordinate object_infos
		  {
		      int token = -1;
		      if ($3) {
			  token = get_object_id($3, $<i>1);
			  if (token == ERR) {
			      yywarning("Illegal object name!  Making random object.");
			      token = -1;
			  }
			  Free($3);
		      }
		      add_opvars(&splev, "iiii", current_coord.x, current_coord.y, $<i>1, token);
		}
		;

object_infos	: /* nothing */
		  {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_O_V_END);
		      add_opcode(&splev, SPO_PUSH, stopit);
		      $<i>$ = 0x00;
		  }
		| object_infos object_info
		  {
		      if (( $<i>1 & $<i>2 ))
			  yyerror("OBJECT extra info already used.");
		      $<i>$ = ( $<i>1 | $<i>2 );
		  }
		;

object_info	: ',' CURSE_TYPE
		  {
		      add_opvars(&splev, "ii", $2, SP_O_V_CURSE);
		      $<i>$ = 0x01;
		  }
		| ',' STRING
		  {
		      int token = get_monster_id($2, (char)0);
		      if (token == ERR) {
			  /* "random" */
			  yywarning("OBJECT: Are you sure you didn't mean NAME:\"foo\"?");
			  token = NON_PM - 1;
		      }
		      add_opvars(&splev, "ii", token, SP_O_V_CORPSENM);
		      Free($2);
		      $<i>$ = 0x02;
		  }
		| ',' INTEGER
		  {
		      add_opvars(&splev, "ii", $2, SP_O_V_SPE);
		      $<i>$ = 0x04;
		  }
		| ',' NAME_ID ':' STRING
		  {
		      add_opvars(&splev, "si", $4, SP_O_V_NAME);
		      $<i>$ = 0x08;
		  }
		;

trap_detail	: TRAP_ID chance ':' trap_name ',' coordinate
		  {
		      add_opvars(&splev, "iiio", current_coord.x, current_coord.y, $<i>4, SPO_TRAP);
		      if ( 1 == $2 ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			  } else yyerror("conditional creation of trap, but no jump point marker.");
		      }
		  }
		;

drawbridge_detail: DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state
		   {
		       int d, state = 0;
		       /* convert dir from a DIRECTION to a DB_DIR */
		       d = $5;
		       switch(d) {
		       case W_NORTH: d = DB_NORTH; break;
		       case W_SOUTH: d = DB_SOUTH; break;
		       case W_EAST:  d = DB_EAST;  break;
		       case W_WEST:  d = DB_WEST;  break;
		       default:
			   yyerror("Invalid drawbridge direction");
			   break;
		       }

		       if ( $<i>7 == D_ISOPEN )
			   state = 1;
		       else if ( $<i>7 == D_CLOSED )
			   state = 0;
		       else
			   yyerror("A drawbridge can only be open or closed!");
		       add_opvars(&splev, "iiiio", current_coord.x, current_coord.y, state, d, SPO_DRAWBRIDGE);
		   }
		;

mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION
		  {
		      add_opvars(&splev, "iiiiio",
				 current_coord.x, current_coord.y,
				 $5, 1, 0, SPO_MAZEWALK);
		  }
		| MAZEWALK_ID ':' coordinate ',' DIRECTION ',' BOOLEAN opt_fillchar
		  {
		      add_opvars(&splev, "iiiiio",
				 current_coord.x, current_coord.y,
				 $5, $<i>7, $<i>8, SPO_MAZEWALK);
		  }
		;

wallify_detail	: WALLIFY_ID
		  {
		      add_opcode(&splev, SPO_WALLIFY, NULL);
		  }
		;

ladder_detail	: LADDER_ID ':' coordinate ',' UP_OR_DOWN
		  {
		      add_opvars(&splev, "iiio", current_coord.x, current_coord.y, $<i>5, SPO_LADDER);
		  }
		;

stair_detail	: STAIR_ID ':' coordinate ',' UP_OR_DOWN
		  {
		      add_opvars(&splev, "iiio", current_coord.x, current_coord.y, $<i>5, SPO_STAIR);
		  }
		;

stair_region	: STAIR_ID ':' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $3);
		  }
		  ',' lev_region ',' UP_OR_DOWN
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $6);

		      add_opvars(&splev, "iiso",
				 (($8) ? LR_UPSTAIR : LR_DOWNSTAIR),
				 0, (char *)0, SPO_LEVREGION);
		  }
		;

portal_region	: PORTAL_ID ':' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $3);
		  }
		 ',' lev_region ',' string
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $6);

		      add_opvars(&splev, "iiso",
				 LR_PORTAL,
				 0, $8, SPO_LEVREGION);
		  }
		;

teleprt_region	: TELEPRT_ID ':' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $3);
		  }
		 ',' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $6);
		  }
		teleprt_detail
		  {
		      int rtype;
		      switch($<i>8) {
		      case -1: rtype = LR_TELE; break;
		      case  0: rtype = LR_DOWNTELE; break;
		      case  1: rtype = LR_UPTELE; break;
		      }
		      add_opvars(&splev, "iiso",
				 rtype,
				 0, (char *)0, SPO_LEVREGION);
		  }
		;

branch_region	: BRANCH_ID ':' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $3);
		  }
		 ',' lev_region
		  {
		      add_opvars(&splev, "iiiii",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2, $6);

		      add_opvars(&splev, "iiso",
				 LR_BRANCH,
				 0, (char *)0, SPO_LEVREGION);
		  }
		;

teleprt_detail	: /* empty */
		  {
			$<i>$ = -1;
		  }
		| ',' UP_OR_DOWN
		  {
			$<i>$ = $2;
		  }
		;

fountain_detail : FOUNTAIN_ID ':' coordinate
		  {
		      add_opvars(&splev, "iio", current_coord.x, current_coord.y, SPO_FOUNTAIN);
		  }
		;

sink_detail : SINK_ID ':' coordinate
		  {
		      add_opvars(&splev, "iio", current_coord.x, current_coord.y, SPO_SINK);
		  }
		;

pool_detail : POOL_ID ':' coordinate
		  {
		      add_opvars(&splev, "iio", current_coord.x, current_coord.y, SPO_POOL);
		  }
		;

replace_terrain_detail : REPLACE_TERRAIN_ID ':' region ',' CHAR ',' CHAR ',' light_state ',' SPERCENT
		  {
		      int chance, from_ter, to_ter;

		      chance = $11;
		      if (chance < 0) chance = 0;
		      else if (chance > 100) chance = 100;

		      from_ter = what_map_char((char) $5);
		      if (from_ter >= MAX_TYPE) yyerror("Replace terrain: illegal 'from' map char");

		      to_ter = what_map_char((char) $7);
		      if (to_ter >= MAX_TYPE) yyerror("Replace terrain: illegal 'to' map char");

		      add_opvars(&splev, "iiii iiiio",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2,
				 from_ter, to_ter, $9, chance, SPO_REPLACETERRAIN);
		  }
		;

terrain_detail : TERRAIN_ID chance ':' coordinate ',' CHAR ',' light_state
		 {
		     int c;

		     c = what_map_char((char) $6);
		     if (c >= MAX_TYPE) yyerror("Terrain: illegal map char");

		     add_opvars(&splev, "iiii iiio",
				current_coord.x, current_coord.y, -1, -1,
				0, c, $8, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			 } else yyerror("conditional terrain modification, but no jump point marker.");
		     }
		 }
	       |
	         TERRAIN_ID chance ':' coordinate ',' HORIZ_OR_VERT ',' INTEGER ',' CHAR ',' light_state
		 {
		     int areatyp, c, x2,y2;

		     areatyp = $<i>6;
		     if (areatyp == 1) {
			 x2 = $8;
			 y2 = -1;
		     } else {
			 x2 = -1;
			 y2 = $8;
		     }

		     c = what_map_char((char) $10);
		     if (c >= MAX_TYPE) yyerror("Terrain: illegal map char");

		     add_opvars(&splev, "iiii iiio",
				current_coord.x, current_coord.y, x2, y2,
				areatyp, c, $12, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			 } else yyerror("conditional terrain modification, but no jump point marker.");
		     }
		 }
	       |
	         TERRAIN_ID chance ':' region ',' FILLING ',' CHAR ',' light_state
		 {
		     int c;

		     c = what_map_char((char) $8);
		     if (c >= MAX_TYPE) yyerror("Terrain: illegal map char");

		     add_opvars(&splev, "iiii iiio",
				current_region.x1, current_region.y1, current_region.x2, current_region.y2,
				3 + $<i>6, c, $10, SPO_TERRAIN);

		     if ( 1 == $2 ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev.init_lev.n_opcodes-1);
			 } else yyerror("conditional terrain modification, but no jump point marker.");
		     }
		 }
	       ;

randline_detail : RANDLINE_ID ':' lineends ',' CHAR ',' light_state ',' INTEGER opt_int
		  {
		      int c;
		      c = what_map_char((char) $5);
		      if ((c == INVALID_TYPE) || (c >= MAX_TYPE)) yyerror("Terrain: illegal map char");
		      add_opvars(&splev, "iiii iiiio",
				 current_region.x1, current_region.y1,
				 current_region.x2, current_region.y2,
				 c, $7, $9, $<i>10, SPO_RANDLINE);
		  }

opt_int		: /* empty */
		  {
			$<i>$ = 0;
		  }
		| ',' INTEGER
		  {
			$<i>$ = $2;
		  }
		;

spill_detail : SPILL_ID ':' coordinate ',' CHAR ',' DIRECTION ',' INTEGER ',' light_state
		{
		    int c, typ;

		    typ = what_map_char((char) $5);
		    if (typ == INVALID_TYPE || typ >= MAX_TYPE) {
			yyerror("SPILL: Invalid map character!");
		    }

		    c = $9;
		    if (c < 1) yyerror("SPILL: Invalid count!");

		    add_opvars(&splev, "iiiiiio", current_coord.x, current_coord.y,
			       typ, $7, c, $11, SPO_SPILL);
		}
		;

diggable_detail : NON_DIGGABLE_ID ':' region
		  {
		     add_opvars(&splev, "iiiio",
				current_region.x1, current_region.y1,
				current_region.x2, current_region.y2, SPO_NON_DIGGABLE);
		  }
		;

passwall_detail : NON_PASSWALL_ID ':' region
		  {
		     add_opvars(&splev, "iiiio",
				current_region.x1, current_region.y1,
				current_region.x2, current_region.y2, SPO_NON_PASSWALL);
		  }
		;

region_detail	: REGION_ID ':' region ',' light_state ',' room_type prefilled
		  {
		      int rt, irr;

		      rt = $<i>7;
		      if (( $<i>8 ) & 1) rt += MAXRTYPE+1;

		      irr = ((( $<i>8 ) & 2) != 0);

		      if(current_region.x1 > current_region.x2 ||
			 current_region.y1 > current_region.y2)
			  yyerror("Region start > end!");

		      if (rt == VAULT && (irr ||
					 (current_region.x2 - current_region.x1 != 1) ||
					 (current_region.y2 - current_region.y1 != 1)))
			 yyerror("Vaults must be exactly 2x2!");

		     add_opvars(&splev, "iiii iiio",
				current_region.x1, current_region.y1,
				current_region.x2, current_region.y2,
				$<i>5, rt, irr, SPO_REGION);
		  }
		;

altar_detail	: ALTAR_ID ':' coordinate ',' alignment ',' altar_type
		  {
		      add_opvars(&splev, "iiiio", current_coord.x, current_coord.y,
				 $<i>7, $<i>5, SPO_ALTAR);
		  }
		;

gold_detail	: GOLD_ID ':' amount ',' coordinate
		  {
		      add_opvars(&splev, "iiio", $<i>3, current_coord.y, current_coord.x, SPO_GOLD);
		  }
		;

engraving_detail: ENGRAVING_ID ':' coordinate ',' engraving_type ',' string
		  {
		      add_opvars(&splev, "iisio",
				 current_coord.x, current_coord.y,
				 $7, $<i>5, SPO_ENGRAVING);
		  }
		;

monster_c	: monster
		| RANDOM_TYPE
		  {
			$<i>$ = - MAX_REGISTERS - 1;
		  }
		| m_register
		;

object_c	: object
		| RANDOM_TYPE
		  {
			$<i>$ = - MAX_REGISTERS - 1;
		  }
		| o_register
		;

m_name		: string
		| RANDOM_TYPE
		  {
			$$ = (char *) 0;
		  }
		;

o_name		: string
		| RANDOM_TYPE
		  {
			$$ = (char *) 0;
		  }
		;

trap_name	: string
		  {
			int token = get_trap_type($1);
			if (token == ERR)
				yyerror("Unknown trap type!");
			$<i>$ = token;
			Free($1);
		  }
		| RANDOM_TYPE
		;

room_type	: string
		  {
			int token = get_room_type($1);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				$<i>$ = OROOM;
			} else
				$<i>$ = token;
			Free($1);
		  }
		| RANDOM_TYPE
		;

prefilled	: /* empty */
		  {
			$<i>$ = 0;
		  }
		| ',' FILLING
		  {
			$<i>$ = $2;
		  }
		| ',' FILLING ',' BOOLEAN
		  {
			$<i>$ = $2 + ($4 << 1);
		  }
		;

coordinate	: coord
		| p_register
		| RANDOM_TYPE
		  {
			current_coord.x = current_coord.y = -MAX_REGISTERS-1;
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
			$<i>$ = - MAX_REGISTERS - 1;
		  }
		;

altar_type	: ALTAR_TYPE
		| RANDOM_TYPE
		;

p_register	: P_REGISTER '[' INTEGER ']'
		  {
		        if (on_plist == 0)
		                yyerror("No random places defined!");
			else if ( $3 >= on_plist )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - $3 - 1;
		  }
		;

o_register	: O_REGISTER '[' INTEGER ']'
		  {
		        if (on_olist == 0)
		                yyerror("No random objects defined!");
			else if ( $3 >= on_olist )
				yyerror("Register Index overflow!");
			else
				$<i>$ = - $3 - 1;
		  }
		;

m_register	: M_REGISTER '[' INTEGER ']'
		  {
		        if (on_mlist == 0)
		                yyerror("No random monsters defined!");
			if ( $3 >= on_mlist )
				yyerror("Register Index overflow!");
			else
				$<i>$ = - $3 - 1;
		  }
		;

a_register	: A_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= 3 )
				yyerror("Register Index overflow!");
			else
				$<i>$ = - $3 - 1;
		  }
		;

place		: coord
		;

monster		: CHAR
		  {
			if (check_monster_char((char) $1))
				$<i>$ = $1 ;
			else {
				yyerror("Unknown monster class!");
				$<i>$ = ERR;
			}
		  }
		;

object		: CHAR
		  {
			char c = $1;
			if (check_object_char(c))
				$<i>$ = c;
			else {
				yyerror("Unknown char class!");
				$<i>$ = ERR;
			}
		  }
		;

string		: STRING
		;

amount		: INTEGER
		| RANDOM_TYPE
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
			  yyerror("IF: Too deeply nested IFs.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      add_opcode(&splev, SPO_CMP, NULL);
		      set_opvar_int(tmppush2, -1);
		      if_list[n_if_list++] = tmppush2;
		      add_opcode(&splev, SPO_PUSH, tmppush2);
		      add_opcode(&splev, $1, NULL);
		      $$ = 1;
		  }
		;

engraving_type	: ENGRAVING_TYPE
		| RANDOM_TYPE
		;

coord		: '(' INTEGER ',' INTEGER ')'
		  {
		        if ($2 < 0 || $4 < 0 || $2 >= COLNO || $4 >= ROWNO)
		           yyerror("Coordinates out of map range!");
			current_coord.x = $2;
			current_coord.y = $4;
		  }
		;

lineends	: coordinate ','
		  {
			current_region.x1 = current_coord.x;
			current_region.y1 = current_coord.y;
		  }
		  coordinate
		  {
			current_region.x2 = current_coord.x;
			current_region.y2 = current_coord.y;
		  }
		;

lev_region	: region
		  {
			$$ = 0;
		  }
		| LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ($3 <= 0 || $3 >= COLNO)
				yyerror("Region out of level range!");
			else if ($5 < 0 || $5 >= ROWNO)
				yyerror("Region out of level range!");
			else if ($7 <= 0 || $7 >= COLNO)
				yyerror("Region out of level range!");
			else if ($9 < 0 || $9 >= ROWNO)
				yyerror("Region out of level range!");
			current_region.x1 = $3;
			current_region.y1 = $5;
			current_region.x2 = $7;
			current_region.y2 = $9;
			$$ = 1;
		  }
		;

region		: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ($2 < 0 || $2 > (int)max_x_map)
			  yyerror("Region out of map range (x1)!");
			else if ($4 < 0 || $4 > (int)max_y_map)
			  yyerror("Region out of map range (y1)!");
			else if ($6 < 0 || $6 > (int)max_x_map)
			  yyerror("Region out of map range (x2)!");
			else if ($8 < 0 || $8 > (int)max_y_map)
			  yyerror("Region out of map range (y2)!");
			current_region.x1 = $2;
			current_region.y1 = $4;
			current_region.x2 = $6;
			current_region.y2 = $8;
		  }
		;


%%

/*lev_comp.y*/
