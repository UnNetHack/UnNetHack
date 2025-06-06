/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you're using precompiled headers, you don't want this either */
#ifdef MICROPORT_BUG
#define MKROOM_H
#endif

#include "hack.h"
#include "mfndpos.h"
#include <ctype.h>

static boolean vamp_rise_msg, disintegested;

static void sanity_check_single_mon(struct monst *, boolean, const char *);
static boolean restrap(struct monst *);
static long mm_2way_aggression(struct monst *, struct monst *);
static long mm_aggression(struct monst *, struct monst *);
static long mm_displacement(struct monst *, struct monst *);
static int pick_animal(void);
static void kill_eggs(struct obj *);
static int pickvampshape(struct monst *);
static boolean isspecmon(struct monst *);
static boolean validspecmon(struct monst *, int);
static struct permonst *accept_newcham_form(struct monst *, int);
static struct obj *make_corpse(struct monst *, unsigned);
static void m_detach(struct monst *, struct permonst *);
#ifdef WEBB_DISINT
static void lifesaved_monster(struct monst *, uchar);
#else
static void lifesaved_monster(struct monst *);
#endif
static void migrate_mon(struct monst *, xint16, xint16);
static boolean ok_to_obliterate(struct monst *);
static void deal_with_overcrowding(struct monst *);

#ifdef REINCARNATION
#define LEVEL_SPECIFIC_NOCORPSE(mdat) \
    (Is_rogue_level(&u.uz) || \
     (level.flags.graveyard && is_undead(mdat) && rn2(3)))
#else
#define LEVEL_SPECIFIC_NOCORPSE(mdat) \
    (level.flags.graveyard && is_undead(mdat) && rn2(3))
#endif


#if 0
/* part of the original warning code which was replaced in 3.3.1 */
#define warnDelay 10
long lastwarntime;
int lastwarnlev;

const char *warnings[] = {
    "white", "pink", "red", "ruby", "purple", "black"
};

static void warn_effects();
#endif /* 0 */

static void
sanity_check_single_mon(struct monst *mtmp, boolean chk_geno, const char *msg)
{
    if (mtmp->data < &mons[LOW_PM] || mtmp->data >= &mons[NUMMONS]) {
        impossible("illegal mon data %s; mnum=%d (%s)", fmt_ptr((genericptr_t) mtmp->data), mtmp->mnum, msg);
    } else {
        int mndx = monsndx(mtmp->data);

        if (mtmp->mnum != mndx) {
            impossible("monster mnum=%d, monsndx=%d (%s)", mtmp->mnum, mndx, msg);
            mtmp->mnum = mndx;
        }
        if (DEADMONSTER(mtmp)) {
#if 0
            /* bad if not fmons list or if not vault guard */
            if (strcmp(msg, "fmon") || !mtmp->isgd)
                impossible("dead monster on %s; %s at <%d,%d>",
                           msg, mons[mndx].mname, mtmp->mx, mtmp->my);
#endif
            return;
        }
        if (chk_geno && (mvitals[mndx].mvflags & G_GENOD) != 0) {
            impossible("genocided %s in play (%s)", mons[mndx].mname, msg);
        }
    }
    if (mtmp->isshk && !has_eshk(mtmp)) {
        impossible("shk without eshk (%s)", msg);
    }
    if (mtmp->ispriest && !has_epri(mtmp)) {
        impossible("priest without epri (%s)", msg);
    }
    if (mtmp->isgd && !has_egd(mtmp)) {
        impossible("guard without egd (%s)", msg);
    }
    if (mtmp->isminion && !has_emin(mtmp)) {
        impossible("minion without emin (%s)", msg);
    }
    /* guardian angel on astral level is tame but has emin rather than edog */
    if (mtmp->mtame && !has_edog(mtmp) && !mtmp->isminion) {
        impossible("pet without edog (%s)", msg);
    }
}

void
mon_sanity_check(void)
{
    int x, y;
    struct monst *mtmp, *m;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        /* dead monsters should still have sane data */
        sanity_check_single_mon(mtmp, TRUE, "fmon");
        if (DEADMONSTER(mtmp) && !mtmp->isgd) {
            continue;
        }

        x = mtmp->mx, y = mtmp->my;
        if (!isok(x, y) && !(mtmp->isgd && x == 0 && y == 0)) {
            impossible("mon (%s, %s) claims to be at <%d,%d>?",
                       mtmp->data->mname, fmt_ptr(mtmp), x, y);
        } else if (mtmp == u.usteed) {
            /* steed is in fmon list but not on the map; its
               <mx,my> coordinates should match hero's location */
            if (x != u.ux || y != u.uy) {
                impossible("steed (%s, %s) claims to be at <%d,%d>?",
                               mtmp->data->mname, fmt_ptr(mtmp), x, y);
            }
        } else if (level.monsters[x][y] != mtmp) {
            impossible("mon (%s, %s) at <%d,%d> is not there!",
                       mtmp->data->mname, fmt_ptr(mtmp), x, y);
        } else if (mtmp->wormno) {
            sanity_check_worm(mtmp);
        /* some temp mstate bits can be expected for a mon on fmon, as part of
           removing it, but DEADMONSTER check above should skip those. */
        } else if (mon_offmap(mtmp)) {
            impossible("floor mon (%s) with mstate set to 0x%08lx",
                       fmt_ptr((genericptr_t) mtmp), mtmp->mstate);
        }
    }

    for (x = 1; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if ((mtmp = level.monsters[x][y]) != 0) {
                for (m = fmon; m; m = m->nmon) {
                    if (m == mtmp) {
                        break;
                    }
                }
                if (!m) {
                    impossible("map mon (%s, %s) at <%d,%d> not in fmon list!",
                               mtmp->data->mname, fmt_ptr(mtmp), x, y);
                } else if (mtmp == u.usteed) {
                    impossible("steed (%s, %s) is on the map at <%d,%d>!",
                               mtmp->data->mname, fmt_ptr(mtmp), x, y);
                } else if ((mtmp->mx != x || mtmp->my != y)
                         && mtmp->data != &mons[PM_LONG_WORM]) {
                    impossible("map mon (%s, %s) at <%d,%d> is found at <%d,%d>?",
                               mtmp->data->mname, fmt_ptr(mtmp), mtmp->mx, mtmp->my, x, y);
                }
            }
        }
    }

    for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
        sanity_check_single_mon(mtmp, FALSE, "migr");

        if ((mtmp->mstate & ~(MON_MIGRATING | MON_LIMBO | MON_ENDGAME_MIGR | MON_OFFMAP)) != 0L ||
             !(mtmp->mstate & MON_MIGRATING))
            impossible("migrating mon (%s) with mstate set to 0x%08lx",
                       fmt_ptr((genericptr_t) mtmp), mtmp->mstate);
    }

    wormno_sanity_check(); /* test for bogus worm tail */
}

void
remove_monster(coordxy x, coordxy y)
{
    if (level.monsters[x][y] &&
        (level.monsters[x][y]->data == &mons[PM_GIANT_TURTLE] &&
         (!level.monsters[x][y]->minvis || See_invisible))) {
        unblock_point(x, y);
    }
    level.monsters[x][y] = (struct monst *)0;
}

/* Would monster be OK with poison gas? */
/* Does not check for actual poison gas at the location. */
/* Returns one of M_POISONGAS_foo */
int
m_poisongas_ok(struct monst *mtmp)
{
    int px, py;
    boolean is_you = (mtmp == &youmonst);

    /* Non living, non breathing, immune monsters are not concerned */
    if (nonliving(mtmp->data) ||
         is_vampshifter(mtmp) ||
         breathless(mtmp->data) ||
         immune_poisongas(mtmp->data)) {
        return M_POISONGAS_OK;
    }
    /* not is_swimmer(); assume that non-fish are swimming on
       the surface and breathing the air above it periodically
       unless located at water spot on plane of water */
    px = is_you ? u.ux : mtmp->mx;
    py = is_you ? u.uy : mtmp->my;
    if ((mtmp->data->mlet == S_EEL || Is_waterlevel(&u.uz)) && is_pool(px, py)) {
        return M_POISONGAS_OK;
    }
    /* exclude monsters with poison gas breath attack:
       adult green dragon and Chromatic Dragon (and iron golem,
       but nonliving() and breathless() tests also catch that) */
    if (attacktype_fordmg(mtmp->data, AT_BREA, AD_DRST) ||
         attacktype_fordmg(mtmp->data, AT_BREA, AD_RBRE)) {
        return M_POISONGAS_OK;
    }
    if (is_you && (u.uinvulnerable || Breathless || Underwater)) {
        return M_POISONGAS_OK;
    }
    if (is_you ? Poison_resistance : resists_poison(mtmp)) {
        return M_POISONGAS_MINOR;
    }
    return M_POISONGAS_BAD;
}

/* return True if mon is capable of converting other monsters into zombies */
boolean
zombie_maker(struct monst *mon)
{
    struct permonst *pm = mon->data;

    if (mon->mcan) {
        return FALSE;
    }

    switch (pm->mlet) {
    case S_ZOMBIE:
        /* Z-class monsters that aren't actually zombies go here */
        if (pm == &mons[PM_GHOUL] || pm == &mons[PM_SKELETON]) {
            return FALSE;
        }
        return TRUE;

    case S_LICH:
        /* all liches will create zombies as well */
        return TRUE;
    }

    return FALSE;
}

/* Return monster index of zombie monster which this monster could
   be turned into, or NON_PM if it doesn't have a direct counterpart.
   Sort of the zombie-specific inverse of undead_to_corpse. */
int
zombie_form(struct permonst *pm)
{
    switch (pm->mlet) {
    case S_ZOMBIE: /* when already a zombie/ghoul/skeleton, will stay as is */
        return NON_PM;
    case S_KOBOLD:
        return PM_KOBOLD_ZOMBIE;

    case S_ORC:
        return PM_ORC_ZOMBIE;

    case S_GIANT:
        if (pm == &mons[PM_ETTIN]) {
            return PM_ETTIN_ZOMBIE;
        }
        return PM_GIANT_ZOMBIE;

    case S_HUMAN:
    case S_KOP:
        if (is_elf(pm)) {
            return PM_ELF_ZOMBIE;
        }
        return PM_HUMAN_ZOMBIE;

    case S_HUMANOID:
        if (is_dwarf(pm)) {
            return PM_DWARF_ZOMBIE;
        }
        break;

    case S_GNOME:
        return PM_GNOME_ZOMBIE;
    }
    return NON_PM;
}

/* convert the monster index of an undead to its living counterpart */
int
undead_to_corpse(int mndx)
{
    switch (mndx) {
    case PM_KOBOLD_ZOMBIE:
    case PM_KOBOLD_MUMMY:   mndx = PM_KOBOLD;  break;
    case PM_DWARF_ZOMBIE:
    case PM_DWARF_MUMMY:    mndx = PM_DWARF;  break;
    case PM_GNOME_ZOMBIE:
    case PM_GNOME_MUMMY:    mndx = PM_GNOME;  break;
    case PM_ORC_ZOMBIE:
    case PM_ORC_MUMMY:  mndx = PM_ORC;  break;
    case PM_ELF_ZOMBIE:
    case PM_ELF_MUMMY:  mndx = PM_ELF;  break;
    case PM_VAMPIRE:
    case PM_VAMPIRE_LORD:
#if 0   /* DEFERRED */
    case PM_VAMPIRE_MAGE:
#endif
    case PM_HUMAN_ZOMBIE:
    case PM_HUMAN_MUMMY:    mndx = PM_HUMAN;  break;
    case PM_GIANT_ZOMBIE:
    case PM_GIANT_MUMMY:    mndx = PM_GIANT;  break;
    case PM_ETTIN_ZOMBIE:
    case PM_ETTIN_MUMMY:    mndx = PM_ETTIN;  break;
    default:  break;
    }
    return mndx;
}

/* Convert the monster index of some monsters (such as quest guardians)
 * to their generic species type.
 *
 * Return associated character class monster, rather than species
 * if mode is 1.
 */
int
genus(int mndx, int mode)
{
    switch (mndx) {
/* Quest guardians */
    case PM_STUDENT:     mndx = mode ? PM_ARCHEOLOGIST  : PM_HUMAN; break;
    case PM_CHIEFTAIN:   mndx = mode ? PM_BARBARIAN : PM_HUMAN; break;
    case PM_NEANDERTHAL: mndx = mode ? PM_CAVEMAN   : PM_HUMAN; break;
    case PM_ATTENDANT:   mndx = mode ? PM_HEALER    : PM_HUMAN; break;
    case PM_PAGE:        mndx = mode ? PM_KNIGHT    : PM_HUMAN; break;
    case PM_ABBOT:       mndx = mode ? PM_MONK      : PM_HUMAN; break;
    case PM_ACOLYTE:     mndx = mode ? PM_PRIEST    : PM_HUMAN; break;
    case PM_HUNTER:      mndx = mode ? PM_RANGER    : PM_HUMAN; break;
    case PM_THUG:        mndx = mode ? PM_ROGUE     : PM_HUMAN; break;
    case PM_ROSHI:       mndx = mode ? PM_SAMURAI   : PM_HUMAN; break;
    case PM_GUIDE:       mndx = mode ? PM_TOURIST   : PM_HUMAN; break;
    case PM_APPRENTICE:  mndx = mode ? PM_WIZARD    : PM_HUMAN; break;
    case PM_WARRIOR:     mndx = mode ? PM_VALKYRIE  : PM_HUMAN; break;
    default:
        if (mndx >= LOW_PM && mndx < NUMMONS) {
            struct permonst *ptr = &mons[mndx];
            if (is_human(ptr)) mndx = PM_HUMAN;
            else if (is_elf(ptr)) mndx = PM_ELF;
            else if (is_dwarf(ptr)) mndx = PM_DWARF;
            else if (is_gnome(ptr)) mndx = PM_GNOME;
            else if (is_orc(ptr)) mndx = PM_ORC;
        }
        break;
    }
    return mndx;
}

/* return monster index if chameleon, or NON_PM if not */
int
pm_to_cham(int mndx)
{
    int mcham = NON_PM;

    /*
     * As of 3.6.0 we just check M2_SHAPESHIFTER instead of having a
     * big switch statement with hardcoded shapeshifter types here.
     */
    if (mndx >= LOW_PM && is_shapeshifter(&mons[mndx])) {
        mcham = mndx;
    }
    return mcham;
}

/* for deciding whether corpse will carry along full monster data */
#define KEEPTRAITS(mon) ((mon)->isshk || (mon)->mtame || \
                         unique_corpstat((mon)->data) || \
                         is_reviver((mon)->data) || \
                         /* normally leader the will be unique, */ \
                         /* but they might have been polymorphed  */ \
                         (mon)->m_id == quest_status.leader_m_id || \
                         /* special cancellation handling for these */  \
                         (dmgtype((mon)->data, AD_SEDU) ||      \
                          dmgtype((mon)->data, AD_SSEX)))

/* Creates a monster corpse, a "special" corpse, or nothing if it doesn't
 * leave corpses.  Monsters which leave "special" corpses should have
 * G_NOCORPSE set in order to prevent wishing for one, finding tins of one,
 * etc....
 */
