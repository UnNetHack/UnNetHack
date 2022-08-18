/*  Copyright 1988, 1989, 1990, 1992, M. Stephenson       */
/* NetHack may be freely redistributed.  See license for details. */

/*  attribute modification routines. */

#include "hack.h"
#include "artifact.h"
#include "artilist.h"

/* #define DEBUG */ /* uncomment for debugging info */

/* part of the output on gain or loss of attribute */
static
const char  * const plusattr[] = {
    "strong", "smart", "wise", "agile", "tough", "charismatic"
},
* const minusattr[] = {
    "weak", "stupid", "foolish", "clumsy", "fragile", "repulsive"
};
/* also used by enlightenment for non-abbreviated status info */
const char *const attrname[] = {
    "strength", "intelligence", "wisdom",
    "dexterity", "constitution", "charisma"
};


static
const struct innate {
    schar ulevel;
    long    *ability;
    const char *gainstr, *losestr;
}   arc_abil[] = { {     1, &(HStealth), "", "" },
                   {   1, &(HFast), "", "" },
                   {  10, &(HSearching), "perceptive", "" },
                   {   0, 0, 0, 0 } },

    bar_abil[] = { {     1, &(HPoison_resistance), "", "" },
                   {   7, &(HFast), "quick", "slow" },
                   {  15, &(HStealth), "stealthy", "" },
                   {   0, 0, 0, 0 } },

    cav_abil[] = { {     7, &(HFast), "quick", "slow" },
                   {  15, &(HWarning), "sensitive", "" },
                   {   0, 0, 0, 0 } },

    con_abil[] = { {   1, &(HSick_resistance), "", "" },
                   {   7, &(HPoison_resistance), "healthy", "" },
                   {  20, &(HSearching), "perceptive", "unaware" },
                   {   0, 0, 0, 0 } },

hea_abil[] = { {     1, &(HPoison_resistance), "", "" },
               {  15, &(HWarning), "sensitive", "" },
               {   0, 0, 0, 0 } },

    kni_abil[] = { {     7, &(HFast), "quick", "slow" },
                   {   0, 0, 0, 0 } },

    mon_abil[] = { {   1, &(HFast), "", "" },
                   {   1, &(HSleep_resistance), "", "" },
                   {   1, &(HSee_invisible), "", "" },
                   {   3, &(HPoison_resistance), "healthy", "" },
                   {   5, &(HStealth), "stealthy", "" },
                   {   7, &(HWarning), "sensitive", "" },
                   {   9, &(HSearching), "perceptive", "unaware" },
                   {  11, &(HFire_resistance), "cool", "warmer" },
                   {  13, &(HCold_resistance), "warm", "cooler" },
                   {  15, &(HShock_resistance), "insulated", "conductive" },
                   {  17, &(HTeleport_control), "controlled", "uncontrolled" },
                   {   0, 0, 0, 0 } },

    pri_abil[] = { {    15, &(HWarning), "sensitive", "" },
                   {  20, &(HFire_resistance), "cool", "warmer" },
                   {   0, 0, 0, 0 } },

    ran_abil[] = { {   1, &(HSearching), "", "" },
                   {   7, &(HStealth), "stealthy", "" },
                   {  15, &(HSee_invisible), "", "" },
                   {   0, 0, 0, 0 } },

    rog_abil[] = { {     1, &(HStealth), "", ""  },
                   {  10, &(HSearching), "perceptive", "" },
                   {   0, 0, 0, 0 } },

    sam_abil[] = { {     1, &(HFast), "", "" },
                   {  15, &(HStealth), "stealthy", "" },
                   {   0, 0, 0, 0 } },

    tou_abil[] = { {    10, &(HSearching), "perceptive", "" },
                   {  20, &(HPoison_resistance), "hardy", "" },
                   {   0, 0, 0, 0 } },

    val_abil[] = { {     1, &(HCold_resistance), "", "" },
                   {   1, &(HStealth), "", "" },
                   {   7, &(HFast), "quick", "slow" },
                   {   0, 0, 0, 0 } },

    wiz_abil[] = { {    15, &(HWarning), "sensitive", "" },
                   {  17, &(HTeleport_control), "controlled", "uncontrolled" },
                   {   0, 0, 0, 0 } },

    /* Intrinsics conferred by race */
    dwa_abil[] = { { 1, &HInfravision, "", "" },
                   { 0, 0, 0, 0 } },

    elf_abil[] = { { 1, &HInfravision, "", "" },
                   { 4, &(HSleep_resistance), "awake", "tired" },
                   { 0, 0, 0, 0 } },

    gno_abil[] = { { 1, &HInfravision, "", "" },
                   { 0, 0, 0, 0 } },

    orc_abil[] = { { 1, &HInfravision, "", "" },
                   { 1, &(HPoison_resistance), "", "" },
                   { 0, 0, 0, 0 } },

    hum_abil[] = { { 0, 0, 0, 0 } };

