/*  SCCS Id: @(#)tutorial.c 3.4 2009/09/05  */
/*  Copyright 2009, Alex Smith        */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "quest.h"
#include "qtext.h"

const char *qt_tutorial_messages[] = {
    "qt_tutorial_welcome",
    "qt_tutorial_cursor_numpad",
    "qt_tutorial_cursor_vikeys",
    "qt_tutorial_look_tame",
    "qt_tutorial_look_hostile",
    "qt_tutorial_look_peaceful",
    "qt_tutorial_look_invisible",
    "qt_tutorial_doors",
    "qt_tutorial_corridor",
    "qt_tutorial_secretdoor",
    "qt_tutorial_poolormoat",
    "qt_tutorial_lava",
    "qt_tutorial_stairs",
    "qt_tutorial_fountain",
    "qt_tutorial_throne",
    "qt_tutorial_sink",
    "qt_tutorial_grave",
    "qt_tutorial_altar",
    "qt_tutorial_drawbridge",
    "qt_tutorial_trap",
    "qt_tutorial_l1upstairs",
    "qt_tutorial_farmove_vikeys",
    "qt_tutorial_farmove_numpad",
    "qt_tutorial_travel",
    "qt_tutorial_diagonals_vi",
    "qt_tutorial_diagonals_num",
    "qt_tutorial_repeat_vikeys",
    "qt_tutorial_repeat_numpad",
    "qt_tutorial_chooseitem",
    "qt_tutorial_massunequip",
    "qt_tutorial_callmonster",
    "qt_tutorial_multidrop",
    "qt_tutorial_massinventory",
    "qt_tutorial_secondwield",
    "qt_tutorial_look_reminder",
    "qt_tutorial_fire",
    "qt_tutorial_dgn_overview",
    "qt_tutorial_viewtutorial",
    "qt_tutorial_check_items",
    "qt_tutorial_objective",
    "qt_tutorial_saveload",
    "qt_tutorial_messagerecall",
    "qt_tutorial_spells",
    "qt_tutorial_thrownweapons",
    "qt_tutorial_projectiles",
    "qt_tutorial_elbereth",
    "qt_tutorial_melee",
    "qt_tutorial_item_gold",
    "qt_tutorial_item_weapon",
    "qt_tutorial_item_armor",
    "qt_tutorial_item_food",
    "qt_tutorial_item_scroll",
    "qt_tutorial_item_wand",
    "qt_tutorial_item_ring",
    "qt_tutorial_item_potion",
    "qt_tutorial_item_tool",
    "qt_tutorial_item_container",
    "qt_tutorial_item_amulet",
    "qt_tutorial_item_gem",
    "qt_tutorial_item_statue",
    "qt_tutorial_item_book",
    "qt_tutorial_callitem",
    "qt_tutorial_artifact",
    "qt_tutorial_randappearance",
    "qt_tutorial_levelup",
    "qt_tutorial_rankup",
    "qt_tutorial_abilup",
    "qt_tutorial_abildown",
    "qt_tutorial_dlevelchange",
    "qt_tutorial_damaged",
    "qt_tutorial_pwused",
    "qt_tutorial_pwempty",
    "qt_tutorial_acimproved",
    "qt_tutorial_gainedexp",
    "qt_tutorial_hunger",
    "qt_tutorial_satiation",
    "qt_tutorial_status",
    "qt_tutorial_enhance",
    "qt_tutorial_polyself",
    "qt_tutorial_engraving",
    "qt_tutorial_majortrouble",
    "qt_tutorial_burden",
    "qt_tutorial_equipcurse",
    "qt_tutorial_mailscroll",
    "qt_tutorial_caster_armor",
    "qt_tutorial_weapon_skill",
    "qt_tutorial_engulfed",
    "qt_tutorial_death",
    "qt_tutorial_shopentry",
    "qt_tutorial_shopbuy",
    "qt_tutorial_mines",
    "qt_tutorial_sokoban",
    "qt_tutorial_oracle",
};

/* Display a tutorial message, if it hasn't been displayed before.
   Returns TRUE if a tutorial message is output. */
