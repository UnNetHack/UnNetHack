/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Align._
import MonFlag._
import PerMonst._
import Pm._
import You._

object MonData {

def verysmall(ptr: permonst) =		(ptr.msize < MZ_SMALL)
def bigmonst(ptr: permonst) =		(ptr.msize >= MZ_LARGE)
def hugemonst(ptr: permonst) =		(ptr.msize >= MZ_HUGE)

def pm_resistance(ptr: permonst,typ: Int) = ((ptr.mresists & (typ)) != 0)

def resists_fire(mon: permonst) =	((mon.mintrinsics & MR_FIRE) != 0)
def resists_cold(mon: permonst) =	((mon.mintrinsics & MR_COLD) != 0)
def resists_sleep(mon: permonst) =	((mon.mintrinsics & MR_SLEEP) != 0)
def resists_disint(mon: permonst) =	((mon.mintrinsics & MR_DISINT) != 0)
def resists_elec(mon: permonst) =	((mon.mintrinsics & MR_ELEC) != 0)
def resists_poison(mon: permonst) =	((mon.mintrinsics & MR_POISON) != 0)
def resists_acid(mon: permonst) =	((mon.mintrinsics & MR_ACID) != 0)
def resists_ston(mon: permonst) =	((mon.mintrinsics & MR_STONE) != 0)

def is_lminion(mon: permonst) =		(is_minion(mon.data) && 
			 mon.data.maligntyp >= A_COALIGNED && 
			 (mon.data != mons(PM_ANGEL) || 
			  EPRImon.shralign > 0))

def is_flyer(ptr: permonst) =		((ptr.mflags1 & M1_FLY) != 0L)
def is_floater(ptr: permonst) =		(ptr.mlet == S_EYE)
def is_clinger(ptr: permonst) =		((ptr.mflags1 & M1_CLING) != 0L)
def is_swimmer(ptr: permonst) =		((ptr.mflags1 & M1_SWIM) != 0L)
def breathless(ptr: permonst) =		((ptr.mflags1 & M1_BREATHLESS) != 0L)
def amphibious(ptr: permonst) =		((ptr.mflags1 & (M1_AMPHIBIOUS | M1_BREATHLESS)) != 0L)
def passes_walls(ptr: permonst) =	((ptr.mflags1 & M1_WALLWALK) != 0L)
def amorphous(ptr: permonst) =		((ptr.mflags1 & M1_AMORPHOUS) != 0L)
def noncorporeal(ptr: permonst) =	(ptr.mlet == S_GHOST)
def tunnels(ptr: permonst) =		((ptr.mflags1 & M1_TUNNEL) != 0L)
def needspick(ptr: permonst) =		((ptr.mflags1 & M1_NEEDPICK) != 0L)
def hides_under(ptr: permonst) =	((ptr.mflags1 & M1_CONCEAL) != 0L)
def is_hider(ptr: permonst) =		((ptr.mflags1 & M1_HIDE) != 0L)
def haseyes(ptr: permonst) =		((ptr.mflags1 & M1_NOEYES) == 0L)
def eyecount(ptr: permonst) =		(if(!haseyes(ptr)) 0 else 
			 (ptr == mons(PM_CYCLOPS) || 
			  if(ptr == mons(PM_FLOATING_EYE)) 1 else 2)
def nohands(ptr: permonst) =		((ptr.mflags1 & M1_NOHANDS) != 0L)
def nolimbs(ptr: permonst) =		((ptr.mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
def notake(ptr: permonst) =		((ptr.mflags1 & M1_NOTAKE) != 0L)
def has_head(ptr: permonst) =		((ptr.mflags1 & M1_NOHEAD) == 0L)
def has_horns(ptr: permonst) =		(num_horns(ptr) > 0)
def is_whirly(ptr: permonst) =		(ptr.mlet == S_VORTEX || 
			 ptr == mons(PM_AIR_ELEMENTAL))
def flaming(ptr: permonst) =		(ptr == mons(PM_FIRE_VORTEX) || 
			 ptr == mons(PM_FLAMING_SPHERE) || 
			 ptr == mons(PM_FIRE_ELEMENTAL) || 
			 ptr == mons(PM_SALAMANDER))
def is_silent(ptr: permonst) =		(ptr.msound == MS_SILENT)
def unsolid(ptr: permonst) =		((ptr.mflags1 & M1_UNSOLID) != 0L)
def mindless(ptr: permonst) =		((ptr.mflags1 & M1_MINDLESS) != 0L)
def humanoid(ptr: permonst) =		((ptr.mflags1 & M1_HUMANOID) != 0L)
def is_animal(ptr: permonst) =		((ptr.mflags1 & M1_ANIMAL) != 0L)
def slithy(ptr: permonst) =		((ptr.mflags1 & M1_SLITHY) != 0L)
def is_wooden(ptr: permonst) =		(ptr == mons(PM_WOOD_GOLEM))
def thick_skinned(ptr: permonst) =	((ptr.mflags1 & M1_THICK_HIDE) != 0L)
def lays_eggs(ptr: permonst) =		((ptr.mflags1 & M1_OVIPAROUS) != 0L)
def regenerates(ptr: permonst) =	((ptr.mflags1 & M1_REGEN) != 0L)
def noregen(ptr: permonst) =		((ptr.mflags3 & M3_NOREGEN) != 0L)
def perceives(ptr: permonst) =		((ptr.mflags1 & M1_SEE_INVIS) != 0L)
def can_teleport(ptr: permonst) =	((ptr.mflags1 & M1_TPORT) != 0L)
def control_teleport(ptr: permonst) =	((ptr.mflags1 & M1_TPORT_CNTRL) != 0L)
def telepathic(ptr: permonst) =		(ptr == mons(PM_FLOATING_EYE) || 
			 ptr == mons(PM_MIND_FLAYER) || 
			 ptr == mons(PM_MASTER_MIND_FLAYER))
def is_armed(ptr: permonst) =		attacktype(ptr, AT_WEAP)
def acidic(ptr: permonst) =		((ptr.mflags1 & M1_ACID) != 0L)
def poisonous(ptr: permonst) =		((ptr.mflags1 & M1_POIS) != 0L)
def carnivorous(ptr: permonst) =	((ptr.mflags1 & M1_CARNIVORE) != 0L)
def herbivorous(ptr: permonst) =	((ptr.mflags1 & M1_HERBIVORE) != 0L)
def metallivorous(ptr: permonst) =	((ptr.mflags1 & M1_METALLIVORE) != 0L)
def polyok(ptr: permonst) =		((ptr.mflags2 & M2_NOPOLY) == 0L)
def is_undead(ptr: permonst) =		((ptr.mflags2 & M2_UNDEAD) != 0L)
def is_were(ptr: permonst) =		((ptr.mflags2 & M2_WERE) != 0L)
def is_vampire(ptr: permonst) =		(ptr.mlet == S_VAMPIRE || 
			 (ptr.mflags2 & M2_VAMPIRE) != 0)
/** feeds on blood */
def is_vampiric(ptr: permonst) =	((is_vampire(ptr)) || 
			 ptr == mons(PM_VAMPIRE_BAT))
def is_elf(ptr: permonst) =		((ptr.mflags2 & M2_ELF) != 0L)
def is_dwarf(ptr: permonst) =		((ptr.mflags2 & M2_DWARF) != 0L)
def is_gnome(ptr: permonst) =		((ptr.mflags2 & M2_GNOME) != 0L)
def is_orc(ptr: permonst) =		((ptr.mflags2 & M2_ORC) != 0L)
def is_human(ptr: permonst) =		((ptr.mflags2 & M2_HUMAN) != 0L)
def your_race(ptr: permonst) =		((ptr.mflags2 & urace.selfmask) != 0L)
def is_bat(ptr: permonst) =		(ptr == mons(PM_BAT) || 
			 ptr == mons(PM_GIANT_BAT) || 
			 ptr == mons(PM_VAMPIRE_BAT))
def is_rat(ptr: permonst =	(ptr == mons(PM_SEWER_RAT) || 
			 ptr == mons(PM_GIANT_RAT) || 
			 ptr == mons(PM_RABID_RAT) || 
			 ptr == mons(PM_ENORMOUS_RAT))
def is_bird(ptr: permonst) =		(ptr.mlet == S_BAT && !is_bat(ptr))
def is_giant(ptr: permonst) =		((ptr.mflags2 & M2_GIANT) != 0L)
def is_golem(ptr: permonst) =		(ptr.mlet == S_GOLEM)
def is_statue(ptr: permonst) =		(ptr.mlet == S_STATUE)
def is_domestic(ptr: permonst) =	((ptr.mflags2 & M2_DOMESTIC) != 0L)
def is_demon(ptr: permonst) =		((ptr.mflags2 & M2_DEMON) != 0L)
def is_mercenary(ptr: permonst) =	((ptr.mflags2 & M2_MERC) != 0L)
def is_male(ptr: permonst) =		((ptr.mflags2 & M2_MALE) != 0L)
def is_female(ptr: permonst) =		((ptr.mflags2 & M2_FEMALE) != 0L)
def is_neuter(ptr: permonst) =		((ptr.mflags2 & M2_NEUTER) != 0L)
def is_wanderer(ptr: permonst) =	((ptr.mflags2 & M2_WANDER) != 0L)
def always_hostile(ptr: permonst) =	((ptr.mflags2 & M2_HOSTILE) != 0L)
def always_peaceful(ptr: permonst) =	((ptr.mflags2 & M2_PEACEFUL) != 0L)
def race_hostile(ptr: permonst) =	((ptr.mflags2 & urace.hatemask) != 0L)
def race_peaceful(ptr: permonst) =	((ptr.mflags2 & urace.lovemask) != 0L)
def extra_nasty(ptr: permonst) =	((ptr.mflags2 & M2_NASTY) != 0L)
def strongmonst(ptr: permonst) =	((ptr.mflags2 & M2_STRONG) != 0L)
def can_breathe(ptr: permonst) =	attacktype(ptr, AT_BREA)
def cantwield(ptr: permonst) =		(nohands(ptr) || verysmall(ptr))
def could_twoweap(ptr: permonst) =	(ptr.mattk[1].aatyp == AT_WEAP)
def cantweararm(ptr: permonst) =	(breakarm(ptr) || sliparm(ptr))
def throws_rocks(ptr: permonst) =	((ptr.mflags2 & M2_ROCKTHROW) != 0L)
def type_is_pname(ptr: permonst) =	((ptr.mflags2 & M2_PNAME) != 0L)
def is_lord(ptr: permonst) =		((ptr.mflags2 & M2_LORD) != 0L)
def is_prince(ptr: permonst) =		((ptr.mflags2 & M2_PRINCE) != 0L)
def is_ndemon(ptr: permonst) =		(is_demon(ptr) && 
			 ((ptr.mflags2 & (M2_LORD|M2_PRINCE)) == 0L))
def is_dlord(ptr: permonst) =		(is_demon(ptr) && is_lord(ptr))
def is_dprince(ptr: permonst) =		(is_demon(ptr) && is_prince(ptr))
def is_minion(ptr: permonst) =		(ptr.mflags2 & M2_MINION)
def likes_gold(ptr: permonst) =		((ptr.mflags2 & M2_GREEDY) != 0L)
def likes_gems(ptr: permonst) =		((ptr.mflags2 & M2_JEWELS) != 0L)
def likes_objs(ptr: permonst) =		((ptr.mflags2 & M2_COLLECT) != 0L || 
			 is_armed(ptr))
def likes_magic(ptr: permonst) =	((ptr.mflags2 & M2_MAGIC) != 0L)
def webmaker(ptr: permonst) =		(ptr == mons(PM_CAVE_SPIDER) || 
			 ptr == mons(PM_GIANT_SPIDER))
def is_unicorn(ptr: permonst) =		(ptr.mlet == S_UNICORN && likes_gems(ptr))
def is_longworm(ptr: permonst) =	((ptr == mons(PM_BABY_LONG_WORM)) || 
			 (ptr == mons(PM_LONG_WORM)) || 
			 (ptr == mons(PM_LONG_WORM_TAIL)))
def is_covetous(ptr: permonst) =	((ptr->mflags3 & M3_COVETOUS))
def infravision(ptr: permonst) =	((ptr->mflags3 & M3_INFRAVISION))
def infravisible(ptr: permonst) =	((ptr->mflags3 & M3_INFRAVISIBLE))
def is_mplayer(ptr: permonst) =		(((ptr) >= mons(PM_ARCHEOLOGIST)) && 
			 ((ptr) <= mons(PM_WIZARD)))
def is_rider(ptr: permonst) =		(ptr == mons(PM_DEATH) || 
			 ptr == mons(PM_FAMINE) || 
			 ptr == mons(PM_PESTILENCE))
def is_guardian(ptr: permonst) =	(ptr == mons(PM_VLAD_THE_IMPALER) || 
			 ptr == mons(PM_WIZARD_OF_YENDOR) || 
			 ptr.msound == MS_NEMESIS || 
			 ptr == mons(PM_CTHULHU))
def is_placeholder(ptr: permonst) =	(ptr == mons(PM_ORC) || 
			 ptr == mons(PM_GIANT) || 
			 ptr == mons(PM_ELF) || 
			 ptr == mons(PM_HUMAN))
/* return TRUE if the monster tends to revive */
def is_reviver(ptr: permonst) =		(is_rider(ptr) || ptr.mlet == S_TROLL)


def is_eye(ptr: permonst) =		((ptr == mons(PM_FLOATING_EYE)) || 
			 (ptr == mons(PM_EVIL_EYE)))

/* this returns the light's range, or 0 if none; if we add more light emitting
monsters, we'll likely have to add a new light range field to mons[] */
def emits_light(ptr: permonst) =	((ptr.mlet == S_LIGHT || 
			  ptr == mons(PM_FLAMING_SPHERE) || 
			  ptr == mons(PM_SHOCKING_SPHERE) || 
			  ptr == mons(PM_WAX_GOLEM) || 
			  ptr == mons(PM_BLAZING_FERN) || 
			  ptr == mons(PM_BLAZING_FERN_SPROUT) || 
			  ptr == mons(PM_BLAZING_FERN_SPORE) || 
			  (!strcmp(ptr.mname, "glowing dragon")) || 
			  (!strcmp(ptr.mname, "baby glowing dragon")) || 
			  if(ptr == mons(PM_FIRE_VORTEX)) 1 else 
			 if(ptr == mons(PM_FIRE_ELEMENTAL)) 1 else 0)
/*	[note: the light ranges above were reduced to 1 for performance...] */
def likes_lava(ptr: permonst) =		(ptr == mons(PM_FIRE_ELEMENTAL) || 
			 ptr == mons(PM_SALAMANDER))
def pm_invisible(ptr: permonst) = (ptr == mons(PM_STALKER) || 
		   ptr == mons(PM_BLACK_LIGHT))

/* could probably add more */
def likes_fire(ptr: permonst) =		(ptr == mons(PM_FIRE_VORTEX) || 
			  ptr == mons(PM_FLAMING_SPHERE) || 
			 likes_lava(ptr))

def touch_petrifies(ptr: permonst) =	(ptr == mons(PM_COCKATRICE) || 
			 ptr == mons(PM_CHICKATRICE))

def is_gargoyle(ptr: permonst) =	(ptr == mons(PM_GARGOYLE) || 
			 ptr == mons(PM_WINGED_GARGOYLE))

def is_weeping(ptr: permonst) =		(ptr == mons(PM_WEEPING_ANGEL) || 
			 ptr == mons(PM_WEEPING_ARCHANGEL))

def touch_disintegrates(ptr: permonst) = (ptr == mons(PM_DISINTEGRATOR))

def is_mind_flayer(ptr: permonst) =	(ptr == mons(PM_MIND_FLAYER) || 
			 ptr == mons(PM_MASTER_MIND_FLAYER))

def nonliving(ptr: permonst) =		(is_golem(ptr) || is_undead(ptr) || 
			 is_statue(ptr) || 
			 ptr.mlet == S_VORTEX || 
			 ptr == mons(PM_MANES))

def is_blinker(ptr: permonst) =		(ptr.mflags3 & M3_BLINKER)
def is_groupattacker(ptr: permonst) =   (ptr.mflags3 & M3_GROUPATTACK)

def likes_swamp(ptr: permonst) =	(ptr.mlet == S_PUDDING || 
			 ptr.mlet == S_FUNGUS || 
			 ptr == mons(PM_OCHRE_JELLY))
def stationary(ptr: permonst) =		(ptr.mflags3 & M3_STATIONARY)

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
def vegan(ptr: permonst) =		(ptr.mlet == S_BLOB || 
			 ptr.mlet == S_JELLY ||            
			 ptr.mlet == S_FUNGUS ||           
			 ptr.mlet == S_VORTEX ||           
			 ptr.mlet == S_LIGHT ||            
			(ptr.mlet == S_ELEMENTAL &&        
			 ptr != mons(PM_STALKER)) ||       
			(ptr.mlet == S_GOLEM &&            
			 ptr != mons(PM_FLESH_GOLEM) &&    
			 ptr != mons(PM_LEATHER_GOLEM)) || 
			 noncorporeal(ptr))
def vegetarian(ptr: permonst) =		(vegan(ptr) || 
			(ptr.mlet == S_PUDDING &&         
			 ptr != mons(PM_BLACK_PUDDING)))

/* Keep track of ferns, fern sprouts, fern spores, and other plants */

def is_fern_sprout(ptr: permonst) =	(ptr == mons(PM_ARCTIC_FERN_SPROUT) || 
			 ptr == mons(PM_BLAZING_FERN_SPROUT) || 
			 ptr == mons(PM_DUNGEON_FERN_SPROUT) || 
			 ptr == mons(PM_SWAMP_FERN_SPROUT))

def is_fern_spore(ptr: permonst) =	(ptr == mons(PM_FERN_SPORE) || 
			 ptr == mons(PM_ARCTIC_FERN_SPORE) || 
			 ptr == mons(PM_BLAZING_FERN_SPORE) || 
			 ptr == mons(PM_DUNGEON_FERN_SPORE) || 
			 ptr == mons(PM_SWAMP_FERN_SPORE))

def is_fern(ptr: permonst) =		(is_fern_sprout(ptr) || 
			 ptr == mons(PM_ARCTIC_FERN) || 
			 ptr == mons(PM_BLAZING_FERN) || 
			 ptr == mons(PM_DUNGEON_FERN) || 
			 ptr == mons(PM_SWAMP_FERN))

def is_vegetation(ptr: permonst) =	(is_fern(ptr) || 
			 ptr == mons(PM_DEVIL_S_SNARE))

/* For vampires */
def has_blood(ptr: permonst) =		(!vegetarian(ptr) && 
			   (ptr.mlet != S_GOLEM || ptr == mons(PM_FLESH_GOLEM)) && 
			   (!is_undead(ptr) || is_vampire(ptr)))

def befriend_with_obj(ptr: permonst, obj: obj) = (obj.oclass == FOOD_CLASS && 
			     (is_domestic(ptr) || (is_rat(ptr) && Role_if(PM_CONVICT))))

def is_rockbreaker(ptr: permonst) =	((ptr.msound == MS_LEADER || 
			  is_blkmktstaff((ptr)) || 
			  is_rider((ptr))) && 
			 !mtmp.mpeaceful)
}
