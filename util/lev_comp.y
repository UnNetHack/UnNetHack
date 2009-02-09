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
extern void FDECL(scan_map, (char *, sp_lev *, mazepart *));
extern void FDECL(add_opcode, (sp_lev *, int, genericptr_t));
extern genericptr_t FDECL(get_last_opcode_data1, (sp_lev *, int));
extern genericptr_t FDECL(get_last_opcode_data2, (sp_lev *, int,int));
extern boolean FDECL(check_subrooms, (sp_lev *));
extern boolean FDECL(write_level_file, (char *,sp_lev *));

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
static long if_list[MAX_NESTED_IFS];

static short n_olist = 0, n_mlist = 0, n_plist = 0, n_if_list = 0;
static short on_olist = 0, on_mlist = 0, on_plist = 0;

static long lev_flags;

unsigned int max_x_map, max_y_map;

static xchar in_room;

extern int fatal_error;
extern int want_warnings;
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


%token	<i> CHAR INTEGER BOOLEAN PERCENT
%token	<i> MESSAGE_ID MAZE_ID LEVEL_ID LEV_INIT_ID GEOMETRY_ID NOMAP_ID
%token	<i> OBJECT_ID COBJECT_ID MONSTER_ID TRAP_ID DOOR_ID DRAWBRIDGE_ID
%token	<i> MAZEWALK_ID WALLIFY_ID REGION_ID FILLING
%token	<i> RANDOM_OBJECTS_ID RANDOM_MONSTERS_ID RANDOM_PLACES_ID
%token	<i> ALTAR_ID LADDER_ID STAIR_ID NON_DIGGABLE_ID NON_PASSWALL_ID ROOM_ID
%token	<i> PORTAL_ID TELEPRT_ID BRANCH_ID LEV CHANCE_ID
%token	<i> CORRIDOR_ID GOLD_ID ENGRAVING_ID FOUNTAIN_ID POOL_ID SINK_ID NONE
%token	<i> RAND_CORRIDOR_ID DOOR_STATE LIGHT_STATE CURSE_TYPE ENGRAVING_TYPE
%token	<i> DIRECTION RANDOM_TYPE O_REGISTER M_REGISTER P_REGISTER A_REGISTER
%token	<i> ALIGNMENT LEFT_OR_RIGHT CENTER TOP_OR_BOT ALTAR_TYPE UP_OR_DOWN
%token	<i> SUBROOM_ID NAME_ID FLAGS_ID FLAG_TYPE MON_ATTITUDE MON_ALERTNESS
%token	<i> MON_APPEARANCE ROOMDOOR_ID IF_ID THEN_ID ELSE_ID ENDIF_ID
%token	<i> CONTAINED
%token	<i> ',' ':' '(' ')' '[' ']'
%token	<map> STRING MAP_ID
%type	<i> h_justif v_justif trap_name room_type door_state light_state
%type	<i> alignment altar_type a_register roomfill filling door_pos
%type	<i> door_wall walled secret amount chance
%type	<i> engraving_type flags flag_list prefilled lev_region lev_init
%type	<i> monster monster_c m_register object object_c o_register
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
			        splev.init_lev.init_present = (boolean) $3;
				splev.init_lev.flags = (long) $2;
			        if (check_subrooms(&splev)) {
				   if (!write_level_file($1, &splev)) {
					yyerror("Can't write output file!!");
					exit(EXIT_FAILURE);
				   }
				}
			}
			Free($1);
		  }
		;

