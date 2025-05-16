/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef WIZARD
static int wiz_showkills(void);    /* showborn patch */
#endif

static boolean minimal_enlightenment(void);

static void enlght_line(const char *, const char *, const char *, const char *);
static char *enlght_combatinc(const char *, int, int, char *);
static void enlght_halfdmg(int, int);

/* allmain.c */
extern int monclock;

/* #vanquished command */
static int dovanquished(void)
{
    list_vanquishedonly();
    return 0;
}

static int
dooverview_or_wiz_where(void)
{
    dooverview();
    return 0;
}

/* -enlightenment and conduct- */
static winid en_win;
static const char
    You_[] = "You ",
    are[]  = "are ",  were[]  = "were ",
    have[] = "have ", had[]   = "had ",
    can[]  = "can ",  could[] = "could ";
static const char
    have_been[]  = "have been ",
    have_never[] = "have never ", never[] = "never ";

/* macros to simplify output of enlightenment messages; also used by
   conduct and achievements */
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), final ? (past) : (present), (suffix), (ps))
#define you_are(attr, ps) enl_msg(You_, are, were, (attr), (ps))
#define you_have(attr, ps) enl_msg(You_, have, had, (attr), (ps))
#define you_can(attr, ps) enl_msg(You_, can, could, (attr), (ps))
#define you_have_been(goodthing) \
    enl_msg(You_, have_been, were, (goodthing), "")
#define you_have_never(badthing) \
    enl_msg(You_, have_never, never, (badthing), "")
#define you_have_X(something) \
    enl_msg(You_, have, (const char *) "", (something), "")

static void
enlght_out(const char *buf)
{
    putstr(en_win, 0, buf);
}

static int want_display = FALSE;
static void
enlght_line(
    const char *start,
    const char *middle,
    const char *end,
    const char *ps)
{
#ifndef NO_ENLGHT_CONTRACTIONS
    static const struct contrctn {
        const char *twowords, *contrctn;
    } contra[] = {
        { " are not ", " aren't " },
        { " were not ", " weren't " },
        { " have not ", " haven't " },
        { " had not ", " hadn't " },
        { " can not ", " can't " },
        { " could not ", " couldn't " },
    };
    int i;
#endif
    char buf[BUFSZ];

    Sprintf(buf, " %s%s%s%s.", start, middle, end, ps);
#ifndef NO_ENLGHT_CONTRACTIONS
    if (strstri(buf, " not ")) { /* TODO: switch to libc strstr() */
        for (i = 0; i < SIZE(contra); ++i) {
            (void) strsubst(buf, contra[i].twowords, contra[i].contrctn);
        }
    }
#endif
    if (want_display) {
        enlght_out(buf);
    }
    dump_list_item(buf);
}

/* format increased damage or chance to hit */
static char *
enlght_combatinc(const char *inctyp, int incamt, int final, char *outbuf)
{
    char numbuf[24];
    const char *modif, *bonus;

    if (final
#ifdef WIZARD
        || wizard
#endif
        ) {
        Sprintf(numbuf, "%s%d",
                (incamt > 0) ? "+" : "", incamt);
        modif = (const char *) numbuf;
    } else {
        int absamt = abs(incamt);

        if (absamt <= 3) {
            modif = "small";
        } else if (absamt <= 6) {
            modif = "moderate";
        } else if (absamt <= 12) {
            modif = "large";
        } else {
            modif = "huge";
        }
    }
    bonus = (incamt > 0) ? "bonus" : "penalty";
    /* "bonus to hit" vs "damage bonus" */
    if (!strcmp(inctyp, "damage")) {
        const char *ctmp = inctyp;
        inctyp = bonus;
        bonus = ctmp;
    }
    Sprintf(outbuf, "%s %s %s", an(modif), bonus, inctyp);
    return outbuf;
}

