/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* shknam.c -- initialize a shop */

#include "hack.h"

static boolean stock_room_goodpos(struct mkroom *, int, int, int, int);
static void nameshk(struct monst *, const char * const *);
static int  shkinit(const struct shclass *, struct mkroom *);
#ifdef BLACKMARKET
static void stock_blkmar(const struct shclass *, struct mkroom *, int);
#endif /* BLACKMARKET */

/*
 *  Name prefix codes:
 *      dash          -  female, personal name
 *      underscore    _  female, general name
 *      plus          +  male, personal name
 *      vertical bar  |  male, general name (implied for most of shktools)
 *      equals        =  gender not specified, personal name
 *
 *  Personal names do not receive the honorific prefix "Mr." or "Ms.".
 */

static const char * const shkliquors[] = {
    /* Ukraine */
    "Njezjin", "Tsjernigof", "Ossipewsk", "Gorlowka",
    /* Belarus */
    "Gomel",
    /* N. Russia */
    "Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja",
    "Narodnaja", "Kyzyl",
    /* Silezie */
    "Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice",
    "Brzeg", "Krnov", "Hradec Kralove",
    /* Schweiz */
    "Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm",
    "Flims", "Vals", "Scuol",
    "Chur", "Pagig",
    0
};

static const char * const shkbooks[] = {
    /* Eire */
    "Skibbereen", "Kanturk", "Rath Luirc", "Ennistymon", "Lahinch",
    "Loughrea", "Croagh", "Mastergeehy", "Ballyjamesduff",
    "Kinnegad", "Lugnaquillia", "Enniscorthy", "Gweebarra",
    "Kittamagh", "Nenagh", "Sneem", "Ballingeary", "Kilgarvan",
    "Cahersiveen", "Glenbeigh", "Kilmihil", "Kiltamagh",
    "Droichead Atha", "Inniscrone", "Clonegal", "Lisnaskea",
    "Culdaff", "Dunfanaghy", "Inishbofin", "Kesh",
    0
};

static const char * const shkarmors[] = {
    /* Turquie */
    "Demirci", "Kalecik", "Boyabai", "Yildizeli", "Gaziantep",
    "Siirt", "Akhalataki", "Tirebolu", "Aksaray", "Ermenak",
    "Iskenderun", "Kadirli", "Siverek", "Pervari", "Malasgirt",
    "Bayburt", "Ayancik", "Zonguldak", "Balya", "Tefenni",
    "Artvin", "Kars", "Makharadze", "Malazgirt", "Midyat",
    "Birecik", "Kirikkale", "Alaca", "Polatli", "Nallihan",
    0
};

static const char * const shkwands[] = {
    /* Wales */
    "Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach",
    "Rhaeader", "Llandrindod", "Llanfair-ym-muallt",
    "Y-Fenni", "Maesteg", "Rhydaman", "Beddgelert",
    "Curig", "Llanrwst", "Llanerchymedd", "Caergybi",
    /* Scotland */
    "Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar",
    "Kerloch", "Beinn a Ghlo", "Drumnadrochit", "Morven",
    "Uist", "Storr", "Sgurr na Ciche", "Cannich", "Gairloch",
    "Kyleakin", "Dunvegan",
    0
};

static const char * const shkrings[] = {
    /* Hollandse familienamen */
    "Feyfer", "Flugi", "Gheel", "Havic", "Haynin", "Hoboken",
    "Imbyze", "Juyn", "Kinsky", "Massis", "Matray", "Moy",
    "Olycan", "Sadelin", "Svaving", "Tapper", "Terwen", "Wirix",
    "Ypey",
    /* Skandinaviske navne */
    "Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",
    "Enontekis", "Rovaniemi", "Avasaksa", "Haparanda",
    "Lulea", "Gellivare", "Oeloe", "Kajaani", "Fauske",
    0
};

static const char * const shkfoods[] = {
    /* Indonesia */
    "Djasinga", "Tjibarusa", "Tjiwidej", "Pengalengan",
    "Bandjar", "Parbalingga", "Bojolali", "Sarangan",
    "Ngebel", "Djombang", "Ardjawinangun", "Berbek",
    "Papar", "Baliga", "Tjisolok", "Siboga", "Banjoewangi",
    "Trenggalek", "Karangkobar", "Njalindoeng", "Pasawahan",
    "Pameunpeuk", "Patjitan", "Kediri", "Pemboeang", "Tringanoe",
    "Makin", "Tipor", "Semai", "Berhala", "Tegal", "Samoe",
    0
};

static const char * const shkweapons[] = {
    /* Perigord */
    "Voulgezac", "Rouffiac", "Lerignac", "Touverac", "Guizengeard",
    "Melac", "Neuvicq", "Vanzac", "Picq", "Urignac", "Corignac",
    "Fleac", "Lonzac", "Vergt", "Queyssac", "Liorac", "Echourgnac",
    "Cazelon", "Eypau", "Carignan", "Monbazillac", "Jonzac",
    "Pons", "Jumilhac", "Fenouilledes", "Laguiolet", "Saujon",
    "Eymoutiers", "Eygurande", "Eauze", "Labouheyre",
    0
};

