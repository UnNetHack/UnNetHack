/* Copyright (c) Mike Threepoint, 1989.               */
/* NetHack may be freely redistributed.  See license for details. */

#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"
#include "skills.h"
#include "color.h"

/* first pass -- object descriptive text */
struct objdescr obj_descr[] = {
#define OBJECTS_DESCR_INIT
#include "objects.h"
#undef OBJECTS_DESCR_INIT
};

#undef OBJ
#undef OBJECT

/* second pass -- object definitions */
struct objclass objects[] = {
#define OBJECTS_INIT
#include "objects.h"
#undef OBJECTS_INIT
};

#undef BITS
#undef COLOR_FIELD
#undef OBJECT
#undef HARDGEM

void objects_init(void);

/* dummy routine used to force linkage */
void
objects_init(void)
{
    return;
}

/*objects.c*/