boolean
check_tutorial_message(const char *msgid)
{
    if (!flags.tutorial) {
        return FALSE;
    }
    int index;
    for (index = 0; index < QT_T_MAX; index++) {
        if (!strcmp(msgid, qt_tutorial_messages[index])) {
            break;
        }
    }
    if (pl_tutorial[index]) {
        return FALSE;
    }
    pl_tutorial[index] = 1;
    flush_screen(1);
    com_pager(msgid);
    return TRUE;
}

/* Displays a tutorial message pertaining to object class oclass, if
   it hasn't been shown already. Returns 1 if the object class is one
   for which tutorial messages exist, regardless of whether the
   message is shown or not. This assumes that the object classes haven't
   been customized too heavily (possibly a custom boulder, and that's it). */
int
check_tutorial_oclass(int oclass)
{
    switch (oclass) {
        case WEAPON_CLASS:
            check_tutorial_message("qt_tutorial_item_weapon");
            return 1;
        case FOOD_CLASS:
            check_tutorial_message("qt_tutorial_item_food");
            return 1;
        case GEM_CLASS:
            check_tutorial_message("qt_tutorial_item_gem");
            return 1;
        case TOOL_CLASS:
            check_tutorial_message("qt_tutorial_item_tool");
            return 1;
        case AMULET_CLASS:
            check_tutorial_message("qt_tutorial_item_amulet");
            return 1;
        case POTION_CLASS:
            check_tutorial_message("qt_tutorial_item_potion");
            return 1;
        case SCROLL_CLASS:
            check_tutorial_message("qt_tutorial_item_scroll");
            return 1;
        case SPBOOK_CLASS:
            check_tutorial_message("qt_tutorial_item_book");
            return 1;
        case ARMOR_CLASS:
            check_tutorial_message("qt_tutorial_item_armor");
            return 1;
        case WAND_CLASS:
            check_tutorial_message("qt_tutorial_item_wand");
            return 1;
        case RING_CLASS:
            check_tutorial_message("qt_tutorial_item_ring");
            return 1;
        case ROCK_CLASS:
            check_tutorial_message("qt_tutorial_item_statue");
            return 1;
        case COIN_CLASS:
            check_tutorial_message("qt_tutorial_item_gold");
            return 1;
        default:
            return 0;
            /* venom/ball/chain/mimic don't concern us */
    }
}

/* Displays a tutorial message pertaining to the location at (lx, ly),
   if there is a message and it hasn't been shown already. Returns TRUE
   if a message is shown. */