static struct obj *
make_corpse(struct monst *mtmp, unsigned int corpseflags)
{
    struct permonst *mdat = mtmp->data;
    int num;
    struct obj *obj = (struct obj *)0;
    int x = mtmp->mx, y = mtmp->my;
    int mndx = monsndx(mdat);
    unsigned corpstatflags = corpseflags;
    boolean burythem = ((corpstatflags & CORPSTAT_BURIED) != 0);

    switch (mndx) {
    case PM_GRAY_DRAGON:
    case PM_SILVER_DRAGON:
#if 0   /* DEFERRED */
    case PM_SHIMMERING_DRAGON:
#endif
    case PM_RED_DRAGON:
    case PM_ORANGE_DRAGON:
    case PM_WHITE_DRAGON:
    case PM_BLACK_DRAGON:
    case PM_BLUE_DRAGON:
    case PM_GREEN_DRAGON:
    case PM_GLOWING_DRAGON:
    case PM_YELLOW_DRAGON:
    case PM_CHROMATIC_DRAGON: {
        /* Make dragon scales.  This assumes that the order of the
         * dragons is the same as the order of the scales. */

        /* With reviving the dead corpses, this should result in a 50%
         * chance of getting the chromatic dragon scales in the Dragon Caves. */
        int chance = 0;
        if (mtmp->mrevived == 0) {
            chance = mndx == PM_CHROMATIC_DRAGON ? 6 : 3;
        } else if (mtmp->mrevived == 1) {
            chance = 20;
        } else {
            if (canseemon(mtmp)) {
                pline("%s scales look quite %s...", s_suffix(Monnam(mtmp)), hcolor("brittle"));
            }
        }
        if (chance > 0 && rnf(1, chance)) {
            num = GRAY_DRAGON_SCALES + monsndx(mdat) - PM_GRAY_DRAGON;
            obj = mksobj_at(num, x, y, FALSE, FALSE);
            obj->spe = 0;
            obj->cursed = obj->blessed = FALSE;
        }
        goto default_1;
    }

    case PM_TIAMAT:
        /* Make chromatic dragon scales from Tiamat's body only once. */
        if (mtmp->mrevived == 0) {
            obj = mksobj_at(CHROMATIC_DRAGON_SCALES, x, y, FALSE, FALSE);
            obj->spe = 0;
            obj->cursed = obj->blessed = FALSE;
        }
        goto default_1;

    case PM_WHITE_UNICORN:
    case PM_GRAY_UNICORN:
    case PM_BLACK_UNICORN:
        if ((mtmp->mrevived > 1) || (mtmp->mrevived == 1 && rn2(20))) {
            if (canseemon(mtmp)) {
                pline("%s recently regrown horn crumbles to dust.", s_suffix(Monnam(mtmp)));
            }
        } else {
            (void) mksobj_at(UNICORN_HORN, x, y, TRUE, FALSE);
        }
        goto default_1;
    case PM_LONG_WORM:
        (void) mksobj_at(WORM_TOOTH, x, y, TRUE, FALSE);
        goto default_1;
    case PM_VAMPIRE:
    case PM_VAMPIRE_LORD:
#if 0   /* DEFERRED */
    case PM_VAMPIRE_MAGE:
#endif
        /* include mtmp in the mkcorpstat() call */
        num = undead_to_corpse(mndx);
        corpstatflags |= CORPSTAT_INIT;
        obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, corpstatflags);
        obj->age -= 100; /* this is an *OLD* corpse */
        break;
    case PM_KOBOLD_MUMMY:
    case PM_DWARF_MUMMY:
    case PM_GNOME_MUMMY:
    case PM_ORC_MUMMY:
    case PM_ELF_MUMMY:
    case PM_HUMAN_MUMMY:
    case PM_GIANT_MUMMY:
    case PM_ETTIN_MUMMY:
    case PM_KOBOLD_ZOMBIE:
    case PM_DWARF_ZOMBIE:
    case PM_GNOME_ZOMBIE:
    case PM_ORC_ZOMBIE:
    case PM_ELF_ZOMBIE:
    case PM_HUMAN_ZOMBIE:
    case PM_GIANT_ZOMBIE:
    case PM_ETTIN_ZOMBIE:
        num = undead_to_corpse(mndx);
        corpstatflags |= CORPSTAT_INIT;
        obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, corpstatflags);
        obj->age -= 100; /* this is an *OLD* corpse */
        break;
    case PM_IRON_GOLEM:
        num = d(2, 6);
        while (num--)
            obj = mksobj_at(IRON_CHAIN, x, y, TRUE, FALSE);
        free_mgivenname(mtmp); /* don't christen obj */
        break;
    case PM_GLASS_GOLEM:
        num = d(2, 4); /* very low chance of creating all glass gems */
        while (num--)
            obj = mksobj_at((LAST_GEM + rnd(9)), x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    case PM_CLAY_GOLEM:
        obj = mksobj_at(ROCK, x, y, FALSE, FALSE);
        obj->quan = (long)(rn2(20) + 50);
        obj->owt = weight(obj);
        free_mgivenname(mtmp);
        break;
    case PM_STONE_GOLEM:
        corpstatflags &= ~CORPSTAT_INIT;
        obj = mkcorpstat(STATUE, (struct monst *) 0, mdat, x, y, corpstatflags);
        break;
    case PM_WOOD_GOLEM:
        num = d(2, 4);
        while (num--) {
            obj = mksobj_at(QUARTERSTAFF, x, y, TRUE, FALSE);
        }
        free_mgivenname(mtmp);
        break;
    case PM_LEATHER_GOLEM:
        num = d(2, 4);
        while (num--) {
            obj = mksobj_at(LEATHER_ARMOR, x, y, TRUE, FALSE);
        }
        free_mgivenname(mtmp);
        break;
    case PM_WAX_GOLEM:
        num = d(2, 4);
        while (num--)
            obj = mksobj_at(WAX_CANDLE, x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    case PM_GOLD_GOLEM:
        /* Good luck gives more coins */
        obj = mkgold((long)(200 - rnl(101)), x, y);
        free_mgivenname(mtmp);
        break;
    case PM_PAPER_GOLEM:
        num = rnd(4);
        while (num--)
            obj = mksobj_at(SCR_BLANK_PAPER, x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    case PM_SKELETON:
        if (!rn2(40)) {
            mksobj_at(SKELETON_KEY, x, y, TRUE, FALSE);
        }
        goto default_1;
default_1:
    default:
        if (mvitals[mndx].mvflags & G_NOCORPSE) {
            return (struct obj *)0;
        } else {
            corpstatflags |= CORPSTAT_INIT;
            /* preserve the unique traits of some creatures */
            obj = mkcorpstat(CORPSE, KEEPTRAITS(mtmp) ? mtmp : 0,
                             mdat, x, y, corpstatflags);
            obj->mrevived = mtmp->mrevived;
            if (burythem) {
                boolean dealloc;

                (void) bury_an_obj(obj, &dealloc);
                newsym(x, y);
                return dealloc ? (struct obj *) 0 : obj;
            }
        }
        break;
    }
    /* All special cases should precede the G_NOCORPSE check */

    if (!obj) {
        return (struct obj *)0;
    }

    /* if polymorph or undead turning has killed this monster,
       prevent the same attack beam from hitting its corpse */
    if (flags.bypasses) bypass_obj(obj);

    if (has_mgivenname(mtmp)) {
        obj = oname(obj, MGIVENNAME(mtmp));
    }

    /* Avoid "It was hidden under a green mold corpse!"
     *  during Blind combat. An unseen monster referred to as "it"
     *  could be killed and leave a corpse.  If a hider then hid
     *  underneath it, you could be told the corpse type of a
     *  monster that you never knew was there without this.
     *  The code in hitmu() substitutes the word "something"
     *  if the corpses obj->dknown is 0.
     */
    if (Blind && !sensemon(mtmp)) obj->dknown = 0;

    stackobj(obj);
    newsym(x, y);
    return obj;
}

/* check mtmp and water/lava for compatibility, 0 (survived), 1 (died) */
int
minliquid(struct monst *mtmp)
{
    boolean inpool, inlava, inswamp, infountain, grounded, inair;

    grounded = !is_flyer(mtmp->data) && !is_floater(mtmp->data);
    /* there's no "above the surface" on the plane of water */
    inpool = (is_pool(mtmp->mx, mtmp->my) && grounded) || Is_waterlevel(&u.uz);
    inlava = is_lava(mtmp->mx, mtmp->my) && grounded;
    inswamp = is_swamp(mtmp->mx, mtmp->my) && grounded;
    infountain = IS_FOUNTAIN(levl[mtmp->mx][mtmp->my].typ);
    inair = (is_open_air(mtmp->mx, mtmp->my) && grounded(mtmp->data));

    /* Flying and levitation keeps our steed out of the liquid
       (but not water-walking or swimming; note: if hero is in a
       water location on the Plane of Water, flight and levitating
       are blocked so this (Flying || Levitation) test fails there
       and steed will be subject to water effects, as intended) */
    if (mtmp == u.usteed && (Flying || Levitation))
        return 0;

    /* Gremlin multiplying won't go on forever since the hit points
     * keep going down, and when it gets to 1 hit point the clone
     * function will fail.
     */
    if (mtmp->data == &mons[PM_GREMLIN] && (inpool || infountain || inswamp) && rn2(3)) {
        if (split_mon(mtmp, (struct monst *)0))
            dryup(mtmp->mx, mtmp->my, FALSE);
        if (inpool) {
            water_damage_chain(mtmp->minvent, FALSE);
        }
        return 0;
    } else if (mtmp->data == &mons[PM_IRON_GOLEM] && (inpool || inswamp) && !rn2(5)) {
        int dam = d(2, 6);
        if (cansee(mtmp->mx, mtmp->my))
            pline("%s rusts.", Monnam(mtmp));
        mtmp->mhp -= dam;
        if (mtmp->mhpmax > dam) mtmp->mhpmax -= dam;
        if (DEADMONSTER(mtmp)) {
            mondead(mtmp);
            if (DEADMONSTER(mtmp)) {
                return 1;
            }
        }
        water_damage_chain(mtmp->minvent, FALSE);
        return 0;
    }

    if (inlava) {
        /*
         * Lava effects much as water effects. Lava likers are able to
         * protect their stuff. Fire resistant monsters can only protect
         * themselves  --ALI
         */
        if (!is_clinger(mtmp->data) && !likes_lava(mtmp->data)) {
            /* not fair...?  hero doesn't automatically teleport away
               from lava, just from water */
            if (can_teleport(mtmp->data) && !tele_restrict(mtmp)) {
                if (rloc(mtmp, RLOC_MSG)) {
                    return 0;
                }
            }
            if (!resists_fire(mtmp)) {
                if (cansee(mtmp->mx, mtmp->my)) {
                    struct attack *dummy = &mtmp->data->mattk[0];
                    const char *how = on_fire(mtmp->data, dummy);

                    pline("%s %s.", Monnam(mtmp),
                          !strcmp(how, "boiling") ? "boils away"
                             : !strcmp(how, "melting") ? "melts away"
                                : "burns to a crisp");
                }
                /* unlike fire -> melt ice -> pool, there's no way for the
                   hero to create lava beneath a monster, so the !mon_moving
                   case is not expected to happen (and we haven't made a
                   player-against-monster variation of the message above) */
                if (flags.mon_moving) {
                    mondead(mtmp);
                } else {
                    xkilled(mtmp, XKILL_NOMSG);
                }
            } else {
                mtmp->mhp -= 1;
                if (DEADMONSTER(mtmp)) {
                    if (cansee(mtmp->mx, mtmp->my))
                        pline("%s surrenders to the fire.", Monnam(mtmp));
                    mondead(mtmp);
                }
                else if (cansee(mtmp->mx, mtmp->my))
                    pline("%s burns slightly.", Monnam(mtmp));
            }
            if (!DEADMONSTER(mtmp)) {
                (void) fire_damage_chain(mtmp->minvent, FALSE, FALSE,
                                   mtmp->mx, mtmp->my);
                (void) rloc(mtmp, RLOC_MSG);
                return 0;
            }
            return 1;
        }
    } else if (inpool) {
        /* Most monsters drown in pools.  flooreffects() will take care of
         * water damage to dead monsters' inventory, but survivors need to
         * be handled here.  Swimmers are able to protect their stuff...
         */
        if (!is_clinger(mtmp->data)
            && !is_swimmer(mtmp->data) && !amphibious(mtmp->data)) {
            /* like hero with teleport intrinsic or spell, teleport away
               if possible */
            if (can_teleport(mtmp->data) && !tele_restrict(mtmp)) {
                if (rloc(mtmp, RLOC_MSG)) {
                    return 0;
                }
            }
            if (cansee(mtmp->mx, mtmp->my)) {
                pline("%s %s.", Monnam(mtmp),
                      is_vegetation(mtmp->data) ? "sinks" : "drowns");
            }
            if (u.ustuck && u.uswallow && u.ustuck == mtmp) {
                /* This can happen after a purple worm plucks you off a
                   flying steed while you are over water. */
                pline("%s sinks as %s rushes in and flushes you out.",
                      Monnam(mtmp), hliquid("water"));
            }
            if (flags.mon_moving) {
                mondead(mtmp);
            } else {
                xkilled(mtmp, XKILL_NOMSG);
            }
            if (!DEADMONSTER(mtmp)) {
                water_damage_chain(mtmp->minvent, FALSE);
                if (!rloc(mtmp, RLOC_NOMSG)) {
                    deal_with_overcrowding(mtmp);
                }
                return 0;
            }
            return 1;
        }
    } else if (inswamp) {
        if (!is_clinger(mtmp->data)
            && !is_swimmer(mtmp->data) && !amphibious(mtmp->data)) {
            water_damage_chain(mtmp->minvent, FALSE);
            return 0;
        }
    } else if (inair) { /* technically a gas, but... */
        mon_aireffects(mtmp);
        return 1; /* either sent to another level or dead */
    } else {
        /* but eels have a difficult time outside */
        if (mtmp->data->mlet == S_EEL && !Is_waterlevel(&u.uz)) {
            /* as mhp gets lower, the rate of further loss slows down */
            if (mtmp->mhp > 1 && rn2(mtmp->mhp) > rn2(8)) {
                mtmp->mhp--;
            }
            monflee(mtmp, 2, FALSE, FALSE);
        }
    }
    return 0;
}

int
mcalcmove(struct monst *mon)
{
    int mmove = mon->data->mmove;

    /* Note: MSLOW's `+ 1' prevents slowed speed 1 getting reduced to 0;
     *       MFAST's `+ 2' prevents hasted speed 1 from becoming a no-op;
     *       both adjustments have negligible effect on higher speeds.
     */
    if (mon->mspeed == MSLOW)
        mmove = (2 * mmove + 1) / 3;
    else if (mon->mspeed == MFAST)
        mmove = (4 * mmove + 2) / 3;

    if (mon == u.usteed) {
        if (u.ugallop && flags.mv) {
            /* increase movement by a factor of 1.5; also increase variance of
            movement speed (if it's naturally 24, we don't want it to always
            become 36) */
            mmove = ((rn2(2) ? 4 : 5) * mmove) / 3;
        }
    }

    return mmove;
}

/* actions that happen once per ``turn'', regardless of each
   individual monster's metabolism; some of these might need to
   be reclassified to occur more in proportion with movement rate */
void
mcalcdistress(void)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;

        /* must check non-moving monsters once/turn in case they managed
           to end up in water or lava; note: when not in liquid they regen,
           shape-shift, timeout temporary maladies just like other monsters */
        if (mtmp->data->mmove == 0) {
            if (vision_full_recalc) vision_recalc(0);
            if (minliquid(mtmp)) continue;
        }

        /* regenerate hit points */
        mon_regen(mtmp, FALSE);

        /* possibly polymorph shapechangers and lycanthropes */
        if (mtmp->cham >= LOW_PM) {
            decide_to_shapeshift(mtmp,
                    (canspotmon(mtmp) || (u.uswallow && mtmp == u.ustuck)) ? SHIFT_MSG : 0);
        }
        were_change(mtmp);

        /* gradually time out temporary problems */
        if (mtmp->mblinded && !--mtmp->mblinded)
            mtmp->mcansee = 1;
        if (mtmp->mfrozen && !--mtmp->mfrozen)
            mtmp->mcanmove = 1;
        if (mtmp->mfleetim && !--mtmp->mfleetim)
            mtmp->mflee = 0;

        /* FIXME: mtmp->mlstmv ought to be updated here */
    }
}

int
movemon(void)
{
    struct monst *mtmp, *nmtmp;
    boolean somebody_can_move = FALSE;
#if 0
    /* part of the original warning code which was replaced in 3.3.1 */
    warnlevel = 0;
#endif

    /*
       Some of you may remember the former assertion here that
       because of deaths and other actions, a simple one-pass
       algorithm wasn't possible for movemon.  Deaths are no longer
       removed to the separate list fdmon; they are simply left in
       the chain with hit points <= 0, to be cleaned up at the end
       of the pass.

       The only other actions which cause monsters to be removed from
       the chain are level migrations and losedogs().  I believe losedogs()
       is a cleanup routine not associated with monster movements, and
       monsters can only affect level migrations on themselves, not others
       (hence the fetching of nmon before moving the monster).  Currently,
       monsters can jump into traps, read cursed scrolls of teleportation,
       and drink cursed potions of raise level to change levels.  These are
       all reflexive at this point.  Should one monster be able to level
       teleport another, this scheme would have problems.
     */

    for (mtmp = fmon; mtmp; mtmp = nmtmp) {
        /* end monster movement early if hero is flagged to leave the level */
        if (u.utotype
#ifdef SAFERHANGUP
            /* or if the program has lost contact with the user */
            || program_state.done_hup
#endif
            ) {
            somebody_can_move = FALSE;
            break;
        }
        nmtmp = mtmp->nmon;
        /* one dead monster needs to perform a move after death: vault
           guard whose temporary corridor is still on the map; live
           guards who have led the hero back to civilization get moved
           off the map too; gd_move() decides whether the temporary
           corridor can be removed and guard discarded (via clearing
           mon->isgd flag so that dmonsfree() will get rid of mon) */
        if (mtmp->isgd && !mtmp->mx && !(mtmp->mstate & MON_MIGRATING)) {
            /* parked at <0,0>; eventually isgd should get set to false */
            if (monstermoves > mtmp->mlstmv) {
                (void) gd_move(mtmp);
                mtmp->mlstmv = monstermoves;
            }
            continue;
        }
        if (DEADMONSTER(mtmp)) {
            continue;
        }

        /* Find a monster that we have not treated yet.  */
        if(mtmp->movement < NORMAL_SPEED)
            continue;

        mtmp->movement -= NORMAL_SPEED;
        if (mtmp->movement >= NORMAL_SPEED)
            somebody_can_move = TRUE;

        if (vision_full_recalc) vision_recalc(0); /* vision! */

        /* reset obj bypasses before next monster moves */
        if (flags.bypasses) {
            clear_bypasses();
        }
        clear_splitobjs();
        if (minliquid(mtmp)) continue;

        /* after losing equipment, try to put on replacement */
        if (mtmp->misc_worn_check & I_SPECIAL) {
            long oldworn;

            mtmp->misc_worn_check &= ~I_SPECIAL;
            oldworn = mtmp->misc_worn_check;
            m_dowear(mtmp, FALSE);
            if (mtmp->misc_worn_check != oldworn || !mtmp->mcanmove) {
                continue;
            }
        }

        if (is_hider(mtmp->data)) {
            /* unwatched mimics and piercers may hide again  [MRS] */
            if(restrap(mtmp)) continue;
            if (M_AP_TYPE(mtmp) == M_AP_FURNITURE || M_AP_TYPE(mtmp) == M_AP_OBJECT) {
                continue;
            }
            if (mtmp->mundetected) {
                continue;
            }
        } else if (mtmp->data->mlet == S_EEL &&
                !mtmp->mundetected &&
                (mtmp->mflee || !next2u(mtmp->mx, mtmp->my)) &&
                !canseemon(mtmp) &&
                !rn2(4)) {
            /* some eels end up stuck in isolated pools, where they
               can't--or at least won't--move, so they never reach
               their post-move chance to re-hide */
            if (hideunder(mtmp)) {
                continue;
            }
        }

        /* continue if the monster died fighting */
        if (Conflict && !mtmp->iswiz && mtmp->mcansee) {
            /* Note:
             *  Conflict does not take effect in the first round.
             *  Therefore, A monster when stepping into the area will
             *  get to swing at you.
             *
             *  The call to fightm() must be _last_.  The monster might
             *  have died if it returns 1.
             */
            if (couldsee(mtmp->mx, mtmp->my) &&
                (distu(mtmp->mx, mtmp->my) <= BOLT_LIM*BOLT_LIM) &&
                fightm(mtmp))
                continue; /* mon might have died */
        }
        if (dochugw(mtmp, TRUE)) { /* otherwise just move the monster */
            continue;
        }
    }
#if 0
    /* part of the original warning code which was replaced in 3.3.1 */
    if(warnlevel > 0)
        warn_effects();
#endif

    if (any_light_source())
        vision_full_recalc = 1; /* in case a mon moved with a light source */
    /* reset obj bypasses after last monster has moved */
    if (flags.bypasses) {
        clear_bypasses();
    }
    clear_splitobjs();
    /* remove dead monsters; dead vault guard will be left at <0,0>
       if temporary corridor out of vault hasn't been removed yet */
    dmonsfree();

    /* a monster may have levteleported player -dlc */
    if (u.utotype) {
        deferred_goto();
        /* changed levels, so these monsters are dormant */
        somebody_can_move = FALSE;
    }

    return somebody_can_move;
}

#define mstoning(obj)   (ofood(obj) && \
                         (touch_petrifies(&mons[(obj)->corpsenm]) || \
                          (obj)->corpsenm == PM_MEDUSA))

/*
 * Maybe eat a metallic object (not just gold).
 * Return value: 0 => nothing happened, 1 => monster ate something,
 * 2 => monster died (it must have grown into a genocided form, but
 * that can't happen at present because nothing which eats objects
 * has young and old forms).
 */
int
meatmetal(struct monst *mtmp)
{
    struct obj *otmp;
    struct permonst *ptr;
    int poly, grow, heal, mstone;

    /* If a pet, eating is handled separately, in dog.c */
    if (mtmp->mtame) return 0;

    /* Eats topmost metal object if it is there */
    for (otmp = level.objects[mtmp->mx][mtmp->my];
         otmp; otmp = otmp->nexthere) {
        /* Don't eat indigestible/choking/inappropriate objects */
        if ((mtmp->data == &mons[PM_RUST_MONSTER] && !is_rustprone(otmp)) ||
            (otmp->otyp == AMULET_OF_STRANGULATION) ||
            (otmp->otyp == RIN_SLOW_DIGESTION)) {
            continue;
        }
        if (is_metallic(otmp) && !obj_resists(otmp, 5, 95) &&
            touch_artifact(otmp, mtmp)) {
            if (mtmp->data == &mons[PM_RUST_MONSTER] && otmp->oerodeproof) {
                if (canseemon(mtmp) && flags.verbose) {
                    pline("%s eats %s!",
                          Monnam(mtmp),
                          distant_name(otmp, doname));
                }
                /* The object's rustproofing is gone now */
                otmp->oerodeproof = 0;
                mtmp->mstun = 1;
                if (canseemon(mtmp) && flags.verbose) {
                    pline("%s spits %s out in disgust!",
                          Monnam(mtmp), distant_name(otmp, doname));
                }
            } else {
                if (cansee(mtmp->mx, mtmp->my) && flags.verbose)
                    pline("%s eats %s!", Monnam(mtmp),
                          distant_name(otmp, doname));
                else if (flags.verbose)
                    You_hear("a crunching sound.");
                mtmp->meating = otmp->owt/2 + 1;
                /* Heal up to the object's weight in hp */
                if (mtmp->mhp < mtmp->mhpmax) {
                    mtmp->mhp += objects[otmp->otyp].oc_weight;
                    if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
                }
                if(otmp == uball) {
                    unpunish();
                    delobj(otmp);
                } else if (otmp == uchain) {
                    unpunish(); /* frees uchain */
                } else {
                    poly = polyfodder(otmp);
                    grow = mlevelgain(otmp);
                    heal = mhealup(otmp);
                    mstone = mstoning(otmp);
                    delobj(otmp);
                    ptr = mtmp->data;
                    if (poly) {
                        if (newcham(mtmp, (struct permonst *)0,
                                    FALSE, FALSE))
                            ptr = mtmp->data;
                    } else if (grow) {
                        ptr = grow_up(mtmp, (struct monst *)0);
                    } else if (mstone) {
                        if (poly_when_stoned(ptr)) {
                            mon_to_stone(mtmp);
                            ptr = mtmp->data;
                        } else if (!resists_ston(mtmp)) {
                            if (canseemon(mtmp))
                                pline("%s turns to stone!", Monnam(mtmp));
                            monstone(mtmp);
                            ptr = (struct permonst *)0;
                        }
                    } else if (heal) {
                        mtmp->mhp = mtmp->mhpmax;
                    }
                    if (!ptr) return 2; /* it died */
                }
                /* Left behind a pile? */
                if (rnd(25) < 3)
                    (void)mksobj_at(ROCK, mtmp->mx, mtmp->my, TRUE, FALSE);
                newsym(mtmp->mx, mtmp->my);
                return 1;
            }
        }
    }
    return 0;
}

/* monster eats a pile of objects */
int
meatobj(struct monst *mtmp)       /* for gelatinous cubes */

{
    struct obj *otmp, *otmp2;
    struct permonst *ptr;
    int poly, grow, heal, count = 0, ecount = 0;
    char buf[BUFSZ];

    buf[0] = '\0';
    /* If a pet, eating is handled separately, in dog.c */
    if (mtmp->mtame) return 0;

    /* eat organic objects, including cloth and wood, if present;
       engulf others, except huge rocks and metal attached to player
       [despite comment at top, doesn't assume that eater is a g.cube] */
    for (otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;

        if (is_organic(otmp) && !obj_resists(otmp, 5, 95) &&
            touch_artifact(otmp, mtmp)) {
            if (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]) &&
                !resists_ston(mtmp))
                continue;
            if (otmp->otyp == AMULET_OF_STRANGULATION ||
                otmp->otyp == RIN_SLOW_DIGESTION)
                continue;
            if (Is_sokoprize(otmp)) continue;
            ++count;
            if (cansee(mtmp->mx, mtmp->my) && flags.verbose)
                pline("%s eats %s!", Monnam(mtmp),
                      distant_name(otmp, doname));
            else if (flags.soundok && flags.verbose)
                You_hear("a slurping sound.");
            /* Heal up to the object's weight in hp */
            if (mtmp->mhp < mtmp->mhpmax) {
                mtmp->mhp += objects[otmp->otyp].oc_weight;
                if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
            }
            if (Has_contents(otmp)) {
                struct obj *otmp3;
                /* contents of eaten containers become engulfed; this
                   is arbitrary, but otherwise g.cubes are too powerful */
                while ((otmp3 = otmp->cobj) != 0) {
                    obj_extract_self(otmp3);
                    if (otmp->otyp == ICE_BOX && otmp3->otyp == CORPSE) {
                        otmp3->age = monstermoves - otmp3->age;
                        start_corpse_timeout(otmp3);
                    }
                    (void) mpickobj(mtmp, otmp3);
                }
            }
            poly = polyfodder(otmp);
            grow = mlevelgain(otmp);
            heal = mhealup(otmp);
            delobj(otmp);   /* munch */
            ptr = mtmp->data;
            if (poly) {
                if (newcham(mtmp, (struct permonst *)0, FALSE, FALSE))
                    ptr = mtmp->data;
            } else if (grow) {
                ptr = grow_up(mtmp, (struct monst *)0);
            } else if (heal) {
                mtmp->mhp = mtmp->mhpmax;
            }
            /* in case it polymorphed or died */
            if (ptr != &mons[PM_GELATINOUS_CUBE])
                return !ptr ? 2 : 1;
        } else if (otmp->oclass != ROCK_CLASS &&
                   otmp != uball && otmp != uchain &&
                   !Is_sokoprize(otmp)) {
            ++ecount;
            if (ecount == 1) {
                Sprintf(buf, "%s engulfs %s.", Monnam(mtmp),
                        distant_name(otmp, doname));
            } else if (ecount == 2)
                Sprintf(buf, "%s engulfs several objects.", Monnam(mtmp));
            obj_extract_self(otmp);
            (void) mpickobj(mtmp, otmp); /* slurp */
        }
        /* Engulf & devour is instant, so don't set meating */
        if (mtmp->minvis) newsym(mtmp->mx, mtmp->my);
    }
    if (ecount > 0) {
        if (cansee(mtmp->mx, mtmp->my) && flags.verbose && buf[0])
            pline("%s", buf);
        else if (flags.soundok && flags.verbose)
            You_hear("%s slurping sound%s.",
                     ecount == 1 ? "a" : "several",
                     ecount == 1 ? "" : "s");
    }
    return ((count > 0) || (ecount > 0)) ? 1 : 0;
}

