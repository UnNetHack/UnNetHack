/*  SCCS Id: @(#)restore.c  3.4 2003/09/06  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "tcap.h" /* for TERMLIB and ASCIIGRAPH */
#include <assert.h>

#if defined(MICRO)
extern int dotcnt;  /* shared with save */
extern int dotrow;  /* shared with save */
#endif

#ifdef USE_TILES
extern void substitute_tiles(d_level *);       /* from tile.c */
#endif

#ifdef ZEROCOMP
static void zerocomp_minit(void);
static void zerocomp_mread(int, genericptr_t, unsigned int);
static int zerocomp_mgetc(void);
#endif
static void find_lev_obj(void);
static void restlevchn(NHFILE *);
static void restdamage(NHFILE *);
static void restobj(NHFILE *, struct obj *);
static struct obj *restobjchn(NHFILE *, boolean);
static void restmon(NHFILE *, struct monst *);
static struct monst *restmonchn(NHFILE *);
static struct fruit *loadfruitchn(NHFILE *);
static void freefruitchn(struct fruit *);
static void ghostfruit(struct obj *);
static boolean restgamestate(NHFILE *);
static void restlevelstate(void);
static int restlevelfile(xint8);
static void restore_gamelog(NHFILE *);
static void restore_msghistory(NHFILE *);
static void reset_oattached_mids(boolean);
static void rest_levl(NHFILE *, boolean);
static void rest_stairs(NHFILE *);

/*
 * Save a mapping of IDs from ghost levels to the current level.  This
 * map is used by the timer routines when restoring ghost levels.
 */
#define N_PER_BUCKET 64
struct bucket {
    struct bucket *next;
    struct {
        unsigned gid; /* ghost ID */
        unsigned nid; /* new ID */
    } map[N_PER_BUCKET];
};

static void clear_id_mapping(void);
static void add_id_mapping(unsigned, unsigned);

static int n_ids_mapped = 0;
static struct bucket *id_map = 0;


#ifdef AMII_GRAPHICS
void  amii_setpens(int) ;  /* use colors from save file */
extern int amii_numcolors;
#endif

#include "quest.h"

static NEARDATA struct fruit *oldfruit;
static NEARDATA long omoves;

#define Is_IceBox(o) ((o)->otyp == ICE_BOX ? TRUE : FALSE)

/* third arg passed to mread() should be 'unsigned' but most calls use
   sizeof so are attempting to pass 'size_t'; mread()'s prototype results
   in an implicit conversion; this macro does it explicitly */
#define Mread(fd,adr,siz) mread((fd), (genericptr_t) (adr), (unsigned) (siz))

/* Recalculate level.objects[x][y], since this info was not saved. */
static void
find_lev_obj(void)
{
    struct obj *fobjtmp = (struct obj *)0;
    struct obj *otmp;
    int x, y;

    for (x=0; x<COLNO; x++) {
        for (y=0; y<ROWNO; y++) {
            level.objects[x][y] = (struct obj *)0;
        }
    }

    /*
     * Reverse the entire fobj chain, which is necessary so that we can
     * place the objects in the proper order.  Make all obj in chain
     * OBJ_FREE so place_object will work correctly.
     */
    while ((otmp = fobj) != 0) {
        fobj = otmp->nobj;
        otmp->nobj = fobjtmp;
        otmp->where = OBJ_FREE;
        fobjtmp = otmp;
    }
    /* fobj should now be empty */

    /* Set level.objects (as well as reversing the chain back again) */
    while ((otmp = fobjtmp) != 0) {
        fobjtmp = otmp->nobj;
        place_object(otmp, otmp->ox, otmp->oy);
    }
}

/* Things that were marked "in_use" when the game was saved (ex. via the
 * infamous "HUP" cheat) get used up here.
 */
void
inven_inuse(boolean quietly)
{
    struct obj *otmp, *otmp2;

    for (otmp = invent; otmp; otmp = otmp2) {
        otmp2 = otmp->nobj;
        if (otmp->in_use) {
            if (!quietly) {
                pline("Finishing off %s...", xname(otmp));
            }
            useup(otmp);
        }
    }
}

static void
restlevchn(NHFILE *nhfp)
{
    int cnt = 0;
    s_level *tmplev, *x;

    sp_levchn = (s_level *) 0;
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &cnt, sizeof cnt);
    }
    for (; cnt > 0; cnt--) {
        tmplev = (s_level *)alloc(sizeof(s_level));
        if (nhfp->structlevel) {
            Mread(nhfp->fd, tmplev, sizeof *tmplev);
        }
        if (!sp_levchn) {
            sp_levchn = tmplev;
        } else {
            for (x = sp_levchn; x->next; x = x->next) {
                ;
            }
            x->next = tmplev;
        }
        tmplev->next = (s_level *)0;
    }
}

static void
restdamage(NHFILE *nhfp)
{
    unsigned int dmgcount = 0;
    int counter;
    struct damage *tmp_dam;
    boolean ghostly = (nhfp->ftype == NHF_BONESFILE);

    if (nhfp->structlevel) {
        Mread(nhfp->fd, &dmgcount, sizeof dmgcount);
    }
    counter = (int) dmgcount;

    if (!counter) {
        return;
    }
    do {
        tmp_dam = (struct damage *) alloc(sizeof *tmp_dam);
        if (nhfp->structlevel) {
            Mread(nhfp->fd, tmp_dam, sizeof *tmp_dam);
        }

        if (ghostly) {
            tmp_dam->when += (moves - omoves);
        }

        tmp_dam->next = level.damagelist;
        level.damagelist = tmp_dam;
    } while (--counter > 0);
}

