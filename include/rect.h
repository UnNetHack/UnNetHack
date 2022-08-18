/*  SCCS Id: @(#)rect.h 3.4 1990/02/22  */
/* Copyright (c) 1990 by Jean-Christophe Collet           */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef RECT_H
#define RECT_H

typedef struct nhrect {
    coordxy lx, ly;
    coordxy hx, hy;
} NhRect;

#endif /* RECT_H */
