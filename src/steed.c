/* Copyright (c) Kevin Hugo, 1998-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Monsters that might be ridden */
static NEARDATA const char steeds[] = {
    S_QUADRUPED, S_UNICORN, S_ANGEL, S_CENTAUR, S_DRAGON, S_JABBERWOCK, '\0'
};

static boolean landing_spot(coord *, int, int);
static void maybewakesteed(struct monst *);

/* caller has decided that hero can't reach something while mounted */
void
rider_cant_reach(void)
{
    You("aren't skilled enough to reach from %s.", y_monnam(u.usteed));
}

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
boolean
can_saddle(struct monst *mtmp)
{
    struct permonst *ptr = mtmp->data;

    return (strchr(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM) &&
            (!humanoid(ptr) || ptr->mlet == S_CENTAUR) &&
            !amorphous(ptr) && !noncorporeal(ptr) &&
            !is_whirly(ptr) && !unsolid(ptr));
}

int
use_saddle(struct obj *otmp)
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;
    const char *s;

    if (!u_handsy()) {
        return ECMD_OK;
    }

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *)0)) {
        pline("%s", Never_mind);
        return ECMD_CANCEL;
    }
    if (!u.dx && !u.dy) {
        pline("Saddle yourself?  Very funny...");
        return ECMD_OK;
    }
    if (!isok(u.ux+u.dx, u.uy+u.dy) ||
        !(mtmp = m_at(u.ux+u.dx, u.uy+u.dy)) ||
        !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return ECMD_TIME;
    }

    /* Is this a valid monster? */
    if (mtmp->misc_worn_check & W_SADDLE ||
        which_armor(mtmp, W_SADDLE)) {
        pline("%s doesn't need another one.", Monnam(mtmp));
        return ECMD_TIME;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        if (!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
            Sprintf(kbuf, "attempting to saddle %s", an(mtmp->data->mname));
            instapetrify(kbuf);
        }
    }
#ifdef WEBB_DISINT
    if (touch_disintegrates(ptr)) {
        char kbuf[BUFSZ];
        if (!oresist_disintegration(otmp)) {
            pline("%s disintegrates!", Yname2(otmp));
            useup(otmp);
        }
        Sprintf(kbuf, "attempting to saddle %s", a_monnam(mtmp));
        instadisintegrate(kbuf);
    }
#endif

    if (ptr == &mons[PM_INCUBUS] || ptr == &mons[PM_SUCCUBUS]) {
        pline("Shame on you!");
        exercise(A_WIS, FALSE);
        return ECMD_TIME;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest ||
        mtmp->isgd || mtmp->iswiz) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return ECMD_TIME;
    }
    if (!can_saddle(mtmp)) {
        You_cant("saddle such a creature.");
        return ECMD_TIME;
    }

    /* Calculate your chance */
    chance = ACURR(A_DEX) + ACURR(A_CHA)/2 + 2*mtmp->mtame;
    chance += u.ulevel * (mtmp->mtame ? 20 : 5);
    if (!mtmp->mtame) {
        chance -= 10*mtmp->m_lev;
    }
    if (Role_if(PM_KNIGHT)) {
        chance += 20;
    }
    switch (P_SKILL(P_RIDING)) {
    case P_ISRESTRICTED:
    case P_UNSKILLED:
    default:
        chance -= 20;
        break;
    case P_BASIC:
        break;
    case P_SKILLED:
        chance += 15;
        break;
    case P_EXPERT:
        chance += 30;
        break;
    }
    if (Confusion || Fumbling || Glib) {
        chance -= 20;
    } else if (uarmg &&
             (s = OBJ_DESCR(objects[uarmg->otyp])) != (char *)0 &&
             !strncmp(s, "riding ", 7))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf &&
             (s = OBJ_DESCR(objects[uarmf->otyp])) != (char *)0 &&
             !strncmp(s, "riding ", 7))
        /* ... or for "riding boots" */
        chance += 10;
    if (otmp->cursed) {
        chance -= 50;
    }

    /* [intended] steed becomes alert if possible */
    maybewakesteed(mtmp);

    /* Make the attempt */
    if (rn2(100) < chance) {
        You("put the saddle on %s.", mon_nam(mtmp));
        if (otmp->owornmask) {
            remove_worn_item(otmp, FALSE);
        }
        freeinv(otmp);
        /* !can_saddle(mtmp) already eliminated above */
        put_saddle_on_mon(otmp, mtmp);
    } else {
        pline("%s resists!", Monnam(mtmp));
    }

    return ECMD_TIME;
}