level_def	: MAZE_ID ':' string ',' filling
		  {
			splev.init_lev.filling = (schar) $5;
		        splev.init_lev.levtyp = SP_LEV_MAZE;
			if (index($3, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen($3) > 8)
			    yyerror("Level names limited to 8 characters.");
			$$ = $3;
			in_room = 0;
			n_plist = n_mlist = n_olist = 0;
		  }
		| LEVEL_ID ':' string
		  {
			splev.init_lev.levtyp = SP_LEV_ROOMS;
			if (index($3, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen($3) > 8)
			    yyerror("Level names limited to 8 characters.");
			$$ = $3;
		  }
		;

lev_init	: /* nothing */
		  {
			$$ = 0;
		  }
		| LEV_INIT_ID ':' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled
		  {
			splev.init_lev.fg = what_map_char((char) $3);
			if (splev.init_lev.fg == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			splev.init_lev.bg = what_map_char((char) $5);
			if (splev.init_lev.bg == INVALID_TYPE)
			    yyerror("Invalid background type.");
			splev.init_lev.smoothed = $7;
			splev.init_lev.joined = $9;
			if (splev.init_lev.joined &&
			    splev.init_lev.fg != CORR && splev.init_lev.fg != ROOM)
			    yyerror("Invalid foreground type for joined map.");
			splev.init_lev.lit = $11;
			splev.init_lev.walled = $13;
			$$ = 1;
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
		| room_chance
		| room_def
		| room_name
		| sink_detail
		| stair_detail
		| stair_region
		| subroom_def
		| teleprt_region
		| trap_detail
		| wallify_detail
		;

ifstatement 	: IF_ID chance
		  {
		     opcmp *tmpcmp = New(opcmp);
		     opjmp *tmpjmp = New(opjmp);

		     if (n_if_list >= MAX_NESTED_IFS)
		       yyerror("Too deeply nested IF-statements!");
		     tmpcmp->cmp_what = 0;
		     tmpcmp->cmp_val = (long) $2;
		     add_opcode(&splev, SPO_CMP, tmpcmp);
		     tmpjmp->jmp_target = -1;
		     if_list[n_if_list++] = splev.init_lev.n_opcodes;
		     add_opcode(&splev, SPO_JG, tmpjmp);
		  }
		 if_ending
		  {
		     /* do nothing */
		  }
		;

if_ending	: THEN_ID levstatements ENDIF_ID
		  {
		     if (n_if_list > 0) {
			opjmp *tmpjmp;
			tmpjmp = (opjmp *) splev.opcodes[if_list[--n_if_list]].opdat;
			tmpjmp->jmp_target = splev.init_lev.n_opcodes-1;
		     } else yyerror("IF...THEN ... huh?!");
		  }
		| THEN_ID levstatements
		  {
		     if (n_if_list > 0) {
			long tmppos = splev.init_lev.n_opcodes;
			opjmp *tmpjmp = New(opjmp);

			tmpjmp->jmp_target = -1;
			add_opcode(&splev, SPO_JMP, tmpjmp);

			tmpjmp = (opjmp *) splev.opcodes[if_list[--n_if_list]].opdat;
			tmpjmp->jmp_target = splev.init_lev.n_opcodes-1;

			if_list[n_if_list++] = tmppos;
		     } else yyerror("IF...THEN ... huh?!");
		  }
		 ELSE_ID levstatements ENDIF_ID
		  {
		     if (n_if_list > 0) {
			opjmp *tmpjmp;
			tmpjmp = (opjmp *) splev.opcodes[if_list[--n_if_list]].opdat;
			tmpjmp->jmp_target = splev.init_lev.n_opcodes-1;
		     } else yyerror("IF...THEN...ELSE ... huh?!");
		  }
		;

message		: MESSAGE_ID ':' STRING
		  {
		     if (strlen($3) > 254)
		       yyerror("Message string > 255 characters.");

		     add_opcode(&splev, SPO_MESSAGE, $3);
		  }
		;

random_corridors: RAND_CORRIDOR_ID
		  {
		     corridor *tmpcorridor = New(corridor);
		     tmpcorridor->src.room = -1;

		     add_opcode(&splev, SPO_CORRIDOR, tmpcorridor);
		  }
		;

corridor	: CORRIDOR_ID ':' corr_spec ',' corr_spec
		  {
		     corridor *tmpcor = New(corridor);

		     tmpcor->src.room = $3.room;
		     tmpcor->src.wall = $3.wall;
		     tmpcor->src.door = $3.door;
		     tmpcor->dest.room = $5.room;
		     tmpcor->dest.wall = $5.wall;
		     tmpcor->dest.door = $5.door;

		     add_opcode(&splev, SPO_CORRIDOR, tmpcor);
		  }
		| CORRIDOR_ID ':' corr_spec ',' INTEGER
		  {
		     corridor *tmpcor = New(corridor);

		     tmpcor->src.room = $3.room;
		     tmpcor->src.wall = $3.wall;
		     tmpcor->src.door = $3.door;
		     tmpcor->dest.room = -1;
		     tmpcor->dest.wall = $5;

		     add_opcode(&splev, SPO_CORRIDOR, tmpcor);
		  }
		;

corr_spec	: '(' INTEGER ',' DIRECTION ',' door_pos ')'
		  {
			$$.room = $2;
			$$.wall = $4;
			$$.door = $6;
		  }
		;

subroom_def	: SUBROOM_ID ':' room_type ',' light_state ',' subroom_pos ',' room_size ',' string roomfill
		  {
		     room *tmpr = New(room);

		     tmpr->parent.str = $11;
		     tmpr->name.str = (char *) 0;
		     tmpr->rtype = $3;
		     tmpr->rlit = $5;
		     tmpr->filled = $12;
		     tmpr->xalign = ERR;
		     tmpr->yalign = ERR;
		     tmpr->x = current_coord.x;
		     tmpr->y = current_coord.y;
		     tmpr->w = current_size.width;
		     tmpr->h = current_size.height;

		     add_opcode(&splev, SPO_SUBROOM, tmpr);

		     in_room = 1;
		  }
		;

room_def	: ROOM_ID ':' room_type ',' light_state ',' room_pos ',' room_align ',' room_size roomfill
		  {
		     room *tmpr = New(room);

		     tmpr->name.str = (char *) 0;
		     tmpr->parent.str = (char *) 0;
		     tmpr->rtype = $3;
		     tmpr->rlit = $5;
		     tmpr->filled = $12;
		     tmpr->xalign = current_align.x;
		     tmpr->yalign = current_align.y;
		     tmpr->x = current_coord.x;
		     tmpr->y = current_coord.y;
		     tmpr->w = current_size.width;
		     tmpr->h = current_size.height;

		     add_opcode(&splev, SPO_ROOM, tmpr);

		     in_room = 1;
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
		     room *tmpr = (room *) get_last_opcode_data2(&splev, SPO_ROOM, SPO_SUBROOM);

		     if (!tmpr)
		       yyerror("There's no room to name?!");

		     if (tmpr->name.str)
		       yyerror("This room already has a name!");
		     else
		       tmpr->name.str = $3;
		  }
		;

room_chance	: CHANCE_ID ':' INTEGER
		   {
		      room *tmpr = (room *) get_last_opcode_data2(&splev, SPO_ROOM, SPO_SUBROOM);

		      if (!tmpr)
			yyerror("There's no room to assign a chance to?!");

		      if (tmpr->chance)
			yyerror("This room already assigned a chance!");
		      else if (tmpr->rtype == OROOM)
			yyerror("Only typed rooms can have a chance!");
		      else if ($3 < 1 || $3 > 99)
			yyerror("The chance is supposed to be percentile.");
		      else
			tmpr->chance = $3;
		   }
		;

door_detail	: ROOMDOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos
		  {
			/* ERR means random here */
			if ($7 == ERR && $9 != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			} else {
			   room *tmpr = (room *) get_last_opcode_data2(&splev, SPO_ROOM, SPO_SUBROOM);
			   room_door *rdoor;

			   if (!tmpr)
			     yyerror("Roomdoor without room?!");

			   rdoor = New(room_door);

			   rdoor->secret = $3;
			   rdoor->mask = $5;
			   rdoor->wall = (schar) $7;
			   rdoor->pos = $9;

			   add_opcode(&splev, SPO_ROOM_DOOR, rdoor);
			}
		  }
		| DOOR_ID ':' door_state ',' coordinate
		  {
		     door *tmpdoor = New(door);

		     tmpdoor->x = current_coord.x;
		     tmpdoor->y = current_coord.y;
		     tmpdoor->mask = $<i>3;

		     add_opcode(&splev, SPO_DOOR, tmpdoor);
		  }
		;

secret		: BOOLEAN
		| RANDOM_TYPE
		;

door_wall	: DIRECTION
		| RANDOM_TYPE
		;

door_pos	: INTEGER
		| RANDOM_TYPE
		;

filling		: CHAR
		  {
			$$ = get_floor_type((char)$1);
		  }
		| RANDOM_TYPE
		  {
			$$ = -1;
		  }
		;

map_definition	: NOMAP_ID
		  {
		     mazepart *tmppart = New(mazepart);

		     tmppart->halign = 1;
		     tmppart->valign = 1;
		     tmppart->xsize = 0;
		     tmppart->ysize = 0;
		     max_x_map = COLNO-1;
		     max_y_map = ROWNO;

		     add_opcode(&splev, SPO_MAP, tmppart);

		  }
		| map_geometry MAP_ID
		  {
		     mazepart *tmpp = New(mazepart);

		     tmpp->halign = $<i>1 % 10;
		     tmpp->valign = $<i>1 / 10;
		     scan_map($2, &splev, tmpp);
		     Free($2);
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
		     add_opcode(&splev, SPO_RANDOM_OBJECTS, tmp_olist);
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
		     add_opcode(&splev, SPO_RANDOM_PLACES, tmp_plist);
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
		     add_opcode(&splev, SPO_RANDOM_MONSTERS, tmp_mlist);
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

monster_detail	: MONSTER_ID chance ':' monster_c ',' m_name ',' coordinate
		  {
		     monster *tmpm = New(monster);

		     tmpm->x = current_coord.x;
		     tmpm->y = current_coord.y;
		     tmpm->class = $<i>4;
		     tmpm->peaceful = -1; /* no override */
		     tmpm->asleep = -1;
		     tmpm->align = - MAX_REGISTERS - 2;
		     tmpm->name.str = 0;
		     tmpm->appear = 0;
		     tmpm->appear_as.str = 0;
		     tmpm->chance = $2;
		     tmpm->id = NON_PM;
		     if ($6) {
			int token = get_monster_id($6, (char) $<i>4);
			if (token == ERR)
			  yywarning(
			    "Invalid monster name!  Making random monster.");
			else
			  tmpm->id = token;
			Free($6);
		     }
		     add_opcode(&splev, SPO_MONSTER, tmpm);

		  }
		 monster_infos
		  {
		      /* nothing here */
		  }
		;

monster_infos	: /* nothing */
		| monster_infos monster_info
		;

monster_info	: ',' string
		  {
		     monster *tmpm =
		       (monster *) get_last_opcode_data1(&splev, SPO_MONSTER);
		     if (!tmpm) yyerror("No monster defined?!");
		     tmpm->name.str = $2;
		  }
		| ',' MON_ATTITUDE
		  {
		     monster *tmpm =
		       (monster *) get_last_opcode_data1(&splev, SPO_MONSTER);
		     if (!tmpm) yyerror("No monster defined?!");
		     tmpm->peaceful = $<i>2;
		  }
		| ',' MON_ALERTNESS
		  {
		     monster *tmpm =
		       (monster *) get_last_opcode_data1(&splev, SPO_MONSTER);
		     if (!tmpm) yyerror("No monster defined?!");
		     tmpm->asleep = $<i>2;
		  }
		| ',' alignment
		  {
		     monster *tmpm =
		       (monster *) get_last_opcode_data1(&splev, SPO_MONSTER);
		     if (!tmpm) yyerror("No monster defined?!");
		     tmpm->align = $<i>2;
		  }
		| ',' MON_APPEARANCE string
		  {
		     monster *tmpm =
		       (monster *) get_last_opcode_data1(&splev, SPO_MONSTER);
		     if (!tmpm) yyerror("No monster defined?!");
		     tmpm->appear = $<i>2;
		     tmpm->appear_as.str = $3;
		  }
		;

object_detail	: OBJECT_ID object_desc
		  {
		  }
		| COBJECT_ID object_desc
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->containment = 2;

			/* 1: is contents of preceeding object with 2 */
			/* 2: is a container */
			/* 0: neither */
		  }
		;

object_desc	: chance ':' object_c ',' o_name
		  {
		     object *tmpobj = New(object);

		     tmpobj->class = $<i>3;
		     tmpobj->corpsenm = NON_PM;
		     tmpobj->curse_state = -1;
		     tmpobj->name.str = 0;
		     tmpobj->chance = $1;
		     tmpobj->id = -1;
		     if ($5) {
			int token = get_object_id($5, $<i>3);
			if (token == ERR)
			  yywarning(
			    "Illegal object name!  Making random object.");
			else
			  tmpobj->id = token;
			Free($5);
		     }
		     add_opcode(&splev, SPO_OBJECT, tmpobj);

		  }
		 ',' object_where object_infos
		  {
		      /* nothing here */
		  }
		;

object_where	: coordinate
		  {
		     object *tmpobj = (object *)
		       get_last_opcode_data1(&splev, SPO_OBJECT);

		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->containment = 0;
		     tmpobj->x = current_coord.x;
		     tmpobj->y = current_coord.y;
		  }
		| CONTAINED
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);

		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->containment = 1;
		     /* random coordinate, will be overridden anyway */
		     tmpobj->x = -MAX_REGISTERS-1;
		     tmpobj->y = -MAX_REGISTERS-1;
		  }
		;

object_infos	: /* nothing */
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);

		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->spe = -127;
	/* Note below: we're trying to make as many of these optional as
	 * possible.  We clearly can't make curse_state, enchantment, and
	 * monster_id _all_ optional, since ",random" would be ambiguous.
	 * We can't even just make enchantment mandatory, since if we do that
	 * alone, ",random" requires too much lookahead to parse.
	 */
		  }
		| ',' curse_state ',' monster_id ',' enchantment optional_name
		  {
		  }
		| ',' curse_state ',' enchantment optional_name
		  {
		  }
		| ',' monster_id ',' enchantment optional_name
		  {
		  }
		;

