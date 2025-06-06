/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static void m_lose_armor(struct monst *, struct obj *, boolean) NONNULLPTRS;
static void clear_bypass(struct obj *) NO_NNARGS;
static void m_dowear_type(struct monst *, long, boolean, boolean) NONNULLARG1;
static int extra_pref(struct monst *, struct obj *) NONNULLARG1;

static const struct worn {
    long w_mask;
    struct obj **w_obj;
    const char *w_what; /* for failing sanity check's feedback */
} worn[] = {
    { W_ARM, &uarm, "suit" },
    { W_ARMC, &uarmc, "cloak" },
    { W_ARMH, &uarmh, "helmet" },
    { W_ARMS, &uarms, "shield" },
    { W_ARMG, &uarmg, "gloves" },
    { W_ARMF, &uarmf, "boots" },
    { W_ARMU, &uarmu, "shirt" },
    { W_RINGL, &uleft, "left ring" },
    { W_RINGR, &uright, "right ring" },
    { W_WEP, &uwep, "weapon" },
    { W_SWAPWEP, &uswapwep, "alternate weapon" },
    { W_QUIVER, &uquiver, "quiver" },
    { W_AMUL, &uamul, "amulet" },
    { W_TOOL, &ublindf, "facewear" }, /* blindfold|towel|lenses */
    { W_BALL, &uball, "chained ball" },
    { W_CHAIN, &uchain, "attached chain" },
    { 0, 0, (char *) 0 }
};

/* This only allows for one blocking item per property */
#define w_blocks(o, m) \
    ((o->otyp == MUMMY_WRAPPING && ((m) & W_ARMC)) ? INVIS : \
     (o->otyp == TINFOIL_HAT && ((m) & W_ARMH)) ? TELEPAT : \
     (o->otyp == CORNUTHAUM && ((m) & W_ARMH) && \
      !Role_if(PM_WIZARD)) ? CLAIRVOYANT : \
     (is_art(o, ART_EYES_OF_THE_OVERWORLD) && \
      ((m) & W_TOOL) != 0L) ? BLINDED : \
     0)
/* note: monsters don't have clairvoyance, so dependency on hero's role here
   has no significant effect on their use of w_blocks() */

/* calc the range of hero's unblind telepathy */
void
recalc_telepat_range(void)
{
    const struct worn *wp;
    int nobjs = 0;

    for (wp = worn; wp->w_mask; wp++) {
        struct obj *oobj = *(wp->w_obj);

        if (oobj && objects[oobj->otyp].oc_oprop == TELEPAT) {
            nobjs++;
        }
    }
    /* count all artifacts with SPFX_ESP as one */
    if (ETelepat & W_ART) {
        nobjs++;
    }

    if (nobjs) {
        u.unblind_telepat_range = (BOLT_LIM * BOLT_LIM) * nobjs;
    } else {
        u.unblind_telepat_range = -1;
    }
}

/* Updated to use the extrinsic and blocked fields. */
void
setworn(struct obj *obj, long mask)
{
    const struct worn *wp;
    struct obj *oobj;
    int p;

    if ((mask & (W_ARM | I_SPECIAL)) == (W_ARM | I_SPECIAL)) {
        /* restoring saved game; no properties are conferred via skin */
        uskin = obj;
        /* assert( !uarm ); */
    } else {
        for (wp = worn; wp->w_mask; wp++) {
            if (wp->w_mask & mask) {
                oobj = *(wp->w_obj);
                if (oobj && !(oobj->owornmask & wp->w_mask)) {
                    warning("Setworn: mask=0x%08lx.", wp->w_mask);
                }
                if (oobj) {
                    if (u.twoweap && (oobj->owornmask & (W_WEP | W_SWAPWEP))) {
                        set_twoweap(FALSE); /* u.twoweap = FALSE */
                    }
                    oobj->owornmask &= ~wp->w_mask;
                    if (wp->w_mask & ~(W_SWAPWEP | W_QUIVER)) {
                        /* leave as "x = x <op> y", here and below, for broken
                         * compilers */
                        p = objects[oobj->otyp].oc_oprop;
                        u.uprops[p].extrinsic =
                            u.uprops[p].extrinsic & ~wp->w_mask;
                        /* if the hero removed an extrinsic-granting item,
                           nearby monsters will notice and attempt attacks of
                           that type again */
                        monstunseesu_prop(p);
                        if ((p = w_blocks(oobj, mask)) != 0) {
                            u.uprops[p].blocked &= ~wp->w_mask;
                            /* HACK: telepathy-blocking items also block clairvoyance */
                            if (p == TELEPAT) {
                                u.uprops[CLAIRVOYANT].blocked &= ~wp->w_mask;
                            }
                        }
                        if (oobj->oartifact) {
                            set_artifact_intrinsic(oobj, 0, mask);
                        }
                    }
                    /* in case wearing or removal is in progress or removal
                       is pending (via 'A' command for multiple items) */
                    cancel_doff(oobj, wp->w_mask);
                }
                *(wp->w_obj) = obj;
                if (obj) {
                    obj->owornmask |= wp->w_mask;
                    /* Prevent getting/blocking intrinsics from wielding
                     * potions, through the quiver, etc.
                     * Allow weapon-tools, too.
                     * wp_mask should be same as mask at this point.
                     */
                    if (wp->w_mask & ~(W_SWAPWEP | W_QUIVER)) {
                        if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
                            mask != W_WEP) {
                            p = objects[obj->otyp].oc_oprop;
                            u.uprops[p].extrinsic =
                                u.uprops[p].extrinsic | wp->w_mask;
                            if ((p = w_blocks(obj, mask)) != 0) {
                                u.uprops[p].blocked |= wp->w_mask;
                                /* HACK: telepathy-blocking items also block clairvoyance */
                                if (p == TELEPAT) {
                                    u.uprops[CLAIRVOYANT].blocked |= wp->w_mask;
                                }
                            }
                        }
                        if (obj->oartifact) {
                            set_artifact_intrinsic(obj, 1, mask);
                        }
                    }
                }
            }
        }
        if (obj && (obj->owornmask & W_ARMOR) != 0L) {
            violated(CONDUCT_NUDISM);
            /* Restoring a game and naming worn armor uses setworn.  *
             * This can unneccessariely increase the conduct-counter *
             * (only visible in Wizmode)                             */
        }
    }
    update_inventory();
    recalc_telepat_range();
}