struct lvl_sounds *
rest_lvl_sounds(NHFILE *nhfp)
{
    int marker = 0;
    struct lvl_sounds *or = NULL;
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &marker, sizeof marker);
    }
    if (marker) {
        or = (struct lvl_sounds *)alloc(sizeof(struct lvl_sounds));
        if (nhfp->structlevel) {
            Mread(nhfp->fd, or, sizeof *or);
        }
        or->sounds = NULL;
        if (or->n_sounds) {
            int i;
            int len;
            or->sounds = (struct lvl_sound_bite *)alloc(sizeof(struct lvl_sound_bite)*or->n_sounds);
            for (i = 0; i < or->n_sounds; i++) {
                if (nhfp->structlevel) {
                    Mread(nhfp->fd, &(or->sounds[i].flags), sizeof or->sounds[i].flags);
                    Mread(nhfp->fd, &len, sizeof len);
                }
                or->sounds[i].msg = (char *)alloc(len);
                if (nhfp->structlevel) {
                    Mread(nhfp->fd, or->sounds[i].msg, len);
                }
            }
        }
    }
    return or;
}

struct mon_gen_override *
rest_mongen_override(NHFILE *nhfp)
{
    int marker = 0;
    struct mon_gen_override *or = NULL;
    struct mon_gen_tuple *mt = NULL;
    int next;

    if (nhfp->structlevel) {
        Mread(nhfp->fd, &marker, sizeof marker );
    }
    if (marker) {
        or = (struct mon_gen_override *)alloc(sizeof(struct mon_gen_override));
        if (nhfp->structlevel) {
            Mread(nhfp->fd, or, sizeof *or);
        }
        if (or->gen_chances) {
            or->gen_chances = NULL;
            do {
                mt = (struct mon_gen_tuple *)alloc(sizeof(struct mon_gen_tuple));
                if (nhfp->structlevel) {
                    Mread(nhfp->fd, mt, sizeof *mt );
                }
                if (mt->next) {
                    next = 1;
                } else {
                    next = 0;
                }
                mt->next = or->gen_chances;
                or->gen_chances = mt;
            } while (next);
        }
    }
    return or;
}

/* restore one object */
static void
restobj(NHFILE *nhfp, struct obj *otmp)
{
    int buflen = 0;

    if (nhfp->structlevel) {
        Mread(nhfp->fd, otmp, sizeof *otmp);
    }

    /* next object pointers are invalid; otmp->cobj needs to be left
       as is--being non-null is key to restoring container contents */
    otmp->nobj = otmp->nexthere = (struct obj *) 0;
    /* non-null oextra needs to be reconstructed */
    if (otmp->oextra) {
        otmp->oextra = newoextra();

        /* oname - object's name */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }

        if (buflen > 0) { /* includes terminating '\0' */
            new_oname(otmp, buflen);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, ONAME(otmp), buflen);
            }
        }

        /* omonst - corpse or statue might retain full monster details */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            newomonst(otmp);
            /* this is actually a monst struct, so we
               can just defer to restmon() here */
            restmon(nhfp, OMONST(otmp));
        }

        /* omailcmd - feedback mechanism for scroll of mail */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            char *omailcmd = (char *) alloc(buflen);

            if (nhfp->structlevel) {
                Mread(nhfp->fd, omailcmd, buflen);
            }
            new_omailcmd(otmp, omailcmd);
            free((genericptr_t) omailcmd);
        }

        /* omid - monster id number, connecting corpse to ghost */
        newomid(otmp); /* superfluous; we're already allocated otmp->oextra */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &OMID(otmp), sizeof OMID(otmp));
        }
    }
}

static struct obj *
restobjchn(NHFILE *nhfp, boolean frozen)
{
    struct obj *otmp, *otmp2 = 0;
    struct obj *first = (struct obj *) 0;
    int buflen = 0;
    boolean ghostly = (nhfp->ftype == NHF_BONESFILE);

    while (1) {
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }

        if (buflen == -1) {
            break;
        }

        otmp = newobj();
        assert(otmp != 0);
        restobj(nhfp, otmp);
        if (!first) {
            first = otmp;
        } else {
            otmp2->nobj = otmp;
        }

        if (ghostly) {
            unsigned nid = next_ident();

            add_id_mapping(otmp->o_id, nid);
            otmp->o_id = nid;
        }
        if (ghostly && otmp->otyp == SLIME_MOLD)
            ghostfruit(otmp);
        /* Ghost levels get object age shifted from old player's clock
         * to new player's clock.  Assumption: new player arrived
         * immediately after old player died.
         */
        if (ghostly && !frozen && !age_is_relative(otmp)) {
            otmp->age = moves - omoves + otmp->age;
        }

        /* get contents of a container or statue */
        if (Has_contents(otmp)) {
            struct obj *otmp3;

            otmp->cobj = restobjchn(nhfp, Is_IceBox(otmp));
            /* restore container back pointers */
            for (otmp3 = otmp->cobj; otmp3; otmp3 = otmp3->nobj) {
                otmp3->ocontainer = otmp;
            }
        }
        if (otmp->bypass) {
            otmp->bypass = 0;
        }
        if (!ghostly) {
            /* fix the pointers */
#ifdef NEXT_VERSION
            if (otmp->o_id == context.victual.o_id) {
                context.victual.piece = otmp;
            }
            if (otmp->o_id == context.tin.o_id) {
                context.tin.tin = otmp;
            }
            if (otmp->o_id == context.spbook.o_id) {
                context.spbook.book = otmp;
            }
#endif
        }
        otmp2 = otmp;
    }
    if (first && otmp2->nobj) {
        impossible("Restobjchn: error reading objchn.");
        otmp2->nobj = 0;
    }

    return first;
}