void
mpickgold(struct monst *mtmp)
{
    struct obj *gold;
    int mat_idx;

    if ((gold = g_at(mtmp->mx, mtmp->my)) != 0) {
        mat_idx = objects[gold->otyp].oc_material;
        obj_extract_self(gold);
        add_to_minv(mtmp, gold);
        if (cansee(mtmp->mx, mtmp->my) ) {
            if (flags.verbose && !mtmp->isgd)
                pline("%s picks up some %s.", Monnam(mtmp),
                      mat_idx == GOLD ? "gold" : "money");
            newsym(mtmp->mx, mtmp->my);
        }
    }
}

boolean
mpickstuff(struct monst *mtmp, const char *str)
{
    struct obj *otmp, *otmp2;

    /* let angry 1ES pick up stuff so she can smash boulders */
    if ((mtmp->data != &mons[PM_BLACK_MARKETEER]) && (!mtmp->mpeaceful))  {
/*  prevent shopkeepers from leaving the door of their shop */
        if(mtmp->isshk && inhishop(mtmp)) return FALSE;
    }

    for (otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;
        /*  Nymphs take everything.  Most monsters don't pick up corpses. */
        if (!str ? searches_for_item(mtmp, otmp) :
            !!(index(str, otmp->oclass))) {
            if (otmp->otyp == CORPSE && mtmp->data->mlet != S_NYMPH &&
                /* let a handful of corpse types thru to can_carry() */
                !touch_petrifies(&mons[otmp->corpsenm]) &&
                otmp->corpsenm != PM_LIZARD &&
                !acidic(&mons[otmp->corpsenm])) continue;
            if (!touch_artifact(otmp, mtmp)) continue;
            if (!can_carry(mtmp, otmp)) continue;
            if (is_pool(mtmp->mx, mtmp->my)) continue;
#ifdef INVISIBLE_OBJECTS
            if (otmp->oinvis && !perceives(mtmp->data)) continue;
#endif
            if (Is_sokoprize(otmp)) continue;

            /* let monster pickup the object */
            mpickup_obj(mtmp, otmp);

            return TRUE; /* pick only one object */
        }
    }
    return FALSE;
}

void
mpickup_obj(struct monst *mtmp, struct obj *otmp)
{
    struct obj *otmp3 = otmp;
    if (otmp->otyp == BOULDER && is_rockbreaker(mtmp->data)) {
        if (cansee(mtmp->mx, mtmp->my)) {
            pline("A thunderclap rings out, and %s shatters!", (distu(mtmp->mx, mtmp->my) <= 5) ? doname(otmp) : distant_name(otmp, doname));
            pline("%s strides through the dust cloud.", Monnam(mtmp));
        } else {
            pline("A thunderclap rings out!");
        }
    } else {
        int carryamt = can_carry(mtmp, otmp);
        if (carryamt != otmp->quan) {
            otmp3 = splitobj(otmp, carryamt);
        }
        if (cansee(mtmp->mx, mtmp->my) && flags.verbose) {
            pline("%s picks up %s.", Monnam(mtmp),
                  (distu(mtmp->mx, mtmp->my) <= 5) ?
                  doname(otmp3) : distant_name(otmp3, doname));
        }
    }
    if (otmp->otyp == BOULDER && is_rockbreaker(mtmp->data)) {
        remove_object(otmp);
    } else {
        obj_extract_self(otmp3);      /* remove from floor */
        (void) mpickobj(mtmp, otmp3); /* may merge and free otmp3 */
        m_dowear(mtmp, FALSE);
        newsym(mtmp->mx, mtmp->my);
    }
    m_dowear(mtmp, FALSE);
    newsym(mtmp->mx, mtmp->my);
}

int
curr_mon_load(struct monst *mtmp)
{
    int curload = 0;
    struct obj *obj;

    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        if (obj->otyp != BOULDER || !throws_rocks(mtmp->data)) {
            curload += obj->owt;
        }
    }

    return curload;
}

int
max_mon_load(struct monst *mtmp)
{
    long maxload;

    /* Base monster carrying capacity is equal to human maximum
     * carrying capacity, or half human maximum if not strong.
     * (for a polymorphed player, the value used would be the
     * non-polymorphed carrying capacity instead of max/half max).
     * This is then modified by the ratio between the monster weights
     * and human weights.  Corpseless monsters are given a capacity
     * proportional to their size instead of weight.
     */
    if (!mtmp->data->cwt)
        maxload = (MAX_CARR_CAP * (long)mtmp->data->msize) / MZ_HUMAN;
    else if (!strongmonst(mtmp->data)
             || (strongmonst(mtmp->data) && (mtmp->data->cwt > WT_HUMAN)))
        maxload = (MAX_CARR_CAP * (long)mtmp->data->cwt) / WT_HUMAN;
    else maxload = MAX_CARR_CAP;    /*strong monsters w/cwt <= WT_HUMAN*/

    if (!strongmonst(mtmp->data)) maxload /= 2;

    if (maxload < 1) maxload = 1;

    return (int) maxload;
}

/* for restricting monsters' object-pickup.
 *
 * to support the new pet behavior, this now returns the max # of objects
 * that a given monster could pick up from a pile. frequently this will be
 * otmp->quan, but special cases for 'only one' now exist so.
 *
 * this will probably cause very amusing behavior with pets and gold coins.
 *
 * TODO: allow picking up 2-N objects from a pile of N based on weight.
 *       Change from 'int' to 'long' to accomate big stacks of gold.
 *       Right now we fake it by reporting a partial quantity, but the
 *       likesgold handling m_move results in picking up the whole stack.
 */
int
can_carry(struct monst *mtmp, struct obj *otmp)
{
    int otyp = otmp->otyp, newload = otmp->owt;
    struct permonst *mdat = mtmp->data;

    if (notake(mdat)) {
        return 0; /* can't carry anything */
    }

    if (otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]) &&
        !(mtmp->misc_worn_check & W_ARMG) && !resists_ston(mtmp))
        return 0;
    if (otyp == CORPSE && is_rider(&mons[otmp->corpsenm]))
        return 0;
    if (objects[otyp].oc_material == SILVER && mon_hates_silver(mtmp) &&
        (otyp != BELL_OF_OPENING || !is_covetous(mdat)))
        return 0;

    /* hostile monsters who like gold will pick up the whole stack;
       tame monsters with hands will pick up the partial stack */
    int iquan = (otmp->quan > (long)LARGEST_INT) ? 20000 + rn2(LARGEST_INT-20000+1) : (int)otmp->quan;

    /* monsters without hands can't pick up multiple objects at once
     * unless they have an engulfing attack
     *
     * ...dragons, of course, can always carry gold pieces and gems somehow
     */
    if (iquan > 1) {
        boolean glomper = FALSE;

        if (mtmp->data->mlet == S_DRAGON &&
            (otmp->oclass == COIN_CLASS || otmp->oclass == GEM_CLASS)) {
            glomper = TRUE;
        } else {
            for (int nattk = 0; nattk < NATTK; nattk++) {
                if (mtmp->data->mattk[nattk].aatyp == AT_ENGL) {
                    glomper = TRUE;
                    break;
                }
            }
        }
        if (nohands(mtmp->data) && !glomper) {
            /* a big mouth can carry a lot */
            if (!has_head(mtmp->data)) {
                return 1;
            }
            /* limit how many gold pieces monsters with no hands can lift */
            if (otmp->oclass == COIN_CLASS) {
                if (iquan > 200) {
                    long new_iquan = d(6, 50);
                    if (new_iquan < otmp->quan) {
                        iquan = new_iquan;
                        long saved_quan = otmp->quan;
                        otmp->quan = iquan;
                        newload = weight(otmp);
                        otmp->quan = saved_quan;
                    }
                }
            }
        }
    }

    /* Steeds don't pick up stuff (to avoid shop abuse) */
    if (mtmp == u.usteed) {
        return 0;
    }

    if (mtmp->isshk) {
        return iquan; /* no limit */
    }
    if (mtmp->mpeaceful && !mtmp->mtame) {
        return 0;
    }
    /* otherwise players might find themselves obligated to violate
     * their alignment if the monster takes something they need
     */

    /* special--boulder throwers carry unlimited amounts of boulders
     * and some monsters can shatter boulders (will be handled in mpickstuff) */
    if ((throws_rocks(mdat) || is_rockbreaker(mdat)) && otyp == BOULDER) {
        return iquan;
    }

    /* nymphs deal in stolen merchandise, but not boulders or statues */
    if (mdat->mlet == S_NYMPH) {
        return (otmp->oclass == ROCK_CLASS) ? 0 : iquan;
    }

    if (curr_mon_load(mtmp) + newload > max_mon_load(mtmp)) {
        return 0;
    }

    return iquan;
}

/* return number of acceptable neighbour positions */
int
mfndpos(struct monst *mon, coord *poss, long int *info, long int flag)

             /* coord poss[9] */
             /* long info[9] */

{
    struct permonst *mdat = mon->data;
    coordxy x, y, nx, ny;
    int cnt = 0;
    uchar ntyp;
    uchar nowtyp;
    boolean wantpool, poolok, lavaok, nodiag, quantumlock;
    boolean rockok = FALSE, treeok = FALSE, thrudoor;
    int maxx, maxy;

    boolean poisongas_ok, in_poisongas;
    NhRegion *gas_reg;
    int gas_glyph = cmap_to_glyph(S_poisoncloud);

    x = mon->mx;
    y = mon->my;
    nowtyp = levl[x][y].typ;

    nodiag = NODIAG(mdat - mons);
    wantpool = mdat->mlet == S_EEL;
    poolok = ((!Is_waterlevel(&u.uz) && (is_flyer(mdat) || is_floater(mdat) || is_clinger(mdat))) ||
              (is_swimmer(mdat) && !wantpool));
    /* note: floating eye is the only is_floater() so this could be
       simplified, but then adding another floater would be error prone */
    lavaok = (is_flyer(mdat) || is_floater(mdat) || is_clinger(mdat) || likes_lava(mdat));
    quantumlock = (is_weeping(mdat));
    thrudoor = ((flag & (ALLOW_WALL|BUSTDOOR)) != 0L);
    poisongas_ok = ((nonliving(mdat) || is_vampshifter(mon) || breathless(mdat)) || resists_poison(mon));
    in_poisongas = ((gas_reg = visible_region_at(x,y)) != 0 && gas_reg->glyph == gas_glyph);

    if (flag & ALLOW_DIG) {
        struct obj *mw_tmp;

        /* need to be specific about what can currently be dug */
        if (!needspick(mdat)) {
            rockok = treeok = TRUE;
        } else if ((mw_tmp = MON_WEP(mon)) && mw_tmp->cursed &&
                   mon->weapon_check == NO_WEAPON_WANTED) {
            rockok = is_pick(mw_tmp);
            treeok = is_axe(mw_tmp);
        } else {
            rockok = (m_carrying(mon, PICK_AXE) ||
                      (m_carrying(mon, DWARVISH_MATTOCK) &&
                       !which_armor(mon, W_ARMS)));
            treeok = (m_carrying(mon, AXE) ||
                      (m_carrying(mon, BATTLE_AXE) &&
                       !which_armor(mon, W_ARMS)));
        }
        thrudoor |= rockok || treeok;
    }

nexttry:    /* eels prefer the water, but if there is no water nearby,
               they will crawl over land */
    if (mon->mconf) {
        flag |= ALLOW_ALL;
        flag &= ~NOTONL;
    }
    if (!mon->mcansee)
        flag |= ALLOW_SSM;
    maxx = min(x+1, COLNO-1);
    maxy = min(y+1, ROWNO-1);
    for (nx = max(1, x-1); nx <= maxx; nx++) {
        for (ny = max(0, y-1); ny <= maxy; ny++) {
            if (nx == x && ny == y) {
                continue;
            }
            ntyp = levl[nx][ny].typ;
            if (IS_ROCK(ntyp = levl[nx][ny].typ) &&
                 !((flag & ALLOW_WALL) && may_passwall(nx, ny)) &&
                 !((IS_TREE(ntyp) ? treeok : rockok) && may_dig(nx, ny))) {
                continue;
            }
            /* KMH -- Added iron bars */
            if (ntyp == IRONBARS &&
                (!(flag & ALLOW_BARS) ||
                 ((levl[nx][ny].wall_info & W_NONDIGGABLE) && (dmgtype(mdat, AD_RUST) || dmgtype(mdat, AD_CORR))))) {
                continue;
            }
            if (IS_DOOR(ntyp) && !amorphous(mdat) &&
                !(amorphous(mdat) || can_fog(mon)) &&
                ((levl[nx][ny].doormask & D_CLOSED && !(flag & OPENDOOR)) ||
                 (levl[nx][ny].doormask & D_LOCKED && !(flag & UNLOCKDOOR))) &&
                !thrudoor) {
                continue;
            }
            /* avoid poison gas? */
            if (!poisongas_ok &&
                !in_poisongas &&
                (gas_reg = visible_region_at(nx,ny)) &&
                (gas_reg->glyph == gas_glyph)) {
                continue;
            }
            if (nx != x && ny != y && (nodiag ||
#ifdef REINCARNATION
                                      ((IS_DOOR(nowtyp) &&
                                        ((levl[x][y].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))) ||
                                       (IS_DOOR(ntyp) &&
                                        ((levl[nx][ny].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))))
#else
                                      ((IS_DOOR(nowtyp) && (levl[x][y].doormask & ~D_BROKEN)) ||
                                       (IS_DOOR(ntyp) && (levl[nx][ny].doormask & ~D_BROKEN)))
#endif
                                      ))
                continue;
            if(nx != 0 && ny != 0 && canseemon(mon) && quantumlock)
                continue;
            /* avoid open air if mon would fall into it */
            if (is_open_air(nx, ny) && grounded(mdat)) {
                continue;
            }
            if((is_pool(nx, ny) == wantpool || poolok) &&
               (lavaok || !is_lava(nx, ny))) {
                int dispx, dispy;
                boolean monseeu = (mon->mcansee && (!Invis || perceives(mdat)));
                boolean checkobj = OBJ_AT(nx, ny);

                /* Displacement also displaces the Elbereth/scare monster,
                 * as long as you are visible.
                 */
                if (Displaced && monseeu && (mon->mux==nx) && (mon->muy==ny)) {
                    dispx = u.ux;
                    dispy = u.uy;
                } else {
                    dispx = nx;
                    dispy = ny;
                }

                info[cnt] = 0;
                if (onscary(dispx, dispy, mon)) {
                    if(!(flag & ALLOW_SSM)) continue;
                    info[cnt] |= ALLOW_SSM;
                }
                if (u_at(nx, ny) ||
                    (nx == mon->mux && ny == mon->muy)) {
                    if (u_at(nx, ny)) {
                        /* If it's right next to you, it found you,
                         * displaced or no.  We must set mux and muy
                         * right now, so when we return we can tell
                         * that the ALLOW_U means to attack _you_ and
                         * not the image.
                         */
                        mon->mux = u.ux;
                        mon->muy = u.uy;
                    }
                    if(!(flag & ALLOW_U)) continue;
                    info[cnt] |= ALLOW_U;
                } else {
                    if (MON_AT(nx, ny)) {
                        struct monst *mtmp2 = m_at(nx, ny);
                        long mmflag = flag | mm_aggression(mon, mtmp2);

                        if (mmflag & ALLOW_M) {
                            info[cnt] |= ALLOW_M;
                            if (mtmp2->mtame) {
                                if (!(mmflag & ALLOW_TM)) {
                                    continue;
                                }
                                info[cnt] |= ALLOW_TM;
                            }
                        } else {
                            mmflag = flag | mm_displacement(mon, mtmp2);
                            if (!(mmflag & ALLOW_MDISP)) {
                                continue;
                            }
                            info[cnt] |= ALLOW_MDISP;
                        }
                    }
                    /* Note: ALLOW_SANCT only prevents movement, not */
                    /* attack, into a temple. */
                    if (level.flags.has_temple &&
                       *in_rooms(nx, ny, TEMPLE) &&
                       !*in_rooms(x, y, TEMPLE) &&
                       in_your_sanctuary((struct monst *)0, nx, ny)) {
                        if(!(flag & ALLOW_SANCT)) continue;
                        info[cnt] |= ALLOW_SANCT;
                    }
                }
                if (checkobj && sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
                    if(flag & NOGARLIC) continue;
                    info[cnt] |= NOGARLIC;
                }
                if (checkobj && sobj_at(BOULDER, nx, ny)) {
                    if(!(flag & ALLOW_ROCK)) continue;
                    info[cnt] |= ALLOW_ROCK;
                }
                if (monseeu && onlineu(nx, ny)) {
                    if (flag & NOTONL) {
                        continue;
                    }
                    info[cnt] |= NOTONL;
                }
                /* check for diagonal tight squeeze */
                if (nx != x && ny != y &&
                     bad_rock(mdat, x, ny) &&
                     cant_squeeze_thru(mon)) {
                    continue;
                }
                /* The monster avoids a particular type of trap if it's familiar
                 * with the trap type.  Pets get ALLOW_TRAPS and checking is
                 * done in dogmove.c.  In either case, "harmless" traps are
                 * neither avoided nor marked in info[].
                 */
                struct trap *ttmp = t_at(nx, ny);
                if (ttmp) {
                    if (ttmp->ttyp >= TRAPNUM || ttmp->ttyp == 0)  {
                        impossible("A monster looked at a very strange trap of type %d.", ttmp->ttyp);
                        continue;
                    }
                    if ((ttmp->ttyp != RUST_TRAP
                        || mdat == &mons[PM_IRON_GOLEM])
                        && ttmp->ttyp != STATUE_TRAP
                        && ttmp->ttyp != VIBRATING_SQUARE
                        && ((ttmp->ttyp != PIT
                            && ttmp->ttyp != SPIKED_PIT
                            && ttmp->ttyp != TRAPDOOR
                            && ttmp->ttyp != HOLE)
                            || (!is_flyer(mdat)
                                && !is_floater(mdat)
                                && !is_clinger(mdat))
                            || In_sokoban(&u.uz))
                        && (ttmp->ttyp != SLP_GAS_TRAP ||
                            !resists_sleep(mon))
                        && (ttmp->ttyp != BEAR_TRAP ||
                            (mdat->msize > MZ_SMALL &&
                            !amorphous(mdat) && !is_flyer(mdat)))
                        && (ttmp->ttyp != FIRE_TRAP ||
                            !resists_fire(mon))
                        && (ttmp->ttyp != SQKY_BOARD || !is_flyer(mdat))
                        && (ttmp->ttyp != WEB || (!amorphous(mdat) &&
                                                !webmaker(mdat)))
                        ) {
                            if (!(flag & ALLOW_TRAPS)) {
                                if (mon->mtrapseen & (1L << (ttmp->ttyp - 1))) {
                                    continue;
                                }
                            }
                            info[cnt] |= ALLOW_TRAPS;
                        }
                }
                poss[cnt].x = nx;
                poss[cnt].y = ny;
                cnt++;
            }
        }
    }
    if (!cnt && wantpool && !is_pool(x, y)) {
        wantpool = FALSE;
        goto nexttry;
    }
    return cnt;
}

/* Part of mm_aggression that represents two-way aggression.  To avoid
   having to code each case twice, this function contains those cases that
   ought to happen twice, and mm_aggression will call it twice. */
static long
mm_2way_aggression(struct monst *magr, struct monst *mdef)
{
    /* zombies vs things that can be zombified */
    if (zombie_maker(magr) && zombie_form(mdef->data) != NON_PM) {
        return (ALLOW_M | ALLOW_TM);
    }

    return 0;
}

/* Monster against monster special attacks; for the specified monster
   combinations, this allows one monster to attack another adjacent one
   in the absence of Conflict.  There is no provision for targetting
   other monsters; just hand to hand fighting when they happen to be
   next to each other. */
static long
mm_aggression(
    struct monst *magr, /**< monster that is currently deciding where to move */
    struct monst *mdef) /**< another monster which is next to it */
{
    struct permonst *ma, *md;

    /* don't allow pets to fight each other */
    if (magr->mtame && mdef->mtame) {
        return 0;
    }

    ma = magr->data;
    md = mdef->data;

    /* supposedly purple worms are attracted to shrieking because they
       like to eat shriekers, so attack the latter when feasible */
    if (ma == &mons[PM_PURPLE_WORM] &&
        md == &mons[PM_SHRIEKER])
        return ALLOW_M|ALLOW_TM;

