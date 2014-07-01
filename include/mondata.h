/*	SCCS Id: @(#)mondata.h	3.4	2003/01/08	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONDATA_H
#define MONDATA_H

#define verysmall(ptr)		((ptr)->msize < MZ_SMALL)
#define bigmonst(ptr)		((ptr)->msize >= MZ_LARGE)
#define hugemonst(ptr)		((ptr)->msize >= MZ_HUGE)

#define pm_resistance(ptr,typ)	(((ptr)->mresists & (typ)) != 0)

#define resists_fire(mon)	(((mon)->mintrinsics & MR_FIRE) != 0)
#define resists_cold(mon)	(((mon)->mintrinsics & MR_COLD) != 0)
#define resists_sleep(mon)	(((mon)->mintrinsics & MR_SLEEP) != 0)
#define resists_disint(mon)	(((mon)->mintrinsics & MR_DISINT) != 0)
#define resists_elec(mon)	(((mon)->mintrinsics & MR_ELEC) != 0)
#define resists_poison(mon)	(((mon)->mintrinsics & MR_POISON) != 0)
#define resists_acid(mon)	(((mon)->mintrinsics & MR_ACID) != 0)
#define resists_ston(mon)	(((mon)->mintrinsics & MR_STONE) != 0)

#define is_lminion(mon)		(is_minion((mon)->data) && \
				 (mon)->data->maligntyp >= A_COALIGNED && \
				 ((mon)->data != &mons[PM_ANGEL] || \
				  EPRI(mon)->shralign > 0))

#define is_flyer(ptr)		(((ptr)->mflags1 & M1_FLY) != 0L)
#define is_floater(ptr)		((ptr)->mlet == S_EYE)
#define is_clinger(ptr)		(((ptr)->mflags1 & M1_CLING) != 0L)
#define is_swimmer(ptr)		(((ptr)->mflags1 & M1_SWIM) != 0L)
#define breathless(ptr)		(((ptr)->mflags1 & M1_BREATHLESS) != 0L)
#define amphibious(ptr)		(((ptr)->mflags1 & (M1_AMPHIBIOUS | M1_BREATHLESS)) != 0L)
#define passes_walls(ptr)	(((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr)		(((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr)	((ptr)->mlet == S_GHOST)
#define tunnels(ptr)		(((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define needspick(ptr)		(((ptr)->mflags1 & M1_NEEDPICK) != 0L)
#define hides_under(ptr)	(((ptr)->mflags1 & M1_CONCEAL) != 0L)
#define is_hider(ptr)		(((ptr)->mflags1 & M1_HIDE) != 0L)
#define haseyes(ptr)		(((ptr)->mflags1 & M1_NOEYES) == 0L)
#define eyecount(ptr)		(!haseyes(ptr) ? 0 : \
				 ((ptr) == &mons[PM_CYCLOPS] || \
				  (ptr) == &mons[PM_FLOATING_EYE]) ? 1 : 2)
#define nohands(ptr)		(((ptr)->mflags1 & M1_NOHANDS) != 0L)
#define nolimbs(ptr)		(((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#define notake(ptr)		(((ptr)->mflags1 & M1_NOTAKE) != 0L)
#define has_head(ptr)		(((ptr)->mflags1 & M1_NOHEAD) == 0L)
#define has_horns(ptr)		(num_horns(ptr) > 0)
#define is_whirly(ptr)		((ptr)->mlet == S_VORTEX || \
				 (ptr) == &mons[PM_AIR_ELEMENTAL])
#define flaming(ptr)		((ptr) == &mons[PM_FIRE_VORTEX] || \
				 (ptr) == &mons[PM_FLAMING_SPHERE] || \
				 (ptr) == &mons[PM_FIRE_ELEMENTAL] || \
				 (ptr) == &mons[PM_SALAMANDER])
#define is_silent(ptr)		((ptr)->msound == MS_SILENT)
#define unsolid(ptr)		(((ptr)->mflags1 & M1_UNSOLID) != 0L)
#define mindless(ptr)		(((ptr)->mflags1 & M1_MINDLESS) != 0L)
#define humanoid(ptr)		(((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr)		(((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define slithy(ptr)		(((ptr)->mflags1 & M1_SLITHY) != 0L)
#define is_wooden(ptr)		((ptr) == &mons[PM_WOOD_GOLEM])
#define thick_skinned(ptr)	(((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define lays_eggs(ptr)		(((ptr)->mflags1 & M1_OVIPAROUS) != 0L)
#define regenerates(ptr)	(((ptr)->mflags1 & M1_REGEN) != 0L)
#define noregen(ptr)		(((ptr)->mflags3 & M3_NOREGEN) != 0L)
#define perceives(ptr)		(((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define can_teleport(ptr)	(((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr)	(((ptr)->mflags1 & M1_TPORT_CNTRL) != 0L)
#define telepathic(ptr)		((ptr) == &mons[PM_FLOATING_EYE] || \
				 (ptr) == &mons[PM_MIND_FLAYER] || \
				 (ptr) == &mons[PM_MASTER_MIND_FLAYER])
#define is_armed(ptr)		attacktype(ptr, AT_WEAP)
#define acidic(ptr)		(((ptr)->mflags1 & M1_ACID) != 0L)
#define poisonous(ptr)		(((ptr)->mflags1 & M1_POIS) != 0L)
#define carnivorous(ptr)	(((ptr)->mflags1 & M1_CARNIVORE) != 0L)
#define herbivorous(ptr)	(((ptr)->mflags1 & M1_HERBIVORE) != 0L)
#define metallivorous(ptr)	(((ptr)->mflags1 & M1_METALLIVORE) != 0L)
#define polyok(ptr)		(((ptr)->mflags2 & M2_NOPOLY) == 0L)
#define is_undead(ptr)		(((ptr)->mflags2 & M2_UNDEAD) != 0L)
#define is_were(ptr)		(((ptr)->mflags2 & M2_WERE) != 0L)
#define is_vampire(ptr)		((ptr)->mlet == S_VAMPIRE || \
				 ((ptr)->mflags2 & M2_VAMPIRE) != 0)
/** feeds on blood */
#define is_vampiric(ptr)	((is_vampire(ptr)) || \
				 (ptr) == &mons[PM_VAMPIRE_BAT])
