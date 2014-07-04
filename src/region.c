/* Copyright (c) 1996 by Jean-Christophe Collet	 */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

/*
 * This should really go into the level structure, but
 * I'll start here for ease. It *WILL* move into the level
 * structure eventually.
 */

static NhRegion **regions;
static int n_regions = 0;
static int max_regions = 0;

#define NO_CALLBACK (-1)

boolean FDECL(inside_gas_cloud, (genericptr,genericptr));
boolean FDECL(expire_gas_cloud, (genericptr,genericptr));
boolean FDECL(revive_cthulhu, (genericptr, genericptr));
boolean FDECL(inside_rect, (NhRect *,int,int));
boolean FDECL(inside_region, (NhRegion *,int,int));
NhRegion *FDECL(create_region, (NhRect *,int));
void FDECL(add_rect_to_reg, (NhRegion *,NhRect *));
void FDECL(add_mon_to_reg, (NhRegion *,struct monst *));
void FDECL(remove_mon_from_reg, (NhRegion *,struct monst *));
boolean FDECL(mon_in_region, (NhRegion *,struct monst *));

void FDECL(free_region, (NhRegion *));
void FDECL(add_region, (NhRegion *));
void FDECL(remove_region, (NhRegion *));

static void FDECL(reset_region_mids, (NhRegion *));

static callback_proc callbacks[] = {
#define INSIDE_GAS_CLOUD 0
    inside_gas_cloud,
#define EXPIRE_GAS_CLOUD 1
    expire_gas_cloud,
#define REVIVE_CTHULHU 2	/* Cthulhu comes back... */
    revive_cthulhu
};

/* Should be inlined. */
boolean
inside_rect(r, x, y)
NhRect *r;
int x, y;
{
    return (x >= r->lx && x <= r->hx && y >= r->ly && y <= r->hy);
}

/*
 * Check if a point is inside a region.
 */
boolean
inside_region(reg, x, y)
NhRegion *reg;
int x, y;
{
    int i;

    if (reg == NULL || !inside_rect(&(reg->bounding_box), x, y))
	return FALSE;
    for (i = 0; i < reg->nrects; i++)
	if (inside_rect(&(reg->rects[i]), x, y))
	    return TRUE;
    return FALSE;
}

/*
 * Create a region. It does not activate it.
 */
NhRegion *
create_region(rects, nrect)
NhRect *rects;
int nrect;
{
    int i;
    NhRegion *reg;

    reg = (NhRegion *) alloc(sizeof (NhRegion));
    /* Determines bounding box */
    if (nrect > 0) {
	reg->bounding_box = rects[0];
    } else {
	reg->bounding_box.lx = 99;
	reg->bounding_box.ly = 99;
	reg->bounding_box.hx = 0;
	reg->bounding_box.hy = 0;
    }
    reg->nrects = nrect;
    reg->rects = nrect > 0 ? (NhRect *)alloc((sizeof (NhRect)) * nrect) : NULL;
    for (i = 0; i < nrect; i++) {
	if (rects[i].lx < reg->bounding_box.lx)
	    reg->bounding_box.lx = rects[i].lx;
	if (rects[i].ly < reg->bounding_box.ly)
	    reg->bounding_box.ly = rects[i].ly;
	if (rects[i].hx > reg->bounding_box.hx)
	    reg->bounding_box.hx = rects[i].hx;
	if (rects[i].hy > reg->bounding_box.hy)
	    reg->bounding_box.hy = rects[i].hy;
	reg->rects[i] = rects[i];
    }
    reg->ttl = -1;		/* Defaults */
    reg->attach_2_u = FALSE;
    reg->attach_2_m = 0;
    /* reg->attach_2_o = NULL; */
    reg->enter_msg = NULL;
    reg->leave_msg = NULL;
    reg->expire_f = NO_CALLBACK;
    reg->enter_f = NO_CALLBACK;
    reg->can_enter_f = NO_CALLBACK;
    reg->leave_f = NO_CALLBACK;
    reg->can_leave_f = NO_CALLBACK;
    reg->inside_f = NO_CALLBACK;
    clear_hero_inside(reg);
    clear_heros_fault(reg);
    reg->n_monst = 0;
    reg->max_monst = 0;
    reg->monsters = NULL;
    reg->arg = NULL;
    return reg;
}