curse_state	: RANDOM_TYPE
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->curse_state = -1;
		  }
		| CURSE_TYPE
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->curse_state = $1;
		  }
		;

monster_id	: STRING
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     int token = get_monster_id($1, (char)0);

		     if (!tmpobj) yyerror("No object defined?!");

		     if (token == ERR)	/* "random" */
		       tmpobj->corpsenm = NON_PM - 1;
		     else
		       tmpobj->corpsenm = token;
		     Free($1);
		  }
		;

enchantment	: RANDOM_TYPE
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->spe = -127;
		  }
		| INTEGER
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->spe = $1;
		  }
		;

optional_name	: /* nothing */
		| ',' NONE
		  {
		  }
		| ',' STRING
		  {
		     object *tmpobj =
		       (object *) get_last_opcode_data1(&splev, SPO_OBJECT);
		     if (!tmpobj) yyerror("No object defined?!");
		     tmpobj->name.str = $2;
		  }
		;

trap_detail	: TRAP_ID chance ':' trap_name ',' coordinate
		  {
		     trap *tmptrap = New(trap);

		     tmptrap->x = current_coord.x;
		     tmptrap->y = current_coord.y;
		     tmptrap->type = $<i>4;
		     tmptrap->chance = $2;

		     add_opcode(&splev, SPO_TRAP, tmptrap);
		  }
		;

