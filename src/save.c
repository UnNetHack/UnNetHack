/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "quest.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif
#if !defined(LSC) && !defined(O_WRONLY) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#ifdef MICRO
int dotcnt, dotrow; /* also used in restore */
#endif

#ifdef ZEROCOMP
static void bputc(int);
#endif
static void savelevchn(NHFILE *);
static void savelevl(NHFILE *,boolean);
static void savedamage(NHFILE *);
static void save_stairs(NHFILE *);
static void saveobj(NHFILE *, struct obj *);
static void saveobjchn(NHFILE *, struct obj **) NO_NNARGS;
static void savemon(NHFILE *, struct monst *);
static void savemonchn(NHFILE *, struct monst *) NO_NNARGS;
static void savetrapchn(NHFILE *, struct trap *) NO_NNARGS;
static void save_gamelog(NHFILE *);
static void savegamestate(NHFILE *);
static void savelev_core(NHFILE *, xint8);
static void save_msghistory(NHFILE *);
static void save_mongen_override(NHFILE *, struct mon_gen_override *);
static void save_lvl_sounds(NHFILE *, struct lvl_sounds *);

#ifdef ZEROCOMP
static void zerocomp_bufon(int);
static void zerocomp_bufoff(int);
static void zerocomp_bflush(int);
static void zerocomp_bwrite(int, genericptr_t, unsigned int);
static void zerocomp_bputc(int);
#endif

#if defined(HANGUPHANDLING)
#define HUP if (!program_state.done_hup)
#else
#define HUP
#endif

#ifdef MENU_COLOR
extern struct menucoloring *menu_colorings;
#endif

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
extern const struct text_color_option *text_colors;
#endif

/* need to preserve these during save to avoid accessing freed memory */
static unsigned ustuck_id = 0, usteed_id = 0;

int
dosave(void)
{
    if (iflags.debug_fuzzer) {
        return 0;
    }
    clear_nhwindow(WIN_MESSAGE);
    if (yn("Really save?") == 'n') {
        clear_nhwindow(WIN_MESSAGE);
        if (multi > 0) {
            nomul(0, 0);
        }
    } else {
        clear_nhwindow(WIN_MESSAGE);
        pline("Saving...");
#if defined(HANGUPHANDLING)
        program_state.done_hup = 0;
#endif
        if (dosave0()) {
#ifdef LIVELOGFILE
            livelog_game_action("saved");
#endif
            program_state.something_worth_saving = 0;
            u.uhp = -1;     /* universal game's over indicator */
            /* make sure they see the Saving message */
            display_nhwindow(WIN_MESSAGE, TRUE);
            exit_nhwindows("Be seeing you...");
            nh_terminate(EXIT_SUCCESS);
        } else {
            docrt();
        }
    }
    return 0;
}