static long next_check = 600L;  /* arbitrary first setting */
static void exerper(void);
static void postadjabil(long *);

/* adjust an attribute; return TRUE if change is made, FALSE otherwise */
boolean
adjattrib(int ndx, int incr, int msgflg)

                    /* positive => no message, zero => message, and */
{               /* negative => conditional (msg if change made) */
    if (Fixed_abil || !incr) return FALSE;

    if ((ndx == A_INT || ndx == A_WIS)
        && uarmh && uarmh->otyp == DUNCE_CAP) {
        if (msgflg == 0)
            Your("cap constricts briefly, then relaxes again.");
        return FALSE;
    }

    if (incr > 0) {
        if ((AMAX(ndx) >= ATTRMAX(ndx)) && (ACURR(ndx) >= AMAX(ndx))) {
            if (msgflg == 0 && flags.verbose)
                pline("You're already as %s as you can get.",
                      plusattr[ndx]);
            ABASE(ndx) = AMAX(ndx) = ATTRMAX(ndx); /* just in case */
            return FALSE;
        }

        ABASE(ndx) += incr;
        if(ABASE(ndx) > AMAX(ndx)) {
            incr = ABASE(ndx) - AMAX(ndx);
            AMAX(ndx) += incr;
            if(AMAX(ndx) > ATTRMAX(ndx))
                AMAX(ndx) = ATTRMAX(ndx);
            ABASE(ndx) = AMAX(ndx);
        }
    } else {
        if (ABASE(ndx) <= ATTRMIN(ndx)) {
            if (msgflg == 0 && flags.verbose)
                pline("You're already as %s as you can get.",
                      minusattr[ndx]);
            ABASE(ndx) = ATTRMIN(ndx); /* just in case */
            return FALSE;
        }

        ABASE(ndx) += incr;
        if(ABASE(ndx) < ATTRMIN(ndx)) {
            incr = ABASE(ndx) - ATTRMIN(ndx);
            ABASE(ndx) = ATTRMIN(ndx);
            AMAX(ndx) += incr;
            if(AMAX(ndx) < ATTRMIN(ndx))
                AMAX(ndx) = ATTRMIN(ndx);
        }
    }
    if (msgflg <= 0)
        You_feel("%s%s!",
                 (incr > 1 || incr < -1) ? "very " : "",
                 (incr > 0) ? plusattr[ndx] : minusattr[ndx]);
    flags.botl = 1;
    if (moves > 1 && (ndx == A_STR || ndx == A_CON))
        (void)encumber_msg();
    return TRUE;
}

void
gainstr(register struct obj *otmp, register int incr)
{
    int num = 1;

    if(incr) num = incr;
    else {
        if(ABASE(A_STR) < 18) num = (rn2(4) ? 1 : rnd(6) );
        else if (ABASE(A_STR) < STR18(85)) num = rnd(10);
    }
    (void) adjattrib(A_STR, (otmp && otmp->cursed) ? -num : num, TRUE);
}

void
losestr(register int num)    /* may kill you; cause may be poison or monster like 'a' */

{
    int ustr = ABASE(A_STR) - num;

    while(ustr < 3) {
        ++ustr;
        --num;
        if (Upolyd) {
            u.mh -= 6;
            u.mhmax -= 6;
        } else {
            u.uhp -= 6;
            u.uhpmax -= 6;
        }
    }
    (void) adjattrib(A_STR, -num, TRUE);
}