/* restore one monster */
static void
restmon(NHFILE *nhfp, struct monst *mtmp)
{
    int buflen = 0;

    if (nhfp->structlevel) {
        Mread(nhfp->fd, mtmp, sizeof *mtmp);
    }

    /* next monster pointer is invalid */
    mtmp->nmon = (struct monst *) 0;
    /* non-null mextra needs to be reconstructed */
    if (mtmp->mextra) {
        mtmp->mextra = newmextra();

        /* mgivenname - monster's name */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) { /* includes terminating '\0' */
            new_mgivenname(mtmp, buflen);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, MGIVENNAME(mtmp), buflen);
            }
        }
        /* egd - vault guard */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }

        if (buflen > 0) {
            newegd(mtmp);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, EGD(mtmp), sizeof (struct egd));
            }
        }
        /* epri - temple priest */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            newepri(mtmp);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, EPRI(mtmp), sizeof (struct epri));
            }
        }
        /* eshk - shopkeeper */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            neweshk(mtmp);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, ESHK(mtmp), sizeof (struct eshk));
            }
        }
        /* emin - minion */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            newemin(mtmp);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, EMIN(mtmp), sizeof (struct emin));
            }
        }
        /* edog - pet */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            newedog(mtmp);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, EDOG(mtmp), sizeof (struct edog));
            }
        }
        /* mcorpsenm - obj->corpsenm for mimic posing as corpse or
           statue (inline int rather than pointer to something) */
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &MCORPSENM(mtmp), sizeof MCORPSENM(mtmp));
        }
    } /* mextra */
}

static struct monst *
restmonchn(NHFILE *nhfp)
{
    struct monst *mtmp, *mtmp2 = 0;
    struct monst *first = (struct monst *) 0;
    int offset, buflen = 0;
    boolean ghostly = (nhfp->ftype == NHF_BONESFILE);

    while (1) {
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }
        if (buflen == -1) {
            break;
        }

        mtmp = newmonst();
        assert(mtmp != 0);
        restmon(nhfp, mtmp);
        if (!first) {
            first = mtmp;
        } else {
            mtmp2->nmon = mtmp;
        }

        if (ghostly) {
            unsigned nid = next_ident();

            add_id_mapping(mtmp->m_id, nid);
            mtmp->m_id = nid;
        }
        offset = mtmp->mnum;
        mtmp->data = &mons[offset];
        if (ghostly) {
            int mndx = (mtmp->cham == NON_PM) ? monsndx(mtmp->data) : mtmp->cham;

            if (propagate(mndx, TRUE, ghostly) == 0) {
                /* cookie to trigger purge in getbones() */
                mtmp->mhpmax = DEFUNCT_MONSTER;
            }
        }
        if (mtmp->minvent) {
            struct obj *obj;
            mtmp->minvent = restobjchn(nhfp, FALSE);
            /* restore monster back pointer */
            for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                obj->ocarry = mtmp;
            }
        }
        if (mtmp->mw) {
            struct obj *obj;

            for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                if (obj->owornmask & W_WEP) {
                    break;
                }
            }
            if (obj) {
                mtmp->mw = obj;
            } else {
                MON_NOWEP(mtmp);
                impossible("bad monster weapon restore");
            }
        }

        if (mtmp->isshk) {
            restshk(mtmp, ghostly);
        }
        if (mtmp->ispriest) {
            restpriest(mtmp, ghostly);
        }

        if (!ghostly) {
            if (mtmp->m_id == polearm.m_id) {
                polearm.hitmon = mtmp;
            }
        }
        mtmp2 = mtmp;
    }
    if (first && mtmp2->nmon) {
        impossible("Restmonchn: error reading monchn.");
        mtmp2->nmon = 0;
    }
    return first;
}

static struct fruit *
loadfruitchn(NHFILE *nhfp)
{
    struct fruit *flist, *fnext;

    flist = 0;
    for (;;) {
        fnext = newfruit();
        if (nhfp->structlevel) {
            Mread(nhfp->fd, fnext, sizeof *fnext);
        }
        if (fnext->fid != 0) {
            fnext->nextf = flist;
            flist = fnext;
        } else {
            break;
        }
    }
    dealloc_fruit(fnext);

    return flist;
}

static void
freefruitchn(struct fruit *flist)
{
    struct fruit *fnext;

    while (flist) {
        fnext = flist->nextf;
        dealloc_fruit(flist);
        flist = fnext;
    }
}

static void
ghostfruit(struct obj *otmp)
{
    struct fruit *oldf;

    for (oldf = oldfruit; oldf; oldf = oldf->nextf) {
        if (oldf->fid == otmp->spe) {
            break;
        }
    }

    if (!oldf) {
        impossible("no old fruit?");
    } else {
        otmp->spe = fruitadd(oldf->fname);
    }
}

static boolean
restgamestate(NHFILE *nhfp)
{
    struct flag newgameflags;
    /* discover is actually flags.explore */
    boolean remember_discover = discover;
    struct obj *otmp;
    unsigned long uid = 0;
    boolean defer_perm_invent;

    if (nhfp->structlevel) {
        Mread(nhfp->fd, &uid, sizeof uid);
    }
    if (uid != getuid()) {      /* strange ... */
        /* for wizard mode, issue a reminder; for others, treat it
           as an attempt to cheat and refuse to restore this file */
        pline("Saved game was not yours.");
#ifdef WIZARD
        if (!wizard) {
#endif
        return FALSE;
        }
    }

    /* we want to be able to revert to command line/environment/config
       file option values instead of keeping old save file option values
       if partial restore fails and we resort to starting a new game */
    newgameflags = flags;
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &flags, sizeof flags);
    }
    /* avoid keeping permanent inventory window up to date during restore
       (setworn() calls update_inventory); attempting to include the cost
       of unpaid items before shopkeeper's bill is available is a no-no;
       named fruit names aren't accessible yet either
       [3.6.2: moved perm_invent from flags to iflags to keep it out of
       save files; retaining the override here is simpler than trying to
       to figure out where it really belongs now] */
    defer_perm_invent = iflags.perm_invent;
    iflags.perm_invent = FALSE;

    flags.bypasses = 0; /* never use the saved value of bypasses */
    if (remember_discover) {
        discover = remember_discover;
    }

    role_init();    /* Reset the initial role, race, gender, and alignment */