drawbridge_detail: DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state
		   {
		        int x, y, dir;
			drawbridge *tmpdb = New(drawbridge);

			x = tmpdb->x = current_coord.x;
			y = tmpdb->y = current_coord.y;
			/* convert dir from a DIRECTION to a DB_DIR */
			dir = $5;
			switch(dir) {
			case W_NORTH: dir = DB_NORTH; y--; break;
			case W_SOUTH: dir = DB_SOUTH; y++; break;
			case W_EAST:  dir = DB_EAST;  x++; break;
			case W_WEST:  dir = DB_WEST;  x--; break;
			default:
			    yyerror("Invalid drawbridge direction");
			    break;
			}
			tmpdb->dir = dir;

			if ( $<i>7 == D_ISOPEN )
			    tmpdb->db_open = 1;
			else if ( $<i>7 == D_CLOSED )
			    tmpdb->db_open = 0;
			else
			    yyerror("A drawbridge can only be open or closed!");

			add_opcode(&splev, SPO_DRAWBRIDGE, tmpdb);
		   }
		;

mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION
		  {
		      walk *tmpwalk = New(walk);

		      tmpwalk->x = current_coord.x;
		      tmpwalk->y = current_coord.y;
		      tmpwalk->dir = $5;

		      add_opcode(&splev, SPO_MAZEWALK, tmpwalk);
		  }
		;