#define is_elf(ptr)		(((ptr)->mflags2 & M2_ELF) != 0L)
#define is_dwarf(ptr)		(((ptr)->mflags2 & M2_DWARF) != 0L)
#define is_gnome(ptr)		(((ptr)->mflags2 & M2_GNOME) != 0L)
#define is_orc(ptr)		(((ptr)->mflags2 & M2_ORC) != 0L)
#define is_human(ptr)		(((ptr)->mflags2 & M2_HUMAN) != 0L)
#define your_race(ptr)		(((ptr)->mflags2 & urace.selfmask) != 0L)
#define is_bat(ptr)		((ptr) == &mons[PM_BAT] || \
				 (ptr) == &mons[PM_GIANT_BAT] || \
				 (ptr) == &mons[PM_VAMPIRE_BAT])
#ifdef CONVICT
# define is_rat(ptr)		((ptr) == &mons[PM_SEWER_RAT] || \
				 (ptr) == &mons[PM_GIANT_RAT] || \
				 (ptr) == &mons[PM_RABID_RAT] || \
				 (ptr) == &mons[PM_ENORMOUS_RAT])
#else /* CONVICT */
# define is_rat(ptr)		((ptr) == &mons[PM_SEWER_RAT] || \
				 (ptr) == &mons[PM_GIANT_RAT] || \
				 (ptr) == &mons[PM_RABID_RAT])
