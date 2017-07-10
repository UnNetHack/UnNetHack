/*	SCCS Id: @(#)extern.h	3.4	2003/03/10	*/
/* Copyright (c) Steve Creps, 1988.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#define E extern

/* ### alloc.c ### */

#if 0
E long *alloc(unsigned int);
#endif
E char *fmt_ptr(const genericptr,char *);

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

E void moveloop(void);
E void stop_occupation(void);
E void display_gamewindows(void);
E void newgame(void);
E void welcome(BOOLEAN_P);
E boolean can_regenerate(void);
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
E time_t get_realtime(void);
#endif

/* ### apply.c ### */

E int doapply(void);
E int dorub(void);
E int dojump(void);
E int jump(int);
E int number_leashed(void);
E void o_unleash(struct obj *);
E void m_unleash(struct monst *,BOOLEAN_P);
E void unleash_all(void);
E boolean next_to_u(void);
E struct obj *get_mleash(struct monst *);
E void check_leash(XCHAR_P,XCHAR_P);
E boolean um_dist(XCHAR_P,XCHAR_P,XCHAR_P);
E boolean snuff_candle(struct obj *);
E boolean snuff_lit(struct obj *);
E boolean catch_lit(struct obj *);
E void fix_attributes_and_properties(struct obj *, int);
E boolean tinnable(struct obj *);
E void reset_trapset(void);
E void fig_transform(genericptr_t, long);
E int unfixable_trouble_count(BOOLEAN_P);

/* ### artifact.c ### */

E void init_artifacts(void);
E void save_artifacts(int);
E void restore_artifacts(int);
E const char *artiname(int);
E struct obj *mk_artifact(struct obj *,ALIGNTYP_P);
E const char *artifact_name(const char *,short *);
E boolean exist_artifact(int,const char *);
E void artifact_exists(struct obj *,const char *,BOOLEAN_P);
E int nartifact_exist(void);
E boolean spec_ability(struct obj *,unsigned long);
E boolean confers_luck(struct obj *);
E boolean arti_reflects(struct obj *);
E boolean restrict_name(struct obj *,const char *,BOOLEAN_P);
E boolean defends(int,struct obj *);
E boolean protects(int,struct obj *);
E void set_artifact_intrinsic(struct obj *,BOOLEAN_P,long);
E int touch_artifact(struct obj *,struct monst *);
E int spec_abon(struct obj *,struct monst *);
E int spec_dbon(struct obj *,struct monst *,int);
E void discover_artifact(XCHAR_P);
E boolean undiscovered_artifact(XCHAR_P);
E int disp_artifact_discoveries(winid);
E boolean artifact_hit(struct monst *,struct monst *, struct obj *,int *,int);
E int doinvoke(void);
E void arti_speak(struct obj *);
E boolean artifact_light(struct obj *);
E long spec_m2(struct obj *);
E boolean artifact_has_invprop(struct obj *,UCHAR_P);
E long arti_cost(struct obj *);
E boolean MATCH_WARN_OF_MON(struct monst *);
E const char *get_warned_of_monster(struct obj *);

/* ### attrib.c ### */

E boolean adjattrib(int,int,int);
E void change_luck(SCHAR_P);
E int stone_luck(BOOLEAN_P);
E boolean has_luckitem(void);
E void set_moreluck(void);
E void gainstr(struct obj *,int);
E void losestr(int);
E void restore_attrib(void);
E void exercise(int,BOOLEAN_P);
E void exerchk(void);
E void reset_attribute_clock(void);
E void init_attr(int);
E void redist_attr(void);
E void adjabil(int,int);
E int newhp(void);
E schar acurr(int);
E schar acurrstr(void);
E void adjalign(int);
E const char * beautiful(void);
E int uhp(void);
E int uhpmax(void);

/* ### ball.c ### */