#if defined(UNIX) || defined(VMS) || defined (__EMX__) || defined(WIN32)
/*ARGSUSED*/
void
hangup(
    int sig_unused UNUSED) /**< called as signal() handler, so sent
                                at least one arg */
{
# ifdef NOSAVEONHANGUP
    (void) signal(SIGINT, SIG_IGN);
    clearlocks();
#  ifndef VMS
    nh_terminate(EXIT_FAILURE);
#  endif
# else  /* SAVEONHANGUP */
    if (!program_state.done_hup++) {
        if (program_state.something_worth_saving) {
            (void) dosave0();
        }
#  ifdef VMS
        /* don't call exit when already within an exit handler;
           that would cancel any other pending user-mode handlers */
        if (!program_state.exiting) {
#  endif
        {
            clearlocks();
            nh_terminate(EXIT_FAILURE);
        }
    }
# endif
    return;
}
#endif

/* returns 1 if save successful */
int
dosave0(void)
{
    const char *fq_save;
    xint8 ltmp;
    char whynot[BUFSZ];
    NHFILE *nhfp, *onhfp;
    int res = 0;

#ifdef WHEREIS_FILE
    delete_whereis();
#endif

    program_state.saving++; /* inhibit status and perm_invent updates */

    /* we may get here via hangup signal, in which case we want to fix up
       a few of things before saving so that they won't be restored in
       an improper state; these will be no-ops for normal save sequence */
    u.uinvulnerable = 0;
    if (iflags.save_uswallow) {
        u.uswallow = 1;
        iflags.save_uswallow = 0;
    }
    if (iflags.save_uinwater) {
        u.uinwater = 1;
        iflags.save_uinwater = 0;
    }
    if (iflags.save_uburied) {
        u.uburied = 1;
        iflags.save_uburied = 0;
    }

    if (!program_state.something_worth_saving || !SAVEF[0]) {
        goto done;
    }

    fq_save = fqname(SAVEF, SAVEPREFIX, 1); /* level files take 0 */

#if defined(UNIX) || defined(VMS)
    (void) signal(SIGHUP, SIG_IGN);
#endif
#ifndef NO_SIGNAL
    (void) signal(SIGINT, SIG_IGN);
#endif

    HUP if (iflags.window_inited) {
        uncompress_area(fq_save, SAVEF);
        nhfp = open_savefile();
        if (nhfp) {
            close_nhfile(nhfp);
            clear_nhwindow(WIN_MESSAGE);
            There("seems to be an old save file.");
            if (yn("Overwrite the old file?") == 'n') {
                compress_area(fq_save, SAVEF);
                goto done;
            }
        }
    }

    HUP mark_synch();   /* flush any buffered screen output */

    nhfp = create_savefile();
    if (!nhfp) {
        HUP pline("Cannot open save file.");
        (void) delete_savefile(); /* ab@unido */
        goto done;
    }

    vision_recalc(2); /* shut down vision to prevent problems
                         in the event of an impossible() call */

    /* undo date-dependent luck adjustments made at startup time */
    if (flags.moonphase == FULL_MOON) { /* ut-sally!fletcher */
        change_luck(-1); /* and unido!ab */
    }
    if (flags.friday13) {
        change_luck(1);
    }
    if (iflags.window_inited) {
        HUP clear_nhwindow(WIN_MESSAGE);
    }

#ifdef MICRO
    dotcnt = 0;
    dotrow = 2;
    curs(WIN_MAP, 1, 1);
    if (!WINDOWPORT(X11))
        putstr(WIN_MAP, 0, "Saving:");
    }
#endif
    nhfp->mode = WRITING | FREEING;
    store_version(nhfp);
    store_savefileinfo(nhfp);
    if (nhfp && nhfp->fplog) {
        (void) fprintf(nhfp->fplog, "# post-validation\n");
    }
    store_plname_in_file(nhfp);
    ustuck_id = (u.ustuck ? u.ustuck->m_id : 0);
    usteed_id = (u.usteed ? u.usteed->m_id : 0);

    savelev(nhfp, ledger_no(&u.uz));
    savegamestate(nhfp);

    /* While copying level files around, zero out u.uz to keep
     * parts of the restore code from completely initializing all
     * in-core data structures, since all we're doing is copying.
     * This also avoids at least one nasty core dump.
     */
    uz_save = u.uz;
    u.uz.dnum = u.uz.dlevel = 0;
    /* these pointers are no longer valid, and at least u.usteed
     * may mislead place_monster() on other levels
     */
    set_ustuck((struct monst *) 0); /* also clears u.uswallow */
    u.usteed = (struct monst *)0;

    for (ltmp = (xint8)1; ltmp <= maxledgerno(); ltmp++) {
        if (ltmp == ledger_no(&uz_save)) {
            continue;
        }
        if (!(level_info[ltmp].flags & LFILE_EXISTS)) {
            continue;
        }
#ifdef MICRO
        curs(WIN_MAP, 1 + dotcnt++, dotrow);
        if (dotcnt >= (COLNO - 1)) {
            dotrow++;
            dotcnt = 0;
        }
        if (!WINDOWPORT(X11)) {
            putstr(WIN_MAP, 0, ".");
        }
        mark_synch();
#endif
        onhfp = open_levelfile(ltmp, whynot);
        if (!onhfp) {
            HUP pline("%s", whynot);
            close_nhfile(nhfp);
            (void) delete_savefile();
            HUP Strcpy(killer.name, whynot);
            HUP done(TRICKED);
            goto done;
        }
        minit(); /* ZEROCOMP */
        getlev(onhfp, hackpid, ltmp);
        close_nhfile(onhfp);
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &ltmp, sizeof ltmp); /* lvl no. */
        }
        savelev(nhfp, ltmp);     /* actual level*/
        delete_levelfile(ltmp);
    }
    close_nhfile(nhfp);

    u.uz = uz_save;
    uz_save.dnum = uz_save.dlevel = 0;

    /* get rid of current level --jgm */
    delete_levelfile(ledger_no(&u.uz));
    delete_levelfile(0);
    compress_area(FILE_AREA_SAVE, fq_save);
    /* this should probably come sooner... */
    program_state.something_worth_saving = 0;
    res = 1;

 done:
    program_state.saving--;
    return res;
}

static void
save_gamelog(NHFILE *nhfp)
{
    nhUse(nhfp);
#ifdef NEXT_VERSION
    struct gamelog_line *tmp = gamelog, *tmp2;
    int slen;

    while (tmp) {
        tmp2 = tmp->next;
        if (perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                slen = Strlen(tmp->text);
                bwrite(nhfp->fd, (genericptr_t) &slen, sizeof slen);
                bwrite(nhfp->fd, (genericptr_t) tmp->text, slen);
                bwrite(nhfp->fd, (genericptr_t) tmp, sizeof (struct gamelog_line));
            }
        }
        if (release_data(nhfp)) {
            free((genericptr_t) tmp->text);
            free((genericptr_t) tmp);
        }
        tmp = tmp2;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            slen = -1;
            bwrite(nhfp->fd, (genericptr_t) &slen, sizeof slen);
        }
    }
    if (release_data(nhfp)) {
        gamelog = NULL;
    }
#endif
}

static void
savegamestate(NHFILE *nhfp)
{
    unsigned long uid;
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
    time_t realtime;
#endif

    program_state.saving++; /* caller should/did already set this... */
    uid = (unsigned long) getuid();
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &uid, sizeof uid);
        bwrite(nhfp->fd, (genericptr_t) &flags, sizeof flags);
    }

    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &u, sizeof u);
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
        realtime = get_realtime();
        bwrite(nhfp->fd, (genericptr_t) &realtime, sizeof realtime);