wallify_detail	: WALLIFY_ID
		  {
		      add_opcode(&splev, SPO_WALLIFY, NULL);
		  }
		;

ladder_detail	: LADDER_ID ':' coordinate ',' UP_OR_DOWN
		  {
		     lad *tmplad = New(lad);

		     tmplad->x = current_coord.x;
		     tmplad->y = current_coord.y;
		     tmplad->up = $<i>5;
		     add_opcode(&splev, SPO_LADDER, tmplad);
		  }
		;

stair_detail	: STAIR_ID ':' coordinate ',' UP_OR_DOWN
		  {
		     stair *tmpstair = New(stair);

		     tmpstair->x = current_coord.x;
		     tmpstair->y = current_coord.y;
		     tmpstair->up = $<i>5;
		     add_opcode(&splev, SPO_STAIR, tmpstair);
		  }
		;

stair_region	: STAIR_ID ':' lev_region
		  {
		      lev_region *tmplreg = New(lev_region);

		      tmplreg->in_islev = $3;
		      tmplreg->inarea.x1 = current_region.x1;
		      tmplreg->inarea.y1 = current_region.y1;
		      tmplreg->inarea.x2 = current_region.x2;
		      tmplreg->inarea.y2 = current_region.y2;

		      add_opcode(&splev, SPO_LEVREGION, tmplreg);
		  }
		 ',' lev_region ',' UP_OR_DOWN
		  {
		     lev_region *tmplreg = (lev_region *) 
		       get_last_opcode_data1(&splev, SPO_LEVREGION);

		     if (!tmplreg) yyerror("No lev_region defined?!");
		     tmplreg->del_islev = $6;
		     tmplreg->delarea.x1 = current_region.x1;
		     tmplreg->delarea.y1 = current_region.y1;
		     tmplreg->delarea.x2 = current_region.x2;
		     tmplreg->delarea.y2 = current_region.y2;
		     if($8)
		       tmplreg->rtype = LR_UPSTAIR;
		     else
		       tmplreg->rtype = LR_DOWNSTAIR;
		     tmplreg->rname.str = 0;
		  }
		;