static const char * const shktools[] = {
    /* Spmi */
    "Ymla", "Eed-morra", "Cubask", "Nieb", "Bnowr Falr", "Telloc Cyaj",
    "Sperc", "Noskcirdneh", "Yawolloh", "Hyeghu", "Niskal", "Trahnil",
    "Htargcm", "Enrobwem", "Kachzi Rellim", "Regien", "Donmyar",
    "Yelpur", "Nosnehpets", "Stewe", "Renrut", "_Zlaw", "Nosalnef",
    "Rewuorb", "Rellenk", "Yad", "Cire Htims", "Y-crad", "Nenilukah",
    "Corsh", "Aned",
#ifdef OVERLAY
    "Erreip", "Nehpets", "Mron", "Snivek", "Lapu", "Kahztiy",
#endif
#ifdef WIN32
    "Lechaim", "Lexa", "Niod",
#endif
#ifdef MAC
    "Nhoj-lee", "Evad\'kh", "Ettaw-noj", "Tsew-mot", "Ydna-s",
    "Yao-hang", "Tonbar", "Kivenhoug",
#endif
#ifdef AMIGA
    "Falo", "Nosid-da\'r", "Ekim-p", "Rebrol-nek", "Noslo", "Yl-rednow",
    "Mured-oog", "Ivrajimsal",
#endif
#ifdef TOS
    "Nivram",
#endif
#ifdef VMS
    "Lez-tneg", "Ytnu-haled", "Niknar",
#endif
    0
};

static const char * const shklight[] = {
    /* Romania */
    "Zarnesti", "Slanic", "Nehoiasu", "Ludus", "Sighisoara", "Nisipitu",
    "Razboieni", "Bicaz", "Dorohoi", "Vaslui", "Fetesti", "Tirgu Neamt",
    "Babadag", "Zimnicea", "Zlatna", "Jiu", "Eforie", "Mamaia",
    /* Bulgaria */
    "Silistra", "Tulovo", "Panagyuritshte", "Smolyan", "Kirklareli",
    "Pernik", "Lom", "Haskovo", "Dobrinishte", "Varvara", "Oryahovo",
    "Troyan", "Lovech", "Sliven",
    0
};

static const char * const shkgeneral[] = {
    /* Suriname */
    "Hebiwerie", "Possogroenoe", "Asidonhopo", "Manlobbi",
    "Adjama", "Pakka Pakka", "Kabalebo", "Wonotobo",
    "Akalapi", "Sipaliwini",
    /* Greenland */
    "Annootok", "Upernavik", "Angmagssalik",
    /* N. Canada */
    "Aklavik", "Inuvik", "Tuktoyaktuk",
    "Chicoutimi", "Ouiatchouane", "Chibougamau",
    "Matagami", "Kipawa", "Kinojevis",
    "Abitibi", "Maganasipi",
    /* Iceland */
    "Akureyri", "Kopasker", "Budereyri", "Akranes", "Bordeyri",
    "Holmavik",
    0
};

static const char *shkmusic[] = {
    "John", "Paul", "George", "Ringo",
    "Elvis", "Mick", "Keith", "Ron", "Charlie",
    "Joseph", "Franz", "Richard", "Ludwig", "Wolfgang Amadeus",
    "Johann Sebastian",
    "Karlheinz", "Gyorgy",
    "Luciano", "Placido", "Jose", "Enrico",
    "Falco", "_Britney", "_Christina", "_Toni", "_Brandy",
    0
};

static const char *shkpet[] = {
    /* Albania */
    "Elbasan", "Vlore", "Shkoder", "Berat", "Kavaje", "Pogradec",
    "Sarande", "Peshkopi", "Shijak", "Librazhd", "Tepelene",
    "Fushe-Kruje", "Rreshen",
    0
};

static const char *shktins[] = {
    /* Sweden */
    "Trosa", "Torshalla", "Morgongava", "Uppsala", "Norrkoping",
    "Nybro", "Alingsas", "Vadstena", "Fagersta", "Skelleftea",
    "Solleftea", "Ystad", "Avesta", "Sala", "Norrtalje",
    0
};

/*
 * To add new shop types, all that is necessary is to edit the shtypes[] array.
 * See mkroom.h for the structure definition.  Typically, you'll have to lower
 * some or all of the probability fields in old entries to free up some
 * percentage for the new type.
 *
 * The placement type field is not yet used but will be in the near future.
 *
 * The iprobs array in each entry defines the probabilities for various kinds
 * of objects to be present in the given shop type.  You can associate with
 * each percentage either a generic object type (represented by one of the
 * *_CLASS macros) or a specific object (represented by an onames.h define).
 * In the latter case, prepend it with a unary minus so the code can know
 * (by testing the sign) whether to use mkobj() or mksobj().
 */

