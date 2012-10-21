/*	SCCS Id: @(#)engrave.h	3.4	1991/07/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ENGRAVE_H
#define ENGRAVE_H

struct engr {
	struct engr *nxt_engr __attribute__ ((aligned (8)));
#if SIZEOF_VOIDP==4
	uint32_t dummy_padding_nxt_engr;
#endif
	char *engr_txt __attribute__ ((aligned (8)));
#if SIZEOF_VOIDP==4
	uint32_t dummy_padding_engr_txt;
#endif
	xchar engr_x, engr_y;
	unsigned engr_lth;	/* for save & restore; not length of text */
	int32_t engr_time;		/* moment engraving was (will be) finished */
	xchar engr_type;
#define DUST	   1
#define ENGRAVE    2
#define BURN	   3
#define MARK	   4
#define ENGR_BLOOD 5
#define HEADSTONE  6
#define N_ENGRAVE  6
};

#define newengr(lth) (struct engr *)alloc((unsigned)(lth) + sizeof(struct engr))
#define dealloc_engr(engr) free((genericptr_t) (engr))

#endif /* ENGRAVE_H */