/*
 * Add rectangle to region.
 */
void
add_rect_to_reg(reg, rect)
NhRegion *reg;
NhRect *rect;
{
    NhRect *tmp_rect;

    tmp_rect = (NhRect *) alloc(sizeof (NhRect) * (reg->nrects + 1));
    if (reg->nrects > 0) {
	(void) memcpy((genericptr_t) tmp_rect, (genericptr_t) reg->rects,
		      (sizeof (NhRect) * reg->nrects));
	free((genericptr_t) reg->rects);
    }
    tmp_rect[reg->nrects] = *rect;
    reg->nrects++;
    reg->rects = tmp_rect;
    /* Update bounding box if needed */
    if (reg->bounding_box.lx > rect->lx)
	reg->bounding_box.lx = rect->lx;
    if (reg->bounding_box.ly > rect->ly)
	reg->bounding_box.ly = rect->ly;
    if (reg->bounding_box.hx < rect->hx)
	reg->bounding_box.hx = rect->hx;
    if (reg->bounding_box.hy < rect->hy)
	reg->bounding_box.hy = rect->hy;
}

/*
 * Add a monster to the region
 */
void
add_mon_to_reg(reg, mon)
NhRegion *reg;
struct monst *mon;
{
    int i;
    unsigned *tmp_m;

    if (reg->max_monst <= reg->n_monst) {
	tmp_m = (unsigned *)
		    alloc(sizeof (unsigned) * (reg->max_monst + MONST_INC));
	if (reg->max_monst > 0) {
	    for (i = 0; i < reg->max_monst; i++)
		tmp_m[i] = reg->monsters[i];
	    free((genericptr_t) reg->monsters);
	}
	reg->monsters = tmp_m;
	reg->max_monst += MONST_INC;
    }
    reg->monsters[reg->n_monst++] = mon->m_id;
}

/*
 * Remove a monster from the region list (it left or died...)
 */
void
remove_mon_from_reg(reg, mon)
NhRegion *reg;
struct monst *mon;
{
    register int i;

    for (i = 0; i < reg->n_monst; i++)
	if (reg->monsters[i] == mon->m_id) {
	    reg->n_monst--;
	    reg->monsters[i] = reg->monsters[reg->n_monst];
	    return;
	}
}

/*
 * Check if a monster is inside the region.
 * It's probably quicker to check with the region internal list
 * than to check for coordinates.
 */
boolean
mon_in_region(reg, mon)
NhRegion *reg;
struct monst *mon;
{
    int i;

    for (i = 0; i < reg->n_monst; i++)
	if (reg->monsters[i] == mon->m_id)
	    return TRUE;
    return FALSE;
}

#if 0
/* not yet used */

/*
 * Clone (make a standalone copy) the region.
 */
NhRegion *
clone_region(reg)
NhRegion *reg;
{
    NhRegion *ret_reg;

    ret_reg = create_region(reg->rects, reg->nrects);
    ret_reg->ttl = reg->ttl;
    ret_reg->attach_2_u = reg->attach_2_u;
    ret_reg->attach_2_m = reg->attach_2_m;
 /* ret_reg->attach_2_o = reg->attach_2_o; */
    ret_reg->expire_f = reg->expire_f;
    ret_reg->enter_f = reg->enter_f;
    ret_reg->can_enter_f = reg->can_enter_f;
    ret_reg->leave_f = reg->leave_f;
    ret_reg->can_leave_f = reg->can_leave_f;
    ret_reg->player_flags = reg->player_flags;	/* set/clear_hero_inside,&c*/
    ret_reg->n_monst = reg->n_monst;
    if (reg->n_monst > 0) {
	ret_reg->monsters = (unsigned *)
				alloc((sizeof (unsigned)) * reg->n_monst);
	(void) memcpy((genericptr_t) ret_reg->monsters, (genericptr_t) reg->monsters,
		      sizeof (unsigned) * reg->n_monst);
    } else
	ret_reg->monsters = NULL;
    return ret_reg;
}

#endif	/*0*/

/*
 * Free mem from region.
 */
void
free_region(reg)
NhRegion *reg;
{
    if (reg) {
	if (reg->rects)
	    free((genericptr_t) reg->rects);
	if (reg->monsters)
	    free((genericptr_t) reg->monsters);
	free((genericptr_t) reg);
    }
}

