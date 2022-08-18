/*  SCCS Id: @(#)restore.c  3.4 2003/09/06  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "tcap.h" /* for TERMLIB and ASCIIGRAPH */

#if defined(MICRO)
extern int dotcnt;  /* shared with save */
extern int dotrow;  /* shared with save */
#endif

#ifdef USE_TILES
extern void substitute_tiles(d_level *);       /* from tile.c */
#endif

#ifdef ZEROCOMP
static int mgetc();
#endif
static void find_lev_obj(void);
static void restlevchn(int);
static void restdamage(int, boolean);
static void restobj(int, struct obj *);
static struct obj *restobjchn(int, boolean, boolean);
static void restmon(int, struct monst *);
static struct monst *restmonchn(int, boolean);
static struct fruit *loadfruitchn(int);
static void freefruitchn(struct fruit *);
static void ghostfruit(struct obj *);
static boolean restgamestate(int, unsigned int *, unsigned int *);
static void restlevelstate(unsigned int, unsigned int);
static int restlevelfile(int, xint8);
static void reset_oattached_mids(boolean);

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

boolean restoring = FALSE;
static NEARDATA struct fruit *oldfruit;
static NEARDATA long omoves;

#define Is_IceBox(o) ((o)->otyp == ICE_BOX ? TRUE : FALSE)

/* Recalculate level.objects[x][y], since this info was not saved. */
static void
find_lev_obj(void)
{
    struct obj *fobjtmp = (struct obj *)0;
    struct obj *otmp;
    int x, y;

    for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++)
            level.objects[x][y] = (struct obj *)0;

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
            if (!quietly) pline("Finishing off %s...", xname(otmp));
            useup(otmp);
        }
    }
}

static void
restlevchn(int fd)
{
    int cnt;
    s_level *tmplev, *x;

    sp_levchn = (s_level *) 0;
    mread(fd, (genericptr_t) &cnt, sizeof(int));
    for(; cnt > 0; cnt--) {

        tmplev = (s_level *)alloc(sizeof(s_level));
        mread(fd, (genericptr_t) tmplev, sizeof(s_level));
        if(!sp_levchn) sp_levchn = tmplev;
        else {

            for(x = sp_levchn; x->next; x = x->next);
            x->next = tmplev;
        }
        tmplev->next = (s_level *)0;
    }
}