/* report half physical or half spell damage */
static void
enlght_halfdmg(int category, int final)
{
    const char *category_name;
    char buf[BUFSZ];

    switch (category) {
    case HALF_PHDAM:
        category_name = "physical";
        break;

    case HALF_SPDAM:
        category_name = "spell";
        break;

    default:
        category_name = "unknown";
        break;
    }
    Sprintf(buf, " %s %s damage", (final || wizard) ? "half" : "reduced", category_name);
    enl_msg(You_, "take", "took", buf, from_what(category));
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
static boolean
cause_known(int propindx) /* index of a property which can be conveyed by worn item */
{
    struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = invent; o; o = o->nobj) {
        if (!(o->owornmask & mask)) {
            continue;
        }
        if ((int) objects[o->otyp].oc_oprop == propindx &&
             objects[o->otyp].oc_name_known && o->dknown) {
            return TRUE;
        }
    }
    return FALSE;
}

void
enlightenment(int final, boolean want_disp)
            /* 0 => still in progress; 1 => over, survived; 2 => dead */

{
    int ltmp;
    char buf[BUFSZ];
    boolean magic = FALSE;

    want_display = want_disp;

    Sprintf(buf, final ? "Final Attributes:" : "Current Attributes:");
    if (want_display) {
        en_win = create_nhwindow(NHW_MENU);
        putstr(en_win, 0, buf);
        putstr(en_win, 0, "");
    }
    dump_title(buf);
    dump_list_start();

    if (u.uevent.uhand_of_elbereth) {
        static const char * const hofe_titles[3] = {
            "the Hand of Elbereth",
            "the Envoy of Balance",
            "the Glory of Arioch"
        };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1], "");
    }

    /* heaven or hell modes */
    if (heaven_or_hell_mode) {
        if (u.ulives > 1) {
            Sprintf(buf, "%ld lives left", u.ulives);
        } else if (u.ulives == 0) {
            Sprintf(buf, "no lives left");
        } else {
            Sprintf(buf, "%ld life left", u.ulives);
        }
        you_have(buf, "");
    }

    if (u.lastprayed) {
        Sprintf(buf, "You last %s %ld turns ago",
                u.lastprayresult == PRAY_GIFT ? "received a gift" :
                u.lastprayresult == PRAY_ANGER ? "angered your god" :
                u.lastprayresult == PRAY_CONV ? "converted to a new god" :
                "prayed",
                moves - u.lastprayed);
        enl_msg(buf, "", "", "", "");
        if (u.lastprayresult == PRAY_GOOD) {
            enl_msg("That prayer was well received", "", "", "", "");
        } else if (u.lastprayresult == PRAY_BAD) {
            enl_msg("That prayer was poorly received", "", "", "", "");
        } else if (u.lastprayresult == PRAY_INPROG) {
            enl_msg("That prayer ", "is ", "was ", "in progress", "");
        }
        if (u.reconciled) {
            if (u.reconciled == REC_REC) {
                Sprintf(buf, " since reconciled with your god");
                enl_msg("You ", "have", "had", buf, "");
            } else if (u.reconciled == REC_MOL) {
                Sprintf(buf, " since mollified your god");
                enl_msg("You ", "have", "had", buf, "");
            }
        }
    }

    /* note: piousness 3 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20) {
        you_are("piously aligned", "");
    } else if (u.ualign.record > 13) {
        you_are("devoutly aligned", "");
    } else if (u.ualign.record > 8) {
        you_are("fervently aligned", "");
    } else if (u.ualign.record > 3) {
        you_are("stridently aligned", "");
    } else if (u.ualign.record == 3) {
        you_are("aligned", "");
    } else if (u.ualign.record > 0) {
        you_are("haltingly aligned", "");
    } else if (u.ualign.record == 0) {
        you_are("nominally aligned", "");
    } else if (u.ualign.record >= -3) {
        you_have("strayed", "");
    } else if (u.ualign.record >= -8) {
        you_have("sinned", "");
    } else {
        you_have("transgressed", "");
    }
#ifdef WIZARD
    if (wizard || final) {
        Sprintf(buf, " %d", u.uhunger);
        enl_msg("Hunger level ", "is", "was", buf, "");
        Sprintf(buf, " %d / %ld", u.ualign.record, ALIGNLIM);
        enl_msg("Your alignment ", "is", "was", buf, "");
        Sprintf(buf, " %d - %d",
                min_monster_difficulty(), max_monster_difficulty());
        enl_msg("Monster difficulty range ", "is", "was", buf, "");
        Sprintf(buf, " %d", level_difficulty());
        enl_msg("Level difficulty ", "is", "was", buf, "");
    }
    if (wizard || final) {
        Sprintf(buf, " u%s", encode_base32(level_info[0].seed));
        enl_msg("Seed ", "is", "was", buf, "");
        if (is_game_pre_seeded) {
            enl_msg("You ", "are playing", "played", " a pre-seeded game", "");
        }
    }
    if ((wizard || final) && (monclock > 0)) {
        Sprintf(buf, "%2.2fx", (float)MIN_MONGEN_RATE/monclock);

        enl_msg("Monster generation rate ", "is ", "was ", buf, "");
    }
    if (wizard) {
        Snprintf(buf, sizeof(buf), "Current process id is %d", getpid());
        enl_msg("", "", "", buf, "");
    }
#endif

    /*** Resistances to troubles ***/
    if (Fire_resistance) {
        you_are("fire resistant", from_what(FIRE_RES));
    }
    if (Cold_resistance) {
        you_are("cold resistant", from_what(COLD_RES));
    }
    if (Sleep_resistance) {
        you_are("sleep resistant", from_what(SLEEP_RES));
    }
    if (Disint_resistance) {
        you_are("disintegration resistant", from_what(DISINT_RES));
    }
    if (Shock_resistance) {
        you_are("shock resistant", from_what(SHOCK_RES));
    }
    if (Poison_resistance) {
        you_are("poison resistant", from_what(POISON_RES));
    }
    if (Drain_resistance) {
        you_are("level-drain resistant", from_what(DRAIN_RES));
    }
    if (Sick_resistance) {
        you_are("immune to sickness", from_what(SICK_RES));
    }
    if (Antimagic) {
        you_are("magic-protected", from_what(ANTIMAGIC));
    }
    if (Acid_resistance) {
        you_are("acid resistant", from_what(ACID_RES));
    }
    if (Stone_resistance) {
        you_are("petrification resistant", from_what(STONE_RES));
    }
    if (Invulnerable) {
        you_are("invulnerable", from_what(INVULNERABLE));
    }
    if (u.uedibility) {
        you_can("recognize detrimental food", "");
    }

    /*** Troubles ***/
    if (!flags.perma_hallu && Halluc_resistance) {
        enl_msg("You resist", "", "ed", " hallucinations", "");
    }
    if (flags.perma_hallu) {
        you_are("permanently hallucinating", "");
    }
    if (final) {
        if (!flags.perma_hallu && Hallucination) {
            you_are("hallucinating", "");
        }
        if (Stunned) {
            you_are("stunned", "");
        }
        if (Confusion) {
            you_are("confused", "");
        }
        if (Blinded) {
            you_are("blinded", from_what(BLINDED));
        }
        if (Sick) {
            if (u.usick_type & SICK_VOMITABLE) {
                you_are("sick from food poisoning", "");
            }
            if (u.usick_type & SICK_NONVOMITABLE) {
                you_are("sick from illness", "");
            }
        }
        if (Punished) {
            you_are("punished", "");
        }
    }
    if (Stoned) {
        you_are("turning to stone", "");
    }
    if (Slimed) {
        you_are("turning into slime", "");
    }
    if (Strangled) {
        if (u.uburied) {
            you_are("buried", "");
        } else {
            if (final && (Strangled & I_SPECIAL)) {
                enlght_out(" You died from strangulation.");
            } else {
                Strcpy(buf, "being strangled");
                if (wizard) {
                    Sprintf(eos(buf), " (%ld)", (Strangled & TIMEOUT));
                }
                you_are(buf, from_what(STRANGLED));
            }
        }
    }
    if (Glib) {
        Sprintf(buf, "slippery %s", fingers_or_gloves(TRUE));
        you_have(buf, "");
    }
    if (Fumbling) {
        enl_msg("You fumble", "", "d", "", "");
    }
    if (Wounded_legs && !u.usteed) {
        Sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
        you_have(buf, "");
    }