void
change_luck(register schar n)
{
    u.uluck += n;
    if (u.uluck < 0 && u.uluck < LUCKMIN) u.uluck = LUCKMIN;
    if (u.uluck > 0 && u.uluck > LUCKMAX) u.uluck = LUCKMAX;
}

int
stone_luck(boolean parameter) /* So I can't think up of a good name.  So sue me. --KAA */
{
    register struct obj *otmp;
    register long bonchance = 0;

    for (otmp = invent; otmp; otmp = otmp->nobj)
        if (confers_luck(otmp)) {
            if (otmp->cursed) bonchance -= otmp->quan;
            else if (otmp->blessed) bonchance += otmp->quan;
            else if (parameter) bonchance += otmp->quan;
        }

    return sgn((int)bonchance);
}

boolean
has_luckitem(void)
{
    register struct obj *otmp;

    for (otmp = invent; otmp; otmp = otmp->nobj)
        if (confers_luck(otmp)) return TRUE;
    return FALSE;
}

/* there has just been an inventory change affecting a luck-granting item */
void
set_moreluck(void)
{
    if (!has_luckitem()) u.moreluck = 0;
    else if (stone_luck(TRUE) >= 0) u.moreluck = LUCKADD;
    else u.moreluck = -LUCKADD;
}

void
restore_attrib(void)
{
    int i;

    for(i = 0; i < A_MAX; i++) {    /* all temporary losses/gains */

        if(ATEMP(i) && ATIME(i)) {
            if(!(--(ATIME(i)))) { /* countdown for change */
                ATEMP(i) += ATEMP(i) > 0 ? -1 : 1;

                if(ATEMP(i)) /* reset timer */
                    ATIME(i) = 100 / ACURR(A_CON);
            }
        }
    }
    (void)encumber_msg();
}

#define AVAL    50      /* tune value for exercise gains */

