/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "artilist.h"
/*
 * Note:  both artilist[] and artiexist[] have a dummy element #0,
 *    so loops over them should normally start at #1.  The primary
 *    exception is the save & restore code, which doesn't care about
 *    the contents, just the total size.
 */

extern boolean notonhead;   /* for long worms */

#define get_artifact(o) \
    (((o)&&(o)->oartifact) ? &artilist[(int) (o)->oartifact] : 0)

static boolean bane_applies(const struct artifact *, struct monst *);
static int spec_applies(const struct artifact *, struct monst *);
static int arti_invoke(struct obj*);
static boolean Mb_hit(struct monst *magr, struct monst *mdef,
                      struct obj *, int *, int, boolean, char *);

/* The amount added to the victim's total hit points to insure that the
   victim will be killed even after damage bonus/penalty adjustments.
   Most such penalties are small, and 200 is plenty; the exception is
   half physical damage.  3.3.1 and previous versions tried to use a very
   large number to account for this case; now, we just compute the fatal
   damage by adding it to 2 times the total hit points instead of 1 time.
   Note: this will still break if they have more than about half the number
   of hit points that will fit in a 15 bit integer. */
#define FATAL_DAMAGE_MODIFIER 200

/* coordinate effects from spec_dbon() with messages in artifact_hit() */
static int spec_dbon_applies = 0;

/* flags including which artifacts have already been created */
static boolean artiexist[1+NROFARTIFACTS+1];
/* and a discovery list for them (no dummy first entry here) */
static xint16 artidisco[NROFARTIFACTS];

static void hack_artifacts(void);
static boolean attacks(int, struct obj *);

/* handle some special cases; must be called after u_init() */
static void
hack_artifacts(void)
{
    struct artifact *art;
    int alignmnt = aligns[flags.initalign].value;

    /* Fix up the alignments of "gift" artifacts */
    for (art = artilist+1; art->otyp; art++) {
        if (art->role == Role_switch && art->alignment != A_NONE) {
            art->alignment = alignmnt;
        }
    }

    /* Excalibur can be used by any lawful character, not just knights */
    if (!Role_if(PM_KNIGHT)) {
        artilist[ART_EXCALIBUR].role = NON_PM;
    }

    /* Fix up the quest artifact */
    if (urole.questarti) {
        artilist[urole.questarti].alignment = alignmnt;
        artilist[urole.questarti].role = Role_switch;
    }
    return;
}

/* zero out the artifact existence list */
void
init_artifacts(void)
{
    (void) memset((genericptr_t) artiexist, 0, sizeof artiexist);
    (void) memset((genericptr_t) artidisco, 0, sizeof artidisco);
    hack_artifacts();
}

void
save_artifacts(NHFILE *nhfp)
{
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) artiexist, sizeof artiexist);
        bwrite(nhfp->fd, (genericptr_t) artidisco, sizeof artidisco);
    }
}

void
restore_artifacts(NHFILE *nhfp)
{
    if (nhfp->structlevel) {
        mread(nhfp->fd, (genericptr_t) artiexist, sizeof artiexist);
        mread(nhfp->fd, (genericptr_t) artidisco, sizeof artidisco);
    }
    hack_artifacts(); /* redo non-saved special cases */
}

const char *
artiname(int artinum)
{
    if (artinum <= 0 || artinum > NROFARTIFACTS) {
        return "";
    }

    return artilist[artinum].name;
}

/*
   Make an artifact.  If a specific alignment is specified, then an object of
   the appropriate alignment is created from scratch, or 0 is returned if
   none is available.  (If at least one aligned artifact has already been
   given, then unaligned ones also become eligible for this.)
   If no alignment is given, then 'otmp' is converted
   into an artifact of matching type, or returned as-is if that's not possible.

   For the 2nd case, caller should use ``obj = mk_artifact(obj, A_NONE);''
   for the 1st, ``obj = mk_artifact((struct obj *) 0, some_alignment);''.
 */
struct obj *
mk_artifact(
    struct obj *otmp, /**< existing object; ignored if alignment specified */
    aligntyp alignment) /**< target alignment, or A_NONE */
{
    const struct artifact *a;
    int n, m;
    boolean by_align = (alignment != A_NONE);
    short o_typ = (by_align || !otmp) ? 0 : otmp->otyp;
    boolean unique = !by_align && otmp && objects[o_typ].oc_unique;
    short eligible[NROFARTIFACTS];

    /* gather eligible artifacts */
    for (n = 0, a = artilist+1, m = 1; a->otyp; a++, m++) {
        if ((!by_align ? a->otyp == o_typ :
             (a->alignment == alignment ||
              (a->alignment == A_NONE && u.ugifts > 0))) &&
            (!(a->spfx & SPFX_NOGEN) || unique) && !artiexist[m]) {
            /* role before race, so certain classes will still get their
               first sacrifice gift even if they're not the right species */
            if (by_align && Role_if(a->role)) {
                goto make_artif; /* 'a' points to the desired one */
            } else if (by_align && a->race != NON_PM && race_hostile(&mons[a->race])) {
                continue; /* skip enemies' equipment */
            } else {
                eligible[n++] = m;
            }
        }
    }

    if (n) {        /* found at least one candidate */
        m = eligible[rn2(n)];   /* [0..n-1] */
        a = &artilist[m];

        /* make an appropriate object if necessary, then christen it */
make_artif: if (by_align) {
    otmp = mksobj((int)a->otyp, TRUE, FALSE);
}
        otmp = oname(otmp, a->name);
        otmp->oartifact = m;
        artiexist[m] = TRUE;
    } else {
        /* nothing appropriate could be found; return the original object */
        if (by_align) {
            otmp = 0; /* (there was no original object) */
        }
    }
    return otmp;
}

/*
 * Returns the full name (with articles and correct capitalization) of an
 * artifact named "name" if one exists, or NULL, it not.
 * The given name must be rather close to the real name for it to match.
 * The object type of the artifact is returned in otyp if the return value
 * is non-NULL.
 */
const char*
artifact_name(
    const char *name, /**< string from player that might be an artifact name */
    short *otyp_p,    /**< secondary output */
    boolean fuzzy)    /**< whether to allow extra or omitted spaces or dashes */
{
    const struct artifact *a;
    const char *aname;

    if (!strncmpi(name, "the ", 4)) {
        name += 4;
    }

    for (a = artilist + 1; a->otyp; a++) {
        aname = a->name;
        if (!strncmpi(aname, "the ", 4)) {
            aname += 4;
        }
        if (!fuzzy ? !strcmpi(name, aname) : fuzzymatch(name, aname, " -", TRUE)) {
            if (otyp_p) {
                *otyp_p = a->otyp;
            }

            return a->name;
        }
    }

    return (char *) 0;
}

boolean
exist_artifact(int otyp, const char *name)
{
    const struct artifact *a;
    boolean *arex;

    if (otyp && *name) {
        for (a = artilist+1, arex = artiexist+1; a->otyp; a++, arex++) {
            if ((int) a->otyp == otyp && !strcmp(a->name, name)) {
                return *arex;
            }
        }
    }

    return FALSE;
}

void
artifact_exists(struct obj *otmp, const char *name, boolean mod)
{
    const struct artifact *a;

    if (otmp && *name) {
        for (a = artilist + 1; a->otyp; a++) {
            if (a->otyp == otmp->otyp && !strcmp(a->name, name)) {
                int m = a - artilist;
                otmp->oartifact = (char)(mod ? m : 0);
                otmp->age = 0;
                if (otmp->otyp == RIN_INCREASE_DAMAGE) {
                    otmp->spe = 0;
                }
                artiexist[m] = mod;
                break;
            }
        }
    }
}

int
nartifact_exist(void)
{
    int a = 0;
    int n = SIZE(artiexist);

    while (n > 1) {
        if (artiexist[--n]) {
            a++;
        }
    }

    return a;
}