static void
restdamage(int fd, boolean ghostly)
{
    int counter;
    struct damage *tmp_dam;

    mread(fd, (genericptr_t) &counter, sizeof(counter));
    if (!counter)
        return;
    tmp_dam = (struct damage *)alloc(sizeof(struct damage));
    while (--counter >= 0) {
        char damaged_shops[5], *shp = (char *)0;

        mread(fd, (genericptr_t) tmp_dam, sizeof(*tmp_dam));
        if (ghostly)
            tmp_dam->when += (monstermoves - omoves);
        Strcpy(damaged_shops,
               in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
        if (u.uz.dlevel) {
            /* when restoring, there are two passes over the current
             * level.  the first time, u.uz isn't set, so neither is
             * shop_keeper().  just wait and process the damage on
             * the second pass.
             */
            for (shp = damaged_shops; *shp; shp++) {
                struct monst *shkp = shop_keeper(*shp);

                if (shkp && inhishop(shkp) &&
                    repair_damage(shkp, tmp_dam, TRUE))
                    break;
            }
        }
        if (!shp || !*shp) {
            tmp_dam->next = level.damagelist;
            level.damagelist = tmp_dam;
            tmp_dam = (struct damage *)alloc(sizeof(*tmp_dam));
        }
    }
    free((genericptr_t)tmp_dam);
}

struct lvl_sounds *
rest_lvl_sounds(int fd)
{
    int marker;
    struct lvl_sounds *or = NULL;
    mread(fd, (genericptr_t) &marker, sizeof(marker));
    if (marker) {
        or = (struct lvl_sounds *)alloc(sizeof(struct lvl_sounds));
        mread(fd, (genericptr_t) or, sizeof(*or));
        or->sounds = NULL;
        if (or->n_sounds) {
            int i;
            int len;
            or->sounds = (struct lvl_sound_bite *)alloc(sizeof(struct lvl_sound_bite)*or->n_sounds);
            for (i = 0; i < or->n_sounds; i++) {
                mread(fd, (genericptr_t)&(or->sounds[i].flags), sizeof(or->sounds[i].flags));
                mread(fd, (genericptr_t)&len, sizeof(len));
                or->sounds[i].msg = (char *)alloc(len);
                mread(fd, (genericptr_t)or->sounds[i].msg, len);
            }
        }
    }
    return or;
}

struct mon_gen_override *
rest_mongen_override(int fd)
{
    int marker;
    struct mon_gen_override *or = NULL;
    struct mon_gen_tuple *mt = NULL;
    int next;

    mread(fd, (genericptr_t) &marker, sizeof(marker));
    if (marker) {
        or = (struct mon_gen_override *)alloc(sizeof(struct mon_gen_override));
        mread(fd, (genericptr_t) or, sizeof(*or));
        if (or->gen_chances) {
            or->gen_chances = NULL;
            do {
                mt = (struct mon_gen_tuple *)alloc(sizeof(struct mon_gen_tuple));
                mread(fd, (genericptr_t) mt, sizeof(*mt));
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
restobj(int fd, struct obj *otmp)
{
    int buflen;

    mread(fd, (genericptr_t) otmp, sizeof(struct obj));

    /* next object pointers are invalid; otmp->cobj needs to be left
       as is--being non-null is key to restoring container contents */
    otmp->nobj = otmp->nexthere = (struct obj *) 0;
    /* non-null oextra needs to be reconstructed */
    if (otmp->oextra) {
        otmp->oextra = newoextra();

        /* oname - object's name */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) { /* includes terminating '\0' */
            new_oname(otmp, buflen);
            mread(fd, (genericptr_t) ONAME(otmp), buflen);
        }
        /* omonst - corpse or statue might retain full monster details */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newomonst(otmp);
            /* this is actually a monst struct, so we
               can just defer to restmon() here */
            restmon(fd, OMONST(otmp));
        }
        /* omid - monster id number, connecting corpse to ghost */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newomid(otmp);
            mread(fd, (genericptr_t) OMID(otmp), buflen);
        }
        /* olong - temporary gold */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newolong(otmp);
            mread(fd, (genericptr_t) OLONG(otmp), buflen);
        }
        /* omailcmd - feedback mechanism for scroll of mail */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            char *omailcmd = (char *) alloc(buflen);

            mread(fd, (genericptr_t) omailcmd, buflen);
            new_omailcmd(otmp, omailcmd);
            free((genericptr_t) omailcmd);
        }
    }
}