    /* Since the quest guardians are under siege, it makes sense to have
       them fight hostiles.  (But we don't want the quest leader to be in danger.) */
    if(ma->msound==MS_GUARDIAN && mdef->mpeaceful==FALSE)
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(md->msound==MS_GUARDIAN && magr->mpeaceful==FALSE)
        return ALLOW_M|ALLOW_TM;

    /* elves vs. orcs */
    if(is_elf(ma) && is_orc(md))
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(is_elf(md) && is_orc(ma))
        return ALLOW_M|ALLOW_TM;

    /* elves vs. kobolds */
    if(is_elf(ma) && md->mlet==S_KOBOLD)
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(is_elf(md) && ma->mlet==S_KOBOLD)
        return ALLOW_M|ALLOW_TM;

    /* angels vs. demons */
    if(ma->mlet==S_ANGEL && is_demon(md))
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(md->mlet==S_ANGEL && is_demon(ma))
        return ALLOW_M|ALLOW_TM;

    /* Nazgul vs. hobbits */
    if(ma == &mons[PM_NAZGUL] && md == &mons[PM_HOBBIT])
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(md == &mons[PM_NAZGUL] && ma == &mons[PM_HOBBIT])
        return ALLOW_M|ALLOW_TM;

    /* deep orcs vs. dwarves */
    if(is_dwarf(ma) && md == &mons[PM_DEEP_ORC])
        return ALLOW_M|ALLOW_TM;
    /* and vice versa */
    if(is_dwarf(md) && ma == &mons[PM_DEEP_ORC])
        return ALLOW_M|ALLOW_TM;

    /* woodchucks vs. The Oracle */
    if(ma == &mons[PM_WOODCHUCK] && md == &mons[PM_ORACLE])
        return ALLOW_M|ALLOW_TM;

    /* ravens like eyes */
    if(ma == &mons[PM_RAVEN] && is_eye(md))
        return ALLOW_M|ALLOW_TM;

    /* dungeon fern spores hate everything */
    if(is_fern_spore(ma) && !is_fern_spore(md) && !is_vegetation(md))
        return ALLOW_M|ALLOW_TM;
    /* and everything hates them */
    if(is_fern_spore(md) && !is_fern_spore(ma) && !is_vegetation(ma))
        return ALLOW_M|ALLOW_TM;

    return (mm_2way_aggression(magr, mdef) | mm_2way_aggression(mdef, magr));
}

/* Monster displacing another monster out of the way */
static long
mm_displacement(
    struct monst *magr, /**< monster that is currently deciding where to move */
    struct monst *mdef) /**< another monster which is next to it */
{
    struct permonst *pa = magr->data, *pd = mdef->data;

    /* if attacker can't barge through, there's nothing to do;
       or if defender can barge through too, don't let attacker
       do so, otherwise they might just end up swapping places
       again when defender gets its chance to move */
    if ((pa->mflags3 & M3_DISPLACES) &&
        ((pd->mflags3 & M3_DISPLACES) == 0) &&
        /* no displacing grid bugs diagonally */
        !(magr->mx != mdef->mx && magr->my != mdef->my && NODIAG(monsndx(pd))) &&
        /* no displacing trapped monsters or multi-location longworms */
        !mdef->mtrapped && (!mdef->wormno || !count_wsegs(mdef)) &&
        /* riders can move anything; others, same size or smaller only */
        (is_rider(pa) || pa->msize >= pd->msize))
        return ALLOW_MDISP;
    return 0L;
}

/* Is the square close enough for the monster to move or attack into? */
boolean
monnear(struct monst *mon, coordxy x, coordxy y)
{
    int distance = dist2(mon->mx, mon->my, x, y);

    if (distance == 2 && NODIAG(mon->data - mons)) {
        return 0;
    }
    return (boolean)(distance < 3);
}

/* really free dead monsters */
void
dmonsfree(void)
{
    struct monst **mtmp, *freetmp;
    int count = 0;
    char buf[QBUFSZ];

    buf[0] = '\0';
    for (mtmp = &fmon; *mtmp;) {
        freetmp = *mtmp;
        if (DEADMONSTER(freetmp) && !freetmp->isgd) {
            count++;
            if (count > iflags.purge_monsters) {
                describe_level(buf);
                impossible("dmonsfree: %d removed doesn't match %d pending on %s",
                        count, iflags.purge_monsters, buf);
            }

            *mtmp = freetmp->nmon;
            freetmp->nmon = NULL;
            dealloc_monst(freetmp);
        } else {
            mtmp = &(freetmp->nmon);
        }
    }

    if (count != iflags.purge_monsters) {
        describe_level(buf);
        impossible("dmonsfree: %d removed doesn't match %d pending on %s",
                   count, iflags.purge_monsters, buf);
    }
    iflags.purge_monsters = 0;
}

/* called when monster is moved to larger structure */
void
replmon(struct monst *mtmp, struct monst *mtmp2)
{
    struct obj *otmp;

    /* transfer the monster's inventory */
    for (otmp = mtmp2->minvent; otmp; otmp = otmp->nobj) {
#ifdef DEBUG
        if (otmp->where != OBJ_MINVENT || otmp->ocarry != mtmp)
            panic("replmon: minvent inconsistency");
#endif
        otmp->ocarry = mtmp2;
    }
    mtmp->minvent = 0;

    /* remove the old monster from the map and from `fmon' list */
    relmon(mtmp, (struct monst **) 0);

    /* finish adding its replacement */
    if (mtmp != u.usteed) {
        /* don't place steed onto the map */
        place_monster(mtmp2, mtmp2->mx, mtmp2->my);
    }
    if (mtmp2->wormno)      /* update level.monsters[wseg->wx][wseg->wy] */
        place_wsegs(mtmp2, NULL); /* locations to mtmp2 not mtmp. */
    if (emits_light(mtmp2->data)) {
        /* since this is so rare, we don't have any `mon_move_light_source' */
        new_light_source(mtmp2->mx, mtmp2->my,
                         emits_light(mtmp2->data),
                         LS_MONSTER, monst_to_any(mtmp2));
        /* here we rely on the fact that `mtmp' hasn't actually been deleted */
        del_light_source(LS_MONSTER, monst_to_any(mtmp));
    }
    mtmp2->nmon = fmon;
    fmon = mtmp2;
    if (u.ustuck == mtmp) {
        set_ustuck(mtmp2);
    }
    if (u.usteed == mtmp) {
        u.usteed = mtmp2;
    }
    if (mtmp2->isshk) {
        replshk(mtmp, mtmp2);
    }

    /* discard the old monster */
    dealloc_monst(mtmp);
}

/* release mon from the display and the map's monster list,
   maybe transfer it to one of the other monster lists */
void
relmon(struct monst *mon, struct monst **monst_list) /**< &migrating_mons or &mydogs or null */
{
    struct monst *mtmp;
    int mx = mon->mx, my = mon->my;
    boolean on_map = (m_at(mx, my) == mon),
            unhide = (monst_list != 0);

    if (!fmon) {
        panic("relmon: no fmon available.");
    }

    if (unhide) {
        /* can't remain hidden across level changes (exception: wizard
           clone can continue imitating some other monster form); also,
           might be imitating a boulder so need line-of-sight unblocking */
        mon->mundetected = 0;
        if (M_AP_TYPE(mon) && M_AP_TYPE(mon) != M_AP_MONSTER)
            seemimic(mon);
    }

    if (on_map) {
        mon->mtrapped = 0;
        if (mon->wormno) {
            remove_worm(mon);
        } else {
            remove_monster(mx, my);
        }
    }

    if (mon == fmon) {
        fmon = fmon->nmon;
    } else {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (mtmp->nmon == mon) {
                break;
            }
        }

        if (mtmp) {
            mtmp->nmon = mon->nmon;
        } else {
            panic("relmon: mon not in list.");
        }
    }

    if (unhide) {
        if (on_map) {
            newsym(mx, my);
        }
        /* insert into mydogs or migrating_mons */
        mon->nmon = *monst_list;
        *monst_list = mon;
    } else {
        /* orphan has no next monster */
        mon->nmon = 0;
    }
}

void
copy_mextra(struct monst *mtmp2, struct monst *mtmp1)
{
    if (!mtmp2 || !mtmp1 || !mtmp1->mextra)
        return;

    if (!mtmp2->mextra)
        mtmp2->mextra = newmextra();
    if (MGIVENNAME(mtmp1)) {
        new_mgivenname(mtmp2, (int) strlen(MGIVENNAME(mtmp1)) + 1);
        Strcpy(MGIVENNAME(mtmp2), MGIVENNAME(mtmp1));
    }
    if (EGD(mtmp1)) {
        if (!EGD(mtmp2))
            newegd(mtmp2);
        *EGD(mtmp2) = *EGD(mtmp1);
    }
    if (EPRI(mtmp1)) {
        if (!EPRI(mtmp2))
            newepri(mtmp2);
        *EPRI(mtmp2) = *EPRI(mtmp1);
    }
    if (ESHK(mtmp1)) {
        if (!ESHK(mtmp2))
            neweshk(mtmp2);
        *ESHK(mtmp2) = *ESHK(mtmp1);
    }
    if (EMIN(mtmp1)) {
        if (!EMIN(mtmp2))
            newemin(mtmp2);
        *EMIN(mtmp2) = *EMIN(mtmp1);
    }
    if (EDOG(mtmp1)) {
        if (!EDOG(mtmp2))
            newedog(mtmp2);
        *EDOG(mtmp2) = *EDOG(mtmp1);
    }
    if (has_mcorpsenm(mtmp1))
        MCORPSENM(mtmp2) = MCORPSENM(mtmp1);
}

void
dealloc_mextra(struct monst *m)
{
    struct mextra *x = m->mextra;

    if (x) {
        if (x->mgivenname) {
            free((genericptr_t) x->mgivenname);
        }
        if (x->egd)
            free((genericptr_t) x->egd);
        if (x->epri)
            free((genericptr_t) x->epri);
        if (x->eshk) {
            struct mkroom *sroom = &rooms[x->eshk->shoproom - ROOMOFFSET];
            /* remove mon as resident of a room */
            if (sroom->resident == m) {
                sroom->resident = NULL;
            }
            free((genericptr_t) x->eshk);
        }
        if (x->emin)
            free((genericptr_t) x->emin);
        if (x->edog)
            free((genericptr_t) x->edog);
        /* [no action needed for x->mcorpsenm] */

        free((genericptr_t) x);
        m->mextra = (struct mextra *) 0;
    }
}

void
dealloc_monst(struct monst *mon)
{
    char buf[QBUFSZ];

    buf[0] = '\0';
    if (mon->nmon) {
        describe_level(buf);
        panic("dealloc_monst with nmon on %s", buf);
    }
    if (mon->mextra)
        dealloc_mextra(mon);
    free((genericptr_t) mon);
}

/* remove effects of mtmp from other data structures */
static void
m_detach(struct monst *mtmp, struct permonst *mptr) /**< reflects mtmp->data _prior_ to mtmp's death */
{
    boolean onmap = (mtmp->mx > 0);

    if (mtmp == polearm.hitmon) {
        polearm.hitmon = 0;
    }
    if (mtmp->mleashed) m_unleash(mtmp, FALSE);
    /* to prevent an infinite relobj-flooreffects-hmon-killed loop */
    mtmp->mtrapped = 0;
    mtmp->mhp = 0; /* simplify some tests: force mhp to 0 */
    relobj(mtmp, 0, FALSE);
    if (onmap || mtmp == level.monsters[0][0]) {
        if (mtmp->wormno) {
            remove_worm(mtmp);
        } else {
            remove_monster(mtmp->mx, mtmp->my);
        }
    }
    if (emits_light(mptr))
        del_light_source(LS_MONSTER, monst_to_any(mtmp));
    if (M_AP_TYPE(mtmp)) {
        seemimic(mtmp);
    }
    if (onmap) {
        newsym(mtmp->mx, mtmp->my);
    }
    unstuck(mtmp);
    if (onmap) {
        fill_pit(mtmp->mx, mtmp->my);
    }
    if (mtmp->isshk) {
        shkgone(mtmp);
    }
    if (mtmp->wormno) {
        wormgone(mtmp);
    }
    if (In_endgame(&u.uz)) {
        mtmp->mstate |= MON_ENDGAME_FREE;
    }

    if ((mtmp->mstate & MON_DETACH) != 0) {
        impossible("m_detach: %s is already detached?", minimal_monnam(mtmp, FALSE));
    } else {
        mtmp->mstate |= MON_DETACH;
        iflags.purge_monsters++;
    }
}

/* find the worn amulet of life saving which will save a monster */
struct obj *
mlifesaver(struct monst *mon)
{
    if (!nonliving(mon->data) || is_vampshifter(mon)) {
        struct obj *otmp = which_armor(mon, W_AMUL);

        if (otmp && otmp->otyp == AMULET_OF_LIFE_SAVING)
            return otmp;
    }
    return (struct obj *)0;
}

static void
#ifdef WEBB_DISINT
lifesaved_monster(struct monst *mtmp, uchar adtyp)


#else


#endif
{
    struct obj *lifesave = mlifesaver(mtmp);

    if (lifesave) {
        /* not canseemon; amulets are on the head, so you don't want */
        /* to show this for a long worm with only a tail visible. */
        /* Nor do you check invisibility, because glowing and disinte- */
        /* grating amulets are always visible. */
        if (cansee(mtmp->mx, mtmp->my)) {
            pline("But wait...");
            pline("%s medallion begins to glow!",
                  s_suffix(Monnam(mtmp)));
            makeknown(AMULET_OF_LIFE_SAVING);
            /* amulet is visible, but monster might not be */
            if (canseemon(mtmp)) {
                if (attacktype(mtmp->data, AT_EXPL)
                    || attacktype(mtmp->data, AT_BOOM)
#ifdef WEBB_DISINT
                    || adtyp == AD_DISN
#endif
                    )
                    pline("%s reconstitutes!", Monnam(mtmp));
                else
                    pline("%s looks much better!", Monnam(mtmp));
                pline_The("medallion crumbles to dust!");
            }
        }
        m_useup(mtmp, lifesave);
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        /* equip replacement amulet, if any, on next move */
        mtmp->misc_worn_check |= I_SPECIAL;

        boolean survivor = !(mvitals[monsndx(mtmp->data)].mvflags & G_GENOD);
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        if (mtmp->mtame && !mtmp->isminion) {
            wary_dog(mtmp, !survivor);
        }
        if (mtmp->mhpmax <= 0) {
            mtmp->mhpmax = 10;
        }
        mtmp->mhp = mtmp->mhpmax;
        if (!survivor) {
            /* genocided monster can't be life-saved */
            if (cansee(mtmp->mx, mtmp->my)) {
                pline("Unfortunately %s is still genocided...",
                      mon_nam(mtmp));
            }
            mtmp->mhp = 0;
        }
    }
}

#ifndef WEBB_DISINT
void
mondead(mtmp)
register struct monst *mtmp;
#else
void
mondead(struct monst *mtmp)
{
    mondead_helper(mtmp, 0); /* mmm... default parameter values */
}

void
mondead_helper(struct monst *mtmp, uchar adtyp)


#endif
{
    struct permonst *mptr;
    int tmp;
    struct obj* otmp;

    mtmp->mhp = 0; /* in case caller hasn't done this */
#ifdef WEBB_DISINT
    lifesaved_monster(mtmp, adtyp);
#else
    lifesaved_monster(mtmp);
#endif
    if (!DEADMONSTER(mtmp)) {
        return;
    }

    if (is_vampshifter(mtmp)) {
        int mndx = mtmp->cham;
        int x = mtmp->mx, y = mtmp->my;

        /* this only happens if shapeshifted */
        if (mndx >= LOW_PM && mndx != monsndx(mtmp->data) &&
             !(mvitals[mndx].mvflags & G_GENOD)) {
            char buf[BUFSZ];
            boolean in_door = (amorphous(mtmp->data) && closed_door(mtmp->mx, mtmp->my));
            /* alternate message phrasing for some monster types */
            boolean spec_mon = (nonliving(mtmp->data) ||
                                noncorporeal(mtmp->data) ||
                                amorphous(mtmp->data));
            /* disintegrated or digested */
            boolean spec_death = (disintegested ||
                                  noncorporeal(mtmp->data) ||
                                  amorphous(mtmp->data));

            /* construct a format string before transformation;
               will be capitalized when used, expects one %s arg */
            Sprintf(buf, "%s suddenly %s and rises as %%s!",
                    x_monnam(mtmp, ARTICLE_THE,
                             spec_mon ? (char *) 0 : "seemingly dead",
                             (SUPPRESS_INVISIBLE | SUPPRESS_IT), FALSE),
                    spec_death ? "reconstitutes" : "transforms");
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
            if (mtmp->mhpmax <= 0) {
                mtmp->mhpmax = 10;
            }
            mtmp->mhp = mtmp->mhpmax;
            /* mtmp==u.ustuck can happen if previously a fog cloud
               or poly'd hero is hugging a vampire bat */
            if (mtmp == u.ustuck) {
                if (u.uswallow) {
                    expels(mtmp, mtmp->data, FALSE);
                } else {
                    uunstick();
                }
            }
            if (in_door) {
                coord new_xy;

                if (enexto(&new_xy, mtmp->mx, mtmp->my, &mons[mndx])) {
                    rloc_to(mtmp, new_xy.x, new_xy.y);
                }
            }
            newcham(mtmp, &mons[mndx], FALSE, FALSE);
            if (mtmp->data == &mons[mndx]) {
                mtmp->cham = NON_PM;
            } else {
                mtmp->cham = mndx;
            }
            if (canspotmon(mtmp)) {
                /* 3.6.0 used a_monnam(mtmp); that was weird if mtmp was
                   named: "Dracula suddenly transforms and rises as Dracula";
                   3.6.1 used mtmp->data->mname; that ignored hallucination */
                pline(upstart(buf),
                      x_monnam(mtmp, ARTICLE_A, (char *) 0,
                               (SUPPRESS_NAME | SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));
                vamp_rise_msg = TRUE;
            }
            newsym(x, y);
            return;
        }
    }

    /* dead vault guard is actually kept at coordinate <0,0> until
       his temporary corridor to/from the vault has been removed;
       need to do this after life-saving and before m_detach() */
    if (mtmp->isgd && !grddead(mtmp)) {
        return;
    }

    /* Player is thrown from his steed when it dies */
    if (mtmp == u.usteed)
        dismount_steed(DISMOUNT_GENERIC);

    /* extinguish monster's armor */
    if ((otmp = which_armor(mtmp, W_ARM)) && (Is_glowing_dragon_armor(otmp->otyp)))
        end_burn(otmp, FALSE);

    mptr = mtmp->data; /* save this for m_detach() */
    /* restore chameleon, lycanthropes to true form at death */
    if (mtmp->cham >= LOW_PM) {
        set_mon_data(mtmp, &mons[mtmp->cham]);
        mtmp->cham = NON_PM;
    } else if (mtmp->data == &mons[PM_WEREJACKAL]) {
        set_mon_data(mtmp, &mons[PM_HUMAN_WEREJACKAL]);
    } else if (mtmp->data == &mons[PM_WEREWOLF]) {
        set_mon_data(mtmp, &mons[PM_HUMAN_WEREWOLF]);
    } else if (mtmp->data == &mons[PM_WERERAT]) {
        set_mon_data(mtmp, &mons[PM_HUMAN_WERERAT]);
    }

    /* if MAXMONNO monsters of a given type have died, and it
     * can be done, extinguish that monster.
     *
     * mvitals[].died does double duty as total number of dead monsters
     * and as experience factor for the player killing more monsters.
     * this means that a dragon dying by other means reduces the
     * experience the player gets for killing a dragon directly; this
     * is probably not too bad, since the player likely finagled the
     * first dead dragon via ring of conflict or pets, and extinguishing
     * based on only player kills probably opens more avenues of abuse
     * for rings of conflict and such.
     */
    tmp = monsndx(mtmp->data);
    if (mvitals[tmp].died < 255) mvitals[tmp].died++;

    /* killing an artifact-guardian is ordinary robbery */
    if (is_guardian(mtmp->data))
        violated(CONDUCT_THIEVERY);

    /* if it's a (possibly polymorphed) quest leader, mark him as dead */
    if (mtmp->m_id == quest_status.leader_m_id)
        quest_status.leader_is_dead = TRUE;
#ifdef MAIL
    /* if the mail daemon dies, no more mail delivery.  -3. */
    if (tmp == PM_MAIL_DAEMON) mvitals[tmp].mvflags |= G_GENOD;
#endif

#ifdef KOPS
    if (mtmp->data->mlet == S_KOP) {
        stairway *stway = stairway_find_type_dir(FALSE, FALSE);

        /* Dead Kops may come back. */
        switch (rnd(5)) {
        case 1: /* returns near the stairs */
            if (stway) {
                (void) makemon(mtmp->data, stway->sx, stway->sy, NO_MM_FLAGS);
                break;
            }
            /* fall-through */
        case 2: /* randomly */
            (void) makemon(mtmp->data, 0, 0, NO_MM_FLAGS);
            break;
        default:
            break;
        }
    }
#endif

#ifdef BLACKMARKET
    if (Is_blackmarket(&u.uz) && tmp == PM_ONE_EYED_SAM) {
        bars_around_portal(TRUE);
    }
#endif /* BLACKMARKET */

    if(mtmp->iswiz) wizdead();
    if(mtmp->data->msound == MS_NEMESIS) nemdead();

#ifdef RECORD_ACHIEVE
    if(mtmp->data == &mons[PM_MEDUSA])
        achieve.killed_medusa = 1;
#ifdef LIVELOGFILE
    livelog_achieve_update();
#endif
#endif

    if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph)) {
        unmap_object(mtmp->mx, mtmp->my);
    }
    m_detach(mtmp, mptr);
}