int
check_tutorial_location(coordxy lx, coordxy ly, boolean from_farlook)
{
    struct rm *l = &(levl[lx][ly]);
    if (!flags.tutorial) {
        return FALSE; /* short-circuit */
    }
    if (glyph_is_trap(l->glyph)) { /* seen traps only */
        if (check_tutorial_message("qt_tutorial_trap")) {
            return TRUE;
        }
    }
    if (IS_DOOR(l->typ) && l->doormask >= D_ISOPEN) {
        if (check_tutorial_message("qt_tutorial_doors")) {
            return TRUE;
        }
    }
    if (l->typ == CORR) {
        if (check_tutorial_message("qt_tutorial_corridor")) {
            return TRUE;
        }
    }
    /* A freebie: we give away the location of a secret door or
       corridor, once. This is so that the advice to search will
       always end up coming good, to avoid confusing new players;
       it also deals with the horrific possibility of a player's
       first game having no visible exits from the first room (it
       can happen!) */
    if (l->typ == SCORR || l->typ == SDOOR) {
        if (check_tutorial_message("qt_tutorial_secretdoor")) {
            return TRUE;
        }
    }
    if (l->typ == POOL || l->typ == MOAT) {
        if (check_tutorial_message("qt_tutorial_poolormoat")) {
            return TRUE;
        }
    }
    if (l->typ == LAVAPOOL) {
        if (check_tutorial_message("qt_tutorial_lava")) {
            return TRUE;
        }
    }
    if (l->typ == STAIRS) {
        stairway *stway = stairway_at(lx, ly);
        /* In which direction? */
        if (stway && stway->up) {
            if (u.uz.dlevel > 1) {
                if (check_tutorial_message("qt_tutorial_stairs")) {
                    return TRUE;
                } else if (from_farlook) {
                    if (check_tutorial_message("qt_tutorial_l1upstairs")) {
                        return TRUE;
                    }
                }
            }
        } else {
            if (check_tutorial_message("qt_tutorial_stairs")) {
                return TRUE;
            }
        }
    }
    if (l->typ == FOUNTAIN) {
        if (check_tutorial_message("qt_tutorial_fountain")) {
            return TRUE;
        }
    }
    if (l->typ == THRONE) {
        if (check_tutorial_message("qt_tutorial_throne")) {
            return TRUE;
        }
    }
    if (l->typ == SINK) {
        if (check_tutorial_message("qt_tutorial_sink")) {
            return TRUE;
        }
    }
    if (l->typ == GRAVE) {
        if (check_tutorial_message("qt_tutorial_grave")) {
            return TRUE;
        }
    }
    if (l->typ == ALTAR) {
        if (check_tutorial_message("qt_tutorial_altar")) {
            return TRUE;
        }
    }
    if (IS_DRAWBRIDGE(l->typ)) {
        if (check_tutorial_message("qt_tutorial_drawbridge")) {
            return TRUE;
        }
    }
    return FALSE;
}
/* Display tutorial messages that may result from farlook data. */
void
check_tutorial_farlook(coordxy x, coordxy y)
{
    int glyph = glyph_at(x, y);
    /* Monsters */
    if (!flags.tutorial) {
        return; /* short-circuit */
    }
    if (glyph_is_invisible(glyph)) {
        check_tutorial_message("qt_tutorial_look_invisible");
        return;
    }
    if (glyph_is_pet(glyph) || glyph_is_ridden_monster(glyph)) {
        check_tutorial_message("qt_tutorial_look_tame");
        return;
    }
    if (glyph_is_monster(glyph)) {
        if (x == u.ux && y == u.uy) {
            return; /* you aren't hostile */
        }
        if (!MON_AT(x, y)) {
            return; /* to prevent null pointer deref */
        }
        if (m_at(x, y)->mpeaceful) {
            check_tutorial_message("qt_tutorial_look_peaceful");
        } else if (m_at(x, y)->mtame) { /* without showpet on */
            check_tutorial_message("qt_tutorial_look_tame");
        } else {
            check_tutorial_message("qt_tutorial_look_hostile");
        }
        return;
    }
    /* Items */
    if (glyph_is_object(glyph)) {
        check_tutorial_oclass(objects[glyph_to_obj(glyph)].oc_class);
        return;
    }
    /* Terrain */
    if (glyph_is_cmap(glyph) || glyph_is_trap(glyph)) {
        check_tutorial_location(x, y, TRUE);
        return;
    }
}