#endif
    }

    /* must come before migrating_objs and migrating_mons are freed */
    save_timers(nhfp, RANGE_GLOBAL);
    save_light_sources(nhfp, RANGE_GLOBAL);

    /* when FREEING, deletes objects in invent and sets invent to Null;
       pointers into invent (uwep, uarmg, uamul, &c) are set to Null too */
    saveobjchn(nhfp, &invent);
    saveobjchn(nhfp, &migrating_objs); /* frees objs and sets to Null */
    savemonchn(nhfp, migrating_mons);
    if (release_data(nhfp)) {
        migrating_mons = 0;
    }
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) mvitals, sizeof mvitals);
    }
    save_dungeon(nhfp, (boolean) !!perform_bwrite(nhfp),
                 (boolean) !!release_data(nhfp));
    savelevchn(nhfp);
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &moves, sizeof moves);
        bwrite(nhfp->fd, (genericptr_t) &monstermoves, sizeof monstermoves);
        bwrite(nhfp->fd, (genericptr_t) &game_loop_counter, sizeof game_loop_counter);
        bwrite(nhfp->fd, (genericptr_t) &quest_status, sizeof(struct q_score));
        bwrite(nhfp->fd, (genericptr_t) spl_book, sizeof(struct spell) * (MAXSPELL + 1));
    }
    save_artifacts(nhfp);
    save_oracles(nhfp);
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) pl_tutorial, sizeof pl_tutorial);
        bwrite(nhfp->fd, (genericptr_t) pl_character, sizeof pl_character);
        bwrite(nhfp->fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
        bwrite(nhfp->fd, (genericptr_t) &current_fruit, sizeof current_fruit);
    }
    savefruitchn(nhfp);
    savenames(nhfp);
    save_msghistory(nhfp);
    save_gamelog(nhfp);
    save_waterlevel(nhfp);

#ifdef RECORD_ACHIEVE
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &achieve, sizeof achieve);
    }
#endif

    if (nhfp->structlevel) {
        bflush(nhfp->fd);
    }
    program_state.saving--;
}

/* potentially called from goto_level(do.c) as well as savestateinlock() */
boolean
tricked_fileremoved(NHFILE *nhfp, char *whynot)
{
    if (!nhfp) {
        pline("%s", whynot);
        pline("Probably someone removed it.");
        Strcpy(killer.name, whynot);
        done(TRICKED);
        return TRUE;
    }
    return FALSE;
}

#ifdef INSURANCE
void
savestateinlock(void)
{
    int hpid = 0;
    static boolean havestate = TRUE;
    char whynot[BUFSZ];
    NHFILE *nhfp;

    program_state.saving++; /* inhibit status and perm_invent updates */

    /* When checkpointing is on, the full state needs to be written
     * on each checkpoint.  When checkpointing is off, only the pid
     * needs to be in the level.0 file, so it does not need to be
     * constantly rewritten.  When checkpointing is turned off during
     * a game, however, the file has to be rewritten once to truncate
     * it and avoid restoring from outdated information.
     *
     * Restricting havestate to this routine means that an additional
     * noop pid rewriting will take place on the first "checkpoint" after
     * the game is started or restored, if checkpointing is off.
     */
    if (flags.ins_chkpt || havestate) {
        /* save the rest of the current game state in the lock file,
         * following the original int pid, the current level number,
         * and the current savefile name, which should not be subject
         * to any internal compression schemes since they must be
         * readable by an external utility
         */
        nhfp = open_levelfile(0, whynot);
        if (tricked_fileremoved(nhfp, whynot)) {
            program_state.saving--;
            return;
        }

        if (nhfp->structlevel) {
            (void) read(nhfp->fd, (genericptr_t) &hpid, sizeof hpid);
        }
        if (hackpid != hpid) {
            Sprintf(whynot,
                    "Level #0 pid (%d) doesn't match ours (%d)!",
                    hpid, hackpid);
            goto giveup;
        }
        close_nhfile(nhfp);

        nhfp = create_levelfile(0, whynot);
        if (!nhfp) {
            pline("%s", whynot);
 giveup:
            Strcpy(killer.name, whynot);
            /* done(TRICKED) will return when running in wizard mode;
               clear the display-update-suppression flag before rather
               than after so that screen updating behaves normally;
               game data shouldn't be inconsistent yet, unlike it would
               become midway through saving */
            program_state.saving--;
            done(TRICKED);
            return;
        }
        nhfp->mode = WRITING;
        if (nhfp->structlevel) {
            (void) write(nhfp->fd, (genericptr_t) &hackpid, sizeof hackpid);
        }
        if (flags.ins_chkpt) {
            int currlev = ledger_no(&u.uz);

            if (nhfp->structlevel) {
                (void) write(nhfp->fd, (genericptr_t) &currlev, sizeof currlev);
            }
            save_savefile_name(nhfp);
            store_version(nhfp);
            store_savefileinfo(nhfp);
            store_plname_in_file(nhfp);
            savegamestate(nhfp);
        }
        close_nhfile(nhfp);
    }
    program_state.saving--;
    havestate = flags.ins_chkpt;
}
#endif