#if defined(WIZARD)
    if (Wounded_legs && u.usteed && (wizard || final)) {
        Strcpy(buf, x_monnam(u.usteed, ARTICLE_YOUR, (char *)0,
                             SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, FALSE));
        *buf = highc(*buf);
        enl_msg(buf, " has", " had", " wounded legs", "");
    }
#endif
    if (Sleepy) {
        if (magic || cause_known(SLEEPY)) {
            Strcpy(buf, from_what(SLEEPY));
            if (wizard) {
                Sprintf(eos(buf), " (%ld)", (HSleepy & TIMEOUT));
            }
            enl_msg("You ", "fall", "fell", " asleep uncontrollably", buf);
        }
    }
    if (Hunger) {
        enl_msg("You hunger", "", "ed", " rapidly", "");
    }

    /*** Vision and senses ***/
    if (See_invisible) {
        enl_msg(You_, "see", "saw", " invisible", "");
    }
    if (Blind_telepat) {
        you_are("telepathic", from_what(TELEPAT));
    }
    if (Warning) {
        you_are("warned", from_what(WARNING));
    }
    if (Warn_of_mon && flags.warntype) {
        Sprintf(buf, "aware of the presence of %s",
                (flags.warntype & M2_ORC) ? "orcs" :
                (flags.warntype & M2_DEMON) ? "demons" :
                (flags.warntype & M2_GIANT) ? "giants" :
                (flags.warntype & M2_WERE) ? "lycanthropes" :
                (flags.warntype & M2_UNDEAD) ? "undead" :
                something);
        you_are(buf, from_what(WARN_OF_MON));
    } else if (Warn_of_mon && uwep) {
        const char *monster_name = get_warned_of_monster(uwep);
        if (monster_name) {
            Sprintf(buf, "aware of the presence of %s", monster_name);
            you_are(buf, from_what(WARN_OF_MON));
        }
    }
    if (Undead_warning) {
        you_are("warned of undead", from_what(WARN_UNDEAD));
    }
    if (Searching) {
        you_have("automatic searching", from_what(SEARCHING));
    }
    if (Clairvoyant) {
        you_are("clairvoyant", from_what(CLAIRVOYANT));
    }
    if (Infravision) {
        you_have("infravision", from_what(INFRAVISION));
    }
    if (Detect_monsters) {
        you_are("sensing the presence of monsters", "");
    }
    if (u.umconf) {
        you_are("going to confuse monsters", "");
    }
    if (flags.confused_reading) {
        you_are("confused from reading magic", "");
    }

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if (uleft && uleft->otyp == RIN_ADORNMENT) {
            adorn += uleft->spe;
        }
        if (uright && uright->otyp == RIN_ADORNMENT) {
            adorn += uright->spe;
        }
        if (adorn < 0) {
            you_are("poorly adorned", from_what(ADORNED));
        } else {
            you_are("adorned", from_what(ADORNED));
        }
    }
    if (Invisible) {
        you_are("invisible", from_what(INVIS));
    } else if (Invis) {
        you_are("invisible to others", from_what(INVIS));
    }
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis) {
        you_are("visible", from_what(-INVIS));
    }
    if (Displaced) {
        you_are("displaced", from_what(DISPLACED));
    }
    if (Stealth) {
        you_are("stealthy", from_what(STEALTH));
    }
    if (Aggravate_monster) {
        enl_msg("You aggravate", "", "d", " monsters",
                from_what(AGGRAVATE_MONSTER));
    }
    if (Conflict) {
        enl_msg("You cause", "", "d", " conflict", from_what(CONFLICT));
    }

    /*** Transportation ***/
    if (Jumping) {
        you_can("jump", from_what(JUMPING));
    }
    if (Teleportation) {
        you_can("teleport", from_what(TELEPORT));
    }
    if (Teleport_control) {
        you_have("teleport control", from_what(TELEPORT_CONTROL));
    }
    if (Lev_at_will) {
        you_are("levitating, at will", "");
    } else if (Levitation) {
        you_are("levitating", from_what(LEVITATION)); /* without control */
    } else if (Flying) {
        you_can("fly", from_what(FLYING));
    }
    if (Wwalking) {
        you_can("walk on water", from_what(WWALKING));
    }
    if (Swimming) {
        you_can("swim", from_what(SWIMMING));
    }
    if (Breathless) {
        you_can("survive without air", from_what(MAGICAL_BREATHING));
    } else if (Amphibious) {
        you_can("breathe water", from_what(MAGICAL_BREATHING));
    }
    if (Passes_walls) {
        you_can("walk through walls", from_what(PASSES_WALLS));
    }
    /* If you die while dismounting, u.usteed is still set.  Since several
     * places in the done() sequence depend on u.usteed, just detect this
     * special case. */
    if (u.usteed && (final < 2 || strcmp(killer.name, "riding accident"))) {
        Sprintf(buf, "riding %s", y_monnam(u.usteed));
        you_are(buf, "");
    }
    if (u.uswallow) {
        Sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
#ifdef WIZARD
        if (wizard || final) {
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        }
#endif
        you_are(buf, "");
    } else if (u.ustuck) {
        Sprintf(buf, "%s %s",
                (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
                a_monnam(u.ustuck));
        you_are(buf, "");
    }

    /*** Physical attributes ***/
    if (u.uhitinc) {
        you_have(enlght_combatinc("to hit", u.uhitinc, final, buf), "");
    }
    if (u.udaminc) {
        you_have(enlght_combatinc("damage", u.udaminc, final, buf), "");
    }
    if (Slow_digestion) {
        you_have("slower digestion", from_what(SLOW_DIGESTION));
    }
    if (Regeneration && can_regenerate()) {
        enl_msg("You regenerate", "", "d", "", from_what(REGENERATION));
    }
    if (!can_regenerate()) {
        if (marathon_mode) {
            enl_msg("You ", "can not", "could not", " regenerate in marathon mode", "");
        } else if (is_elf(youmonst.data)) {
            you_are("in direct contact with cold iron", "");
            Sprintf(buf, " regenerate because of %s", yobjnam(which_obj_prevents_regeneration(), (char *) 0));
            enl_msg("You ", "can not", "could not", buf, "");
        } else if (is_vampiric(youmonst.data)) {
            you_are("in direct contact with silver", "");
        }
    }
    if (u.uspellprot || Protection) {
        int prot = 0;

        if (uleft && uleft->otyp == RIN_PROTECTION) {
            prot += uleft->spe;
        }
        if (uright && uright->otyp == RIN_PROTECTION) {
            prot += uright->spe;
        }
        if (HProtection & INTRINSIC) {
            prot += u.ublessed;
        }
        prot += u.uspellprot;

        if (prot < 0) {
            you_are("ineffectively protected", "");
        } else if (prot > 0) {
            you_are("protected", "");
        }
    }
    if (Half_physical_damage) {
        enlght_halfdmg(HALF_PHDAM, final);
    }
    if (Half_spell_damage) {
        enlght_halfdmg(HALF_SPDAM, final);
    }
    if (Half_gas_damage) {
        enl_msg(You_, "take", "took", " reduced poison gas damage", "");
    }
    /* polymorph and other shape change */
    if (Protection_from_shape_changers) {
        you_are("protected from shape changers", from_what(PROT_FROM_SHAPE_CHANGERS));
    }
    if (Polymorph) {
        you_are("polymorphing", "");
    }
    if (Polymorph_control) {
        you_have("polymorph control", from_what(POLYMORPH_CONTROL));
    }
    if (u.ulycn >= LOW_PM) {
        Strcpy(buf, an(mons[u.ulycn].mname));
        you_are(buf, "");
    }
    if (Upolyd) {
        if (u.umonnum == u.ulycn) {
            Strcpy(buf, "in beast form");
        } else {
            Sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
        }
#ifdef WIZARD
        if (wizard || final) {
            Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
#endif
        you_are(buf, "");
    }
    if (Unchanging) {
        you_can("not change from your current form", from_what(UNCHANGING));
    }
    if (Fast) {
        you_are(Very_fast ? "very fast" : "fast", from_what(FAST));
    }
    if (Reflecting) {
        you_have("reflection", from_what(REFLECTING));
    }
    if (Free_action) {
        you_have("free action", from_what(FREE_ACTION));
    }
    if (Fixed_abil) {
        you_have("fixed abilities", from_what(FIXED_ABIL));
    }
    if (Lifesaved) {
        enl_msg("Your life ", "will be", "would have been", " saved", "");
    }
    if (u.twoweap) {
        you_are("wielding two weapons at once", "");
    }

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int)Luck);
        Sprintf(buf, "%s%slucky",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "");