#define CHECK_TUTORIAL_COMMAND_BUFSIZE 100
static const char *check_tutorial_command_message = NULL;
static char check_tutorial_command_buffer[CHECK_TUTORIAL_COMMAND_BUFSIZE];
static int check_tutorial_command_pointer = 0;
static int check_tutorial_command_count = 0;
void
check_tutorial_command(char c)
{
    int i, r;
    char lc;
    boolean travel = TRUE;
    boolean farmove = TRUE;
    boolean repeat = TRUE;
    boolean massunequip = TRUE;
    boolean look_reminder = TRUE;
    int secondwield = 0;
    if (c == 'm') {
        if (u.dx && u.dy) {
            c = 'y';
        } else {
            c = 'b';
        }
    }
    check_tutorial_command_buffer[check_tutorial_command_pointer] = c;
    i = check_tutorial_command_pointer;
    check_tutorial_command_pointer++;
    check_tutorial_command_pointer %= CHECK_TUTORIAL_COMMAND_BUFSIZE;
    check_tutorial_command_count++;
    check_tutorial_command_message = 0;
    r = 0;
    lc = c;
    do
    {
        c = check_tutorial_command_buffer[i];
        if (lc != c) {
            repeat = FALSE;
        }
        if (c != 'y' && c != 'b' && c != 'G') {
            travel = FALSE;
        }
        if (c != 'y' && c != 'b') {
            farmove = FALSE;
        }
        if (c != 'R' && c != 'T') {
            massunequip = FALSE;
        }
        if (c == ';') {
            look_reminder = FALSE;
        }
        if (c == 'w') {
            secondwield++;
        }
        r++;
        if (r > check_tutorial_command_count) {
            break;
        }
        if (moves > 125 && r > 5 && farmove) {
            if (iflags.num_pad) {
                check_tutorial_command_message = "qt_tutorial_farmove_numpad";
            } else {
                check_tutorial_command_message = "qt_tutorial_farmove_vikeys";
            }
            break;
        }
        if (moves > 125 && r > 30 && travel) {
            check_tutorial_command_message = "qt_tutorial_travel";
            break;
        }
        if (moves > 80 && r > 20 && c == 'b') {
            if (iflags.num_pad) {
                check_tutorial_command_message = "qt_tutorial_diagonals_num";
            } else {
                check_tutorial_command_message = "qt_tutorial_diagonals_vi";
            }
            break;
        }
        if (repeat && r > 5 && c == 's') {
            if (iflags.num_pad) {
                check_tutorial_command_message = "qt_tutorial_repeat_numpad";
            } else {
                check_tutorial_command_message = "qt_tutorial_repeat_vikeys";
            }
            break;
        }
        if (moves > 45 && r >= 2 && massunequip) {
            check_tutorial_command_message = "qt_tutorial_massunequip";
            break;
        }
        if (moves > 45 && r >= 2 && repeat && c == 'd') {
            check_tutorial_command_message = "qt_tutorial_multidrop";
            break;
        }
        if (moves > 45 && r >= 2 && repeat && c == 'I') {
            check_tutorial_command_message = "qt_tutorial_massinventory";
            break;
        }
        if (moves > 45 && secondwield >= 3 && r == 50) {
            check_tutorial_command_message = "qt_tutorial_secondwield";
            break;
        }
        if (r >= 3 && repeat && c == 't') {
            check_tutorial_command_message = "qt_tutorial_fire";
            break;
        }
        i--;
        if (i == -1) {
            i = CHECK_TUTORIAL_COMMAND_BUFSIZE - 1;
        }
        lc = c;
    } while (r < 100);

    if (check_tutorial_command_message == 0 && look_reminder &&
        check_tutorial_command_count >= 100)
        check_tutorial_command_message = "qt_tutorial_look_reminder";
}