const struct shclass shtypes[] = {
    {"general store", RANDOM_CLASS, 41,
     D_SHOP, {{100, RANDOM_CLASS}, {0, 0}, {0, 0}}, shkgeneral},
    {"used armor dealership", ARMOR_CLASS, 14,
     D_SHOP, {{90, ARMOR_CLASS}, {10, WEAPON_CLASS}, {0, 0}},
     shkarmors},
    {"second-hand bookstore", SCROLL_CLASS, 10, D_SHOP,
     {{90, SCROLL_CLASS}, {10, SPBOOK_CLASS}, {0, 0}}, shkbooks},
    {"liquor emporium", POTION_CLASS, 10, D_SHOP,
     {{100, POTION_CLASS}, {0, 0}, {0, 0}}, shkliquors},
    {"antique weapons outlet", WEAPON_CLASS, 5, D_SHOP,
     {{90, WEAPON_CLASS}, {10, ARMOR_CLASS}, {0, 0}}, shkweapons},
    {"delicatessen", FOOD_CLASS, 5, D_SHOP,
     {{83, FOOD_CLASS}, {5, -POT_FRUIT_JUICE}, {4, -POT_BOOZE},
      {5, -POT_WATER}, {3, -ICE_BOX}}, shkfoods},
    {"jewelers", RING_CLASS, 3, D_SHOP,
     {{85, RING_CLASS}, {10, GEM_CLASS}, {5, AMULET_CLASS}, {0, 0}},
     shkrings},
    {"quality apparel and accessories", WAND_CLASS, 3, D_SHOP,
     {{90, WAND_CLASS}, {5, -LEATHER_GLOVES}, {5, -ELVEN_CLOAK}, {0, 0}},
     shkwands},
    {"hardware store", TOOL_CLASS, 3, D_SHOP,
     {{100, TOOL_CLASS}, {0, 0}, {0, 0}}, shktools},
    /* Actually shktools is ignored; the code specifically chooses a
     * random implementor name (along with candle shops having
     * random shopkeepers)
     */
    {"rare books", SPBOOK_CLASS, 3, D_SHOP,
     {{90, SPBOOK_CLASS}, {10, SCROLL_CLASS}, {0, 0}}, shkbooks},
    {"canned food factory", FOOD_CLASS, 1, D_SHOP,
     {{10, -ICE_BOX}, {90, -TIN},
      /* shopkeeper will pay for corpses, but they aren't generated */
      /* on the shop floor */
      {0, -CORPSE}, {0, 0}}, shktins},
    {"rare instruments", TOOL_CLASS, 1, D_SHOP,
     {{10, -TIN_WHISTLE  }, { 3, -MAGIC_WHISTLE  },
      {10, -WOODEN_FLUTE }, { 3, -MAGIC_FLUTE    },
      {10, -TOOLED_HORN  }, { 3, -FROST_HORN },
      { 3, -FIRE_HORN    }, { 3, -HORN_OF_PLENTY },
      {10, -WOODEN_HARP  }, { 3, -MAGIC_HARP },
      {10, -BELL     }, {10, -BUGLE      },
      {10, -LEATHER_DRUM }, { 2, -DRUM_OF_EARTHQUAKE},
      { 5, -T_SHIRT  }, { 5, -LOCK_PICK  },
      {0, 0}}, shkmusic},
    {"pet store", FOOD_CLASS, 1, D_SHOP, {
         {67, -FIGURINE}, {5, -LEASH}, {10, -TRIPE_RATION}, {5, -SADDLE},
         {10, -TIN_WHISTLE}, {3, -MAGIC_WHISTLE}
     }, shkpet},
    /* Shops below this point are "unique".  That is they must all have a
     * probability of zero.  They are only created via the special level
     * loader.
     */
    {"lighting store", TOOL_CLASS, 0, D_SHOP,
     {{31, -WAX_CANDLE}, {49, -TALLOW_CANDLE},
      {5, -BRASS_LANTERN}, {10, -OIL_LAMP}, {1, -WAN_LIGHT}, {1, -WAN_LIGHTNING}, {3, -MAGIC_LAMP}}, shklight},
#ifdef BLACKMARKET
    {"black market", RANDOM_CLASS, 0, D_SHOP,
     {{100, RANDOM_CLASS}, {0, 0}, {0, 0}}, 0},
#endif /* BLACKMARKET */
    {(char *)0, 0, 0, 0, {{0, 0}, {0, 0}, {0, 0}}, 0}
};

/* validate shop probabilities; otherwise incorrect local changes could
   end up provoking infinite loops or wild subscripts fetching garbage */