#ifdef WIZARD
        if (wizard || final) {
            Sprintf(eos(buf), " (%d)", Luck);
        }
#endif
        you_are(buf, "");
    }
#ifdef WIZARD
    else if (wizard || final) {
        enl_msg("Your luck ", "is", "was", " zero", "");
    }
#endif
    if (u.moreluck > 0) {
        you_have("extra luck", "");
    } else if (u.moreluck < 0) {
        you_have("reduced luck", "");
    }
    if (has_luckitem()) {
        ltmp = stone_luck(FALSE);
        if (ltmp <= 0) {
            enl_msg("Bad luck ", "times", "timed", " out slowly for you", "");
        }
        if (ltmp >= 0) {
            enl_msg("Good luck ", "times", "timed", " out slowly for you", "");
        }
    }

    if (u.ugangr) {
        Sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
#ifdef WIZARD
        if (wizard || final) {
            Sprintf(eos(buf), " (%d)", u.ugangr);
        }
#endif
        enl_msg(u_gname(), " is", " was", buf, "");
    } else {
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
#if 0
            /* "can [not] safely pray" vs "could [not] have safely prayed" */
               Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
                       final ? "have " : "", final ? "ed" : "");
#else
            Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
#ifdef WIZARD
            if (wizard || final) {
                Sprintf(eos(buf), " (%d)", u.ublesscnt);
            }