void
put_saddle_on_mon(struct obj *saddle, struct monst *mtmp)
{
    if (!can_saddle(mtmp) || which_armor(mtmp, W_SADDLE)) {
        if (saddle) {
            impossible("put_saddle_on_mon: saddle obj could get orphaned");
        }
        return;
    }
    if (!saddle) {
        if ((saddle = mksobj(SADDLE, TRUE, FALSE)) != 0) {
            fully_identify_obj(saddle);
            /* mpickobj can later override identification if out-of-view */
        } else {
            return;
        }
    }
    if (mpickobj(mtmp, saddle)) {
        panic("merged saddle?");
    }
    mtmp->misc_worn_check |= W_SADDLE;
    saddle->owornmask = W_SADDLE;
    saddle->leashmon = mtmp->m_id;
    update_mon_intrinsics(mtmp, saddle, TRUE, FALSE);
}

/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
boolean
can_ride(struct monst *mtmp)
{
    return (mtmp->mtame && humanoid(youmonst.data) &&
            !verysmall(youmonst.data) && !bigmonst(youmonst.data) &&
            (!Underwater || is_swimmer(mtmp->data)));
}

/* the #ride command */
int
doride(void)
{
    boolean forcemount = FALSE;

    if (u.usteed) {
        dismount_steed(DISMOUNT_BYCHOICE);
    } else if (getdir((char *)0) && isok(u.ux+u.dx, u.uy+u.dy)) {
        if (wizard && yn("Force the mount to succeed?") == 'y') {
            forcemount = TRUE;
        }
        return (mount_steed(m_at(u.ux + u.dx, u.uy + u.dy), forcemount) ? ECMD_TIME : ECMD_OK);
    } else {
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

/* Start riding, with the given monster */
boolean
mount_steed(
    struct monst *mtmp, /*< The animal */
    boolean force)      /*< Quietly force this animal */
{
    struct obj *otmp;
    char buf[BUFSZ];
    struct permonst *ptr;

    /* Sanity checks */
    if (u.usteed) {
        You("are already riding %s.", mon_nam(u.usteed));
        return (FALSE);
    }

    /* Is the player in the right form? */
    if (Hallucination && !force) {
        pline("Maybe you should find a designated driver.");
        return (FALSE);
    }
    /* While riding Wounded_legs refers to the steed's,
     * not the hero's legs.
     * That opens up a potential abuse where the player
     * can mount a steed, then dismount immediately to
     * heal leg damage, because leg damage is always
     * healed upon dismount (Wounded_legs context switch).
     * By preventing a hero with Wounded_legs from
     * mounting a steed, the potential for abuse is
     * reduced.  However, dismounting still immediately
     * heals the steed's wounded legs.  [In 3.4.3 and
     * earlier, that unintentionally made the hero's
     * temporary 1 point Dex loss become permanent.]
     */
    if (Wounded_legs) {
        Your("%s are in no shape for riding.", makeplural(body_part(LEG)));
#ifdef WIZARD
        if (force && wizard && yn("Heal your legs?") == 'y') {
            HWounded_legs = EWounded_legs = 0;
        } else
#endif
        return (FALSE);
    }

    if (Upolyd && (!humanoid(youmonst.data) || verysmall(youmonst.data) ||
                   bigmonst(youmonst.data) || slithy(youmonst.data))) {
        You("won't fit on a saddle.");
        return (FALSE);
    }
    if (!force && (near_capacity() > SLT_ENCUMBER)) {
        You_cant("do that while carrying so much stuff.");
        return (FALSE);
    }

    /* Can the player reach and see the monster? */
    if (!mtmp || (!force && ((Blind && !Blind_telepat) ||
                             mtmp->mundetected ||
                             M_AP_TYPE(mtmp) == M_AP_FURNITURE ||
                             M_AP_TYPE(mtmp) == M_AP_OBJECT))) {
        pline("I see nobody there.");
        return (FALSE);
    }
    if (mtmp->data == &mons[PM_LONG_WORM] &&
         (u.ux + u.dx != mtmp->mx || u.uy + u.dy != mtmp->my)) {
        /* As of 3.6.2:  test_move(below) is used to check for trying to mount
           diagonally into or out of a doorway or through a tight squeeze;
           attempting to mount a tail segment when hero was not adjacent
           to worm's head could trigger an impossible() in worm_cross()
           called from test_move(), so handle not-on-head before that */
        You("couldn't ride %s, let alone its tail.", a_monnam(mtmp));
        return FALSE;
    }
    if (u.uswallow || u.ustuck || u.utrap || Punished ||
        !test_move(u.ux, u.uy, mtmp->mx-u.ux, mtmp->my-u.uy, TEST_MOVE)) {
        if (Punished || !(u.uswallow || u.ustuck || u.utrap)) {
            You("are unable to swing your %s over.", body_part(LEG));
        } else {
            You("are stuck here for now.");
        }
        return (FALSE);
    }

    /* Is this a valid monster? */
    otmp = which_armor(mtmp, W_SADDLE);
    if (!otmp) {
        pline("%s is not saddled.", Monnam(mtmp));
        return (FALSE);
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        Sprintf(kbuf, "attempting to ride %s", an(mtmp->data->mname));
        instapetrify(kbuf);
    }
    if (!mtmp->mtame || mtmp->isminion) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return (FALSE);
    }
    if (mtmp->mtrapped) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);

        You_cant("mount %s while %s's trapped in %s.",
                 mon_nam(mtmp), mhe(mtmp),
                 an(defsyms[trap_to_defsym(t->ttyp)].explanation));
        return (FALSE);
    }

    if (!force && !Role_if(PM_KNIGHT) && !(--mtmp->mtame)) {
        /* no longer tame */
        newsym(mtmp->mx, mtmp->my);
        pline("%s resists%s!", Monnam(mtmp),
              mtmp->mleashed ? " and its leash comes off" : "");
        if (mtmp->mleashed) {
            m_unleash(mtmp, FALSE);
        }
        return (FALSE);
    }
    if (!force && Underwater && !is_swimmer(ptr)) {
        You_cant("ride that creature while under %s.", hliquid("water"));
        return (FALSE);
    }
    if (!can_saddle(mtmp) || !can_ride(mtmp)) {
        You_cant("ride such a creature.");
        return FALSE;
    }

    /* Is the player impaired? */
    if (!force && !is_floater(ptr) && !is_flyer(ptr) &&
        Levitation && !Lev_at_will) {
        You("cannot reach %s.", mon_nam(mtmp));
        return (FALSE);
    }
    if (!force && uarm && is_metallic(uarm) &&
        greatest_erosion(uarm)) {
        Your("%s armor is too stiff to be able to mount %s.",
             uarm->oeroded ? "rusty" : "corroded",
             mon_nam(mtmp));
        return (FALSE);
    }
    if (!force && (Confusion || Fumbling || Glib || Wounded_legs ||
                   otmp->cursed ||
                   otmp->greased ||
                   (u.ulevel+mtmp->mtame < rnd(MAXULEV/2+5)))) {
        if (Levitation) {
            pline("%s slips away from you.", Monnam(mtmp));
            return FALSE;
        }
        You("slip while trying to get on %s.", mon_nam(mtmp));

        Sprintf(buf, "slipped while mounting %s",
                /* "a saddled mumak" or "a saddled pony called Dobbin" */
                x_monnam(mtmp, ARTICLE_A, (char *)0,
                         SUPPRESS_IT|SUPPRESS_INVISIBLE|SUPPRESS_HALLUCINATION,
                         TRUE));
        losehp(Maybe_Half_Phys(rn1(5, 10)), buf, NO_KILLER_PREFIX);
        return (FALSE);
    }

    /* Success */
    maybewakesteed(mtmp);
    if (!force) {
        if (Levitation && !is_floater(ptr) && !is_flyer(ptr)) {
            /* Must have Lev_at_will at this point */
            pline("%s magically floats up!", Monnam(mtmp));
        }
        You("mount %s.", mon_nam(mtmp));
        if (Flying) {
            You("and %s take flight together.", mon_nam(mtmp));
        }
    }
    /* setuwep handles polearms differently when you're mounted */
    if (uwep && is_pole(uwep)) {
        unweapon = FALSE;
    }
    u.usteed = mtmp;

    boolean was_stealthy = Stealth != 0;
    steed_vs_stealth();
    if (was_stealthy && !Stealth) {
        You("aren't stealthy anymore.");
    }

    remove_monster(mtmp->mx, mtmp->my);
    teleds(mtmp->mx, mtmp->my, TRUE);
    flags.botl = TRUE;
    return TRUE;
}