/* called e.g. when obj is destroyed */
/* Updated to use the extrinsic and blocked fields. */
void
setnotworn(struct obj *obj)
{
    const struct worn *wp;
    int p;

    if (!obj) {
        return;
    }
    if (u.twoweap && (obj == uwep || obj == uswapwep)) {
        set_twoweap(FALSE); /* u.twoweap = FALSE */
    }
    for (wp = worn; wp->w_mask; wp++) {
        if (obj == *(wp->w_obj)) {
            /* in case wearing or removal is in progress or removal
               is pending (via 'A' command for multiple items) */
            cancel_doff(obj, wp->w_mask);

            *(wp->w_obj) = (struct obj *) 0;
            p = objects[obj->otyp].oc_oprop;
            u.uprops[p].extrinsic = u.uprops[p].extrinsic & ~wp->w_mask;
            monstunseesu_prop(p); /* remove this extrinsic from seenres */
            obj->owornmask &= ~wp->w_mask;
            if (obj->oartifact) {
                set_artifact_intrinsic(obj, 0, wp->w_mask);
            }
            if ((p = w_blocks(obj, wp->w_mask)) != 0) {
                u.uprops[p].blocked &= ~wp->w_mask;
                /* HACK: telepathy-blocking items also block clairvoyance */
                if (p == TELEPAT) {
                    u.uprops[CLAIRVOYANT].blocked &= ~wp->w_mask;
                }
            }
        }
    }
    update_inventory();
    recalc_telepat_range();
}

/* called when saving with FREEING flag set has just discarded inventory */
void
allunworn(void)
{
    const struct worn *wp;

    u.twoweap = 0; /* uwep and uswapwep are going away */
    /* remove stale pointers; called after the objects have been freed
       (without first being unworn) while saving invent during game save;
       note: uball and uchain might not be freed yet but we clear them
       here anyway (savegamestate() and its callers deal with them) */
    for (wp = worn; wp->w_mask; wp++) {
        /* object is already gone so we don't/can't update is owornmask */
        *(wp->w_obj) = (struct obj *) 0;
    }
}

/* return item worn in slot indicated by wornmask; needed by poly_obj() */
struct obj *
wearmask_to_obj(long wornmask)
{
    const struct worn *wp;

    for (wp = worn; wp->w_mask; wp++) {
        if (wp->w_mask & wornmask) {
            return *wp->w_obj;
        }
    }
    return (struct obj *) 0;
}

/* return a bitmask of the equipment slot(s) a given item might be worn in */
long
wearslot(struct obj *obj)
{
    int otyp = obj->otyp;
    /* practically any item can be wielded or quivered; it's up to
       our caller to handle such things--we assume "normal" usage */
    long res = 0L; /* default: can't be worn anywhere */

    switch (obj->oclass) {
    case AMULET_CLASS:
        res = W_AMUL; /* WORN_AMUL */
        break;
    case RING_CLASS:
        res = W_RINGL | W_RINGR; /* W_RING, BOTH_SIDES */
        break;
    case ARMOR_CLASS:
        switch (objects[otyp].oc_armcat) {
        case ARM_SUIT:
            res = W_ARM;
            break; /* WORN_ARMOR */
        case ARM_SHIELD:
            res = W_ARMS;
            break; /* WORN_SHIELD */
        case ARM_HELM:
            res = W_ARMH;
            break; /* WORN_HELMET */
        case ARM_GLOVES:
            res = W_ARMG;
            break; /* WORN_GLOVES */
        case ARM_BOOTS:
            res = W_ARMF;
            break; /* WORN_BOOTS */
        case ARM_CLOAK:
            res = W_ARMC;
            break; /* WORN_CLOAK */
        case ARM_SHIRT:
            res = W_ARMU;
            break; /* WORN_SHIRT */
        }
        break;
    case WEAPON_CLASS:
        res = W_WEP | W_SWAPWEP;
        if (objects[otyp].oc_merge) {
            res |= W_QUIVER;
        }
        break;
    case TOOL_CLASS:
        if (otyp == BLINDFOLD || otyp == TOWEL || otyp == LENSES) {
            res = W_TOOL; /* WORN_BLINDF */
        } else if (is_weptool(obj) || otyp == TIN_OPENER) {
            res = W_WEP | W_SWAPWEP;
        } else if (otyp == SADDLE) {
            res = W_SADDLE;
        }
        break;
    case FOOD_CLASS:
        if (obj->otyp == MEAT_RING) {
            res = W_RINGL | W_RINGR;
        }
        break;
    case GEM_CLASS:
        res = W_QUIVER;
        break;
    case BALL_CLASS:
        res = W_BALL;
        break;
    case CHAIN_CLASS:
        res = W_CHAIN;
        break;
    default:
        break;
    }
    return res;
}