void
savelev(NHFILE *nhfp, xint8 lev)
{
    boolean set_uz_save = (uz_save.dnum == 0 && uz_save.dlevel == 0);

    /* caller might have already set up gu.uz_save and zeroed u.uz;
       if not, we need to set it for save_bubbles(); caveat: if the
       player quits during character selection, u.uz won't be set yet
       but we'll be called during run-down */
    if (set_uz_save && perform_bwrite(nhfp)) {
        if (u.uz.dnum == 0 && u.uz.dlevel == 0) {
            program_state.something_worth_saving = 0;
            panic("savelev: where are we?");
        }
        uz_save = u.uz;
    }

    savelev_core(nhfp, lev);

    if (set_uz_save) {
        uz_save.dnum = uz_save.dlevel = 0; /* unset */
    }
}

static void
savelev_core(NHFILE *nhfp, xint8 lev)
{
#ifdef TOS
    short tlev;
#endif

    program_state.saving++; /* even if current mode is FREEING */

    if (!nhfp) {
        panic("Save on bad file!"); /* impossible */
    }
    /*
     *  Level file contents:
     *    version info (handled by caller);
     *    save file info (compression type; also by caller);
     *    process ID;
     *    internal level number (ledger number);
     *    bones info;
     *    actual level data.
     *
     *  If we're tearing down the current level without saving anything
     *  (which happens at end of game or upon entrance to endgame or
     *  after an aborted restore attempt) then we don't want to do any
     *  actual I/O.  So when only freeing, we skip to the bones info
     *  portion (which has some freeing to do), then jump quite a bit
     *  further ahead to the middle of the 'actual level data' portion.
     */
    if (nhfp->mode != FREEING) {
        /* WRITE_SAVE (probably ORed with FREE_SAVE), or COUNT_SAVE */

        /* purge any dead monsters (necessary if we're starting
           a panic save rather than a normal one, or sometimes
           when changing levels without taking time -- e.g.
           create statue trap then immediately level teleport) */
        if (iflags.purge_monsters) {
            dmonsfree();
        }

        if (lev >= 0 && lev <= maxledgerno()) {
            level_info[lev].flags |= VISITED;
        }
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &hackpid, sizeof hackpid);
        }
#ifdef TOS
        tlev = lev;
        tlev &= 0x00ff;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &tlev, sizeof tlev);
        }
#else
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &lev, sizeof lev);
        }
#endif
    }

    /* bones info comes before level data; the intent is for an external
       program ('hearse') to be able to match a bones file with the
       corresponding log file entry--or perhaps just skip that?--without
       the guessing that was needed in 3.4.3 and without having to
       interpret level data to find where to start; unfortunately it
       still needs to handle all the data compression schemes */
    savecemetery(nhfp, &level.bonesinfo);

    if (nhfp->mode == FREEING) { /* see above */
        goto skip_lots;
    }

    savelevl(nhfp, ((sfsaveinfo.sfi1 & SFI1_RLECOMP) == SFI1_RLECOMP));
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &monstermoves, sizeof(monstermoves));
        save_stairs(nhfp);
        bwrite(nhfp->fd, (genericptr_t) &updest, sizeof(dest_area));
        bwrite(nhfp->fd, (genericptr_t) &dndest, sizeof(dest_area));
        bwrite(nhfp->fd, (genericptr_t) &level.flags, sizeof(level.flags));
        bwrite(nhfp->fd, (genericptr_t) doors, sizeof(doors));
    }
    save_rooms(nhfp); /* no dynamic memory to reclaim */

    /* from here on out, saving also involves allocated memory cleanup */
skip_lots:
    /* timers and lights must be saved before monsters and objects */
    save_timers(nhfp, RANGE_LEVEL);
    save_light_sources(nhfp, RANGE_LEVEL);

    savemonchn(nhfp, fmon);
    save_worm(nhfp); /* save worm information */
    savetrapchn(nhfp, ftrap);
    saveobjchn(nhfp, &fobj);
    saveobjchn(nhfp, &level.buriedobjlist);
    saveobjchn(nhfp, &billobjs);
    save_mongen_override(nhfp, level.mon_gen);
    save_lvl_sounds(nhfp, level.sounds);
    save_engravings(nhfp);
    savedamage(nhfp); /* pending shop wall and/or floor repair */
    save_regions(nhfp);
    if (nhfp->mode != FREEING) {
        if (nhfp->structlevel) {
            bflush(nhfp->fd);
        }
    }
    program_state.saving--;
    if (release_data(nhfp)) {
        int x,y;
        for (y = 0; y < ROWNO; y++) {
            for (x = 0; x < COLNO; x++) {
                level.monsters[x][y] = 0;
            }
        }
        fmon = 0;
        ftrap = 0;
        fobj = 0;
        level.buriedobjlist = 0;
        billobjs = 0;
        free(level.mon_gen);
        level.mon_gen = NULL;
        level.sounds = NULL;
    }
}