/* You and your steed have moved */
void
exercise_steed(void)
{
    if (!u.usteed) {
        return;
    }

    /* It takes many turns of riding to exercise skill
       and even more exercise to raise the skill level */
    if (u.urideturns++ >= 20) {
        u.urideturns = 0;
        use_skill(P_RIDING, 1);
    }
    return;
}

/* The player kicks or whips the steed */
void
kick_steed(void)
{
    char He[BUFSZ]; /* monverbself() appends to the "He"/"She"/"It" value */

    if (!u.usteed) {
        return;
    }

    /* [ALI] Various effects of kicking sleeping/paralyzed steeds */
    if (helpless(u.usteed)) {
        /* We assume a message has just been output of the form
         * "You kick <steed>."
         */
        Strcpy(He, mhe(u.usteed));
        *He = highc(*He);
        if ((u.usteed->mcanmove || u.usteed->mfrozen) && !rn2(2)) {
            if (u.usteed->mcanmove) {
                u.usteed->msleeping = 0;
            } else if (u.usteed->mfrozen > 2) {
                u.usteed->mfrozen -= 2;
            } else {
                u.usteed->mfrozen = 0;
                u.usteed->mcanmove = 1;
            }
            if (helpless(u.usteed)) {
                pline("%s stirs.", He);
            } else {
                /* if hallucinating, might yield "He rouses herself" or
                   "She rouses himself" */
                pline("%s!", monverbself(u.usteed, He, "rouse", (char *) 0));
            }
        } else {
            pline("%s does not respond.", He);
        }
        return;
    }

    /* Make the steed less tame and check if it resists */
    if (u.usteed->mtame) {
        u.usteed->mtame--;
    }
    if (!u.usteed->mtame && u.usteed->mleashed) {
        m_unleash(u.usteed, TRUE);
    }
    if (!u.usteed->mtame || (u.ulevel+u.usteed->mtame < rnd(MAXULEV/2+5))) {
        newsym(u.usteed->mx, u.usteed->my);
        dismount_steed(DISMOUNT_THROWN);
        return;
    }

    pline("%s gallops!", Monnam(u.usteed));
    /* Unskilled:  10-19 (more) turns
     * Basic:      20-39 (more) turns
     * Skilled:    30-59 (more) turns
     * Expert:     40-79 (more) turns */
    u.ugallop += rn1(P_SKILL(P_RIDING)*10, P_SKILL(P_RIDING)*10);
    return;
}