/* for 'sanity_check' option, called by you_sanity_check() */
void
check_wornmask_slots(void)
{
    /* we'll skip ball and chain here--they warrant separate sanity check */
#define IGNORE_SLOTS (W_ART | W_ARTI | W_SADDLE | W_BALL| W_CHAIN)
    char whybuf[BUFSZ];
    const struct worn *wp;
    struct obj *o, *otmp;
    long m;

    for (wp = worn; wp->w_mask; wp++) {
        m = wp->w_mask;
        if ((m & IGNORE_SLOTS) != 0L && (m & ~IGNORE_SLOTS) == 0L) {
            continue;
        }
        if ((o = *wp->w_obj) != 0) {
            whybuf[0] = '\0';
            /* slot pointer (uarm, uwep, &c) is populated; check that object
               is in inventory and has the relevant owornmask bit set */
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                if (otmp == o) {
                    break;
                }
            }
            if (!otmp) {
                Sprintf(whybuf, "%s (%s) not found in invent",
                        wp->w_what, fmt_ptr(o));
            } else if ((o->owornmask & m) == 0L) {
                Sprintf(whybuf, "%s bit not set in owornmask [0x%08lx]",
                        wp->w_what, o->owornmask);
            } else if ((o->owornmask & ~(m | IGNORE_SLOTS)) != 0L) {
                Sprintf(whybuf, "%s wrong bit set in owornmask [0x%08lx]",
                        wp->w_what, o->owornmask);
            }
            if (whybuf[0]) {
                warning("Worn-slot insanity: %s.", whybuf);
            }
        } /* o != NULL */

        /* check whether any item other than the one in the slot pointer
           claims to be worn/wielded in this slot; make this test whether
           'o' is Null or not; [sanity_check_worn(mkobj.c) for object by
           object checking will most likely have already caught this] */
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp != o && (otmp->owornmask & m) != 0L &&
                /* embedded scales owornmask is W_ARM|I_SPECIAL so would
                   give a false complaint about item other than uarm having
                   W_ARM bit set if we didn't screen it out here */
                 (m != W_ARM || otmp != uskin ||
                  (otmp->owornmask & I_SPECIAL) == 0L)) {
                Sprintf(whybuf, "%s [0x%08lx] has %s mask 0x%08lx bit set",
                        simpleonames(otmp), otmp->owornmask, wp->w_what, m);
                impossible("Worn-slot insanity: %s.", whybuf);
            }
        }
    } /* for wp in worn[] */

#ifdef EXTRA_SANITY_CHECKS
    if (uskin) {
        const char *what = "embedded scales";

        o = uskin;
        m = W_ARM | I_SPECIAL;
        whybuf[0] = '\0';
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp == o) {
                break;
            }
        }
        if (!otmp) {
            Sprintf(whybuf, "%s (%s) not found in invent",
                    what, fmt_ptr(o));
        } else if ((o->owornmask & m) != m) {
            Sprintf(whybuf, "%s bits not set in owornmask [0x%08lx]",
                    what, o->owornmask);
        } else if ((o->owornmask & ~(m | IGNORE_SLOTS)) != 0L) {
            Sprintf(whybuf, "%s wrong bit set in owornmask [0x%08lx]",
                    what, o->owornmask);
        } else if (!Is_dragon_scales(o)) {
            Sprintf(whybuf, "%s (%s) %s not dragon scales",
                    what, simpleonames(o), otense(o, "are"));
        } else if (Dragon_scales_to_pm(o) != &mons[u.umonnum]) {
            Sprintf(whybuf, "%s, hero is not %s",
                    what, an(mons[u.umonnum].pmnames[NEUTRAL]));
        }
        if (whybuf[0]) {
            impossible("Worn-slot insanity: %s.", whybuf);
        }
    } /* uskin */
#endif /* EXTRA_SANITY_CHECKS */

#ifdef EXTRA_SANITY_CHECKS
    /* dual wielding: not a slot but lots of things to verify */
    if (u.twoweap) {
        const char *why = NULL;

        if (!uwep || !uswapwep) {
            Sprintf(whybuf, "without %s%s%s",
                    !uwep ? "uwep" : "",
                    (!uwep && !uswapwep) ? " and without " : "",
                    !uswapwep ? "uswapwep" : "");
            why = whybuf;
        } else if (uarms) {
            why = "while wearing shield";
        } else if (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep)) {
            why = "uwep is not a weapon";
        } else if (is_launcher(uwep) || is_ammo(uwep) || is_missile(uwep)) {
            why = "uwep is not a melee weapon";
        } else if (bimanual(uwep)) {
            why = "uwep is two-handed";
        } else if (uswapwep->oclass != WEAPON_CLASS && !is_weptool(uswapwep)) {
            why = "uswapwep is not a weapon";
        } else if (is_launcher(uswapwep) || is_ammo(uswapwep) || is_missile(uswapwep)) {
            why = "uswapwep is not a melee weapon";
        } else if (bimanual(uswapwep)) {
            why = "uswapwep is two-handed";
        } else if (!could_twoweap(youmonst.data)) {
            why = "without two weapon attacks";
        }

        if (why) {
            impossible("Two-weapon insanity: %s.", why);
        }
    }
#endif /* EXTRA_SANITY_CHECKS */
    return;
#undef IGNORE_SLOTS
} /* check_wornmask_slots() */

void
mon_set_minvis(struct monst *mon)
{
    mon->perminvis = 1;
    if (!mon->invis_blkd) {
        mon->minvis = 1;
        if (mon->data == &mons[PM_GIANT_TURTLE]) {
            unblock_point(mon->mx, mon->my);
        }
        newsym(mon->mx, mon->my);       /* make it disappear */
        if (mon->wormno) {
            see_wsegs(mon); /* and any tail too */
        }
    }
}

void
mon_adjust_speed(struct monst *mon,
                 int adjust,      /**< positive => increase speed, negative => decrease */
                 struct obj *obj) /**< item to make known if effect can be seen */
{
    struct obj *otmp;
    boolean give_msg = !in_mklev, petrify = FALSE;
    unsigned int oldspeed = mon->mspeed;