void
exercise(int i, boolean inc_or_dec)
{
#ifdef DEBUG
    pline("Exercise:");
#endif
    if (i == A_CHA) return;     /* INT can only be exercised by fighting
                                   a weeping angel's mental reflection. */

    /* no physical exercise while polymorphed; the body's temporary */
    if (Upolyd && i != A_WIS) return;

    if(abs(AEXE(i)) < AVAL) {
        /*
         *  Law of diminishing returns (Part I):
         *
         *  Gain is harder at higher attribute values.
         *  79% at "3" --> 0% at "18"
         *  Loss is even at all levels (50%).
         *
         *  Note: *YES* ACURR is the right one to use.
         */
        AEXE(i) += (inc_or_dec) ? (rn2(19) > ACURR(i)) : -rn2(2);
#ifdef DEBUG
        pline("%s, %s AEXE = %d",
              (i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" :
              (i == A_DEX) ? "Dex" : "Con",
              (inc_or_dec) ? "inc" : "dec", AEXE(i));
#endif
    }
    if (moves > 0 && (i == A_STR || i == A_CON)) (void)encumber_msg();
}

static void
exerper(void)
{
    if(!(moves % 10)) {
        /* Hunger Checks */

        int hs = (u.uhunger > 1000) ? SATIATED :
                 (u.uhunger > 150) ? NOT_HUNGRY :
                 (u.uhunger > 50) ? HUNGRY :
                 (u.uhunger > 0) ? WEAK : FAINTING;

#ifdef DEBUG
        pline("exerper: Hunger checks");
#endif
        switch (hs) {
        case SATIATED:  if (maybe_polyd(!is_vampire(youmonst.data),
                                        !Race_if(PM_VAMPIRE))) /* undead */
                exercise(A_DEX, FALSE);
            if (Role_if(PM_MONK))
                exercise(A_WIS, FALSE);
            break;
        case NOT_HUNGRY:    exercise(A_CON, TRUE); break;
        case WEAK:      exercise(A_STR, FALSE);
            if (Role_if(PM_MONK))           /* fasting */
                exercise(A_WIS, TRUE);
            break;
        case FAINTING:
        case FAINTED:   exercise(A_CON, FALSE); break;
        }

        /* Encumberance Checks */
#ifdef DEBUG
        pline("exerper: Encumber checks");
#endif
        switch (near_capacity()) {
        case MOD_ENCUMBER:  exercise(A_STR, TRUE); break;
        case HVY_ENCUMBER:  exercise(A_STR, TRUE);
            exercise(A_DEX, FALSE); break;
        case EXT_ENCUMBER:  exercise(A_DEX, FALSE);
            exercise(A_CON, FALSE); break;
        }

    }

    /* status checks */
    if(!(moves % 5)) {
#ifdef DEBUG
        pline("exerper: Status checks");
#endif
        if ((HClairvoyant & (INTRINSIC|TIMEOUT)) &&
            !BClairvoyant) exercise(A_WIS, TRUE);
        if (HRegeneration) exercise(A_STR, TRUE);

        if(Sick || Vomiting) exercise(A_CON, FALSE);
        if(Confusion || Hallucination) exercise(A_WIS, FALSE);
        if ((Wounded_legs && !u.usteed) || Fumbling || HStun) {
            exercise(A_DEX, FALSE);
        }
    }
}

void
exerchk(void)
{
    int i, mod_val;

    /*  Check out the periodic accumulations */
    exerper();

#ifdef DEBUG
    if(moves >= next_check)
        pline("exerchk: ready to test. multi = %d.", multi);
#endif
    /*  Are we ready for a test?    */
    if(moves >= next_check && !multi) {
#ifdef DEBUG
        pline("exerchk: testing.");
#endif
        /*
         *  Law of diminishing returns (Part II):
         *
         *  The effects of "exercise" and "abuse" wear
         *  off over time.  Even if you *don't* get an
         *  increase/decrease, you lose some of the
         *  accumulated effects.
         */
        for(i = 0; i < A_MAX; AEXE(i++) /= 2) {

            if(ABASE(i) >= 18 || !AEXE(i)) continue;
            if(i == A_CHA) continue; /* INT can only be exercised by fighting
                                        a weeping angel's mental reflection. */

#ifdef DEBUG
            pline("exerchk: testing %s (%d).",
                  (i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" :
                  (i == A_DEX) ? "Dex" : "Con", AEXE(i));
#endif
            /*
             *  Law of diminishing returns (Part III):
             *
             *  You don't *always* gain by exercising.
             *  [MRS 92/10/28 - Treat Wisdom specially for balance.]
             */
            if(rn2(AVAL) > ((i != A_WIS) ? abs(AEXE(i)*2/3) : abs(AEXE(i))))
                continue;
            mod_val = sgn(AEXE(i));

#ifdef DEBUG
            pline("exerchk: changing %d.", i);
#endif
            if(adjattrib(i, mod_val, -1)) {
#ifdef DEBUG
                pline("exerchk: changed %d.", i);
#endif
                /* if you actually changed an attrib - zero accumulation */
                AEXE(i) = 0;
                /* then print an explanation */
                switch(i) {
                case A_STR: You((mod_val >0) ?
                                "must have been exercising." :
                                "must have been abusing your body.");
                    break;
                case A_WIS: You((mod_val >0) ?
                                "must have been very observant." :
                                "haven't been paying attention.");
                    break;
                case A_DEX: You((mod_val >0) ?
                                "must have been working on your reflexes." :
                                "haven't been working on reflexes lately.");
                    break;
                case A_CON: You((mod_val >0) ?
                                "must be leading a healthy life-style." :
                                "haven't been watching your health.");
                    break;
                case A_INT: You((mod_val >0) ?
                                "must have been really concentrating lately." :
                                "haven't been thinking things through.");
                    break;
                }
            }
        }
        next_check += rn1(200, 800);
#ifdef DEBUG
        pline("exerchk: next check at %ld.", next_check);
#endif
    }
}

/* next_check will otherwise have its initial 600L after a game restore */
void
reset_attribute_clock(void)
{
    if (moves > 600L) next_check = moves + rn1(50, 800);
}


void
init_attr(register int np)
{
    register int i, x, tryct;


    for(i = 0; i < A_MAX; i++) {
        ABASE(i) = AMAX(i) = urole.attrbase[i];
        ATEMP(i) = ATIME(i) = 0;
        np -= urole.attrbase[i];
    }

    tryct = 0;
    while(np > 0 && tryct < 100) {

        x = rn2(100);
        for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++);
        if(i >= A_MAX) continue; /* impossible */

        if(ABASE(i) >= ATTRMAX(i)) {

            tryct++;
            continue;
        }
        tryct = 0;
        ABASE(i)++;
        AMAX(i)++;
        np--;
    }

    tryct = 0;
    while(np < 0 && tryct < 100) {      /* for redistribution */

        x = rn2(100);
        for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++);
        if(i >= A_MAX) continue; /* impossible */

        if(ABASE(i) <= ATTRMIN(i)) {

            tryct++;
            continue;
        }
        tryct = 0;
        ABASE(i)--;
        AMAX(i)--;
        np++;
    }
}