#ifdef AMII_GRAPHICS
    amii_setpens(amii_numcolors);   /* use colors from save file */
#endif
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &u, sizeof u);
    }

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &urealtime.realtime, sizeof urealtime.realtime);
    }
#endif

    init_uasmon();
#ifdef CLIPPING
    cliparound(u.ux, u.uy);
#endif
    if (u.uhp <= 0 && (!Upolyd || u.mh <= 0)) {
        u.ux = u.uy = 0;    /* affects pline() [hence You()] */
        You("were not healthy enough to survive restoration.");
        /* wiz1_level.dlevel is used by mklev.c to see if lots of stuff is
         * uninitialized, so we only have to set it and not the other stuff.
         */
        wiz1_level.dlevel = 0;
        u.uz.dnum = 0;
        u.uz.dlevel = 1;
        /* revert to pre-restore option settings */
        iflags.perm_invent = defer_perm_invent;
        youmonst = zeromonst;
        return FALSE;
    }
    /* in case hangup save occurred in midst of level change */
    assign_level(&u.uz0, &u.uz);

    /* this stuff comes after potential aborted restore attempts */
    restore_timers(nhfp, RANGE_GLOBAL, 0L);
    restore_light_sources(nhfp);

    invent = restobjchn(nhfp, FALSE);

    migrating_objs = restobjchn(nhfp, FALSE);
    migrating_mons = restmonchn(nhfp);

    if (nhfp->structlevel) {
        Mread(nhfp->fd, &mvitals[0], sizeof mvitals);
    }

    /*
     * There are some things after this that can have unintended display
     * side-effects too early in the game.
     * Disable see_monsters() here, re-enable it at the top of moveloop()
     */
    defer_see_monsters = TRUE;

    /* this comes after inventory has been loaded */
    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp->owornmask) {
            setworn(otmp, otmp->owornmask);
        }
    }

    /* reset weapon so that player will get a reminder about "bashing"
       during next fight when bare-handed or wielding an unconventional
       item; for pick-axe, we aren't able to distinguish between having
       applied or wielded it, so be conservative and assume the former */
    otmp = uwep;   /* `uwep' usually init'd by setworn() in loop above */
    uwep = 0;      /* clear it and have setuwep() reinit */
    setuwep(otmp); /* (don't need any null check here) */
    if (!uwep || uwep->otyp == PICK_AXE || uwep->otyp == GRAPPLING_HOOK) {
        unweapon = TRUE;
    }

    restore_dungeon(nhfp);
    restlevchn(nhfp);
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &moves, sizeof moves);
        /* hero_seq isn't saved and restored because it can be recalculated */
        hero_seq = moves << 3; /* normally handled in moveloop() */
        Mread(nhfp->fd, &monstermoves, sizeof monstermoves);
        Mread(nhfp->fd, &game_loop_counter, sizeof game_loop_counter);
        Mread(nhfp->fd, &quest_status, sizeof quest_status);
        Mread(nhfp->fd, spl_book, sizeof(struct spell) * (MAXSPELL + 1));
    }
    restore_artifacts(nhfp);
    restore_oracles(nhfp);
    if (nhfp->structlevel) {
        mread(nhfp->fd, (genericptr_t) pl_tutorial, sizeof pl_tutorial);
        Mread(nhfp->fd, pl_character, sizeof pl_character);
        Mread(nhfp->fd, pl_fruit, sizeof pl_fruit);
        Mread(nhfp->fd, &current_fruit, sizeof current_fruit);
    }
    freefruitchn(ffruit);   /* clean up fruit(s) made by initoptions() */
    ffruit = loadfruitchn(nhfp);

    restnames(nhfp);
    restore_msghistory(nhfp);
    restore_gamelog(nhfp);
    restore_waterlevel(nhfp);

#ifdef RECORD_ACHIEVE
    Mread(nhfp->fd, &achieve, sizeof achieve);
#endif

    /* must come after all mons & objs are restored */
    relink_timers(FALSE);
    relink_light_sources(FALSE);
#ifdef WHEREIS_FILE
    touch_whereis();
#endif
    return TRUE;
}

/* update game state pointers to those valid for the current level (so we
 * don't dereference a wild u.ustuck when saving the game state, for instance)
 */
static void
restlevelstate(void)
{
    /*
     * Note: restoring steed and engulfer/holder/holdee is now handled
     * in getlev() and there's nothing left for restlevelstate() to do.
     */
    return;
}

/*ARGSUSED*/
static int
restlevelfile(xint8 ltmp)
{
    char whynot[BUFSZ];
    NHFILE *nhfp = (NHFILE *) 0;

    nhfp = create_levelfile(ltmp, whynot);
    if (!nhfp) {
        /* BUG: should suppress any attempt to write a panic
           save file if file creation is now failing... */
        panic("restlevelfile: %s", whynot);
    }
    bufon(nhfp->fd);
    nhfp->mode = WRITING | FREEING;
    savelev(nhfp, ltmp);
    close_nhfile(nhfp);
    return 2;
}