    switch (adjust) {
    case  2:
        mon->permspeed = MFAST;
        give_msg = FALSE; /* special case monster creation */
        break;
    case  1:
        if (mon->permspeed == MSLOW) {
            mon->permspeed = 0;
        } else {
            mon->permspeed = MFAST;
        }
        break;
    case  0:            /* just check for worn speed boots */
        break;
    case -1:
        if (mon->permspeed == MFAST) {
            mon->permspeed = 0;
        } else {
            mon->permspeed = MSLOW;
        }
        break;
    case -2:
        mon->permspeed = MSLOW;
        give_msg = FALSE; /* (not currently used) */
        break;
    case -3:            /* petrification */
        /* take away intrinsic speed but don't reduce normal speed */
        if (mon->permspeed == MFAST) {
            mon->permspeed = 0;
        }
        petrify = TRUE;
        break;
    case -4: /* green slime */
        if (mon->permspeed == MFAST) {
            mon->permspeed = 0;
        }
        give_msg = FALSE;
        break;
    }

    for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
        if (otmp->owornmask && objects[otmp->otyp].oc_oprop == FAST) {
            break;
        }
    }

    if (otmp) { /* speed boots */
        mon->mspeed = MFAST;
    } else {
        mon->mspeed = mon->permspeed;
    }

    /* no message if monster is immobile (temp or perm) or unseen */
    if (give_msg && (mon->mspeed != oldspeed || petrify) &&
        mon->data->mmove && !(mon->mfrozen || mon->msleeping) && canseemon(mon)) {
        /* fast to slow (skipping intermediate state) or vice versa */
        const char *howmuch = (mon->mspeed + oldspeed == MFAST + MSLOW) ?
                              "much " : "";

        if (petrify) {
            /* mimic the player's petrification countdown; "slowing down"
               even if fast movement rate retained via worn speed boots */
            if (flags.verbose) {
                pline_mon(mon, "%s is slowing down.", Monnam(mon));
            }
        } else if (adjust > 0 || mon->mspeed == MFAST) {
            if (is_weeping(mon->data)) {
                pline_mon(mon, "%s is suddenly changing positions %sfaster.", Monnam(mon), howmuch);
            } else {
                pline_mon(mon, "%s is suddenly moving %sfaster.", Monnam(mon), howmuch);
            }
        } else if (is_weeping(mon->data)) {
            pline_mon(mon, "%s is suddenly changing positions %sslower.", Monnam(mon), howmuch);
        } else {
            pline_mon(mon, "%s seems to be moving %sslower.", Monnam(mon), howmuch);
        }

        /* might discover an object if we see the speed change happen */
        if (obj != 0) {
            learn_wand(obj);
        }
    }
}

/* armor put on or taken off; might be magical variety
   [TODO: rename to 'update_mon_extrinsics()' and change all callers...] */
void
update_mon_intrinsics(struct monst *mon, struct obj *obj, boolean on, boolean silently)
{
    int unseen;
    uchar mask;
    struct obj *otmp;
    int which = (int) objects[obj->otyp].oc_oprop;

    unseen = !canseemon(mon);
    if (Is_glowing_dragon_armor(obj->otyp)) {
        if (on) {
            begin_burn(obj, FALSE);
        } else {
            end_burn(obj, FALSE);
        }
        if (!unseen && !silently) {
            if (on) pline("%s begins to glow.", The(xname(obj)));
        }
    }

    if (!which) {
        goto maybe_blocks;
    }

    if (on) {
        switch (which) {
        case INVIS:
            mon->minvis = !mon->invis_blkd;
            break;
        case FAST:
        {
            boolean save_in_mklev = in_mklev;
            if (silently) {
                in_mklev = TRUE;
            }
            mon_adjust_speed(mon, 0, obj);
            in_mklev = save_in_mklev;
            break;
        }
        /* properties handled elsewhere */
        case ANTIMAGIC:
        case REFLECTING:
        case PROTECTION:
            break;
        /* properties which have no effect for monsters */
        case CLAIRVOYANT:
        case STEALTH:
        case TELEPAT:
            break;
        /* properties which should have an effect but aren't implemented */
        case LEVITATION:
        case FLYING:
        case WWALKING:
            break;
        /* properties which maybe should have an effect but don't */
        case DISPLACED:
        case FUMBLING:
        case JUMPING:
            break;
        default:
            mon->mextrinsics |= (unsigned short) res_to_mr(which);
            break;
        }
    } else {        /* off */
        switch (which) {
        case INVIS:
            mon->minvis = mon->perminvis;
            break;
        case FAST:
        {
            boolean save_in_mklev = in_mklev;
            if (silently) {
                in_mklev = TRUE;
            }
            mon_adjust_speed(mon, 0, obj);
            in_mklev = save_in_mklev;
            break;
        }
        case FIRE_RES:
        case COLD_RES:
        case SLEEP_RES:
        case DISINT_RES:
        case SHOCK_RES:
        case POISON_RES:
        case ACID_RES:
        case STONE_RES:
            /*
             * Update monster's extrinsics (for worn objects only;
             * 'obj' itself might still be worn or already unworn).
             */
            mask = res_to_mr(which);
            for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
                if (otmp == obj || !otmp->owornmask) {
                    continue;
                }
                if ((int) objects[otmp->otyp].oc_oprop == which) {
                    break;
                }
            }
            if (!otmp) {
                mon->mextrinsics &= ~((unsigned short) mask);
            }
            break;
        default:
            break;
        }
    }

maybe_blocks:
    /* obj->owornmask has been cleared by this point, so we can't use it.
       However, since monsters don't wield armor, we don't have to guard
       against that and can get away with a blanket worn-mask value. */
    switch (w_blocks(obj, ~0L)) {
    case INVIS:
        mon->invis_blkd = on ? 1 : 0;
        mon->minvis = on ? 0 : mon->perminvis;
        break;
    default:
        break;
    }

    if (!on && mon == u.usteed && obj->otyp == SADDLE) {
        dismount_steed(DISMOUNT_FELL);
    }

    /* if couldn't see it but now can, or vice versa, update display */
    if (!silently && (unseen ^ !canseemon(mon))) {
        newsym(mon->mx, mon->my);
    }
}

#define update_mon_extrinsics update_mon_intrinsics