static void
savelevl(NHFILE *nhfp, boolean rlecomp)
{
#ifdef RLECOMP
    struct rm *prm, *rgrm;
    int x, y;
    uchar match;

    if (rlecomp) {
        /* perform run-length encoding of rm structs */

        rgrm = &levl[0][0]; /* start matching at first rm */
        match = 0;

        for (y = 0; y < ROWNO; y++) {
            for (x = 0; x < COLNO; x++) {
                prm = &levl[x][y];
                if (prm->glyph == rgrm->glyph &&
                     prm->typ == rgrm->typ &&
                     prm->seenv == rgrm->seenv &&
                     prm->horizontal == rgrm->horizontal &&
                     prm->flags == rgrm->flags &&
                     prm->lit == rgrm->lit &&
                     prm->waslit == rgrm->waslit &&
                     prm->roomno == rgrm->roomno &&
                     prm->edge == rgrm->edge &&
                     prm->candig == rgrm->candig) {
                    match++;
                    if (match > 254) {
                        match = 254; /* undo this match */
                        goto writeout;
                    }
                } else {
                    /* run has been broken, write out run-length encoding */
 writeout:
                    if (nhfp->structlevel) {
                        bwrite(nhfp->fd, (genericptr_t) &match, sizeof match);
                        bwrite(nhfp->fd, (genericptr_t) rgrm, sizeof *rgrm);
                    }
                    /* start encoding again. we have at least 1 rm
                       in the next run, viz. this one. */
                    match = 1;
                    rgrm = prm;
                }
            }
        }
        if (match > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &match, sizeof (uchar));
                bwrite(nhfp->fd, (genericptr_t) rgrm, sizeof (struct rm));
            }
        }
        return;
    }
#else /* !RLECOMP */
    nhUse(rlecomp);
#endif /* ?RLECOMP */
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) levl, sizeof levl);
    }
    return;
}

static void
savelevchn(NHFILE *nhfp)
{
    s_level *tmplev, *tmplev2;
    int cnt = 0;

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev->next) {
        cnt++;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &cnt, sizeof cnt);
        }
    }
    for (tmplev = sp_levchn; tmplev; tmplev = tmplev2) {
        tmplev2 = tmplev->next;
        if (perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) tmplev, sizeof *tmplev);
            }
        }
        if (release_data(nhfp)) {
            free((genericptr_t) tmplev);
        }
    }
    if (release_data(nhfp)) {
        sp_levchn = 0;
    }
}

void
store_plname_in_file(NHFILE *nhfp)
{
    int plsiztmp = PL_NSIZ;

    if (nhfp->structlevel) {
        bufoff(nhfp->fd);
        /* bwrite() before bufon() uses plain write() */
        bwrite(nhfp->fd, (genericptr_t) &plsiztmp, sizeof plsiztmp);
        bwrite(nhfp->fd, (genericptr_t) plname, plsiztmp);
        bufon(nhfp->fd);
    }
    return;
}

static void
save_msghistory(NHFILE *nhfp)
{
    nhUse(nhfp);
#ifdef NEXT_VERSION
    char *msg;
    int msgcount = 0, msglen;
    int minusone = -1;
    boolean init = TRUE;

    if (perform_bwrite(nhfp)) {
        /* ask window port for each message in sequence */
        while ((msg = getmsghistory(init)) != 0) {
            init = FALSE;
            msglen = Strlen(msg);
            if (msglen < 1) {
                continue;
            }
            /* sanity: truncate if necessary (shouldn't happen);
               no need to modify msg[] since terminator isn't written */
            if (msglen > BUFSZ - 1) {
                msglen = BUFSZ - 1;
            }
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &msglen, sizeof msglen);
                bwrite(nhfp->fd, (genericptr_t) msg, msglen);
            }
            ++msgcount;
        }
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &minusone, sizeof (int));
        }
    }
    debug_pline("Stored %d messages into savefile.", msgcount);
    /* note: we don't attempt to handle release_data() here */
#endif
}

#ifdef NEXT_VERSION
/* save Plane of Water's air bubbles and Plane of Air's clouds */
static void
save_bubbles(NHFILE *nhfp, xint8 lev)
{
    xint8 bbubbly;

    /* air bubbles and clouds used to be saved as part of game state
       because restoring them needs dungeon data that isn't available
       during the first pass of their levels; now that they are part of
       the current level instead, we write a zero or non-zero marker
       so that restore can determine whether they are present even when
       u.uz and ledger_no() aren't available to it yet */
    bbubbly = 0;
    if (lev == ledger_no(&water_level) || lev == ledger_no(&air_level))
        bbubbly = lev; /* non-zero */
    if (nhfp->structlevel)
        bwrite(nhfp->fd, (genericptr_t) &bbubbly, sizeof bbubbly);

    if (bbubbly)
        save_waterlevel(nhfp); /* save air bubbles or clouds */
}
#endif

/* used when saving a level and also when saving dungeon overview data */
void
savecemetery(NHFILE *nhfp, struct cemetery **cemeteryaddr)
{
    struct cemetery *thisbones, *nextbones;
    int flag;

    flag = *cemeteryaddr ? 0 : -1;
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &flag, sizeof flag);
        }
    }
    nextbones = *cemeteryaddr;
    while ((thisbones = nextbones) != 0) {
        nextbones = thisbones->next;
        if (perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) thisbones, sizeof *thisbones);
            }
        }
        if (release_data(nhfp)) {
            free(thisbones);
        }
    }
    if (release_data(nhfp)) {
        *cemeteryaddr = 0;
    }
}

static void
savedamage(NHFILE *nhfp)
{
    struct damage *damageptr, *tmp_dam;
    unsigned int xl = 0;

    damageptr = level.damagelist;
    for (tmp_dam = damageptr; tmp_dam; tmp_dam = tmp_dam->next) {
        xl++;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &xl, sizeof xl);
        }
    }
    while (xl--) {
        if (perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) damageptr, sizeof *damageptr);
            }
        }
        tmp_dam = damageptr;
        damageptr = damageptr->next;
        if (release_data(nhfp)) {
            free((genericptr_t) tmp_dam);
        }
    }
    if (release_data(nhfp)) {
        level.damagelist = 0;
        level.damagelist = 0;
    }
}