/*
 * Add a region to the list.
 * This actually activates the region.
 */
void
add_region(reg)
NhRegion *reg;
{
    NhRegion **tmp_reg;
    int i, j;

    if (max_regions <= n_regions) {
	tmp_reg = regions;
	regions = (NhRegion **)alloc(sizeof (NhRegion *) * (max_regions + 10));
	if (max_regions > 0) {
	    (void) memcpy((genericptr_t) regions, (genericptr_t) tmp_reg,
			  max_regions * sizeof (NhRegion *));
	    free((genericptr_t) tmp_reg);
	}
	max_regions += 10;
    }
    regions[n_regions] = reg;
    n_regions++;
    /* Check for monsters inside the region */
    for (i = reg->bounding_box.lx; i <= reg->bounding_box.hx; i++)
	for (j = reg->bounding_box.ly; j <= reg->bounding_box.hy; j++) {
	    /* Some regions can cross the level boundaries */
	    if (!isok(i,j))
		continue;
	    if (MON_AT(i, j) && inside_region(reg, i, j))
		add_mon_to_reg(reg, level.monsters[i][j]);
	    if (reg->visible && cansee(i, j))
		newsym(i, j);
	}
    /* Check for player now... */
    if (inside_region(reg, u.ux, u.uy)) 
	set_hero_inside(reg);
    else
	clear_hero_inside(reg);
}

/*
 * Remove a region from the list & free it.
 */
void
remove_region(reg)
NhRegion *reg;
{
    register int i, x, y;

    for (i = 0; i < n_regions; i++)
	if (regions[i] == reg)
	    break;
    if (i == n_regions)
	return;

    /* Update screen if necessary */
    if (reg->visible)
	for (x = reg->bounding_box.lx; x <= reg->bounding_box.hx; x++)
	    for (y = reg->bounding_box.ly; y <= reg->bounding_box.hy; y++)
		if (isok(x,y) && inside_region(reg, x, y) && cansee(x, y))
		    newsym(x, y);

    free_region(reg);
    regions[i] = regions[n_regions - 1];
    regions[n_regions - 1] = (NhRegion *) 0;
    n_regions--;
}

/*
 * Remove all regions and clear all related data (This must be down
 * when changing level, for instance).
 */
void
clear_regions()
{
    register int i;

    for (i = 0; i < n_regions; i++)
	free_region(regions[i]);
    n_regions = 0;
    if (max_regions > 0)
	free((genericptr_t) regions);
    max_regions = 0;
    regions = NULL;
}

/*
 * This function is called every turn.
 * It makes the regions age, if necessary and calls the appropriate
 * callbacks when needed.
 */
void
run_regions()
{
    register int i, j, k;
    int f_indx;

    /* End of life ? */
    /* Do it backward because the array will be modified */
    for (i = n_regions - 1; i >= 0; i--) {
	if (regions[i]->ttl == 0) {
	    if ((f_indx = regions[i]->expire_f) == NO_CALLBACK ||
		(*callbacks[f_indx])(regions[i], (genericptr_t) 0))
		remove_region(regions[i]);
	}
    }

    /* Process remaining regions */
    for (i = 0; i < n_regions; i++) {
	/* Make the region age */
	if (regions[i]->ttl > 0)
	    regions[i]->ttl--;
	/* Check if player is inside region */
	f_indx = regions[i]->inside_f;
	if (f_indx != NO_CALLBACK && hero_inside(regions[i]))
	    (void) (*callbacks[f_indx])(regions[i], (genericptr_t) 0);
	/* Check if any monster is inside region */
	if (f_indx != NO_CALLBACK) {
	    for (j = 0; j < regions[i]->n_monst; j++) {
		struct monst *mtmp = find_mid(regions[i]->monsters[j], FM_FMON);

		if (!mtmp || mtmp->mhp <= 0 ||
				(*callbacks[f_indx])(regions[i], mtmp)) {
		    /* The monster died, remove it from list */
		    k = (regions[i]->n_monst -= 1);
		    regions[i]->monsters[j] = regions[i]->monsters[k];
		    regions[i]->monsters[k] = 0;
		    --j;    /* current slot has been reused; recheck it next */
		}
	    }
	}
    }
}