void
redist_attr(void)
{
    register int i, tmp;

    for(i = 0; i < A_MAX; i++) {
        if (i==A_INT || i==A_WIS) continue;
        /* Polymorphing doesn't change your mind */
        tmp = AMAX(i);
        AMAX(i) += (rn2(5)-2);
        if (AMAX(i) > ATTRMAX(i)) AMAX(i) = ATTRMAX(i);
        if (AMAX(i) < ATTRMIN(i)) AMAX(i) = ATTRMIN(i);
        ABASE(i) = ABASE(i) * AMAX(i) / tmp;
        /* ABASE(i) > ATTRMAX(i) is impossible */
        if (ABASE(i) < ATTRMIN(i)) ABASE(i) = ATTRMIN(i);
    }
    (void)encumber_msg();
}

static void
postadjabil(long int *ability)
{
    if (!ability) return;
    if (ability == &(HWarning) || ability == &(HSee_invisible))
        see_monsters();
}

void
adjabil(int oldlevel, int newlevel)
{
    register const struct innate *abil, *rabil;
    long mask = FROMEXPER;


    switch (Role_switch) {
    case PM_ARCHEOLOGIST:   abil = arc_abil;    break;
    case PM_BARBARIAN:      abil = bar_abil;    break;
    case PM_CAVEMAN:        abil = cav_abil;    break;
    case PM_CONVICT:        abil = con_abil;    break;
    case PM_HEALER:         abil = hea_abil;    break;
    case PM_KNIGHT:         abil = kni_abil;    break;
    case PM_MONK:           abil = mon_abil;    break;
    case PM_PRIEST:         abil = pri_abil;    break;
    case PM_RANGER:         abil = ran_abil;    break;
    case PM_ROGUE:          abil = rog_abil;    break;
    case PM_SAMURAI:        abil = sam_abil;    break;
    case PM_TOURIST:        abil = tou_abil;    break;
    case PM_VALKYRIE:       abil = val_abil;    break;
    case PM_WIZARD:         abil = wiz_abil;    break;
    default:                abil = 0;       break;
    }

    switch (Race_switch) {
    case PM_ELF:            rabil = elf_abil;   break;
    case PM_ORC:            rabil = orc_abil;   break;
    case PM_HUMAN:
    case PM_DWARF:
    case PM_GNOME:
    case PM_VAMPIRE:
    default:                rabil = 0;      break;
    }

    while (abil || rabil) {
        long prevabil;
        /* Have we finished with the intrinsics list? */
        if (!abil || !abil->ability) {
            /* Try the race intrinsics */
            if (!rabil || !rabil->ability) break;
            abil = rabil;
            rabil = 0;
            mask = FROMRACE;
        }
        prevabil = *(abil->ability);
        if(oldlevel < abil->ulevel && newlevel >= abil->ulevel) {
            /* Abilities gained at level 1 can never be lost
             * via level loss, only via means that remove _any_
             * sort of ability.  A "gain" of such an ability from
             * an outside source is devoid of meaning, so we set
             * FROMOUTSIDE to avoid such gains.
             */
            if (abil->ulevel == 1)
                *(abil->ability) |= (mask|FROMOUTSIDE);
            else
                *(abil->ability) |= mask;
            if(!(*(abil->ability) & INTRINSIC & ~mask)) {
                if(*(abil->gainstr))
                    You_feel("%s!", abil->gainstr);
            }
        } else if (oldlevel >= abil->ulevel && newlevel < abil->ulevel) {
            *(abil->ability) &= ~mask;
            if(!(*(abil->ability) & INTRINSIC)) {
                if(*(abil->losestr))
                    You_feel("%s!", abil->losestr);
                else if(*(abil->gainstr))
                    You_feel("less %s!", abil->gainstr);
            }
        }
        if (prevabil != *(abil->ability))   /* it changed */
            postadjabil(abil->ability);
        abil++;
    }

    if (oldlevel > 0) {
        if (newlevel > oldlevel)
            add_weapon_skill(newlevel - oldlevel);
        else
            lose_weapon_skill(oldlevel - newlevel);
    }
}