void
save_mongen_override(NHFILE *nhfp, struct mon_gen_override *or)
{
    struct mon_gen_tuple *mt;
    struct mon_gen_tuple *prev;
    int marker = 0;

    if (!or) {
        if (perform_bwrite(nhfp)) {
            marker = 0;
            bwrite(nhfp->fd, (genericptr_t) &marker, sizeof(marker));
        }
    } else {
        if (perform_bwrite(nhfp)) {
            marker = 1;
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &marker, sizeof(marker));
                bwrite(nhfp->fd, (genericptr_t) or, sizeof(struct mon_gen_override));
            }
        }
        mt = or->gen_chances;
        while (mt) {
            if (perform_bwrite(nhfp)) {
                if (nhfp->structlevel) {
                    bwrite(nhfp->fd, (genericptr_t) mt, sizeof(struct mon_gen_tuple));
                }
            }
            prev = mt;
            mt = mt->next;
            if (release_data(nhfp)) {
                free(prev);
            }
        }
        if (release_data(nhfp)) {
            or->gen_chances = NULL;
        }
    }
}

void
save_lvl_sounds(NHFILE *nhfp, struct lvl_sounds *or)
{
    int marker = 0;
    int i;
    int len;

    if (!or) {
        if (perform_bwrite(nhfp)) {
            marker = 0;
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &marker, sizeof(marker));
            }
        }
    } else {
        if (perform_bwrite(nhfp)) {
            marker = 1;
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &marker, sizeof(marker));
                bwrite(nhfp->fd, (genericptr_t) or, sizeof(struct lvl_sounds));
            }

            for (i = 0; i < or->n_sounds; i++) {
                if (nhfp->structlevel) {
                    bwrite(nhfp->fd, (genericptr_t)&(or->sounds[i].flags), sizeof(or->sounds[i].flags));
                }
                len = strlen(or->sounds[i].msg)+1;
                if (nhfp->structlevel) {
                    bwrite(nhfp->fd, (genericptr_t)&len, sizeof(len));
                    bwrite(nhfp->fd, (genericptr_t)or->sounds[i].msg, len);
                }
            }
        }
        if (release_data(nhfp)) {
            for (i = 0; i < or->n_sounds; i++) {
                free(or->sounds[i].msg);
            }
            free(or->sounds);
            or->sounds = NULL;
            or->n_sounds = 0;
        }
    }
}

static void
save_stairs(NHFILE *nhfp)
{
    stairway *stway = stairs;
    int buflen = (int) sizeof *stway;

    while (stway) {
        if (perform_bwrite(nhfp)) {
            boolean use_relative = (program_state.restoring != REST_GSTATE &&
                                    stway->tolev.dnum == u.uz.dnum);
            if (use_relative) {
                /* make dlevel relative to current level */
                stway->tolev.dlevel -= u.uz.dlevel;
            }
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
                bwrite(nhfp->fd, (genericptr_t) stway, sizeof *stway);
            }
            if (use_relative) {
                /* reset stairway dlevel back to absolute */
                stway->tolev.dlevel += u.uz.dlevel;
            }
        }
        stway = stway->next;
    }
    if (perform_bwrite(nhfp)) {
        buflen = -1;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
    }
}

static void
saveobj(NHFILE *nhfp, struct obj *otmp)
{
    int buflen, zerobuf = 0;

    buflen = (int) sizeof (struct obj);
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        bwrite(nhfp->fd, (genericptr_t) otmp, buflen);
    }
    if (otmp->oextra) {
        buflen = ONAME(otmp) ? (int) strlen(ONAME(otmp)) + 1 : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) ONAME(otmp), buflen);
            }
        }
        /* defer to savemon() for this one */
        if (OMONST(otmp)) {
            savemon(nhfp, OMONST(otmp));
        } else {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &zerobuf, sizeof zerobuf);
            }
        }
        /* extra info about scroll of mail */
        buflen = OMAILCMD(otmp) ? (int) strlen(OMAILCMD(otmp)) + 1 : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                  bwrite(nhfp->fd, (genericptr_t) OMAILCMD(otmp), buflen);
            }
        }
        /* omid used to be indirect via a pointer in oextra but has
           become part of oextra itself; 0 means not applicable and
           gets saved/restored whenever any other oextra components do */
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &OMID(otmp), sizeof OMID(otmp));
        }
    }
}

/* save an object chain; sets head of list to Null when done;
   handles release_data() for each object in the list */