/*
 * check whether player enters/leaves one or more regions.
 */
boolean
in_out_region(x, y)
xchar
    x, y;
{
    int i, f_indx;

    /* First check if we can do the move */
    for (i = 0; i < n_regions; i++) {
	if (inside_region(regions[i], x, y)
	    && !hero_inside(regions[i]) && !regions[i]->attach_2_u) {
	    if ((f_indx = regions[i]->can_enter_f) != NO_CALLBACK)
		if (!(*callbacks[f_indx])(regions[i], (genericptr_t) 0))
		    return FALSE;
	} else
	    if (hero_inside(regions[i])
		&& !inside_region(regions[i], x, y)
		&& !regions[i]->attach_2_u) {
	    if ((f_indx = regions[i]->can_leave_f) != NO_CALLBACK)
		if (!(*callbacks[f_indx])(regions[i], (genericptr_t) 0))
		    return FALSE;
	}
    }

    /* Callbacks for the regions we do leave */
    for (i = 0; i < n_regions; i++)
	if (hero_inside(regions[i]) &&
		!regions[i]->attach_2_u && !inside_region(regions[i], x, y)) {
	    clear_hero_inside(regions[i]);
	    if (regions[i]->leave_msg != NULL)
		pline("%s", regions[i]->leave_msg);
	    if ((f_indx = regions[i]->leave_f) != NO_CALLBACK)
		(void) (*callbacks[f_indx])(regions[i], (genericptr_t) 0);
	}

    /* Callbacks for the regions we do enter */
    for (i = 0; i < n_regions; i++)
	if (!hero_inside(regions[i]) &&
		!regions[i]->attach_2_u && inside_region(regions[i], x, y)) {
	    set_hero_inside(regions[i]);
	    if (regions[i]->enter_msg != NULL)
		pline("%s", regions[i]->enter_msg);
	    if ((f_indx = regions[i]->enter_f) != NO_CALLBACK)
		(void) (*callbacks[f_indx])(regions[i], (genericptr_t) 0);
	}
    return TRUE;
}

/*
 * check wether a monster enters/leaves one or more region.
*/
boolean
m_in_out_region(mon, x, y)
struct monst *mon;
xchar x, y;
{
    int i, f_indx;

    /* First check if we can do the move */
    for (i = 0; i < n_regions; i++) {
	if (inside_region(regions[i], x, y) &&
		!mon_in_region(regions[i], mon) &&
		regions[i]->attach_2_m != mon->m_id) {
	    if ((f_indx = regions[i]->can_enter_f) != NO_CALLBACK)
		if (!(*callbacks[f_indx])(regions[i], mon))
		    return FALSE;
	} else if (mon_in_region(regions[i], mon) &&
		!inside_region(regions[i], x, y) &&
		regions[i]->attach_2_m != mon->m_id) {
	    if ((f_indx = regions[i]->can_leave_f) != NO_CALLBACK)
		if (!(*callbacks[f_indx])(regions[i], mon))
		    return FALSE;
	}
    }

    /* Callbacks for the regions we do leave */
    for (i = 0; i < n_regions; i++)
	if (mon_in_region(regions[i], mon) &&
		regions[i]->attach_2_m != mon->m_id &&
		!inside_region(regions[i], x, y)) {
	    remove_mon_from_reg(regions[i], mon);
	    if ((f_indx = regions[i]->leave_f) != NO_CALLBACK)
		(void) (*callbacks[f_indx])(regions[i], mon);
	}

    /* Callbacks for the regions we do enter */
    for (i = 0; i < n_regions; i++)
	if (!hero_inside(regions[i]) &&
		!regions[i]->attach_2_u && inside_region(regions[i], x, y)) {
	    add_mon_to_reg(regions[i], mon);
	    if ((f_indx = regions[i]->enter_f) != NO_CALLBACK)
		(void) (*callbacks[f_indx])(regions[i], mon);
	}
    return TRUE;
}

/*
 * Checks player's regions after a teleport for instance.
 */
void
update_player_regions()
{
    register int i;

    for (i = 0; i < n_regions; i++)
	if (!regions[i]->attach_2_u && inside_region(regions[i], u.ux, u.uy))
	    set_hero_inside(regions[i]);
	else
	    clear_hero_inside(regions[i]);
}

/*
 * Ditto for a specified monster.
 */