int
dorecover(NHFILE *nhfp)
{
    xint8 ltmp = 0;
    int rtmp;
    struct obj *otmp;

    /* suppress map display if some part of the code tries to update that */
    program_state.restoring = REST_GSTATE;

    get_plname_from_file(nhfp, plname);
    getlev(nhfp, 0, (xint8) 0);
    if (!restgamestate(nhfp)) {
        NHFILE tnhfp;

        display_nhwindow(WIN_MESSAGE, TRUE);
        zero_nhfile(&tnhfp);
        tnhfp.mode = FREEING;
        tnhfp.fd = -1;
        savelev(&tnhfp, 0); /* discard current level */
        /* no need for close_nhfile(&tnhfp), which
           is not really affiliated with an open file */
        close_nhfile(nhfp);
        (void) delete_savefile();
        u.usteed_mid = u.ustuck_mid = 0;
        program_state.restoring = 0;
        return 0;
    }
    restlevelstate();
#ifdef INSURANCE
    savestateinlock();
#endif
    rtmp = restlevelfile(ledger_no(&u.uz));
    if (rtmp < 2) {
        return rtmp;  /* dorecover called recursively */
    }

    program_state.restoring = REST_LEVELS;

    /* these pointers won't be valid while we're processing the
     * other levels, but they'll be reset again by restlevelstate()
     * afterwards, and in the meantime at least u.usteed may mislead
     * place_monster() on other levels
     */
    u.ustuck = (struct monst *)0;
    u.usteed = (struct monst *)0;

#ifdef MICRO
# ifdef AMII_GRAPHICS
    {
        extern struct window_procs amii_procs;
        if (windowprocs.win_init_nhwindows== amii_procs.win_init_nhwindows) {
            extern winid WIN_BASE;
            clear_nhwindow(WIN_BASE); /* hack until there's a hook for this */
        }
    }
# else
    clear_nhwindow(WIN_MAP);
# endif
    clear_nhwindow(WIN_MESSAGE);
    You("return to level %d in %s%s.",
        depth(&u.uz), dungeons[u.uz.dnum].dname,
        flags.debug ? " while in debug mode" :
        flags.explore ? " while in explore mode" : "");
    curs(WIN_MAP, 1, 1);
    dotcnt = 0;
    dotrow = 2;
    if (!WINDOWPORT(X11)) {
        putstr(WIN_MAP, 0, "Restoring:");
    }
#endif
    while (1) {
        if (nhfp->structlevel) {
#ifdef ZEROCOMP
            if (read(nhfp->fd, (genericptr_t) &ltmp, sizeof ltmp) < 0) {
#else
            if (read(nhfp->fd, (genericptr_t) &ltmp, sizeof ltmp) != sizeof ltmp) {
#endif
                break;
            }
        }
        getlev(nhfp, 0, ltmp);
#ifdef MICRO
        curs(WIN_MAP, 1+dotcnt++, dotrow);
        if (dotcnt >= (COLNO - 1)) {
            dotrow++;
            dotcnt = 0;
        }
        if (strncmpi("X11", windowprocs.name, 3)) {
            putstr(WIN_MAP, 0, ".");
        }
        mark_synch();
#endif
        rtmp = restlevelfile(ltmp);
        if (rtmp < 2) {
            return rtmp;  /* dorecover called recursively */
        }
    }

    rewind_nhfile(nhfp); /* return to beginning of file */
    (void) validate(nhfp, (char *) 0, FALSE);
    get_plname_from_file(nhfp, plname);

    getlev(nhfp, 0, (xint8) 0);
    close_nhfile(nhfp);
    restlevelstate();
    program_state.something_worth_saving = 1; /* useful data now exists */

    if (!wizard && !discover) {
        (void) delete_savefile();
    }
#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) {
        assign_rogue_graphics(TRUE);
    }
#endif
    if (Is_moria_level(&u.uz)) {
        assign_moria_graphics(TRUE);
    }
#ifdef USE_TILES
    substitute_tiles(&u.uz);
#endif
    restlevelstate();
    max_rank_sz(); /* to recompute mrank_sz (botl.c) */

    /* this comes after inventory has been loaded */
    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp->owornmask) {
            setworn(otmp, otmp->owornmask);
        }
    }

    /* reset weapon so that player will get a reminder about "bashing"
       during next fight when bare-handed or wielding an unconventional
       item; for pick-axe, we aren't able to distinguish between having
       applied or wielded it, so be conservative and assume the former */
    otmp = uwep;    /* `uwep' usually init'd by setworn() in loop above */
    uwep = 0;   /* clear it and have setuwep() reinit */
    setuwep(otmp);  /* (don't need any null check here) */
    if (!uwep || uwep->otyp == PICK_AXE || uwep->otyp == GRAPPLING_HOOK) {
        unweapon = TRUE;
    }

    /* take care of iron ball & chain */
    for (otmp = fobj; otmp; otmp = otmp->nobj) {
        if (otmp->owornmask) {
            setworn(otmp, otmp->owornmask);
        }
    }

    /* in_use processing must be after:
     *    + The inventory has been read so that freeinv() works.
     *    + The current level has been restored so billing information
     *  is available.
     */
    inven_inuse(FALSE);

    load_qtlist();  /* re-load the quest text info */
    reset_attribute_clock();
    /* Set up the vision internals, after levl[] data is loaded
       but before docrt(). */
    reglyph_darkroom();
    vision_reset();
    vision_full_recalc = 1; /* recompute vision (not saved) */

    run_timers();   /* expire all timers that have gone off while away */
    program_state.restoring = 0; /* affects bot() so clear before docrt() */

    u.usteed_mid = u.ustuck_mid = 0;
    docrt();
    clear_nhwindow(WIN_MESSAGE);
    program_state.something_worth_saving++; /* useful data now exists */

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
    /* Start the timer here (realtime has already been set) */
    urealtime.start_timing = current_epoch();