portal_region	: PORTAL_ID ':' lev_region
		  {
		     lev_region *tmplreg = New(lev_region);

		     tmplreg->in_islev = $3;
		     tmplreg->inarea.x1 = current_region.x1;
		     tmplreg->inarea.y1 = current_region.y1;
		     tmplreg->inarea.x2 = current_region.x2;
		     tmplreg->inarea.y2 = current_region.y2;

		     add_opcode(&splev, SPO_LEVREGION, tmplreg);
		  }
		 ',' lev_region ',' string
		  {
		     lev_region *tmplreg = (lev_region *)
		       get_last_opcode_data1(&splev, SPO_LEVREGION);

		     if (!tmplreg) yyerror("No lev_region defined?!");
		     tmplreg->del_islev = $6;
		     tmplreg->delarea.x1 = current_region.x1;
		     tmplreg->delarea.y1 = current_region.y1;
		     tmplreg->delarea.x2 = current_region.x2;
		     tmplreg->delarea.y2 = current_region.y2;
		     tmplreg->rtype = LR_PORTAL;
		     tmplreg->rname.str = $8;
		  }
		;

teleprt_region	: TELEPRT_ID ':' lev_region
		  {
		     lev_region *tmplreg = New(lev_region);

		     tmplreg->in_islev = $3;
		     tmplreg->inarea.x1 = current_region.x1;
		     tmplreg->inarea.y1 = current_region.y1;
		     tmplreg->inarea.x2 = current_region.x2;
		     tmplreg->inarea.y2 = current_region.y2;

		     add_opcode(&splev, SPO_LEVREGION, tmplreg);
		  }
		 ',' lev_region
		  {
		     lev_region *tmplreg = (lev_region *)
		       get_last_opcode_data1(&splev, SPO_LEVREGION);

		     if (!tmplreg) yyerror("No lev_region defined?!");
		     tmplreg->del_islev = $6;
		     tmplreg->delarea.x1 = current_region.x1;
		     tmplreg->delarea.y1 = current_region.y1;
		     tmplreg->delarea.x2 = current_region.x2;
		     tmplreg->delarea.y2 = current_region.y2;
		  }
		teleprt_detail
		  {
		     lev_region *tmplreg = (lev_region *)
		       get_last_opcode_data1(&splev, SPO_LEVREGION);

		     if (!tmplreg) yyerror("No lev_region defined?!");
		     switch($<i>8) {
		      case -1: tmplreg->rtype = LR_TELE; break;
		      case 0: tmplreg->rtype = LR_DOWNTELE; break;
		      case 1: tmplreg->rtype = LR_UPTELE; break;
		     }
		     tmplreg->rname.str = 0;
		  }
		;