int
newhp(void)
{
    int hp, conplus;


    if (u.ulevel == 0) {
        /* Initialize hit points */
        hp = urole.hpadv.infix + urace.hpadv.infix;
        if (urole.hpadv.inrnd > 0) hp += rnd(urole.hpadv.inrnd);
        if (urace.hpadv.inrnd > 0) hp += rnd(urace.hpadv.inrnd);

        /* Initialize alignment stuff */
        u.ualign.type = aligns[flags.initalign].value;
        u.ualign.record = urole.initrecord;

        return hp;
    } else {
        if (u.ulevel < urole.xlev) {
            hp = urole.hpadv.lofix + urace.hpadv.lofix;
            if (urole.hpadv.lornd > 0) hp += rnd(urole.hpadv.lornd);
            if (urace.hpadv.lornd > 0) hp += rnd(urace.hpadv.lornd);
        } else {
            hp = urole.hpadv.hifix + urace.hpadv.hifix;
            if (urole.hpadv.hirnd > 0) hp += rnd(urole.hpadv.hirnd);
            if (urace.hpadv.hirnd > 0) hp += rnd(urace.hpadv.hirnd);
        }
    }

    if (ACURR(A_CON) <= 3) conplus = -2;
    else if (ACURR(A_CON) <= 6) conplus = -1;
    else if (ACURR(A_CON) <= 14) conplus = 0;
    else if (ACURR(A_CON) <= 16) conplus = 1;
    else if (ACURR(A_CON) == 17) conplus = 2;
    else if (ACURR(A_CON) == 18) conplus = 3;
    else conplus = 4;

    hp += conplus;

    if (hp <= 0) {
        hp = 1;
    }
    if (u.ulevel < MAXULEV) {
        u.uhpinc[u.ulevel] = (xchar) hp;
    }
    return hp;
}