#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

    /* Success! */
    welcome(FALSE);
    return 1;
}

static void
rest_stairs(NHFILE *nhfp)
{
    int buflen = 0;
    stairway stway = { 0 };
    stairway *newst;

    stairway_free_all();
    while (1) {
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &buflen, sizeof buflen);
        }

        if (buflen == -1) {
            break;
        }

        if (nhfp->structlevel) {
            Mread(nhfp->fd, &stway, sizeof stway);
        }
        if (program_state.restoring != REST_GSTATE &&
             stway.tolev.dnum == u.uz.dnum) {
            /* stairway dlevel is relative, make it absolute */
            stway.tolev.dlevel += u.uz.dlevel;
        }
        stairway_add(stway.sx, stway.sy, stway.up, stway.isladder, &(stway.tolev));
        newst = stairway_at(stway.sx, stway.sy);
        if (newst) {
            newst->u_traversed = stway.u_traversed;
        }
    }
}

void
restcemetery(NHFILE *nhfp, struct cemetery **cemeteryaddr)
{
    struct cemetery *bonesinfo, **bonesaddr;
    int cflag = 0;

    if (nhfp->structlevel) {
        Mread(nhfp->fd, &cflag, sizeof cflag);
    }
    if (cflag == 0) {
        bonesaddr = cemeteryaddr;
        do {
            bonesinfo = (struct cemetery *) alloc(sizeof *bonesinfo);
            if (nhfp->structlevel) {
                Mread(nhfp->fd, bonesinfo, sizeof *bonesinfo);
            }
            *bonesaddr = bonesinfo;
            bonesaddr = &(*bonesaddr)->next;
        } while (*bonesaddr);
    } else {
        *cemeteryaddr = 0;
    }
}

/*ARGSUSED*/
static void
rest_levl(
    NHFILE *nhfp,
#ifdef RLECOMP
    boolean rlecomp
#else
    boolean rlecomp UNUSED
#endif
)
{
#ifdef RLECOMP
    short i, j;
    uchar len;
    struct rm r;

    if (rlecomp) {
        (void) memset((genericptr_t) &r, 0, sizeof(r));
        i = 0;
        j = 0;
        len = 0;
        while (i < ROWNO) {
            while (j < COLNO) {
                if (len > 0) {
                    levl[j][i] = r;
                    len -= 1;
                    j += 1;
                } else {
                    if (nhfp->structlevel) {
                        Mread(nhfp->fd, &len, sizeof len);
                        Mread(nhfp->fd, &r, sizeof r);
                    }
                }
            }
            j = 0;
            i += 1;
        }
        return;
    }
#endif /* RLECOMP */
    if (nhfp->structlevel) {
        Mread(nhfp->fd, levl, sizeof levl);
    }
}

void
trickery(char *reason)
{
    pline("Strange, this map is not as I remember it.");
    pline("Somebody is trying some trickery here...");
    pline("This game is void.");
    Strcpy(killer.name, reason ? reason : "");
    done(TRICKED);
}

void
getlev(NHFILE *nhfp, int pid, xint8 lev)
{
    struct trap *trap;
    struct monst *mtmp;
    long elapsed;
    branch *br;
    int hpid = 0;
    xint8 dlvl = 0;
    int x, y;
    boolean ghostly = (nhfp->ftype == NHF_BONESFILE);
#ifdef TOS
    short tlev;
#endif

    if (ghostly) {
        clear_id_mapping();
    }

#if defined(MSDOS) || defined(OS2)
    if (nhfp->structlevel) {
        setmode(nhfp->fd, O_BINARY);
    }
#endif
    /* Load the old fruit info.  We have to do it first, so the
     * information is available when restoring the objects.
     */
    if (ghostly) {
        oldfruit = loadfruitchn(nhfp);
    }

    /* First some sanity checks */
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &hpid, sizeof hpid);
    }
/* CHECK:  This may prevent restoration */
#ifdef TOS
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &tlev, sizeof tlev);
    }
    dlvl = tlev & 0x00ff;
#else
    if (nhfp->structlevel) {
        Mread(nhfp->fd, &dlvl, sizeof dlvl);
    }