#endif /* CONVICT */
#define is_bird(ptr)		((ptr)->mlet == S_BAT && !is_bat(ptr))
#define is_giant(ptr)		(((ptr)->mflags2 & M2_GIANT) != 0L)
#define is_golem(ptr)		((ptr)->mlet == S_GOLEM)
#define is_statue(ptr)		((ptr)->mlet == S_STATUE)
#define is_domestic(ptr)	(((ptr)->mflags2 & M2_DOMESTIC) != 0L)
#define is_demon(ptr)		(((ptr)->mflags2 & M2_DEMON) != 0L)
#define is_mercenary(ptr)	(((ptr)->mflags2 & M2_MERC) != 0L)
#define is_male(ptr)		(((ptr)->mflags2 & M2_MALE) != 0L)
#define is_female(ptr)		(((ptr)->mflags2 & M2_FEMALE) != 0L)
#define is_neuter(ptr)		(((ptr)->mflags2 & M2_NEUTER) != 0L)
#define is_wanderer(ptr)	(((ptr)->mflags2 & M2_WANDER) != 0L)
#define always_hostile(ptr)	(((ptr)->mflags2 & M2_HOSTILE) != 0L)
#define always_peaceful(ptr)	(((ptr)->mflags2 & M2_PEACEFUL) != 0L)
#define race_hostile(ptr)	(((ptr)->mflags2 & urace.hatemask) != 0L)
#define race_peaceful(ptr)	(((ptr)->mflags2 & urace.lovemask) != 0L)
#define extra_nasty(ptr)	(((ptr)->mflags2 & M2_NASTY) != 0L)
#define strongmonst(ptr)	(((ptr)->mflags2 & M2_STRONG) != 0L)
#define can_breathe(ptr)	attacktype(ptr, AT_BREA)
#define cantwield(ptr)		(nohands(ptr) || verysmall(ptr))
#define could_twoweap(ptr)	((ptr)->mattk[1].aatyp == AT_WEAP)
#define cantweararm(ptr)	(breakarm(ptr) || sliparm(ptr))
#define throws_rocks(ptr)	(((ptr)->mflags2 & M2_ROCKTHROW) != 0L)
#define type_is_pname(ptr)	(((ptr)->mflags2 & M2_PNAME) != 0L)
#define is_lord(ptr)		(((ptr)->mflags2 & M2_LORD) != 0L)
#define is_prince(ptr)		(((ptr)->mflags2 & M2_PRINCE) != 0L)
#define is_ndemon(ptr)		(is_demon(ptr) && \
				 (((ptr)->mflags2 & (M2_LORD|M2_PRINCE)) == 0L))
#define is_dlord(ptr)		(is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr)		(is_demon(ptr) && is_prince(ptr))
#define is_minion(ptr)		((ptr)->mflags2 & M2_MINION)
#define likes_gold(ptr)		(((ptr)->mflags2 & M2_GREEDY) != 0L)
#define likes_gems(ptr)		(((ptr)->mflags2 & M2_JEWELS) != 0L)
#define likes_objs(ptr)		(((ptr)->mflags2 & M2_COLLECT) != 0L || \
				 is_armed(ptr))
#define likes_magic(ptr)	(((ptr)->mflags2 & M2_MAGIC) != 0L)
#define webmaker(ptr)		((ptr) == &mons[PM_CAVE_SPIDER] || \
				 (ptr) == &mons[PM_GIANT_SPIDER])
#define is_unicorn(ptr)		((ptr)->mlet == S_UNICORN && likes_gems(ptr))
#define is_longworm(ptr)	(((ptr) == &mons[PM_BABY_LONG_WORM]) || \
				 ((ptr) == &mons[PM_LONG_WORM]) || \
				 ((ptr) == &mons[PM_LONG_WORM_TAIL]))
#define is_covetous(ptr)	((ptr->mflags3 & M3_COVETOUS))
#define infravision(ptr)	((ptr->mflags3 & M3_INFRAVISION))
#define infravisible(ptr)	((ptr->mflags3 & M3_INFRAVISIBLE))
#define is_mplayer(ptr)		(((ptr) >= &mons[PM_ARCHEOLOGIST]) && \
				 ((ptr) <= &mons[PM_WIZARD]))