int
find_mac(struct monst *mon)
{
    struct obj *obj;
    int base = mon->data->ac;
    long mwflags = mon->misc_worn_check;

    /* blinkers are special; when they are not attacking, they get
     * AC bonus. */
    if (is_blinker(mon->data) && mon->mflee) {
        base -= 10;
    }


    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (obj->owornmask & mwflags) {
            base -= ARM_BONUS(obj);
        }
        /* since ARM_BONUS is positive, subtracting it increases AC */
    }
    /* same cap as for hero [find_ac(do_wear.c)] */
    if (abs(base) > AC_MAX) {
        base = sgn(base) * AC_MAX;
    }
    return base;
}

/*
 * weapons are handled separately;
 * rings and eyewear aren't used by monsters
 */

/* Wear the best object of each type that the monster has.  During creation,
 * the monster can put everything on at once; otherwise, wearing takes time.
 * This doesn't affect monster searching for objects--a monster may very well
 * search for objects it would not want to wear, because we don't want to
 * check which_armor() each round.
 *
 * We'll let monsters put on shirts and/or suits under worn cloaks, but
 * not shirts under worn suits.  This is somewhat arbitrary, but it's
 * too tedious to have them remove and later replace outer garments,
 * and preventing suits under cloaks makes it a little bit too easy for
 * players to influence what gets worn.  Putting on a shirt underneath
 * already worn body armor is too obviously buggy...
 */
void
m_dowear(struct monst *mon, boolean creation)
{
    boolean can_wear_armor;

#define RACE_EXCEPTION TRUE
    /* Note the restrictions here are the same as in dowear in do_wear.c
     * except for the additional restriction on intelligence.  (Players
     * are always intelligent, even if polymorphed).
     */
    if (verysmall(mon->data) || nohands(mon->data) || is_animal(mon->data)) {
        return;
    }
    /* give mummies a chance to wear their wrappings
     * and let skeletons wear their initial armor */
    if (mindless(mon->data) && (!creation ||
                                (mon->data->mlet != S_MUMMY && mon->data != &mons[PM_SKELETON])))
        return;

    m_dowear_type(mon, W_AMUL, creation, FALSE);
    can_wear_armor = !cantweararm(mon->data); /* for suit, cloak, shirt */
    /* can't put on shirt if already wearing suit */
    if (can_wear_armor && !(mon->misc_worn_check & W_ARM)) {
        m_dowear_type(mon, W_ARMU, creation, FALSE);
    }
    /* WrappingAllowed() makes any size between small and huge eligible;
       treating small as a special case allows hobbits, gnomes, and
       kobolds to wear all cloaks; large and huge allows giants and such
       to wear mummy wrappings but not other cloaks */
    if (can_wear_armor || WrappingAllowed(mon->data)) {
        m_dowear_type(mon, W_ARMC, creation, FALSE);
    }
    m_dowear_type(mon, W_ARMH, creation, FALSE);
    if (!MON_WEP(mon) || !bimanual(MON_WEP(mon))) {
        m_dowear_type(mon, W_ARMS, creation, FALSE);
    }
    m_dowear_type(mon, W_ARMG, creation, FALSE);
    if (!slithy(mon->data) && mon->data->mlet != S_CENTAUR) {
        m_dowear_type(mon, W_ARMF, creation, FALSE);
    }
    if (can_wear_armor) {
        m_dowear_type(mon, W_ARM, creation, FALSE);
    } else {
        m_dowear_type(mon, W_ARM, creation, RACE_EXCEPTION);
    }
}