E void ballfall(void);
E void placebc(void);
E void unplacebc(void);
E void set_bc(int);
E void move_bc(int,int,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E boolean drag_ball(XCHAR_P,XCHAR_P,
		int *,xchar *,xchar *,xchar *,xchar *, boolean *,BOOLEAN_P);
E void drop_ball(XCHAR_P,XCHAR_P);
E void drag_down(void);

/* ### bones.c ### */

E boolean can_make_bones(void);
E void savebones(struct obj *);
E int getbones(void);

/* ### botl.c ### */

E int xlev_to_rank(int);
E int title_to_mon(const char *,int *,int *);
E void max_rank_sz(void);
#ifdef SCORE_ON_BOTL
E long botl_score(void);
#endif
E int describe_level(char *);
E const char *rank(void);
E const char *rank_of(int,SHORT_P,BOOLEAN_P);
E void bot(void);
#ifdef DUMP_LOG
E void bot1str(char *);
E void bot2str(char *);
#endif

/* ### cmd.c ### */

#ifdef USE_TRAMPOLI
E int doextcmd(void);
E int domonability(void);
E int doprev_message(void);
E int timed_occupation(void);
E int wiz_attributes(void);
E int enter_explore_mode(void);
# ifdef WIZARD
E int wiz_detect(void);
E int wiz_genesis(void);
E int wiz_identify(void);
E int wiz_level_tele(void);
E int wiz_map(void);
E int wiz_where(void);
E int wiz_wish(void);
# endif /* WIZARD */
#endif /* USE_TRAMPOLI */
E void reset_occupations(void);
E void set_occupation(int (*)(void),const char *,int);
#ifdef REDO
E char pgetchar(void);
E void pushch(CHAR_P);
E void savech(CHAR_P);
#endif
#ifdef WIZARD
E void add_debug_extended_commands(void);
#endif /* WIZARD */
E void rhack(char *);
E int doextlist(void);
E int extcmd_via_menu(void);
E void enlightenment(int,BOOLEAN_P);
E void show_conduct(int,BOOLEAN_P);
E int xytod(SCHAR_P,SCHAR_P);
E void dtoxy(coord *,int);
E int movecmd(CHAR_P);
E int getdir(const char *);
E void confdir(void);
E int isok(int,int);
E int get_adjacent_loc(const char *, const char *, XCHAR_P, XCHAR_P, coord *);
E const char *click_to_cmd(int,int,int);
E char readchar(void);
#ifdef WIZARD
E void sanity_check(void);
#endif
E char yn_function(const char *, const char *, CHAR_P);
E char paranoid_yn(const char *, BOOLEAN_P);
E void sokoban_trickster(void);
E void list_vanquished(int, BOOLEAN_P);
E void list_vanquishedonly(void);

/* ### dbridge.c ### */

E boolean is_pool(int,int);
E boolean is_lava(int,int);
E boolean is_ice(int,int);
E boolean is_swamp(int,int);
E boolean is_icewall(int,int);
E boolean is_any_icewall(int,int);
E int is_drawbridge_wall(int,int);
E boolean is_db_wall(int,int);
E boolean find_drawbridge(int *,int*);
E boolean create_drawbridge(int,int,int,int);
E void open_drawbridge(int,int);
E boolean close_drawbridge(int,int);
E void destroy_drawbridge(int,int);

/* ### decl.c ### */

E void decl_init(void);

/* ### detect.c ### */

E struct obj *o_in(struct obj*,CHAR_P);
E struct obj *o_material(struct obj*,unsigned);
E int gold_detect(struct obj *);
E int food_detect(struct obj *);
E int object_detect(struct obj *,int,BOOLEAN_P);
E int monster_detect(struct obj *,int);
E int trap_detect(struct obj *);
E const char *level_distance(d_level *);
E void use_crystal_ball(struct obj *);
E void do_mapping(void);
E void do_vicinity_map(void);
E void cvt_sdoor_to_door(struct rm *);
#ifdef USE_TRAMPOLI
E void findone(int,int,genericptr_t);
E void openone(int,int,genericptr_t);
#endif
E int findit(void);
E int openit(void);
E void find_trap(struct trap *);
E int dosearch0(int);
E int dosearch(void);
E void sokoban_detect(void);

/* ### dig.c ### */

E boolean is_digging(void);
#ifdef USE_TRAMPOLI
E int dig(void);
#endif
E int holetime(void);
E schar fillholetyp(int, int);
E boolean dig_check(struct monst *, BOOLEAN_P, int, int);
E void digactualhole(int,int,struct monst *,int);
E boolean dighole(BOOLEAN_P);
E int use_pick_axe(struct obj *);
E int use_pick_axe2(struct obj *);
E boolean mdig_tunnel(struct monst *);
E void watch_dig(struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E void zap_dig(void);
E struct obj *bury_an_obj(struct obj *, boolean *);
E void bury_objs(int,int);
E void unearth_objs(int,int);
E void rot_organic(genericptr_t, long);
E void rot_corpse(genericptr_t, long);
#if 0
E void bury_monst(struct monst *);
E void bury_you(void);
E void unearth_you(void);
E void escape_tomb(void);
E void bury_obj(struct obj *);
#endif

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
E struct obj * vobj_at(XCHAR_P,XCHAR_P);
#endif /* INVISIBLE_OBJECTS */
E void magic_map_background(XCHAR_P,XCHAR_P,int);
E void map_background(XCHAR_P,XCHAR_P,int);
E void map_trap(struct trap *,int);
E void map_object(struct obj *,int);
E void map_invisible(XCHAR_P,XCHAR_P);
E void unmap_object(int,int);
E void map_location(int,int,int);
E void feel_location(XCHAR_P,XCHAR_P);
E void newsym(int,int);
E void shieldeff(XCHAR_P,XCHAR_P);
E void tmp_at(int,int);
E void swallowed(int);
E void under_ground(int);
E void under_water(int);
E void see_monsters(void);
E void set_mimic_blocking(void);
E void see_objects(void);
E void see_traps(void);
E void curs_on_u(void);
E int doredraw(void);
E void docrt(void);
E void show_glyph(int,int,int);
E void clear_glyph_buffer(void);
E void row_refresh(int,int,int);
E void cls(void);
E void flush_screen(int);
#ifdef DUMP_LOG
E void dump_screen(void);
#endif
E int back_to_glyph(XCHAR_P,XCHAR_P);
E int zapdir_to_glyph(int,int,int);
E int glyph_at(XCHAR_P,XCHAR_P);
E void set_wall_state(void);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
E int drop(struct obj *);
E int wipeoff(void);
#endif
E int dodrop(void);
E boolean boulder_hits_pool(struct obj *,int,int,BOOLEAN_P);
E boolean flooreffects(struct obj *,int,int,const char *);
E void doaltarobj(struct obj *);
E boolean canletgo(struct obj *,const char *);
E void dropx(struct obj *);
E void dropy(struct obj *);
E void obj_no_longer_held(struct obj *);
E int doddrop(void);
E int dodown(void);
E int doup(void);
#ifdef INSURANCE
E void save_currentstate(void);
#endif
E void goto_level(d_level *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
E void schedule_goto(d_level *,BOOLEAN_P,BOOLEAN_P,int,
			     const char *,const char *);
E void deferred_goto(void);
E boolean revive_corpse(struct obj *);
E void revive_mon(genericptr_t, long);
E int donull(void);
E int dowipe(void);
E void set_wounded_legs(long,int);
E void heal_legs(void);

/* ### do_name.c ### */

E void do_oname(struct obj *);
E int getpos(coord *,BOOLEAN_P,const char *);
E struct monst *christen_monst(struct monst *,const char *);
E int do_mname(void);
E struct obj *oname(struct obj *,const char *);
E int ddocall(void);
E void docall(struct obj *);
E void docall_input(int);
E const char *rndghostname(void);
E char *x_monnam(struct monst *,int,const char *,int,BOOLEAN_P);
E char *l_monnam(struct monst *);
E char *mon_nam(struct monst *);
E char *noit_mon_nam(struct monst *);
E char *Monnam(struct monst *);
E char *noit_Monnam(struct monst *);
E char *m_monnam(struct monst *);
E char *y_monnam(struct monst *);
E char *Adjmonnam(struct monst *,const char *);
E char *Amonnam(struct monst *);
E char *a_monnam(struct monst *);
E char *distant_monnam(struct monst *,int,char *);
E const char *rndmonnam(void);
E const char *hcolor(const char *);
E const char *rndcolor(void);
#ifdef REINCARNATION
E const char *roguename(void);
#endif
E struct obj *realloc_obj(struct obj *, int, genericptr_t, int, const char *);
E char *coyotename(struct monst *,char *);

/* ### do_wear.c ### */

E int lucky_fedora(void);
#ifdef USE_TRAMPOLI
E int Armor_on(void);
E int Boots_on(void);
E int Gloves_on(void);
E int Helmet_on(void);
E int select_off(struct obj *);
E int take_off(void);
#endif
E void off_msg(struct obj *);
E void set_wear(void);
E boolean donning(struct obj *);
E void cancel_don(void);
E int Armor_off(void);
E int Armor_gone(void);
E int Helmet_off(void);
E int Gloves_off(void);
E int Boots_off(void);
E int Cloak_off(void);
E int Shield_off(void);
#ifdef TOURIST
E int Shirt_off(void);
#endif
E void Amulet_off(void);
E void Ring_on(struct obj *);
E void Ring_off(struct obj *);
E void Ring_gone(struct obj *);
E void Blindf_on(struct obj *);
E void Blindf_off(struct obj *);
E int dotakeoff(void);
E int doremring(void);
E int cursed(struct obj *);
E int armoroff(struct obj *);
E int canwearobj(struct obj *, long *, BOOLEAN_P);
E int dowear(void);
E int doputon(void);
E void find_ac(void);
E void glibr(void);
E struct obj *some_armor(struct monst *);
E void erode_armor(struct monst *,BOOLEAN_P);
E struct obj *stuck_ring(struct obj *,int);
E struct obj *unchanger(void);
E void reset_remarm(void);
E int doddoremarm(void);
E int destroy_arm(struct obj *);
E void adj_abon(struct obj *,SCHAR_P);

/* ### dog.c ### */

E void initedog(struct monst *);
E struct monst *make_familiar(struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E struct monst *makedog(void);
E void update_mlstmv(void);
E void losedogs(void);
E void mon_arrive(struct monst *,BOOLEAN_P);
E void mon_catchup_elapsed_time(struct monst *,long);
E void keepdogs(BOOLEAN_P);
E void migrate_to_level(struct monst *,XCHAR_P,XCHAR_P,coord *);
E int dogfood(struct monst *,struct obj *);
E struct monst *tamedog(struct monst *,struct obj *);
E void abuse_dog(struct monst *);
E void wary_dog(struct monst *, BOOLEAN_P);

/* ### dogmove.c ### */

E int dog_nutrition(struct monst *,struct obj *);
E int dog_eat(struct monst *,struct obj *,int,int,BOOLEAN_P);
E int dog_move(struct monst *,int);
#ifdef USE_TRAMPOLI
E void wantdoor(int,int,genericptr_t);
#endif

/* ### dokick.c ### */

E boolean ghitm(struct monst *,struct obj *);
E void container_impact_dmg(struct obj *);
E int dokick(void);
E boolean ship_object(struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E void obj_delivery(void);
E schar down_gate(XCHAR_P,XCHAR_P);
E void impact_drop(struct obj *,XCHAR_P,XCHAR_P,XCHAR_P);

/* ### dothrow.c ### */

E int dothrow(void);
E int dofire(void);
E void hitfloor(struct obj *);
E void hurtle(int,int,int,BOOLEAN_P);
E void mhurtle(struct monst *,int,int,int);
E void throwit(struct obj *,long,BOOLEAN_P);
E int omon_adj(struct monst *,struct obj *,BOOLEAN_P);
E int thitmonst(struct monst *,struct obj *);
E int hero_breaks(struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E int breaks(struct obj *,XCHAR_P,XCHAR_P);
E boolean breaktest(struct obj *);
E boolean walk_path(coord *, coord *, boolean (*)(genericptr_t,int,int), genericptr_t);
E boolean hurtle_step(genericptr_t, int, int);

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E int def_char_to_objclass(CHAR_P);
E int def_char_to_monclass(CHAR_P);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void assign_graphics(glyph_t *,int,int,int);
E void switch_graphics(int);
#ifdef REINCARNATION
E void assign_rogue_graphics(BOOLEAN_P);
#endif
E void assign_utf8graphics_symbol(int, glyph_t);
E void assign_moria_graphics(BOOLEAN_P);

/* ### dump.c ### */

E void dump(const char *, const char *);
E void dump_blockquote_start(void);
E void dump_blockquote_end(void);
E void dump_text(const char *, const char *);
E void dump_header_html(const char *);
E void dump_html(const char *, const char *);
E void dump_init(void);
E void dump_exit(void);
E void dump_object(const char, const struct obj *, const char *);
E void dump_title(char *);
E void dump_subtitle(const char *);
E void dump_line(const char *, const char *);
E void dump_list_start(void);
E void dump_list_item(const char *);
E void dump_list_item_object(struct obj *);
E void dump_list_item_link(const char *, const char *);
E void dump_list_end(void);
E void dump_definition_list_start(void);
E void dump_definition_list_dd(const char *);
E void dump_definition_list_dt(const char *);
E void dump_definition_list_end(void);
E void dump_containerconts(struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
E char* html_escape_character(const char);
E char* html_link(const char *, const char *);
#ifdef DUMP_LOG
E int dump_screenshot(void);
E int dumpoverview(void);
#endif

/* ### dungeon.c ### */

E void save_dungeon(int,BOOLEAN_P,BOOLEAN_P);
E void restore_dungeon(int);
E void insert_branch(branch *,BOOLEAN_P);
E void init_dungeons(void);
E s_level *find_level(const char *);
E s_level *Is_special(d_level *);
#ifdef RANDOMIZED_PLANES
E s_level *get_next_elemental_plane(d_level *);
E d_level *get_first_elemental_plane(void);
#endif
E branch *Is_branchlev(d_level *);
E xchar ledger_no(d_level *);
E xchar maxledgerno(void);
E schar depth(d_level *);
E xchar dunlev(d_level *);
E xchar dunlevs_in_dungeon(d_level *);
E xchar ledger_to_dnum(XCHAR_P);
E xchar ledger_to_dlev(XCHAR_P);
E xchar deepest_lev_reached(BOOLEAN_P);
E boolean on_level(d_level *,d_level *);
E void next_level(BOOLEAN_P);
E void prev_level(BOOLEAN_P);
E void u_on_newpos(int,int);
E void u_on_sstairs(void);
E void u_on_upstairs(void);
E void u_on_dnstairs(void);
E boolean On_stairs(XCHAR_P,XCHAR_P);
E void get_level(d_level *,int);
E boolean Is_botlevel(d_level *);
E boolean Can_fall_thru(d_level *);
E boolean Can_dig_down(d_level *);
E boolean Can_rise_up(int,int,d_level *);
E boolean In_quest(d_level *);
E boolean In_mines(d_level *);
E boolean In_sheol(d_level *);
E branch *dungeon_branch(const char *);
E boolean at_dgn_entrance(const char *);
E boolean In_hell(d_level *);
E boolean In_V_tower(d_level *);
E boolean On_W_tower_level(d_level *);
E boolean In_W_tower(int,int,d_level *);
E void find_hell(d_level *);
E void goto_hell(BOOLEAN_P,BOOLEAN_P);
E void assign_level(d_level *,d_level *);
E void assign_rnd_level(d_level *,d_level *,int);
E int induced_align(int);
E boolean Invocation_lev(d_level *);
E xchar level_difficulty(void);
E schar lev_by_name(const char *);
#ifdef WIZARD
E schar print_dungeon(BOOLEAN_P,schar *,xchar *);
#endif
E int donamelevel(void);
E int dooverview(void);
E void forget_mapseen(int);
E void init_mapseen(d_level *);
E void recalc_mapseen(void);
E void recbranch_mapseen(d_level *, d_level *);
E void remdun_mapseen(int);
E char *get_annotation(d_level *);
E const char *get_generic_level_description(d_level *);

/* ### eat.c ### */

#ifdef USE_TRAMPOLI
E int eatmdone(void);
E int eatfood(void);
E int opentin(void);
E int unfaint(void);
#endif
E boolean is_edible(struct obj *);
E void init_uhunger(void);
E int Hear_again(void);
E void reset_eat(void);
E int doeat(void);
E void gethungry(void);
E void morehungry(int);
E void lesshungry(int);
E boolean is_fainted(void);
E void reset_faint(void);
E void violated_vegetarian(void);
#if 0
E void sync_hunger(void);
#endif
E void newuhs(BOOLEAN_P);
E struct obj *floorfood(const char *,int);
E void vomit(void);
E int eaten_stat(int,struct obj *);
E void food_disappears(struct obj *);
E void food_substitution(struct obj *,struct obj *);
E boolean bite_monster(struct monst *mon);
E void fix_petrification(void);
E void consume_oeaten(struct obj *,int);
E boolean maybe_finished_meal(BOOLEAN_P);

/* ### end.c ### */

E void done1(int);
E int done2(void);
#ifdef USE_TRAMPOLI
E void done_intr(int);
#endif
E void done_in_by(struct monst *);
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E void panic(const char *,...) PRINTF_F(1,2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void done(int);
E void container_contents(struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
E void terminate(int);
E int num_genocides(void);

/* ### engrave.c ### */

E char *random_engraving(char *);
E void wipeout_text(char *,int,unsigned);
E boolean can_reach_floor(void);
E const char *surface(int,int);
E const char *ceiling(int,int);
E struct engr *engr_at(XCHAR_P,XCHAR_P);
#ifdef ELBERETH
E int sengr_at(const char *,XCHAR_P,XCHAR_P);
#endif
E void u_wipe_engr(int);
E void wipe_engr_at(XCHAR_P,XCHAR_P,XCHAR_P);
E void read_engr_at(int,int);
E void make_engr_at(int,int,const char *,long,XCHAR_P);
E void del_engr_at(int,int);
E int freehand(void);
E int doengrave(void);
E int doengrave_elbereth(void);
E void save_engravings(int,int);
E void rest_engravings(int);
E void del_engr(struct engr *);
E void rloc_engr(struct engr *);
E void make_grave(int,int,const char *);

/* ### exper.c ### */

E long newuexp(int);
E int experience(struct monst *,int);
E void more_experienced(int,int,int);
E void losexp(const char *);
E void newexplevel(void);
E void pluslvl(BOOLEAN_P);
E long rndexp(BOOLEAN_P);

/* ### explode.c ### */

E void explode(int,int,int,int,CHAR_P,int);
E long scatter(int, int, int, unsigned int, struct obj *);
E void splatter_burning_oil(int, int);

/* ### extralev.c ### */

#ifdef REINCARNATION
E void makeroguerooms(void);
E void corr(int,int);
E void makerogueghost(void);
#endif

/* ### files.c ### */

E char *fname_encode(const char *, CHAR_P, char *, char *, int);
E char *fname_decode(CHAR_P, char *, char *, int);
E const char *fqname(const char *, int, int);
#ifndef FILE_AREAS
E FILE *fopen_datafile(const char *,const char *,int);
#endif
E boolean uptodate(int,const char *);
E void store_version(int);
#ifdef MFLOPPY
E void set_lock_and_bones(void);
#endif
E void set_levelfile_name(char *,int);
E int create_levelfile(int,char *);
E int open_levelfile(int,char *);
E void delete_levelfile(int);
E void clearlocks(void);
E int create_bonesfile(d_level*,char **, char *);
#ifdef MFLOPPY
E void cancel_bonesfile(void);
#endif
E void commit_bonesfile(d_level *);
E int open_bonesfile(d_level*,char **);
E int delete_bonesfile(d_level*);
E void compress_bonesfile(void);
E void set_savefile_name(void);
#ifdef INSURANCE
E void save_savefile_name(int);
#endif
#if defined(WIZARD) && !defined(MICRO)
E void set_error_savefile(void);
#endif
E int create_savefile(void);
E int open_savefile(void);
E int delete_savefile(void);
E int restore_saved_game(void);
E void compress(const char *);
E void uncompress(const char *);
E void compress_area(const char *, const char *);
E void uncompress_area(const char *, const char *);
#ifndef FILE_AREAS
E boolean lock_file(const char *,int,int);
E void unlock_file(const char *);
#endif
#ifdef USER_SOUNDS
E boolean can_read_file(const char *);
#endif
E void read_config_file(const char *);
E void check_recordfile(const char *);
#if defined(WIZARD)
E void read_wizkit(void);
#endif
E void paniclog(const char *, const char *);
E int validate_prefix_locations(char *);
E char** get_saved_games(void);
E void free_saved_games(char**);
#ifdef SELF_RECOVER
E boolean recover_savefile(void);
#endif
#ifdef HOLD_LOCKFILE_OPEN
E void really_close(void);
#endif
#ifdef WHEREIS_FILE
E void touch_whereis(void);
E void delete_whereis(void);
E void signal_whereis(int);
#endif

/* ### fountain.c ### */

E void floating_above(const char *);
E void dogushforth(int);
# ifdef USE_TRAMPOLI
E void gush(int,int,genericptr_t);
# endif
E void dryup(XCHAR_P,XCHAR_P, BOOLEAN_P);
E void drinkfountain(void);
E void dipfountain(struct obj *);
#ifdef SINKS
E void breaksink(int,int);
E void drinksink(void);
#endif

/* ### hack.c ### */

#ifdef DUNGEON_GROWTH
E void catchup_dgn_growths(int);
E void dgn_growths(BOOLEAN_P,BOOLEAN_P);
#endif
E boolean revive_nasty(int,int,const char*);
E void movobj(struct obj *,XCHAR_P,XCHAR_P);
E boolean may_dig(XCHAR_P,XCHAR_P);
E boolean may_passwall(XCHAR_P,XCHAR_P);
E boolean bad_rock(struct permonst *,XCHAR_P,XCHAR_P);
E boolean invocation_pos(XCHAR_P,XCHAR_P);
E boolean test_move(int, int, int, int, int);
E void domove(void);
E void invocation_message(void);
E void wounds_message(struct monst *);
E char *mon_wounds(struct monst *);
E void spoteffects(BOOLEAN_P);
E char *in_rooms(XCHAR_P,XCHAR_P,int);
E boolean in_town(int,int);
E void check_special_room(BOOLEAN_P);
E int dopickup(void);
E void lookaround(void);
E int monster_nearby(void);
E void nomul(int, const char *);
E void unmul(const char *);
E void showdmg(int, BOOLEAN_P);
E void losehp(int,const char *,BOOLEAN_P);
E void losehp_how(int,const char *,BOOLEAN_P,int);
E void set_uhpmax(int,BOOLEAN_P);
E void check_uhpmax(void);
E int weight_cap(void);
E int inv_weight(void);
E int near_capacity(void);
E int calc_capacity(int);
E int max_capacity(void);
E boolean check_capacity(const char *);
E int inv_cnt(void);
#ifdef GOLDOBJ
E long money_cnt(struct obj *);
#endif

/* ### hacklib.c ### */

E boolean digit(CHAR_P);
E boolean letter(CHAR_P);
E char highc(CHAR_P);
E char lowc(CHAR_P);
E char *lcase(char *);
E char *upstart(char *);
E char *mungspaces(char *);
E char *eos(char *);
E void sanitizestr(char *);
E char *strkitten(char *,CHAR_P);
E char *s_suffix(const char *);
E char *xcrypt(const char *,char *);
E boolean onlyspace(const char *);
E char *tabexpand(char *);
E char *visctrl(CHAR_P);
E const char *ordin(int);
E char *sitoa(int);
E int sgn(int);
E int rounddiv(long,int);
E int dist2(int,int,int,int);
E int isqrt(int);
E int distmin(int,int,int,int);
E boolean online2(int,int,int,int);
E boolean pmatch(const char *,const char *);
#ifndef STRNCMPI
E int strncmpi(const char *,const char *,int);
#endif
#ifndef STRSTRI
E char *strstri(const char *,const char *);
#endif
E boolean fuzzymatch(const char *,const char *,const char *,BOOLEAN_P);
E void init_random(unsigned int);
E void reseed_random(void);
E void set_random_state(unsigned int);
E int getyear(void);
E int getmonth(void);
E int getmday(void);
#if 0
E char *yymmdd(time_t);
#endif
E long yyyymmdd(time_t);
E int phase_of_the_moon(void);
E boolean friday_13th(void);
E boolean towelday(void);
E boolean pirateday(void);
E int night(void);
E int midnight(void);
E boolean piday(void);
E boolean aprilfoolsday(void);
E boolean discordian_holiday(void);
E char *iso8601(time_t);
E char *iso8601_duration(long);
E char *get_formatted_time(time_t, const char *);
E time_t current_epoch(void);

/* ### invent.c ### */

E void assigninvlet(struct obj *);
E struct obj *merge_choice(struct obj *,struct obj *);
E int merged(struct obj **,struct obj **);
#ifdef USE_TRAMPOLI
E int ckunpaid(struct obj *);
#endif
E void addinv_core1(struct obj *);
E void addinv_core2(struct obj *);
E struct obj *addinv(struct obj *);
E struct obj *hold_another_object(struct obj *,const char *,const char *,const char *);
E void useupall(struct obj *);
E void useup(struct obj *);
E void consume_obj_charge(struct obj *,BOOLEAN_P);
E void freeinv_core(struct obj *);
E void freeinv(struct obj *);
E void delallobj(int,int);
E void delobj(struct obj *);
E struct obj *sobj_at(int,int,int);
E struct obj *carrying(int);
E boolean have_lizard(void);
E struct obj *o_on(unsigned int,struct obj *);
E boolean obj_here(struct obj *,int,int);
E boolean wearing_armor(void);
E boolean is_worn(struct obj *);
E struct obj *g_at(int,int);
E struct obj *mkgoldobj(long);
E struct obj *getobj(const char *,const char *);
E int ggetobj(const char *,int (*)(OBJ_P),int,BOOLEAN_P,unsigned *);
E void fully_identify_obj(struct obj *);
E int identify(struct obj *);
E void identify_pack(int);
E int askchain(struct obj **,const char *,int,int (*)(OBJ_P),
			int (*)(OBJ_P),int,const char *);
E void prinv(const char *,struct obj *,long);
E char *xprname(struct obj *,const char *,CHAR_P,BOOLEAN_P,long,long);
E int ddoinv(void);
E char display_inventory(const char *,BOOLEAN_P);
E char dump_inventory(const char *,BOOLEAN_P,BOOLEAN_P);
E int display_binventory(int,int,BOOLEAN_P);
E struct obj *display_cinventory(struct obj *);
E struct obj *display_minventory(struct monst *,int,char *);
E int dotypeinv(void);
E const char *dfeature_at(int,int,char *);
E int look_here(int,BOOLEAN_P);
E int dolook(void);
E boolean will_feel_cockatrice(struct obj *,BOOLEAN_P);
E void feel_cockatrice(struct obj *,BOOLEAN_P);
E void stackobj(struct obj *);
E int doprgold(void);
E int doprwep(void);
E int doprarm(void);
E int doprring(void);
E int dopramulet(void);
E int doprtool(void);
E int doprinuse(void);
E void useupf(struct obj *,long);
E char *let_to_name(CHAR_P,BOOLEAN_P);
E void free_invbuf(void);
E void reassign(void);
E int doorganize(void);
E int count_objects(struct obj *);
E int count_unidentified(struct obj *);
E int count_unpaid(struct obj *);
E int count_buc(struct obj *,int);
E void carry_obj_effects(struct obj *);
E const char *currency(long);
E void silly_thing(const char *,struct obj *);
E struct obj *getnextgetobj(void);
#ifdef SORTLOOT
E int sortloot_cmp(struct obj *, struct obj *);
#endif
E boolean is_racial_armor(struct obj *);
E boolean is_racial_weapon(struct obj *);
E void identify_dragon(int);

/* ### ioctl.c ### */

#if defined(UNIX) || defined(__BEOS__)
E void getwindowsz(void);
E void getioctls(void);
E void setioctls(void);
# ifdef SUSPEND
E int dosuspend(void);
# endif /* SUSPEND */
#endif /* UNIX || __BEOS__ */

/* ### light.c ### */

E void new_light_source(XCHAR_P, XCHAR_P, int, int, genericptr_t);
E void del_light_source(int, genericptr_t);
E void do_light_sources(char **);
E struct monst *find_mid(unsigned, unsigned);
E void save_light_sources(int, int, int);
E void restore_light_sources(int);
E void relink_light_sources(BOOLEAN_P);
E void obj_move_light_source(struct obj *, struct obj *);
E boolean any_light_source(void);
E void snuff_light_source(int, int);
E boolean obj_sheds_light(struct obj *);
E boolean obj_is_burning(struct obj *);
E void obj_split_light_source(struct obj *, struct obj *);
E void obj_merge_light_sources(struct obj *,struct obj *);
E int candle_light_range(struct obj *);
#ifdef WIZARD
E int wiz_light_sources(void);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
E int forcelock(void);
E int picklock(void);
#endif
E boolean picking_lock(int *,int *);
E boolean picking_at(int,int);
E void reset_pick(void);
E int pick_lock(struct obj *,int,int,BOOLEAN_P);
E int doforce(void);
E boolean boxlock(struct obj *,struct obj *);
E boolean doorlock(struct obj *,int,int);
E int doopen(void);
E int doclose(void);
E int artifact_door(int,int);
#ifdef AUTO_OPEN
E  int doopen_indir(int,int);
#endif

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

E int maccreat(const char *,long);
E int macopen(const char *,int,long);
E int macclose(int);
E int macread(int,void *,unsigned);
E int macwrite(int,void *,unsigned);
E long macseek(int,long,short);
E int macunlink(const char *);

/* ### macsnd.c ### */

E void mac_speaker(struct obj *,char *);

/* ### macunix.c ### */

E void regularize(char *);
E void getlock(void);

/* ### macwin.c ### */

E void lock_mouse_cursor(Boolean);
E int SanePositions(void);

/* ### mttymain.c ### */

E void getreturn(char *);
E void msmsg(const char *,...);
E void gettty(void);
E void setftty(void);
E void settty(const char *);
E int tgetch(void);
E void cmov(int x, int y);
E void nocmov(int x, int y);

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
E void getmailstatus(void);
# endif
E void ckmailstatus(void);
E void read_hint(struct obj *);
E void readmail(struct obj *);
E void maybe_hint(void);
#endif /* MAIL */

/* ### makemon.c ### */

E boolean is_home_elemental(struct permonst *);
E struct monst *clone_mon(struct monst *,XCHAR_P,XCHAR_P);
E struct monst *makemon(struct permonst *,int,int,int);
E boolean create_critters(int,struct permonst *);
E struct permonst *rndmonst(void);
E void reset_rndmonst(int);
E struct permonst *mkclass(CHAR_P,int);
E int adj_lev(struct permonst *);
E struct permonst *grow_up(struct monst *,struct monst *);
E int mongets(struct monst *,int);
E int golemhp(int);
E boolean peace_minded(struct permonst *);
E void set_malign(struct monst *);
E void set_mimic_sym(struct monst *);
E int mbirth_limit(int);
E void mimic_hit_msg(struct monst *, SHORT_P);
#ifdef GOLDOBJ
E void mkmonmoney(struct monst *, long);
#endif
E int bagotricks(struct obj *);
E boolean propagate(int, BOOLEAN_P,BOOLEAN_P);
E void create_camera_demon(struct obj *,int,int);
E int min_monster_difficulty(void);
E int max_monster_difficulty(void);
E boolean prohibited_by_generation_flags(struct permonst *);

/* ### mapglyph.c ### */

E void mapglyph(int, glyph_t *, int *, unsigned *, int, int);

/* ### mcastu.c ### */

E int castmu(struct monst *,struct attack *,BOOLEAN_P,BOOLEAN_P);
E int buzzmu(struct monst *,struct attack *);

/* ### mhitm.c ### */

E int fightm(struct monst *);
E int mattackm(struct monst *,struct monst *);
E int noattacks(struct permonst *);
E int sleep_monst(struct monst *,int,int);
E void slept_monst(struct monst *);
E long attk_protection(int);
E void maybe_freeze_m(struct monst *,int,int*);

/* ### mhitu.c ### */

E const char *mpoisons_subj(struct monst *,struct attack *);
E void u_slow_down(void);
E struct monst *cloneu(void);
E void expels(struct monst *,struct permonst *,BOOLEAN_P);
E struct attack *getmattk(struct permonst *,int,int *,struct attack *);
E int mattacku(struct monst *);
E int magic_negation(struct monst *);
E int gazemu(struct monst *,struct attack *);
E void mdamageu(struct monst *,int);
E int could_seduce(struct monst *,struct monst *,struct attack *);
#ifdef SEDUCE
E int doseduce(struct monst *);
#endif
E void maybe_freeze_u(int*);
E void spore_dies(struct monst *);

/* ### minion.c ### */

E void msummon(struct monst *);
E void summon_minion(ALIGNTYP_P,BOOLEAN_P);
E int demon_talk(struct monst *);
E long bribe(struct monst *);
E int dprince(ALIGNTYP_P);
E int dlord(ALIGNTYP_P);
E int llord(void);
E int ndemon(ALIGNTYP_P);
E int lminion(void);

/* ### mklev.c ### */

E void mineralize(int, int, int, int, boolean);
#ifdef USE_TRAMPOLI
E int do_comp(genericptr_t,genericptr_t);
#endif
E void sort_rooms(void);
E void add_room(int,int,int,int,BOOLEAN_P,SCHAR_P,BOOLEAN_P);
E void add_subroom(struct mkroom *,int,int,int,int, BOOLEAN_P,SCHAR_P,BOOLEAN_P);
E void makecorridors(int);
E void add_door(int,int,struct mkroom *);
E void mkpoolroom(void);
E void mklev(void);
#ifdef SPECIALIZATION
E void topologize(struct mkroom *,BOOLEAN_P);
#else
E void topologize(struct mkroom *);
#endif
E void place_branch(branch *,XCHAR_P,XCHAR_P);
E boolean occupied(XCHAR_P,XCHAR_P);
E int okdoor(XCHAR_P,XCHAR_P);
E void dodoor(int,int,struct mkroom *);
E void mktrap(int,int,struct mkroom *,coord*);
E void mkstairs(XCHAR_P,XCHAR_P,CHAR_P,struct mkroom *);
E void mkinvokearea(void);
E void wallwalk_right(XCHAR_P,XCHAR_P,SCHAR_P,SCHAR_P,SCHAR_P,int);
#ifdef ADVENT_CALENDAR
E boolean mk_advcal_portal(void);
#endif

/* ### mkmap.c ### */

void flood_fill_rm(int,int,int,BOOLEAN_P,BOOLEAN_P);
void remove_rooms(int,int,int,int);

/* ### mkmaze.c ### */

E void wallification(int,int,int,int);
E void wall_extends(int,int,int,int);
E void walkfrom(int,int, SCHAR_P);
E void makemaz(const char *);
E void mazexy(coord *);
E void bound_digging(void);
E void mkportal(XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E boolean bad_location(XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E int place_lregion(XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,d_level *);
E void movebubbles(void);
E void water_friction(void);
E void save_waterlevel(int,int);
E void restore_waterlevel(int);
E const char *waterbody_name(XCHAR_P,XCHAR_P);

#ifdef ADVENT_CALENDAR
E void fill_advent_calendar(boolean);
#endif

/* ### mkobj.c ### */

E struct obj *mkobj_at(CHAR_P,int,int,BOOLEAN_P);
E struct obj *mksobj_at(int,int,int,BOOLEAN_P,BOOLEAN_P);
E struct obj *mkobj(CHAR_P,BOOLEAN_P);
E int rndmonnum(void);
E struct obj *splitobj(struct obj *,long);
E void replace_object(struct obj *,struct obj *);
E void bill_dummy_object(struct obj *);
E struct obj *mksobj(int,BOOLEAN_P,BOOLEAN_P);
E int bcsign(struct obj *);
E int weight(struct obj *);
E struct obj *mkgold(long,int,int);
E struct obj *mkcorpstat(int,struct monst *,struct permonst *,int,int,BOOLEAN_P);
E struct obj *obj_attach_mid(struct obj *, unsigned);
E struct monst *get_mtraits(struct obj *, BOOLEAN_P);
E struct obj *mk_tt_object(int,int,int);
E struct obj *mk_named_object(int,struct permonst *,int,int,const char *);
E struct obj *rnd_treefruit_at(int, int);
E void rnd_treesticks_at(int, int);
E void start_corpse_timeout(struct obj *);
E void bless(struct obj *);
E void unbless(struct obj *);
E void curse(struct obj *);
E void uncurse(struct obj *);
E void blessorcurse(struct obj *,int);
E boolean is_flammable(struct obj *);
E boolean is_rottable(struct obj *);
E void place_object(struct obj *,int,int);
E void remove_object(struct obj *);
E void discard_minvent(struct monst *);
E void obj_extract_self(struct obj *);
E void extract_nobj(struct obj *, struct obj **);
E void extract_nexthere(struct obj *, struct obj **);
E int add_to_minv(struct monst *, struct obj *);
E struct obj *add_to_container(struct obj *, struct obj *);
E void add_to_migration(struct obj *);
E void add_to_buried(struct obj *);
E void dealloc_obj(struct obj *);
E void obj_ice_effects(int, int, BOOLEAN_P);
E long peek_at_iced_corpse_age(struct obj *);
#ifdef WIZARD
E void obj_sanity_check(void);
#endif

/* ### mkroom.c ### */

E struct mkroom * pick_room(BOOLEAN_P);
E void mkroom(int);
E void fill_zoo(struct mkroom *);
E boolean nexttodoor(int,int);
E boolean bydoor(int,int);
E boolean somexyspace(struct mkroom *,coord *,int);
E boolean has_dnstairs(struct mkroom *);
E boolean has_upstairs(struct mkroom *);
E int somex(struct mkroom *);
E int somey(struct mkroom *);
E boolean inside_room(struct mkroom *,XCHAR_P,XCHAR_P);
E boolean somexy(struct mkroom *,coord *);
E void mkundead(coord *,BOOLEAN_P,int);
E struct permonst *courtmon(void);
E void save_rooms(int);
E void rest_rooms(int);
E struct mkroom *search_special(SCHAR_P);

/* #### mksheol.c ### */

E void mksheol(void*);

/* ### mon.c ### */

E int select_newcham_form(struct monst *);
E void remove_monster(int, int);
E int undead_to_corpse(int);
E int genus(int,int);
E int pm_to_cham(int);
E int minliquid(struct monst *);
E int movemon(void);
E int meatmetal(struct monst *);
E int meatobj(struct monst *);
E void mpickgold(struct monst *);
E boolean mpickstuff(struct monst *,const char *);
E void mpickup_obj(struct monst *, struct obj *);
E int curr_mon_load(struct monst *);
E int max_mon_load(struct monst *);
E boolean can_carry(struct monst *,struct obj *);
E int mfndpos(struct monst *,coord *,long *,long);
E boolean monnear(struct monst *,int,int);
E void dmonsfree(void);
E int mcalcmove(struct monst*);
E void mcalcdistress(void);
E void replmon(struct monst *,struct monst *);
E void relmon(struct monst *);
E struct obj *mlifesaver(struct monst *);
E boolean corpse_chance(struct monst *,struct monst *,BOOLEAN_P);
#ifdef WEBB_DISINT
E void mondead_helper(struct monst *, uchar);
#endif
E void mondead(struct monst *);
E void mondied(struct monst *);
E void mongone(struct monst *);
E void monstone(struct monst *);
E void monkilled(struct monst *,const char *,int);
E void unstuck(struct monst *);
E void killed(struct monst *);
E void xkilled(struct monst *,int);
E void mon_to_stone(struct monst*);
E void mnexto(struct monst *);
E boolean mnearto(struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E void poisontell(int);
E void poisoned(const char *,int,const char *,int);
E void m_respond(struct monst *);
E void setmangry(struct monst *);
E void wakeup(struct monst *);
E void wake_nearby(void);
E void wake_nearto(int,int,int);
E void seemimic(struct monst *);
E void rescham(void);
E void restartcham(void);
E void restore_cham(struct monst *);
E void mon_animal_list(BOOLEAN_P);
E int newcham(struct monst *,struct permonst *,BOOLEAN_P,BOOLEAN_P);
E int can_be_hatched(int);
E int egg_type_from_parent(int,BOOLEAN_P);
E boolean dead_species(int,BOOLEAN_P);
E void kill_genocided_monsters(void);
E void kill_monster_on_level(int);
E void golemeffects(struct monst *,int,int);
E boolean angry_guards(BOOLEAN_P);
E void pacify_guards(void);

/* ### mondata.c ### */

E void set_mon_data(struct monst *,struct permonst *,int);
E struct attack *attacktype_fordmg(struct permonst *,int,int);
E boolean attacktype(struct permonst *,int);
E boolean poly_when_stoned(struct permonst *);
E boolean resists_drli(struct monst *);
E boolean resists_magm(struct monst *);
E boolean resists_blnd(struct monst *);
E boolean can_blnd(struct monst *,struct monst *,UCHAR_P,struct obj *);
E boolean ranged_attk(struct permonst *);
E boolean hates_silver(struct permonst *);
E boolean passes_bars(struct permonst *);
E boolean can_track(struct permonst *);
E boolean breakarm(struct permonst *);
E boolean sliparm(struct permonst *);
E boolean sticks(struct permonst *);
E int num_horns(struct permonst *);
/* E boolean canseemon(struct monst *); */
E struct attack *dmgtype_fromattack(struct permonst *,int,int);
E boolean dmgtype(struct permonst *,int);
E int max_passive_dmg(struct monst *,struct monst *);
E int monsndx(struct permonst *);
E int name_to_mon(const char *);
E int gender(struct monst *);
E int pronoun_gender(struct monst *);
E boolean levl_follower(struct monst *);
E int little_to_big(int);
E int big_to_little(int);
E const char *locomotion(const struct permonst *,const char *);
E const char *stagger(const struct permonst *,const char *);
E const char *on_fire(struct permonst *,struct attack *);
E const struct permonst *raceptr(struct monst *);

/* ### monmove.c ### */

E boolean itsstuck(struct monst *);
E boolean mb_trapped(struct monst *);
E void mon_regen(struct monst *,BOOLEAN_P);
E int dochugw(struct monst *);
E boolean onscary(int,int,struct monst *);
E void monflee(struct monst *, int, BOOLEAN_P, BOOLEAN_P);
E int dochug(struct monst *);
E int m_move(struct monst *,int);
E boolean closed_door(int,int);
E boolean accessible(int,int);
E void set_apparxy(struct monst *);
E boolean can_ooze(struct monst *);

/* ### monst.c ### */

E void monst_init(void);

/* ### monstr.c ### */

E void monstr_init(void);

/* ### mplayer.c ### */

E struct monst *mk_mplayer(struct permonst *,XCHAR_P, XCHAR_P,BOOLEAN_P);
E void create_mplayers(int,BOOLEAN_P);
E void mplayer_talk(struct monst *);

#if defined(MICRO) || defined(WIN32)

/* ### msdos.c,os2.c,tos.c,winnt.c ### */

#  ifndef WIN32
E int tgetch(void);
#  endif
#  ifndef TOS
E char switchar(void);
#  endif
# ifndef __GO32__
E long freediskspace(char *);
#  ifdef MSDOS
E int findfirst_file(char *);
E int findnext_file(void);
E long filesize_nh(char *);
#  else
E int findfirst(char *);
E int findnext(void);
E long filesize(char *);
#  endif /* MSDOS */
E char *foundfile_buffer(void);
# endif /* __GO32__ */
E void chdrive(char *);
# ifndef TOS
E void disable_ctrlP(void);
E void enable_ctrlP(void);
# endif
# if defined(MICRO) && !defined(WINNT)
E void get_scr_size(void);
#  ifndef TOS
E void gotoxy(int,int);
#  endif
# endif
# ifdef TOS
E int _copyfile(char *,char *);
E int kbhit(void);
E void set_colors(void);
E void restore_colors(void);
#  ifdef SUSPEND
E int dosuspend(void);
#  endif
# endif /* TOS */
# ifdef WIN32
E char *get_username(int *);
E void nt_regularize(char *);
E int (*nt_kbhit)(void);
E void Delay(int);
# endif /* WIN32 */
#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

E int thitu(int,int,struct obj *,const char *);
E int ohitmon(struct monst *,struct obj *,int,BOOLEAN_P);
E void thrwmu(struct monst *);
E int spitmu(struct monst *,struct attack *);
E int breamu(struct monst *,struct attack *);
E boolean linedup(XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E boolean lined_up(struct monst *);
E struct obj *m_carrying(struct monst *,int);
E void m_useup(struct monst *,struct obj *);
E void m_throw(struct monst *,int,int,int,int,int,struct obj *);
E boolean hits_bars(struct obj **,int,int,int,int,int,int);
E void hit_bars(struct obj **,int,int,int,int,boolean,boolean);
E void dissolve_bars(int,int);

/* ### muse.c ### */

E boolean find_defensive(struct monst *);
E int use_defensive(struct monst *);
E int rnd_defensive_item(struct monst *);
E boolean find_offensive(struct monst *);
#ifdef USE_TRAMPOLI
E int mbhitm(struct monst *,struct obj *);
#endif
E int use_offensive(struct monst *);
E int rnd_offensive_item(struct monst *);
E boolean find_misc(struct monst *);
E int use_misc(struct monst *);
E int rnd_misc_item(struct monst *);
E boolean searches_for_item(struct monst *,struct obj *);
E boolean mon_reflects(struct monst *,const char *);
E boolean ureflects(const char *,const char *);
E boolean munstone(struct monst *,BOOLEAN_P);

/* ### music.c ### */

E void awaken_monsters(int);
E void do_earthquake(int);
E void awaken_soldiers(void);
E int do_play_instrument(struct obj *);

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
E void init_lan_features(void);
E char *lan_username(void);
# ifdef LAN_MAIL
E boolean lan_mail_check(void);
E void lan_mail_read(struct obj *);
E void lan_mail_init(void);
E void lan_mail_finish(void);
E void lan_mail_terminate(void);
# endif
#endif

/* ### nttty.c ### */

#ifdef WIN32CON
E void get_scr_size(void);
E int nttty_kbhit(void);
E void nttty_open(void);
E void nttty_rubout(void);
E int tgetch(void);
E int ntposkey(int *, int *, int *));
E void set_output_mode(int);
E void synch_cursor(void);
#endif

/* ### o_init.c ### */

E void init_objects(void);
E int find_skates(void);
E void oinit(void);
E void savenames(int,int);
E void restnames(int);
E void discover_object(int,BOOLEAN_P,BOOLEAN_P);
E void undiscover_object(int);
E int dodiscovered(void);
E void dragons_init(void);
E void makeknown_msg(int);

/* ### objects.c ### */

E void objects_init(void);

/* ### objnam.c ### */

E char *obj_typename(int);
E char *simple_typename(int);
E char *dump_typename(int);
E boolean obj_is_pname(struct obj *);
E char *distant_name(struct obj *,char *(*)(OBJ_P));
E char *fruitname(BOOLEAN_P);
E char *xname(struct obj *);
E char *mshot_xname(struct obj *);
E boolean the_unique_obj(struct obj *obj);
E char *doname(struct obj *);
E char *doname_with_price(struct obj *);
E boolean not_fully_identified(struct obj *);
E char *corpse_xname(struct obj *,BOOLEAN_P);
E char *cxname(struct obj *);
#ifdef SORTLOOT
E char *cxname2(struct obj *);
#endif
E char *killer_xname(struct obj *);
E const char *singular(struct obj *,char *(*)(OBJ_P));
E char *an(const char *);
E char *An(const char *);
E char *The(const char *);
E char *the(const char *);
E char *aobjnam(struct obj *,const char *);
E char *Tobjnam(struct obj *,const char *);
E char *otense(struct obj *,const char *);
E char *vtense(const char *,const char *);
E char *Doname2(struct obj *);
E char *yname(struct obj *);
E char *Yname2(struct obj *);
E char *ysimple_name(struct obj *);
E char *Ysimple_name2(struct obj *);
E char *makeplural(const char *);
E char *makesingular(const char *);
E struct obj *readobjnam(char *,struct obj *,BOOLEAN_P);
E int rnd_class(int,int);
E const char *cloak_simple_name(struct obj *);
E const char *mimic_obj_name(struct monst *);

/* ### options.c ### */

E boolean match_optname(const char *,const char *,int,BOOLEAN_P);
E void initoptions(void);
E void parseoptions(char *,BOOLEAN_P,BOOLEAN_P);
E boolean parse_monster_color(char *);
E boolean parse_symbol(const char *);
E boolean parse_monster_symbol(const char *);
E boolean parse_object_symbol(const char *);
E int doset(void);
E int dotogglepickup(void);
E void option_help(void);
E void next_opt(winid,const char *);
E int fruitadd(char *);
E int choose_classes_menu(const char *,int,BOOLEAN_P,char *,char *);
E void add_menu_cmd_alias(CHAR_P, CHAR_P);
E char map_menu_cmd(CHAR_P);
E void assign_warnings(uchar *);
E char *nh_getenv(const char *);
E void set_duplicate_opt_detection(int);
E void set_wc_option_mod_status(unsigned long, int);
E void set_wc2_option_mod_status(unsigned long, int);
E void set_option_mod_status(const char *,int);
#ifdef AUTOPICKUP_EXCEPTIONS
E int add_autopickup_exception(const char *);
E void free_autopickup_exceptions(void);
#endif /* AUTOPICKUP_EXCEPTIONS */
#ifdef MENU_COLOR
E boolean add_menu_coloring(char *);
#endif /* MENU_COLOR */

/* ### pager.c ### */

E int dowhatis(void);
E int doquickwhatis(void);
E int doidtrap(void);
E int dowhatdoes(void);
E char *dowhatdoes_core(CHAR_P, char *);
E int dohelp(void);
E int dohistory(void);
E void checkfile(char *,struct permonst *,BOOLEAN_P,BOOLEAN_P);

/* ### pcmain.c ### */

#if defined(MICRO) || defined(WIN32)
# ifdef CHDIR
E void chdirx(char *,BOOLEAN_P);
# endif /* CHDIR */
#endif /* MICRO || WIN32 */

/* ### pcsys.c ### */

#if defined(MICRO) || defined(WIN32)
E void flushout(void);
E int dosh(void);
# ifdef MFLOPPY
E void eraseall(const char *,const char *);
E void copybones(int);
E void playwoRAMdisk(void);
E int saveDiskPrompt(int);
E void gameDiskPrompt(void);
# endif
E void append_slash(char *);
E void getreturn(const char *);
# ifndef AMIGA
E void msmsg(const char *,...);
# endif
E FILE *fopenp(const char *,const char *);
#endif /* MICRO || WIN32 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
E void gettty(void);
E void settty(const char *);
E void setftty(void);
E void error(const char *,...);
#if defined(TIMED_DELAY) && defined(_MSC_VER)
E void msleep(unsigned);
#endif
#endif /* MICRO || WIN32 */

/* ### pcunix.c ### */

#if defined(MICRO)
E void regularize(char *);
#endif /* MICRO */
#if defined(PC_LOCKING)
E void getlock(void);
#endif

/* ### pickup.c ### */

E void observe_quantum_cat(struct obj *, boolean);
E void open_coffin(struct obj *, boolean);
#ifdef GOLDOBJ
E int collect_obj_classes(char *,struct obj *,BOOLEAN_P,boolean (*)(OBJ_P), int *);
#else
E int collect_obj_classes(char *,struct obj *,BOOLEAN_P,BOOLEAN_P,boolean (*)(OBJ_P), int *);
#endif
E void add_valid_menu_class(int);
E boolean allow_all(struct obj *);
E boolean allow_category(struct obj *);
E boolean is_worn_by_type(struct obj *);
#ifdef USE_TRAMPOLI
E int ck_bag(struct obj *);
E int in_container(struct obj *);
E int out_container(struct obj *);
#endif
E int pickup(int);
E int pickup_object(struct obj *, long, BOOLEAN_P);
E int query_category(const char *, struct obj *, int, menu_item **, int);
E int query_objlist(const char *, struct obj *, int,
				menu_item **, int, boolean (*)(OBJ_P));
E struct obj *pick_obj(struct obj *);
E int encumber_msg(void);
E int doloot(void);
E int dotip(void);
E int use_container(struct obj *,int);
E int loot_mon(struct monst *,int *,boolean *);
E const char *safe_qbuf(const char *,unsigned,
				const char *,const char *,const char *);
E boolean is_autopickup_exception(struct obj *, BOOLEAN_P);

/* ### pline.c ### */

E void msgpline_add(int, char *);
E void msgpline_free(void);
E void pline(const char *,...) PRINTF_F(1,2);
E void Norep(const char *,...) PRINTF_F(1,2);
E void free_youbuf(void);
E void You(const char *,...) PRINTF_F(1,2);
E void Your(const char *,...) PRINTF_F(1,2);
E void You_feel(const char *,...) PRINTF_F(1,2);
E void You_cant(const char *,...) PRINTF_F(1,2);
E void You_hear(const char *,...) PRINTF_F(1,2);
E void pline_The(const char *,...) PRINTF_F(1,2);
E void There(const char *,...) PRINTF_F(1,2);
E void verbalize(const char *,...) PRINTF_F(1,2);
E void raw_printf(const char *,...) PRINTF_F(1,2);
E void impossible(const char *,...) PRINTF_F(1,2);
E void warning(const char *,...) PRINTF_F(1,2);
E const char *align_str(ALIGNTYP_P);
E void mstatusline(struct monst *);
E void ustatusline(void);
E void self_invis_message(void);

/* ### polyself.c ### */

E void init_uasmon(void);
E void set_uasmon(void);
E void change_sex(void);
E void polyself(BOOLEAN_P);
E int polymon(int);
E void rehumanize(void);
E int dobreathe(void);
E int dospit(void);
E int doremove(void);
E int dospinweb(void);
E int dosummon(void);
E int dogaze(void);
E int dohide(void);
E int domindblast(void);
E void skinback(BOOLEAN_P);
E const char *mbodypart(struct monst *,int);
E const char *body_part(int);
E int poly_gender(void);
E void ugolemeffects(int,int);
E boolean is_playermon_genocided(void);

/* ### potion.c ### */

E void set_itimeout(long *,long);
E void incr_itimeout(long *,int);
E void make_confused(long,BOOLEAN_P);
E void make_stunned(long,BOOLEAN_P);
E void make_blinded(long,BOOLEAN_P);
E void make_sick(long, const char *, BOOLEAN_P,int);
E void make_vomiting(long,BOOLEAN_P);
E boolean make_hallucinated(long,BOOLEAN_P,long);
E int dodrink(void);
E int dopotion(struct obj *);
E int peffects(struct obj *);
E void healup(int,int,BOOLEAN_P,BOOLEAN_P);
E void strange_feeling(struct obj *,const char *);
E void potionhit(struct monst *,struct obj *,BOOLEAN_P);
E void potionbreathe(struct obj *);
E boolean get_wet(struct obj *);
E int dodip(void);
E void djinni_from_bottle(struct obj *);
E struct monst *split_mon(struct monst *,struct monst *);
E const char *bottlename(void);

/* ### pray.c ### */

#ifdef USE_TRAMPOLI
E int prayer_done(void);
#endif
E int dosacrifice(void);
E boolean can_pray(BOOLEAN_P);
E int dopray(void);
E const char *u_gname(void);
E int doturn(void);
E const char *a_gname(void);
E const char *a_gname_at(XCHAR_P x,XCHAR_P y);
E const char *align_gname(ALIGNTYP_P);
E const char *halu_gname(ALIGNTYP_P);
E const char *rnd_gname(int);
E const char *align_gtitle(ALIGNTYP_P);
E void altar_wrath(int,int);
#ifdef ASTRAL_ESCAPE
E int invoke_amulet(struct obj *);
#endif
E int in_trouble(void);


/* ### priest.c ### */

E int move_special(struct monst *,BOOLEAN_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			   XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E char temple_occupied(char *);
E int pri_move(struct monst *);
E void priestini(d_level *,struct mkroom *,int,int,BOOLEAN_P);
E char *priestname(struct monst *,char *);
E boolean p_coaligned(struct monst *);
E struct monst *findpriest(CHAR_P);
E void intemple(int);
E void priest_talk(struct monst *);
E struct monst *mk_roamer(struct permonst *,ALIGNTYP_P, XCHAR_P,XCHAR_P,BOOLEAN_P);
E void reset_hostility(struct monst *);
E boolean in_your_sanctuary(struct monst *,XCHAR_P,XCHAR_P);
E void ghod_hitsu(struct monst *);
E void angry_priest(void);
E void clearpriests(void);
E void restpriest(struct monst *,BOOLEAN_P);

/* ### quest.c ### */

E void onquest(void);
E void nemdead(void);
E void artitouch(void);
E boolean ok_to_quest(void);
E void leader_speaks(struct monst *);
E void nemesis_speaks(void);
E void quest_chat(struct monst *);
E void quest_talk(struct monst *);
E void quest_stat_check(struct monst *);
E void finish_quest(struct obj *);

/* ### questpgr.c ### */

E void load_qtlist(void);
E void unload_qtlist(void);
E short quest_info(int);
E const char *ldrname(void);
E boolean is_quest_artifact(struct obj*);
E void com_pager(int);
E void qt_pager(int);
E char *string_subst(char *);
E void qt_com_firstline(int, char*);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
E void srandom(unsigned);
E char *initstate(unsigned,char *,int);
E char *setstate(char *);
E long random(void);
#endif /* RANDOM */

/* ### read.c ### */

E int doread(void);
E boolean is_chargeable(struct obj *);
E void recharge(struct obj *,int);
E void forget_objects(int);
E void forget_levels(int);
E void forget_traps(void);
E void forget_map(int);
E int seffects(struct obj *);
#ifdef USE_TRAMPOLI
E void set_lit(int,int,genericptr_t);
#endif
E void litroom(BOOLEAN_P,struct obj *);
E void do_genocide(int,BOOLEAN_P);
E void punish(struct obj *);
E void unpunish(void);
E boolean cant_create(int *, BOOLEAN_P);
#ifdef WIZARD
E boolean create_particular(void);
#endif
E void drop_boulder_on_player(BOOLEAN_P, BOOLEAN_P, BOOLEAN_P, BOOLEAN_P);
E int  drop_boulder_on_monster(int, int, BOOLEAN_P, BOOLEAN_P);

/* ### rect.c ### */

E void init_rect(void);
E NhRect *get_rect(NhRect *);
E NhRect *rnd_rect(void);
E void remove_rect(NhRect *);
E void add_rect(NhRect *);
E void split_rects(NhRect *,NhRect *);

/* ## region.c ### */
E void clear_regions(void);
E void run_regions(void);
E boolean in_out_region(XCHAR_P,XCHAR_P);
E boolean m_in_out_region(struct monst *,XCHAR_P,XCHAR_P);
E void update_player_regions(void);
E void update_monster_region(struct monst *);
E NhRegion *visible_region_at(XCHAR_P,XCHAR_P);
E void show_region(NhRegion*, XCHAR_P, XCHAR_P);
E void save_regions(int,int);
E void rest_regions(int,BOOLEAN_P);
E NhRegion* create_gas_cloud(XCHAR_P, XCHAR_P, int, size_t, int);
E NhRegion* create_cthulhu_death_cloud(XCHAR_P, XCHAR_P, int, size_t, int);

/* ### restore.c ### */

E void inven_inuse(BOOLEAN_P);
E int dorecover(int);
E void trickery(char *);
E void getlev(int,int,XCHAR_P,BOOLEAN_P);
E void minit(void);
E boolean lookup_id_mapping(unsigned, unsigned *);
#ifdef ZEROCOMP
E int mread(int,genericptr_t,unsigned int);
#else
E void mread(int,genericptr_t,unsigned int);
#endif

/* ### rip.c ### */

E void genl_outrip(winid,int);

/* ### rnd.c ### */

E int rn2(int);
E int rnl(int);
E int rnd(int);
E int d(int,int);
E int rne(int);
E int rnz(int);
E int rnf(int, int);

/* ### role.c ### */

E boolean validrole(int);
E boolean validrace(int, int);
E boolean validgend(int, int, int);
E boolean validalign(int, int, int);
E int randrole(void);
E int randrace(int);
E int randgend(int, int);
E int randalign(int, int);
E int str2role(char *);
E int str2race(char *);
E int str2gend(char *);
E int str2align(char *);
E boolean ok_role(int, int, int, int);
E int pick_role(int, int, int, int);
E boolean ok_race(int, int, int, int);
E int pick_race(int, int, int, int);
E boolean ok_gend(int, int, int, int);
E int pick_gend(int, int, int, int);
E boolean ok_align(int, int, int, int);
E int pick_align(int, int, int, int);
E void role_init(void);
E void rigid_role_checks(void);
E void plnamesuffix(void);
E const char *Hello(struct monst *);
E const char *Goodbye(void);
E char *build_plselection_prompt(char *, int, int, int, int, int);
E char *root_plselection_prompt(char *, int, int, int, int, int);
E void violated(int);
E boolean successful_cdt(int);
E boolean intended_cdt(int);
E boolean superfluous_cdt(int);
E boolean failed_cdt(int);

/* ### rumors.c ### */

E char *getrumor(int,char *, BOOLEAN_P);
E void outrumor(int,int);
E void outoracle(BOOLEAN_P, BOOLEAN_P);
E void save_oracles(int,int);
E void restore_oracles(int);
E int doconsult(struct monst *);

/* ### save.c ### */

E int dosave(void);
#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
E void hangup(int);
#endif
E int dosave0(void);
#ifdef INSURANCE
E void savestateinlock(void);
#endif
#ifdef MFLOPPY
E boolean savelev(int,XCHAR_P,int);
E boolean swapin_file(int);
E void co_false(void);
#else
E void savelev(int,XCHAR_P,int);
#endif
E void bufon(int);
E void bufoff(int);
E void bflush(int);
E void bwrite(int,genericptr_t,unsigned int);
E void bclose(int);
E void savefruitchn(int,int);
E void free_dungeons(void);
E void freedynamicdata(void);

/* ### shk.c ### */

#ifdef GOLDOBJ
E long money2mon(struct monst *, long);
E void money2u(struct monst *, long);
#endif
E char *shkname(struct monst *);
E void shkgone(struct monst *);
E void set_residency(struct monst *,BOOLEAN_P);
E void replshk(struct monst *,struct monst *);
E void restshk(struct monst *,BOOLEAN_P);
E char inside_shop(XCHAR_P,XCHAR_P);
E void u_left_shop(char *,BOOLEAN_P);
E void remote_burglary(XCHAR_P,XCHAR_P);
E void u_entered_shop(char *);
E boolean same_price(struct obj *,struct obj *);
E void shopper_financial_report(void);
E int inhishop(struct monst *);
E struct monst *shop_keeper(CHAR_P);
E boolean tended_shop(struct mkroom *);
E void delete_contents(struct obj *);
E void obfree(struct obj *,struct obj *);
E void home_shk(struct monst *,BOOLEAN_P);
E void make_happy_shk(struct monst *,BOOLEAN_P);
E void hot_pursuit(struct monst *);
E void make_angry_shk(struct monst *,XCHAR_P,XCHAR_P);
E int dopay(void);
E boolean paybill(int);
E void finish_paybill(void);
E struct obj *find_oid(unsigned);
E long contained_cost(struct obj *,struct monst *,long,BOOLEAN_P, BOOLEAN_P);
E long contained_gold(struct obj *);
E void picked_container(struct obj *);
E long unpaid_cost(struct obj *);
E void bill_box_content(struct obj *,BOOLEAN_P);
E void addtobill(struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
E void append_honorific(char *);
E void splitbill(struct obj *,struct obj *);
E void subfrombill(struct obj *,struct monst *);
E long stolen_value(struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P);
E void sellobj_state(int);
E void sellobj(struct obj *,XCHAR_P,XCHAR_P);
E int doinvbill(int);
E struct monst *shkcatch(struct obj *,XCHAR_P,XCHAR_P);
E void add_damage(XCHAR_P,XCHAR_P,long);
E int repair_damage(struct monst *,struct damage *,BOOLEAN_P);
E int shk_move(struct monst *);
E void after_shk_move(struct monst *);
E boolean is_fshk(struct monst *);
E void shopdig(int);
E void pay_for_damage(const char *,BOOLEAN_P);
E boolean costly_spot(XCHAR_P,XCHAR_P);
E struct obj *shop_object(XCHAR_P,XCHAR_P);
E void price_quote(struct obj *);
E void shk_chat(struct monst *);
E void check_unpaid_usage(struct obj *,BOOLEAN_P);
E void check_unpaid(struct obj *);
E void costly_gold(XCHAR_P,XCHAR_P,long);
E long get_cost_of_shop_item(struct obj *);
E boolean block_door(XCHAR_P,XCHAR_P);
E boolean block_entry(XCHAR_P,XCHAR_P);
#ifdef BLACKMARKET
E void blkmar_guards(struct monst *);
E void set_black_marketeer_angry(void);
E void bars_around_portal(BOOLEAN_P);
#endif /* BLACKMARKET */
E char *shk_your(char *,struct obj *);
E char *Shk_Your(char *,struct obj *);

/* ### shknam.c ### */

E void stock_room(int,struct mkroom *);
E boolean saleable(struct monst *,struct obj *);
E int get_shop_item(int);
E void shop_selection_init(void);

/* ### sit.c ### */

E void take_gold(void);
E int dosit(void);
E void rndcurse(void);
E void attrcurse(void);

/* ### sounds.c ### */

E void dosounds(void);
E const char *growl_sound(struct monst *);
E void growl(struct monst *);
E void yelp(struct monst *);
E void whimper(struct monst *);
E void beg(struct monst *);
E int dotalk(void);
#ifdef USER_SOUNDS
E int add_sound_mapping(const char *);
E void play_sound_for_message(const char *);
#endif

/* ### sys/msdos/sound.c ### */

#ifdef MSDOS
E int assign_soundcard(char *);
#endif

/* ### sp_lev.c ### */

E boolean check_room(xchar *,xchar *,xchar *,xchar *,BOOLEAN_P);
E boolean create_room(XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			      XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P);
E void create_secret_door(struct mkroom *,XCHAR_P);
E boolean dig_corridor(coord *,coord *,BOOLEAN_P,SCHAR_P,SCHAR_P);
E void fill_room(struct mkroom *,BOOLEAN_P);
E boolean load_special(const char *);

/* ### spell.c ### */

#ifdef USE_TRAMPOLI
E int learn(void);
#endif
E int study_book(struct obj *);
E void book_disappears(struct obj *);
E void book_substitution(struct obj *,struct obj *);
E void age_spells(void);
E int docast(void);
E int spell_skilltype(int);
E int spelleffects(int,BOOLEAN_P);
E void losespells(void);
E int dovspell(void);
E void initialspell(struct obj *);
E void dump_spells(void);
E boolean parse_spellorder(char *);

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
E int stealarm(void);
#endif
#ifdef GOLDOBJ
E long somegold(long);
#else
E long somegold(void);
#endif
E void stealgold(struct monst *);
E void remove_worn_item(struct obj *,BOOLEAN_P);
E int steal(struct monst *, char *);
E int mpickobj(struct monst *,struct obj *);
E void stealamulet(struct monst *);
E void mdrop_special_objs(struct monst *);
E void relobj(struct monst *,int,BOOLEAN_P);
#ifdef GOLDOBJ
E struct obj *findgold(struct obj *);
#endif

/* ### steed.c ### */

#ifdef STEED
E void rider_cant_reach(void);
E boolean can_saddle(struct monst *);
E int use_saddle(struct obj *);
E boolean can_ride(struct monst *);
E int doride(void);
E boolean mount_steed(struct monst *, BOOLEAN_P);
E void exercise_steed(void);
E void kick_steed(void);
E void dismount_steed(int);
E void place_monster(struct monst *,int,int);
#endif

/* ### teleport.c ### */

E boolean goodpos(int,int,struct monst *,unsigned);
E boolean enexto(coord *,XCHAR_P,XCHAR_P,struct permonst *);
E boolean enexto_core(coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned);
E boolean enexto_core_range(coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned,int);
E int epathto(coord *,int,XCHAR_P,XCHAR_P,struct permonst *);
E void teleds(int,int,BOOLEAN_P);
E boolean safe_teleds(BOOLEAN_P);
E boolean teleport_pet(struct monst *,BOOLEAN_P);
E void tele(void);
E int dotele(void);
E void level_tele(void);
E void domagicportal(struct trap *);
E void tele_trap(struct trap *);
E void level_tele_trap(struct trap *);
E void rloc_to(struct monst *,int,int);
E boolean rloc(struct monst *, BOOLEAN_P);
E boolean tele_restrict(struct monst *);
E void mtele_trap(struct monst *, struct trap *,int);
E int mlevel_tele_trap(struct monst *, struct trap *,BOOLEAN_P,int);
E void rloco(struct obj *);
E int random_teleport_level(void);
E boolean u_teleport_mon(struct monst *,BOOLEAN_P);

/* ### termcap.c ### */
E void term_start_color(int);
E void term_end_color(void);
E void term_start_attr(int);
E void term_end_attr(int);

/* ### tile.c ### */
#ifdef USE_TILES
E void substitute_tiles(d_level *);
#endif

/* ### timeout.c ### */

E void burn_away_slime(void);
E void nh_timeout(void);
E void fall_asleep(int, BOOLEAN_P);
E void attach_egg_hatch_timeout(struct obj *);
E void attach_fig_transform_timeout(struct obj *);
E void kill_egg(struct obj *);
E void hatch_egg(genericptr_t, long);
E void learn_egg_type(int);
E void burn_object(genericptr_t, long);
E void begin_burn(struct obj *, BOOLEAN_P);
E void end_burn(struct obj *, BOOLEAN_P);
E void do_storms(void);
E boolean start_timer(long, SHORT_P, SHORT_P, genericptr_t);
E long stop_timer(SHORT_P, genericptr_t);
E void run_timers(void);
E void obj_move_timers(struct obj *, struct obj *);
E void obj_split_timers(struct obj *, struct obj *);
E void obj_stop_timers(struct obj *);
E boolean obj_is_local(struct obj *);
E void save_timers(int,int,int);
E void restore_timers(int,int,BOOLEAN_P,long);
E void relink_timers(BOOLEAN_P);
#ifdef WIZARD
E int wiz_timeout_queue(void);
E void timer_sanity_check(void);
#endif

/* ### topten.c ### */

#ifdef RECORD_CONDUCT
E long encodeconduct(void);
#endif
E void topten(int);
E void prscore(int,char **);
E struct obj *tt_oname(struct obj *);

/* ### track.c ### */

E void initrack(void);
E void settrack(void);
E coord *gettrack(int,int);

/* ### trap.c ### */

E boolean burnarmor(struct monst *);
E boolean rust_dmg(struct obj *,const char *,int,BOOLEAN_P,struct monst *);
E void grease_protect(struct obj *,const char *,struct monst *);
E struct trap *maketrap(int,int,int);
E void fall_through(BOOLEAN_P);
E struct monst *animate_statue(struct obj *,XCHAR_P,XCHAR_P,int,int *);
E struct monst *activate_statue_trap(struct trap *,XCHAR_P,XCHAR_P,BOOLEAN_P);
E void dotrap(struct trap *, unsigned);
E void seetrap(struct trap *);
E int mintrap(struct monst *);
E void instapetrify(const char *);
E void minstapetrify(struct monst *,BOOLEAN_P);
#ifdef WEBB_DISINT
E int instadisintegrate(const char *);
E int minstadisintegrate(struct monst *);
#endif
E void selftouch(const char *);
E void mselftouch(struct monst *,const char *,BOOLEAN_P);
E void float_up(void);
E void fill_pit(int,int);
E int float_down(long, long);
E int fire_damage(struct obj *,BOOLEAN_P,BOOLEAN_P,XCHAR_P,XCHAR_P);
E boolean water_damage(struct obj *,BOOLEAN_P,BOOLEAN_P);
E boolean drown(void);
E void drain_en(int);
E int dountrap(void);
E int untrap(BOOLEAN_P);
E boolean chest_trap(struct obj *,int,BOOLEAN_P);
E void deltrap(struct trap *);
E boolean delfloortrap(struct trap *);
E struct trap *t_at(int,int);
E void b_trapped(const char *,int);
E boolean unconscious(void);
E boolean lava_effects(void);
E boolean swamp_effects(void);
E void blow_up_landmine(struct trap *);
E int launch_obj(SHORT_P,int,int,int,int,int);

/* ### u_init.c ### */

E void u_init(void);

/* ### uhitm.c ### */

E void hurtmarmor(struct monst *,int);
E boolean attack_checks(struct monst *,struct obj *);
E void check_caitiff(struct monst *);
E schar find_roll_to_hit(struct monst *);
E boolean attack(struct monst *);
E boolean hmon(struct monst *,struct obj *,int);
E int damageum(struct monst *,struct attack *);
E void missum(struct monst *,struct attack *);
E int passive(struct monst *,BOOLEAN_P,int,UCHAR_P);
E void passive_obj(struct monst *,struct obj *,struct attack *);
E void stumble_onto_mimic(struct monst *);
E int flash_hits_mon(struct monst *,struct obj *);

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
E void port_help(void);
# endif
#endif /* UNIX */


/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
E void gettty(void);
E void settty(const char *);
E void setftty(void);
E void intron(void);
E void introff(void);
E void error(const char *,...) PRINTF_F(1,2);
#endif /* UNIX || __BEOS__ */

/* ### unixunix.c ### */

#ifdef UNIX
E void getlock(void);
E void regularize(char *);
# if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
E void msleep(unsigned);
# endif
# ifdef SHELL
E int dosh(void);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
E int child(int);
# endif
#ifdef FILE_AREAS
E char *make_file_name(const char *, const char *);
E FILE *fopen_datafile_area(const char *,const char *,const char *, BOOLEAN_P);
E FILE *freopen_area(const char *,const char *,const char *, FILE *);
E int chmod_area(const char *, const char *, int);
E int open_area(const char *, const char *, int, int);
E int creat_area(const char *, const char *, int);
E boolean lock_file_area(const char *, const char *,int);
E void unlock_file_area(const char *, const char *);
E int rename_area(const char *, const char *, const char *);
E int remove_area(const char *, const char *);
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS 
E int hide_privileges(BOOLEAN_P);
# endif
#endif /* UNIX */

/* ### vault.c ### */

E boolean grddead(struct monst *);
E char vault_occupied(char *);
E void invault(void);
E int gd_move(struct monst *);
E void paygd(void);
E long hidden_gold(void);
E boolean gd_sound(void);

/* ### version.c ### */

E char *version_string(char *);
E char *getversionstring(char *);
E int doversion(void);
E int doextversion(void);
#ifdef MICRO
E boolean comp_times(long);
#endif
E boolean check_version(struct version_info *,
				const char *,BOOLEAN_P);
E unsigned long get_feature_notice_ver(char *);
E unsigned long get_current_feature_ver(void);
#ifdef RUNTIME_PORT_ID
E void append_port_id(char *);
#endif

/* ### video.c ### */

#ifdef MSDOS
E int assign_video(char *);
# ifdef NO_TERMS
E void gr_init(void);
E void gr_finish(void);
# endif
E void tileview(BOOLEAN_P);
#endif
#ifdef VIDEOSHADES
E int assign_videoshades(char *);
E int assign_videocolors(char *);
#endif

/* ### vis_tab.c ### */

#ifdef VISION_TABLES
E void vis_tab_init(void);
#endif

/* ### vision.c ### */

E void vision_init(void);
E int does_block(int,int,struct rm*);
E void vision_reset(void);
E void vision_recalc(int);
E void block_point(int,int);
E void unblock_point(int,int);
E boolean clear_path(int,int,int,int);
E void do_clear_area(int,int,int, void (*)(int,int,genericptr_t),genericptr_t);

#ifdef VMS

/* ### vmsfiles.c ### */

E int vms_link(const char *,const char *);
E int vms_unlink(const char *);
E int vms_creat(const char *,unsigned int);
E int vms_open(const char *,int,unsigned int);
E boolean same_dir(const char *,const char *);
E int c__translate(int);
E char *vms_basename(const char *);

/* ### vmsmail.c ### */

E unsigned long init_broadcast_trapping(void);
E unsigned long enable_broadcast_trapping(void);
E unsigned long disable_broadcast_trapping(void);
# if 0
E struct mail_info *parse_next_broadcast(void);
# endif /*0*/

/* ### vmsmain.c ### */

E int main(int, char **);
# ifdef CHDIR
E void chdirx(const char *,BOOLEAN_P);
# endif /* CHDIR */

/* ### vmsmisc.c ### */

E void vms_abort(void);
E void vms_exit(int);

/* ### vmstty.c ### */

E int vms_getchar(void);
E void gettty(void);
E void settty(const char *);
E void shuttty(const char *);
E void setftty(void);
E void intron(void);
E void introff(void);
E void error(const char *,...) PRINTF_F(1,2);
#ifdef TIMED_DELAY
E void msleep(unsigned);
#endif

/* ### vmsunix.c ### */

E void getlock(void);
E void regularize(char *);
E int vms_getuid(void);
E boolean file_is_stmlf(int);
E int vms_define(const char *,const char *,int);
E int vms_putenv(const char *);
E char *verify_termcap(void);
# if defined(CHDIR) || defined(SHELL) || defined(SECURE)
E void privoff(void);
E void privon(void);
# endif
# ifdef SHELL
E int dosh(void);
# endif
# if defined(SHELL) || defined(MAIL)
E int vms_doshell(const char *,BOOLEAN_P);
# endif
# ifdef SUSPEND
E int dosuspend(void);
# endif

#endif /* VMS */

/* ### weapon.c ### */

E int hitval(struct obj *,struct monst *);
E int dmgval(struct obj *,struct monst *);
E struct obj *select_rwep(struct monst *);
E struct obj *select_hwep(struct monst *);
E void possibly_unwield(struct monst *,BOOLEAN_P);
E int mon_wield_item(struct monst *);
E int abon(void);
E int dbon(void);
E int enhance_weapon_skill(void);
#ifdef DUMP_LOG
E void dump_weapon_skill(void);
#endif
E void unrestrict_weapon_skill(int);
E void use_skill(int,int);
E void add_weapon_skill(int);
E void lose_weapon_skill(int);
E int weapon_type(struct obj *);
E int uwep_skill_type(void);
E int weapon_hit_bonus(struct obj *);
E int weapon_dam_bonus(struct obj *);
E void skill_init(const struct def_skill *);
E boolean can_advance_something(void);

/* ### were.c ### */

E void were_change(struct monst *);
E void new_were(struct monst *);
E int were_summon(struct permonst *,BOOLEAN_P,int *,char *);
E void you_were(void);
E void you_unwere(BOOLEAN_P);

/* ### wield.c ### */

E void setuwep(struct obj *);
E void setuqwep(struct obj *);
E void setuswapwep(struct obj *);
E int dowield(void);
E int doswapweapon(void);
E int dowieldquiver(void);
E int dounwield(void);
E boolean wield_tool(struct obj *,const char *);
E int can_twoweapon(void);
E void drop_uswapwep(void);
E int dotwoweapon(void);
E void uwepgone(void);
E void uswapwepgone(void);
E void uqwepgone(void);
E void untwoweapon(void);
E void erode_obj(struct obj *,BOOLEAN_P,BOOLEAN_P);
E int chwepon(struct obj *,int);
E int welded(struct obj *);
E void weldmsg(struct obj *);
E void setmnotwielded(struct monst *,struct obj *);
E void unwield_weapons_silently(void);

/* ### windows.c ### */

E void choose_windows(const char *);
E char genl_message_menu(CHAR_P,int,const char *);
E void genl_preference_update(const char *);

/* ### wizard.c ### */

E void amulet(void);
E int mon_has_amulet(struct monst *);
E int mon_has_special(struct monst *);
E int tactics(struct monst *);
E void aggravate(void);
E void clonewiz(void);
E int pick_nasty(void);
E int nasty(struct monst*);
E void resurrect(void);
E void intervene(void);
E void wizdead(void);
E void cuss(struct monst *);

/* ### worm.c ### */

E int get_wormno(void);
E void initworm(struct monst *,int);
E void worm_move(struct monst *);
E void worm_nomove(struct monst *);
E void wormgone(struct monst *);
E void wormhitu(struct monst *);
E void cutworm(struct monst *,XCHAR_P,XCHAR_P,struct obj *);
E void see_wsegs(struct monst *);
E void detect_wsegs(struct monst *,BOOLEAN_P);
E void save_worm(int,int);
E void rest_worm(int);
E void place_wsegs(struct monst *);
E void remove_worm(struct monst *);
E void place_worm_tail_randomly(struct monst *,XCHAR_P,XCHAR_P);
E int count_wsegs(struct monst *);
E boolean worm_known(struct monst *);
E void flip_worm_segs_vertical(struct monst *, int);
E void flip_worm_segs_horizontal(struct monst *, int);

/* ### worn.c ### */

E void setworn(struct obj *,long);
E void setnotworn(struct obj *);
E void mon_set_minvis(struct monst *);
E void mon_adjust_speed(struct monst *,int,struct obj *);
E void update_mon_intrinsics(struct monst *,struct obj *,BOOLEAN_P,BOOLEAN_P);
E int find_mac(struct monst *);
E void m_dowear(struct monst *,BOOLEAN_P);
E struct obj *which_armor(struct monst *,long);
E void mon_break_armor(struct monst *,BOOLEAN_P);
E void bypass_obj(struct obj *);
E void clear_bypasses(void);
E int racial_exception(struct monst *, struct obj *);

/* ### write.c ### */

E int dowrite(struct obj *);

/* ### zap.c ### */

E int bhitm(struct monst *,struct obj *);
E void probe_monster(struct monst *);
E boolean get_obj_location(struct obj *,xchar *,xchar *,int);
E boolean get_mon_location(struct monst *,xchar *,xchar *,int);
E struct monst *get_container_location(struct obj *obj, int *, int *);
E struct monst *montraits(struct obj *,coord *);
E struct monst *revive(struct obj *);
E int unturn_dead(struct monst *);
E void cancel_item(struct obj *);
E boolean drain_item(struct obj *);
E struct obj *poly_obj(struct obj *, int);
E boolean obj_resists(struct obj *,int,int);
E boolean obj_shudders(struct obj *);
E void do_osshock(struct obj *);
E int bhito(struct obj *,struct obj *);
E int bhitpile(struct obj *,int (*)(OBJ_P,OBJ_P),int,int);
E int zappable(struct obj *);
E void zapnodir(struct obj *);
E int dozap(void);
E int zapyourself(struct obj *,BOOLEAN_P);
E boolean cancel_monst(struct monst *,struct obj *, BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
E void weffects(struct obj *);
E int spell_damage_bonus(void);
E const char *exclam(int force);
E void hit(const char *,struct monst *,const char *);
E void miss(const char *,struct monst *);
E struct monst *bhit(int,int,int,int,int (*)(MONST_P,OBJ_P),
			     int (*)(OBJ_P,OBJ_P),struct obj *, boolean *);
E struct monst *boomhit(int,int);
E int burn_floor_paper(int,int,BOOLEAN_P,BOOLEAN_P);
E void buzz(int,int,XCHAR_P,XCHAR_P,int,int);
E void melt_ice(XCHAR_P,XCHAR_P);
E int zap_over_floor(XCHAR_P,XCHAR_P,int,boolean *);
E void fracture_rock(struct obj *);
E boolean break_statue(struct obj *);
E void destroy_item(int,int);
E int destroy_mitem(struct monst *,int,int);
E int resist(struct monst *,CHAR_P,int,int);
E void makewish(BOOLEAN_P);
E void remove_corpse(struct obj *);

/* ### livelog.c ### */
#ifdef LIVELOGFILE
E boolean livelog_start(void);
E void livelog_achieve_update(void);
E void livelog_wish(char*);
#ifdef LIVELOG_SHOUT
E int doshout(void);
#endif
E void livelog_shoplifting(const char*, const char*, long);
#endif
E void livelog_game_started(const char*, const char*, const char*, const char*);
E void livelog_game_action(const char*);
E void livelog_generic(const char*, const char*);
E void livelog_genocide(const char*, int);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

/* ### tutorial.c ### */
E boolean check_tutorial_message(int);
E void maybe_tutorial(void);
E int tutorial_redisplay(void);
E void tutorial_redisplay_message(void);
E void check_tutorial_farlook(int, int);
E void check_tutorial_command(char);
E int check_tutorial_location(int, int, BOOLEAN_P);
E int check_tutorial_oclass(int);

/* ### unicode.c ### */
E glyph_t get_unicode_codepoint(int);
E int pututf8char(glyph_t);

#undef E

#endif /* EXTERN_H */