#endif
            you_can(buf, "");
        }
    }

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
            p = "survived after being killed ";
            switch (u.umortality) {
            case 0:  p = !final ? (char *)0 : "survived";  break;
            case 1:  Strcpy(buf, "once");  break;
            case 2:  Strcpy(buf, "twice");  break;
            case 3:  Strcpy(buf, "thrice");  break;
            default: Sprintf(buf, "%d times", u.umortality);
                break;
            }
        } else {    /* game ended in character's death */
            p = "are dead";
            switch (u.umortality) {
            case 0:  warning("dead without dying?");
            case 1:  break;     /* just "are dead" */
            default: Sprintf(buf, " (%d%s time!)", u.umortality,
                             ordin(u.umortality));
                break;
            }
        }
        if (p) {
            enl_msg(You_, "have been killed ", p, buf, "");
        }
    }
    dump_list_end();
    dump("", "");

    if (want_display) {
        display_nhwindow(en_win, TRUE);
        destroy_nhwindow(en_win);
    }
}

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns FALSE if menu cancelled (dismissed with ESC), TRUE otherwise.
 */
static boolean
minimal_enlightenment(void)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    int genidx, n;
    char buf[BUFSZ], buf2[BUFSZ];
    static const char untabbed_fmtstr[] = "%-15s: %-12s";
    static const char untabbed_deity_fmtstr[] = "%-17s%s";
    static const char tabbed_fmtstr[] = "%s:\t%-12s";
    static const char tabbed_deity_fmtstr[] = "%s\t%s";
    static const char *fmtstr;
    static const char *deity_fmtstr;

    fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
    deity_fmtstr = iflags.menu_tab_sep ?
                   tabbed_deity_fmtstr : untabbed_deity_fmtstr;
    any.a_void = 0;
    buf[0] = buf2[0] = '\0';
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Starting", FALSE);

    /* Starting name, race, role, gender */
    Sprintf(buf, fmtstr, "name", plname);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "race", urace.noun);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "role",
            (flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Starting alignment */
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Current name, race, role, gender */
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Current", FALSE);
    Sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd) {
        Sprintf(buf, fmtstr, "role (base)",
                (u.mfemale && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    } else {
        Sprintf(buf, fmtstr, "role",
                (flags.female && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }
    /* don't want poly_gender() here; it forces `2' for non-humanoids */
    genidx = is_neuter(youmonst.data) ? 2 : flags.female;
    Sprintf(buf, fmtstr, "gender", genders[genidx].adj);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd && (int)u.mfemale != genidx) {
        Sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }

    /* Current alignment */
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Current position of hero */
    if (wizard) {
        Sprintf(buf2, "(%2d,%2d)", u.ux, u.uy);
        Sprintf(buf, fmtstr, "position", buf2);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }

    /* Deity list */
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Deities", FALSE);
    Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_CHAOTIC) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_CHAOTIC)       ? " (s)" :
            (u.ualign.type   == A_CHAOTIC)       ? " (c)" : "");
    Sprintf(buf, fmtstr, "Chaotic", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_NEUTRAL) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_NEUTRAL)       ? " (s)" :
            (u.ualign.type   == A_NEUTRAL)       ? " (c)" : "");
    Sprintf(buf, fmtstr, "Neutral", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type &&
             u.ualign.type == A_LAWFUL)  ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_LAWFUL)        ? " (s)" :
            (u.ualign.type   == A_LAWFUL)        ? " (c)" : "");
    Sprintf(buf, fmtstr, "Lawful", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    end_menu(tmpwin, "Base Attributes");
    n = select_menu(tmpwin, PICK_NONE, &selected);
    destroy_nhwindow(tmpwin);
    return (n != -1);
}