branch_region	: BRANCH_ID ':' lev_region
		  {
		     lev_region *tmplreg = New(lev_region);

		     tmplreg->in_islev = $3;
		     tmplreg->inarea.x1 = current_region.x1;
		     tmplreg->inarea.y1 = current_region.y1;
		     tmplreg->inarea.x2 = current_region.x2;
		     tmplreg->inarea.y2 = current_region.y2;

		     add_opcode(&splev, SPO_LEVREGION, tmplreg);
		  }
		 ',' lev_region
		  {
		     lev_region *tmplreg = (lev_region *)
		       get_last_opcode_data1(&splev, SPO_LEVREGION);

		     if (!tmplreg) yyerror("No lev_region defined?!");
		     tmplreg->del_islev = $6;
		     tmplreg->delarea.x1 = current_region.x1;
		     tmplreg->delarea.y1 = current_region.y1;
		     tmplreg->delarea.x2 = current_region.x2;
		     tmplreg->delarea.y2 = current_region.y2;
		     tmplreg->rtype = LR_BRANCH;
		     tmplreg->rname.str = 0;
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

fountain_detail : FOUNTAIN_ID ':' coordinate
		  {
		     fountain *tmpfountain = New(fountain);

		     tmpfountain->x = current_coord.x;
		     tmpfountain->y = current_coord.y;

		     add_opcode(&splev, SPO_FOUNTAIN, tmpfountain);
		  }
		;

sink_detail : SINK_ID ':' coordinate
		  {
		     sink *tmpsink = New(sink);

		     tmpsink->x = current_coord.x;
		     tmpsink->y = current_coord.y;

		     add_opcode(&splev, SPO_SINK, tmpsink);
		  }
		;

pool_detail : POOL_ID ':' coordinate
		  {
		     pool *tmppool = New(pool);

		     tmppool->x = current_coord.x;
		     tmppool->y = current_coord.y;

		     add_opcode(&splev, SPO_POOL, tmppool);
		  }
		;

diggable_detail : NON_DIGGABLE_ID ':' region
		  {
		     digpos *tmpdig = New(digpos);

		     tmpdig->x1 = current_region.x1;
		     tmpdig->y1 = current_region.y1;
		     tmpdig->x2 = current_region.x2;
		     tmpdig->y2 = current_region.y2;

		     add_opcode(&splev, SPO_NON_DIGGABLE, tmpdig);
		  }
		;

passwall_detail : NON_PASSWALL_ID ':' region
		  {
		     digpos *tmppass = New(digpos);

		     tmppass->x1 = current_region.x1;
		     tmppass->y1 = current_region.y1;
		     tmppass->x2 = current_region.x2;
		     tmppass->y2 = current_region.y2;

		     add_opcode(&splev, SPO_NON_PASSWALL, tmppass);
		  }
		;

region_detail	: REGION_ID ':' region ',' light_state ',' room_type prefilled
		  {
		     region *tmpreg = New(region);

		     tmpreg->x1 = current_region.x1;
		     tmpreg->y1 = current_region.y1;
		     tmpreg->x2 = current_region.x2;
		     tmpreg->y2 = current_region.y2;
		     tmpreg->rlit = $<i>5;
		     tmpreg->rtype = $<i>7;
		     if ($<i>8 & 1) tmpreg->rtype += MAXRTYPE+1;
		     tmpreg->rirreg = (($<i>8 & 2) != 0);
		     if(current_region.x1 > current_region.x2 ||
			current_region.y1 > current_region.y2)
		       yyerror("Region start > end!");
		     if(tmpreg->rtype == VAULT &&
			(tmpreg->rirreg ||
			 (tmpreg->x2 - tmpreg->x1 != 1) ||
			 (tmpreg->y2 - tmpreg->y1 != 1)))
		       yyerror("Vaults must be exactly 2x2!");

		     add_opcode(&splev, SPO_REGION, tmpreg);
		  }
		;

altar_detail	: ALTAR_ID ':' coordinate ',' alignment ',' altar_type
		  {
		     altar *tmpaltar = New(altar);

		     tmpaltar->x = current_coord.x;
		     tmpaltar->y = current_coord.y;
		     tmpaltar->align = $<i>5;
		     tmpaltar->shrine = $<i>7;

		     add_opcode(&splev, SPO_ALTAR, tmpaltar);
		  }
		;

gold_detail	: GOLD_ID ':' amount ',' coordinate
		  {
		     gold *tmpgold = New(gold);

		     tmpgold->x = current_coord.x;
		     tmpgold->y = current_coord.y;
		     tmpgold->amount = $<i>3;

		     add_opcode(&splev, SPO_GOLD, tmpgold);
		  }
		;

engraving_detail: ENGRAVING_ID ':' coordinate ',' engraving_type ',' string
		  {
		     engraving *tmpengraving = New(engraving);

		     tmpengraving->x = current_coord.x;
		     tmpengraving->y = current_coord.y;
		     tmpengraving->engr.str = $7;
		     tmpengraving->etype = $<i>5;

		     add_opcode(&splev, SPO_ENGRAVING, tmpengraving);
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
			$$ = 100;	/* default is 100% */
		  }
		| PERCENT
		  {
			if ($1 <= 0 || $1 > 100)
			    yyerror("Expected percentile chance.");
			$$ = $1;
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

region		: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ($2 < 0 || $2 > (int)max_x_map)
				yyerror("Region out of map range!");
			else if ($4 < 0 || $4 > (int)max_y_map)
				yyerror("Region out of map range!");
			else if ($6 < 0 || $6 > (int)max_x_map)
				yyerror("Region out of map range!");
			else if ($8 < 0 || $8 > (int)max_y_map)
				yyerror("Region out of map range!");
			current_region.x1 = $2;
			current_region.y1 = $4;
			current_region.x2 = $6;
			current_region.y2 = $8;
		  }
		;

%%

/*lev_comp.y*/