#endif
    if ((pid && pid != hpid) || (lev && dlvl != lev)) {
        char trickbuf[BUFSZ];

        if (pid && pid != hpid) {
            Sprintf(trickbuf, "PID (%d) doesn't match saved PID (%d)!",
                    hpid, pid);
        } else {
            Sprintf(trickbuf, "This is level %d, not %d!", dlvl, lev);
        }
#ifdef WIZARD
        if (wizard) {
            pline("%s", trickbuf);
        }
#endif
        trickery(trickbuf);
    }

    restcemetery(nhfp, &level.bonesinfo);
    rest_levl(nhfp, (boolean) ((sfrestinfo.sfi1 & SFI1_RLECOMP) == SFI1_RLECOMP));

    if (nhfp->structlevel) {
        //Mread(nhfp->fd, lastseentyp, sizeof lastseentyp);
        Mread(nhfp->fd, &omoves, sizeof omoves);
    }
    elapsed = moves - omoves;

    if (nhfp->structlevel) {
        rest_stairs(nhfp);
        Mread(nhfp->fd, &updest, sizeof updest);
        Mread(nhfp->fd, &dndest, sizeof dndest);
        Mread(nhfp->fd, &level.flags, sizeof level.flags);
        Mread(nhfp->fd, doors, sizeof doors);
    }
    rest_rooms(nhfp); /* No joke :-) */
    if (nroom) {
        doorindex = rooms[nroom - 1].fdoor + rooms[nroom - 1].doorct;
    } else {
        doorindex = 0;
    }

    restore_timers(nhfp, RANGE_LEVEL, elapsed);
    restore_light_sources(nhfp);
    fmon = restmonchn(nhfp);

    rest_worm(nhfp); /* restore worm information */
    ftrap = 0;
    for (;;) {
        trap = newtrap();
        if (nhfp->structlevel) {
            Mread(nhfp->fd, trap, sizeof *trap);
        }
        if (trap->tx != 0) {
            trap->ntrap = ftrap;
            ftrap = trap;
        } else {
            break;
        }
    }
    dealloc_trap(trap);

    fobj = restobjchn(nhfp, FALSE);
    find_lev_obj();
    /* restobjchn()'s `frozen' argument probably ought to be a callback
       routine so that we can check for objects being buried under ice */
    level.buriedobjlist = restobjchn(nhfp, FALSE);
    billobjs = restobjchn(nhfp, FALSE);
    level.mon_gen = rest_mongen_override(nhfp);
    level.sounds = rest_lvl_sounds(nhfp);
    rest_engravings(nhfp);

    /* reset level.monsters for new level */
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            level.monsters[x][y] = (struct monst *) 0;
        }
    }
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (mtmp->isshk) {
            set_residency(mtmp, FALSE);
        }
        if (mtmp->m_id == u.usteed_mid) {
            /* steed is kept on fmon list but off the map */
            u.usteed = mtmp;
            u.usteed_mid = 0;
        } else {
            if (mtmp->m_id == u.ustuck_mid) {
                set_ustuck(mtmp);
                u.ustuck_mid = 0;
            }
            place_monster(mtmp, mtmp->mx, mtmp->my);
            if (mtmp->wormno) {
                place_wsegs(mtmp, NULL);
            }
            if (hides_under(mtmp->data) && mtmp->mundetected) {
                (void) hideunder(mtmp);
            }
        }

        /* regenerate monsters while on another level */
        if (!u.uz.dlevel) {
            continue;
        }
        if (ghostly) {
            /* reset peaceful/malign relative to new character;
               shopkeepers will reset based on name */
            if (!mtmp->isshk)
                mtmp->mpeaceful = (is_unicorn(mtmp->data) &&
                                   (sgn(u.ualign.type) == sgn(mtmp->data->maligntyp))) ? 1 :
                    peace_minded(mtmp->data);
            set_malign(mtmp);
        } else if (elapsed > 0L) {
            mon_catchup_elapsed_time(mtmp, elapsed);
        }
        /* update shape-changers in case protection against
           them is different now than when the level was saved */
        restore_cham(mtmp);
        /* give hiders a chance to hide before their next move */
        if (ghostly || (elapsed > 0 && elapsed > (long) rnd(10))) {
            hide_monst(mtmp);
        }
    }

    restdamage(nhfp);
    rest_regions(nhfp);

    if (ghostly) {
        stairway *stway = stairs;
        while (stway) {
            if (!stway->isladder && !stway->up &&
                 stway->tolev.dnum == u.uz.dnum) {
                break;
            }
            stway = stway->next;
        }
        /* Now get rid of all the temp fruits... */
        freefruitchn(oldfruit),  oldfruit = 0;

        if (lev > ledger_no(&medusa_level) &&
             lev < ledger_no(&stronghold_level) && !stway) {
            coord cc;
            d_level dest;

            dest.dnum = u.uz.dnum;
            dest.dlevel = u.uz.dlevel + 1;

            mazexy(&cc);
            stairway_add(cc.x, cc.y, FALSE, FALSE, &dest);
            levl[cc.x][cc.y].typ = STAIRS;
        }

        br = Is_branchlev(&u.uz);
        if (br && u.uz.dlevel == 1) {
            d_level ltmp;

            if (on_level(&u.uz, &br->end1)) {
                assign_level(&ltmp, &br->end2);
            } else {
                assign_level(&ltmp, &br->end1);
            }

            switch (br->type) {
            case BR_STAIR:
            case BR_NO_END1:
            case BR_NO_END2: /* OK to assign to sstairs if it's not used */
                stway = stairs;
                while (stway) {
                    if (stway->tolev.dnum != u.uz.dnum) {
                        break;
                    }
                    stway = stway->next;
                }
                if (stway) {
                    assign_level(&(stway->tolev), &ltmp);
                }
                break;
            case BR_PORTAL: /* max of 1 portal per level */
            {
                struct trap *ttmp;
                for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
                    if (ttmp->ttyp == MAGIC_PORTAL) {
                        break;
                    }
                }
                if (!ttmp) {
                    panic("getlev: need portal but none found");
                }
                assign_level(&ttmp->dst, &ltmp);
            }
            break;
            }
        } else if (!br) {
            /* Remove any dangling portals. */
            struct trap *ttmp;
            for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
                if (ttmp->ttyp == MAGIC_PORTAL) {
                    deltrap(ttmp);
                    break; /* max of 1 portal/level */
                }
            }
        }
    }

    /* must come after all mons & objs are restored */
    relink_timers(ghostly);
    relink_light_sources(ghostly);
    reset_oattached_mids(ghostly);
#ifdef DUNGEON_GROWTH
    if (!ghostly) {
        catchup_dgn_growths((monstermoves - omoves) / 5);
    }
#endif
    if (ghostly) {
        clear_id_mapping();
    }
}

