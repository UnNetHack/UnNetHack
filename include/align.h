/* Copyright (c) Mike Stephenson, Izchak Miller  1991.        */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ALIGN_H
#define ALIGN_H

typedef schar aligntyp;     /* basic alignment type */

typedef struct  align {     /* alignment & record */
    aligntyp type;
    int record;
} align;

/* bounds for "record" -- respect initial alignments of 10 */
#define ALIGNLIM    (10L + (moves/200L))

#define A_NONE        (-128)    /* the value range of type */

#define A_CHAOTIC   (-1)
#define A_NEUTRAL     0
#define A_LAWFUL      1

#define A_COALIGNED   1
#define A_OPALIGNED (-1)

/* align masks */
#define AM_NONE         0x00
#define AM_CHAOTIC      0x01
#define AM_NEUTRAL      0x02
#define AM_LAWFUL       0x04
#define AM_MASK         0x07 /* mask for "normal" alignment values */

/* Some altars are considered shrines, add a flag for that
   for the altarmask field of struct rm. */
#define AM_SHRINE       0x08
/* High altar on Astral plane or Moloch's sanctum */
#define AM_SANCTUM      0x10

/* special level flags, gone by the time the level has been loaded */
#define AM_SPLEV_CO     0x20 /* co-aligned: force alignment to match hero's  */
#define AM_SPLEV_NONCO  0x40 /* non-co-aligned: force alignment to not match */
#define AM_SPLEV_RANDOM 0x80

#define Amask2align(x) \
    ((aligntyp) ((((x) & AM_MASK) == 0) ? A_NONE :           \
                 (((x) & AM_MASK) == AM_LAWFUL) ? A_LAWFUL : \
           ((int) ((x) & AM_MASK)) - 2)) /* 2 => 0, 1 => -1 */
#define Align2amask(x) \
    ((unsigned) (((x) == A_NONE) ? AM_NONE :     \
                 ((x) == A_LAWFUL) ? AM_LAWFUL : \
                 ((x) + 2))) /* -1 => 1, 0 => 2 */

#endif /* ALIGN_H */