void
update_monster_region(mon)
struct monst *mon;
{
    register int i;

    for (i = 0; i < n_regions; i++) {
	if (inside_region(regions[i], mon->mx, mon->my)) {
	    if (!mon_in_region(regions[i], mon))
		add_mon_to_reg(regions[i], mon);
	} else {
	    if (mon_in_region(regions[i], mon))
		remove_mon_from_reg(regions[i], mon);
	}
    }
}

#if 0
/* not yet used */

/*
 * Change monster pointer in regions
 * This happens, for instance, when a monster grows and
 * need a new structure (internally that is).
 */
void
replace_mon_regions(monold, monnew)
struct monst *monold, *monnew;
{
    register int i;

    for (i = 0; i < n_regions; i++)
	if (mon_in_region(regions[i], monold)) {
	    remove_mon_from_reg(regions[i], monold);
	    add_mon_to_reg(regions[i], monnew);
	}
}

/*
 * Remove monster from all regions it was in (ie monster just died)
 */
void
remove_mon_from_regions(mon)
struct monst *mon;
{
    register int i;

    for (i = 0; i < n_regions; i++)
	if (mon_in_region(regions[i], mon))
	    remove_mon_from_reg(regions[i], mon);
}

#endif	/*0*/

/*
 * Check if a spot is under a visible region (eg: gas cloud).
 * Returns NULL if not, otherwise returns region.
 */
NhRegion *
visible_region_at(x, y)
xchar x, y;
{
    register int i;

    for (i = 0; i < n_regions; i++)
	if (inside_region(regions[i], x, y) && regions[i]->visible &&
		regions[i]->ttl != 0)
	    return regions[i];
    return (NhRegion *) 0;
}

void
show_region(reg, x, y)
NhRegion *reg;
xchar x, y;
{
    show_glyph(x, y, reg->glyph);
}

/**
 * save_regions :
 */
void
save_regions(fd, mode)
int fd;
int mode;
{
    int i, j;
    unsigned n;

    if (!perform_bwrite(mode)) goto skip_lots;

    bwrite(fd, (genericptr_t) &moves, sizeof (moves));	/* timestamp */
    bwrite(fd, (genericptr_t) &n_regions, sizeof (n_regions));
    for (i = 0; i < n_regions; i++) {
	bwrite(fd, (genericptr_t) &regions[i]->bounding_box, sizeof (NhRect));
	bwrite(fd, (genericptr_t) &regions[i]->nrects, sizeof (short));
	for (j = 0; j < regions[i]->nrects; j++)
	    bwrite(fd, (genericptr_t) &regions[i]->rects[j], sizeof (NhRect));
	bwrite(fd, (genericptr_t) &regions[i]->attach_2_u, sizeof (boolean));
	n = 0;
	bwrite(fd, (genericptr_t) &regions[i]->attach_2_m, sizeof (unsigned));
	n = regions[i]->enter_msg != NULL ? strlen(regions[i]->enter_msg) : 0;
	bwrite(fd, (genericptr_t) &n, sizeof n);
	if (n > 0)
	    bwrite(fd, (genericptr_t) regions[i]->enter_msg, n);
	n = regions[i]->leave_msg != NULL ? strlen(regions[i]->leave_msg) : 0;
	bwrite(fd, (genericptr_t) &n, sizeof n);
	if (n > 0)
	    bwrite(fd, (genericptr_t) regions[i]->leave_msg, n);
	bwrite(fd, (genericptr_t) &regions[i]->ttl, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->expire_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->can_enter_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->enter_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->can_leave_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->leave_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->inside_f, sizeof (short));
	bwrite(fd, (genericptr_t) &regions[i]->player_flags, sizeof (boolean));
	bwrite(fd, (genericptr_t) &regions[i]->n_monst, sizeof (short));
	for (j = 0; j < regions[i]->n_monst; j++)
	    bwrite(fd, (genericptr_t) &regions[i]->monsters[j],
	     sizeof (unsigned));
	bwrite(fd, (genericptr_t) &regions[i]->visible, sizeof (boolean));
	bwrite(fd, (genericptr_t) &regions[i]->glyph, sizeof (int));
	bwrite(fd, (genericptr_t) &regions[i]->arg, sizeof (genericptr_t));
    }

skip_lots:
    if (release_data(mode))
	clear_regions();
}