/* Display tutorial messages based on the state of the character. */
void
maybe_tutorial(void)
{
    /* So that we don't get two different combats if a monster disappears */
    static int time_since_combat = 0;
    /* Don't give messages on the first combat of any reload-from-save, to
       avoid overloading the player with tutorial messages */
    static int firstcombat = 1;
    int old_time_since_combat;
    int i;
    struct monst *mtmp;
    static struct attribs oldattribs = {{0, 0, 0, 0, 0, 0}};
    static int oldac = 300; /* an impossible value */

    if (!flags.tutorial) {
        return; /* short-circuit */
    }

    /* Check to see if any tutorial triggers have occured.
       Stop checking once one message is output. */

    /* Welcome message: show immediately */
    if (check_tutorial_message("qt_tutorial_welcome")) {
        return;
    }

    /* Terrain checks; these only occur if at least 3 turns have elapsed,
       to avoid overwhelming the player early on. */
    if (moves > 3) {
        int dx, dy;
        int monsterglyph = -1;
        for (dx = -1; dx <= 1; dx++) {
            for (dy = -1; dy <= 1; dy++) {
                if (isok(u.ux+dx, u.uy+dy)) {
                    int lx = u.ux+dx;
                    int ly = u.uy+dy;
                    /* Terrain checks */
                    if (check_tutorial_location(lx, ly, FALSE)) {
                        return;
                    }
                    /* Some non-terrain checks in the same loop */
                    if (glyph_is_invisible(glyph_at(lx, ly))) {
                        if (check_tutorial_message("qt_tutorial_look_invisible")) {
                            return;
                        }
                    }
                    if (glyph_is_monster(glyph_at(lx, ly))) {
                        if (monsterglyph == glyph_at(lx, ly)) {
                            if (check_tutorial_message("qt_tutorial_callmonster")) {
                                return;
                            }
                        }
                        monsterglyph = glyph_at(lx, ly);
                    }
                }
            }
        }
    }
    /* Check to see if we're in combat. */
    ++time_since_combat;
    old_time_since_combat = time_since_combat;
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (!DEADMONSTER(mtmp) && cansee(mtmp->mx, mtmp->my) && !mtmp->mtame) {
            time_since_combat = 0;
        }
    }
    /* Ambient messages that only come up during combat, and only one
       message per combat */
    if (!time_since_combat && old_time_since_combat > 5) {
        if (!firstcombat) {
            if (u.uz.dlevel >= 3) {
                if (check_tutorial_message("qt_tutorial_elbereth")) {
                    return;
                }
            }
            for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
                if (spellid(i) == SPE_FORCE_BOLT) {
                    if (check_tutorial_message("qt_tutorial_spells")) {
                        return;
                    }
                }
            }
            if (check_tutorial_message("qt_tutorial_melee")) {
                return;
            }
        } else {
            old_time_since_combat = 0;
            firstcombat = 0;
        }
    }
    /* Events. */
    if (spellid(1)) {
        if ((uarm && is_metallic(uarm)) ||
            (uarms && !Role_if(PM_PRIEST)) || /* priests start with shields */
            (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE) ||
            (uarmg && is_metallic(uarmg)) ||
            (uarmf && is_metallic(uarmf)))
            if (check_tutorial_message("qt_tutorial_caster_armor")) {
                return;
            }
    }
    if (uwep && uwep != &zeroobj && weapon_dam_bonus(uwep) < 0 && !u.twoweap) {
        if (check_tutorial_message("qt_tutorial_weapon_skill")) {
            return;
        }
    }
    if (u.ulevel >= 2) {
        if (check_tutorial_message("qt_tutorial_levelup")) {
            return;
        }
    }
    if (u.ulevel >= 3) {
        if (check_tutorial_message("qt_tutorial_rankup")) {
            return;
        }
    }
    int incdec = 0;
    boolean firstturn = oldattribs.a[0] == 0;
    for (i = 0; i < A_MAX; i++) {
        if (u.acurr.a[i] > oldattribs.a[i]) {
            incdec = 1;
        }
        if (u.acurr.a[i] < oldattribs.a[i]) {
            incdec = -1;
        }
        oldattribs.a[i] = u.acurr.a[i];
    }
    if (!firstturn) {
        if (incdec > 0) {
            if (check_tutorial_message("qt_tutorial_abilup")) {
                return;
            }
        }
        if (incdec < 0) {
            if (check_tutorial_message("qt_tutorial_abildown")) {
                return;
            }
        }
    }
    if (u.uac < oldac && oldac != 300) {
        if (check_tutorial_message("qt_tutorial_acimproved")) {
            return;
        }
    }
    oldac = u.uac;
    if (Confusion || Sick || Blind || Stunned || Hallucination || Slimed) {
        if (check_tutorial_message("qt_tutorial_status")) {
            return;
        }
    }
    if (u.uz.dlevel >= 2) {
        if (check_tutorial_message("qt_tutorial_dlevelchange")) {
            return;
        }
    }
    if (u.uz.dlevel >= 10) {
        if (check_tutorial_message("qt_tutorial_dgn_overview")) {
            return;
        }
    }
    if (u.uhp < u.uhpmax) {
        if (check_tutorial_message("qt_tutorial_damaged")) {
            return;
        }
    }
    if (u.uen < u.uenmax) {
        if (check_tutorial_message("qt_tutorial_pwused")) {
            return;
        }
    }
    if (u.uen < 5 && u.uenmax > 10) {
        if (check_tutorial_message("qt_tutorial_pwempty")) {
            return;
        }
    }
    if (u.umonster != u.umonnum) {
        if (check_tutorial_message("qt_tutorial_polyself")) {
            return;
        }
    }
    if (u.uexp > 0) {
        if (check_tutorial_message("qt_tutorial_gainedexp")) {
            return;
        }
    }
    if (u.uhs >= HUNGRY) {
        if (check_tutorial_message("qt_tutorial_hunger")) {
            return;
        }
    }
    if (u.uhs <= SATIATED) {
        if (check_tutorial_message("qt_tutorial_satiation")) {
            return;
        }
    }
    if (can_advance_something()) {
        if (check_tutorial_message("qt_tutorial_enhance")) {
            return;
        }
    }
    if (u.uswallow) {
        if (check_tutorial_message("qt_tutorial_engulfed")) {
            return;
        }
    }
    if (near_capacity() > UNENCUMBERED) {
        if (check_tutorial_message("qt_tutorial_burden")) {
            return;
        }
    }
    if (in_trouble() > 0 && can_pray(0) && !IS_ALTAR(levl[u.ux][u.uy].typ)) {
        if (check_tutorial_message("qt_tutorial_majortrouble")) {
            return;
        }
    }
    if (inside_shop(u.ux, u.uy)) {
        if (check_tutorial_message("qt_tutorial_shopentry")) {
            return;
        }
    }
    if (u.uz.dnum == mines_dnum) {
        if (check_tutorial_message("qt_tutorial_mines")) {
            return;
        }
    }
    if (u.uz.dnum == sokoban_dnum) {
        if (check_tutorial_message("qt_tutorial_sokoban")) {
            return;
        }
    }
    if (check_tutorial_command_message && !strcmp(check_tutorial_command_message, "qt_tutorial_fire")) {
        if (check_tutorial_message("qt_tutorial_fire")) {
            return;
        }
    }
    /* Item-dependent events. */
    {
        int projectile_groups = 0;
        int launcher_groups = 0;
        struct obj *otmp;
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->bknown && otmp->cursed) {
                if (check_tutorial_message("qt_tutorial_equipcurse")) {
                    return;
                }
            }