static void
saveobjchn(NHFILE *nhfp, struct obj **obj_p)
{
    struct obj *otmp = *obj_p;
    struct obj *otmp2;
    boolean is_invent = (otmp && otmp == invent);
    int minusone = -1;

    while (otmp) {
        otmp2 = otmp->nobj;
        if (perform_bwrite(nhfp)) {
            saveobj(nhfp, otmp);
        }
        if (Has_contents(otmp)) {
            saveobjchn(nhfp, &otmp->cobj);
        }
        if (release_data(nhfp)) {
            /*
             * If these are on the floor, the discarding could be
             * due to game save, or we could just be changing levels.
             * Always invalidate the pointer, but ensure that we have
             * the o_id in order to restore the pointer on reload.
             */
#if NEXT_VERSION
            if (otmp == context.victual.piece) {
                context.victual.o_id = otmp->o_id;
                context.victual.piece = (struct obj *) 0;
            }
            if (otmp == context.tin.tin) {
                context.tin.o_id = otmp->o_id;
                context.tin.tin = (struct obj *) 0;
            }
            if (otmp == context.spbook.book) {
                context.spbook.o_id = otmp->o_id;
                context.spbook.book = (struct obj *) 0;
            }
#endif
            otmp->where = OBJ_FREE; /* set to free so dealloc will work */
            otmp->nobj = NULL;      /* nobj saved into otmp2 */
            otmp->cobj = NULL;      /* contents handled above */
            otmp->timed = 0;        /* not timed any more */
            otmp->lamplit = 0;      /* caller handled lights */
            otmp->leashmon = 0;     /* mon->mleashed could still be set;
                                     * unfortunately, we can't clear that
                                     * until after the monster is saved */
            /* clear 'uball' and 'uchain' pointers if resetting their mask;
               could also do same for other worn items but don't need to */
            if ((otmp->owornmask & (W_BALL | W_CHAIN)) != 0) {
                setworn((struct obj *) 0,
                        otmp->owornmask & (W_BALL | W_CHAIN));
            }
            otmp->owornmask = 0L;   /* no longer care */
            dealloc_obj(otmp);
        }
        otmp = otmp2;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &minusone, sizeof (int));
        }
    }
    if (release_data(nhfp)) {
        if (is_invent) {
            allunworn(); /* clear uwep, uarm, uball, &c pointers */
        }
        *obj_p = (struct obj *) 0;
    }
}

static void
savemon(NHFILE *nhfp, struct monst *mtmp)
{
    int buflen;

    mtmp->mtemplit = 0; /* normally clear; if set here then a panic save
                         * is being written while bhit() was executing */
    buflen = (int) sizeof (struct monst);
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        bwrite(nhfp->fd, (genericptr_t) mtmp, buflen);
    }
    if (mtmp->mextra) {
        buflen = MGIVENNAME(mtmp) ? (int) strlen(MGIVENNAME(mtmp)) + 1 : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) MGIVENNAME(mtmp), buflen);
            }
        }
        buflen = EGD(mtmp) ? (int) sizeof (struct egd) : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) EGD(mtmp), buflen);
            }
        }
        buflen = EPRI(mtmp) ? (int) sizeof (struct epri) : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof buflen);
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) EPRI(mtmp), buflen);
            }
        }
        buflen = ESHK(mtmp) ? (int) sizeof (struct eshk) : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof (int));
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) ESHK(mtmp), buflen);
            }
        }
        buflen = EMIN(mtmp) ? (int) sizeof (struct emin) : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof (int));
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) EMIN(mtmp), buflen);
            }
        }
        buflen = EDOG(mtmp) ? (int) sizeof (struct edog) : 0;
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &buflen, sizeof (int));
        }
        if (buflen > 0) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) EDOG(mtmp), buflen);
            }
        }
        /* mcorpsenm is inline int rather than pointer to something,
           so doesn't need to be preceded by a length field */
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &MCORPSENM(mtmp), sizeof MCORPSENM(mtmp));
        }
    }
}

static void
savemonchn(NHFILE *nhfp, struct monst *mtmp)
{
    struct monst *mtmp2;
    int minusone = -1;

    while (mtmp) {
        mtmp2 = mtmp->nmon;
        if (perform_bwrite(nhfp)) {
            mtmp->mnum = monsndx(mtmp->data);
            if (mtmp->ispriest) {
                forget_temple_entry(mtmp); /* EPRI() */
            }
            savemon(nhfp, mtmp);
        }
        if (mtmp->minvent) {
            saveobjchn(nhfp, &mtmp->minvent);
        }
        if (release_data(nhfp)) {
            if (mtmp == polearm.hitmon) {
                polearm.m_id = mtmp->m_id;
                polearm.hitmon = NULL;
            }
            if (mtmp == u.ustuck) {
                u.ustuck_mid = u.ustuck->m_id;
            }
            if (mtmp == u.usteed) {
                u.usteed_mid = u.usteed->m_id;
            }
            mtmp->nmon = NULL; /* nmon saved into mtmp2 */
            dealloc_monst(mtmp);
        }
        mtmp = mtmp2;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &minusone, sizeof (int));
        }
    }
}

static void
savetrapchn(NHFILE *nhfp, struct trap *trap)
{
    static struct trap zerotrap;
    struct trap *trap2;

    while (trap) {
        trap2 = trap->ntrap;
        if (perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) trap, sizeof *trap);
            }
        }
        if (release_data(nhfp)) {
            dealloc_trap(trap);
        }
        trap = trap2;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &zerotrap, sizeof zerotrap);
        }
    }
}

/* save all the fruit names and ID's; this is used only in saving whole games
 * (not levels) and in saving bones levels.  When saving a bones level,
 * we only want to save the fruits which exist on the bones level; the bones
 * level routine marks nonexistent fruits by making the fid negative.
 */