void
shop_selection_init()
{
    register int i, j, item_prob, shop_prob;

    for (shop_prob = 0, i = 0; i < SIZE(shtypes)-1; i++) {
        shop_prob += shtypes[i].prob;
        for (item_prob = 0, j = 0; j < SIZE(shtypes[0].iprobs); j++)
            item_prob += shtypes[i].iprobs[j].iprob;
        if (item_prob != 100)
            panic("item probabilities total to %d for %s shops!",
                  item_prob, shtypes[i].name);
    }
    if (shop_prob != 100)
        panic("shop probabilities total to %d!", shop_prob);
}

/* make an object of the appropriate type for a shop square */
static void
mkshobj_at(const struct shclass *shp, int sx, int sy, int color)
{
    struct monst *mtmp;
    int atype;
    struct permonst *ptr;

    if (!Is_blackmarket(&u.uz) && rn2(100) < depth(&u.uz) &&
        !MON_AT(sx, sy) && (ptr = mkclass(S_MIMIC, 0)) &&
        (mtmp = makemon(ptr, sx, sy, NO_MM_FLAGS)) != 0) {
        /* note: makemon will set the mimic symbol to a shop item */
        if (rn2(10) >= depth(&u.uz)) {
            mtmp->m_ap_type = M_AP_OBJECT;
            mtmp->mappearance = STRANGE_OBJECT;
        }
        return;
    }
#ifdef BLACKMARKET
    /* black market shops should be a little messy */
    if (Is_blackmarket(&u.uz)) {
        if (!rn2(20)) return; /* leave an empty square */
        if (!rn2(10)) {
            (void) mkobj_at(rn2(5) ? GEM_CLASS : TOOL_CLASS, sx, sy, TRUE);
            return;
        }
    }
#endif /* BLACKMARKET */
    atype = get_shop_item(shp - shtypes);
    if (atype < 0) {
        (void) mksobj_at(-atype, sx, sy, TRUE, TRUE);
    } else {
        struct obj *otmp;
redo:
        otmp = mkobj_at(atype, sx, sy, TRUE);

        if ((shp->symb == RANDOM_CLASS) && (color >= 0)) {
            if (objects[otmp->otyp].oc_color != color) {
                obj_extract_self(otmp);
                obfree(otmp, NULL);
                goto redo;
            }
        }
    }
}

/* extract a shopkeeper name for the given shop type */
static void
nameshk(shk, nlp)
struct monst *shk;
const char * const *nlp;
{
    int i, trycnt, names_avail;
    const char *shname = 0;
    struct monst *mtmp;
    int name_wanted;
    s_level *sptr;

    if (!nlp) {
        shname = m_monnam(shk);
    } else if (nlp == shklight && In_mines(&u.uz)
               && (sptr = Is_special(&u.uz)) != 0 && sptr->flags.town) {
        /* special-case minetown lighting shk */
        shname = "Izchak";
        shk->female = FALSE;
    } else {
        /* We want variation from game to game, without needing the save
           and restore support which would be necessary for randomization;
           try not to make too many assumptions about time_t's internals;
           use ledger_no rather than depth to keep mine town distinct. */
        int nseed = game_seed / 257;

        name_wanted = ledger_no(&u.uz) + (nseed % 13) - (nseed % 5);
        if (name_wanted < 0) name_wanted += (13 + 5);
        shk->female = name_wanted & 1;

        for (names_avail = 0; nlp[names_avail]; names_avail++)
            continue;

        for (trycnt = 0; trycnt < 50; trycnt++) {
            if (nlp == shktools) {
                shname = shktools[rn2(names_avail)];
                shk->female = 0; /* reversed below for '_' prefix */
            } else if (nlp == shkmusic) {
                shname = shkmusic[rn2(names_avail)];
            } else if (name_wanted < names_avail) {
                shname = nlp[name_wanted];
            } else if ((i = rn2(names_avail)) != 0) {
                shname = nlp[i - 1];
            } else if (nlp != shkgeneral) {
                nlp = shkgeneral; /* try general names */
                for (names_avail = 0; nlp[names_avail]; names_avail++)
                    continue;
                continue;   /* next `trycnt' iteration */
            } else {
                shname = shk->female ? "-Lucrezia" : "+Dirk";
            }

            if (*shname == '_' || *shname == '-') {
                shk->female = 1;
            } else if (*shname == '|' || *shname == '+') {
                shk->female = 0;
            }

            /* is name already in use on this level? */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp) || (mtmp == shk) || !mtmp->isshk) continue;
                if (!strstr(ESHK(mtmp)->shknam, shname)) continue;
                break;
            }
            if (!mtmp) break; /* new name */
        }
    }