#ifdef MAIL
            if (otmp->otyp == SCR_MAIL) {
                if (check_tutorial_message("qt_tutorial_mailscroll")) {
                    return;
                }
            }
#endif
            if (otmp->oartifact) {
                if (check_tutorial_message("qt_tutorial_artifact")) {
                    return;
                }
            }
            if (otmp->unpaid) {
                if (check_tutorial_message("qt_tutorial_shopbuy")) {
                    return;
                }
            }
            if (!objects[otmp->otyp].oc_name_known) {
                switch (objects[otmp->otyp].oc_class) {
                case POTION_CLASS:
                    if (otmp->otyp == POT_WATER) {
                        break;
                    }
                /* fall through */
                case SCROLL_CLASS:
                    if (otmp->otyp == SCR_BLANK_PAPER) {
                        break;
                    }
#ifdef MAIL
                    if (otmp->otyp == SCR_MAIL) {
                        break;
                    }
#endif
                /* fall through */
                case WAND_CLASS:
                case SPBOOK_CLASS:
                    if (otmp->otyp == SPE_BLANK_PAPER) {
                        break;
                    }
                    if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
                        break;
                    }
                /* fall through */
                case RING_CLASS:
                case AMULET_CLASS:
                    if (check_tutorial_message("qt_tutorial_randappearance")) {
                        return;
                    }
                    break;
                default: break;
                }
            }
            /* Containers; minor spoiler here, in that it doesn't trigger
               off a bag of tricks and a savvy player might notice that, but
               that's not a freebie I'm worried about. */
            switch (otmp->otyp) {
            case SACK:
                /* starting inventory for arcs/rogues, and we don't want to
                   give the message until a second container's listed */
                if (Role_if(PM_ARCHEOLOGIST)) {
                    break;
                }
                if (Role_if(PM_ROGUE)) {
                    break;
                }
                /* fall through */
            case LARGE_BOX:
            case CHEST:
            case ICE_BOX:
            case OILSKIN_SACK:
            case BAG_OF_HOLDING:
                if (check_tutorial_message("qt_tutorial_item_container")) {
                    return;
                }
                break;
            default: break;
            }
            /* Requiring a specific item during combat... */
            if (!time_since_combat && old_time_since_combat > 5) {
                switch (otmp->otyp) {
                /* Projectiles. */
                case ARROW:
                case ELVEN_ARROW:
                case ORCISH_ARROW:
                case SILVER_ARROW:
                case YA:
                    projectile_groups |= 0x1;
                    break;
                case CROSSBOW_BOLT:
                    projectile_groups |= 0x2;
                    break;
                case FLINT:
                case ROCK:
                    projectile_groups |= 0x4;
                    break;
                /* Launchers. */
                case BOW:
                case ELVEN_BOW:
                case ORCISH_BOW:
                case YUMI:
                    launcher_groups |= 0x1;
                    break;
                case CROSSBOW:
                    launcher_groups |= 0x2;
                    break;
                case SLING:
                    launcher_groups |= 0x4;
                    break;
                /* Thrown weapons. Don't count our wielded weapon in this. */
                case DART:
                case SHURIKEN:
                case BOOMERANG:
                case SPEAR:
                case ELVEN_SPEAR:
                case ORCISH_SPEAR:
                case DWARVISH_SPEAR:
                case JAVELIN:
                case DAGGER:
                case ELVEN_DAGGER:
                case ORCISH_DAGGER:
                case SILVER_DAGGER:
                    if (otmp == uwep) {
                        break;
                    }
                    if (check_tutorial_message("qt_tutorial_thrownweapons")) {
                        return;
                    }
                    break;
                default: break;
                }
            }
        }
        if (projectile_groups & launcher_groups) {
            if (check_tutorial_message("qt_tutorial_projectiles")) {
                return;
            }
        }
    }
    /* Items on the current square. */
    {
        struct obj *otmp;
        for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
            switch (otmp->otyp) {
            case LARGE_BOX:
            case CHEST:
            case ICE_BOX:
                if (check_tutorial_message("qt_tutorial_item_container")) {
                    return;
                }
                break;
            default: break;
            }
        }
    }
    /* Ambient messages that only come up outside combat */
    if (time_since_combat > 5) {
        if (check_tutorial_command_message) {
            if (check_tutorial_message(check_tutorial_command_message)) {
                return;
            }
        }
        if (moves >= 10) {
            if (check_tutorial_message("qt_tutorial_viewtutorial")) {
                return;
            }
        }
        if (moves >= 30) {
            if (check_tutorial_message("qt_tutorial_check_items")) {
                return;
            }
        }
        if (moves >= 60) {
            if (check_tutorial_message("qt_tutorial_objective")) {
                return;
            }
        }
        if (moves >= 100) {
            if (check_tutorial_message("qt_tutorial_saveload")) {
                return;
            }
        }
        if (moves >= 150) {
            if (check_tutorial_message("qt_tutorial_messagerecall")) {
                return;
            }
        }
    }
}