static struct obj *
restobjchn(int fd, boolean ghostly, boolean frozen)
{
    struct obj *otmp, *otmp2 = 0;
    struct obj *first = (struct obj *) 0;
    int buflen;

    while (1) {
        mread(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen == -1)
            break;

        otmp = newobj();
        restobj(fd, otmp);
        if (!first)
            first = otmp;
        else
            otmp2->nobj = otmp;

        if (ghostly) {
            unsigned nid = flags.ident++;
            add_id_mapping(otmp->o_id, nid);
            otmp->o_id = nid;
        }
        if (ghostly && otmp->otyp == SLIME_MOLD)
            ghostfruit(otmp);
        /* Ghost levels get object age shifted from old player's clock
         * to new player's clock.  Assumption: new player arrived
         * immediately after old player died.
         */
        if (ghostly && !frozen && !age_is_relative(otmp))
            otmp->age = monstermoves - omoves + otmp->age;

        /* get contents of a container or statue */
        if (Has_contents(otmp)) {
            struct obj *otmp3;

            otmp->cobj = restobjchn(fd, ghostly, Is_IceBox(otmp));
            /* restore container back pointers */
            for (otmp3 = otmp->cobj; otmp3; otmp3 = otmp3->nobj)
                otmp3->ocontainer = otmp;
        } else if (SchroedingersBox(otmp)) {
            struct obj *catcorpse;

            /*
             * TODO:  Remove this after 3.6.x save compatibility is dropped.
             *
             * As of 3.6.2, SchroedingersBox() always has a cat corpse in it.
             * For 3.6.[01], it was empty and its weight was falsified
             * to have the value it would have had if there was one inside.
             * Put a non-rotting cat corpse in this box to convert to 3.6.2.
             *
             * [Note: after this fix up, future save/restore of this object
             * will take the Has_contents() code path above.]
             */
            if ((catcorpse = mksobj(CORPSE, TRUE, FALSE)) != 0) {
                otmp->spe = 1; /* flag for special SchroedingersBox */
                set_corpsenm(catcorpse, PM_HOUSECAT);
                (void) stop_timer(ROT_CORPSE, obj_to_any(catcorpse));
                add_to_container(otmp, catcorpse);
                otmp->owt = weight(otmp);
            }
        }
        if (otmp->bypass)
            otmp->bypass = 0;
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
restmon(int fd, struct monst *mtmp)
{
    int buflen;

    mread(fd, (genericptr_t) mtmp, sizeof(struct monst));

    /* next monster pointer is invalid */
    mtmp->nmon = (struct monst *) 0;
    /* non-null mextra needs to be reconstructed */
    if (mtmp->mextra) {
        mtmp->mextra = newmextra();

        /* mname - monster's name */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) { /* includes terminating '\0' */
            new_mgivenname(mtmp, buflen);
            mread(fd, (genericptr_t) MGIVENNAME(mtmp), buflen);
        }
        /* egd - vault guard */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newegd(mtmp);
            mread(fd, (genericptr_t) EGD(mtmp), sizeof(struct egd));
        }
        /* epri - temple priest */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newepri(mtmp);
            mread(fd, (genericptr_t) EPRI(mtmp), sizeof(struct epri));
        }
        /* eshk - shopkeeper */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            neweshk(mtmp);
            mread(fd, (genericptr_t) ESHK(mtmp), sizeof(struct eshk));
        }
        /* emin - minion */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newemin(mtmp);
            mread(fd, (genericptr_t) EMIN(mtmp), sizeof(struct emin));
        }
        /* edog - pet */
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen > 0) {
            newedog(mtmp);
            mread(fd, (genericptr_t) EDOG(mtmp), sizeof(struct edog));
        }
        /* mcorpsenm - obj->corpsenm for mimic posing as corpse or
           statue (inline int rather than pointer to something) */
        mread(fd, (genericptr_t) &MCORPSENM(mtmp), sizeof MCORPSENM(mtmp));
    } /* mextra */
}