/** TRUE if corpse might be dropped, magr may die if mon was swallowed */
boolean
corpse_chance(
    struct monst *mon,
    struct monst *magr, /**< killer, if swallowed */
    boolean was_swallowed) /**< digestion */
{
    struct permonst *mdat = mon->data;
    int i, tmp;

    if (mdat == &mons[PM_VLAD_THE_IMPALER] || mdat->mlet == S_LICH
#ifdef WEBB_DISINT
        || mdat == &mons[PM_DISINTEGRATOR]
#endif
        ) {
        if (cansee(mon->mx, mon->my) && !was_swallowed)
            pline("%s body crumbles into dust.", s_suffix(Monnam(mon)));
        return FALSE;
    }

    /* Trolls don't leave a corpse when the player is wielding Trollsbane */
    if (mdat->mlet == S_TROLL && uwep && uwep->oartifact == ART_TROLLSBANE) {
        if (cansee(mon->mx, mon->my)) {
            pline("%s corpse flares brightly and burns to ashes.", s_suffix(Monnam(mon)));
            return FALSE;
        }
    }

    /* Gas spores always explode upon death */
    for (i = 0; i < NATTK; i++) {
        if (mdat->mattk[i].aatyp == AT_BOOM) {
            if (mdat->mattk[i].damn)
                tmp = d((int)mdat->mattk[i].damn,
                        (int)mdat->mattk[i].damd);
            else if(mdat->mattk[i].damd)
                tmp = d((int)mdat->mlevel+1, (int)mdat->mattk[i].damd);
            else tmp = 0;
            if (was_swallowed && magr) {
                if (magr == &youmonst) {
                    There("is an explosion in your %s!",
                          body_part(STOMACH));
                    Sprintf(killer.name, "%s explosion",
                            s_suffix(mdat->mname));
                    losehp(Maybe_Half_Phys(tmp), killer.name, KILLED_BY_AN);
                } else {
                    if (flags.soundok) You_hear("an explosion.");
                    magr->mhp -= tmp;
                    if (magr->mhp < 1) mondied(magr);
                    if (magr->mhp < 1) { /* maybe lifesaved */
                        if (canspotmon(magr))
                            pline("%s rips open!", Monnam(magr));
                    } else if (canseemon(magr))
                        pline("%s seems to have indigestion.",
                              Monnam(magr));
                }

                return FALSE;
            }

            Sprintf(killer.name, "%s explosion", s_suffix(mdat->mname));
            killer.format = KILLED_BY_AN;
            explode(mon->mx, mon->my, -1, tmp, MON_EXPLODE, EXPL_NOXIOUS);
            return (FALSE);
        }
    }

    /* must duplicate this below check in xkilled() since it results in
     * creating no objects as well as no corpse
     */
    if (LEVEL_SPECIFIC_NOCORPSE(mdat))
        return FALSE;

    if (((bigmonst(mdat) || mdat == &mons[PM_LIZARD]) && !mon->mcloned) ||
         is_golem(mdat) ||
         is_mplayer(mdat) ||
         is_rider(mdat) ||
         mon->isshk) {
        return TRUE;
    }
    tmp = 2 + ((mdat->geno & G_FREQ) < 2) + verysmall(mdat);
    return (boolean) !rn2(tmp);
}

/** Creates Cthulhu's death message and death cloud. */
void
cthulhu_dies(struct monst *mon) /**< Cthulhu's struct */
{
    /* really dead? */
    if (mon->mhp <= 0) {
        /* Cthulhu Deliquesces... */
        if (cansee(mon->mx, mon->my)) {
            pline("%s body deliquesces into a cloud of noxious gas!",
                  s_suffix(Monnam(mon)));
        } else {
            You_hear("hissing and bubbling!");
        }
        /* ...into a stinking cloud... */
        if (mvitals[PM_CTHULHU].died == 1 &&
            distu(mon->mx, mon->my) > 2) {
            /* Cthulhu got killed while meditating and the player
             * was not next to him.
             * You can't get rid of the True Final Boss so easily! */
            (void) create_cthulhu_death_cloud(mon->mx, mon->my, 2, 4, rnd(4));
        } else {
            (void) create_cthulhu_death_cloud(mon->mx, mon->my, 3, 8, rn1(30, 30));
        }
    }
}