void
savefruitchn(NHFILE *nhfp)
{
    static struct fruit zerofruit;
    struct fruit *f2, *f1;

    f1 = ffruit;
    while (f1) {
        f2 = f1->nextf;
        if (f1->fid >= 0 && perform_bwrite(nhfp)) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) f1, sizeof *f1);
            }
        }
        if (release_data(nhfp)) {
            dealloc_fruit(f1);
        }
        f1 = f2;
    }
    if (perform_bwrite(nhfp)) {
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &zerofruit, sizeof zerofruit);
        }
    }
    if (release_data(nhfp)) {
        ffruit = 0;
    }
}

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)

void
free_percent_color_options(const struct percent_color_option *list_head)
{
    if (list_head == NULL) {
        return;
    }
    free_percent_color_options(list_head->next);
    free((genericptr_t)list_head);
}

void
free_text_color_options(const struct text_color_option *list_head)
{
    if (list_head == NULL) {
        return;
    }
    free_text_color_options(list_head->next);
    free((genericptr_t)list_head->text);
    free((genericptr_t)list_head);
}

void
free_status_colors(void)
{
    free_percent_color_options(hp_colors); hp_colors = NULL;
    free_percent_color_options(pw_colors); pw_colors = NULL;
    free_text_color_options(text_colors); text_colors = NULL;
}
#endif

void
store_savefileinfo(NHFILE *nhfp)
{
    /* sfcap (decl.c) describes the savefile feature capabilities
     * that are supported by this port/platform build.
     *
     * sfsaveinfo (decl.c) describes the savefile info that actually
     * gets written into the savefile, and is used to determine the
     * save file being written.
     *
     * sfrestinfo (decl.c) describes the savefile info that is
     * being used to read the information from an existing savefile.
     */

    if (nhfp->structlevel) {
        bufoff(nhfp->fd);
        /* bwrite() before bufon() uses plain write() */
        bwrite(nhfp->fd, (genericptr_t) &sfsaveinfo, (unsigned) sizeof sfsaveinfo);
        bufon(nhfp->fd);
    }
    return;
}

/* also called by prscore(); this probably belongs in dungeon.c... */
void
free_dungeons(void)
{
#ifdef FREE_ALL_MEMORY
    NHFILE tnhfp;

    zero_nhfile(&tnhfp);    /* also sets fd to -1 */
    tnhfp.mode = FREEING;
    savelevchn(&tnhfp);
    save_dungeon(&tnhfp, FALSE, TRUE);
#endif
    return;
}

void
freedynamicdata(void)
{
    NHFILE tnhfp;

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    free_status_colors();
#endif
    zero_nhfile(&tnhfp);    /* also sets fd to -1 */
    tnhfp.mode = FREEING;
    unload_qtlist();
    free_invbuf();  /* let_to_name (invent.c) */
    free_youbuf();  /* You_buf,&c (pline.c) */
    msgpline_free();
#ifdef MENU_COLOR
    free_menu_coloring();
#endif
    tmp_at(DISP_FREEMEM, 0);    /* temporary display effects */
#ifdef FREE_ALL_MEMORY
# define free_current_level() savelev(&tnhfp, -1)
# define freeobjchn(X) (saveobjchn(&tnhfp, &X), X = 0)
# define freemonchn(X) (savemonchn(&tnhfp, X), X = 0)
# define freefruitchn() savefruitchn(&tnhfp)
# define freenames() savenames(&tnhfp)
# define free_killers() save_killers(&tnhfp)
# define free_oracles() save_oracles(&tnhfp)
# define free_waterlevel() save_waterlevel(&tnhfp)
# define free_timers(R) save_timers(&tnhfp, R)
# define free_light_sources(R) save_light_sources(&tnhfp, R)
# define free_animals() mon_animal_list(FALSE)

    /* move-specific data */
    dmonsfree();        /* release dead monsters */

    /* level-specific data */
    free_timers(RANGE_LEVEL);
    free_light_sources(RANGE_LEVEL);
    freemonchn(fmon);
    freeobjchn(fobj);
    freeobjchn(level.buriedobjlist);
    freeobjchn(billobjs);

    /* game-state data */
    free_timers(RANGE_GLOBAL);
    free_light_sources(RANGE_GLOBAL);
    freeobjchn(invent);
    freeobjchn(migrating_objs);
    freemonchn(migrating_mons);
    freemonchn(mydogs);     /* ascension or dungeon escape */
    /* freelevchn();    [folded into free_dungeons()] */
    free_animals();
    free_oracles();
    freefruitchn();
    freenames();
    free_waterlevel();
    free_dungeons();

    /* some pointers in iflags */
    if (iflags.wc_font_map) {
        free(iflags.wc_font_map);
    }
    if (iflags.wc_font_message) {
        free(iflags.wc_font_message);
    }
    if (iflags.wc_font_text) {
        free(iflags.wc_font_text);
    }
    if (iflags.wc_font_menu) {
        free(iflags.wc_font_menu);
    }
    if (iflags.wc_font_status) {
        free(iflags.wc_font_status);
    }
    if (iflags.wc_tile_file) {
        free(iflags.wc_tile_file);
    }
#ifdef AUTOPICKUP_EXCEPTIONS
    free_autopickup_exceptions();
#endif

#endif  /* FREE_ALL_MEMORY */
    return;
}

/*save.c*/