static struct monst *
restmonchn(int fd, boolean ghostly)
{
    struct monst *mtmp, *mtmp2 = 0;
    struct monst *first = (struct monst *) 0;
    int offset, buflen;

    while (1) {
        mread(fd, (genericptr_t) &buflen, sizeof(buflen));
        if (buflen == -1)
            break;

        mtmp = newmonst();
        restmon(fd, mtmp);
        if (!first)
            first = mtmp;
        else
            mtmp2->nmon = mtmp;

        if (ghostly) {
            unsigned nid = flags.ident++;
            add_id_mapping(mtmp->m_id, nid);
            mtmp->m_id = nid;
        }
        offset = mtmp->mnum;
        mtmp->data = &mons[offset];
        if (ghostly) {
            int mndx = monsndx(mtmp->data);
            if (propagate(mndx, TRUE, ghostly) == 0) {
                /* cookie to trigger purge in getbones() */
                mtmp->mhpmax = DEFUNCT_MONSTER;
            }
        }
        if (mtmp->minvent) {
            struct obj *obj;
            mtmp->minvent = restobjchn(fd, ghostly, FALSE);
            /* restore monster back pointer */
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                obj->ocarry = mtmp;
        }
        if (mtmp->mw) {
            struct obj *obj;

            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                if (obj->owornmask & W_WEP)
                    break;
            if (obj)
                mtmp->mw = obj;
            else {
                MON_NOWEP(mtmp);
                impossible("bad monster weapon restore");
            }
        }

        if (mtmp->isshk)
            restshk(mtmp, ghostly);
        if (mtmp->ispriest)
            restpriest(mtmp, ghostly);

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
loadfruitchn(int fd)
{
    struct fruit *flist, *fnext;

    flist = 0;
    while (fnext = newfruit(),
           mread(fd, (genericptr_t)fnext, sizeof *fnext),
           fnext->fid != 0) {
        fnext->nextf = flist;
        flist = fnext;
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

    for (oldf = oldfruit; oldf; oldf = oldf->nextf)
        if (oldf->fid == otmp->spe) break;

    if (!oldf) impossible("no old fruit?");
    else otmp->spe = fruitadd(oldf->fname);
}

static boolean
restgamestate(int fd, unsigned int *stuckid, unsigned int *steedid)
{
    /* discover is actually flags.explore */
    boolean remember_discover = discover;
    struct obj *otmp;
    int uid;

    mread(fd, (genericptr_t) &uid, sizeof uid);
    if (uid != getuid()) {      /* strange ... */
        /* for wizard mode, issue a reminder; for others, treat it
           as an attempt to cheat and refuse to restore this file */
        pline("Saved game was not yours.");
#ifdef WIZARD
        if (!wizard)
#endif
        return FALSE;
    }

    mread(fd, (genericptr_t) &flags, sizeof(struct flag));
    flags.bypasses = 0; /* never use the saved value of bypasses */
    if (remember_discover) discover = remember_discover;

    role_init();    /* Reset the initial role, race, gender, and alignment */
#ifdef AMII_GRAPHICS
    amii_setpens(amii_numcolors);   /* use colors from save file */
#endif
    mread(fd, (genericptr_t) &u, sizeof(struct you));
    init_uasmon();
#ifdef CLIPPING
    cliparound(u.ux, u.uy);
#endif
    if(u.uhp <= 0 && (!Upolyd || u.mh <= 0)) {
        u.ux = u.uy = 0;    /* affects pline() [hence You()] */
        You("were not healthy enough to survive restoration.");
        /* wiz1_level.dlevel is used by mklev.c to see if lots of stuff is
         * uninitialized, so we only have to set it and not the other stuff.
         */
        wiz1_level.dlevel = 0;
        u.uz.dnum = 0;
        u.uz.dlevel = 1;
        return(FALSE);
    }

    /* this stuff comes after potential aborted restore attempts */
    restore_timers(fd, RANGE_GLOBAL, FALSE, 0L);
    restore_light_sources(fd);
    invent = restobjchn(fd, FALSE, FALSE);
    migrating_objs = restobjchn(fd, FALSE, FALSE);
    migrating_mons = restmonchn(fd, FALSE);
    mread(fd, (genericptr_t) mvitals, sizeof(mvitals));

    /*
     * There are some things after this that can have unintended display
     * side-effects too early in the game.
     * Disable see_monsters() here, re-enable it at the top of moveloop()
     */
    defer_see_monsters = TRUE;

    restore_dungeon(fd);
    restlevchn(fd);
    mread(fd, (genericptr_t) &moves, sizeof moves);
    mread(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
    mread(fd, (genericptr_t) &game_loop_counter, sizeof game_loop_counter);
    mread(fd, (genericptr_t) &quest_status, sizeof(struct q_score));
    mread(fd, (genericptr_t) spl_book,
          sizeof(struct spell) * (MAXSPELL + 1));
    restore_artifacts(fd);
    restore_oracles(fd);
    if (u.ustuck)
        mread(fd, (genericptr_t) stuckid, sizeof (*stuckid));

    if (u.usteed)
        mread(fd, (genericptr_t) steedid, sizeof (*steedid));

    mread(fd, (genericptr_t) pl_tutorial, sizeof pl_tutorial);
    mread(fd, (genericptr_t) pl_character, sizeof pl_character);

    mread(fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
    mread(fd, (genericptr_t) &current_fruit, sizeof current_fruit);
    freefruitchn(ffruit);   /* clean up fruit(s) made by initoptions() */
    ffruit = loadfruitchn(fd);

    restnames(fd);
    restore_waterlevel(fd);

#ifdef RECORD_ACHIEVE
    mread(fd, (genericptr_t) &achieve, sizeof achieve);
#endif
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
    mread(fd, &urealtime.realtime, sizeof urealtime.realtime);
#endif

    /* must come after all mons & objs are restored */
    relink_timers(FALSE);
    relink_light_sources(FALSE);
#ifdef WHEREIS_FILE
    touch_whereis();
#endif
    return(TRUE);
}

/* update game state pointers to those valid for the current level (so we
 * don't dereference a wild u.ustuck when saving the game state, for instance)
 */
static void
restlevelstate(unsigned int stuckid, unsigned int steedid)
{
    struct monst *mtmp;

    if (stuckid) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->m_id == stuckid) break;
        if (!mtmp) panic("Cannot find the monster ustuck.");
        u.ustuck = mtmp;
    }

    if (steedid) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->m_id == steedid) break;
        if (!mtmp) panic("Cannot find the monster usteed.");
        u.usteed = mtmp;
        remove_monster(mtmp->mx, mtmp->my);
    }
}

/*ARGSUSED*/
static int
restlevelfile(
    int fd UNUSED, /**< fd used in MFLOPPY only */
    xint8 ltmp
)
#if defined(macintosh) && (defined(__SC__) || defined(__MRC__))
# pragma unused(fd)
#endif
{
    int nfd;
    char whynot[BUFSZ];

    nfd = create_levelfile(ltmp, whynot);
    if (nfd < 0) {
        /* BUG: should suppress any attempt to write a panic
           save file if file creation is now failing... */
        panic("restlevelfile: %s", whynot);
    }
#ifdef MFLOPPY
    if (!savelev(nfd, ltmp, COUNT_SAVE)) {

        /* The savelev can't proceed because the size required
         * is greater than the available disk space.
         */
        pline("Not enough space on `%s' to restore your game.",
              levels);

        /* Remove levels and bones that may have been created.
         */
        (void) close(nfd);
# ifdef AMIGA
        clearlocks();
# else
        eraseall(levels, alllevels);
        eraseall(levels, allbones);

        /* Perhaps the person would like to play without a
         * RAMdisk.
         */
        if (ramdisk) {
            /* PlaywoRAMdisk may not return, but if it does
             * it is certain that ramdisk will be 0.
             */
            playwoRAMdisk();
            /* Rewind save file and try again */
            (void) lseek(fd, (off_t)0, 0);
            (void) uptodate(fd, (char *)0); /* skip version */
            return dorecover(fd);   /* 0 or 1 */
        } else {
# endif
        pline("Be seeing you...");
        nh_terminate(EXIT_SUCCESS);
# ifndef AMIGA
    }
# endif
    }
#endif
    bufon(nfd);
    savelev(nfd, ltmp, WRITE_SAVE | FREE_SAVE);
    bclose(nfd);
    return(2);
}

int
dorecover(int fd)
{
    unsigned int stuckid = 0, steedid = 0;  /* not a */
    xint8 ltmp;
    int rtmp;
    struct obj *otmp;

#ifdef STORE_PLNAME_IN_FILE
    mread(fd, (genericptr_t) plname, PL_NSIZ);
#endif

    restoring = TRUE;
    getlev(fd, 0, (xint8)0, FALSE);
    if (!restgamestate(fd, &stuckid, &steedid)) {
        display_nhwindow(WIN_MESSAGE, TRUE);
        savelev(-1, 0, FREE_SAVE);  /* discard current level */
        (void) close(fd);
        (void) delete_savefile();
        restoring = FALSE;
        return(0);
    }
    restlevelstate(stuckid, steedid);
#ifdef INSURANCE
    savestateinlock();
#endif
    rtmp = restlevelfile(fd, ledger_no(&u.uz));
    if (rtmp < 2) return(rtmp);  /* dorecover called recursively */

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
        if(windowprocs.win_init_nhwindows== amii_procs.win_init_nhwindows) {
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
    if (strncmpi("X11", windowprocs.name, 3))
        putstr(WIN_MAP, 0, "Restoring:");
#endif
    while(1) {
#ifdef ZEROCOMP
        if(mread(fd, (genericptr_t) &ltmp, sizeof ltmp) < 0)
#else
        if(read(fd, (genericptr_t) &ltmp, sizeof ltmp) != sizeof ltmp)
#endif
            break;
        getlev(fd, 0, ltmp, FALSE);
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
        rtmp = restlevelfile(fd, ltmp);
        if (rtmp < 2) return(rtmp);  /* dorecover called recursively */
    }

#ifdef BSD
    (void) lseek(fd, 0L, 0);
#else
    (void) lseek(fd, (off_t)0, 0);
#endif
    (void) uptodate(fd, (char *)0);     /* skip version info */
#ifdef STORE_PLNAME_IN_FILE
    mread(fd, (genericptr_t) plname, PL_NSIZ);
#endif
    getlev(fd, 0, (xint8)0, FALSE);
    (void) close(fd);

    if (!wizard && !discover)
        (void) delete_savefile();
#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) assign_rogue_graphics(TRUE);
#endif
    if (Is_moria_level(&u.uz)) { assign_moria_graphics(TRUE); }
#ifdef USE_TILES
    substitute_tiles(&u.uz);
#endif
    restlevelstate(stuckid, steedid);
#ifdef MFLOPPY
    gameDiskPrompt();
#endif
    max_rank_sz(); /* to recompute mrank_sz (botl.c) */

    /* this comes after inventory has been loaded */
    for(otmp = invent; otmp; otmp = otmp->nobj)
        if(otmp->owornmask)
            setworn(otmp, otmp->owornmask);

    /* reset weapon so that player will get a reminder about "bashing"
       during next fight when bare-handed or wielding an unconventional
       item; for pick-axe, we aren't able to distinguish between having
       applied or wielded it, so be conservative and assume the former */
    otmp = uwep;    /* `uwep' usually init'd by setworn() in loop above */
    uwep = 0;   /* clear it and have setuwep() reinit */
    setuwep(otmp);  /* (don't need any null check here) */
    if (!uwep || uwep->otyp == PICK_AXE || uwep->otyp == GRAPPLING_HOOK)
        unweapon = TRUE;

    /* take care of iron ball & chain */
    for(otmp = fobj; otmp; otmp = otmp->nobj)
        if(otmp->owornmask)
            setworn(otmp, otmp->owornmask);

    /* in_use processing must be after:
     *    + The inventory has been read so that freeinv() works.
     *    + The current level has been restored so billing information
     *  is available.
     */
    inven_inuse(FALSE);

    load_qtlist();  /* re-load the quest text info */
    reset_attribute_clock();
    /* Set up the vision internals, after levl[] data is loaded */
    /* but before docrt().                      */
    vision_reset();
    vision_full_recalc = 1; /* recompute vision (not saved) */

    run_timers();   /* expire all timers that have gone off while away */
    docrt();
    restoring = FALSE;
    clear_nhwindow(WIN_MESSAGE);
    program_state.something_worth_saving++; /* useful data now exists */

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
    /* Start the timer here (realtime has already been set) */
    urealtime.start_timing = current_epoch();
#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

    /* Success! */
    welcome(FALSE);
    return(1);
}

void
restcemetery(int fd, struct cemetery **cemeteryaddr)
{
    struct cemetery *bonesinfo, **bonesaddr;
    int flag;

    mread(fd, &flag, sizeof flag);
    if (flag == 0) {
        bonesaddr = cemeteryaddr;
        do {
            bonesinfo = (struct cemetery *) alloc(sizeof *bonesinfo);
            mread(fd, bonesinfo, sizeof *bonesinfo);
            *bonesaddr = bonesinfo;
            bonesaddr = &(*bonesaddr)->next;
        } while (*bonesaddr);
    } else {
        *cemeteryaddr = 0;
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
getlev(int fd, int pid, xint8 lev, boolean ghostly)
{
    struct trap *trap;
    struct monst *mtmp;
    branch *br;
    int hpid;
    xint8 dlvl;
    int x, y;
#ifdef TOS
    short tlev;
#endif

    if (ghostly)
        clear_id_mapping();

#if defined(MSDOS) || defined(OS2)
    setmode(fd, O_BINARY);
#endif
    /* Load the old fruit info.  We have to do it first, so the
     * information is available when restoring the objects.
     */
    if (ghostly) oldfruit = loadfruitchn(fd);

    /* First some sanity checks */
    mread(fd, (genericptr_t) &hpid, sizeof(hpid));
/* CHECK:  This may prevent restoration */
#ifdef TOS
    mread(fd, (genericptr_t) &tlev, sizeof(tlev));
    dlvl=tlev&0x00ff;
#else
    mread(fd, (genericptr_t) &dlvl, sizeof(dlvl));
#endif
    if ((pid && pid != hpid) || (lev && dlvl != lev)) {
        char trickbuf[BUFSZ];

        if (pid && pid != hpid)
            Sprintf(trickbuf, "PID (%d) doesn't match saved PID (%d)!",
                    hpid, pid);
        else
            Sprintf(trickbuf, "This is level %d, not %d!", dlvl, lev);
#ifdef WIZARD
        if (wizard) pline("%s", trickbuf);
#endif
        trickery(trickbuf);
    }

    restcemetery(fd, &level.bonesinfo);

#ifdef RLECOMP
    {
        short i, j;
        uchar len;
        struct rm r;

#if defined(MAC)
        /* Suppress warning about used before set */
        (void) memset((genericptr_t) &r, 0, sizeof(r));
#endif
        i = 0; j = 0; len = 0;
        while(i < ROWNO) {
            while(j < COLNO) {
                if(len > 0) {
                    levl[j][i] = r;
                    len -= 1;
                    j += 1;
                } else {
                    mread(fd, (genericptr_t)&len, sizeof(uchar));
                    mread(fd, (genericptr_t)&r, sizeof(struct rm));
                }
            }
            j = 0;
            i += 1;
        }
    }
#else
    mread(fd, (genericptr_t) levl, sizeof(levl));
#endif  /* RLECOMP */

    mread(fd, (genericptr_t)&omoves, sizeof(omoves));
    mread(fd, (genericptr_t)&upstair, sizeof(stairway));
    mread(fd, (genericptr_t)&dnstair, sizeof(stairway));
    mread(fd, (genericptr_t)&upladder, sizeof(stairway));
    mread(fd, (genericptr_t)&dnladder, sizeof(stairway));
    mread(fd, (genericptr_t)&sstairs, sizeof(stairway));
    mread(fd, (genericptr_t)&updest, sizeof(dest_area));
    mread(fd, (genericptr_t)&dndest, sizeof(dest_area));
    mread(fd, (genericptr_t)&level.flags, sizeof(level.flags));
    mread(fd, (genericptr_t)doors, sizeof(doors));
    rest_rooms(fd);     /* No joke :-) */
    if (nroom)
        doorindex = rooms[nroom - 1].fdoor + rooms[nroom - 1].doorct;
    else
        doorindex = 0;

    restore_timers(fd, RANGE_LEVEL, ghostly, monstermoves - omoves);
    restore_light_sources(fd);
    fmon = restmonchn(fd, ghostly);

    /* reset level.monsters for new level */
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            level.monsters[x][y] = (struct monst *) 0;
        }
    }

    /* regenerate animals while on another level */
    if (u.uz.dlevel) {
        struct monst *mtmp2;

        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon;
            if (ghostly) {
                /* reset peaceful/malign relative to new character */
                if(!mtmp->isshk)
                    /* shopkeepers will reset based on name */
                    mtmp->mpeaceful = peace_minded(mtmp->data);
                set_malign(mtmp);
            } else if (monstermoves > omoves)
                mon_catchup_elapsed_time(mtmp, monstermoves - omoves);

            /* update shape-changers in case protection against
               them is different now than when the level was saved */
            restore_cham(mtmp);
        }
    }

    rest_worm(fd);  /* restore worm information */
    ftrap = 0;
    while (trap = newtrap(),
           mread(fd, (genericptr_t)trap, sizeof(struct trap)),
           trap->tx != 0) { /* need "!= 0" to work around DICE 3.0 bug */
        trap->ntrap = ftrap;
        ftrap = trap;
    }
    dealloc_trap(trap);
    fobj = restobjchn(fd, ghostly, FALSE);
    find_lev_obj();
    /* restobjchn()'s `frozen' argument probably ought to be a callback
       routine so that we can check for objects being buried under ice */
    level.buriedobjlist = restobjchn(fd, ghostly, FALSE);
    billobjs = restobjchn(fd, ghostly, FALSE);
    level.mon_gen = rest_mongen_override(fd);
    level.sounds = rest_lvl_sounds(fd);
    rest_engravings(fd);

    /* reset level.monsters for new level */
    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            level.monsters[x][y] = (struct monst *) 0;
    for (mtmp = level.monlist; mtmp; mtmp = mtmp->nmon) {
        if (mtmp->isshk)
            set_residency(mtmp, FALSE);
        place_monster(mtmp, mtmp->mx, mtmp->my);
        if (mtmp->wormno) {
            place_wsegs(mtmp, NULL);
        }
    }
    restdamage(fd, ghostly);

    rest_regions(fd, ghostly);
    if (ghostly) {
        /* Now get rid of all the temp fruits... */
        freefruitchn(oldfruit),  oldfruit = 0;

        if (lev > ledger_no(&medusa_level) &&
            lev < ledger_no(&stronghold_level) && xdnstair == 0) {
            coord cc;

            mazexy(&cc);
            xdnstair = cc.x;
            ydnstair = cc.y;
            levl[cc.x][cc.y].typ = STAIRS;
        }

        br = Is_branchlev(&u.uz);
        if (br && u.uz.dlevel == 1) {
            d_level ltmp;

            if (on_level(&u.uz, &br->end1))
                assign_level(&ltmp, &br->end2);
            else
                assign_level(&ltmp, &br->end1);

            switch(br->type) {
            case BR_STAIR:
            case BR_NO_END1:
            case BR_NO_END2: /* OK to assign to sstairs if it's not used */
                assign_level(&sstairs.tolev, &ltmp);
                break;
            case BR_PORTAL: /* max of 1 portal per level */
            {
                struct trap *ttmp;
                for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
                    if (ttmp->ttyp == MAGIC_PORTAL)
                        break;
                if (!ttmp) panic("getlev: need portal but none found");
                assign_level(&ttmp->dst, &ltmp);
            }
            break;
            }
        } else if (!br) {
            /* Remove any dangling portals. */
            struct trap *ttmp;
            for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
                if (ttmp->ttyp == MAGIC_PORTAL) {
                    deltrap(ttmp);
                    break; /* max of 1 portal/level */
                }
        }
    }

    /* must come after all mons & objs are restored */
    relink_timers(ghostly);
    relink_light_sources(ghostly);
    reset_oattached_mids(ghostly);
#ifdef DUNGEON_GROWTH
    if (!ghostly) catchup_dgn_growths((monstermoves - omoves) / 5);
#endif
    if (ghostly)
        clear_id_mapping();
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

    if (n_ids_mapped)
        for (curr = id_map; curr; curr = curr->next) {
            /* first bucket might not be totally full */
            if (curr == id_map) {
                i = n_ids_mapped % N_PER_BUCKET;
                if (i == 0) i = N_PER_BUCKET;
            } else
                i = N_PER_BUCKET;

            while (--i >= 0)
                if (gid == curr->map[i].gid) {
                    *nidp = curr->map[i].nid;
                    return TRUE;
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

#ifdef ZEROCOMP
#define RLESC '\0'  /* Leading character for run of RLESC's */

#ifndef ZEROCOMP_BUFSIZ
#define ZEROCOMP_BUFSIZ BUFSZ
#endif
static NEARDATA unsigned char inbuf[ZEROCOMP_BUFSIZ];
static NEARDATA unsigned short inbufp = 0;
static NEARDATA unsigned short inbufsz = 0;
static NEARDATA short inrunlength = -1;
static NEARDATA int mreadfd;

static int
mgetc()
{
    if (inbufp >= inbufsz) {
        inbufsz = read(mreadfd, (genericptr_t)inbuf, sizeof inbuf);
        if (!inbufsz) {
            if (inbufp > sizeof inbuf)
                error("EOF on file #%d.\n", mreadfd);
            inbufp = 1 + sizeof inbuf; /* exactly one warning :-) */
            return -1;
        }
        inbufp = 0;
    }
    return inbuf[inbufp++];
}

void
minit()
{
    inbufsz = 0;
    inbufp = 0;
    inrunlength = -1;
}

int
mread(fd, buf, len)
int fd;
genericptr_t buf;
register unsigned len;
{
    /*register int readlen = 0;*/
    if (fd < 0) error("Restore error; mread attempting to read file %d.", fd);
    mreadfd = fd;
    while (len--) {
        if (inrunlength > 0) {
            inrunlength--;
            *(*((char **)&buf))++ = '\0';
        } else {
            short ch = mgetc();
            if (ch < 0) return -1; /*readlen;*/
            if ((*(*(char **)&buf)++ = (char)ch) == RLESC) {
                inrunlength = mgetc();
            }
        }
        /*readlen++;*/
    }
    return 0; /*readlen;*/
}

#else /* ZEROCOMP */

void
minit(void)
{
    return;
}

void
mread(int fd, genericptr_t buf, unsigned int len)
{
    int rlen;

#if defined(BSD) || defined(ULTRIX)
    rlen = read(fd, buf, (int) len);
    if(rlen != len) {
#else /* e.g. SYSV, __TURBOC__ */
    rlen = read(fd, buf, (unsigned) len);
    if((unsigned)rlen != len) {
#endif
        pline("Read %d instead of %u bytes.", rlen, len);
        if(restoring) {
            (void) close(fd);
            (void) delete_savefile();
            error("Error restoring old game.");
        }
        panic("Error reading level file.");
    }
}
#endif /* ZEROCOMP */

/*restore.c*/