void
rest_regions(fd, ghostly)
int fd;
boolean ghostly; /* If a bones file restore */
{
    int i, j;
    unsigned n;
    long tmstamp;
    char *msg_buf;

    clear_regions();		/* Just for security */
    mread(fd, (genericptr_t) &tmstamp, sizeof (tmstamp));
    if (ghostly) tmstamp = 0;
    else tmstamp = (moves - tmstamp);
    mread(fd, (genericptr_t) &n_regions, sizeof (n_regions));
    max_regions = n_regions;
    if (n_regions > 0)
	regions = (NhRegion **) alloc(sizeof (NhRegion *) * n_regions);
    for (i = 0; i < n_regions; i++) {
	regions[i] = (NhRegion *) alloc(sizeof (NhRegion));
	mread(fd, (genericptr_t) &regions[i]->bounding_box, sizeof (NhRect));
	mread(fd, (genericptr_t) &regions[i]->nrects, sizeof (short));

	if (regions[i]->nrects > 0)
	    regions[i]->rects = (NhRect *)
				  alloc(sizeof (NhRect) * regions[i]->nrects);
	for (j = 0; j < regions[i]->nrects; j++)
	    mread(fd, (genericptr_t) &regions[i]->rects[j], sizeof (NhRect));
	mread(fd, (genericptr_t) &regions[i]->attach_2_u, sizeof (boolean));
	mread(fd, (genericptr_t) &regions[i]->attach_2_m, sizeof (unsigned));

	mread(fd, (genericptr_t) &n, sizeof n);
	if (n > 0) {
	    msg_buf = (char *) alloc(n + 1);
	    mread(fd, (genericptr_t) msg_buf, n);
	    msg_buf[n] = '\0';
	    regions[i]->enter_msg = (const char *) msg_buf;
	} else
	    regions[i]->enter_msg = NULL;

	mread(fd, (genericptr_t) &n, sizeof n);
	if (n > 0) {
	    msg_buf = (char *) alloc(n + 1);
	    mread(fd, (genericptr_t) msg_buf, n);
	    msg_buf[n] = '\0';
	    regions[i]->leave_msg = (const char *) msg_buf;
	} else
	    regions[i]->leave_msg = NULL;

	mread(fd, (genericptr_t) &regions[i]->ttl, sizeof (short));
	/* check for expired region */
	if (regions[i]->ttl >= 0)
	    regions[i]->ttl =
		(regions[i]->ttl > tmstamp) ? regions[i]->ttl - tmstamp : 0;
	mread(fd, (genericptr_t) &regions[i]->expire_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->can_enter_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->enter_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->can_leave_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->leave_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->inside_f, sizeof (short));
	mread(fd, (genericptr_t) &regions[i]->player_flags, sizeof (boolean));
	if (ghostly) {	/* settings pertained to old player */
	    clear_hero_inside(regions[i]);
	    clear_heros_fault(regions[i]);
	}
	mread(fd, (genericptr_t) &regions[i]->n_monst, sizeof (short));
	if (regions[i]->n_monst > 0)
	    regions[i]->monsters =
		(unsigned *) alloc(sizeof (unsigned) * regions[i]->n_monst);
	else
	    regions[i]->monsters = NULL;
	regions[i]->max_monst = regions[i]->n_monst;
	for (j = 0; j < regions[i]->n_monst; j++)
	    mread(fd, (genericptr_t) &regions[i]->monsters[j],
		  sizeof (unsigned));
	mread(fd, (genericptr_t) &regions[i]->visible, sizeof (boolean));
	mread(fd, (genericptr_t) &regions[i]->glyph, sizeof (int));
	mread(fd, (genericptr_t) &regions[i]->arg, sizeof (genericptr_t));
    }
    /* remove expired regions, do not trigger the expire_f callback (yet!);
       also update monster lists if this data is coming from a bones file */
    for (i = n_regions - 1; i >= 0; i--)
	if (regions[i]->ttl == 0)
	    remove_region(regions[i]);
	else if (ghostly && regions[i]->n_monst > 0)
	    reset_region_mids(regions[i]);
}