/*
 * Try to find a dismount point adjacent to the steed's location.
 * If all else fails, try enexto().  Use enexto() as a last resort because
 * enexto() chooses its point randomly, possibly even outside the
 * room's walls, which is not what we want.
 * Adapted from mail daemon code.
 */
static boolean
landing_spot(coord *spot, int reason, int forceit)
                /* landing position (we fill it in) */


{
    int i = 0, x, y, distance, min_distance = -1;
    boolean found = FALSE;
    struct trap *t;

    /* avoid known traps (i == 0) and boulders, but allow them as a backup */
    if (reason != DISMOUNT_BYCHOICE || Stunned || Confusion || Fumbling) {
        i = 1;
    }
    for (; !found && i < 2; ++i) {
        for (x = u.ux - 1; x <= u.ux + 1; x++) {
            for (y = u.uy - 1; y <= u.uy + 1; y++) {
                if (!isok(x, y) || u_at(x, y)) {
                    continue;
                }

                if (accessible(x, y) && !MON_AT(x, y)) {
                    distance = distu(x, y);
                    if (min_distance < 0 || distance < min_distance ||
                        (distance == min_distance && rn2(2))) {
                        if (i > 0 || (((t = t_at(x, y)) == 0 || !t->tseen) &&
                                      (!sobj_at(BOULDER, x, y) ||
                                       throws_rocks(youmonst.data)))) {
                            spot->x = x;
                            spot->y = y;
                            min_distance = distance;
                            found = TRUE;
                        }
                    }
                }
            }
        }
    }

    /* If we didn't find a good spot and forceit is on, try enexto(). */
    if (forceit && min_distance < 0 &&
        !enexto(spot, u.ux, u.uy, youmonst.data))
        return FALSE;

    return found;
}