#ifdef BLACKMARKET
    if (Is_blackmarket(&u.uz) && shk->data != &mons[PM_ONE_EYED_SAM]) {
        int num_prefixes;
        const char *prefix = 0;
        /* One-eyed Sam's lackeys idolize her and took nicknames like hers */
        static const char *prefixes[] = {
            "One-armed", "Two-faced", "Three-fingered", "Cross-eyed",
            "Four-toed", "Iron-lunged", "Two-footed", "One-handed",
            "One-legged", "Barefoot", "Cold-blooded", "Cut-throat",
            "Evil-eyed", "Scar-faced", "Five-toothed", "Four-limbed",
            "Eight-fingered", "Color-blind", "Iron-bellied",
            "Silver-tongued", "Crazy-eyed", "Hot-blooded",
            "Green-skinned", "Sharp-eyed", "Hard-nosed", "Dog-eared",
            "Peg-legged", "Knock-knee", "Hook-handed", "One-nostril",
            "Double-chinned", "Double-jointed", "Nine-lives",
            0
        };
        for (num_prefixes = 0; prefixes[num_prefixes]; num_prefixes++)
            continue;

        for (trycnt = 0; trycnt < 50; trycnt++) {
            prefix = prefixes[rn2(num_prefixes)];
            /* is this prefix already used on this level? */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp) || (mtmp == shk) || !mtmp->isshk) continue;
                if (!strstr(ESHK(mtmp)->shknam, prefix)) continue;
                break;
            }
            if (!mtmp) break; /* found an unused prefix */
        }
        snprintf(ESHK(shk)->shknam, PL_NSIZ, "%s %s", prefix, shname);
    } else {
        (void) strncpy(ESHK(shk)->shknam, shname, PL_NSIZ);
    }
#else
    (void) strncpy(ESHK(shk)->shknam, shname, PL_NSIZ);
#endif /* BLACKMARKET */
    ESHK(shk)->shknam[PL_NSIZ-1] = 0;
}

void
neweshk(mtmp)
struct monst *mtmp;
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!ESHK(mtmp))
        ESHK(mtmp) = (struct eshk *) alloc(sizeof(struct eshk));
    (void) memset((genericptr_t) ESHK(mtmp), 0, sizeof(struct eshk));
    ESHK(mtmp)->bill_p = (struct bill_x *) 0;
}

void
free_eshk(mtmp)
struct monst *mtmp;
{
    if (mtmp->mextra && ESHK(mtmp)) {
        free((genericptr_t) ESHK(mtmp));
        ESHK(mtmp) = (struct eshk *) 0;
    }
    mtmp->isshk = 0;
}

/* create a new shopkeeper in the given room */
static int
shkinit(shp, sroom)
const struct shclass *shp;
struct mkroom *sroom;
{
    register int sh, sx, sy;
    struct monst *shk;
    struct eshk *eshkp;
    long shkmoney; /* Temporary placeholder for Shopkeeper's initial capital */

    /* place the shopkeeper in the given room */
    sh = sroom->fdoor;
    sx = doors[sh].x;
    sy = doors[sh].y;

    /* check that the shopkeeper placement is sane */
    if (sroom->irregular) {
        int rmno = (sroom - rooms) + ROOMOFFSET;

        if (isok(sx-1, sy) && !levl[sx-1][sy].edge &&
            (int) levl[sx-1][sy].roomno == rmno) sx--;
        else if (isok(sx+1, sy) && !levl[sx+1][sy].edge &&
                 (int) levl[sx+1][sy].roomno == rmno) sx++;
        else if (isok(sx, sy-1) && !levl[sx][sy-1].edge &&
                 (int) levl[sx][sy-1].roomno == rmno) sy--;
        else if (isok(sx, sy+1) && !levl[sx][sy+1].edge &&
                 (int) levl[sx][sy+1].roomno == rmno) sy++;
        else goto shk_failed;
    }
    else if(sx == sroom->lx-1) sx++;
    else if(sx == sroom->hx+1) sx--;
    else if(sy == sroom->ly-1) sy++;
    else if(sy == sroom->hy+1) sy--; else {
shk_failed:
#ifdef DEBUG
# ifdef WIZARD
        /* Said to happen sometimes, but I have never seen it. */
        /* Supposedly fixed by fdoor change in mklev.c */
        if(wizard) {
            register int j = sroom->doorct;

            pline("Where is shopdoor?");
            pline("Room at (%d,%d),(%d,%d).",
                  sroom->lx, sroom->ly, sroom->hx, sroom->hy);
            pline("doormax=%d doorct=%d fdoor=%d",
                  doorindex, sroom->doorct, sh);
            while(j--) {
                pline("door [%d,%d]", doors[sh].x, doors[sh].y);
                sh++;
            }
            display_nhwindow(WIN_MESSAGE, FALSE);
        }
# endif
#endif
        return(-1);
    }

    if(MON_AT(sx, sy)) (void) rloc(m_at(sx, sy), FALSE); /* insurance */

    /* now initialize the shopkeeper monster structure */

#ifdef BLACKMARKET
    shk = 0;
    if (Is_blackmarket(&u.uz)) {
        if (sroom->rtype == BLACKSHOP) {
            shk = makemon(&mons[PM_ONE_EYED_SAM], sx, sy, MM_ESHK);
        } else {
            shk = makemon(&mons[PM_BLACK_MARKETEER], sx, sy, MM_ESHK);
        }
    }
    if (!shk) {
        if(!(shk = makemon(&mons[PM_SHOPKEEPER], sx, sy, MM_ESHK))) {
            return(-1);
        }
    }
#else  /* BLACKMARKET */
    if(!(shk = makemon(&mons[PM_SHOPKEEPER], sx, sy, MM_ESHK))) {
        return(-1);
    }
#endif /* BLACKMARKET */