#define is_rider(ptr)		((ptr) == &mons[PM_DEATH] || \
				 (ptr) == &mons[PM_FAMINE] || \
				 (ptr) == &mons[PM_PESTILENCE])
#define is_guardian(ptr)	((ptr) == &mons[PM_VLAD_THE_IMPALER] || \
				 (ptr) == &mons[PM_WIZARD_OF_YENDOR] || \
				 (ptr)->msound == MS_NEMESIS || \
				 (ptr) == &mons[PM_CTHULHU])
#define is_placeholder(ptr)	((ptr) == &mons[PM_ORC] || \
				 (ptr) == &mons[PM_GIANT] || \
				 (ptr) == &mons[PM_ELF] || \
				 (ptr) == &mons[PM_HUMAN])
/* return TRUE if the monster tends to revive */
#define is_reviver(ptr)		(is_rider(ptr) || (ptr)->mlet == S_TROLL)


#ifdef PM_BEHOLDER
#define is_eye(ptr)		(((ptr) == &mons[PM_FLOATING_EYE]) || \
				 ((ptr) == &mons[PM_EVIL_EYE]) || \
				 ((ptr) == &mons[PM_BEHOLDER]))
#else
#define is_eye(ptr)		(((ptr) == &mons[PM_FLOATING_EYE]) || \
				 ((ptr) == &mons[PM_EVIL_EYE]))
#endif

/* this returns the light's range, or 0 if none; if we add more light emitting
   monsters, we'll likely have to add a new light range field to mons[] */
#define emits_light(ptr)	(((ptr)->mlet == S_LIGHT || \
				  (ptr) == &mons[PM_FLAMING_SPHERE] || \
				  (ptr) == &mons[PM_SHOCKING_SPHERE] || \
				  (ptr) == &mons[PM_WAX_GOLEM] || \
				  (ptr) == &mons[PM_BLAZING_FERN] || \
				  (ptr) == &mons[PM_BLAZING_FERN_SPROUT] || \
				  (ptr) == &mons[PM_BLAZING_FERN_SPORE] || \
				  (!strcmp((ptr)->mname, "glowing dragon")) || \
				  (!strcmp((ptr)->mname, "baby glowing dragon")) || \
				  (ptr) == &mons[PM_FIRE_VORTEX]) ? 1 : \
				 ((ptr) == &mons[PM_FIRE_ELEMENTAL]) ? 1 : 0)
/*	[note: the light ranges above were reduced to 1 for performance...] */
#define likes_lava(ptr)		(ptr == &mons[PM_FIRE_ELEMENTAL] || \
				 ptr == &mons[PM_SALAMANDER])
#define pm_invisible(ptr) ((ptr) == &mons[PM_STALKER] || \
			   (ptr) == &mons[PM_BLACK_LIGHT])

/* could probably add more */
#define likes_fire(ptr)		((ptr) == &mons[PM_FIRE_VORTEX] || \
				  (ptr) == &mons[PM_FLAMING_SPHERE] || \
				 likes_lava(ptr))

#define touch_petrifies(ptr)	((ptr) == &mons[PM_COCKATRICE] || \
				 (ptr) == &mons[PM_CHICKATRICE])

#define is_gargoyle(ptr)	((ptr) == &mons[PM_GARGOYLE] || \
				 (ptr) == &mons[PM_WINGED_GARGOYLE])

#define is_weeping(ptr)		((ptr) == &mons[PM_WEEPING_ANGEL] || \
				 (ptr) == &mons[PM_WEEPING_ARCHANGEL])

#ifdef WEBB_DISINT
# define touch_disintegrates(ptr) ((ptr) == &mons[PM_DISINTEGRATOR])
#endif
  
#define is_mind_flayer(ptr)	((ptr) == &mons[PM_MIND_FLAYER] || \
				 (ptr) == &mons[PM_MASTER_MIND_FLAYER])

#define nonliving(ptr)		(is_golem(ptr) || is_undead(ptr) || \
				 is_statue(ptr) || \
				 (ptr)->mlet == S_VORTEX || \
				 (ptr) == &mons[PM_MANES])