schar
acurr(int x)
{
    register int tmp = (u.abon.a[x] + u.atemp.a[x] + u.acurr.a[x]);

    if (x == A_STR) {
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER) return(125);
        if ((uwep && (artilist[uwep->oartifact].spfx & SPFX_STR)) ||
            (uswapwep && (artilist[uswapwep->oartifact].spfx & SPFX_STR))) {
            return (118);
        }
#ifdef WIN32_BUG
        else return(x=((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#else
        else return((schar)((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#endif
    } else if (x == A_CON) {
        if (uwep && (artilist[uwep->oartifact].spfx & SPFX_CON)) return (25);
    } else if (x == A_CHA) {
        if (tmp < 18 && (youmonst.data->mlet == S_NYMPH ||
                         u.umonnum==PM_SUCCUBUS || u.umonnum == PM_INCUBUS))
            return 18;
    } else if (x == A_INT || x == A_WIS) {
        /* yes, this may raise int/wis if player is sufficiently
         * stupid.  there are lower levels of cognition than "dunce".
         */
        if (uarmh && uarmh->otyp == DUNCE_CAP) return(6);
    }
#ifdef WIN32_BUG
    return(x=((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#else
    return((schar)((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#endif
}

/* condense clumsy ACURR(A_STR) value into value that fits into game formulas
 */
schar
acurrstr(void)
{
    register int str = ACURR(A_STR);

    if (str <= 18) return((schar)str);
    if (str <= 121) return((schar)(19 + str / 50)); /* map to 19-21 */
    else return((schar)(str - 100));
}

/* avoid possible problems with alignment overflow, and provide a centralized
 * location for any future alignment limits
 */
void
adjalign(register int n)
{
    register int newalign = u.ualign.record + n;

    if(n < 0) {
        if(newalign < u.ualign.record)
            u.ualign.record = newalign;
    } else
    if(newalign > u.ualign.record) {
        u.ualign.record = newalign;
        if(u.ualign.record > ALIGNLIM)
            u.ualign.record = ALIGNLIM;
    }
}

/** Returns "beautiful", "handsome" or "ugly"
 * according to gender and charisma.
 */
const char *
beautiful(void)
{
    return (ACURR(A_CHA) > 14 ?
            (poly_gender()==1 ? "beautiful" : "handsome") :
            "ugly");
}

/** Returns the hitpoints of your current form. */
int
uhp(void)
{
    return (Upolyd ? u.mh : u.uhp);
}

/** Returns the maximal hitpoints of your current form. */
int
uhpmax(void)
{
    return (Upolyd ? u.mhmax : u.uhpmax);
}

/* when wearing (or taking off) an unID'd item, this routine is used
   to distinguish between observable +0 result and no-visible-effect
   due to an attribute not being able to exceed maximum or minimum */
boolean
extremeattr(int attrindx) /* does attrindx's value match its max or min? */

{
    /* Fixed_abil and racial MINATTR/MAXATTR aren't relevant here */
    int lolimit = 3, hilimit = 25, curval = ACURR(attrindx);

    /* upper limit for Str is 25 but its value is encoded differently */
    if (attrindx == A_STR) {
        hilimit = STR19(25); /* 125 */
        /* lower limit for Str can also be 25 */
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER) {
            lolimit = hilimit;
        }
    } else if (attrindx == A_CON) {
        if (uwep && (artilist[uwep->oartifact].spfx & SPFX_CON)) {
            lolimit = hilimit;
        }
    }
    /* this exception is hypothetical; the only other worn item affecting
       Int or Wis is another helmet so can't be in use at the same time */
    if (attrindx == A_INT || attrindx == A_WIS) {
        if (uarmh && uarmh->otyp == DUNCE_CAP) {
            hilimit = lolimit = 6;
        }
    }

    /* are we currently at either limit? */
    return (curval == lolimit || curval == hilimit) ? TRUE : FALSE;
}

/* change hero's alignment type, possibly losing use of artifacts */
void
uchangealign(int newalign,
             int reason) /**< 0==conversion, 1==helm-of-OA on, 2==helm-of-OA off */
{
    aligntyp oldalign = u.ualign.type;

    u.ublessed = 0; /* lose divine protection */
    /* You/Your/pline message with call flush_screen(), triggering bot(),
       so the actual data change needs to come before the message */
    flags.botl = TRUE; /* status line needs updating */
    if (reason == 0) {
        /* conversion via altar */
        livelog_printf(LL_ALIGNMENT, "permanently converted to %s", aligns[1 - newalign].adj);
        u.ualignbase[A_CURRENT] = (aligntyp) newalign;
        /* worn helm of opposite alignment might block change */
        if (!uarmh || uarmh->otyp != HELM_OF_OPPOSITE_ALIGNMENT) {
            u.ualign.type = u.ualignbase[A_CURRENT];
        }
        You("have a %ssense of a new direction.", (u.ualign.type != oldalign) ? "sudden " : "");
    } else {
        /* putting on or taking off a helm of opposite alignment */
        if (reason == 1) {
            /* don't livelog taking it back off */
            livelog_printf(LL_ALIGNMENT, "used a helm to turn %s", aligns[1 - newalign].adj);
        }
        u.ualign.type = (aligntyp) newalign;
        if (reason == 1) {
            Your("mind oscillates %s.", Hallucination ? "wildly" : "briefly");
        } else if (reason == 2) {
            Your("mind is %s.",
                 Hallucination ? "much of a muchness" : "back in sync with your body");
        }
    }
    if (u.ualign.type != oldalign) {
        u.ualign.record = 0; /* slate is wiped clean */
        retouch_equipment(0);
    }
}

/*attrib.c*/
