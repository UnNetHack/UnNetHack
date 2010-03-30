/*	SCCS Id: @(#)sp_lev.h	3.4	1996/05/08	*/
/* Copyright (c) 1989 by Jean-Christophe Collet			  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SP_LEV_H
#define SP_LEV_H

    /* wall directions */
#define W_NORTH		1
#define W_SOUTH		2
#define W_EAST		4
#define W_WEST		8
#define W_ANY		(W_NORTH|W_SOUTH|W_EAST|W_WEST)

    /* MAP limits */
#define MAP_X_LIM	76
#define MAP_Y_LIM	21

    /* Per level flags */
#define NOTELEPORT	0x00000001L
#define HARDFLOOR	0x00000002L
#define NOMMAP		0x00000004L
#define SHORTSIGHTED	0x00000008L
#define ARBOREAL	0x00000010L
#define NOFLIPX		0x00000020L
#define NOFLIPY		0x00000040L
#define MAZELEVEL	0x00000080L
#define PREMAPPED	0x00000100L
#define SHROUD		0x00000200L
#define STORMY		0x00000400L
#define GRAVEYARD	0x00000800L

/* different level layout initializers */
#define LVLINIT_NONE		0
#define LVLINIT_SOLIDFILL	1
#define LVLINIT_MAZEGRID	2
#define LVLINIT_MINES		3

/* max. layers of object containment */
#define MAX_CONTAINMENT 10

/* max. # of random registers */
#define MAX_REGISTERS	10

/* max. nested depth of subrooms */
#define MAX_NESTED_ROOMS 5

/* max. # of opcodes per special level */
#define SPCODER_MAX_RUNTIME	65536

/* Opcodes for creating the level
 * If you change these, also change opcodestr[] in util/lev_main.c
 */
#define SPO_NULL		0
#define SPO_MESSAGE		1  /* Str_or_Len */
#define SPO_MONSTER		2  /* monster */
#define SPO_OBJECT		3  /* object */
#define SPO_ENGRAVING		4  /* engraving */
#define SPO_ROOM		5  /* room */
#define SPO_SUBROOM		6  /* room */
#define SPO_DOOR		7  /* door */
#define SPO_STAIR		8  /* stair */
#define SPO_LADDER		9  /* lad */
#define SPO_ALTAR		10 /* altar */
#define SPO_FOUNTAIN		11 /* fountain */
#define SPO_SINK		12 /* sink */
#define SPO_POOL		13 /* pool */
#define SPO_TRAP		14 /* trap */
#define SPO_GOLD		15 /* gold */
#define SPO_CORRIDOR		16 /* corridor */
#define SPO_LEVREGION		17 /* lev_region */
#define SPO_RANDOM_OBJECTS	18 /* Str_or_Len */
#define SPO_RANDOM_PLACES	19 /* Str_or_Len*2 (x+1, y+1, x+1, y+1,...) */
#define SPO_RANDOM_MONSTERS	20 /* Str_or_Len */
#define SPO_DRAWBRIDGE		21 /* drawbridge */
#define SPO_MAZEWALK		22 /* walk */
#define SPO_NON_DIGGABLE	23 /* digpos */
#define SPO_NON_PASSWALL	24 /* digpos */
#define SPO_WALLIFY		25
#define SPO_MAP			26 /* mazepart */
#define SPO_ROOM_DOOR		27 /* room_door */
#define SPO_REGION		28 /* region */
#define SPO_CMP			29 /* opcmp    compare */
#define SPO_JMP			30 /* opjmp    jump */
#define SPO_JL			31 /* opjmp    jump if less than */
#define SPO_JG			32 /* opjmp    jump if greater than */
#define SPO_JGE			33
#define SPO_SPILL		34	  /* spill a particular type of terrain into an area */
#define SPO_TERRAIN		35 /* terrain */
#define SPO_REPLACETERRAIN	36 /* replaceterrain */
#define SPO_EXIT		37
#define SPO_ENDROOM		38
#define SPO_RANDLINE		39 /* randline */
#define SPO_POP_CONTAINER	40

#define SPO_PUSH		41
#define SPO_POP			42

#define SPO_RN2			43
#define SPO_DEC			44
#define SPO_COPY		45

#define SPO_JE			46
#define SPO_JNE			47

#define SPO_MON_GENERATION	48
#define SPO_END_MONINVENT	49

#define MAX_SP_OPCODES		50


/* MONSTER and OBJECT can take a variable number of parameters,
 * they also pop different # of values from the stack. So,
 * first we pop a value that tells what the _next_ value will
 * mean.
 */
/* MONSTER */
#define SP_M_V_PEACEFUL         0
#define SP_M_V_ALIGN            1
#define SP_M_V_ASLEEP           2
#define SP_M_V_APPEAR           3
#define SP_M_V_NAME             4

#define SP_M_V_FEMALE		5
#define SP_M_V_INVIS		6
#define SP_M_V_CANCELLED	7
#define SP_M_V_REVIVED		8
#define SP_M_V_AVENGE		9
#define SP_M_V_FLEEING		10
#define SP_M_V_BLINDED		11
#define SP_M_V_PARALYZED	12
#define SP_M_V_STUNNED		13
#define SP_M_V_CONFUSED		14
#define SP_M_V_SEENTRAPS	15