/* Redisplay tutorial messages. */
int
tutorial_redisplay(void)
{
    winid tempwin = create_nhwindow(NHW_MENU);
    anything i;
    int c;
    menu_item *s;
    start_menu(tempwin);
    for (i.a_int = 0; i.a_int < SIZE(pl_tutorial); i.a_int++) {
        if (pl_tutorial[i.a_int] > 0) {
            char *name = qt_com_firstline(qt_tutorial_messages[i.a_int]);
            const char *menu_entry;
            /* adding 10 to namebuf removes the 'Tutorial: ' at the start */
            menu_entry = name ? name + 10 : "(not found)";
            i.a_int++;
            add_menu(tempwin, NO_GLYPH, MENU_DEFCNT, &i, 0, 0, ATR_NONE, menu_entry, FALSE);
            free(name);
        }
    }
    end_menu(tempwin, "Which tutorial?");
    c = select_menu(tempwin, PICK_ONE, &s);
    if (c <= 0) {
        destroy_nhwindow(tempwin);
        return 0;
    }
    i.a_int = s[0].item.a_int;
    free((genericptr_t)s);
    flush_screen(1);
    com_pager(qt_tutorial_messages[i.a_int - 1]);
    destroy_nhwindow(tempwin);
    return 0;
}

/* tutorial.c */