void
spore_dies(struct monst *mon)
{
    if (mon->mhp <= 0) {
        int sporetype;
        coord mm; schar ltyp;
        mm.x = mon->mx; mm.y = mon->my;
        ltyp = levl[mm.x][mm.y].typ;
        create_gas_cloud(mm.x, mm.y, rn1(2, 1), rnd(8), rn1(3, 2));
        /* all fern spores have a 2/3 chance of creating nothing, except for
           the generic fern spore, which guarantees a terrain-appropriate fern */
        if (mon->data == &mons[PM_DUNGEON_FERN_SPORE]) {
            /* dungeon ferns cannot reproduce on ice, lava, or water; swamp is okay */
            if (!is_ice(mm.x, mm.y) && !is_lava(mm.x, mm.y) && !is_pool(mm.x, mm.y))
                sporetype = 0;
            else return;
            if (rn2(3)) return;
        } else if (mon->data == &mons[PM_ARCTIC_FERN_SPORE]) {
            /* arctic ferns can only reproduce on water, ice, or in swamp */
            if (ltyp == POOL || ltyp == MOAT || is_ice(mm.x, mm.y) || is_swamp(mm.x, mm.y))
                sporetype = 1;
            else return;
            if (rn2(3)) return;
        } else if (mon->data == &mons[PM_BLAZING_FERN_SPORE]) {
            /* blazing ferns can only reproduce on lava */
            if (is_lava(mm.x, mm.y))
                sporetype = 2;
            else return;
            if (rn2(3)) return;
        } else if (mon->data == &mons[PM_SWAMP_FERN_SPORE]) {
            /* swamp ferns can only reproduce in swamp */
            if (is_swamp(mm.x, mm.y))
                sporetype = 3;
            else return;
            if (rn2(3)) return;
        } else {
            if (is_ice(mm.x, mm.y) || is_pool(mm.x, mm.y)) {
                sporetype = 1;
            } else if (is_lava(mm.x, mm.y)) {
                sporetype = 2;
            } else if (is_swamp(mm.x, mm.y)) {
                sporetype = 3;
            } else {
                sporetype = rn2(4);
            }
        }
        /* when creating a new fern, 5/6 chance of creating
           a fern sprout and 1/6 chance of a fully-grown one */
        switch (sporetype) {
        case 0:
            if (!rn2(6)) makemon(&mons[PM_DUNGEON_FERN], mm.x, mm.y, NO_MM_FLAGS);
            else makemon(&mons[PM_DUNGEON_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
            break;
        case 1:
            if (ltyp == POOL || ltyp == MOAT || is_swamp(mm.x, mm.y)) {
                levl[mm.x][mm.y].typ = ICE;
                You_hear(Hallucination ? "someone selling hot chocolate." : "a crackling sound.");
            }
            if (!rn2(6)) makemon(&mons[PM_ARCTIC_FERN], mm.x, mm.y, NO_MM_FLAGS);
            else makemon(&mons[PM_ARCTIC_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
            break;
        case 2:
            levl[mm.x][mm.y].typ = ROOM;
            if (!rn2(6)) makemon(&mons[PM_BLAZING_FERN], mm.x, mm.y, NO_MM_FLAGS);
            else makemon(&mons[PM_BLAZING_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
            break;
        case 3:
            if (!rn2(6)) makemon(&mons[PM_SWAMP_FERN], mm.x, mm.y, NO_MM_FLAGS);
            else makemon(&mons[PM_SWAMP_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
            break;
        default:
            warning("Unknown spore type: (%d)", sporetype);
            break;
        }

    }
}

/** drop (perhaps) a cadaver and remove monster */
void
mondied(struct monst *mdef)
{
    mondead(mdef);
    if (!DEADMONSTER(mdef)) {
        return; /* lifesaved */
    }

    if (corpse_chance(mdef, (struct monst *)0, FALSE) &&
        (accessible(mdef->mx, mdef->my) || is_pool(mdef->mx, mdef->my)))
        (void) make_corpse(mdef, CORPSTAT_NONE);
}

/** monster disappears, not dies */
void
mongone(struct monst *mdef)
{
    mdef->mhp = 0; /* can skip some inventory bookkeeping */

    /* dead vault guard is actually kept at coordinate <0,0> until
       his temporary corridor to/from the vault has been removed */
    if (mdef->isgd && !grddead(mdef)) {
        return;
    }

    /* Player is thrown from his steed when it disappears */
    if (mdef == u.usteed)
        dismount_steed(DISMOUNT_GENERIC);

    /* stuck to you? release */
    unstuck(mdef);
    /* drop special items like the Amulet so that a dismissed Kop or nurse
       can't remove them from the game */
    mdrop_special_objs(mdef);
    /* release rest of monster's inventory--it is removed from game */
    discard_minvent(mdef);
    m_detach(mdef, mdef->data);
}

/** drop a statue or rock and remove monster */
void
monstone(struct monst *mdef)
{
    struct obj *otmp, *obj, *oldminvent;
    coordxy x = mdef->mx, y = mdef->my;
    boolean wasinside = FALSE;

    /* we have to make the statue before calling mondead, to be able to
     * put inventory in it, and we have to check for lifesaving before
     * making the statue....
     */
    mdef->mhp = 0; /* in case caller hasn't done this */
#ifdef WEBB_DISINT
    lifesaved_monster(mdef, AD_STON);
#else
    lifesaved_monster(mdef);
#endif
    if (!DEADMONSTER(mdef)) {
        return;
    }

    mdef->mtrapped = 0; /* (see m_detach) */

    if ((int)mdef->data->msize > MZ_TINY ||
        !rn2(2 + ((int) (mdef->data->geno & G_FREQ) > 2))) {
        oldminvent = 0;
        /* some objects may end up outside the statue */
        while ((obj = mdef->minvent) != 0) {
            obj_extract_self(obj);
            if (obj->owornmask)
                update_mon_intrinsics(mdef, obj, FALSE, TRUE);
            obj_no_longer_held(obj);
            if (obj->owornmask & W_WEP)
                setmnotwielded(mdef, obj);
            obj->owornmask = 0L;
            if (obj->otyp == BOULDER ||
#if 0               /* monsters don't carry statues */
                (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
                obj_resists(obj, 0, 0)) {
                if (flooreffects(obj, x, y, "fall")) continue;
                place_object(obj, x, y);
            } else {
                if (obj->lamplit) end_burn(obj, TRUE);
                obj->nobj = oldminvent;
                oldminvent = obj;
            }
        }

        /* defer statue creation until after inventory removal
           so that saved monster traits won't retain any stale
           item-conferred attributes */
        otmp = mkcorpstat(STATUE, mdef, mdef->data, x, y, CORPSTAT_NONE);
        if (has_mgivenname(mdef)) {
            otmp = oname(otmp, MGIVENNAME(mdef));
        }
        while ((obj = oldminvent) != 0) {
            oldminvent = obj->nobj;
            obj->nobj = 0; /* avoid merged-> obfree-> dealloc_obj-> panic */
            (void) add_to_container(otmp, obj);
        }
        /* Archeologists should not break unique statues */
        if (mdef->data->geno & G_UNIQ)
            otmp->spe = 1;
        otmp->owt = weight(otmp);
    } else
        otmp = mksobj_at(ROCK, x, y, TRUE, FALSE);

    stackobj(otmp);
    /* mondead() already does this, but we must do it before the newsym */
    if (glyph_is_invisible(levl[x][y].glyph)) {
        unmap_object(x, y);
    }
    if (cansee(x, y)) newsym(x, y);
    /* We don't currently trap the hero in the statue in this case but we could */
    if (u.uswallow && u.ustuck == mdef) wasinside = TRUE;
    mondead(mdef);
    if (wasinside) {
        if (is_animal(mdef->data))
            You("%s through an opening in the new %s.",
                locomotion(youmonst.data, "jump"),
                xname(otmp));
    }
}

/* another monster has killed the monster mdef */
void
monkilled(struct monst *mdef, const char *fltxt, int how)
{
    boolean be_sad = FALSE;     /* true if unseen pet is killed */
    boolean kenny = (!strcmp(m_monnam(mdef), "Kenny"));

    if ((mdef->wormno ? worm_known(mdef) : cansee(mdef->mx, mdef->my))
        && fltxt)
        pline("%s is %s%s%s!", Monnam(mdef),
              nonliving(mdef->data) ? "destroyed" : "killed",
              *fltxt ? " by the " : "",
              fltxt
              );
    else
        be_sad = (mdef->mtame != 0);

    /* no corpses if digested or disintegrated */
    if (how == AD_DGST || how == -AD_RBRE
#ifdef WEBB_DISINT
        || how == AD_DISN)
        mondead_helper(mdef, how);
#else
        )
        mondead(mdef);
#endif
    else
        mondied(mdef);

    if (mdef->data == &mons[PM_CTHULHU]) {
        cthulhu_dies(mdef);
    }

    if (is_fern_spore(mdef->data)) {
        spore_dies(mdef);
    }

    if (be_sad && mdef->mhp <= 0) {
        if (kenny || (Hallucination && !rn2(4))) {
            verbalize("Oh my god, they killed Kenny!");
            verbalize("You bastards!");
        } else {
            You("have a %s feeling for a moment, then it passes.",
                (Hallucination ? "plaid" : "sad"));
        }
    }
    return;
}

void
set_ustuck(struct monst *mtmp)
{
    if (iflags.sanity_check || iflags.debug_fuzzer) {
        if (mtmp && !m_next2u(mtmp)) {
            impossible("Sticking to %s at distu %d?", mon_nam(mtmp), mdistu(mtmp));
        }
    }

    flags.botl = TRUE;
    u.ustuck = mtmp;
    if (!u.ustuck) {
        u.uswallow = 0;
        u.uswldtim = 0;
    }
}

void
unstuck(struct monst* mtmp)
{
    if (u.ustuck == mtmp) {
        struct permonst *ptr = mtmp->data;
        unsigned swallowed = u.uswallow;

        /* do this first so that docrt()'s botl update is accurate;
           clears u.uswallow as well as setting u.ustuck to Null */
        set_ustuck((struct monst *) 0);

        if (swallowed) {
            u.ux = mtmp->mx;
            u.uy = mtmp->my;
            if (Punished && uchain->where != OBJ_FLOOR) {
                placebc();
            }
            vision_full_recalc = 1;
            docrt();
        }
        /* prevent holder/engulfer from immediately re-holding/re-engulfing
           [note: this call to unstuck() might be because u.ustuck has just
           changed shape and doesn't have a holding attack any more, hence
           don't set mspec_used unconditionally] */
        if (!mtmp->mspec_used && (dmgtype(ptr, AD_STCK) ||
                                  attacktype(ptr, AT_ENGL) ||
                                  attacktype(ptr, AT_HUGS))) {
            mtmp->mspec_used = rnd(2);
        }
    }
}

void
killed(struct monst *mtmp)
{
    xkilled(mtmp, XKILL_GIVEMSG);
}

/* the player has killed the monster mtmp */
void
xkilled(
    struct monst *mtmp,
    int xkill_flags) /**< 1: suppress message, 2: suppress corpse, 4: pacifist */
{
    int tmp, x = mtmp->mx, y = mtmp->my;
    struct permonst *mdat;
    int mndx;
    struct obj *otmp;
    struct trap *t;
    boolean redisp = FALSE;
    boolean wasinside = u.uswallow && (u.ustuck == mtmp);
    boolean burycorpse = FALSE;
    boolean nomsg = (xkill_flags & XKILL_NOMSG) != 0;
    boolean nocorpse = (xkill_flags & XKILL_NOCORPSE) != 0;
    boolean noconduct = (xkill_flags & XKILL_NOCONDUCT) != 0;

    mtmp->mhp = 0; /* caller will usually have already done this */

    /* KMH, conduct */
    if (!noconduct) { /* KMH, conduct */
        violated(CONDUCT_PACIFISM);
    }

    if (!nomsg) {
        boolean namedpet = has_mgivenname(mtmp) && !Hallucination;
        const char *verb = nonliving(mtmp->data) ? "destroy" : "kill";

        You("%s %s!",
            verb,
            !(wasinside || canspotmon(mtmp)) ? "it" :
            !mtmp->mtame ? mon_nam(mtmp) :
            x_monnam(mtmp, namedpet ? ARTICLE_NONE : ARTICLE_THE,
            "poor", namedpet ? SUPPRESS_SADDLE : 0, FALSE));
    }

    if (mtmp->mtrapped && (t = t_at(x, y)) != 0 &&
        is_pit(t->ttyp)) {
        if (sobj_at(BOULDER, x, y)) {
            nocorpse = TRUE; /* Prevent corpses/treasure being created
                                "on top" of boulder that is about to fall in.
                                This is out of order, but cannot be helped
                                unless this whole routine is rearranged. */
        }
        if (m_carrying(mtmp, BOULDER)) {
            burycorpse = TRUE;
        }
    }

    /* your pet knows who just killed it...watch out */
    if (mtmp->mtame && !mtmp->isminion) EDOG(mtmp)->killed_by_u = 1;

    if (wasinside && thrownobj && thrownobj != uball &&
        /* don't give to mon if missile is going to return to hero */
        thrownobj != (struct obj *) iflags.returning_missile) {
        /* thrown object has killed hero's engulfer; add it to mon's
           inventory now so that it will be placed with mon's other
           stuff prior to lookhere/autopickup when hero is expelled
           below (as a side-effect, this missile has immunity from
           being consumed [for this shot/throw only]) */
        mpickobj(mtmp, thrownobj);
        /* let throwing code know that missile has been disposed of */
        thrownobj = 0;
    }

    vamp_rise_msg = FALSE; /* might get set in mondead(); only checked below */
    disintegested = nocorpse; /* alternate vamp_rise message needed if true */
    /* dispose of monster and make cadaver */
    if(stoned) monstone(mtmp);
    else mondead(mtmp);

    if (mtmp->mhp > 0) { /* monster lifesaved */
        /* Cannot put the non-visible lifesaving message in
         * lifesaved_monster() since the message appears only when you
         * kill it (as opposed to visible lifesaving which always
         * appears).
         */
        stoned = FALSE;
        if (!cansee(x, y) && !vamp_rise_msg) {
            pline("Maybe not...");
        }
        return;
    }

    mdat = mtmp->data; /* note: mondead can change mtmp->data */
    mndx = monsndx(mdat);

    if (mdat == &mons[PM_CTHULHU]) {
        cthulhu_dies(mtmp);
    }

    if (is_fern_spore(mdat)) {
        spore_dies(mtmp);
    }

    if (stoned) {
        stoned = FALSE;
        goto cleanup;
    }

    if (nocorpse || LEVEL_SPECIFIC_NOCORPSE(mdat)) {
        goto cleanup;
    }

#ifdef MAIL
    if(mdat == &mons[PM_MAIL_DAEMON]) {
        stackobj(mksobj_at(SCR_MAIL, x, y, FALSE, FALSE));
        redisp = TRUE;
    }
#endif
    if((!accessible(x, y) && !is_pool(x, y)) ||
       (x == u.ux && y == u.uy)) {
        /* might be mimic in wall or corpse in lava or on player's spot */
        redisp = TRUE;
        if(wasinside) spoteffects(TRUE);
    } else if(x != u.ux || y != u.uy) {
        /* might be here after swallowed */
        if (!flags.deathdropless && !rn2(6) &&
            !(mvitals[mndx].mvflags & G_NOCORPSE)
            /* disable death drop for puddings */
            && mdat->mlet != S_PUDDING
#ifdef KOPS
            && mdat->mlet != S_KOP
#endif
            /* no items from cloned monsters */
            && !mtmp->mcloned
            ) {
            int typ;

            otmp = mkobj_at(RANDOM_CLASS, x, y, TRUE);
            /* Don't create large objects from small monsters */
            typ = otmp->otyp;
            if (mdat->msize < MZ_HUMAN && typ != FOOD_RATION
                && typ != LEASH
                && typ != FIGURINE
                && (otmp->owt > 3 ||
                    objects[typ].oc_big /*oc_bimanual/oc_bulky*/ ||
                    is_spear(otmp) || is_pole(otmp) ||
                    typ == MORNING_STAR)) {
                delobj(otmp);
            } else redisp = TRUE;
        }
        /* Whether or not it always makes a corpse is, in theory,
         * different from whether or not the corpse is "special";
         * if we want both, we have to specify it explicitly.
         */
        if (!wasinside && corpse_chance(mtmp, (struct monst *) 0, FALSE)) {
            struct obj *cadaver;
            zombify = (!thrownobj &&
                       !stoned &&
                       !uwep &&
                       zombie_maker(&youmonst) &&
                       zombie_form(mtmp->data) != NON_PM);
            cadaver = make_corpse(mtmp, burycorpse ? CORPSTAT_BURIED
                                                   : CORPSTAT_NONE);
            zombify = FALSE; /* reset */
            if (burycorpse && cadaver && cansee(x, y) && !mtmp->minvis
                && cadaver->where == OBJ_BURIED && !nomsg) {
                pline("%s corpse ends up buried.", s_suffix(Monnam(mtmp)));
            }
        }
    }
    if(redisp) newsym(x, y);

cleanup:
    /* punish bad behaviour */
    if(is_human(mdat) && (!always_hostile(mdat) && mtmp->malign <= 0) &&
       (mndx < PM_ARCHEOLOGIST || mndx > PM_WIZARD) &&
       u.ualign.type != A_CHAOTIC) {
        HTelepat &= ~INTRINSIC;
        change_luck(-2);
        You("murderer!");
        if (Blind && !Blind_telepat)
            see_monsters(); /* Can't sense monsters any more. */
    }
    if((mtmp->mpeaceful && !rn2(2)) || mtmp->mtame) change_luck(-1);
    if (is_unicorn(mdat) &&
        sgn(u.ualign.type) == sgn(mdat->maligntyp)) {
        change_luck(-5);
        You_feel("guilty...");
    }

    /* give experience points */
    int nr_killed = (int)mvitals[mndx].died;
    tmp = experience(mtmp, nr_killed);
    more_experienced(tmp, max(tmp/nr_killed, 1), 0);
    newexplevel();      /* will decide if you go up */

    /* adjust alignment points */
    if (mtmp->m_id == quest_status.leader_m_id) { /* REAL BAD! */
        adjalign(-(u.ualign.record+(int)ALIGNLIM/2));
        pline("That was %sa bad idea...",
              u.uevent.qcompleted ? "probably " : "");
    } else if (mdat->msound == MS_NEMESIS)  /* Real good! */
        adjalign((int)(ALIGNLIM/4));
    else if (mdat->msound == MS_GUARDIAN) { /* Bad */
        adjalign(-(int)(ALIGNLIM/8));
        if (!Hallucination) pline("That was probably a bad idea...");
        else pline("Whoopsie-daisy!");
    } else if (mtmp->ispriest) {
        adjalign((p_coaligned(mtmp)) ? -2 : 2);
        /* cancel divine protection for killing your priest */
        if (p_coaligned(mtmp)) u.ublessed = 0;
        if (mdat->maligntyp == A_NONE)
            adjalign((int)(ALIGNLIM / 4));      /* BIG bonus */
    } else if (mtmp->mtame) {
        adjalign(-15);  /* bad!! */
        /* your god is mighty displeased... */
        if (!Hallucination) You_hear("the rumble of distant thunder...");
        else You_hear("the studio audience applaud!");
    } else if (mtmp->mpeaceful)
        adjalign(-5);

    /* malign was already adjusted for u.ualign.type and randomization */
    adjalign(mtmp->malign);

#ifdef LIVELOG_BONES_KILLER
    livelog_bones_killed(mtmp);
#endif
}

/* changes the monster into a stone monster of the same type */
/* this should only be called when poly_when_stoned() is true */
void
mon_to_stone(struct monst *mtmp)
{
    if (mtmp->data->mlet == S_GOLEM) {
        /* it's a golem, and not a stone golem */
        if (canseemon(mtmp)) {
            pline("%s solidifies...", Monnam(mtmp));
        }
        if (newcham(mtmp, &mons[PM_STONE_GOLEM], FALSE, FALSE)) {
            if (canseemon(mtmp)) {
                pline("Now it's %s.", an(mtmp->data->mname));
            }
        } else {
            if (canseemon(mtmp)) {
                pline("... and returns to normal.");
            }
        }
    } else {
        impossible("Can't polystone %s!", a_monnam(mtmp));
    }
}

boolean
vamp_stone(struct monst *mtmp)
{
    if (is_vampshifter(mtmp)) {
        int mndx = mtmp->cham;
        int x = mtmp->mx, y = mtmp->my;

        /* this only happens if shapeshifted */
        if (mndx >= LOW_PM && mndx != monsndx(mtmp->data) &&
             !(mvitals[mndx].mvflags & G_GENOD)) {
            char buf[BUFSZ];
            boolean in_door = (amorphous(mtmp->data) && closed_door(mtmp->mx, mtmp->my));

            /* construct a format string before transformation */
            Sprintf(buf, "The lapidifying %s %s %s",
                    x_monnam(mtmp, ARTICLE_NONE, (char *) 0,
                             (SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION | SUPPRESS_INVISIBLE | SUPPRESS_IT),
                             FALSE),
                    amorphous(mtmp->data) ? "coalesces on the" :
                    is_flyer(mtmp->data) ? "drops to the" : "writhes on the",
                    surface(x,y));
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
            if (mtmp->mhpmax <= 0) {
                mtmp->mhpmax = 10;
            }
            mtmp->mhp = mtmp->mhpmax;
            /* this can happen if previously a fog cloud */
            if (u.uswallow && (mtmp == u.ustuck)) {
                expels(mtmp, mtmp->data, FALSE);
            }
            if (in_door) {
                coord new_xy;

                if (enexto(&new_xy, mtmp->mx, mtmp->my, &mons[mndx])) {
                    rloc_to(mtmp, new_xy.x, new_xy.y);
                }
            }
            if (canspotmon(mtmp)) {
                pline("%s!", buf);
                display_nhwindow(WIN_MESSAGE, FALSE);
            }
            newcham(mtmp, &mons[mndx], FALSE, FALSE);
            if (mtmp->data == &mons[mndx]) {
                mtmp->cham = NON_PM;
            } else {
                mtmp->cham = mndx;
            }
            if (canspotmon(mtmp)) {
                pline("%s rises from the %s with renewed agility!",
                      Amonnam(mtmp), surface(mtmp->mx, mtmp->my));
            }
            newsym(mtmp->mx, mtmp->my);
            return FALSE;   /* didn't petrify */
        }
    } else if (mtmp->cham >= LOW_PM && (mons[mtmp->cham].mresists & MR_STONE)) {
        /* sandestins are stoning-immune so if hit by stoning damage
           they revert to innate shape rather than become a statue */
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        if (mtmp->mhpmax <= 0) {
            mtmp->mhpmax = 10;
        }
        mtmp->mhp = mtmp->mhpmax;
        (void) newcham(mtmp, &mons[mtmp->cham], FALSE, TRUE);
        newsym(mtmp->mx, mtmp->my);
        return FALSE;   /* didn't petrify */
    }
    return TRUE;
}

/* drop monster into "limbo" - that is, migrate to the current level */
void
m_into_limbo(struct monst *mtmp)
{
    xint16 target_lev = ledger_no(&u.uz), xyloc = MIGR_APPROX_XY;

    mtmp->mstate |= MON_LIMBO;
    migrate_mon(mtmp, target_lev, xyloc);
}

static void
migrate_mon(
    struct monst *mtmp,
    xint16 target_lev, /**< destination level */
    xint16 xyloc)      /**< MIGR_xxx flag for location within destination */
{
    unstuck(mtmp);
    mdrop_special_objs(mtmp);
    migrate_to_level(mtmp, target_lev, xyloc, (coord *) 0);
}

static boolean
ok_to_obliterate(struct monst *mtmp)
{
    /*
     * Add checks for monsters that should not be obliterated
     * here (return FALSE).
     */
    if (mtmp->data == &mons[PM_WIZARD_OF_YENDOR] ||
            is_rider(mtmp->data) ||
            has_emin(mtmp) ||
            has_epri(mtmp) ||
            has_eshk(mtmp) ||
            (u.ustuck == mtmp) ||
            (u.usteed == mtmp)) {
        return FALSE;
    }
    return TRUE;
}

void
elemental_clog(struct monst *mon)
{
    int m_lev = 0;
    static long msgmv = 0L;
    struct monst *mtmp, *m1, *m2, *m3, *m4, *m5, *zm;

    if (In_endgame(&u.uz)) {
        m1 = m2 = m3 = m4 = m5 = zm = (struct monst *) 0;
        if (!msgmv || (moves - msgmv) > 200L) {
            if (!msgmv || rn2(2)) {
                You_feel("besieged.");
            }
            msgmv = moves;
        }
        /*
         * m1 an elemental from another plane.
         * m2 an elemental from this plane.
         * m3 the least powerful monst encountered in loop so far.
         * m4 some other non-tame monster.
         * m5 a pet.
         */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp) || mtmp == mon)
                continue;
            if (mtmp->mx == 0 && mtmp->my == 0)
                continue;
            if (mon_has_amulet(mtmp) || !ok_to_obliterate(mtmp))
                continue;
            if (mtmp->data->mlet == S_ELEMENTAL) {
                if (!is_home_elemental(mtmp->data)) {
                    if (!m1) {
                        m1 = mtmp;
                    }
                } else {
                    if (!m2) {
                        m2 = mtmp;
                    }
                }
            } else {
                if (!mtmp->mtame) {
                    if (!m_lev || mtmp->m_lev < m_lev) {
                        m_lev = mtmp->m_lev;
                        m3 = mtmp;
                    } else if (!m4) {
                        m4 = mtmp;
                    }
                } else {
                    if (!m5) {
                        m5 = mtmp;
                    }
                    break;
                }
            }
        }
        mtmp = m1 ? m1 : m2 ? m2 : m3 ? m3 : m4 ? m4 : m5 ? m5 : zm;
        if (mtmp) {
            int mx = mtmp->mx, my = mtmp->my;

            mtmp->mstate |= MON_OBLITERATE;
            mongone(mtmp);
            /* places in the code might still reference mtmp->mx, mtmp->my */
            /* mtmp->mx = mtmp->my = 0; */
            rloc_to(mon, mx, my);           /* note: mon, not mtmp */

        /* last resort - migrate mon to the next plane */
        } else if (!Is_astralevel(&u.uz)) {
            d_level dest;
            coordxy target_lev;

            dest = u.uz;
            dest.dlevel--;
            target_lev = ledger_no(&dest);
            mon->mstate |= MON_ENDGAME_MIGR;
            migrate_mon(mon, target_lev, MIGR_RANDOM);
        }
    }
}

/* make monster mtmp next to you (if possible);
   might place monst on far side of a wall or boulder */
void
mnexto(struct monst *mtmp, unsigned int rlocflags)
{
    coord mm;

    if (mtmp == u.usteed) {
        /* Keep your steed in sync with you instead */
        mtmp->mx = u.ux;
        mtmp->my = u.uy;
        return;
    }

    if (!enexto(&mm, u.ux, u.uy, mtmp->data) || !isok(mm.x, mm.y)) {
        deal_with_overcrowding(mtmp);
        return;
    }

    rloc_to_flag(mtmp, mm.x, mm.y, rlocflags);
}

static void
deal_with_overcrowding(struct monst *mtmp)
{
    if (In_endgame(&u.uz)) {
        debugpline1("overcrowding: elemental_clog on %s", m_monnam(mtmp));
        elemental_clog(mtmp);
    } else {
        debugpline1("overcrowding: sending %s into limbo", m_monnam(mtmp));
        m_into_limbo(mtmp);
    }
}

/* like mnexto() but requires destination to be directly accessible */
void
maybe_mnexto(struct monst *mtmp)
{
    coord mm;
    struct permonst *ptr = mtmp->data;
    boolean diagok = !NODIAG(ptr - mons);
    int tryct = 20;

    do {
        if (!enexto(&mm, u.ux, u.uy, ptr)) {
            return;
        }
        if (couldsee(mm.x, mm.y) &&
            /* don't move grid bugs diagonally */
             (diagok || mm.x == mtmp->mx || mm.y == mtmp->my)) {
            rloc_to(mtmp, mm.x, mm.y);
            return;
        }
    } while (--tryct > 0);
}

/* mnearto()
 * Put monster near (or at) location if possible.
 * Returns:
 *  2 - if another monster was moved out of this one's way;
 *  1 - if a monster was moved from x, y to put mtmp at x, y.
 *  0 - in most cases.
 * Note: if already at the target spot, result is 1 rather than 0.
 *
 * Might be called recursively if 'move_other' is True; if so, that argument
 * will be False on the nested call so there won't be any further recursion.
 */
int
mnearto(
    struct monst *mtmp,
    coordxy x,
    coordxy y,
    boolean move_other, /* make sure mtmp gets to x, y! so move m_at(x, y) */
    unsigned int rlocflags)
{
    struct monst *othermon = (struct monst *)0;
    coordxy newx, newy;
    coord mm;
    int res = 1;

    if (mtmp->mx == x && mtmp->my == y && m_at(x, y) == mtmp) {
        return res;
    }

    if (move_other && (othermon = m_at(x, y))) {
        if (othermon->wormno)
            remove_worm(othermon);
        else
            remove_monster(x, y);

        othermon->mx = othermon->my = 0; /* 'othermon' is not on the map */
        othermon->mstate |= MON_OFFMAP;
    }

    newx = x;
    newy = y;

    if (!goodpos(newx, newy, mtmp, 0)) {
        /* actually we have real problems if enexto ever fails.
         * migrating_mons that need to be placed will cause
         * no end of trouble.
         */
        if (!enexto(&mm, newx, newy, mtmp->data) || !isok(mm.x, mm.y)) {
            if (othermon) {
                /* othermon already had its mx, my set to 0 above
                 * and this would shortly cause a sanity check to fail
                 * if we just return 0 here. The caller only possesses
                 * awareness of mtmp, not othermon. */
                deal_with_overcrowding(othermon);
            }
            return 0;
        }
        newx = mm.x;
        newy = mm.y;
    }

    rloc_to_flag(mtmp, newx, newy, rlocflags);

    if (move_other && othermon) {
        res = 2; /* moving another monster out of the way */
        if (!mnearto(othermon, x, y, FALSE, rlocflags)) {
            /* no 'move_other' this time */
            deal_with_overcrowding(othermon);
        }
    }

    return res;
}


static const char *poiseff[] = {

    " feel weaker", "r brain is on fire",
    "r judgement is impaired", "r muscles won't obey you",
    " feel very sick", " break out in hives"
};

void
poisontell(int typ)
{
    pline("You%s.", poiseff[typ]);
}

/** Called when an attack or trap has poisoned hero */
void
poisoned(const char *reason,  /**< controls what messages we display */
         int typ,
         const char *pkiller, /**< for score+log file if fatal */
         int fatal)           /**< if fatal is 0, limit damage to adjattrib */
{
    int i, loss, kprefix = KILLED_BY_AN;
    boolean thrown_weapon = (fatal < 0);
    int how;
    boolean blast = !strcmp(reason, "blast");

    if (thrown_weapon) {
        fatal = -fatal;
    }

    /* inform player about being poisoned unless that's already been done;
       "blast" has given a "blast of poison gas" message; "poison arrow",
       "poison dart", etc have implicitly given poison messages too... */
    if (!blast && !strstri(reason, "poison")) {
        boolean plural = (reason[strlen(reason) - 1] == 's') ? 1 : 0;

        /* avoid "The" Orcus's sting was poisoned... */
        pline("%s%s %s poisoned!", isupper(*reason) ? "" : "The ",
              reason, plural ? "were" : "was");
    }

    if (Poison_resistance) {
        if (blast) {
            shieldeff(u.ux, u.uy);
        }
        pline_The("poison doesn't seem to affect you.");
        return;
    }

    /* suppress killer prefix if it already has one */
    i = name_to_mon(pkiller);
    if (i >= LOW_PM && (mons[i].geno & G_UNIQ)) {
        kprefix = KILLED_BY;
        if (!type_is_pname(&mons[i])) {
            pkiller = the(pkiller);
        }
    } else if (!strncmpi(pkiller, "the ", 4) ||
               !strncmpi(pkiller, "an ", 3) ||
               !strncmpi(pkiller, "a ", 2)) {
        /*[ does this need a plural check too? ]*/
        kprefix = KILLED_BY;
    }

    how = strstri(pkiller, "poison") ? DIED : POISONING;
    i = !fatal ? 1 : rn2(fatal + (thrown_weapon ? 20 : 0));
    if (i == 0 && typ != A_CHA) {
        /* used to be instantly fatal; now just gongs your maxhp for (3d6)/2
         * ...which is probably pretty close to fatal anyway for low-levels */
        pline_The("poison was extremely toxic!");
        i = d(4, 6);
        u.uhpmax -= i / 2;
        losehp_how(i, pkiller, kprefix, how);
    } else if (i > 5) {
        boolean cloud = !strcmp(reason, "gas cloud");

        /* HP damage; more likely--but less severe--with missiles */
        loss = thrown_weapon ? rnd(6) : rn1(10, 6);
        if ((blast || cloud) && Half_gas_damage) {
            /* worn towel */
            loss = (loss + 1) / 2;
        }
        losehp(loss, pkiller, kprefix); /* poison damage */
    } else {
        /* attribute loss; if typ is A_STR, reduction in current and
           maximum HP will occur once strength has dropped down to 3 */
        loss = (thrown_weapon || !fatal) ? 1 : d(2, 2); /* was rn1(3,3) */
        /* check that a stat change was made */
        if (adjattrib(typ, -loss, 1)) {
            poisontell(typ);
        }
    }

    if (u.uhp < 1) {
        killer.format = kprefix;
        Strcpy(killer.name, pkiller);
        /* "Poisoned by a poisoned ___" is redundant */
        done(how);
    }
    (void) encumber_msg();
}

/* monster responds to player action; not the same as a passive attack */
/* assumes reason for response has been tested, and response _must_ be made */
void
m_respond(struct monst *mtmp)
{
    if (mtmp->data->msound == MS_SHRIEK) {
        if (!Deaf) {
            pline("%s shrieks.", Monnam(mtmp));
            stop_occupation();
        }
        if (!rn2(10)) {
            if (!rn2(13))
                (void) makemon(&mons[PM_PURPLE_WORM], 0, 0, NO_MM_FLAGS);
            else
                (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);

        }
        aggravate();
    }
    if (mtmp->data == &mons[PM_MEDUSA]) {
        int i;

        for (i = 0; i < NATTK; i++) {
            if (mtmp->data->mattk[i].aatyp == AT_GAZE) {
                (void) gazemu(mtmp, &mtmp->data->mattk[i]);
                break;
            }
        }
    }
}

/* Called whenever the player attacks mtmp; also called in other situations
   where mtmp gets annoyed at the player. Handles mtmp getting annoyed at the
   attack and any ramifications that might have. Useful also in situations
   where mtmp was already hostile; it checks for situations where the player
   shouldn't be attacking and any ramifications /that/ might have. */
void
setmangry(struct monst *mtmp, boolean via_attack UNUSED)
{
    mtmp->mstrategy &= ~STRAT_WAITMASK;
#ifdef BLACKMARKET
    /* Even if the black marketeer is already angry he may not have called
     * for his assistants if he or his staff have not been assaulted yet.
     */
    if (is_blkmktstaff(mtmp->data) && !mtmp->mpeaceful)
        blkmar_guards(mtmp);
#endif /* BLACKMARKET */
    if(!mtmp->mpeaceful) return;
    if(mtmp->mtame) return;
    mtmp->mpeaceful = 0;
    if(mtmp->ispriest) {
        if(p_coaligned(mtmp)) adjalign(-5); /* very bad */
        else adjalign(2);
    } else
        adjalign(-1);       /* attacking peaceful monsters is bad */
    if (couldsee(mtmp->mx, mtmp->my)) {
        if (humanoid(mtmp->data) || mtmp->isshk || mtmp->isgd)
            pline("%s gets angry!", Monnam(mtmp));
        else if (flags.verbose && flags.soundok) growl(mtmp);
    }

#ifdef BLACKMARKET
    /* Don't misbehave in the Black Market or else... */
    if (Is_blackmarket(&u.uz)) {
        if (is_blkmktstaff(mtmp->data) ||
            /* non-tame named monsters are presumably
             * black marketeer's assistants */
            (has_mgivenname(mtmp) && MGIVENNAME(mtmp))) {
            blkmar_guards(mtmp);
        }
    }
#endif /* BLACKMARKET */

    /* attacking your own quest leader will anger his or her guardians */
    if (!flags.mon_moving &&    /* should always be the case here */
        mtmp->data == &mons[quest_info(MS_LEADER)]) {
        struct monst *mon;
        struct permonst *q_guardian = &mons[quest_info(MS_GUARDIAN)];
        int got_mad = 0;

        /* guardians will sense this attack even if they can't see it */
        for (mon = fmon; mon; mon = mon->nmon) {
            if (!DEADMONSTER(mon) && mon->data == q_guardian && mon->mpeaceful) {
                mon->mpeaceful = 0;
                if (canseemon(mon)) ++got_mad;
            }
        }
        if (got_mad && !Hallucination)
            pline_The("%s appear%s to be angry too...",
                      got_mad == 1 ? q_guardian->mname :
                      makeplural(q_guardian->mname),
                      got_mad == 1 ? "s" : "");
    }
}

/* wake up a monster, possibly making it angry in the process */
void
wakeup(struct monst *mtmp, boolean via_attack)
{
    mtmp->msleeping = 0;
    if (M_AP_TYPE(mtmp)) {
        seemimic(mtmp);
    } else if (flags.forcefight && !flags.mon_moving && mtmp->mundetected) {
        mtmp->mundetected = 0;
        newsym(mtmp->mx, mtmp->my);
    }
    finish_meating(mtmp);
    if (via_attack) {
        setmangry(mtmp, TRUE);
    }
}

/* Wake up nearby monsters. */
void
wake_nearby(void)
{
    wake_nearto(u.ux, u.uy, u.ulevel * 20);
}

/* Wake up monsters near some particular location. */
void
wake_nearto(coordxy x, coordxy y, int distance)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) {
            continue;
        }
        if (distance == 0 || dist2(mtmp->mx, mtmp->my, x, y) < distance) {
            /* sleep for N turns uses mtmp->mfrozen, but so does paralysis
               so we leave mfrozen monsters alone */
            mtmp->msleeping = 0; /* wake indeterminate sleep */
            if (!(mtmp->data->geno & G_UNIQ)) {
                mtmp->mstrategy &= ~STRAT_WAITMASK; /* wake 'meditation' */
            }
            if (flags.mon_moving) {
                continue;
            }
            if (mtmp->mtame) {
                if (!mtmp->isminion) {
                    EDOG(mtmp)->whistletime = moves;
                }
                /* Clear mtrack. This is to fix up a pet who is
                   stuck "fleeing" its master. */
                memset(mtmp->mtrack, 0, sizeof mtmp->mtrack);
            }
        }
    }
}

/* NOTE: we must check for mimicry before calling this routine */
void
seemimic(struct monst *mtmp)
{
    boolean is_blocker_appear = (is_lightblocker_mappear(mtmp));

    if (has_mcorpsenm(mtmp)) {
        freemcorpsenm(mtmp);
    }

    mtmp->m_ap_type = M_AP_NOTHING;
    mtmp->mappearance = 0;

    /*
     *  Discovered mimics don't block light.
     */
    if (is_blocker_appear &&
         !does_block(mtmp->mx, mtmp->my, &levl[mtmp->mx][mtmp->my])) {
        unblock_point(mtmp->mx, mtmp->my);
    }

    newsym(mtmp->mx, mtmp->my);
}

/* force all chameleons to become normal */
void
rescham(void)
{
    struct monst *mtmp;
    int mcham;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;

        mcham = (int) mtmp->cham;
        if (mcham >= LOW_PM) {
            (void) newcham(mtmp, &mons[mcham], FALSE, FALSE);
            mtmp->cham = NON_PM;
        }
        if (is_were(mtmp->data) && mtmp->data->mlet != S_HUMAN) {
            new_were(mtmp);
        }
        if (M_AP_TYPE(mtmp) && cansee(mtmp->mx, mtmp->my)) {
            seemimic(mtmp);
            /* we pretend that the mimic doesn't */
            /* know that it has been unmasked.   */
            mtmp->msleeping = 1;
        }
    }
}

/* Let the chameleons change again -dgk */
void
restartcham(void)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        if (!mtmp->mcan) {
            mtmp->cham = pm_to_cham(monsndx(mtmp->data));
        }
        if (mtmp->data->mlet == S_MIMIC && mtmp->msleeping &&
            cansee(mtmp->mx, mtmp->my)) {
            set_mimic_sym(mtmp);
            newsym(mtmp->mx, mtmp->my);
        }
    }
}