#define SP_M_V_END              16 /* end of variable parameters */

/* OBJECT */
#define SP_O_V_SPE              0
#define SP_O_V_CURSE            1
#define SP_O_V_CORPSENM         2
#define SP_O_V_NAME             3
#define SP_O_V_QUAN		4
#define SP_O_V_BURIED		5
#define SP_O_V_LIT		6
#define SP_O_V_ERODED		7
#define SP_O_V_LOCKED		8
#define SP_O_V_TRAPPED		9
#define SP_O_V_RECHARGED	10
#define SP_O_V_INVIS		11
#define SP_O_V_GREASED		12
#define SP_O_V_BROKEN		13
#define SP_O_V_END              14 /* end of variable parameters */


/* When creating objects, we need to know whether
 * it's a container and/or contents.
 */
#define SP_OBJ_CONTENT		0x1
#define SP_OBJ_CONTAINER	0x2



#define SPOVAR_NULL	0
#define SPOVAR_INT	1 /* l */
#define SPOVAR_STRING	2 /* str */

struct opvar {
    xchar spovartyp; /* one of SPOVAR_foo */
    union {
	char *str;
	long l;
    } vardata;
};

struct splevstack {
    long depth;
    long depth_alloc;
    struct opvar *stackdata;
};


/* special level coder CPU flags */
#define SP_CPUFLAG_LT	1
#define SP_CPUFLAG_GT	2
#define SP_CPUFLAG_EQ	4
#define SP_CPUFLAG_ZERO	8

/*
 * Structures manipulated by the special levels loader & compiler
 */

typedef struct {
    xchar x1,y1,x2,y2;
    xchar fg, lit;
    int roughness;
    xchar thick;
} randline;

typedef struct {
	int cmp_what;
	int cmp_val;
} opcmp;

typedef struct {
	long jmp_target;
} opjmp;

typedef union str_or_len {
	char *str;
	int   len;
} Str_or_Len;

typedef struct {
	xchar   init_style; /* one of LVLINIT_foo */
	char	fg, bg;
	boolean smoothed, joined;
	xchar	lit, walled;
	long	flags;
	schar	filling;
	long	n_opcodes;
} lev_init;

typedef struct {
	xchar x, y, mask;
} door;

typedef struct {
	xchar wall, pos, secret, mask;
} room_door;

typedef struct {
	xchar x, y, type;
} trap;

typedef struct {
	Str_or_Len name, appear_as;
	short id;
	aligntyp align;
	xchar x, y, class, appear;
	schar peaceful, asleep;
        short female, invis, cancelled, revived, avenge, fleeing, blinded, paralyzed, stunned, confused;
        long seentraps;
	short has_invent;
} monster;

typedef struct {
	Str_or_Len name;
	int   corpsenm;
	short id, spe;
	xchar x, y, class, containment;
	schar curse_state;
	int   quan;
	short buried;
	short lit;
        short eroded, locked, trapped, recharged, invis, greased, broken;
} object;

typedef struct {
	xchar		x, y;
	aligntyp	align;
	xchar		shrine;
} altar;

typedef struct {
	xchar x, y, dir, db_open;
} drawbridge;

typedef struct {
	xchar x, y, dir, stocked, typ;
} walk;

typedef struct {
	xchar x1, y1, x2, y2;
} digpos;

typedef struct {
	xchar x, y, up;
} lad;

typedef struct {
	xchar x, y, up;
} stair;

typedef struct {
	xchar x1, y1, x2, y2;
	xchar rtype, rlit, rirreg;
} region;

typedef struct {
    xchar areatyp;
    xchar x1,y1,x2,y2;
    xchar ter, tlit;
} terrain;

typedef struct {
    xchar chance;
    xchar x1,y1,x2,y2;
    xchar fromter, toter, tolit;
} replaceterrain;

/* values for rtype are defined in dungeon.h */
typedef struct {
	struct { xchar x1, y1, x2, y2; } inarea;
	struct { xchar x1, y1, x2, y2; } delarea;
	boolean in_islev, del_islev;
	xchar rtype, padding;
	Str_or_Len rname;
} lev_region;

typedef struct {
	xchar x, y;
	int   amount;
} gold;

typedef struct {
	xchar x, y;
	Str_or_Len engr;
	xchar etype;
} engraving;

typedef struct {
	xchar x, y;
} fountain;

typedef struct {
	xchar x, y;
} sink;

typedef struct {
	xchar x, y;
} pool;

typedef struct _room {
	Str_or_Len name;
	Str_or_Len parent;
	xchar x, y, w, h;
	xchar xalign, yalign;
	xchar rtype, chance, rlit, filled;
} room;

typedef struct {
	schar zaligntyp;
	schar keep_region;
	schar halign, valign;
	char xsize, ysize;
	char **map;
} mazepart;

typedef struct {
	struct {
		xchar room;
		xchar wall;
		xchar door;
	} src, dest;
} corridor;

typedef struct {
	int opcode;
	genericptr_t opdat;
} _opcode;

typedef struct {
	lev_init init_lev;
	_opcode	 *opcodes;
} sp_lev;

typedef struct {
	xchar x, y, direction, count, lit;
	char typ;
} spill;

#endif /* SP_LEV_H */