/* Stop riding the current steed */
void
dismount_steed(
    int reason) /**< Player was thrown off etc. */
{
    struct monst *mtmp;
    struct obj *otmp;
    coord cc, steedcc;
    const char *verb = "fall";
    boolean repair_leg_damage = TRUE;
    unsigned save_utrap = u.utrap;
    boolean have_spot = landing_spot(&cc, reason, 0);

    mtmp = u.usteed;        /* make a copy of steed pointer */
    /* Sanity check */
    if (!mtmp) { /* Just return silently */
        return;
    }

    /* Check the reason for dismounting */
    otmp = which_armor(mtmp, W_SADDLE);
    switch (reason) {
    case DISMOUNT_THROWN:
        verb = "are thrown";
        /* fall through */
    case DISMOUNT_FELL:
        You("%s off of %s!", verb, mon_nam(mtmp));
        /* fall through */
    case DISMOUNT_VANISHED:
        if (!have_spot) {
            have_spot = landing_spot(&cc, reason, 1);
        }
        losehp(Maybe_Half_Phys(rn1(10, 10)), "riding accident", KILLED_BY_AN);
        set_wounded_legs(BOTH_SIDES, (int)HWounded_legs + rn1(5, 5));
        repair_leg_damage = FALSE;
        break;
    case DISMOUNT_POLY:
        You("can no longer ride %s.", mon_nam(u.usteed));
        if (!have_spot) {
            have_spot = landing_spot(&cc, reason, 1);
        }
        break;
    case DISMOUNT_ENGULFED:
        /* caller displays message */
        break;
    case DISMOUNT_BONES:
        /* hero has just died... */
        break;
    case DISMOUNT_GENERIC:
        /* no messages, just make it so */
        break;
    case DISMOUNT_BYCHOICE:
    default:
        if (otmp && otmp->cursed) {
            You("can't.  The saddle %s cursed.",
                otmp->bknown ? "is" : "seems to be");
            otmp->bknown = 1; /* ok to skip set_bknown() here */
            return;
        }
        if (!have_spot) {
            You("can't. There isn't anywhere for you to stand.");
            return;
        }
        if (!has_mgivenname(mtmp)) {
            pline("You've been through the dungeon on %s with no name.",
                  an(mtmp->data->mname));
            if (Hallucination) {
                pline("It felt good to get out of the rain.");
            }
        } else {
            You("dismount %s.", mon_nam(mtmp));
        }
    }
    /* While riding, Wounded_legs refers to the steed's legs;
       after dismounting, it reverts to the hero's legs. */
    if (repair_leg_damage) {
        heal_legs(1);
    }

    /* Release the steed and saddle */
    u.usteed = 0;
    u.ugallop = 0L;

    /*
     * rloc(), rloc_to(), and monkilled()->mondead()->m_detach() all
     * expect mtmp to be on the map or else have mtmp->mx be 0, but
     * setting the latter to 0 here would interfere with dropping
     * the saddle.  Prior to 3.6.2, being off the map didn't matter.
     *
     * place_monster() expects mtmp to be alive and not be u.usteed.
     *
     * Unfortunately, <u.ux,u.uy> (former steed's implicit location)
     * might now be occupied by an engulfer, so we can't just put mtmp
     * at that spot.  An engulfer's previous spot will be unoccupied
     * but we don't know where that was and even if we did, it might
     * be hostile terrain.
     */
    steedcc.x = u.ux, steedcc.y = u.uy;
    if (m_at(u.ux, u.uy)) {
        /* hero's spot has a monster in it; hero must have been plucked
           from saddle as engulfer moved into his spot--other dismounts
           shouldn't run into this situation; find nearest viable spot */
        if (!enexto(&steedcc, u.ux, u.uy, mtmp->data) &&
            /* no spot? must have been engulfed by a lurker-above over
               water or lava; try requesting a location for a flyer */
             !enexto(&steedcc, u.ux, u.uy, &mons[PM_BAT]))
            /* still no spot; last resort is any spot within bounds */
            (void) enexto(&steedcc, u.ux, u.uy, &mons[PM_GHOST]);
    }

    if (!DEADMONSTER(mtmp)) {
        in_steed_dismounting++;
        place_monster(mtmp, steedcc.x, steedcc.y);
        in_steed_dismounting--;
        /* if for bones, there's no reason to place the hero;
           we want to make room for potential ghost, so move steed */
        if (reason == DISMOUNT_BONES) {
            /* move the steed to an adjacent square */
            if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
                rloc_to(mtmp, cc.x, cc.y);
            } else {
                /* evidently no room nearby; move steed elsewhere */
                (void) rloc(mtmp, RLOC_ERR | RLOC_NOMSG);
            }
            return;
        }

        /* Set hero's and/or steed's positions.  Usually try moving the
           hero first.  Note: for DISMOUNT_ENGULFED, caller hasn't set
           u.uswallow yet but has set u.ustuck. */
        if (!u.uswallow && !u.ustuck && have_spot) {
            struct permonst *mdat = mtmp->data;

            /* The steed may drop into water/lava */
            if (!is_flyer(mdat) && !is_floater(mdat) && !is_clinger(mdat)) {
                if (is_pool(u.ux, u.uy)) {
                    if (!Underwater) {
                        pline("%s falls into the %s!", Monnam(mtmp), surface(u.ux, u.uy));
                    }
                    if (!cant_drown(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                } else if (is_lava(u.ux, u.uy)) {
                    pline("%s is pulled into the %s!", Monnam(mtmp), hliquid("lava"));
                    if (!likes_lava(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                }
            }
            /* Steed dismounting consists of two steps: being moved to another
             * square, and descending to the floor.  We have functions to do
             * each of these activities, but they're normally called
             * individually and include an attempt to look at or pick up the
             * objects on the floor:
             * teleds() --> spoteffects() --> pickup()
             * float_down() --> pickup()
             * We use this kludge to make sure there is only one such attempt.
             *
             * Clearly this is not the best way to do it.  A full fix would
             * involve having these functions not call pickup() at all,
             * instead
             * calling them first and calling pickup() afterwards.  But it
             * would take a lot of work to keep this change from having any
             * unforeseen side effects (for instance, you would no longer be
             * able to walk onto a square with a hole, and autopickup before
             * falling into the hole).
             */
            /* [ALI] No need to move the player if the steed died. */
            if (!DEADMONSTER(mtmp)) {
                /* Keep steed here, move the player to cc;
                 * teleds() clears u.utrap
                 */
                in_steed_dismounting = TRUE;
                teleds(cc.x, cc.y, TRUE);
                in_steed_dismounting = FALSE;

                /* Put your steed in your trap */
                if (save_utrap) {
                    (void) mintrap(mtmp, NO_TRAP_FLAGS);
                }
            }

        /* Couldn't move hero... try moving the steed. */
        } else if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
            /* Keep player here, move the steed to cc */
            rloc_to(mtmp, cc.x, cc.y);
            /* Player stays put */

        /* Otherwise, kill the steed. */
        } else {
            if (reason == DISMOUNT_BYCHOICE) {
                /* [un]#ride: hero gets credit/blame for killing steed */
                killed(mtmp);
                adjalign(-1);
            } else {
                /* other dismount: kill former steed with no penalty;
                   damage type is just "neither AD_DGST nor -AD_RBRE" */
                monkilled(mtmp, "", -AD_PHYS);
            }
        }
    } /* !DEADMONST(mtmp) */

    /* usually return the hero to the surface */
    if (reason != DISMOUNT_ENGULFED && reason != DISMOUNT_BONES) {
        in_steed_dismounting = TRUE;
        (void) float_down(0L, W_SADDLE);
        in_steed_dismounting = FALSE;
        flags.botl = 1;
        (void)encumber_msg();
        vision_full_recalc = 1;
    } else {
        flags.botl = 1;
    }
    /* polearms behave differently when not mounted */
    if (uwep && is_pole(uwep)) {
        unweapon = TRUE;
    }
    return;
}

/* when attempting to saddle or mount a sleeping steed, try to wake it up
   (for the saddling case, it won't be u.usteed yet) */
static void
maybewakesteed(struct monst *steed)
{
    int frozen = (int) steed->mfrozen;
    boolean wasimmobile = helpless(steed);

    steed->msleeping = 0;
    if (frozen) {
        frozen = (frozen + 1) / 2; /* half */
        /* might break out of timed sleep or paralysis */
        if (!rn2(frozen)) {
            steed->mfrozen = 0;
            steed->mcanmove = 1;
        } else {
            /* didn't awake, but remaining duration is halved */
            steed->mfrozen = frozen;
        }
    }
    if (wasimmobile && !helpless(steed)) {
        pline("%s wakes up.", Monnam(steed));
    }
    /* regardless of waking, terminate any meal in progress */
    finish_meating(steed);
}

/* steed has taken on a new shape */
void
poly_steed(
    struct monst *steed,
    struct permonst *oldshape)
{
    if (!can_saddle(steed) || !can_ride(steed)) {
        /* can't get here; newcham() -> mon_break_armor() -> m_lose_armor()
           removes saddle and/or forces hero to dismount, if applicable,
           before newcham() calls us */
        dismount_steed(DISMOUNT_FELL);
    } else {
        char buf[BUFSZ];

        Strcpy(buf, x_monnam(steed, ARTICLE_YOUR, (char *) 0,
                             SUPPRESS_SADDLE, FALSE));
        if (oldshape != steed->data) {
            (void) strsubst(buf, "your ", "your new ");
        }
        You("adjust yourself in the saddle on %s.", buf);

        /* riding blocks stealth unless hero+steed fly */
        steed_vs_stealth();
    }
}

/* decide whether hero's steed is able to move;
   doesn't check for holding traps--those affect the hero directly */
boolean
stucksteed(boolean checkfeeding)
{
    struct monst *steed = u.usteed;

    if (steed) {
        /* check whether steed can move */
        if (helpless(steed)) {
            pline("%s won't move!", YMonnam(steed));
            return TRUE;
        }
        /* optionally check whether steed is in the midst of a meal */
        if (checkfeeding && steed->meating) {
            pline("%s is still eating.", YMonnam(steed));
            return TRUE;
        }
    }
    return FALSE;
}

void
place_monster(struct monst *mon, coordxy x, coordxy y)
{
    struct monst *othermon;
    const char *monnm, *othnm;
    char buf[QBUFSZ];

    buf[0] = '\0';
    /* normal map bounds are <1..COLNO-1,0..ROWNO-1> but sometimes
       vault guards (either living or dead) are parked at <0,0> */
    if (!isok(x, y) && (x != 0 || y != 0 || !mon->isgd)) {
        describe_level(buf);
        impossible("trying to place %s at <%d,%d> mstate:%lx on %s",
                   minimal_monnam(mon, TRUE), x, y, mon->mstate, buf);
        x = y = 0;
    }
    if ((mon == u.usteed && !in_steed_dismounting) ||
        /* special case is for convoluted vault guard handling */
        (DEADMONSTER(mon) && !(mon->isgd && x == 0 && y == 0))) {
        describe_level(buf);
        impossible("placing %s onto map, mstate:%lx, on %s?",
                   (mon == u.usteed) ? "steed" : "defunct monster",
                   mon->mstate, buf);
        return;
    }
    if ((othermon = level.monsters[x][y]) != 0) {
        describe_level(buf);
        monnm = minimal_monnam(mon, FALSE);
        othnm = (mon != othermon) ? minimal_monnam(othermon, TRUE) : "itself";
        impossible("placing %s over %s at <%d,%d>, mstates:%lx %lx on %s?",
                   monnm, othnm, x, y, othermon->mstate, mon->mstate, buf);
    }
    mon->mx = x, mon->my = y;
    level.monsters[x][y] = mon;
    mon->mstate = MON_FLOOR;

    if (mon->data == &mons[PM_GIANT_TURTLE] && (!mon->minvis || See_invisible)) {
        block_point(x, y);
    }
}

/*steed.c*/