/* called when restoring a monster from a saved level; protection
   against shape-changing might be different now than it was at the
   time the level was saved. */
void
restore_cham(struct monst *mon)
{
    int mcham;

    if (Protection_from_shape_changers) {
        mcham = (int) mon->cham;
        if (mcham >= LOW_PM) {
            mon->cham = NON_PM;
            (void) newcham(mon, &mons[mcham], FALSE, FALSE);
        } else if (is_were(mon->data) && !is_human(mon->data)) {
            new_were(mon);
        }
    } else if (mon->cham == NON_PM) {
        mon->cham = pm_to_cham(monsndx(mon->data));
    }
}

/* unwatched hiders may hide again; if so, returns True */
static boolean
restrap(struct monst *mtmp)
{
    struct trap *t;

    if (mtmp->mcan || M_AP_TYPE(mtmp) || cansee(mtmp->mx, mtmp->my) ||
        rn2(3) || mtmp == u.ustuck ||
        /* can't hide while trapped except in pits */
        (mtmp->mtrapped && (t = t_at(mtmp->mx, mtmp->my)) && !is_pit(t->ttyp)) ||
        (sensemon(mtmp) && next2u(mtmp->mx, mtmp->my))) {
        return FALSE;
    }

    if (mtmp->data->mlet == S_MIMIC) {
        set_mimic_sym(mtmp);
        return TRUE;
    } else if (levl[mtmp->mx][mtmp->my].typ == ROOM) {
        mtmp->mundetected = 1;
        return TRUE;
    }

    return FALSE;
}

/* reveal a hiding monster at x,y, either under nonexistent object,
   or an eel out of water. */
void
maybe_unhide_at(coordxy x, coordxy y)
{
    struct monst *mtmp;
    boolean undetected = FALSE, trapped = FALSE;

    if ((mtmp = m_at(x, y)) != (struct monst *) 0) {
        undetected = mtmp->mundetected;
        trapped = mtmp->mtrapped;
    } else if (u_at(x, y)) {
        mtmp = &youmonst;
        undetected = u.uundetected;
        trapped = u.utrap;
    } else {
        return;
    }

    if (undetected &&
        ((hides_under(mtmp->data) &&
          (!OBJ_AT(x, y) || trapped || !can_hide_under_obj(level.objects[x][y]))) ||
         (mtmp->data->mlet == S_EEL && !is_pool(x, y)))) {
        (void) hideunder(mtmp);
    }
}

/* monster/hero tries to hide under something at the current location */
boolean
hideunder(struct monst *mtmp)
{
    struct trap *t;
    boolean undetected = FALSE, is_u = (mtmp == &youmonst);
    coordxy x = is_u ? u.ux : mtmp->mx, y = is_u ? u.uy : mtmp->my;

    if (mtmp == u.ustuck) {
        ; /* can't hide if holding you or held by you */
    } else if (is_u ? (u.utrap && u.utraptype != TT_PIT)
                    : (mtmp->mtrapped && (t = t_at(x, y)) != 0
                       && !is_pit(t->ttyp))) {
        ; /* can't hide while stuck in a non-pit trap */
    } else if (mtmp->data->mlet == S_EEL) {
        undetected = (is_pool(x, y) && !Is_waterlevel(&u.uz));
    } else if (hides_under(mtmp->data) && OBJ_AT(x, y)) {
        struct obj *otmp = level.objects[x][y];

        /* most monsters won't hide under cockatrice corpse */
        if (otmp->nexthere || otmp->otyp != CORPSE ||
            (mtmp == &youmonst ? Stone_resistance : resists_ston(mtmp)) ||
            !touch_petrifies(&mons[otmp->corpsenm])) {
            undetected = TRUE;
        }
    }

    if (is_u)
        u.uundetected = undetected;
    else
        mtmp->mundetected = undetected;
    return undetected;
}

/* called when returning to a previously visited level */
void
hide_monst(struct monst *mon)
{
    boolean hider_under = hides_under(mon->data) || mon->data->mlet == S_EEL;

    if ((is_hider(mon->data) || hider_under) &&
         !(mon->mundetected || M_AP_TYPE(mon))) {
        coordxy x = mon->mx, y = mon->my;
        char save_viz = viz_array[y][x];

        /* override vision, forcing hero to be unable to see monster's spot */
        viz_array[y][x] &= ~(IN_SIGHT | COULD_SEE);
        if (is_hider(mon->data)) {
            (void) restrap(mon);
        }
        /* try again if mimic missed its 1/3 chance to hide */
        if (mon->data->mlet == S_MIMIC && !M_AP_TYPE(mon)) {
            (void) restrap(mon);
        }
        viz_array[y][x] = save_viz;
        if (hider_under) {
            (void) hideunder(mon);
        }
    }
}

short *animal_list = 0;     /* list of PM values for animal monsters */
int animal_list_count;

void
mon_animal_list(boolean construct)
{
    if (construct) {
        short animal_temp[SPECIAL_PM];
        int i, n;

        /* if (animal_list) impossible("animal_list already exists"); */

        for (n = 0, i = LOW_PM; i < SPECIAL_PM; i++) {
            if (is_animal(&mons[i])) animal_temp[n++] = i;
        }
        /* if (n == 0) animal_temp[n++] = NON_PM; */

        animal_list = (short *)alloc(n * sizeof *animal_list);
        (void) memcpy((genericptr_t)animal_list,
                      (genericptr_t)animal_temp,
                      n * sizeof *animal_list);
        animal_list_count = n;
    } else {    /* release */
        if (animal_list) free((genericptr_t)animal_list), animal_list = 0;
        animal_list_count = 0;
    }
}

static int
pick_animal(void)
{
    if (!animal_list) {
        mon_animal_list(TRUE);
    }

    int res = animal_list[rn2(animal_list_count)];
    /* rogue level should use monsters represented by uppercase letters
       only, but since chameleons aren't generated there (not uppercase!)
       we don't perform a lot of retries */
    if (Is_rogue_level(&u.uz) && !isupper((uchar) mons[res].mlet)) {
        res = animal_list[rn2(animal_list_count)];
    }
    return res;
}

void
decide_to_shapeshift(struct monst *mon, int shiftflags)
{
    struct permonst *ptr = 0;
    int mndx;
    unsigned was_female = mon->female;
    boolean msg = FALSE, dochng = FALSE;

    if ((shiftflags & SHIFT_MSG) ||
        ((shiftflags & SHIFT_SEENMSG) && sensemon(mon))) {
        msg = TRUE;
    }

    if (!is_vampshifter(mon)) {
        /* regular shapeshifter */
        if (!rn2(6)) {
            dochng = TRUE;
        }
    } else {
        /* The vampire has to be in good health (mhp) to maintain
         * its shifted form.
         *
         * If we're shifted and getting low on hp, maybe shift back, or
         * if we're a fog cloud at full hp, maybe pick a different shape.
         * If we're not already shifted and in good health, maybe shift.
         */
        if (mon->data->mlet != S_VAMPIRE) {
            if ((mon->mhp <= (mon->mhpmax + 5) / 6) && rn2(4) && mon->cham >= LOW_PM) {
                ptr = &mons[mon->cham];
                dochng = TRUE;
            } else if (mon->data == &mons[PM_FOG_CLOUD] &&
                       mon->mhp == mon->mhpmax &&
                       !rn2(4) &&
                       (!canseemon(mon) ||
                        mdistu(mon) > BOLT_LIM * BOLT_LIM)) {
                /* if a fog cloud, maybe change to wolf or vampire bat;
                   those are more likely to take damage--at least when
                   tame--and then switch back to vampire; they'll also
                   switch to fog cloud if they encounter a closed door */
                mndx = pickvampshape(mon);
                if (mndx >= LOW_PM) {
                    ptr = &mons[mndx];
                    dochng = (ptr != mon->data);
                }
            }
        } else {
            if (mon->mhp >= 9 * mon->mhpmax / 10 && !rn2(6) &&
                (!canseemon(mon) ||
                 mdistu(mon) > BOLT_LIM * BOLT_LIM)) {
                dochng = TRUE; /* 'ptr' stays Null */
            }
        }
    }
    if (dochng) {
        if (newcham(mon, ptr, FALSE, msg) && is_vampshifter(mon)) {
            /* for vampshift, override the 10% chance for sex change */
            ptr = mon->data;
            if (!is_male(ptr) && !is_female(ptr) && !is_neuter(ptr)) {
                mon->female = was_female;
            }
        }
    }
}

static int
pickvampshape(struct monst *mon)
{
    int mndx = mon->cham, wolfchance = 10;
    /* avoid picking monsters with lowercase display symbols ('d' for wolf
       and 'v' for fog cloud) on rogue level*/
    boolean uppercase_only = Is_rogue_level(&u.uz);

    switch (mndx) {
    case PM_VLAD_THE_IMPALER:
        /* ensure Vlad can keep carrying the Candelabrum */
        if (mon_has_special(mon))
            break; /* leave mndx as is */
        wolfchance = 3;
    /* fall through */
    case PM_VAMPIRE_LORD: /* vampire lord or Vlad can become wolf */
        if (!rn2(wolfchance) && !is_open_air(mon->mx, mon->my)) {
            mndx = PM_WOLF;
            break;
        }
    /* fall through*/
    case PM_VAMPIRE: /* any vampire can become fog or bat */
        mndx = (!rn2(4) && !uppercase_only) ? PM_FOG_CLOUD : PM_VAMPIRE_BAT;
        break;
    }
    return mndx;
}

/* nonshapechangers who warrant special polymorph handling */
static boolean
isspecmon(struct monst *mon)
{
    return (mon->isshk ||
            mon->ispriest ||
            mon->isgd ||
            mon->m_id == quest_status.leader_m_id);
}

/* restrict certain special monsters (shopkeepers, aligned priests,
   vault guards) to forms that allow them to behave sensibly (catching
   gold, speaking?) so that they don't need too much extra code */
static boolean
validspecmon(struct monst *mon, int mndx)
{
    if (mndx == NON_PM) {
        return TRUE; /* caller wants random */
    }

    if (!accept_newcham_form(mon, mndx)) {
        return FALSE; /* geno'd or !polyok */
    }

    if (isspecmon(mon)) {
        struct permonst *ptr = &mons[mndx];

        /* reject notake because object manipulation is expected
           and nohead because speech capability is expected */
        if (notake(ptr) || !has_head(ptr)) {
            return FALSE;
        }
        /* [should we check ptr->msound here too?] */
    }
    return TRUE; /* potential new form is ok */
}

/* prevent wizard mode user from specifying invalid vampshifter shape */
boolean
validvamp(struct monst *mon, int *mndx_p, int monclass)
{
    /* simplify caller's usage */
    if (!is_vampshifter(mon)) {
        return validspecmon(mon, *mndx_p);
    }

    if (mon->cham == PM_VLAD_THE_IMPALER && mon_has_special(mon)) {
        /* Vlad with Candelabrum; override choice, then accept it */
        *mndx_p = PM_VLAD_THE_IMPALER;
        return TRUE;
    }
    if (*mndx_p >= LOW_PM && is_shapeshifter(&mons[*mndx_p])) {
        /* player picked some type of shapeshifter; use mon's self
           (vampire or chameleon) */
        *mndx_p = mon->cham;
        return TRUE;
    }
    /* basic vampires can't become wolves; any can become fog or bat
       (we don't enforce upper-case only for rogue level here) */
    if (*mndx_p == PM_WOLF) {
        return (boolean) (mon->cham != PM_VAMPIRE);
    }
    if (*mndx_p == PM_FOG_CLOUD || *mndx_p == PM_VAMPIRE_BAT) {
        return TRUE;
    }

    /* if we get here, specific type was no good; try by class */
    switch (monclass) {
    case S_VAMPIRE:
        *mndx_p = mon->cham;
        break;
    case S_BAT:
        *mndx_p = PM_VAMPIRE_BAT;
        break;
    case S_VORTEX:
        *mndx_p = PM_FOG_CLOUD;
        break;
    case S_DOG:
        if (mon->cham != PM_VAMPIRE) {
            *mndx_p = PM_WOLF;
            break;
        }
        /* fall through */
    default:
        *mndx_p = NON_PM;
        break;
    }
    return (boolean) (*mndx_p != NON_PM);
}

int
select_newcham_form(struct monst *mon)
{
    int mndx = NON_PM;
    int tryct;

    switch (mon->cham) {
    case PM_SANDESTIN:
        if (rn2(7)) {
            mndx = pick_nasty();
        }
        break;
    case PM_DOPPELGANGER:
        if (!rn2(7)) {
            mndx = pick_nasty();
        } else if (rn2(3)) { /* role monsters */
            mndx = rn1(PM_WIZARD - PM_ARCHEOLOGIST+1, PM_ARCHEOLOGIST);
        } else if (!rn2(3)) { /* quest guardians */
            mndx = rn1(PM_APPRENTICE - PM_STUDENT + 1, PM_STUDENT);
            /* avoid own role's guardian */
            if (mndx == urole.guardnum) {
                mndx = NON_PM;
            }
        } else { /* general humanoids */
            tryct = 5;
            do {
                mndx = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
                if (humanoid(&mons[mndx]) && polyok(&mons[mndx])) {
                    break;
                }
            } while (--tryct > 0);
            if (!tryct) {
                mndx = NON_PM;
            }
        }
        break;
    case PM_CHAMELEON:
        if (!rn2(3)) mndx = pick_animal();
        break;
    case PM_VLAD_THE_IMPALER:
    case PM_VAMPIRE_LORD:
    case PM_VAMPIRE:
        mndx = pickvampshape(mon);
        break;
    case NON_PM: /* ordinary */
    {
        struct obj *m_armr = which_armor(mon, W_ARM);

        if (m_armr && Is_dragon_scales(m_armr->otyp))
            mndx = Dragon_scales_to_pm(m_armr) - mons;
        else if (m_armr && Is_dragon_mail(m_armr->otyp))
            mndx = Dragon_mail_to_pm(m_armr) - mons;
    }
    break;
    }
#ifdef WIZARD
    /* For debugging only: allow control of polymorphed monster; not saved */
    if (wizard && iflags.mon_polycontrol) {
        char pprompt[BUFSZ], parttwo[QBUFSZ], buf[BUFSZ];
        int monclass, len;

        /* construct prompt in pieces */
        Sprintf(pprompt, "Change %s", noit_mon_nam(mon));
        Sprintf(parttwo, " @ %s into what?",
                coord_desc((int) mon->mx, (int) mon->my, buf,
                           (iflags.getpos_coords != GPCOORDS_NONE) ?
                            iflags.getpos_coords : GPCOORDS_MAP));
        /* combine the two parts, not exceeding QBUFSZ-1 in overall length;
           if combined length is too long it has to be due to monster's
           name so we'll chop enough of that off to fit the second part */
        if ((len = (int) strlen(pprompt) + (int) strlen(parttwo)) >= QBUFSZ) {
            /* strlen(parttwo) is less than QBUFSZ/2 so strlen(pprompt) is
               more than QBUFSZ/2 and excess amount being truncated can't
               exceed pprompt's length and back up to before &pprompt[0]) */
            *(eos(pprompt) - (len - (QBUFSZ - 1))) = '\0';
        }
        Strcat(pprompt, parttwo);

        buf[0] = '\0'; /* clear buffer for EDIT_GETLIN */
#define TRYLIMIT 5
        tryct = TRYLIMIT;
        do {
            if (tryct == TRYLIMIT-1) {
                /* first retry */
                /* change "into what?" to "into what kind of monster?" */
                if (strlen(pprompt) + sizeof " kind of monster" - 1 < QBUFSZ) {
                    Strcpy(eos(pprompt) - 1, " kind of monster?");
                }
            }
#undef TRYLIMIT
            monclass = 0;
            getlin(pprompt, buf);
            mungspaces(buf);
            /* for ESC, take form selected above (might be NON_PM) */
            if (*buf == '\033') {
                break;
            }
            /* for "*", use NON_PM to pick an arbitrary shape below */
            if (!strcmp(buf, "*") || !strcmpi(buf, "random")) {
                mndx = NON_PM;
                break;
            }
            mndx = name_to_mon(buf);
            if (mndx == NON_PM) {
                /* didn't get a type, so check whether it's a class
                   (single letter or text match with def_monsyms[]) */
                monclass = name_to_monclass(buf, &mndx);
                if (monclass && mndx == NON_PM) {
                    mndx = mkclass_poly(monclass);
                }
            }
            if (mndx >= LOW_PM) {
                /* got a specific type of monster; use it if we can */
                if (validvamp(mon, &mndx, monclass)) {
                    break;
                }
                /* can't; revert to random in case we exhaust tryct */
                mndx = NON_PM;
            }

            pline("It can't become that.");
        } while (--tryct > 0);

        if (!tryct) {
            pline("%s", thats_enough_tries);
        }
        if (is_vampshifter(mon) && !validvamp(mon, &mndx, monclass)) {
            mndx = pickvampshape(mon); /* don't resort to arbitrary */
        }
    }
#endif /*WIZARD*/
    if (mndx == NON_PM) mndx = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
    return mndx;
}

/* this used to be inline within newcham() but monpolycontrol needs it too */
static struct permonst *
accept_newcham_form(struct monst *mon, int mndx)
{
    struct permonst *mdat;

    if (mndx == NON_PM) {
        return 0;
    }
    mdat = &mons[mndx];
    if ((mvitals[mndx].mvflags & G_GENOD) != 0) {
        return 0;
    }
    if (is_placeholder(mdat)) {
        return 0;
    }
    /* select_newcham_form() might deliberately pick a player
       character type (random selection never does) which
       polyok() rejects, so we need a special case here */
    if (is_mplayer(mdat)) {
        return mdat;
    }
    /* shapeshifters are rejected by polyok() but allow a shapeshifter
       to take on its 'natural' form */
    if (is_shapeshifter(mdat) && mon->cham >= LOW_PM && mdat == &mons[mon->cham]) {
        return mdat;
    }
    /* polyok() rules out M2_PNAME, M2_WERE, and all humans except Kops */
    return polyok(mdat) ? mdat : 0;
}

void
mgender_from_permonst(struct monst *mtmp, struct permonst *mdat)
{
    if (is_male(mdat)) {
        if (mtmp->female) {
            mtmp->female = FALSE;
        }
    } else if (is_female(mdat)) {
        if (!mtmp->female) {
            mtmp->female = TRUE;
        }
    } else if (!is_neuter(mdat)) {
        if (!rn2(10)) {
            mtmp->female = !mtmp->female;
        }
    }
}

/* make a chameleon take on another shape, or a polymorph target
   (possibly self-inflicted) become a different monster;
   returns 1 if it actually changes form */