#define is_blinker(ptr)		((ptr)->mflags3 & M3_BLINKER)
#define is_groupattacker(ptr)   ((ptr)->mflags3 & M3_GROUPATTACK)

#define likes_swamp(ptr)	((ptr)->mlet == S_PUDDING || \
				 (ptr)->mlet == S_FUNGUS || \
				 (ptr) == &mons[PM_OCHRE_JELLY])
#define stationary(ptr)		((ptr)->mflags3 & M3_STATIONARY)

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr)		((ptr)->mlet == S_BLOB || \
				 (ptr)->mlet == S_JELLY ||            \
				 (ptr)->mlet == S_FUNGUS ||           \
				 (ptr)->mlet == S_VORTEX ||           \
				 (ptr)->mlet == S_LIGHT ||            \
				((ptr)->mlet == S_ELEMENTAL &&        \
				 (ptr) != &mons[PM_STALKER]) ||       \
				((ptr)->mlet == S_GOLEM &&            \
				 (ptr) != &mons[PM_FLESH_GOLEM] &&    \
				 (ptr) != &mons[PM_LEATHER_GOLEM]) || \
				 noncorporeal(ptr))
#define vegetarian(ptr)		(vegan(ptr) || \
				((ptr)->mlet == S_PUDDING &&         \
				 (ptr) != &mons[PM_BLACK_PUDDING]))

/* Keep track of ferns, fern sprouts, fern spores, and other plants */

#define is_fern_sprout(ptr)	((ptr) == &mons[PM_ARCTIC_FERN_SPROUT] || \
				 (ptr) == &mons[PM_BLAZING_FERN_SPROUT] || \
				 (ptr) == &mons[PM_DUNGEON_FERN_SPROUT] || \
				 (ptr) == &mons[PM_SWAMP_FERN_SPROUT])

#define is_fern_spore(ptr)	((ptr) == &mons[PM_FERN_SPORE] || \
				 (ptr) == &mons[PM_ARCTIC_FERN_SPORE] || \
				 (ptr) == &mons[PM_BLAZING_FERN_SPORE] || \
				 (ptr) == &mons[PM_DUNGEON_FERN_SPORE] || \
				 (ptr) == &mons[PM_SWAMP_FERN_SPORE])

#define is_fern(ptr)		(is_fern_sprout(ptr) || \
				 (ptr) == &mons[PM_ARCTIC_FERN] || \
				 (ptr) == &mons[PM_BLAZING_FERN] || \
				 (ptr) == &mons[PM_DUNGEON_FERN] || \
				 (ptr) == &mons[PM_SWAMP_FERN])

#define is_vegetation(ptr)	(is_fern(ptr) || \
				 (ptr) == &mons[PM_DEVIL_S_SNARE])

/* For vampires */
#define has_blood(ptr)		(!vegetarian(ptr) && \
				   ((ptr)->mlet != S_GOLEM || (ptr) == &mons[PM_FLESH_GOLEM]) && \
				   (!is_undead(ptr) || is_vampire(ptr)))

#ifdef CONVICT
#define befriend_with_obj(ptr, obj) ((obj)->oclass == FOOD_CLASS && \
				     (is_domestic(ptr) || (is_rat(ptr) && Role_if(PM_CONVICT))))
#else
#define befriend_with_obj(ptr, obj) ((obj)->oclass == FOOD_CLASS && \
				     is_domestic(ptr))
#endif

#define is_rockbreaker(ptr)	(((ptr)->msound == MS_LEADER || \
				  is_blkmktstaff((ptr)) || \
				  is_rider((ptr))) && \
				 !mtmp->mpeaceful)

#ifdef BLACKMARKET
#define is_blkmktstaff(ptr)	(Is_blackmarket(&u.uz) && \
				 ((ptr) == &mons[PM_BLACK_MARKETEER] || \
				  (ptr) == &mons[PM_ONE_EYED_SAM]))
#endif /* BLACKMARKET */

#endif /* MONDATA_H */