static void
m_dowear_type(
    struct monst *mon,
    long flag,               /* wornmask value */
    boolean creation,        /* no wear messages when mon is being created */
    boolean racialexception) /* small monsters that are allowed for player
                              * races (gnomes) can wear suits */
{
    struct obj *old, *best, *obj;
    long oldmask = 0L;
    int m_delay = 0;
    int sawmon = canseemon(mon),
        sawloc = cansee(mon->mx, mon->my);
    boolean autocurse;
    char nambuf[BUFSZ];

    if (mon->mfrozen) {
        return; /* probably putting previous item on */
    }

    /* Get a copy of monster's name before altering its visibility */
    Strcpy(nambuf, See_invisible ? Monnam(mon) : mon_nam(mon));

    old = which_armor(mon, flag);
    if (old && old->cursed) {
        return;
    }
    if (old && flag == W_AMUL) {
        return; /* no amulet better than life-saving or reflection */
    }
    best = old;

    for (obj = mon->minvent; obj; obj = obj->nobj) {
        switch (flag) {
        case W_AMUL:
            if (obj->oclass != AMULET_CLASS ||
                (obj->otyp != AMULET_OF_LIFE_SAVING &&
                 obj->otyp != AMULET_OF_REFLECTION))
                continue;
            best = obj;
            goto outer_break; /* life-saving or reflection; use it */
        case W_ARMU:
            if (!is_shirt(obj)) {
                continue;
            }
            break;
        case W_ARMC:
            if (!is_cloak(obj)) {
                continue;
            }
            /* mummy wrapping is only cloak allowed when bigger than human */
            if (mon->data->msize > MZ_HUMAN && obj->otyp != MUMMY_WRAPPING) {
                continue;
            }
            /* avoid mummy wrapping if it will allow hero to see mon (unless
               this is a new mummy; an invisible one is feasible via ^G) */
            if (mon->minvis && w_blocks(obj, W_ARMC) == INVIS && !See_invisible && !creation) {
                continue;
            }
            break;
        case W_ARMH:
            if (!is_helmet(obj)) {
                continue;
            }
            /* changing alignment is not implemented for monsters;
               priests and minions could change alignment but wouldn't
               want to, so they reject helms of opposite alignment */
            if (obj->otyp == HELM_OF_OPPOSITE_ALIGNMENT &&
                 (mon->ispriest || mon->isminion)) {
                continue;
            }
            /* (flimsy exception matches polyself handling) */
            if (has_horns(mon->data) && !is_flimsy(obj)) {
                continue;
            }
            break;
        case W_ARMS:
            if (!is_shield(obj)) {
                continue;
            }
            break;
        case W_ARMG:
            if (!is_gloves(obj)) {
                continue;
            }
            break;
        case W_ARMF:
            if (!is_boots(obj)) {
                continue;
            }
            break;
        case W_ARM:
            if (!is_suit(obj)) {
                continue;
            }
            if (racialexception && (racial_exception(mon, obj) < 1)) {
                continue;
            }
            break;
        }
        if (obj->owornmask) {
            continue;
        }
        /* I'd like to define a VISIBLE_ARM_BONUS which doesn't assume the
         * monster knows obj->spe, but if I did that, a monster would keep
         * switching forever between two -2 caps since when it took off one
         * it would forget spe and once again think the object is better
         * than what it already has.
         */
        if (best && (ARM_BONUS(best) + extra_pref(mon, best) >= ARM_BONUS(obj) + extra_pref(mon, obj))) {
            continue;
        }
        best = obj;
    }
outer_break:
    if (!best || best == old) {
        return;
    }

    /* same auto-cursing behavior as for hero */
    autocurse = ((best->otyp == HELM_OF_OPPOSITE_ALIGNMENT || best->otyp == DUNCE_CAP) &&
                 !best->cursed);

    /* if wearing a cloak, account for the time spent removing
       and re-wearing it when putting on a suit or shirt */
    if ((flag == W_ARM || flag == W_ARMU) &&
         (mon->misc_worn_check & W_ARMC)) {
        m_delay += 2;
    }
    /* when upgrading a piece of armor, account for time spent
       taking off current one */
    if (old) {
        m_delay += objects[old->otyp].oc_delay;

        oldmask = old->owornmask; /* needed later by artifact_light() */
        old->owornmask = 0L; /* avoid doname() showing "(being worn)" */
    }

    if (!creation) {
        if (sawmon) {
            char buf[BUFSZ], oldarm[BUFSZ], newarm[BUFSZ + sizeof "another "];

            /* "<Mon> [removes <oldarm> and ]puts on <newarm>."
               uses accessory verbs for armor but we can live with that */
            if (old) {
                Strcpy(oldarm, distant_name(old, doname));
                Snprintf(buf, sizeof buf, " removes %s and", oldarm);
            } else {
                buf[0] = oldarm[0] = '\0';
            }
            Strcpy(newarm, distant_name(best, doname));
            /* a monster will swap an item of the same type as the one it
               is replacing when the enchantment is better;
               if newarm and oldarm have identical descriptions, substitute
               "another <newarm>" for "a|an <newarm>" */
            if (!strcmpi(newarm, oldarm)) {
                /* size of newarm[] has been overallocated to guarantee
                   enough room to insert "another " */
                if (!strncmpi(newarm, "a ", 2)) {
                    (void) strsubst(newarm, "a ", "another ");
                } else if (!strncmpi(newarm, "an ", 3)) {
                    (void) strsubst(newarm, "an ", "another ");
                }
                newarm[BUFSZ - 1] = '\0';
            }
            pline_mon(mon, "%s%s puts on %s.", Monnam(mon), buf, newarm);
            if (autocurse) {
                pline("%s %s %s %s for a moment.", s_suffix(Monnam(mon)),
                      simpleonames(best), otense(best, "glow"),
                      hcolor(NH_BLACK));
            }
        } /* can see it */
        m_delay += objects[best->otyp].oc_delay;
        mon->mfrozen = m_delay;
        if (mon->mfrozen) {
            mon->mcanmove = 0;
        }
    }
    if (old) {
        update_mon_extrinsics(mon, old, FALSE, creation);

        /* owornmask was cleared above but artifact_light() expects it */
        old->owornmask = oldmask;
        if (old->lamplit && artifact_light(old)) {
            end_burn(old, FALSE);
        }
        old->owornmask = 0L;
    }
    mon->misc_worn_check |= flag;
    best->owornmask |= flag;
    if (autocurse) {
        curse(best);
    }
    if (artifact_light(best) && !best->lamplit) {
        begin_burn(best, FALSE);
        vision_recalc(1);
        if (!creation && best->lamplit && cansee(mon->mx, mon->my)) {
            const char *adesc = arti_light_description(best);

            if (sawmon) { /* could already see monster */
                pline("%s %s to shine %s.", Yname2(best),
                      otense(best, "begin"), adesc);
            } else if (canseemon(mon)) { /* didn't see it until new light */
                pline("%s %s shining %s.", Yname2(best),
                      otense(best, "are"), adesc);
            } else if (sawloc) { /* saw location but not invisible monster */
                pline("%s begins to shine %s.", Something, adesc);
            } else { /* didn't see location until new light */
                pline("%s is shining %s.", Something, adesc);
            }
        }
    }
    update_mon_extrinsics(mon, best, TRUE, creation);
    /* if couldn't see it but now can, or vice versa, */
    if (!creation && (sawmon ^ canseemon(mon))) {
        if (mon->minvis && !See_invisible) {
            pline("Suddenly you cannot see %s.", nambuf);
            makeknown(best->otyp);
        /* } else if (!mon->minvis) {
         *     pline("%s suddenly appears!", Amonnam(mon)); */
        }
    }
}
#undef RACE_EXCEPTION

struct obj *
which_armor(struct monst *mon, long flag)
{
    if (mon == &youmonst) {
        switch (flag) {
        case W_ARM:
            return uarm;
        case W_ARMC:
            return uarmc;
        case W_ARMH:
            return uarmh;
        case W_ARMS:
            return uarms;
        case W_ARMG:
            return uarmg;
        case W_ARMF:
            return uarmf;
        case W_ARMU:
            return uarmu;
        default:
            impossible("bad flag in which_armor");
            return 0;
        }
    } else {
        struct obj *obj;

        for (obj = mon->minvent; obj; obj = obj->nobj) {
            if (obj->owornmask & flag) {
                return obj;
            }
        }
        return (struct obj *)0;
    }
}