int
doattributes(void)
{
    if (!minimal_enlightenment()) {
        return 0;
    }
    if (wizard || discover) {
        enlightenment(0, TRUE);
    }
    return 0;
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
int
doconduct(void)
{
    show_conduct(0, TRUE);
    return 0;
}

void
show_conduct(int final, boolean want_disp)
{
    char buf[BUFSZ];
    int ngenocided;
    int cdt;

    want_display = want_disp;

    /* Create the conduct window */
    Sprintf(buf, "Voluntary challenges:");
    if (want_display) {
        en_win = create_nhwindow(NHW_MENU);
        putstr(en_win, 0, buf);
        putstr(en_win, 0, "");
    }
    dump_title(buf);
    dump_list_start();

    if (heaven_or_hell_mode) {
        if (hell_and_hell_mode) {
            you_have_been("following the path of Hell and Hell");
        } else {
            you_have_been("following the path of Heaven or Hell");
        }
    }

    if (marathon_mode) {
        you_have_been("playing in marathon mode");
    }

    if (flags.deathdropless) {
        you_have_been("ignoring all death drops");
    }

    /* list all major conducts */

    for (cdt=FIRST_CONDUCT; cdt<=LAST_CONDUCT; cdt++) {
        if (successful_cdt(cdt)) {
            if (!superfluous_cdt(cdt)) {
                enl_msg(conducts[cdt].prefix, /* "You " */
                        conducts[cdt].presenttxt, /* "have been"    */
                        conducts[cdt].pasttxt, /* "were"    */
                        conducts[cdt].suffix, ""); /* "a pacifist"  */
            }
        } else if (intended_cdt(cdt)) {
            you_have_X(conducts[cdt].failtxt); /* "pretended to be a pacifist" */
        }
    }

    if (failed_cdt(CONDUCT_PACIFISM) || failed_cdt(CONDUCT_SADISM)) {
        if (u.uconduct.killer == 0) {
            you_have_never("killed a creature");
        } else {
            Sprintf(buf, "killed %ld creature%s", u.uconduct.killer,
                    plur(u.uconduct.killer));
            you_have_X(buf);
        }
    }

    /* now list the remaining statistical details */

    if (!u.uconduct.weaphit) {
        you_have_never("hit with a wielded weapon");
    } else if (wizard || final) {
        Sprintf(buf, "used a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        you_have_X(buf);
    }

#ifdef WIZARD
    if ((wizard || final) && u.uconduct.literate) {
        Sprintf(buf, "read items or engraved %ld time%s",
                u.uconduct.literate, plur(u.uconduct.literate));
        you_have_X(buf);
    }
    if ((wizard || final) && u.uconduct.armoruses) {
        Sprintf(buf, "put on armor %ld time%s",
                u.uconduct.armoruses, plur(u.uconduct.armoruses));
        you_have_X(buf);
    }
#endif
    if (!u.uconduct.non_racial_armor &&
        /* only show when armor was worn at all */
        u.uconduct.armoruses > 0) {
        Sprintf(buf, "wearing only %s armor", urace.adj);
        you_have_been(buf);
    }

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("genocided any monsters");
    } else {
        Sprintf(buf, "genocided %d type%s of monster%s",
                ngenocided, plur(ngenocided), plur(ngenocided));
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles) {
        you_have_never("polymorphed an object");
    }
#ifdef WIZARD
    else if (wizard || final) {
        Sprintf(buf, "polymorphed %ld item%s",
                u.uconduct.polypiles, plur(u.uconduct.polypiles));
        you_have_X(buf);
    }
#endif

    if (!u.uconduct.polyselfs) {
        you_have_never("changed form");
    }
#ifdef WIZARD
    else if (wizard || final) {
        Sprintf(buf, "changed form %ld time%s",
                u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
        you_have_X(buf);
    }
#endif

    if (!u.uconduct.wishes) {
        you_have_X("used no wishes");
    } else {
        Sprintf(buf, "used %ld wish%s",
                u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
        you_have_X(buf);

        if (u.uconduct.wishmagic) {
            Sprintf(buf, "used %ld wish%s for magical items",
                    u.uconduct.wishmagic, (u.uconduct.wishmagic > 1L) ? "es" : "");
            you_have_X(buf);
        }

        if (u.uconduct.wisharti) {
            Sprintf(buf, "used %ld wish%s for %s",
                    u.uconduct.wisharti, (u.uconduct.wisharti > 1L) ? "es" : "",
                    (u.uconduct.wisharti == 1L) ? "an artifact" : "artifacts");
            you_have_X(buf);
        }

        if (!u.uconduct.wisharti) {
            enl_msg(You_, "have not wished", "did not wish",
                    " for any artifacts", "");
        }
    }

#ifdef ELBERETH_CONDUCT
    /* no point displaying the conduct if Elbereth doesn't do anything */
    if (flags.elberethignore) {
        you_have_been("ignored by Elbereth");
    } else {
        if (u.uconduct.elbereths) {
            Sprintf(buf, "engraved Elbereth %ld time%s",
                    u.uconduct.elbereths, plur(u.uconduct.elbereths));
            you_have_X(buf);
        } else {
            you_have_never("engraved Elbereth");
        }
    }
#endif /* ELBERETH_CONDUCT */

    if (wizard || discover || final) {
        if (!flags.bones) {
            you_have_X("disabled loading of bones levels");
        } else if (!u.uconduct.bones) {
            you_have_never("encountered a bones level");
        } else {
            Sprintf(buf, "encountered %ld bones level%s",
                    u.uconduct.bones, plur(u.uconduct.bones));
            you_have_X(buf);
        }
    }

#ifdef RECORD_ACHIEVE
    if ((wizard || final) && !u.uconduct.sokoban) {
        you_have_never("used any Sokoban shortcuts");
    } else if (wizard || final) {
        Sprintf(buf, "used Sokoban shortcuts %ld time%s",
                u.uconduct.sokoban, plur(u.uconduct.sokoban));
        you_have_X(buf);
    }
#endif

    dump_list_end();
    dump("", "");

    /* Pop up the window and wait for a key */
    if (want_display) {
        display_nhwindow(en_win, TRUE);
        destroy_nhwindow(en_win);
    }
}

/* cmd.c */