void
get_plname_from_file(NHFILE *nhfp, char *plbuf)
{
    int pltmpsiz = 0;

    if (nhfp->structlevel) {
        (void) read(nhfp->fd, (genericptr_t) &pltmpsiz, sizeof(pltmpsiz));
        (void) read(nhfp->fd, (genericptr_t) plbuf, pltmpsiz);
    }
    return;
}

static void
restore_gamelog(NHFILE *nhfp)
{
    nhUse(nhfp);
#ifdef NEXT_VERSION
    int slen = 0;
    char msg[BUFSZ*2];
    struct gamelog_line tmp;

    while (1) {
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &slen, sizeof slen);
        }
        if (slen == -1) {
            break;
        }
        if (slen > ((BUFSZ*2) - 1)) {
            panic("restore_gamelog: msg too big (%d)", slen);
        }
        if (nhfp->structlevel) {
            Mread(nhfp->fd, msg, slen);
            Mread(nhfp->fd, &tmp, sizeof tmp);
            msg[slen] = '\0';
            gamelog_add(tmp.flags, tmp.turn, msg);
        }
    }
#endif
}

static void
restore_msghistory(NHFILE *nhfp)
{
    nhUse(nhfp);
#ifdef NEXT_VERSION
    int msgsize = 0, msgcount = 0;
    char msg[BUFSZ];

    while (1) {
        if (nhfp->structlevel) {
            Mread(nhfp->fd, &msgsize, sizeof msgsize);
        }
        if (msgsize == -1) {
            break;
        }
        if (msgsize > BUFSZ - 1) {
            panic("restore_msghistory: msg too big (%d)", msgsize);
        }
        if (nhfp->structlevel) {
            Mread(nhfp->fd, msg, msgsize);
        }
        msg[msgsize] = '\0';
        putmsghistory(msg, TRUE);
        ++msgcount;
    }
    if (msgcount) {
        putmsghistory((char *) 0, TRUE);
    }
    debug_pline("Read %d messages from savefile.", msgcount);
#endif
}

/* Clear all structures for object and monster ID mapping. */
static void
clear_id_mapping(void)
{
    struct bucket *curr;

    while ((curr = id_map) != 0) {
        id_map = curr->next;
        free((genericptr_t) curr);
    }
    n_ids_mapped = 0;
}

/* Add a mapping to the ID map. */
static void
add_id_mapping(unsigned int gid, unsigned int nid)
{
    int idx;

    idx = n_ids_mapped % N_PER_BUCKET;
    /* idx is zero on first time through, as well as when a new bucket is */
    /* needed */
    if (idx == 0) {
        struct bucket *gnu = (struct bucket *) alloc(sizeof(struct bucket));
        gnu->next = id_map;
        id_map = gnu;
    }

    id_map->map[idx].gid = gid;
    id_map->map[idx].nid = nid;
    n_ids_mapped++;
}

/*
 * Global routine to look up a mapping.  If found, return TRUE and fill
 * in the new ID value.  Otherwise, return false and return -1 in the new
 * ID.
 */
boolean
lookup_id_mapping(unsigned int gid, unsigned int *nidp)
{
    int i;
    struct bucket *curr;

    if (n_ids_mapped) {
        for (curr = id_map; curr; curr = curr->next) {
            /* first bucket might not be totally full */
            if (curr == id_map) {
                i = n_ids_mapped % N_PER_BUCKET;
                if (i == 0) {
                    i = N_PER_BUCKET;
                }
            } else {
                i = N_PER_BUCKET;
            }

            while (--i >= 0) {
                if (gid == curr->map[i].gid) {
                    *nidp = curr->map[i].nid;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

static void
reset_oattached_mids(boolean ghostly)
{
    struct obj *otmp;
    unsigned oldid, nid;
    for (otmp = fobj; otmp; otmp = otmp->nobj) {
        if (ghostly && has_omonst(otmp)) {
            struct monst *mtmp = OMONST(otmp);

            mtmp->m_id = 0;
            mtmp->mpeaceful = mtmp->mtame = 0; /* pet's owner died! */
        }
        if (ghostly && has_omid(otmp)) {
            (void) memcpy((genericptr_t) &oldid, (genericptr_t) OMID(otmp),
                          sizeof(oldid));
            if (lookup_id_mapping(oldid, &nid)) {
                (void) memcpy((genericptr_t) OMID(otmp), (genericptr_t) &nid,
                              sizeof(nid));
            } else {
                free_omid(otmp);
            }
        }
    }
}

int
validate(NHFILE *nhfp, const char *name, boolean without_waitsynch_perfile)
{
    readLenType rlen = 0;
    struct savefile_info sfi;
    unsigned long utdflags = 0L;
    boolean verbose = name ? TRUE : FALSE, reslt = FALSE;

    if (nhfp->structlevel) {
        utdflags |= UTD_CHECKSIZES;
    }
    if (without_waitsynch_perfile) {
        utdflags |= UTD_WITHOUT_WAITSYNCH_PERFILE;
    }
    if (!(reslt = uptodate(nhfp, name, utdflags))) {
        return 1;
    }

    if ((nhfp->mode & WRITING) == 0) {
        if (nhfp->structlevel) {
            rlen = (readLenType) read(nhfp->fd, (genericptr_t) &sfi, sizeof sfi);
        }
    } else {
        if (nhfp->structlevel) {
            rlen = (readLenType) read(nhfp->fd, (genericptr_t) &sfi, sizeof sfi);
        }
        minit(); /* ZEROCOMP */
        if (rlen == 0) {
            if (verbose) {
                pline("File \"%s\" is empty during save file feature check?", name);
                wait_synch();
            }
            return -1;
        }
    }

    return 0;
}

#undef Mread

/*restore.c*/