/* remove an item of armor and then drop it */
static void
m_lose_armor(
    struct monst *mon,
    struct obj *obj,
    boolean polyspot)
{
    extract_from_minvent(mon, obj, TRUE, FALSE);
    place_object(obj, mon->mx, mon->my);
    if (polyspot) {
        bypass_obj(obj);
    }
    /* call stackobj() if we ever drop anything that can merge */
    newsym(mon->mx, mon->my);
}

/* clear bypass bits for an object chain, plus contents if applicable */
static void
clear_bypass(struct obj *objchn)
{
    struct obj *o;

    for (o = objchn; o; o = o->nobj) {
        o->bypass = 0;
        if (Has_contents(o)) {
            clear_bypass(o->cobj);
        }
    }
}

/* all objects with their bypass bit set should now be reset to normal;
   this can be a relatively expensive operation so is only called if
   svc.context.bypasses is set */
void
clear_bypasses(void)
{
    struct monst *mtmp;

    /*
     * 'Object' bypass is also used for one monster function:
     * polymorph control of long worms.  Activated via setting
     * svc.context.bypasses even if no specific object has been
     * bypassed.
     */

    clear_bypass(fobj);
    clear_bypass(invent);
    clear_bypass(migrating_objs);
    clear_bypass(level.buriedobjlist);
    clear_bypass(billobjs);
    clear_bypass(go.objs_deleted);
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) {
            continue;
        }
        clear_bypass(mtmp->minvent);
        /* long worm created by polymorph has mon->mextra->mcorpsenm set
           to PM_LONG_WORM to flag it as not being subject to further
           polymorph (so polymorph zap won't hit monster to transform it
           into a long worm, then hit that worm's tail and transform it
           again on same zap); clearing mcorpsenm reverts worm to normal */
        if (mtmp->data == &mons[PM_LONG_WORM] && has_mcorpsenm(mtmp)) {
            MCORPSENM(mtmp) = NON_PM;
        }
    }
    for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
        clear_bypass(mtmp->minvent);
        /* no MCORPSENM(mtmp)==PM_LONG_WORM check here; long worms can't
           be just created by polymorph and migrating at the same time */
    }
    /* this is a no-op since mydogs is only non-Null during level change or
       final ascension and we aren't called at those times, but be thorough */
    for (mtmp = mydogs; mtmp; mtmp = mtmp->nmon) {
        clear_bypass(mtmp->minvent);
    }
    /* ball and chain can be "floating", not on any object chain (when
       hero is swallowed by an engulfing monster, for instance) */
    if (uball) {
        uball->bypass = 0;
    }
    if (uchain) {
        uchain->bypass = 0;
    }
    flags.bypasses = FALSE;
}

void
bypass_obj(struct obj *obj)
{
    obj->bypass = 1;
    flags.bypasses = TRUE;
}

/* set or clear the bypass bit in a list of objects */
void
bypass_objlist(struct obj *objchain, boolean on) /**< TRUE => set, FALSE => clear */
{
    if (on && objchain) {
        flags.bypasses = TRUE;
    }
    while (objchain) {
        objchain->bypass = on ? 1 : 0;
        objchain = objchain->nobj;
    }
}

/* return the first object without its bypass bit set; set that bit
   before returning so that successive calls will find further objects */
struct obj *
nxt_unbypassed_obj(struct obj *objchain)
{
    while (objchain) {
        if (!objchain->bypass) {
            bypass_obj(objchain);
            break;
        }
        objchain = objchain->nobj;
    }
    return objchain;
}