boolean
spec_ability(struct obj *otmp, long unsigned int abil)
{
    const struct artifact *arti = get_artifact(otmp);

    return (boolean)(arti && (arti->spfx & abil));
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
confers_luck(struct obj *obj)
{
    /* might as well check for this too */
    if (obj->otyp == LUCKSTONE) {
        return TRUE;
    }

    return (obj->oartifact && spec_ability(obj, SPFX_LUCK));
}

/* used to check whether a monster is getting reflection from an artifact */
boolean
arti_reflects(struct obj *obj)
{
    const struct artifact *arti = get_artifact(obj);

    if (arti) {
        /* while being worn */
        if ((obj->owornmask & ~W_ART) && (arti->spfx & SPFX_REFLECT)) {
            return TRUE;
        }
        /* just being carried */
        if (arti->cspfx & SPFX_REFLECT) {
            return TRUE;
        }
    }
    return FALSE;
}

/* decide whether this obj is effective when attacking against shades;
   does not consider the bonus for blessed objects versus undead */
boolean
shade_glare(struct obj *obj)
{
    const struct artifact *arti;

    /* any silver object is effective */
    if (objects[obj->otyp].oc_material == SILVER) {
        return TRUE;
    }
    /* non-silver artifacts with bonus against undead also are effective */
    arti = get_artifact(obj);
    if (arti && (arti->spfx & SPFX_DFLAG2) && arti->mtype == M2_UNDEAD) {
        return TRUE;
    }
    /* [if there was anything with special bonus against noncorporeals,
       it would be effective too] */
    /* otherwise, harmless to shades */
    return FALSE;
}

/** returns 1 if name is restricted */
boolean
restrict_name(
    struct obj *otmp,
    const char *name,
    boolean restrict_typ) /**< restrict for otmp->otyp? */
{
    const struct artifact *a;
    const char *aname;

    if (!*name) {
        return FALSE;
    }
    if (!strncmpi(name, "the ", 4)) {
        name += 4;
    }

    /* Since almost every artifact is SPFX_RESTR, it doesn't cost
       us much to do the string comparison before the spfx check.
       Bug fix:  don't name multiple elven daggers "Sting".
     */
    for (a = artilist+1; a->otyp; a++) {
        if (restrict_typ && a->otyp != otmp->otyp) {
            continue;
        }
        aname = a->name;
        if (!strncmpi(aname, "the ", 4)) {
            aname += 4;
        }
        if (!strcmpi(aname, name)) {
            return ((boolean)((a->spfx & (SPFX_NOGEN|SPFX_RESTR)) != 0 ||
                              otmp->quan > 1L));
        }
    }

    return FALSE;
}

static boolean
attacks(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;

    if ((weap = get_artifact(otmp)) != 0) {
        return (boolean)(weap->attk.adtyp == adtyp);
    }
    return FALSE;
}

boolean
defends(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;
    unsigned long mask;

    if ((weap = get_artifact(otmp)) != 0) {
        switch (adtyp) {
        case AD_FIRE:
            mask = (weap->defn & SPDF_FIRE);
            break;
        case AD_COLD:
            mask = (weap->defn & SPDF_COLD);
            break;
        case AD_ELEC:
            mask = (weap->defn & SPDF_ELEC);
            break;
        case AD_SLEE:
            mask = (weap->defn & SPDF_SLEEP);
            break;
        case AD_DISN:
            mask = (weap->defn & SPDF_DISINT);
            break;
        case AD_DRST:
            mask = (weap->defn & SPDF_POISON);
            break;
        case AD_ACID:
            mask = (weap->defn & SPDF_ACID);
            break;
        case AD_MAGM:
            mask = (weap->defn & SPDF_MAGIC);
            break;
        case AD_BLND:
            mask = (weap->defn & SPDF_BLIND);
            break;
        case AD_WERE:
            mask = (weap->defn & SPDF_WERE);
            break;
        case AD_DRLI:
            mask = (weap->defn & SPDF_DRAIN);
            break;
        case AD_STUN:
            mask = (weap->defn & SPDF_STUN);
            break;
        case AD_CONF:
            mask = (weap->defn & SPDF_CONFUSE);
            break;
        case AD_DISE:
        case AD_DRIN:
            mask = 0;       /* none yet */
            break;
        default:
#ifdef DEBUG
            /* Don't whine about this unless we really want to */
            pline("strange attack type in defends(): %d", adtyp);
#endif
            return FALSE;
        }
        return (mask > 0);  /* straight cast will fail */
    }
    return FALSE;
}

/* used for monsters */
boolean
defends_when_carried(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;

    if ((weap = get_artifact(otmp))) {
        return (boolean) (weap->cary.adtyp == adtyp);
    }
    return FALSE;
}

/* determine whether an item confers Protection */
boolean
protects(struct obj *otmp, boolean being_worn)
{
    const struct artifact *arti;

    if (being_worn && objects[otmp->otyp].oc_oprop == PROTECTION) {
        return TRUE;
    }
    arti = get_artifact(otmp);
    if (!arti) {
        return FALSE;
    }
    return (arti->cspfx & SPFX_PROTECT) || (being_worn && (arti->spfx & SPFX_PROTECT));
}

/*
 * a potential artifact has just been worn/wielded/picked-up or
 * unworn/unwielded/dropped.  Pickup/drop only set/reset the W_ART mask.
 */
void
set_artifact_intrinsic(struct obj *otmp, boolean on, long int wp_mask)
{
    long *mask = 0;
    const struct artifact *oart = get_artifact(otmp);
    uchar dtyp;
    long spfx;

    if (!oart) {
        return;
    }

    /* effects from the defn field */
    dtyp = (wp_mask != W_ART) ? 0 : oart->cary.adtyp;

    if (dtyp == AD_FIRE) {
        mask = &EFire_resistance;
    } else if (dtyp == AD_COLD) {
        mask = &ECold_resistance;
    } else if (dtyp == AD_ELEC) {
        mask = &EShock_resistance;
    } else if (dtyp == AD_MAGM) {
        mask = &EAntimagic;
    } else if (dtyp == AD_DISN) {
        mask = &EDisint_resistance;
    } else if (dtyp == AD_DRST) {
        mask = &EPoison_resistance;
    } else if (dtyp == AD_DRLI) {
        mask = &EDrain_resistance;
    }

    if (mask && wp_mask == W_ART && !on) {
        /* find out if some other artifact also confers this intrinsic */
        /* if so, leave the mask alone */
        struct obj* obj;
        for (obj = invent; obj; obj = obj->nobj) {
            if (obj != otmp && obj->oartifact) {
                const struct artifact *art = get_artifact(obj);
                if (art && art->cary.adtyp == dtyp) {
                    mask = (long *) 0;
                    break;
                }
            }
        }
    }
    if (mask) {
        if (on) {
            *mask |= wp_mask;
        } else {
            *mask &= ~wp_mask;
        }
    }

    /* effects from the defn field; could be more than one
        ...Note that we don't have to check for other stuff, because
        this uses wp_mask directly and you can only have one of
        any given thing in the appropriate slot */
    if (wp_mask & (W_WEP | W_ARMOR | W_AMUL | W_RING | W_TOOL)) {
        if (oart->defn & SPDF_FIRE) {
            if (on) {
                EFire_resistance |= wp_mask;
            } else {
                EFire_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_COLD) {
            if (on) {
                ECold_resistance |= wp_mask;
            } else {
                ECold_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_ELEC) {
            if (on) {
                EShock_resistance |= wp_mask;
            } else {
                EShock_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_SLEEP) {
            if (on) {
                ESleep_resistance |= wp_mask;
            } else {
                ESleep_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_POISON) {
            if (on) {
                EPoison_resistance |= wp_mask;
            } else {
                EPoison_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_ACID) {
            if (on) {
                EAcid_resistance |= wp_mask;
            } else {
                EAcid_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_DISINT) {
            if (on) {
                EDisint_resistance |= wp_mask;
            } else {
                EDisint_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_DRAIN) {
            if (on) {
                EDrain_resistance |= wp_mask;
            } else {
                EDrain_resistance &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_MAGIC) {
            if (on) {
                EAntimagic |= wp_mask;
            } else {
                EAntimagic &= ~wp_mask;
            }
        }
        if (oart->defn & SPDF_WERE) {
            /* Doesn't have a specific resistance in the table */
        }
        if (oart->defn & SPDF_BLIND) {
            /* Doesn't have a specific resistance in the table */
        }
        if (oart->defn & SPDF_CONFUSE) {
            /* Doesn't have a specific resistance in the table */
        }
        if (oart->defn & SPDF_STUN) {
            /* Doesn't have a specific resistance in the table */
        }
    }

    /* intrinsics from the spfx field; there could be more than one */
    spfx = (wp_mask != W_ART) ? oart->spfx : oart->cspfx;
    if (spfx && wp_mask == W_ART && !on) {
        /* don't change any spfx also conferred by other artifacts */
        struct obj* obj;
        for (obj = invent; obj; obj = obj->nobj) {
            if (obj != otmp && obj->oartifact) {
                const struct artifact *art = get_artifact(obj);
                if (art) {
                    spfx &= ~art->cspfx;
                }
            }
        }
    }

    if (spfx & SPFX_SEARCH) {
        if (on) {
            ESearching |= wp_mask;
        } else {
            ESearching &= ~wp_mask;
        }
    }
    if (spfx & SPFX_POLYC) {
        if (on) {
            EPolymorph_control |= wp_mask;
        } else {
            EPolymorph_control &= ~wp_mask;
        }
    }
    if (spfx & SPFX_HALRES) {
        /* make_hallucinated must (re)set the mask itself to get
         * the display right */
        /* restoring needed because this is the only artifact intrinsic
         * that can print a message--need to guard against being printed
         * when restoring a game
         */
        (void) make_hallucinated((long) !on, program_state.restoring ? FALSE : TRUE, wp_mask);
    }
    if (spfx & SPFX_ESP) {
        if (on) {
            ETelepat |= wp_mask;
        } else {
            ETelepat &= ~wp_mask;
        }
        see_monsters();
    }
    if (spfx & SPFX_DISPL) {
        if (on) {
            EDisplaced |= wp_mask;
        } else {
            EDisplaced &= ~wp_mask;
        }
    }
    if (spfx & SPFX_REGEN) {
        if (on) {
            ERegeneration |= wp_mask;
        } else {
            ERegeneration &= ~wp_mask;
        }
    }
    if (spfx & SPFX_TCTRL) {
        if (on) {
            ETeleport_control |= wp_mask;
        } else {
            ETeleport_control &= ~wp_mask;
        }
    }
    if (spfx & SPFX_WARN) {
        if (spec_m2(otmp)) {
            if (on) {
                EWarn_of_mon |= wp_mask;
                flags.warntype |= spec_m2(otmp);
            } else {
                EWarn_of_mon &= ~wp_mask;
                flags.warntype &= ~spec_m2(otmp);
            }
            see_monsters();
        } else {
            if (on) {
                EWarning |= wp_mask;
            } else {
                EWarning &= ~wp_mask;
            }
        }
    }
    if (spfx & SPFX_WARN_S) {
        if (oart->mtype) {
            if (on) {
                EWarn_of_mon |= wp_mask;
            } else {
                EWarn_of_mon &= ~wp_mask;
            }
            see_monsters();
        } else {
            if (on) {
                EWarning |= wp_mask;
            } else {
                EWarning &= ~wp_mask;
            }
        }
    }
    if (spfx & SPFX_EREGEN) {
        if (on) {
            EEnergy_regeneration |= wp_mask;
        } else {
            EEnergy_regeneration &= ~wp_mask;
        }
    }
    if (spfx & SPFX_HSPDAM) {
        if (on) {
            EHalf_spell_damage |= wp_mask;
        } else {
            EHalf_spell_damage &= ~wp_mask;
        }
    }
    if (spfx & SPFX_HPHDAM) {
        if (on) {
            EHalf_physical_damage |= wp_mask;
        } else {
            EHalf_physical_damage &= ~wp_mask;
        }
    }
    if (spfx & SPFX_XRAY) {
        /* this assumes that no one else is using xray_range */
        if (on) {
            u.xray_range = 3;
        } else {
            u.xray_range = -1;
        }
        vision_full_recalc = 1;
    }
    if ((spfx & SPFX_REFLECT) && (wp_mask & W_WEP)) {
        if (on) {
            EReflecting |= wp_mask;
        } else {
            EReflecting &= ~wp_mask;
        }
    }

    if (wp_mask == W_ART && !on && oart->inv_prop) {
        /* might have to turn off invoked power too */
        if (oart->inv_prop <= LAST_PROP &&
            (u.uprops[oart->inv_prop].extrinsic & W_ARTI))
            (void) arti_invoke(otmp);
    }
}

/* touch_artifact()'s return value isn't sufficient to tell whether it
   dished out damage, and tracking changes to u.uhp, u.mh, Lifesaved
   when trying to avoid second wounding is too cumbersome */
static boolean touch_blasted; /* for retouch_object() */

/*
 * creature (usually hero) tries to touch (pick up or wield) an artifact obj.
 * Returns 0 if the object refuses to be touched.
 * This routine does not change any object chains.
 * Ignores such things as gauntlets, assuming the artifact is not
 * fooled by such trappings.
 */
int
touch_artifact(struct obj *obj, struct monst *mon)
{
    const struct artifact *oart = get_artifact(obj);
    boolean badclass, badalign, self_willed, yours;

    touch_blasted = FALSE;
    if (!oart) {
        return 1;
    }

    /* [ALI] Thiefbane has a special affinity with shopkeepers */
    if ((mon->isshk || mon->data == &mons[PM_ONE_EYED_SAM]) &&
        obj->oartifact == ART_THIEFBANE) {
        return 1;
    }

    yours = (mon == &youmonst);
    /* all quest artifacts are self-willed; it this ever changes, `badclass'
       will have to be extended to explicitly include quest artifacts */
    self_willed = ((oart->spfx & SPFX_INTEL) != 0);
    if (yours) {
        badclass = self_willed &&
                   ((oart->role != NON_PM && !Role_if(oart->role)) ||
                    (oart->race != NON_PM && !Race_if(oart->race)));
        badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
                   (oart->alignment != u.ualign.type || u.ualign.record < 0);
    } else if (!is_covetous(mon->data) && !is_mplayer(mon->data)) {
        badclass = self_willed &&
                   oart->role != NON_PM && oart != &artilist[ART_EXCALIBUR];
        badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
                   (oart->alignment != mon_aligntyp(mon));
    } else {    /* an M3_WANTSxxx monster or a fake player */
        /* special monsters trying to take the Amulet, invocation tools or
           quest item can touch anything except for `spec_applies' artifacts */
        badclass = badalign = FALSE;
    }
    /* weapons which attack specific categories of monsters are
       bad for them even if their alignments happen to match */
    if (!badalign) {
        badalign = bane_applies(oart, mon);
    }

    if (((badclass || badalign) && self_willed) ||
        (badalign && (!yours || !rn2(4))))  {
        int dmg, tmp;
        char buf[BUFSZ];

        if (!yours) {
            return 0;
        }
        You("are blasted by %s power!", s_suffix(the(xname(obj))));
        touch_blasted = TRUE;
        dmg = d((Antimagic ? 2 : 4), (self_willed ? 10 : 4));
        /* add half (maybe quarter) of the usual silver damage bonus */
        if (objects[obj->otyp].oc_material == SILVER && Hate_silver) {
            tmp = rnd(10), dmg += Maybe_Half_Phys(tmp);
        }
        Sprintf(buf, "touching %s", oart->name);
        losehp(dmg, buf, KILLED_BY); /* magic damage, not physical */
        exercise(A_WIS, FALSE);
    }

    /* can pick it up unless you're totally non-synch'd with the artifact */
    if (badclass && badalign && self_willed) {
        if (yours) {
            if (!carried(obj)) {
                pline("%s your grasp!", Tobjnam(obj, "evade"));
            } else {
                pline("%s beyond your control!", Tobjnam(obj, "are"));
            }
        }
        return 0;
    }
    /* This is a kludge, but I'm not sure where else to put it */
    if (oart == &artilist[ART_IRON_BALL_OF_LIBERATION]) {
        if (Role_if(PM_CONVICT) && ((!obj->oerodeproof) || (obj->owt != 300))) {
            obj->oerodeproof = TRUE;
            obj->owt = 300; /* Magically lightened, but still heavy */
        }

        if (Punished && (obj != uball)) {
            unpunish(); /* Remove a mundane heavy iron ball */
        }
    }

    return 1;
}

/* decide whether an artifact itself is vulnerable to a particular type
   of erosion damage, independent of the properties of its bearer */
boolean
arti_immune(struct obj *obj, int dtyp)
{
    const struct artifact *weap = get_artifact(obj);

    if (!weap) {
        return FALSE;
    }
    if (dtyp == AD_PHYS) {
        return FALSE; /* nothing is immune to phys dmg */
    }
    return (weap->attk.adtyp == dtyp ||
            defends(dtyp, obj) ||
            weap->cary.adtyp == dtyp);
}

static boolean
bane_applies(const struct artifact *oart, struct monst *mon)
{
    struct artifact atmp;

    if (oart && (oart->spfx & SPFX_DBONUS) != 0) {
        atmp = *oart;
        atmp.spfx &= SPFX_DBONUS; /* clear other spfx fields */
        if (spec_applies(&atmp, mon)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* decide whether an artifact's special attacks apply against mtmp */
static int
spec_applies(const struct artifact *weap, struct monst *mtmp)
{
    struct permonst *ptr;
    boolean yours;

    if (!(weap->spfx & (SPFX_DBONUS | SPFX_ATTK))) {
        return (weap->attk.adtyp == AD_PHYS);
    }

    yours = (mtmp == &youmonst);
    ptr = mtmp->data;

    if (weap->spfx & SPFX_DMONS) {
        return (ptr == &mons[(int)weap->mtype]);
    } else if (weap->spfx & SPFX_DCLAS) {
        return (weap->mtype == (unsigned long)ptr->mlet);
    } else if (weap->spfx & SPFX_DFLAG1) {
        return ((ptr->mflags1 & weap->mtype) != 0L);
    } else if (weap->spfx & SPFX_DFLAG2) {
        return ((ptr->mflags2 & weap->mtype) || (yours &&
                                                 ((!Upolyd && (urace.selfmask & weap->mtype)) ||
                                                  ((weap->mtype & M2_WERE) && u.ulycn >= LOW_PM))));
    } else if (weap->spfx & SPFX_DALIGN) {
        return yours ? (u.ualign.type != weap->alignment) :
               (ptr->maligntyp == A_NONE ||
                sgn(ptr->maligntyp) != weap->alignment);
    } else if (weap->spfx & SPFX_ATTK) {
        struct obj *defending_weapon = (yours ? uwep : MON_WEP(mtmp));

        if (defending_weapon && defending_weapon->oartifact &&
            defends((int)weap->attk.adtyp, defending_weapon))
            return FALSE;
        switch (weap->attk.adtyp) {
        case AD_FIRE:
            return !(yours ? Fire_resistance : resists_fire(mtmp));
        case AD_COLD:
            return !(yours ? Cold_resistance : resists_cold(mtmp));
        case AD_ELEC:
            return !(yours ? Shock_resistance : resists_elec(mtmp));
        case AD_MAGM:
        case AD_STUN:
            return !(yours ? Antimagic : (rn2(100) < ptr->mr));
        case AD_DRST:
            return !(yours ? Poison_resistance : resists_poison(mtmp));
        case AD_DRLI:
            return !(yours ? Drain_resistance : resists_drli(mtmp));
        case AD_STON:
            return !(yours ? Stone_resistance : resists_ston(mtmp));
        default:    warning("Weird weapon special attack.");
        }
    }
    return 0;
}

/* return the M2 flags of monster that an artifact's special attacks apply against */
long
spec_m2(struct obj *otmp)
{
    const struct artifact *artifact = get_artifact(otmp);

    if (artifact) {
        return artifact->mtype;
    }
    return 0L;
}

/* special attack bonus */
int
spec_abon(struct obj *otmp, struct monst *mon)
{
    const struct artifact *weap = get_artifact(otmp);

    /* no need for an extra check for `NO_ATTK' because this will
       always return 0 for any artifact which has that attribute */

    if (weap && weap->attk.damn && spec_applies(weap, mon)) {
        return rnd((int)weap->attk.damn);
    }
    return 0;
}

/* special damage bonus */
int
spec_dbon(struct obj *otmp, struct monst *mon, int tmp)
{
    const struct artifact *weap = get_artifact(otmp);

    if (otmp->oclass == GEM_CLASS && uwep && uwep->oartifact == ART_GIANTSLAYER) {
        weap = get_artifact(uwep);
    }

    if (!weap || (weap->attk.adtyp == AD_PHYS && /* check for `NO_ATTK' */
                  weap->attk.damn == 0 && weap->attk.damd == 0))
        spec_dbon_applies = FALSE;
    else
        spec_dbon_applies = spec_applies(weap, mon);

    if (spec_dbon_applies) {
        return weap->attk.damd ? rnd((int)weap->attk.damd) : max(tmp, 1);
    }
    return 0;
}

/* add identified artifact to discoveries list */
void
discover_artifact(xint16 m)
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's not present, so add it */
    for (i = 0; i < NROFARTIFACTS; i++) {
        if (artidisco[i] == 0 || artidisco[i] == m) {
            artidisco[i] = m;
            return;
        }
    }
    /* there is one slot per artifact, so we should never reach the
       end without either finding the artifact or an empty slot... */
    warning("couldn't discover artifact (%d)", (int)m);
}

/* used to decide whether an artifact has been fully identified */
boolean
undiscovered_artifact(xint16 m)
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's undiscovered */
    for (i = 0; i < NROFARTIFACTS; i++) {
        if (artidisco[i] == m) {
            return FALSE;
        } else if (artidisco[i] == 0) {
            break;
        }
    }

    return TRUE;
}

/* display a list of discovered artifacts; return their count */
int
disp_artifact_discoveries(winid tmpwin) /**< supplied by dodiscover() */
{
    int i, m, otyp;
    char buf[BUFSZ];

    for (i = 0; i < NROFARTIFACTS; i++) {
        if (artidisco[i] == 0) {
            break; /* empty slot implies end of list */
        }

        if (tmpwin == WIN_ERR) {
            continue; /* for WIN_ERR, we just count */
        }

        if (i == 0) {
            putstr(tmpwin, iflags.menu_headings, "Artifacts");
        }
        m = artidisco[i];
        otyp = artilist[m].otyp;
        Sprintf(buf, "  %s [%s %s]", artiname(m),
                align_str(artilist[m].alignment), simple_typename(otyp));
        putstr(tmpwin, 0, buf);
    }
    return i;
}

/*
 * Magicbane's intrinsic magic is incompatible with normal
 * enchantment magic.  Thus, its effects have a negative
 * dependence on spe.  Against low mr victims, it typically
 * does "double athame" damage, 2d4.  Occasionally, it will
 * cast unbalancing magic which effectively averages out to
 * 4d4 damage (3d4 against high mr victims), for spe = 0.
 *
 * Prior to 3.4.1, the cancel (aka purge) effect always
 * included the scare effect too; now it's one or the other.
 * Likewise, the stun effect won't be combined with either
 * of those two; it will be chosen separately or possibly
 * used as a fallback when scare or cancel fails.
 *
 * [Historical note: a change to artifact_hit() for 3.4.0
 * unintentionally made all of Magicbane's special effects
 * be blocked if the defender successfully saved against a
 * stun attack.  As of 3.4.1, those effects can occur but
 * will be slightly less likely than they were in 3.3.x.]
 */
enum mb_effect_indices {
    MB_INDEX_PROBE = 0,
    MB_INDEX_STUN,
    MB_INDEX_SCARE,
    MB_INDEX_CANCEL,

    NUM_MB_INDICES
};

#define MB_MAX_DIEROLL      8   /* rolls above this aren't magical */
static const char *const mb_verb[2][NUM_MB_INDICES] = {
    { "probe", "stun",  "scare",  "cancel" },
    { "prod",  "amaze", "tickle", "purge" },
};

/* called when someone is being hit by Magicbane */
static boolean
Mb_hit(struct monst *magr, /**< attacker */
       struct monst *mdef, /**< defender */
       struct obj *mb,     /**< Magicbane */
       int *dmgptr,        /**< extra damage target will suffer */
       int dieroll,        /**< d20 that has already scored a hit */
       boolean vis,        /**< whether the action can be seen */
       char *hittee)       /**< target's name: "you" or mon_nam(mdef) */
{
    struct permonst *old_uasmon;
    const char *verb;
    boolean youattack = (magr == &youmonst),
            youdefend = (mdef == &youmonst),
            resisted = FALSE, do_stun, do_confuse, result;
    int attack_indx, scare_dieroll = MB_MAX_DIEROLL / 2;

    result = FALSE;     /* no message given yet */
    /* the most severe effects are less likely at higher enchantment */
    if (mb->spe >= 3) {
        scare_dieroll /= (1 << (mb->spe / 3));
    }
    /* if target successfully resisted the artifact damage bonus,
       reduce overall likelihood of the assorted special effects */
    if (!spec_dbon_applies) {
        dieroll += 1;
    }

    /* might stun even when attempting a more severe effect, but
       in that case it will only happen if the other effect fails;
       extra damage will apply regardless; 3.4.1: sometimes might
       just probe even when it hasn't been enchanted */
    do_stun = (max(mb->spe, 0) < rn2(spec_dbon_applies ? 11 : 7));

    /* the special effects also boost physical damage; increments are
       generally cumulative, but since the stun effect is based on a
       different criterium its damage might not be included; the base
       damage is either 1d4 (athame) or 2d4 (athame+spec_dbon) depending
       on target's resistance check against AD_STUN (handled by caller)
       [note that a successful save against AD_STUN doesn't actually
       prevent the target from ending up stunned] */
    attack_indx = MB_INDEX_PROBE;
    *dmgptr += rnd(4);     /* (2..3)d4 */
    if (do_stun) {
        attack_indx = MB_INDEX_STUN;
        *dmgptr += rnd(4); /* (3..4)d4 */
    }
    if (dieroll <= scare_dieroll) {
        attack_indx = MB_INDEX_SCARE;
        *dmgptr += rnd(4); /* (3..5)d4 */
    }
    if (dieroll <= (scare_dieroll / 2)) {
        attack_indx = MB_INDEX_CANCEL;
        *dmgptr += rnd(4); /* (4..6)d4 */
    }

    /* give the hit message prior to inflicting the effects */
    verb = mb_verb[!!Hallucination][attack_indx];
    if (youattack || youdefend || vis) {
        result = TRUE;
        pline_The("magic-absorbing blade %s %s!",
                  vtense((const char *)0, verb), hittee);
        /* assume probing has some sort of noticeable feedback
           even if it is being done by one monster to another */
        if (attack_indx == MB_INDEX_PROBE && !canspotmon(mdef)) {
            map_invisible(mdef->mx, mdef->my);
        }
    }

    /* now perform special effects */
    switch (attack_indx) {
    case MB_INDEX_CANCEL:
        old_uasmon = youmonst.data;
        /* No mdef->mcan check: even a cancelled monster can be polymorphed
         * into a golem, and the "cancel" effect acts as if some magical
         * energy remains in spellcasting defenders to be absorbed later.
         */
        if (!cancel_monst(mdef, mb, youattack, FALSE, FALSE)) {
            resisted = TRUE;
        } else {
            do_stun = FALSE;
            if (youdefend) {
                if (youmonst.data != old_uasmon) {
                    *dmgptr = 0; /* rehumanized, so no more damage */
                }
                if (u.uenmax > 0) {
                    u.uenmax--;
                    if (u.uen > 0) {
                        u.uen--;
                    }
                    flags.botl = 1;
                    You("lose magical energy!");
                }
            } else {
                if (mdef->data == &mons[PM_CLAY_GOLEM]) {
                    mdef->mhp = 1; /* cancelled clay golems will die */
                }
                if (youattack && attacktype(mdef->data, AT_MAGC)) {
                    u.uenmax++;
                    u.uen++;
                    flags.botl = 1;
                    You("absorb magical energy!");
                }
            }
        }
        break;

    case MB_INDEX_SCARE:
        if (youdefend) {
            if (Antimagic) {
                resisted = TRUE;
            } else {
                nomul(-3, "being scared stiff");
                nomovemsg = "";
                if (magr && magr == u.ustuck && sticks(youmonst.data)) {
                    set_ustuck((struct monst *) 0);
                    You("release %s!", mon_nam(magr));
                }
            }
        } else {
            if (rn2(2) && resist(mdef, WEAPON_CLASS, 0, NOTELL)) {
                resisted = TRUE;
            } else {
                monflee(mdef, 3, FALSE, (mdef->mhp > *dmgptr));
            }
        }
        if (!resisted) {
            do_stun = FALSE;
        }
        break;

    case MB_INDEX_STUN:
        do_stun = TRUE; /* (this is redundant...) */
        break;

    case MB_INDEX_PROBE:
        if (youattack && (mb->spe == 0 || !rn2(3 * abs(mb->spe)))) {
            pline_The("%s is insightful.", verb);
            /* pre-damage status */
            probe_monster(mdef);
        }
        break;
    }
    /* stun if that was selected and a worse effect didn't occur */
    if (do_stun) {
        if (youdefend) {
            make_stunned((HStun + 3), FALSE);
        } else {
            mdef->mstun = 1;
        }
        /* avoid extra stun message below if we used mb_verb["stun"] above */
        if (attack_indx == MB_INDEX_STUN) {
            do_stun = FALSE;
        }
    }
    /* lastly, all this magic can be confusing... */
    do_confuse = !rn2(12);
    if (do_confuse) {
        if (youdefend) {
            make_confused(HConfusion + 4, FALSE);
        } else {
            mdef->mconf = 1;
        }
    }

    /* now give message(s) describing side-effects */
    if (youattack || youdefend || vis) {
        (void) upstart(hittee); /* capitalize */
        if (resisted) {
            pline("%s resist%s!", hittee, youdefend ? "" : "s");
            shieldeff(youdefend ? u.ux : mdef->mx,
                      youdefend ? u.uy : mdef->my);
        }
        if ((do_stun || do_confuse) && flags.verbose) {
            char buf[BUFSZ];

            buf[0] = '\0';
            if (do_stun) {
                Strcat(buf, "stunned");
            }
            if (do_stun && do_confuse) {
                Strcat(buf, " and ");
            }
            if (do_confuse) {
                Strcat(buf, "confused");
            }
            pline("%s %s %s%c", hittee, youdefend ? "are" : "is",
                  buf, (do_stun && do_confuse) ? '!' : '.');
        }
    }

    return result;
}

/* Function used when someone attacks someone else with an artifact
 * weapon.  Only adds the special (artifact) damage, and returns a 1 if it
 * did something special (in which case the caller won't print the normal
 * hit message).  This should be called once upon every artifact attack;
 * dmgval() no longer takes artifact bonuses into account.  Possible
 * extension: change the killer so that when an orc kills you with
 * Stormbringer it's "killed by Stormbringer" instead of "killed by an orc".
 */
boolean
artifact_hit(struct monst *magr, struct monst *mdef, struct obj *otmp,
             int *dmgptr,
             int dieroll) /**< needed for Magicbane and vorpal blades */
{
    boolean youattack = (magr == &youmonst);
    boolean youdefend = (mdef == &youmonst);
    boolean vis = (!youattack && magr && cansee(magr->mx, magr->my))
                  || (!youdefend && cansee(mdef->mx, mdef->my))
                  || (youattack && u.uswallow && mdef == u.ustuck && !Blind);
    boolean realizes_damage;
    const char *wepdesc;
    static const char you[] = "you";
    char hittee[BUFSZ];
    struct artifact* atmp;

    Strcpy(hittee, youdefend ? you : mon_nam(mdef));

    /* The following takes care of most of the damage, but not all--
     * the exception being for level draining, which is specially
     * handled.  Messages are done in this function, however.
     */
    *dmgptr += spec_dbon(otmp, mdef, *dmgptr);

    if (youattack && youdefend) {
        warning("attacking yourself with weapon?");
        return FALSE;
    }

    realizes_damage = (youdefend || vis ||
                       /* feel the effect even if not seen */
                       (youattack && mdef == u.ustuck));

    /* the four basic attacks: fire, cold, shock and missiles */
    if (attacks(AD_FIRE, otmp)) {
        if (realizes_damage) {
            pline_The("fiery blade %s %s%c",
                      !spec_dbon_applies ? "hits" :
                      (mdef->data == &mons[PM_WATER_ELEMENTAL]) ?
                      "vaporizes part of" : "burns",
                      hittee, !spec_dbon_applies ? '.' : '!');
        }
        if (!rn2(4)) {
            (void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
        }
        if (!rn2(4)) {
            (void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
        }
        if (!rn2(7)) {
            (void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
        }
        /* Fire Brand is instantly fatal to Devils' Snares */
        if (mdef->data == &mons[PM_DEVIL_S_SNARE]) {
            *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
        }
        if (youdefend && Slimed) {
            burn_away_slime();
        }
        return realizes_damage;
    }
    if (attacks(AD_COLD, otmp)) {
        if (realizes_damage) {
            pline_The("ice-cold blade %s %s%c",
                      !spec_dbon_applies ? "hits" : "freezes",
                      hittee, !spec_dbon_applies ? '.' : '!');
        }
        if (!rn2(4)) {
            (void) destroy_mitem(mdef, POTION_CLASS, AD_COLD);
        }
        return realizes_damage;
    }
    if (attacks(AD_ELEC, otmp)) {
        if (realizes_damage) {
            pline_The("massive hammer hits%s %s%c",
                      !spec_dbon_applies ? "" : "!  Lightning strikes",
                      hittee, !spec_dbon_applies ? '.' : '!');
        }
        if (spec_dbon_applies) {
            wake_nearto(mdef->mx, mdef->my, 4 * 4);
        }
        if (!rn2(5)) {
            (void) destroy_mitem(mdef, RING_CLASS, AD_ELEC);
        }
        if (!rn2(5)) {
            (void) destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
        }
        return realizes_damage;
    }
    if (attacks(AD_MAGM, otmp)) {
        if (realizes_damage) {
            pline_The("imaginary widget hits%s %s%c",
                      !spec_dbon_applies ? "" :
                      "!  A hail of magic missiles strikes",
                      hittee, !spec_dbon_applies ? '.' : '!');
        }
        return realizes_damage;
    }

    if (attacks(AD_STUN, otmp) && dieroll <= MB_MAX_DIEROLL) {
        /* Magicbane's special attacks (possibly modifies hittee[]) */
        return Mb_hit(magr, mdef, otmp, dmgptr, dieroll, vis, hittee);
    }

    if (otmp->oartifact != ART_THIEFBANE || !youdefend) {
        if (!spec_dbon_applies) {
            /* since damage bonus didn't apply, nothing more to do;
               no further attacks have side-effects on inventory */
            return FALSE;
        }
    }

    /* Are we about to do something special vs. a monster type? */
    atmp = &artilist[otmp->oartifact];
    if (otmp->oclass == GEM_CLASS && uwep && uwep->oartifact == ART_GIANTSLAYER) {
        atmp = &artilist[ART_GIANTSLAYER];
        otmp->oartifact = ART_GIANTSLAYER;  /* really miserable hack */
    }
    if (atmp->spfx & (SPFX_DFLAG2 | SPFX_DCLAS)) {
        boolean instakill = dieroll < 5; /* 20% chance of instakill for some artifacts */
        switch (otmp->oartifact) {
        case ART_DRAGONBANE:
            if (youattack && instakill) {
                You("pierce the heart of %s!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && instakill) {
                pline("The deadly spear pierces your heart!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp + FATAL_DAMAGE_MODIFIER));
            } else {
                return FALSE;
            }
            return TRUE;
        case ART_WEREBANE:
            if (youattack && instakill) {
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && instakill) {
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp + FATAL_DAMAGE_MODIFIER));
            } else {
                return FALSE;
            }
            return TRUE;
        case ART_GIANTSLAYER:
            if (otmp->oclass == GEM_CLASS) {         /* second part of miserable hack */
                otmp->oartifact = 0L;
            }
            if (youattack && instakill) {
                You("strike %s in the forehead!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && instakill) {
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp + FATAL_DAMAGE_MODIFIER));
                You("are hit in the center of your forehead!");
            }
            return TRUE;
        case ART_OGRESMASHER:
            if (youattack && instakill) {
                You("crush the skull of %s!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && instakill) {
                pline("The monstrous hammer crushes your skull!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp + FATAL_DAMAGE_MODIFIER));
            } else {
                return FALSE;
            }
            return TRUE;
        case ART_TROLLSBANE:
            if (youattack && instakill) {
                pline("As you strike %s, it bursts into flame!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && instakill) {
                You("burst into flame as you are hit!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp + FATAL_DAMAGE_MODIFIER));
            } else {
                return FALSE;
            }
            return TRUE;
        /* below this we don't get any additional handling, so drop through
         * just listed here for potential future reference */
        case ART_DEMONBANE:
        case ART_SUNSWORD:
        case ART_ORCRIST:
        case ART_STING:
        default:
            break;
        }

    }

    if (otmp->oartifact == ART_GRIMTOOTH) {
        otmp->dknown = TRUE;
        if (youdefend) {
            pline_The("jagged blade %s you%s", Poison_resistance ? "hits" : "poisons",
                      Poison_resistance ? "." : "!");
            if (Poison_resistance) {
                return TRUE;
            }
        } else {
            /* Grimtooth is the only non-projectile poisoned weapon
               yet implemented, so place alignment penalties here */
            if (youattack) {
                if (Role_if(PM_SAMURAI) && u.ualign.type == A_LAWFUL) {
                    You("dishonorably use a poisoned weapon!");
                    adjalign(-sgn(u.ualign.type));
                } else if ((u.ualign.type == A_LAWFUL) && (u.ualign.record > -10)) {
                    You_feel("like an evil coward for using a poisoned weapon.");
                    adjalign(-1);
                }
            }
            pline_The("jagged blade %s %s%s", resists_poison(mdef) ? "hits" : "poisons",
                      mon_nam(mdef), resists_poison(mdef) ? "." : "!");
            if (resists_poison(mdef)) {
                return TRUE;
            }
        }
        switch (rnd(10)) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            *dmgptr += d(1, 6);
            break;
        case 10:
            if (!youdefend) {
                pline_The("poison was deadly...");
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else {
                pline_The("poison was extremely toxic!");
                u.uhpmax -= d(4, 6) / 2;
            }
            break;
        }
        return TRUE;
    }

    /* We really want "on a natural 20" but Nethack does it in */
    /* reverse from AD&D. */
    if (spec_ability(otmp, SPFX_BEHEAD)) {
        if (otmp->oartifact == ART_TSURUGI_OF_MURAMASA && dieroll == 1) {
            wepdesc = "The razor-sharp blade";
            /* not really beheading, but so close, why add another SPFX */
            if (youattack && u.uswallow && mdef == u.ustuck) {
                You("slice %s wide open!", mon_nam(mdef));
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                return TRUE;
            }
            if (!youdefend) {
                /* allow normal cutworm() call to add extra damage */
                if (notonhead) {
                    return FALSE;
                }

                if (bigmonst(mdef->data)) {
                    if (youattack) {
                        You("slice deeply into %s!",
                            mon_nam(mdef));
                    } else if (vis) {
                        pline("%s cuts deeply into %s!",
                              Monnam(magr), hittee);
                    }
                    *dmgptr *= 2;
                    return TRUE;
                }
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                pline("%s cuts %s in half!", wepdesc, mon_nam(mdef));
                otmp->dknown = TRUE;
                return TRUE;
            } else {
                if (bigmonst(youmonst.data)) {
                    pline("%s cuts deeply into you!",
                          magr ? Monnam(magr) : wepdesc);
                    *dmgptr *= 2;
                    return TRUE;
                }

                /* Players with negative AC's take less damage instead
                 * of just not getting hit.  We must add a large enough
                 * value to the damage so that this reduction in
                 * damage does not prevent death.
                 */
                *dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
                pline("%s cuts you in half!", wepdesc);
                otmp->dknown = TRUE;
                return TRUE;
            }
        } else if ((otmp->oartifact == ART_VORPAL_BLADE &&
                    (dieroll == 1 || mdef->data->mlet == S_JABBERWOCK)) ||
                   (otmp->oartifact == ART_THIEFBANE && dieroll < 3)) {
            static const char * const behead_msg[2] = {
                "%s beheads %s!",
                "%s decapitates %s!"
            };

            if (youattack && u.uswallow && mdef == u.ustuck) {
                return FALSE;
            }
            wepdesc = artilist[otmp->oartifact].name;
            if (!youdefend) {
                if (!has_head(mdef->data) || notonhead || u.uswallow) {
                    if (youattack) {
                        pline("Somehow, you miss %s wildly.",
                              mon_nam(mdef));
                    } else if (vis) {
                        pline("Somehow, %s misses wildly.",
                              mon_nam(magr));
                    }
                    *dmgptr = 0;
                    return ((boolean)(youattack || vis));
                }
                if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
                    pline("%s slices through %s %s.", wepdesc,
                          s_suffix(mon_nam(mdef)),
                          mbodypart(mdef, NECK));
                    return TRUE;
                }
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                pline(behead_msg[rn2(SIZE(behead_msg))],
                      wepdesc, mon_nam(mdef));
                otmp->dknown = TRUE;
                return TRUE;
            } else {
                if (!has_head(youmonst.data)) {
                    pline("Somehow, %s misses you wildly.",
                          magr ? mon_nam(magr) : wepdesc);
                    *dmgptr = 0;
                    return TRUE;
                }
                if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
                    pline("%s slices through your %s.",
                          wepdesc, body_part(NECK));
                    return TRUE;
                }
                *dmgptr = 2 * (Upolyd ? u.mh : u.uhp)
                          + FATAL_DAMAGE_MODIFIER;
                pline(behead_msg[rn2(SIZE(behead_msg))],
                      wepdesc, "you");
                otmp->dknown = TRUE;
                /* Should amulets fall off? */
                return TRUE;
            }
        }
    }

    if (spec_ability(otmp, SPFX_DRLI)) {
        /* some non-living creatures (golems, vortices) are
           vulnerable to life drain effects */
        const char *life = nonliving(mdef->data) ? "animating force" : "life";

        if (!youdefend) {
            if (resists_drli(mdef)) {
                return FALSE;
            } else {
                if (vis) {
                    if (otmp->oartifact == ART_STORMBRINGER) {
                        pline_The("%s blade draws the %s from %s!",
                                hcolor(NH_BLACK),
                                life,
                                mon_nam(mdef));
                    } else {
                        pline("%s draws the %s from %s!",
                            The(distant_name(otmp, xname)),
                            life,
                            mon_nam(mdef));
                    }
                }
                if (mdef->m_lev == 0) {
                    *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                } else {
                    int drain = monhp_per_lvl(mdef);

                    *dmgptr += drain;
                    mdef->mhpmax -= drain;
                    mdef->m_lev--;
                    drain /= 2;
                    if (drain) {
                        healup(drain, 0, FALSE, FALSE);
                    }
                }
            }
            return vis;

        } else if (!Drain_resistance) {
            int oldhpmax = u.uhpmax;

            if (Blind) {
                You_feel("an %s drain your %s!",
                         otmp->oartifact == ART_STORMBRINGER ?  "unholy blade" : "object",
                         life);
            } else if (otmp->oartifact == ART_STORMBRINGER) {
                pline_The("%s blade drains your %s!",
                          hcolor(NH_BLACK),
                          life);
            } else {
                pline("%s drains your %s!",
                      The(distant_name(otmp, xname)),
                      life);
            }
            losexp("life drainage");
            if (magr && magr->mhp < magr->mhpmax) {
                magr->mhp += (oldhpmax - u.uhpmax)/2;
                if (magr->mhp > magr->mhpmax) {
                    magr->mhp = magr->mhpmax;
                }
            }
            return TRUE;
        }
    }
    /* WAC -- 1/6 chance of cancellation with foobane weapons */
#ifdef BLACKMARKET
    if (otmp->oartifact == ART_ORCRIST ||
        otmp->oartifact == ART_DEMONBANE ||
        otmp->oartifact == ART_THIEFBANE) {
        if (!mdef->mcan && dieroll < 4) {
            if (realizes_damage) {
                pline("%s %s!", The(distant_name(otmp, xname)), Blind ?
                      "roars deafeningly" : "shines brilliantly");
                pline("It strikes %s!", hittee);
            }
            cancel_monst(mdef, otmp, youattack, TRUE, magr == mdef);
            return TRUE;
        }
    }
#endif
    return FALSE;
}

static NEARDATA const char recharge_type[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
static NEARDATA const char invoke_types[] = { ALL_CLASSES, 0 };
/* #invoke: an "ugly check" filters out most objects */

/** the #invoke command */
int
doinvoke(void)
{
    struct obj *obj;

    obj = getobj(invoke_types, "invoke");
    if (!obj) {
        return 0;
    }
    if (!retouch_object(&obj, FALSE)) {
        return 1;
    }
    return arti_invoke(obj);
}

static boolean
find_portal_location(coordxy *portal_x, coordxy *portal_y)
{
    /* collect all suitable locations for magic portal placement */
    coordxy x, y;
    coord locations[9] = { 0 };
    int i = 0;
    for (x = u.ux - 1; x <= u.ux + 1; x++) {
        for (y = u.uy - 1; y <= u.uy + 1; y++) {
            if (!isok(x, y) || u_at(x, y)) {
                continue;
            }
            if (levl[x][y].typ != ROOM && levl[x][y].typ != CORR) {
                continue;
            }
            if (t_at(x, y)) {
                continue;
            }
            locations[i++] = (coord) { x, y };
        }
    }

    /* no suitable locations */
    if (i == 0) {
        return FALSE;
    }

    /* choose a random location */
    int idx = rn2(i);
    *portal_x = locations[idx].x;
    *portal_y = locations[idx].y;

    return TRUE;
}

static int
arti_invoke(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);

    if (!oart || !oart->inv_prop) {
        if (obj->otyp == CRYSTAL_BALL) {
            use_crystal_ball(&obj);
#ifdef ASTRAL_ESCAPE
        } else if (obj->otyp == AMULET_OF_YENDOR ||
                obj->otyp == FAKE_AMULET_OF_YENDOR) {
            /* The Amulet is not technically an artifact
             * in the usual sense... */
            return invoke_amulet(obj);
#endif
        } else {
            pline("%s", nothing_happens);
        }
        return 1;
    }

    if (oart->inv_prop > LAST_PROP) {
        /* It's a special power, not "just" a property */
        if (obj->age > monstermoves) {
            /* the artifact is tired :-) */
            You_feel("that %s %s ignoring you.",
                     the(xname(obj)), otense(obj, "are"));
            /* and just got more so; patience is essential... */
            obj->age += (long) d(3, 10);
            return 1;
        }
        obj->age = monstermoves + rnz(100);

        switch (oart->inv_prop) {
        case TAMING: {
            struct obj pseudo;

            pseudo = zeroobj; /* neither cursed nor blessed */
            pseudo.otyp = SCR_TAMING;
            (void) seffects(&pseudo);
            break;
        }
        case HEALING: {
            int healamt = (u.uhpmax + 1 - u.uhp) / 2;
            long creamed = (long)u.ucreamed;

            if (Upolyd) {
                healamt = (u.mhmax + 1 - u.mh) / 2;
            }
            if (healamt || Sick || Slimed || Blinded > creamed) {
                You_feel("better.");
            } else {
                goto nothing_special;
            }
            if (healamt > 0) {
                if (Upolyd) {
                    u.mh += healamt;
                } else {
                    u.uhp += healamt;
                }
            }
            if (Sick) {
                make_sick(0L, (char *)0, FALSE, SICK_ALL);
            }
            if (Slimed) {
                Slimed = 0L;
            }
            if (Blinded > creamed) {
                make_blinded(creamed, FALSE);
            }
            flags.botl = 1;
            break;
        }
        case ENERGY_BOOST: {
            int epboost = (u.uenmax + 1 - u.uen) / 2;
            if (epboost > 120) {
                epboost = 120; /* arbitrary */
            } else if (epboost < 12) {
                epboost = u.uenmax - u.uen;
            }
            if (epboost) {
                You_feel("re-energized.");
                u.uen += epboost;
                flags.botl = 1;
            } else {
                goto nothing_special;
            }
            break;
        }
        case UNTRAP: {
            if (!untrap(TRUE)) {
                obj->age = 0; /* don't charge for changing their mind */
                return 0;
            }
            break;
        }
        case CHARGE_OBJ: {
            struct obj *otmp = getobj(recharge_type, "charge");
            boolean b_effect;

            if (!otmp) {
                obj->age = 0;
                return 0;
            }
            b_effect = obj->blessed &&
                       (Role_switch == oart->role || !oart->role);
            recharge(otmp, b_effect ? 1 : obj->cursed ? -1 : 0);
            update_inventory();
            break;
        }
        case LEV_TELE:
            level_tele();
            break;
        case CREATE_PORTAL: {
            int i, num_ok_dungeons, last_ok_dungeon = 0;
            d_level newlev;
            extern int n_dgns; /* from dungeon.c */
            anything any;

            any.a_void = 0; /* set all bits to zero */
 #ifdef BLACKMARKET
            if (Is_blackmarket(&u.uz) && *u.ushops) {
                You("feel very disoriented for a moment.");
                break;
            }
 #endif
            winid tmpwin = create_nhwindow(NHW_MENU);
            start_menu(tmpwin);
            /* use index+1 (cant use 0) as identifier */
            for (i = num_ok_dungeons = 0; i < n_dgns; i++) {
                if (!dungeons[i].dunlev_ureached) {
                    continue;
                }
                any.a_int = i+1;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                         dungeons[i].dname, MENU_UNSELECTED);
                num_ok_dungeons++;
                last_ok_dungeon = i;
            }
            end_menu(tmpwin, "Open a portal to which dungeon?");
            if (num_ok_dungeons > 1) {
                /* more than one entry; display menu for choices */
                menu_item *selected;
                int n;

                n = select_menu(tmpwin, PICK_ONE, &selected);
                if (n <= 0) {
                    destroy_nhwindow(tmpwin);
                    goto nothing_special;
                }
                i = selected[0].item.a_int - 1;
                free((genericptr_t)selected);
            } else {
                i = last_ok_dungeon; /* also first & only OK dungeon */
            }
            destroy_nhwindow(tmpwin);

            /*
             * i is now index into dungeon structure for the new dungeon.
             * Find the closest level in the given dungeon, open
             * a use-once portal to that dungeon and go there.
             * The closest level is either the entry or dunlev_ureached.
             */
            newlev.dnum = i;
            if (dungeons[i].depth_start >= depth(&u.uz)) {
                newlev.dlevel = dungeons[i].entry_lev;
            } else {
                newlev.dlevel = dungeons[i].dunlev_ureached;
            }
            if (u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev) ||
               newlev.dnum == u.uz.dnum) {
                You_feel("very disoriented for a moment.");
            } else {
                if (!Blind) {
                    You("are surrounded by a shimmering sphere!");
                } else {
                    You_feel("weightless for a moment.");
                }
                goto_level(&newlev, FALSE, FALSE, FALSE);
            }
            break;
        }
        case ENLIGHTENING:
            enlightenment(0, TRUE);
            break;
        case CREATE_AMMO: {
            struct obj *otmp = mksobj(ARROW, TRUE, FALSE);

            if (!otmp) {
                goto nothing_special;
            }
            otmp->blessed = obj->blessed;
            otmp->cursed = obj->cursed;
            otmp->bknown = obj->bknown;
            if (obj->blessed) {
                if (otmp->spe < 0) {
                    otmp->spe = 0;
                }
                otmp->quan += rnd(10);
            } else if (obj->cursed) {
                if (otmp->spe > 0) {
                    otmp->spe = 0;
                }
            } else {
                otmp->quan += rnd(5);
            }
            otmp->owt = weight(otmp);
            otmp = hold_another_object(otmp, "Suddenly %s out.",
                                       aobjnam(otmp, "fall"), (const char *)0);
            break;
        }

        case SMOKE_CLOUD: {
            coord cc;

            cc.x = u.ux;
            cc.y = u.uy;
            /* Cause trouble if cursed or player is wrong role */
            if (!obj->cursed && (Role_switch == oart->role || !oart->role)) {
                You("may summon a stinking cloud.");
                pline("Where do you want to center the cloud?");
                if (getpos(&cc, TRUE, "the desired position") < 0) {
                    pline("%s", Never_mind);
                    obj->age = 0;
                    return 0;
                }
                if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 64) {
                    You("smell rotten eggs.");
                    return 0;
                }
            }
            pline("A cloud of toxic smoke pours out!");
            (void) create_gas_cloud(cc.x, cc.y, 3+bcsign(obj),
                                    8+4*bcsign(obj), rn1(3, 4));
            break;
        }

        case PHASING: /* Walk through walls and stone like a xorn */
            if (Passes_walls) {
                goto nothing_special;
            }
            if (oart == &artilist[ART_IRON_BALL_OF_LIBERATION]) {
                if (Punished && (obj != uball)) {
                    unpunish(); /* Remove a mundane heavy iron ball */
                }

                if (!Punished) {
                    setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
                    setworn(obj, W_BALL);
                    uball->spe = 1;
                    if (!u.uswallow) {
                        placebc();
                        if (Blind) {
                            set_bc(1); /* set up ball and chain variables */
                        }
                        newsym(u.ux, u.uy); /* see ball&chain if can't see self */
                    }
                    Your("%s chains itself to you!", xname(obj));
                }
            }
            if (!Hallucination) {
                Your("body begins to feel less solid.");
            } else {
                You_feel("one with the spirit world.");
            }
            incr_itimeout(&Phasing, (50 + rnd(100)));
            obj->age += Phasing; /* Time begins after phasing ends */
            break;

        case BIDIRECTIONAL_PORTAL:
            if (noteleport_level(&youmonst)) {
                /* no portal on noteleport levels */
                pline("%s", nothing_happens);
                break;
            }

            coordxy portal_x, portal_y;
            if (!find_portal_location(&portal_x, &portal_y)) {
                /* if no location found, get hot and player has to drop it.*/
                pline("%s %s hot!", Ysimple_name2(obj), otense(obj, "grow"));
                You("drop %s.", ysimple_name(obj));
                obj_extract_self(obj);
                dropz(obj, TRUE);
            } else {
                if (Blind) {
                    pline("%s starts throbbing!", Ysimple_name2(obj));
                } else {
                    pline("%s starts pulsating and transforms into a magic portal!",
                          Ysimple_name2(obj));
                }

                /* set portal info */
                int idx = obj->oartifact - ART_EARTHSTONE;
                flags.portal_stone_location[idx].dnum   = u.uz.dnum;
                flags.portal_stone_location[idx].dlevel = u.uz.dlevel;
#if 0
                /* extract portal stone and bury it under the portal */
                obj_extract_self(obj);
                obj->ox = portal_x; obj->oy = portal_y;
                add_to_buried(obj);
#else
                useup(obj);
#endif
                /* create portal */
                mkportal(portal_x, portal_y, -1, -1);
                struct trap *portal = t_at(portal_x, portal_y);
                seetrap(portal);
            }
        }
    } else {
        long eprop = (u.uprops[oart->inv_prop].extrinsic ^= W_ARTI),
             iprop = u.uprops[oart->inv_prop].intrinsic;
        boolean on = (eprop & W_ARTI) != 0; /* true if invoked prop just set */

        if (on && obj->age > monstermoves) {
            /* the artifact is tired :-) */
            u.uprops[oart->inv_prop].extrinsic ^= W_ARTI;
            You_feel("that %s %s ignoring you.",
                     the(xname(obj)), otense(obj, "are"));
            /* can't just keep repeatedly trying */
            obj->age += (long) d(3, 10);
            return 1;
        } else if (!on) {
            /* when turning off property, determine downtime */
            /* arbitrary for now until we can tune this -dlc */
            obj->age = monstermoves + rnz(100);
        }

        if ((eprop & ~W_ARTI) || iprop) {
nothing_special:
            /* you had the property from some other source too */
            if (carried(obj)) {
                You_feel("a surge of power, but nothing seems to happen.");
            }
            return 1;
        }
        switch (oart->inv_prop) {
        case CONFLICT:
            if (on) {
                You_feel("like a rabble-rouser.");
            } else {
                You_feel("the tension decrease around you.");
            }
            break;
        case LEVITATION:
            if (on) {
                float_up();
                spoteffects(FALSE);
            } else {
                (void) float_down(I_SPECIAL|TIMEOUT, W_ARTI);
            }
            break;
        }
    }

    return 1;
}


/* WAC return TRUE if artifact is always lit */
boolean
artifact_light(struct obj *obj)
{
    return (get_artifact(obj) && obj->oartifact == ART_SUNSWORD);
}

/* KMH -- Talking artifacts are finally implemented */
void
arti_speak(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);
    const char *line;
    char buf[BUFSZ];

    /* Is this a speaking artifact? */
    if (!oart || !(oart->spfx & SPFX_SPEAK)) {
        return;
    }

    line = getrumor(bcsign(obj), buf, TRUE);
    if (!*line) {
        line = "UnNetHack rumors file closed for renovation.";
    }
    pline("%s:", Tobjnam(obj, "whisper"));
    verbalize("%s", line);
    return;
}

boolean
artifact_has_invprop(struct obj *otmp, uchar inv_prop)
{
    const struct artifact *arti = get_artifact(otmp);

    return (boolean)(arti && (arti->inv_prop == inv_prop));
}

/* Return the price sold to the hero of a given artifact or unique item */
long
arti_cost(struct obj *otmp)
{
    if (!otmp->oartifact) {
        return ((long)objects[otmp->otyp].oc_cost);
    } else if (artilist[(int) otmp->oartifact].cost) {
        return (artilist[(int) otmp->oartifact].cost);
    } else {
        return (100L * (long)objects[otmp->otyp].oc_cost);
    }
}

static uchar
abil_to_adtyp(long int *abil)
{
    struct abil2adtyp_tag {
        long *abil;
        uchar adtyp;
    } abil2adtyp[] = {
        { &EFire_resistance, AD_FIRE },
        { &ECold_resistance, AD_COLD },
        { &EShock_resistance, AD_ELEC },
        { &EAntimagic, AD_MAGM },
        { &EDisint_resistance, AD_DISN },
        { &EPoison_resistance, AD_DRST },
        { &EDrain_resistance, AD_DRLI },
    };
    int k;

    for (k = 0; k < SIZE(abil2adtyp); k++) {
        if (abil2adtyp[k].abil == abil) {
            return abil2adtyp[k].adtyp;
        }
    }
    return 0;
}

static unsigned long
abil_to_spfx(long int *abil)
{
    static const struct abil2spfx_tag {
        long *abil;
        unsigned long spfx;
    } abil2spfx[] = {
        { &ESearching, SPFX_SEARCH },
        { &EHalluc_resistance, SPFX_HALRES },
        { &ETelepat, SPFX_ESP },
        { &ERegeneration, SPFX_REGEN },
        { &ETeleport_control, SPFX_TCTRL },
        { &EWarn_of_mon, SPFX_WARN },
        { &EWarning, SPFX_WARN },
        { &EEnergy_regeneration, SPFX_EREGEN },
        { &EHalf_spell_damage, SPFX_HSPDAM },
        { &EHalf_physical_damage, SPFX_HPHDAM },
        { &EReflecting, SPFX_REFLECT },
    };
    int k;

    for (k = 0; k < SIZE(abil2spfx); k++) {
        if (abil2spfx[k].abil == abil) {
            return abil2spfx[k].spfx;
        }
    }
    return 0L;
}

/*
 * Return the first item that is conveying a particular intrinsic.
 */
struct obj *
what_gives(long int *abil)
{
    struct obj *obj;
    uchar dtyp;
    unsigned long spfx;
    long wornbits;
    long wornmask = (W_ARM | W_ARMC | W_ARMH | W_ARMS |
                     W_ARMG | W_ARMF | W_ARMU |
                     W_AMUL | W_RINGL | W_RINGR | W_TOOL |
                     W_ART | W_ARTI);

    if (u.twoweap) {
        wornmask |= W_SWAPWEP;
    }
    dtyp = abil_to_adtyp(abil);
    spfx = abil_to_spfx(abil);
    wornbits = (wornmask & *abil);

    for (obj = invent; obj; obj = obj->nobj) {
        if (obj->oartifact && (abil != &EWarn_of_mon)) {
            const struct artifact *art = get_artifact(obj);

            if (art) {
                if (dtyp) {
                    if ((art->cary.adtyp == dtyp) /* carried */ ||
                        (defends(dtyp, obj) &&
                         (obj->owornmask & ~(W_ART | W_ARTI)))) /* defends while worn */ {
                        return obj;
                    }
                }
                if (spfx) {
                    /* property conferred when carried */
                    if ((art->cspfx & spfx) == spfx) {
                        return obj;
                    }
                    /* property conferred when wielded or worn */
                    if ((art->spfx & spfx) == spfx && obj->owornmask) {
                        return obj;
                    }
                }
            }
        } else {
            if (wornbits && wornbits == (wornmask & obj->owornmask)) {
                return obj;
            }
        }
    }
    return (struct obj *) 0;
}

boolean
MATCH_WARN_OF_MON(struct monst *mon)
{
    /* warned of S_MONSTER? */
    if (uwep && uwep->oartifact) {
        const struct artifact *arti = get_artifact(uwep);
        if (arti->spfx & SPFX_WARN_S &&
            arti->mtype && arti->mtype == (unsigned)mon->data->mlet) {
            return TRUE;
        }
    }

    return (Warn_of_mon && flags.warntype &&
            (flags.warntype & (mon)->data->mflags2));
}

/**
 * Returns the plural name of the monster the player is warned
 * about with SPFX_WARN_S */
const char *
get_warned_of_monster(struct obj *otmp)
{
    if (otmp && otmp->oartifact) {
        const struct artifact *arti = get_artifact(otmp);
        if (arti->spfx & SPFX_WARN_S && arti->mtype) {
            switch (arti->mtype) {
            case S_TROLL: return "trolls"; break;
            case S_DRAGON: return "dragons"; break;
            case S_OGRE: return "ogres"; break;
            case S_JABBERWOCK: return "jabberwocks"; break;
            default: return something; break;
            }
        }
    }

    return NULL;
}

const char *
glow_color(int arti_indx)
{
    int colornum = artilist[arti_indx].acolor;
    const char *colorstr = clr2colorname(colornum);

    return hcolor(colorstr);
}

/* glow verb; [0] holds the value used when blind */
static const char *glow_verbs[] = {
    "quiver", "flicker", "glimmer", "gleam"
};

/* relative strength that Sting is glowing (0..3), to select verb */
static int
glow_strength(int count)
{
    /* glow strength should also be proportional to proximity and
       probably difficulty, but we don't have that information and
       gathering it is more trouble than this would be worth */
    return (count > 12) ? 3 : (count > 4) ? 2 : (count > 0);
}

const char *
glow_verb(int count, boolean ingsfx)
           /* 0 means blind rather than no applicable creatures */

{
    static char resbuf[20];

    Strcpy(resbuf, glow_verbs[glow_strength(count)]);
    /* ing_suffix() will double the last consonant for all the words
       we're using and none of them should have that, so bypass it */
    if (ingsfx) {
        Strcat(resbuf, "ing");
    }
    return resbuf;
}

/* use for warning "glow" for Sting, Orcrist, and Grimtooth */
void
Sting_effects(int orc_count) /**< new count (warn_obj_cnt is old count); -1 is a flag value */
{
    if (uwep && (uwep->oartifact == ART_STING ||
                 uwep->oartifact == ART_ORCRIST ||
                 uwep->oartifact == ART_GRIMTOOTH)) {
        int oldstr = glow_strength(warn_obj_cnt);
        int newstr = glow_strength(orc_count);

        if (orc_count == -1 && warn_obj_cnt > 0) {
            /* -1 means that blindness has just been toggled; give a
               'continue' message that eventual 'stop' message will match */
            pline("%s is %s.", bare_artifactname(uwep), glow_verb(Blind ? 0 : warn_obj_cnt, TRUE));
        } else if (newstr > 0 && newstr != oldstr) {
            /* 'start' message */
            if (!Blind) {
                pline("%s %s %s%c", bare_artifactname(uwep),
                      otense(uwep, glow_verb(orc_count, FALSE)),
                      glow_color(uwep->oartifact),
                      (newstr > oldstr) ? '!' : '.');
            } else if (oldstr == 0) /* quivers */ {
                pline("%s %s slightly.", bare_artifactname(uwep), otense(uwep, glow_verb(0, FALSE)));
            }
        } else if (orc_count == 0 && warn_obj_cnt > 0) {
            /* 'stop' message */
            pline("%s stops %s.", bare_artifactname(uwep), glow_verb(Blind ? 0 : warn_obj_cnt, TRUE));
        }
    }
}

/* called when hero is wielding/applying/invoking a carried item, or
   after undergoing a transformation (alignment change, lycanthropy,
   polymorph) which might affect item access */
int
retouch_object(
    struct obj **objp, /**< might be destroyed or unintentionally dropped */
    boolean loseit) /**< whether to drop it if hero can longer touch it */
{
    struct obj *obj = *objp;

    if (touch_artifact(obj, &youmonst)) {
        char buf[BUFSZ];
        int dmg = 0, tmp;
#if 0
        boolean ag = (objects[obj->otyp].oc_material == SILVER && Hate_silver);
#else
        boolean ag = FALSE;
#endif
        boolean bane = bane_applies(get_artifact(obj), &youmonst);

        /* nothing else to do if hero can successfully handle this object */
        if (!ag && !bane) {
            return 1;
        }

        /* hero can't handle this object, but didn't get touch_artifact()'s
           "<obj> evades your grasp|control" message; give an alternate one */
        You_cant("handle %s%s!", yname(obj), obj->owornmask ? " anymore" : "");
        /* also inflict damage unless touch_artifact() already did so */
        if (!touch_blasted) {
            /* damage is somewhat arbitrary; half the usual 1d20 physical
               for silver, 1d10 magical for <foo>bane, potentially both */
            if (ag) {
                tmp = rnd(10), dmg += Maybe_Half_Phys(tmp);
            }
            if (bane) {
                dmg += rnd(10);
            }
            Sprintf(buf, "handling %s", killer_xname(obj));
            losehp(dmg, buf, KILLED_BY);
            exercise(A_CON, FALSE);
        }
    }

    /* removing a worn item might result in loss of levitation,
       dropping the hero onto a polymorph trap or into water or
       lava and potentially dropping or destroying the item */
    if (obj->owornmask) {
        struct obj *otmp;

        remove_worn_item(obj, FALSE);
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp == obj) {
                break;
            }
        }
        if (!otmp) {
            *objp = obj = 0;
        }
    }

    /* if we still have it and caller wants us to drop it, do so now */
    if (loseit && obj) {
        if (Levitation) {
            freeinv(obj);
            hitfloor(obj, TRUE);
        } else {
            /* dropx gives a message iff item lands on an altar */
            if (!IS_ALTAR(levl[u.ux][u.uy].typ)) {
                pline("%s to the %s.", Tobjnam(obj, "fall"), surface(u.ux, u.uy));
            }
            dropx(obj);
        }
        *objp = obj = 0; /* no longer in inventory */
    }
    return 0;
}

/* an item which is worn/wielded or an artifact which conveys
   something via being carried or which has an #invoke effect
   currently in operation undergoes a touch test; if it fails,
   it will be unworn/unwielded and revoked but not dropped */
static boolean
untouchable(struct obj *obj, boolean drop_untouchable)
{
    struct artifact *art;
    boolean beingworn, carryeffect, invoked;
    long wearmask = ~(W_QUIVER | (u.twoweap ? 0L : W_SWAPWEP) | W_BALL);

    beingworn = ((obj->owornmask & wearmask) ||
                 /* some items in use don't have any wornmask setting */
                 (obj->oclass == TOOL_CLASS &&
                  (obj->lamplit ||
                  (obj->otyp == LEASH && obj->leashmon) ||
                  (Is_container(obj) && Has_contents(obj)))));

    if ((art = get_artifact(obj)) != 0) {
        carryeffect = (art->cary.adtyp || art->cspfx);
        invoked = (art->inv_prop > 0 &&
                   art->inv_prop <= LAST_PROP &&
                   (u.uprops[art->inv_prop].extrinsic & W_ARTI) != 0L);
    } else {
        carryeffect = invoked = FALSE;
    }

    if (beingworn || carryeffect || invoked) {
        if (!retouch_object(&obj, drop_untouchable)) {
            /* "<artifact> is beyond your control" or "you can't handle
               <object>" has been given and it is now unworn/unwielded
               and possibly dropped (depending upon caller); if dropped,
               carried effect was turned off, else we leave that alone;
               we turn off invocation property here if still carried */
            if (invoked && obj) {
                arti_invoke(obj); /* reverse #invoke */
            }
            return TRUE;
        }
    }
    return FALSE;
}

/* check all items currently in use (mostly worn) for touchability */
void
retouch_equipment(int dropflag) /**< 0==don't drop, 1==drop all, 2==drop weapon */
{
    static int nesting = 0; /* recursion control */
    struct obj *obj;
    boolean dropit, had_gloves = (uarmg != 0);
    int had_rings = (!!uleft + !!uright);

    /*
     * We can potentially be called recursively if losing/unwearing
     * an item causes worn helm of opposite alignment to come off or
     * be destroyed.
     *
     * BUG: if the initial call was due to putting on a helm of
     * opposite alignment and it does come off to trigger recursion,
     * after the inner call executes, the outer call will finish
     * using the non-helm alignment rather than the helm alignment
     * which triggered this in the first place.
     */
    if (!nesting++) {
        clear_bypasses(); /* init upon initial entry */
    }

    dropit = (dropflag > 0); /* drop all or drop weapon */
    /* check secondary weapon first, before possibly unwielding primary */
    if (u.twoweap) {
        bypass_obj(uswapwep); /* so loop below won't process it again */
        (void) untouchable(uswapwep, dropit);
    }
    /* check primary weapon next so that they're handled together */
    if (uwep) {
        bypass_obj(uwep); /* so loop below won't process it again */
        (void) untouchable(uwep, dropit);
    }

    /* in case someone is daft enough to add artifact or silver saddle */
    if (u.usteed && (obj = which_armor(u.usteed, W_SADDLE)) != 0) {
        /* untouchable() calls retouch_object() which expects an object in
           hero's inventory, but remove_worn_item() will be harmless for
           saddle and we're suppressing drop, so this works as intended */
        if (untouchable(obj, FALSE)) {
            dismount_steed(DISMOUNT_THROWN);
        }
    }
    /*
     * TODO?  Force off gloves if either or both rings are going to
     * become unworn; force off cloak [suit] before suit [shirt].
     * The torso handling is hypothetical; the case for gloves is
     * not, due to the possibility of unwearing silver rings.
     */

    dropit = (dropflag == 1); /* all untouchable items */
    /* loss of levitation (silver ring, or Heart of Ahriman invocation)
       might cause hero to lose inventory items (by dropping into lava,
       for instance), so inventory traversal needs to rescan the whole
       invent chain each time it moves on to another object; we use bypass
       handling to keep track of which items have already been processed */
    while ((obj = nxt_unbypassed_obj(invent)) != 0) {
        (void) untouchable(obj, dropit);
    }

    if (had_rings != (!!uleft + !!uright) && uarmg && uarmg->cursed) {
        uncurse(uarmg); /* temporary? hack for ring removal plausibility */
    }
    if (had_gloves && !uarmg) {
        selftouch("After losing your gloves, you");
    }

    if (!--nesting) {
        clear_bypasses(); /* reset upon final exit */
    }
}

static int mkot_trap_warn_count = 0;

static int
count_surround_traps(coordxy x, coordxy y)
{
    struct rm *levp;
    struct obj *otmp;
    struct trap *ttmp;
    int dx, dy, glyph, ret = 0;

    for (dx = x - 1; dx < x + 2; ++dx) {
        for (dy = y - 1; dy < y + 2; ++dy) {
            if (!isok(dx, dy)) {
                continue;
            }
            /* If a trap is shown here, don't count it; the hero
             * should be expecting it.  But if there is a trap here
             * that's not shown, either undiscovered or covered by
             * something, do count it.
             */
            glyph = glyph_at(dx, dy);
            if (glyph_is_trap(glyph)) {
                continue;
            }
            if ((ttmp = t_at(dx, dy)) != 0) {
                ++ret;
                continue;
            }
            levp = &levl[dx][dy];
            if (IS_DOOR(levp->typ) && (levp->doormask & D_TRAPPED) != 0) {
                ++ret;
                continue;
            }
            for (otmp = level.objects[dx][dy]; otmp; otmp = otmp->nexthere) {
                if (Is_container(otmp) && otmp->otrapped) {
                    ++ret; /* we're counting locations, so just */
                    break; /* count the first one in a pile     */
                }
            }
        }
    }
    /*
     * [Shouldn't we also check inventory for a trapped container?
     * Even if its trap has already been found, there's no 'tknown'
     * flag to help hero remember that so we have nothing comparable
     * to a shown glyph to justify skipping it.]
     */
    return ret;
}

/* sense adjacent traps if wielding MKoT without wearing gloves */
void
mkot_trap_warn(void)
{
    static const char *const heat[7] = {
        "cool", "slightly warm", "warm", "very warm", "hot", "very hot", "like fire"
    };

    if (!uarmg && uwep && uwep->oartifact == ART_MASTER_KEY_OF_THIEVERY) {
        int idx, ntraps = count_surround_traps(u.ux, u.uy);

        if (ntraps != mkot_trap_warn_count) {
            idx = min(ntraps, SIZE(heat)-1);
            pline_The("Key feels %s%c", heat[idx], (ntraps > 3) ? '!' : '.');
        }
        mkot_trap_warn_count = ntraps;
    } else {
        mkot_trap_warn_count = 0;
    }
}

/* Master Key is magic key if its bless/curse state meets our criteria:
   not cursed for rogues or blessed for non-rogues */
boolean
is_magic_key(struct monst *mon, struct obj *obj)
                   /* if null, non-rogue is assumed */

{
    if (!obj || obj->oartifact != ART_MASTER_KEY_OF_THIEVERY) {
        return FALSE;
    }
    boolean is_rogue = (mon == &youmonst) ? Role_if(PM_ROGUE) : (mon && mon->data == &mons[PM_ROGUE]);
    if (is_rogue ?  !obj->cursed : obj->blessed) {
        return TRUE;
    }
    return FALSE;
}

/* figure out whether 'mon' (usually youmonst) is carrying the magic key */
struct obj *
has_magic_key(struct monst *mon) /**< if null, hero assumed */
{
    struct obj *o;
    short key = artilist[ART_MASTER_KEY_OF_THIEVERY].otyp;

    if (!mon) {
        mon = &youmonst;
    }
    o = (mon == &youmonst) ? invent : mon->minvent;
    while (o) {
        if (is_magic_key(mon, o)) {
            return o;
        }
        o = nxtobj(o, key, FALSE);
    }
    return (struct obj *) 0;
}

/* #define is_art(o,art) ((o) && (o)->oartifact == (art)) */
boolean
is_art(struct obj *obj, int art)
{
    if (obj && obj->oartifact == art) {
        return TRUE;
    }

    return FALSE;
}

/*artifact.c*/