    eshkp = ESHK(shk); /* makemon(...,MM_ESHK) allocates this */
    shk->isshk = shk->mpeaceful = 1;
    set_malign(shk);
    shk->msleeping = 0;
    shk->mtrapseen = ~0; /* we know all the traps already */
    eshkp->shoproom = (sroom - rooms) + ROOMOFFSET;
    sroom->resident = shk;
    eshkp->shoptype = sroom->rtype;
    assign_level(&eshkp->shoplevel, &u.uz);
    eshkp->shd = doors[sh];
    eshkp->shk.x = sx;
    eshkp->shk.y = sy;
    eshkp->robbed = 0L;
    eshkp->credit = 0L;
    eshkp->debit = 0L;
    eshkp->loan = 0L;
    eshkp->following = 0L;
    eshkp->surcharge = 0L;
    eshkp->dismiss_kops = FALSE;
    eshkp->visitct = 0;
    eshkp->cheapskate = (rn2(3)==0) ? TRUE : FALSE;
    eshkp->billct = 0;
    eshkp->bill_p = (struct bill_x *) 0;
    eshkp->customer[0] = '\0';

    shkmoney = 1000L + 30L*(long)rnd(100);  /* initial capital */
    /* [CWC] Lets not create the money yet until we see if the
         shk is a black marketeer, else we'll have to create
       another money object, if GOLDOBJ is defined */

    if (shp->shknms == shkrings)
        (void) mongets(shk, TOUCHSTONE);
    nameshk(shk, shp->shknms);

#ifdef BLACKMARKET
    if (Is_blackmarket(&u.uz))
        shkmoney = 7*shkmoney + rn2(3*shkmoney);
#endif
    /* it's a poor town */
    if (Is_town_level(&u.uz))
        shkmoney /= 4;

    mkmonmoney(shk, shkmoney);

#ifdef BLACKMARKET
    /* One-eyed Sam already got her equipment in makemon */
    if (Is_blackmarket(&u.uz) && sroom->rtype != BLACKSHOP) {
        register struct obj *otmp;
        /* black marketeer's equipment */
        otmp = mksobj(LONG_SWORD, FALSE, FALSE);
        mpickobj(shk, otmp);
        if (otmp->spe < 5) otmp->spe += rnd(5);
        if (!rn2(2)) {
            otmp = mksobj(SHIELD_OF_REFLECTION, FALSE, FALSE);
            mpickobj(shk, otmp);
            if (otmp->spe < 5) otmp->spe += rnd(5);
        }
        if (!rn2(2)) {
            otmp = mksobj(GRAY_DRAGON_SCALE_MAIL, FALSE, FALSE);
            mpickobj(shk, otmp);
            if (otmp->spe < 5) otmp->spe += rnd(5);
        }
        if (!rn2(2)) {
            otmp = mksobj(SPEED_BOOTS, FALSE, FALSE);
            mpickobj(shk, otmp);
            if (otmp->spe < 5) otmp->spe += rnd(5);
        }
        if (!rn2(2)) {
            otmp = mksobj(AMULET_OF_LIFE_SAVING, FALSE, FALSE);
            mpickobj(shk, otmp);
        }
        /* wear armor and amulet */
        m_dowear(shk, TRUE);
        otmp = mksobj(SKELETON_KEY, FALSE, FALSE);
        mpickobj(shk, otmp);
    }
#endif /* BLACKMARKET */

    return(sh);
}

static boolean
stock_room_goodpos(sroom, rmno, sh, sx, sy)
struct mkroom *sroom;
int rmno, sh, sx,sy;
{
    if (sroom->irregular) {
        if (levl[sx][sy].edge ||
                (int)levl[sx][sy].roomno != rmno ||
                distmin(sx, sy, doors[sh].x, doors[sh].y) <= 1) {
            return FALSE;
        }
    } else if ((sx == sroom->lx && doors[sh].x == sx-1) ||
               (sx == sroom->hx && doors[sh].x == sx+1) ||
               (sy == sroom->ly && doors[sh].y == sy-1) ||
               (sy == sroom->hy && doors[sh].y == sy+1)) {
        return FALSE;
    }
    return TRUE;
}

