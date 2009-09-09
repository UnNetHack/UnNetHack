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
#define NOTELEPORT	1
#define HARDFLOOR	2
#define NOMMAP		4
#define SHORTSIGHTED	8
#define ARBOREAL	16
#define NOFLIPX		32
#define NOFLIPY		64

    /* special level types */
#define SP_LEV_ROOMS	1
#define SP_LEV_MAZE	2

/* max. # of random registers */
#define MAX_REGISTERS	10

/* max. # of opcodes per special level */
#define SPCODER_MAX_RUNTIME	65536

/* Opcodes for creating the level */
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
#define SPO_SPILL			  33	  /* spill a particular type of terrain into an area */
#define SPO_TERRAIN		34 /* terrain */
#define SPO_REPLACETERRAIN	35 /* replaceterrain */
#define SPO_EXIT		36
#define MAX_SP_OPCODES		37


/* special level coder CPU flags */
#define SP_CPUFLAG_LT	1
#define SP_CPUFLAG_GT	2
#define SP_CPUFLAG_EQ	4
#define SP_CPUFLAG_ZERO	8

/*
 * Structures manipulated by the special levels loader & compiler
 */

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
	boolean init_present, padding;
	char	fg, bg;
	boolean smoothed, joined;
	xchar	lit, walled;
	long	flags;
	char	levtyp;	/* SP_LEV_xxx */
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
	xchar x, y, chance, type;
} trap;

typedef struct {
	Str_or_Len name, appear_as;
	short id;
	aligntyp align;
	xchar x, y, chance, class, appear;
	schar peaceful, asleep;
} monster;

typedef struct {
	Str_or_Len name;
	int   corpsenm;
	short id, spe;
	xchar x, y, chance, class, containment;
	schar curse_state;
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
	xchar x, y, dir;
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
    xchar chance;
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