int
newcham(
    struct monst *mtmp,
    struct permonst *mdat,
    boolean polyspot, /**< change is the result of wand or spell of polymorph */
    boolean msg) /**< "The oldmon turns into a newmon!" */
{
    int hpn, hpd;
    int mndx, tryct;
    struct permonst *olddata = mtmp->data;
    char *p, oldname[BUFSZ], l_oldname[BUFSZ], newname[BUFSZ];

    /* Riders are immune to polymorph and green slime
       (but apparent Rider might actually be a doppelganger) */
    if (mtmp->cham == NON_PM) {
        /* not a shapechanger */
        if (is_rider(olddata)) {
            return 0;
        }
        /* make Nazgul and erinyes immune too, to reduce chance of
           anomalous extinction feedback during final disclsoure */
        if (mbirth_limit(monsndx(olddata)) < MAXMONNO) {
            return 0;
        }
        /* cancelled shapechangers become uncancelled prior
           to being given a new shape */
        if (mtmp->mcan && !Protection_from_shape_changers) {
            mtmp->cham = pm_to_cham(monsndx(mtmp->data));
            if (mtmp->cham != NON_PM) {
                mtmp->mcan = 0;
            }
        }
    }

    if (msg) {
        /* like Monnam() but never mention saddle */
        Strcpy(oldname, x_monnam(mtmp, ARTICLE_THE, (char *)0,
                                 SUPPRESS_SADDLE, FALSE));
        oldname[0] = highc(oldname[0]);
    }

    /* we need this one whether msg is true or not */
    Strcpy(l_oldname, x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                               has_mgivenname(mtmp) ? SUPPRESS_SADDLE : 0, FALSE));

    /* mdat = 0 -> caller wants a random monster shape */
    tryct = 0;
    if (mdat == 0) {
        /* select_newcham_form() loops when resorting to random but
           it doesn't always pick that so we still retry here too */
        tryct = 20;
        do {
            mndx = select_newcham_form(mtmp);
            mdat = accept_newcham_form(mtmp, mndx);
            /* for the first several tries we require upper-case on
               the rogue level (after that, we take whatever we get) */
            if (mdat) {
                break;
            }
        } while (--tryct > 0);
        if (tryct == 0) {
            return 0;
        }
    } else if (mvitals[monsndx(mdat)].mvflags & G_GENOD) {
        return 0; /* passed in mdat is genocided */
    }

    if (mdat == olddata) {
        return 0; /* still the same monster */
    }

    mgender_from_permonst(mtmp, mdat);

    /* Endgame mplayers start out as "Foo the Bar", but some of the
     * titles are inappropriate when polymorphed, particularly into
     * the opposite sex.  Player characters don't use ranks when
     * polymorphed, so dropping rank for mplayers seems reasonable.
     */
    if (In_endgame(&u.uz) &&
         is_mplayer(olddata) &&
         has_mgivenname(mtmp) &&
         (p = strstr(MGIVENNAME(mtmp), " the ")) != 0) {
        *p = '\0';
    }

    if (mtmp->wormno) {
        /* throw tail away */
        wormgone(mtmp);
        place_monster(mtmp, mtmp->mx, mtmp->my);
    }
    if (M_AP_TYPE(mtmp) && mdat->mlet != S_MIMIC) {
        seemimic(mtmp); /* revert to normal monster */
    }

    /* (this code used to try to adjust the monster's health based on
       a normal one of its type but there are too many special cases
       which need to handled in order to do that correctly, so just
       give the new form the same proportion of HP as its old one had) */
    hpn = mtmp->mhp;
    hpd = mtmp->mhpmax;
    /* set level and hit points */
    newmonhp(mtmp, monsndx(mdat));
    /* new hp: same fraction of max as before */
    mtmp->mhp = (int)(((long)hpn*(long)mtmp->mhp)/(long)hpd);
    /* sanity check (potential overflow) */
    if (mtmp->mhp < 0 || mtmp->mhp > mtmp->mhpmax) {
        mtmp->mhp = mtmp->mhpmax;
    }
    /* unlikely but not impossible; a 1HD creature with 1HP that changes
       into a 0HD creature will require this statement */
    if (!mtmp->mhp) {
        mtmp->mhp = 1;
    }

    /* take on the new form... */
    set_mon_data(mtmp, mdat);

    if (mtmp->mleashed) {
        if (!leashable(mtmp)) {
            m_unleash(mtmp, TRUE);
        } else {
            /* if leashed, persistent inventory window needs updating
               (really only when mon_nam() is going to yield "a frog"
               rather than "Kermit" but no need to micromanage here) */
            update_inventory(); /* x - leash (attached to a <mon>) */
        }
    }

    if (emits_light(olddata) != emits_light(mtmp->data)) {
        /* used to give light, now doesn't, or vice versa,
           or light's range has changed */
        if (emits_light(olddata))
            del_light_source(LS_MONSTER, monst_to_any(mtmp));
        if (emits_light(mtmp->data))
            new_light_source(mtmp->mx, mtmp->my, emits_light(mtmp->data),
                             LS_MONSTER, monst_to_any(mtmp));
    }
    if (!mtmp->perminvis || pm_invisible(olddata))
        mtmp->perminvis = pm_invisible(mdat);
    mtmp->minvis = mtmp->invis_blkd ? 0 : mtmp->perminvis;

    if (mtmp->mundetected) {
        (void) hideunder(mtmp);
    }

    if (u.ustuck == mtmp) {
        if (u.uswallow) {
            if (!attacktype(mdat, AT_ENGL)) {
                /* Does mdat care? */
                if (!noncorporeal(mdat) && !amorphous(mdat) &&
                    !is_whirly(mdat) &&
                    (mdat != &mons[PM_YELLOW_LIGHT])) {
                    char msgtrail[BUFSZ];

                    if (is_vampshifter(mtmp)) {
                        Sprintf(msgtrail, " which was a shapeshifted %s",
                                noname_monnam(mtmp, ARTICLE_NONE));
                    } else if (is_animal(mdat)) {
                        Strcpy(msgtrail, "'s stomach");
                    } else {
                        msgtrail[0] = '\0';
                    }

                    /* Do this even if msg is FALSE */
                    You("%s %s%s!",
                        (amorphous(olddata) || is_whirly(olddata)) ? "emerge from" : "break out of",
                        l_oldname, msgtrail);
                    msg = FALSE; /* message has been given */
                    mtmp->mhp = 1;  /* almost dead */
                }
                expels(mtmp, olddata, FALSE);
            } else {
                /* update swallow glyphs for new monster */
                swallowed(0);
            }
        } else if (!sticks(mdat) && !sticks(youmonst.data))
            unstuck(mtmp);
    }

#ifndef DCC30_BUG
    if (mdat == &mons[PM_LONG_WORM] && (mtmp->wormno = get_wormno()) != 0) {
#else
    /* DICE 3.0 doesn't like assigning and comparing mtmp->wormno in the
     * same expression.
     */
    if (mdat == &mons[PM_LONG_WORM] &&
        (mtmp->wormno = get_wormno(), mtmp->wormno != 0)) {
#endif
        /* we can now create worms with tails - 11/91 */
        initworm(mtmp, rn2(5));
        place_worm_tail_randomly(mtmp, mtmp->mx, mtmp->my);
    }

    newsym(mtmp->mx, mtmp->my);

    if (msg) {
        Strcpy(newname, noname_monnam(mtmp, ARTICLE_A));
        /* oldname was capitalized above; newname will be lower case */
        if (!strcmpi(newname, "it")) { /* can't see or sense it now */
            if (!!strcmpi(oldname, "it")) { /* could see or sense it before */
                pline("%s disappears!", oldname);
            }
            (void) usmellmon(mdat);
        } else { /* can see or sense it now */
            if (!strcmpi(oldname, "it")) { /* couldn't see or sense it before */
                pline("%s appears!", upstart(newname));
            } else {
                pline("%s turns into %s!", oldname, newname);
            }
        }
    }

    /* when polymorph trap/wand/potion produces a vampire, turn in into
       a full-fledged vampshifter unless shape-changing is blocked */
    if (mtmp->cham == NON_PM && mdat->mlet == S_VAMPIRE && !Protection_from_shape_changers) {
        mtmp->cham = pm_to_cham(monsndx(mdat));
    }

    possibly_unwield(mtmp, polyspot); /* might lose use of weapon */
    mon_break_armor(mtmp, polyspot);
    if (!(mtmp->misc_worn_check & W_ARMG))
        mselftouch(mtmp, "No longer petrify-resistant, ",
                   !flags.mon_moving);
    m_dowear(mtmp, FALSE);

    /* This ought to re-test can_carry() on each item in the inventory
     * rather than just checking ex-giants & boulders, but that'd be
     * pretty expensive to perform.  If implemented, then perhaps
     * minvent should be sorted in order to drop heaviest items first.
     */
    /* former giants can't continue carrying boulders */
    if (mtmp->minvent && !throws_rocks(mdat)) {
        struct obj *otmp, *otmp2;

        for (otmp = mtmp->minvent; otmp; otmp = otmp2) {
            otmp2 = otmp->nobj;
            if (otmp->otyp == BOULDER) {
                /* this keeps otmp from being polymorphed in the
                   same zap that the monster that held it is polymorphed */
                if (polyspot) bypass_obj(otmp);
                obj_extract_self(otmp);
                /* probably ought to give some "drop" message here */
                if (flooreffects(otmp, mtmp->mx, mtmp->my, "")) continue;
                place_object(otmp, mtmp->mx, mtmp->my);
            }
        }
    }
    if (mtmp == u.usteed) {
        poly_steed(mtmp, olddata);
    }

    return 1;
}

/* sometimes an egg will be special */
#define BREEDER_EGG (!rn2(77))

/*
 * Determine if the given monster number can be hatched from an egg.
 * Return the monster number to use as the egg's corpsenm.  Return
 * NON_PM if the given monster can't be hatched.
 */
int
can_be_hatched(int mnum)
{
    /* ranger quest nemesis has the oviparous bit set, making it
       be possible to wish for eggs of that unique monster; turn
       such into ordinary eggs rather than forbidding them outright */
    if (mnum == PM_SCORPIUS) mnum = PM_SCORPION;

    mnum = little_to_big(mnum);
    /*
     * Queen bees lay killer bee eggs (usually), but killer bees don't
     * grow into queen bees.  Ditto for [winged-]gargoyles.
     */
    if (mnum == PM_KILLER_BEE || mnum == PM_GARGOYLE ||
        (lays_eggs(&mons[mnum]) && (BREEDER_EGG ||
                                    (mnum != PM_QUEEN_BEE && mnum != PM_WINGED_GARGOYLE))))
        return mnum;
    return NON_PM;
}

/* type of egg laid by #sit; usually matches parent */
int
egg_type_from_parent(int mnum, boolean force_ordinary)
            /* parent monster; caller must handle lays_eggs() check */

{
    if (force_ordinary || !BREEDER_EGG) {
        if (mnum == PM_QUEEN_BEE) mnum = PM_KILLER_BEE;
        else if (mnum == PM_WINGED_GARGOYLE) mnum = PM_GARGOYLE;
    }
    return mnum;
}

/* decide whether an egg of the indicated monster type is viable; */
/* also used to determine whether an egg or tin can be created... */
boolean
dead_species(int m_idx, boolean egg)
{
    /* generic eggs are unhatchable and have corpsenm of NON_PM */
    if (m_idx < LOW_PM) {
        return TRUE;
    }
    /*
     * For monsters with both baby and adult forms, genociding either
     * form kills all eggs of that monster.  Monsters with more than
     * two forms (small->large->giant mimics) are more or less ignored;
     * fortunately, none of them have eggs.  Species extinction due to
     * overpopulation does not kill eggs.
     */
    int alt_idx = egg ? big_to_little(m_idx) : m_idx;
    return ((mvitals[m_idx].mvflags & G_GENOD) ||
            (mvitals[alt_idx].mvflags & G_GENOD));
}

/* kill off any eggs of genocided monsters */
static void
kill_eggs(struct obj *obj_list)
{
    struct obj *otmp;

    for (otmp = obj_list; otmp; otmp = otmp->nobj) {
        if (otmp->otyp == EGG) {
            if (dead_species(otmp->corpsenm, TRUE)) {
                /*
                 * It seems we could also just catch this when
                 * it attempted to hatch, so we wouldn't have to
                 * search all of the objlists.. or stop all
                 * hatch timers based on a corpsenm.
                 */
                kill_egg(otmp);
            }
#if 0   /* not used */
        } else if (otmp->otyp == TIN) {
            if (dead_species(otmp->corpsenm, FALSE))
                otmp->corpsenm = NON_PM; /* empty tin */
        } else if (otmp->otyp == CORPSE) {
            if (dead_species(otmp->corpsenm, FALSE))
                ;   /* not yet implemented... */
#endif
        } else if (Has_contents(otmp)) {
            kill_eggs(otmp->cobj);
        }
    }
}

/** kill all members of genocided species */
void
kill_genocided_monsters(void)
{
    struct monst *mtmp, *mtmp2;
    boolean kill_cham;
    int mndx;

    /*
     * Called during genocide, and again upon level change.  The latter
     * catches up with any migrating monsters as they finally arrive at
     * their intended destinations, so possessions get deposited there.
     *
     * Chameleon handling:
     *  1) if chameleons have been genocided, destroy them
     *     regardless of current form;
     *  2) otherwise, force every chameleon which is imitating
     *     any genocided species to take on a new form.
     */
    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp)) continue;
        mndx = monsndx(mtmp->data);
        kill_cham = (mtmp->cham >= LOW_PM &&
                     (mvitals[mtmp->cham].mvflags & G_GENOD));
        if ((mvitals[mndx].mvflags & G_GENOD) || kill_cham) {
            if (mtmp->cham >= LOW_PM && !kill_cham) {
                (void) newcham(mtmp, (struct permonst *)0, FALSE, FALSE);
            } else {
                mondead(mtmp);
            }
        }
        if (mtmp->minvent) kill_eggs(mtmp->minvent);
    }

    kill_eggs(invent);
    kill_eggs(fobj);
    kill_eggs(migrating_objs);
    kill_eggs(level.buriedobjlist);
}

/**
 * Kills every member of the specified monster species on the current
 * level.
 */
void
kill_monster_on_level(int mndx) /**< Monster index number */
{
    struct monst *mtmp;
    struct monst *mtmp2;
    int tmp_mndx;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp)) continue;
        tmp_mndx = monsndx(mtmp->data);
        if (mndx == tmp_mndx) {
            mondead(mtmp);
        }
    }
}

void
golemeffects(struct monst *mon, int damtype, int dam)
{
    int heal = 0, slow = 0;

    if (mon->data == &mons[PM_FLESH_GOLEM]) {
        if (damtype == AD_ELEC) {
            heal = (dam + 5) / 6;
        } else if (damtype == AD_FIRE || damtype == AD_COLD) {
            slow = 1;
        }
    } else if (mon->data == &mons[PM_IRON_GOLEM]) {
        if (damtype == AD_ELEC) slow = 1;
        else if (damtype == AD_FIRE) heal = dam;
    } else {
        return;
    }
    if (slow) {
        if (mon->mspeed != MSLOW)
            mon_adjust_speed(mon, -1, (struct obj *)0);
    }
    if (heal) {
        if (mon->mhp < mon->mhpmax) {
            mon->mhp += heal;
            if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
            if (cansee(mon->mx, mon->my))
                pline("%s seems healthier.", Monnam(mon));
        }
    }
}

boolean
angry_guards(boolean silent)
{
    struct monst *mtmp;
    int ct = 0, nct = 0, sct = 0, slct = 0;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        if ((mtmp->data == &mons[PM_WATCHMAN] ||
            mtmp->data == &mons[PM_WATCH_CAPTAIN])
           && mtmp->mpeaceful) {
            ct++;
            if (cansee(mtmp->mx, mtmp->my) && mtmp->mcanmove) {
                if (next2u(mtmp->mx, mtmp->my)) {
                    nct++;
                } else {
                    sct++;
                }
            }
            if (mtmp->msleeping || mtmp->mfrozen) {
                slct++;
                mtmp->msleeping = mtmp->mfrozen = 0;
            }
            mtmp->mpeaceful = 0;
        }
    }
    if (ct) {
        if (!silent) { /* do we want pline msgs? */
            if (slct) pline_The("guard%s wake%s up!",
                               slct > 1 ? "s" : "", slct == 1 ? "s" : "");
            if (nct || sct) {
                if (nct) pline_The("guard%s get%s angry!",
                                  nct == 1 ? "" : "s", nct == 1 ? "s" : "");
                else if (!Blind)
                    You_see("%sangry guard%s approaching!",
                        sct == 1 ? "an " : "", sct > 1 ? "s" : "");
            } else {
                You_hear("the shrill sound of a guard's whistle.");
            }
        }
        return TRUE;
    }
    return FALSE;
}

void
pacify_guards(void)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) continue;
        if (is_watch(mtmp->data)) {
            mtmp->mpeaceful = 1;
        }
    }
}

void
mimic_hit_msg(struct monst *mtmp, short int otyp)
{
    short ap = mtmp->mappearance;

    switch (M_AP_TYPE(mtmp)) {
    case M_AP_NOTHING:
    case M_AP_FURNITURE:
    case M_AP_MONSTER:
        break;
    case M_AP_OBJECT:
        if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING) {
            pline("%s seems a more vivid %s than before.",
                  The(simple_typename(ap)),
                  c_obj_colors[objects[ap].oc_color]);
        }
        break;
    }
}

boolean
usmellmon(struct permonst *mdat)
{
    int mndx;
    boolean nonspecific = FALSE;
    boolean msg_given = FALSE;

    if (mdat) {
        if (!olfaction(youmonst.data))
            return FALSE;
        mndx = monsndx(mdat);
        switch (mndx) {
        case PM_ROTHE:
        case PM_MINOTAUR:
            You("notice a bovine smell.");
            msg_given = TRUE;
            break;
        case PM_CAVEMAN:
        case PM_CAVEWOMAN:
        case PM_BARBARIAN:
        case PM_NEANDERTHAL:
            You("smell body odor.");
            msg_given = TRUE;
            break;
        /*
        case PM_PESTILENCE:
        case PM_FAMINE:
        case PM_DEATH:
            break;
        */
        case PM_HORNED_DEVIL:
        case PM_BALROG:
        case PM_ASMODEUS:
        case PM_DISPATER:
        case PM_YEENOGHU:
        case PM_ORCUS:
            break;
        case PM_HUMAN_WEREJACKAL:
        case PM_HUMAN_WERERAT:
        case PM_HUMAN_WEREWOLF:
        case PM_WEREJACKAL:
        case PM_WERERAT:
        case PM_WEREWOLF:
        case PM_OWLBEAR:
            You("detect an odor reminiscent of an animal's den.");
            msg_given = TRUE;
            break;
        /*
        case PM_PURPLE_WORM:
            break;
        */
        case PM_STEAM_VORTEX:
            You("smell steam.");
            msg_given = TRUE;
            break;
        case PM_GREEN_SLIME:
            pline("%s stinks.", Something);
            msg_given = TRUE;
            break;
        case PM_VIOLET_FUNGUS:
        case PM_SHRIEKER:
            You("smell mushrooms.");
            msg_given = TRUE;
            break;
        /* These are here to avoid triggering the
           nonspecific treatment through the default case below*/
        case PM_WHITE_UNICORN:
        case PM_GRAY_UNICORN:
        case PM_BLACK_UNICORN:
        case PM_JELLYFISH:
            break;
        default:
            nonspecific = TRUE;
            break;
        }

        if (nonspecific)
            switch (mdat->mlet) {
            case S_DOG:
                You("notice a dog smell.");
                msg_given = TRUE;
                break;
            case S_DRAGON:
                You("smell a dragon!");
                msg_given = TRUE;
                break;
            case S_FUNGUS:
                pline("%s smells moldy.", Something);
                msg_given = TRUE;
                break;
            case S_UNICORN:
                You("detect a%s odor reminiscent of a stable.",
                    (mndx == PM_PONY) ? "n" : " strong");
                msg_given = TRUE;
                break;
            case S_ZOMBIE:
                You("smell rotting flesh.");
                msg_given = TRUE;
                break;
            case S_EEL:
                You("smell fish.");
                msg_given = TRUE;
                break;
            case S_ORC:
                if (maybe_polyd(is_orc(youmonst.data), Race_if(PM_ORC)))
                    You("notice an attractive smell.");
                else
                    pline("A foul stench makes you feel a little nauseated.");
                msg_given = TRUE;
                break;
            default:
                break;
            }
    }
    return msg_given ? TRUE : FALSE;
}

/* u_aireffects() equivalent for monsters. */
void
mon_aireffects(struct monst *mtmp)
{
    d_level destination;
    boolean to_nowhere, cansee = canseemon(mtmp);

    find_level_beneath(&u.uz, &destination);
    to_nowhere = (destination.dnum == 0 && destination.dlevel == 0);

    if (cansee) {
        pline("Air currents pull %s down.", mon_nam(mtmp));
    }
    mtmp->mhp -= d(20, 12);
    mtmp->mstun = 1;

    if (DEADMONSTER(mtmp) || to_nowhere) {
        /* Send items to the lower floor (possibly breaking them) silently,
         * rather than letting xkilled() drop them noisily */
        struct obj *otmp, *nobj;
        for (otmp = mtmp->minvent; otmp; otmp = nobj) {
            nobj = otmp->nobj;
            obj_extract_self(otmp);
            (void) obj_aireffects(otmp, FALSE);
        }
        /* No corpse is dropped here, because it would give a message about the
         * corpse falling away and disappearing which doesn't make any sense.
         * Assume it splatted on the floor into an unrecognizable state. */
        if (flags.mon_moving) {
            mongone(mtmp);
        } else {
            xkilled(mtmp, XKILL_NOMSG | XKILL_NOCORPSE);
        }
    } else {
        migrate_to_level(mtmp, ledger_no(&destination), MIGR_RANDOM, (coord *) 0);
    }
}

/* setting misc_worn_check's I_SPECIAL bit flags a monster to reassess
   and potentially re-equip gear at the start of its next move;
   this hides the details of that */
void
check_gear_next_turn(struct monst *mon)
{
    mon->misc_worn_check |= I_SPECIAL;
}

/* mon.c */