/* update monster IDs for region being loaded from bones; `ghostly' implied */
static void
reset_region_mids(reg)
NhRegion *reg;
{
    int i = 0, n = reg->n_monst;
    unsigned *mid_list = reg->monsters;

    while (i < n)
	if (!lookup_id_mapping(mid_list[i], &mid_list[i])) {
	    /* shrink list to remove missing monster; order doesn't matter */
	    mid_list[i] = mid_list[--n];
	} else {
	    /* move on to next monster */
	    ++i;
	}
    reg->n_monst = n;
    return;
}

#if 0
/* not yet used */

/*--------------------------------------------------------------*
 *								*
 *			Create Region with just a message	*
 *								*
 *--------------------------------------------------------------*/

NhRegion *
create_msg_region(x, y, w, h, msg_enter, msg_leave)
xchar x, y;
xchar w, h;
const char *msg_enter;
const char *msg_leave;
{
    NhRect tmprect;
    NhRegion *reg = create_region((NhRect *) 0, 0);

    reg->enter_msg = msg_enter;
    reg->leave_msg = msg_leave;
    tmprect.lx = x;
    tmprect.ly = y;
    tmprect.hx = x + w;
    tmprect.hy = y + h;
    add_rect_to_reg(reg, &tmprect);
    reg->ttl = -1;
    return reg;
}


/*--------------------------------------------------------------*
 *								*
 *			Force Field Related Code		*
 *			(unused yet)				*
 *--------------------------------------------------------------*/

boolean
enter_force_field(p1, p2)
genericptr_t p1;
genericptr_t p2;
{
    struct monst *mtmp;

    if (p2 == NULL) {		/* That means the player */
	if (!Blind)
		You("bump into %s. Ouch!",
		    Hallucination ? "an invisible tree" :
			"some kind of invisible wall");
	else
	    pline("Ouch!");
    } else {
	mtmp = (struct monst *) p2;
	if (canseemon(mtmp))
	    pline("%s bumps into %s!", Monnam(mtmp), something);
    }
    return FALSE;
}

NhRegion *
create_force_field(x, y, radius, ttl)
xchar x, y;
int radius, ttl;
{
    int i;
    NhRegion *ff;
    int nrect;
    NhRect tmprect;

    ff = create_region((NhRect *) 0, 0);
    nrect = radius;
    tmprect.lx = x;
    tmprect.hx = x;
    tmprect.ly = y - (radius - 1);
    tmprect.hy = y + (radius - 1);
    for (i = 0; i < nrect; i++) {
	add_rect_to_reg(ff, &tmprect);
	tmprect.lx--;
	tmprect.hx++;
	tmprect.ly++;
	tmprect.hy--;
    }
    ff->ttl = ttl;
    if (!in_mklev && !flags.mon_moving)
	set_heros_fault(ff);		/* assume player has created it */
 /* ff->can_enter_f = enter_force_field; */
 /* ff->can_leave_f = enter_force_field; */
    add_region(ff);
    return ff;
}

#endif	/*0*/

/*--------------------------------------------------------------*
 *								*
 *			Gas cloud related code			*
 *								*
 *--------------------------------------------------------------*/

/*
 * Here is an example of an expire function that may prolong
 * region life after some mods...
 */
boolean
expire_gas_cloud(p1, p2)
genericptr_t p1;
genericptr_t p2;
{
    NhRegion *reg;
    size_t damage;

    reg = (NhRegion *) p1;
    damage = (size_t) reg->arg;

    /* If it was a thick cloud, it dissipates a little first */
    if (damage >= 5) {
	damage /= 2;		/* It dissipates, let's do less damage */
	reg->arg = (genericptr_t) damage;
	reg->ttl = 2;		/* Here's the trick : reset ttl */
	return FALSE;		/* THEN return FALSE, means "still there" */
    }
    return TRUE;		/* OK, it's gone, you can free it! */
}