/* stock a newly-created room with objects */
void
stock_room(shp_indx, sroom)
int shp_indx;
register struct mkroom *sroom;
{
    /*
     * Someday soon we'll dispatch on the shdist field of shclass to do
     * different placements in this routine. Currently it only supports
     * shop-style placement (all squares except a row nearest the first
     * door get objects).
     */
    int sx, sy, sh;
    char buf[BUFSZ];
    int rmno = (sroom - rooms) + ROOMOFFSET;
    const struct shclass *shp = &shtypes[shp_indx];

    /* first, try to place a shopkeeper in the room */
    if ((sh = shkinit(shp, sroom)) < 0)
        return;

    /* make sure no doorways without doors, and no trapped doors, in shops */
    sx = doors[sroom->fdoor].x;
    sy = doors[sroom->fdoor].y;

    if(levl[sx][sy].doormask == D_NODOOR) {
        levl[sx][sy].doormask = D_ISOPEN;
        newsym(sx, sy);
    }
    if(levl[sx][sy].typ == SDOOR) {
        cvt_sdoor_to_door(&levl[sx][sy]);   /* .typ = DOOR */
        newsym(sx, sy);
    }
    if(levl[sx][sy].doormask & D_TRAPPED)
        levl[sx][sy].doormask = D_LOCKED;

    if(levl[sx][sy].doormask == D_LOCKED) {
        register int m = sx, n = sy;

        if(inside_shop(sx+1, sy)) m--;
        else if(inside_shop(sx-1, sy)) m++;
        if(inside_shop(sx, sy+1)) n--;
        else if(inside_shop(sx, sy-1)) n++;
        Sprintf(buf, "Closed for inventory");
        make_engr_at(m, n, buf, 0L, DUST);
    }

#ifdef BLACKMARKET
    if (sroom->rtype == BLACKSHOP) {
        stock_blkmar(shp, sroom, sh);
        level.flags.has_shop = TRUE;
        return;
    }
#endif /* BLACKMARKET */

    static int rainbow_colors[] = {
        CLR_RED,
        CLR_ORANGE,
        CLR_YELLOW,
        CLR_GREEN,
        CLR_BLUE,
        CLR_MAGENTA,
    };

    boolean rainbow_shop = is_rainbow_shop(sroom);

    for (sx = sroom->lx; sx <= sroom->hx; sx++)
        for (sy = sroom->ly; sy <= sroom->hy; sy++) {
            if (stock_room_goodpos(sroom, rmno, sh, sx,sy)) {
                if (!IS_ROOM(levl[sx][sy].typ)) {
                    continue;
                }

                int color_index = sy - sroom->ly;
                if (doors[sroom->fdoor].y < sroom->ly || doors[sroom->fdoor].y > sroom->hy) {
                    color_index--;
                }
                if (rainbow_shop &&
                     color_index >= 0 && color_index < SIZE(rainbow_colors)) {
                    mkshobj_at(shp, sx, sy, rainbow_colors[color_index]);
                } else {
                    mkshobj_at(shp, sx, sy, -1);
                }
            }
        }

    /*
     * Special monster placements (if any) should go here: that way,
     * monsters will sit on top of objects and not the other way around.
     */

    level.flags.has_shop = TRUE;
}

#ifdef BLACKMARKET
/* stock a newly-created black market with objects */
static void
stock_blkmar(shp, sroom, sh)
const struct shclass *shp UNUSED;
register struct mkroom *sroom;
register int sh;
{
    /*
     * Someday soon we'll dispatch on the shdist field of shclass to do
     * different placements in this routine. Currently it only supports
     * shop-style placement (all squares except a row nearest the first
     * door get objects).
     */
    /* [max] removed register int cl,  char buf[bufsz] */
    int i, sx, sy, first = 0, next = 0, total, partial;
    /* int blkmar_gen[NUM_OBJECTS+2]; */
    int *clp, *lastclp;
    int goodcl[12];

    goodcl[ 0] = WEAPON_CLASS;
    goodcl[ 1] = ARMOR_CLASS;
    goodcl[ 2] = RING_CLASS;
    goodcl[ 3] = AMULET_CLASS;
    goodcl[ 4] = TOOL_CLASS;
    goodcl[ 5] = FOOD_CLASS;
    goodcl[ 6] = POTION_CLASS;
    goodcl[ 7] = SCROLL_CLASS;
    goodcl[ 8] = SPBOOK_CLASS;
    goodcl[ 9] = WAND_CLASS;
    goodcl[10] = GEM_CLASS;
    goodcl[11] = 0;

    /* for (i=0; i < NUM_OBJECTS; i++) {
       blkmar_gen[i] = 0;
       } */

    total = 0;
    for (clp=goodcl; *clp!=0; clp++)  {
        lastclp = clp;
        first = bases[*clp];
        /* this assumes that luckstone & loadstone comes just after the gems */
        next = (*clp==GEM_CLASS) ? (LOADSTONE+1) : bases[(*clp)+1];
        total += next-first;
    }
    if (total==0) return;

    if (sroom->hx-sroom->lx<2) return;
    clp = goodcl-1;
    partial = 0;
    int blkmar_size = (sroom->hx-sroom->lx+1) * (sroom->hy-sroom->ly+1);
    for(sx = sroom->lx+1; sx <= sroom->hx; sx++) {
        if (sx==sroom->lx+1 ||
            ((sx-sroom->lx-2)*total)/(sroom->hx-sroom->lx-1)>partial) {
            clp++;
            if (clp>lastclp) clp = lastclp;
            first = bases[*clp];
            next = (*clp==GEM_CLASS) ? (LOADSTONE+1) : bases[(*clp)+1];
            partial += next-first;
        }

        for (sy = sroom->ly; sy <= sroom->hy; sy++) {
            if ((sx == sroom->lx && doors[sh].x == sx-1) ||
                (sx == sroom->hx && doors[sh].x == sx+1) ||
                (sy == sroom->ly && doors[sh].y == sy-1) ||
                (sy == sroom->hy && doors[sh].y == sy+1) ||
                /* the Blackmarket has 400 items on average */
                (!rnf(400, blkmar_size)))
                continue;

            mkobj_at(RANDOM_CLASS, sx, sy, TRUE);
        }
    }

    /*
     * Special monster placements (if any) should go here: that way,
     * monsters will sit on top of objects and not the other way around.
     */
}
#endif /* BLACKMARKET */