void
mon_break_armor(struct monst *mon, boolean polyspot)
{
    struct obj *otmp;
    struct permonst *mdat = mon->data;
    boolean vis = cansee(mon->mx, mon->my),
            handless_or_tiny = (nohands(mdat) || verysmall(mdat)),
            noride = FALSE;
    const char *pronoun = mhim(mon),
               *ppronoun = mhis(mon);

    if (breakarm(mdat)) {
        if ((otmp = which_armor(mon, W_ARM)) != 0) {
            if ((Is_dragon_scales(otmp->otyp) &&
                 mdat == Dragon_scales_to_pm(otmp)) ||
                (Is_dragon_mail(otmp->otyp) && mdat == Dragon_mail_to_pm(otmp))) {
                ; /* no message here;
                     "the dragon merges with his scaly armor" is odd
                     and the monster's previous form is already gone */
            } else {
                if (vis) {
                    pline_mon(mon, "%s breaks out of %s armor!", Monnam(mon), ppronoun);
                } else {
                    You_hear("a cracking sound.");
                }
            }
            m_useup(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0 &&
            /* mummy wrapping adapts to small and very big sizes */
             (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(mdat))) {
            if (otmp->oartifact) {
                if (vis) {
                    pline_mon(mon, "%s %s falls off!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                }
                m_lose_armor(mon, otmp, polyspot);
            } else {
                if (vis) {
                    pline_mon(mon, "%s %s tears apart!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                } else {
                    You_hear("a ripping sound.");
                }
                m_useup(mon, otmp);
            }
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (vis) {
                pline_mon(mon, "%s shirt rips to shreds!", s_suffix(Monnam(mon)));
            } else {
                You_hear("a ripping sound.");
            }
            m_useup(mon, otmp);
        }
    } else if (sliparm(mdat)) {
        /* sliparm checks whirly, noncorporeal, and small or under */
        boolean passes_thru_clothes = !(mdat->msize <= MZ_SMALL);

        if ((otmp = which_armor(mon, W_ARM)) != 0) {
            if (vis) {
                pline_mon(mon, "%s armor falls around %s!",
                          s_suffix(Monnam(mon)), pronoun);
            } else {
                You_hear("a thud.");
            }
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0 &&
            /* mummy wrapping adapts to small and very big sizes */
             (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(mdat))) {
            if (vis) {
                if (is_whirly(mon->data)) {
                    pline_mon(mon, "%s %s falls, unsupported!",
                              s_suffix(Monnam(mon)), cloak_simple_name(otmp));
                } else {
                    pline_mon(mon, "%s shrinks out of %s %s!",
                              Monnam(mon), ppronoun,
                              cloak_simple_name(otmp));
                }
            }
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (vis) {
                if (passes_thru_clothes) {
                    pline_mon(mon, "%s seeps right through %s shirt!",
                              Monnam(mon), ppronoun);
                } else {
                    pline_mon(mon, "%s becomes much too small for %s shirt!",
                          Monnam(mon), ppronoun);
                }
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny) {
        /* [caller needs to handle weapon checks] */
        if ((otmp = which_armor(mon, W_ARMG)) != 0) {
            if (vis) {
                pline_mon(mon, "%s drops %s gloves%s!",
                          Monnam(mon), ppronoun,
                          MON_WEP(mon) ? " and weapon" : "");
            }
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMS)) != 0) {
            if (vis) {
                pline_mon(mon, "%s can no longer hold %s shield!", Monnam(mon), ppronoun);
            } else {
                You_hear("a clank.");
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny || has_horns(mdat)) {
        if ((otmp = which_armor(mon, W_ARMH)) != 0 &&
            /* flimsy test for horns matches polyself handling */
            (handless_or_tiny || !is_flimsy(otmp))) {
            if (vis) {
                pline_mon(mon, "%s helmet falls to the %s!",
                          s_suffix(Monnam(mon)), surface(mon->mx, mon->my));
            } else {
                You_hear("a clank.");
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny || slithy(mdat) || mdat->mlet == S_CENTAUR) {
        if ((otmp = which_armor(mon, W_ARMF)) != 0) {
            if (vis) {
                if (is_whirly(mon->data)) {
                    pline_mon(mon, "%s boots fall away!",
                          s_suffix(Monnam(mon)));
                } else {
                    pline_mon(mon, "%s boots %s off %s feet!",
                              s_suffix(Monnam(mon)),
                              verysmall(mdat) ? "slide" : "are pushed", ppronoun);
                }
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (!can_saddle(mon)) {
        if ((otmp = which_armor(mon, W_SADDLE)) != 0) {
            m_lose_armor(mon, otmp, polyspot);
            if (vis) {
                pline_mon(mon, "%s saddle falls off.", s_suffix(Monnam(mon)));
            }
        }
        if (mon == u.usteed) {
            noride = TRUE;
        }
    }
    if (noride || (mon == u.usteed && !can_ride(mon))) {
        You("can no longer ride %s.", mon_nam(mon));
        if (touch_petrifies(u.usteed->data) &&
            !Stone_resistance && rnl(3)) {
            char buf[BUFSZ];

            You("touch %s.", mon_nam(u.usteed));
            Sprintf(buf, "falling off %s",
                    an(u.usteed->data->mname));
            instapetrify(buf);
        }
        dismount_steed(DISMOUNT_FELL);
    }
}

/* bias a monster's preferences towards armor that has special benefits. */
static int
extra_pref(struct monst *mon, struct obj *obj)
{
    /* currently only does speed boots, but might be expanded if monsters
     * get to use more armor abilities
     */
    if (obj) {
        if (obj->otyp == SPEED_BOOTS && mon->permspeed != MFAST) {
            return 20;
        }
    }
    return 0;
}

/*
 * Exceptions to things based on race.
 * Correctly checks polymorphed player race.
 * Returns:
 *       0 No exception, normal rules apply.
 *       1 If the race/object combination is acceptable.
 *      -1 If the race/object combination is unacceptable.
 */
int
racial_exception(struct monst *mon, struct obj *obj)
{
    const struct permonst *ptr = raceptr(mon);

    /* Acceptable Exceptions: */
    /* Allow hobbits to wear elven armor - LoTR */
    if (ptr == &mons[PM_HOBBIT] && is_elven_armor(obj)) {
        return 1;
    }
    /* Unacceptable Exceptions: */
    /* Checks for object that certain races should never use go here */
    /*  return -1; */

    return 0;
}

/* Remove an object from a monster's inventory. */
void
extract_from_minvent(
    struct monst *mon,
    struct obj *obj,
    boolean do_extrinsics,  /* whether to call update_mon_extrinsics */
    boolean silently)       /* doesn't affect all possible messages,
                             * just update_mon_extrinsics's */
{
    long unwornmask = obj->owornmask;

    /*
     * At its core this is just obj_extract_self(), but it also handles
     * any updates that need to happen if the gear is equipped or in
     * some other sort of state that needs handling.
     * Note that like obj_extract_self(), this leaves obj free.
     */

    if (obj->where != OBJ_MINVENT) {
        impossible("extract_from_minvent called on object not in minvent");
        return;
    }
    /* handle gold dragon scales/scale-mail (lit when worn) before clearing
       obj->owornmask because artifact_light() expects that to be W_ARM */
    if ((unwornmask & W_ARM) != 0 && obj->lamplit && artifact_light(obj)) {
        end_burn(obj, FALSE);
    }

    obj_extract_self(obj);
    obj->owornmask = 0L;
    if (unwornmask) {
        if (!DEADMONSTER(mon) && do_extrinsics) {
            update_mon_extrinsics(mon, obj, FALSE, silently);
        }
        mon->misc_worn_check &= ~unwornmask;
        /* give monster a chance to wear other equipment on its next
           move instead of waiting until it picks something up */
        check_gear_next_turn(mon);
    }
    obj_no_longer_held(obj);
    if (unwornmask & W_WEP) {
        mwepgone(mon); /* unwields and sets weapon_check to NEED_WEAPON */
    }
}

#undef w_blocks

/*worn.c*/