boolean
revive_cthulhu(p1, p2)
genericptr_t p1;
genericptr_t p2;
{
    boolean ret = expire_gas_cloud(p1, p2);
    if (ret) {
	/* Bring back Cthulhu! */
	int cx, cy;
	NhRegion *reg = (NhRegion *) p1;
	struct monst *cthulhu = NULL;
	coord cc;
	
	cx = (reg->bounding_box.lx + reg->bounding_box.hx) / 2;
	cy = (reg->bounding_box.ly + reg->bounding_box.hy) / 2;

	if (enexto(&cc, cx, cy, &mons[PM_CTHULHU])) {
	    cx = cc.x;
	    cy = cc.y;
	} else {
	    cx = cy = 0;	/* Place Cthulhu randomly */
	}

	/* Make sure Cthulhu doesn't get the Amulet again! :-) */
	cthulhu = makemon(&mons[PM_CTHULHU], cx, cy, 
				MM_NOCOUNTBIRTH | NO_MINVENT);
	if (cthulhu) {
		if (canseemon(cthulhu)) {
			pline("%s reforms!", Monnam(cthulhu));
		}
		/* don't let Cthulhu meditate after being killed once
		 * by the player */
		wakeup(cthulhu);
	}
	
    }
    return ret;
}

boolean
inside_gas_cloud(p1, p2)
genericptr_t p1;
genericptr_t p2;
{
    NhRegion *reg;
    struct monst *mtmp;
    int dam;

    reg = (NhRegion *) p1;
    dam = (size_t) reg->arg;
    if (p2 == NULL) {		/* This means *YOU* Bozo! */
	u.incloud = TRUE;
	if (nonliving(youmonst.data) || Breathless)
	    return FALSE;
	if (!Blind)
	    make_blinded(1L, FALSE);
	if (u.uinvulnerable) return FALSE;
	if (!Poison_resistance) {
	    pline("%s is burning your %s!", Something, makeplural(body_part(LUNG)));
	    You("cough and spit blood!");
	    losehp(rnd(dam) + 5, "gas cloud", KILLED_BY_AN);
	    return FALSE;
	} else {
	    /* You("cough!"); */
	    return FALSE;
	}
    } else {			/* A monster is inside the cloud */
	mtmp = (struct monst *) p2;

	/* Non living, non breathing, aquatic, and
	   poison-resistant monsters are not concerned */
	if (!nonliving(mtmp->data) && !breathless(mtmp->data) &&
		!amphibious(mtmp->data) && !resists_poison(mtmp)) {
	    if (cansee(mtmp->mx, mtmp->my))
		pline("%s coughs!", Monnam(mtmp));
	    if (heros_fault(reg)) setmangry(mtmp);
	    if (haseyes(mtmp->data) && mtmp->mcansee) {
		mtmp->mblinded = 1;
		mtmp->mcansee = 0;
	    }
	    if (resists_poison(mtmp))
		return FALSE;
	    mtmp->mhp -= rnd(dam) + 5;
	    if (mtmp->mhp <= 0) {
		if (heros_fault(reg))
		    killed(mtmp);
		else
		    monkilled(mtmp, "gas cloud", AD_DRST);
		if (mtmp->mhp <= 0) {	/* not lifesaved */
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;		/* Monster is still alive */
}

NhRegion *
create_cthulhu_death_cloud(x, y, radius, damage, duration)
xchar x, y;
int radius;
size_t damage;
int duration;
{
    NhRegion *cloud;

    cloud = create_gas_cloud(x, y, radius, damage, duration);
    if (cloud) cloud->expire_f = REVIVE_CTHULHU;

    return cloud;
}

NhRegion *
create_gas_cloud(x, y, radius, damage, duration)
xchar x, y;
int radius;
size_t damage;
int duration;
{
    NhRegion *cloud;
    int i, nrect;
    NhRect tmprect;

    cloud = create_region((NhRect *) 0, 0);
    nrect = radius;
    tmprect.lx = x;
    tmprect.hx = x;
    tmprect.ly = y - (radius - 1);
    tmprect.hy = y + (radius - 1);
    for (i = 0; i < nrect; i++) {
	add_rect_to_reg(cloud, &tmprect);
	tmprect.lx--;
	tmprect.hx++;
	tmprect.ly++;
	tmprect.hy--;
    }
    cloud->ttl = duration;
    if (!in_mklev && !flags.mon_moving)
	set_heros_fault(cloud);		/* assume player has created it */
    cloud->inside_f = INSIDE_GAS_CLOUD;
    cloud->expire_f = EXPIRE_GAS_CLOUD;
    cloud->arg = (genericptr_t) damage;
    cloud->visible = TRUE;
    cloud->glyph = cmap_to_glyph(S_cloud);
    add_region(cloud);
    return cloud;
}

/*region.c*/