/* does shkp's shop stock this item type? */
boolean
saleable(shkp, obj)
struct monst *shkp;
struct obj *obj;
{
    int i, shp_indx = ESHK(shkp)->shoptype - SHOPBASE;
    const struct shclass *shp = &shtypes[shp_indx];

    if (shp->symb == RANDOM_CLASS) return TRUE;
    else for (i = 0; i < SIZE(shtypes[0].iprobs) && shp->iprobs[i].iprob; i++)
            if (shp->iprobs[i].itype < 0 ?
                shp->iprobs[i].itype == -obj->otyp :
                shp->iprobs[i].itype == obj->oclass) return TRUE;
    /* not found */
    return FALSE;
}

/* positive value: class; negative value: specific object type */
int
get_shop_item(type)
int type;
{
    const struct shclass *shp = shtypes+type;
    register int i, j;

    /* select an appropriate object type at random */
    for(j = rnd(100), i = 0; (j -= shp->iprobs[i].iprob) > 0; i++)
        continue;

    return shp->iprobs[i].itype;
}

/* version of shkname() for beginning of sentence */
char *
Shknam(mtmp)
struct monst *mtmp;
{
    char *nam = shkname(mtmp);

    /* 'nam[]' is almost certainly already capitalized, but be sure */
    nam[0] = highc(nam[0]);
    return nam;
}

/* shopkeeper's name, without any visibility constraint; if hallucinating,
   will yield some other shopkeeper's name (not necessarily one residing
   in the current game's dungeon, or who keeps same type of shop) */
char *
shkname(mtmp)
struct monst *mtmp;
{
    char *nam;
    unsigned save_isshk = mtmp->isshk;

    mtmp->isshk = 0; /* don't want mon_nam() calling shkname() */
    /* get a modifiable name buffer along with fallback result */
    nam = noit_mon_nam(mtmp);
    mtmp->isshk = save_isshk;

    if (!mtmp->isshk) {
        impossible("shkname: \"%s\" is not a shopkeeper.", nam);
    } else if (!has_eshk(mtmp)) {
        panic("shkname: shopkeeper \"%s\" lacks 'eshk' data.", nam);
    } else {
        const char *shknm = ESHK(mtmp)->shknam;

        if (Hallucination && !program_state.gameover) {
            const char *const *nlp;
            int num;

            /* count the number of non-unique shop types;
               pick one randomly, ignoring shop generation probabilities;
               pick a name at random from that shop type's list */
            for (num = 0; num < SIZE(shtypes); num++)
                if (shtypes[num].prob == 0) {
                    break;
                }
            if (num > 0) {
                nlp = shtypes[rn2(num)].shknms;
                for (num = 0; nlp[num]; num++) {
                    continue;
                }
                if (num > 0) {
                    shknm = nlp[rn2(num)];
                }
            }
        }
        /* strip prefix if present */
        if (!letter(*shknm)) {
            ++shknm;
        }
        Strcpy(nam, shknm);
    }
    return nam;
}

boolean
shkname_is_pname(mtmp)
struct monst *mtmp;
{
    const char *shknm = ESHK(mtmp)->shknam;

    return (boolean) (*shknm == '-' || *shknm == '+' || *shknm == '=');
}

boolean
is_izchak(shkp, override_hallucination)
struct monst *shkp;
boolean override_hallucination;
{
    const char *shknm;

    if (Hallucination && !override_hallucination) {
        return FALSE;
    }
    if (!shkp->isshk) {
        return FALSE;
    }
    /* outside of town, Izchak becomes just an ordinary shopkeeper */
    if (!in_town(shkp->mx, shkp->my)) {
        return FALSE;
    }
    shknm = ESHK(shkp)->shknam;
    /* skip "+" prefix */
    if (!letter(*shknm)) {
        ++shknm;
    }
    return (boolean) !strcmp(shknm, "Izchak");
}

/*shknam.c*/
