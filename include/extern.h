/* Copyright (c) Steve Creps, 1988.               */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#define E extern

/* ### alloc.c ### */

#if 0
E long *alloc(unsigned int);
#endif
E char *fmt_ptr(const genericptr);

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

extern void early_init(void);
E void moveloop(boolean);
E void stop_occupation(void);
E void display_gamewindows(void);
E void newgame(void);
E void welcome(boolean);
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
E void m_unleash(struct monst *, boolean);
E void unleash_all(void);
E boolean leashable(struct monst *);
E boolean next_to_u(void);
E struct obj *get_mleash(struct monst *);
E void check_leash(xchar, xchar);
E boolean um_dist(xchar, xchar, xchar);
E boolean snuff_candle(struct obj *);
E boolean snuff_lit(struct obj *);
E boolean catch_lit(struct obj *);
E void fix_attributes_and_properties(struct obj *, int);
E boolean tinnable(struct obj *);
E void reset_trapset(void);
E void fig_transform(ANY_P *, long);
E int unfixable_trouble_count(boolean);

/* ### artifact.c ### */

E void init_artifacts(void);
E void save_artifacts(int);
E void restore_artifacts(int);
E const char *artiname(int);
E struct obj *mk_artifact(struct obj *, aligntyp);
E const char *artifact_name(const char *, short *);
E boolean exist_artifact(int, const char *);
E void artifact_exists(struct obj *, const char *, boolean);
E int nartifact_exist(void);
E boolean spec_ability(struct obj *, unsigned long);
E boolean confers_luck(struct obj *);
E boolean arti_reflects(struct obj *);
E boolean shade_glare(struct obj *);
E boolean restrict_name(struct obj *, const char *, boolean);
E boolean defends(int, struct obj *);
E boolean defends_when_carried(int, struct obj *);
E boolean protects(struct obj *, boolean);
E void set_artifact_intrinsic(struct obj *, boolean, long);
E int touch_artifact(struct obj *, struct monst *);
E int spec_abon(struct obj *, struct monst *);
E int spec_dbon(struct obj *, struct monst *, int);
E void discover_artifact(xchar);
E boolean undiscovered_artifact(xchar);
E int disp_artifact_discoveries(winid);
E boolean artifact_hit(struct monst *, struct monst *, struct obj *, int *, int);
E int doinvoke(void);
E void arti_speak(struct obj *);
E boolean artifact_light(struct obj *);
E long spec_m2(struct obj *);
E boolean artifact_has_invprop(struct obj *, uchar);
E long arti_cost(struct obj *);
E boolean MATCH_WARN_OF_MON(struct monst *);
E struct obj *what_gives(long *);
E const char *get_warned_of_monster(struct obj *);
E const char *glow_color(int);
E const char *glow_verb(int, boolean);
E void Sting_effects(int);
E int retouch_object(struct obj **, boolean);
E void retouch_equipment(int);
E void mkot_trap_warn(void);
E boolean is_magic_key(struct monst *, struct obj *);
E struct obj *has_magic_key(struct monst *);

/* ### attrib.c ### */

E boolean adjattrib(int, int, int);
E void change_luck(schar);
E int stone_luck(boolean);
E boolean has_luckitem(void);
E void set_moreluck(void);
E void gainstr(struct obj *, int);
E void losestr(int);
E void restore_attrib(void);
E void exercise(int, boolean);
E void exerchk(void);
E void reset_attribute_clock(void);
E void init_attr(int);
E void redist_attr(void);
E void adjabil(int, int);
E int newhp(void);
E schar acurr(int);
E schar acurrstr(void);
E void adjalign(int);
E const char * beautiful(void);
E int uhp(void);
E int uhpmax(void);
E boolean extremeattr(int);
E void uchangealign(int, int);

/* ### ball.c ### */

E void ballrelease(boolean);
E void ballfall(void);
E void placebc(void);
E void unplacebc(void);
E void set_bc(int);
E void move_bc(int, int, xchar, xchar, xchar, xchar);
E boolean drag_ball(xchar, xchar, int *, xchar *, xchar *, xchar *, xchar *, boolean *, boolean);
E void drop_ball(xchar, xchar);
E void drag_down(void);
E void bc_sanity_check(void);

/* ### bones.c ### */

E void sanitize_name(char *);
E void drop_upon_death(struct monst *, struct obj *, int, int);
E boolean can_make_bones(void);
E void savebones(struct obj *);
E int getbones(void);

/* ### botl.c ### */

E int xlev_to_rank(int);
E int title_to_mon(const char *, int *, int *);
E void max_rank_sz(void);
#ifdef SCORE_ON_BOTL
E long botl_score(void);
#endif
E int describe_level(char *);
E const char *rank(void);
E const char *rank_of(int, short, boolean);
extern const char* botl_realtime(void);
E void bot(void);
#ifdef DUMP_LOG
E void bot1str(char *);
E void bot2str(char *);
E void bot3str(char *);
#endif

/* ### cmd.c ### */

E char randomkey(void);
extern boolean redraw_cmd(char);
extern char cmd_from_func(int(*)(void));
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
E void set_occupation(int (*)(void), const char *, int);
#ifdef REDO
E char pgetchar(void);
E void pushch(char);
E void savech(char);
#endif
#ifdef WIZARD
E void add_debug_extended_commands(void);
#endif /* WIZARD */
extern boolean bind_specialkey(uchar, const char *);
extern char txt2key(char *);
E void parseautocomplete(char *, boolean);
E void reset_commands(boolean);
E void rhack(char *);
E int doextlist(void);
E int extcmd_via_menu(void);
extern boolean bind_key(uchar, const char *);
E void enlightenment(int, boolean);
E void show_conduct(int, boolean);
E int xytod(schar, schar);
E void dtoxy(coord *, int);
E int movecmd(char);
E int dxdy_moveok(void);
E int getdir(const char *);
E void confdir(void);
E const char *directionname(int);
E int isok(int, int);
E int get_adjacent_loc(const char *, const char *, xchar, xchar, coord *);
E const char *click_to_cmd(int, int, int);
E char readchar(void);
E int do_stair_travel(char);
#ifdef WIZARD
E void sanity_check(void);
#endif
E char* key2txt(uchar, char *);
E char yn_function(const char *, const char *, char);
E char paranoid_yn(const char *, boolean);
E void sokoban_trickster(void);
E void list_vanquished(char, boolean);
E void list_vanquishedonly(void);
#ifdef TTY_GRAPHICS
E int tty_debug_show_colors(void);
#endif
#ifdef CURSES_GRAPHICS
E int curses_debug_show_colors(void);
#endif

/* ### dbridge.c ### */

E boolean is_pool(int, int);
E boolean is_lava(int, int);
E boolean is_pool_or_lava(int, int);
E boolean is_ice(int, int);
E boolean is_moat(int, int);
E boolean is_swamp(int, int);
E boolean is_icewall(int, int);
E boolean is_any_icewall(int, int);
E schar db_under_typ(struct rm *lev);
E int is_drawbridge_wall(int, int);
E boolean is_db_wall(int, int);
E boolean find_drawbridge(int *, int*);
E boolean create_drawbridge(int, int, int, int);
E void open_drawbridge(int, int);
E boolean close_drawbridge(int, int);
E void destroy_drawbridge(int, int);

/* ### decl.c ### */

E void decl_init(void);

/* ### detect.c ### */

E boolean trapped_chest_at(int, int, int);
E boolean trapped_door_at(int, int, int);
E struct obj *o_in(struct obj*, char);
E struct obj *o_material(struct obj*, unsigned);
E int gold_detect(struct obj *);
E int food_detect(struct obj *);
E int object_detect(struct obj *, int, boolean);
E int monster_detect(struct obj *, int);
E int trap_detect(struct obj *);
E const char *level_distance(d_level *);
E void use_crystal_ball(struct obj **);
E void do_mapping(void);
E void do_vicinity_map(void);
E void cvt_sdoor_to_door(struct rm *);
#ifdef USE_TRAMPOLI
E void findone(int, int, genericptr_t);
E void openone(int, int, genericptr_t);
#endif
E int findit(void);
E int openit(void);
E boolean detecting(void (*)(int, int, void *));
E void find_trap(struct trap *);
E int dosearch0(int);
E int dosearch(void);
E void sokoban_detect(void);
extern void reveal_terrain(int, int);

/* ### dig.c ### */

E int dig_typ(struct obj *, xchar, xchar);
E boolean is_digging(void);
#ifdef USE_TRAMPOLI
E int dig(void);
#endif
E int holetime(void);
E boolean dig_check(struct monst *, boolean, int, int);
E void digactualhole(int, int, struct monst *, int);
E boolean dighole(boolean, boolean, coord *);
E int use_pick_axe(struct obj *);
E int use_pick_axe2(struct obj *);
E boolean mdig_tunnel(struct monst *);
E void draft_message(boolean);
E void watch_dig(struct monst *, xchar, xchar, boolean);
E void zap_dig(void);
E struct obj *bury_an_obj(struct obj *, boolean *);
E void bury_objs(int, int);
E void unearth_objs(int, int);
E void rot_organic(ANY_P *, long);
E void rot_corpse(ANY_P *, long);
E struct obj *buried_ball(coord *);
E void buried_ball_to_punishment(void);
E void buried_ball_to_freedom(void);
E schar fillholetyp(int, int, boolean);
E void liquid_flow(xchar, xchar, schar, struct trap *, const char *);
E boolean conjoined_pits(struct trap *, struct trap *, boolean);
#if 0
E void bury_monst(struct monst *);
E void bury_you(void);
E void unearth_you(void);
E void escape_tomb(void);
E void bury_obj(struct obj *);
#endif

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
E struct obj * vobj_at(xchar, xchar);
#endif /* INVISIBLE_OBJECTS */
extern int is_safemon(struct monst *);
E void magic_map_background(xchar, xchar, int);
E void map_background(xchar, xchar, int);
E void map_trap(struct trap *, int);
E void map_object(struct obj *, int);
E void map_invisible(xchar, xchar);
E boolean unmap_invisible(int, int);
E void unmap_object(int, int);
extern struct obj *vobj_at(int, int);
E void map_location(int, int, int);
E void feel_newsym(xchar, xchar);
E void feel_location(xchar, xchar);
E void newsym(int, int);
E void newsym_force(int, int);
E void shieldeff(xchar, xchar);
E void tmp_at(int, int);
extern void flash_glyph_at(int, int, int, int);
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
E void show_glyph(int, int, int);
E void clear_glyph_buffer(void);
E void row_refresh(int, int, int);
E void cls(void);
E void flush_screen(int);
#ifdef DUMP_LOG
E void dump_screen(void);
#endif
E int back_to_glyph(xchar, xchar);
E int zapdir_to_glyph(int, int, int);
E int glyph_at(xchar, xchar);
E void set_wall_state(void);
E void unset_seenv(struct rm *, int, int, int, int);
E int warning_of(struct monst *);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
E int drop(struct obj *);
E int wipeoff(void);
#endif
E int dodrop(void);
E boolean boulder_hits_pool(struct obj *, int, int, boolean);
E boolean flooreffects(struct obj *, int, int, const char *);
E void doaltarobj(struct obj *);
E boolean canletgo(struct obj *, const char *);
E void dropx(struct obj *);
E void dropy(struct obj *);
E void dropz(struct obj *, boolean);
E void obj_no_longer_held(struct obj *);
E int doddrop(void);
E int dodown(void);
E int doup(void);
#ifdef INSURANCE
E void save_currentstate(void);
#endif
E void goto_level(d_level *, boolean, boolean, boolean);
E void schedule_goto(d_level *, boolean, boolean, int, const char *, const char *);
E void deferred_goto(void);
E boolean revive_corpse(struct obj *);
E void revive_mon(ANY_P *, long);
extern void zombify_mon(union any *, long);
E int donull(void);
E int dowipe(void);
E void set_wounded_legs(long, int);
E void heal_legs(int);

/* ### do_name.c ### */

E void do_oname(struct obj *);
E char *coord_desc(int, int, char *, char);
E boolean getpos_menu(coord *, int);
extern int getpos(coord *, boolean, const char *);
E void getpos_sethilite(void (*f)(int), boolean (*d)(int,int));
extern void new_mgivenname(struct monst *, int);
extern void free_mgivenname(struct monst *);
E void new_oname(struct obj *, int);
E void free_oname(struct obj *);
E const char *safe_oname(struct obj *);
E struct monst *christen_monst(struct monst *, const char *);
E int do_mname(void);
E struct obj *oname(struct obj *, const char *);
E boolean objtyp_is_callable(int);
E int ddocall(void);
E void docall(struct obj *);
E void docall_input(int);
void namefloorobj(void);
E const char *rndghostname(void);
E char *x_monnam(struct monst *, int, const char *, int, boolean);
E char *l_monnam(struct monst *);
E char *mon_nam(struct monst *);
E char *noit_mon_nam(struct monst *);
E char *Monnam(struct monst *);
E char *noit_Monnam(struct monst *);
E char *noname_monnam(struct monst *, int);
E char *m_monnam(struct monst *);
E char *y_monnam(struct monst *);
E char *Adjmonnam(struct monst *, const char *);
E char *Amonnam(struct monst *);
E char *a_monnam(struct monst *);
E char *distant_monnam(struct monst *, int, char *);
E char *mon_nam_too(struct monst *, struct monst *);
E char *minimal_monnam(struct monst *, boolean);
E const char *rndmonnam(void);
E const char *hcolor(const char *);
E const char *rndcolor(void);
E const char *hliquid(const char *);
#ifdef REINCARNATION
E const char *roguename(void);
#endif
E struct obj *realloc_obj(struct obj *, int, genericptr_t, int, const char *);
E char *coyotename(struct monst *, char *);
E char *rndorcname(char *);
E struct monst *christen_orc(struct monst *, const char *, const char *);
E const char *noveltitle(int *);
E const char *lookup_novel(const char *, int *);
extern char *mon_wounds(struct monst *);
extern void print_mon_wounded(struct monst *, int);

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
E const char *fingers_or_gloves(boolean);
E void off_msg(struct obj *);
E void set_wear(void);
E boolean donning(struct obj *);
E boolean doffing(struct obj *);
E void cancel_doff(struct obj *, long);
E void cancel_don(void);
E int stop_donning(struct obj *);
E int Armor_off(void);
E int Armor_gone(void);
E int Helmet_off(void);
E int Gloves_off(void);
E int Boots_on(void);
E int Boots_off(void);
E int Cloak_off(void);
E int Shield_off(void);
E int Shirt_off(void);
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
E int canwearobj(struct obj *, long *, boolean);
E int dowear(void);
E int doputon(void);
E void find_ac(void);
E void glibr(void);
E struct obj *some_armor(struct monst *);
E void erode_armor(struct monst *, int);
E struct obj *stuck_ring(struct obj *, int);
E struct obj *unchanger(void);
E void reset_remarm(void);
E int doddoremarm(void);
E int destroy_arm(struct obj *);
E void adj_abon(struct obj *, schar);
E boolean inaccessible_equipment(struct obj *, const char *, boolean);

/* ### dog.c ### */

E void newedog(struct monst *);
E void free_edog(struct monst *);
E void initedog(struct monst *);
E struct monst *make_familiar(struct obj *, xchar, xchar, boolean);
E struct monst *makedog(void);
E void update_mlstmv(void);
E void losedogs(void);
E void mon_arrive(struct monst *, boolean);
E void mon_catchup_elapsed_time(struct monst *, long);
E void keepdogs(boolean);
E void migrate_to_level(struct monst *, xchar, xchar, coord *);
E int dogfood(struct monst *, struct obj *);
E boolean tamedog(struct monst *, struct obj *);
E void abuse_dog(struct monst *);
E void wary_dog(struct monst *, boolean);

/* ### dogmove.c ### */

E struct obj *droppables(struct monst *);
E int dog_nutrition(struct monst *, struct obj *);
E int dog_eat(struct monst *, struct obj *, int, int, boolean);
E int dog_move(struct monst *, int);
#ifdef USE_TRAMPOLI
E void wantdoor(int, int, genericptr_t);
#endif
E void finish_meating(struct monst *);
E boolean cursed_object_at(int, int);

/* ### dokick.c ### */

E boolean ghitm(struct monst *, struct obj *);
E void container_impact_dmg(struct obj *, xchar, xchar);
E int dokick(void);
E boolean ship_object(struct obj *, xchar, xchar, boolean);
E void obj_delivery(boolean);
E void deliver_obj_to_mon(struct monst *mtmp, int, unsigned long);
E schar down_gate(xchar, xchar);
E void impact_drop(struct obj *, xchar, xchar, xchar);

/* ### dothrow.c ### */

E int dothrow(void);
E int dofire(void);
E void endmultishot(boolean);
E void hitfloor(struct obj *, boolean);
E void hurtle(int, int, int, boolean);
E void mhurtle(struct monst *, int, int, int);
E boolean throwing_weapon(struct obj *);
E void throwit(struct obj *, long, boolean);
E int omon_adj(struct monst *, struct obj *, boolean);
E int thitmonst(struct monst *, struct obj *);
E int hero_breaks(struct obj *, xchar, xchar, boolean);
E int breaks(struct obj *, xchar, xchar);
E void breakobj(struct obj *, xchar, xchar, boolean, boolean);
E boolean breaktest(struct obj *);
E boolean walk_path(coord *, coord *, boolean (*)(genericptr_t, int, int), genericptr_t);
E boolean hurtle_jump(void *, int, int);
E boolean hurtle_step(void *, int, int);

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E int def_char_to_objclass(char);
E int def_char_to_monclass(char);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void assign_graphics(glyph_t *, int, int, int);
E void switch_graphics(int);
#ifdef REINCARNATION
E void assign_rogue_graphics(boolean);
#endif
E void assign_utf8graphics_symbol(int, glyph_t);
E void assign_moria_graphics(boolean);

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
E void dump_containerconts(struct obj *, boolean, boolean, boolean);
extern const char* html_escape_character(const char);
E char* html_link(const char *, const char *);
#ifdef DUMP_LOG
E int dump_screenshot(void);
E int dumpoverview(void);
#endif

/* ### dungeon.c ### */

E void save_dungeon(int, boolean, boolean);
E void restore_dungeon(int);
E void insert_branch(branch *, boolean);
E void init_dungeons(void);
E s_level *find_level(const char *);
E s_level *Is_special(d_level *);
#ifdef RANDOMIZED_PLANES
E s_level *get_next_elemental_plane(d_level *);
E d_level *get_first_elemental_plane(void);
#endif
E branch *Is_branchlev(d_level *);
E boolean builds_up(d_level *);
E xchar ledger_no(d_level *);
E xchar maxledgerno(void);
E schar depth(d_level *);
E xchar dunlev(d_level *);
E xchar dunlevs_in_dungeon(d_level *);
E xchar ledger_to_dnum(xchar);
E xchar ledger_to_dlev(xchar);
E xchar deepest_lev_reached(boolean);
E boolean on_level(d_level *, d_level *);
E void next_level(boolean);
E void prev_level(boolean);
E void u_on_newpos(int, int);
E void u_on_rndspot(int);
E void u_on_sstairs(int);
E void u_on_upstairs(void);
E void u_on_dnstairs(void);
E boolean On_stairs(xchar, xchar);
E void get_level(d_level *, int);
E boolean Is_botlevel(d_level *);
E boolean Can_fall_thru(d_level *);
E boolean Can_dig_down(d_level *);
E boolean Can_rise_up(int, int, d_level *);
E boolean has_ceiling(d_level *);
E boolean In_quest(d_level *);
E boolean In_mines(d_level *);
E boolean In_sheol(d_level *);
E branch *dungeon_branch(const char *);
E boolean at_dgn_entrance(const char *);
E boolean In_hell(d_level *);
E boolean In_V_tower(d_level *);
E boolean On_W_tower_level(d_level *);
E boolean In_W_tower(int, int, d_level *);
E void find_hell(d_level *);
E void goto_hell(boolean, boolean);
E void assign_level(d_level *, d_level *);
E void assign_rnd_level(d_level *, d_level *, int);
E int induced_align(int);
E boolean Invocation_lev(d_level *);
E xchar level_difficulty(void);
E schar lev_by_name(const char *);
#ifdef WIZARD
E schar print_dungeon(boolean, schar *, xchar *);
#endif
E int donamelevel(void);
E int dooverview(void);
E void show_overview(int, int);
E void forget_mapseen(int);
E void init_mapseen(d_level *);
E void recalc_mapseen(void);
E void mapseen_temple(struct monst *);
E void recbranch_mapseen(d_level *, d_level *);
E void room_discovered(int);
E void overview_stats(winid, const char *, long *, long *);
E void remdun_mapseen(int);
E char *get_annotation(d_level *);
E const char *get_generic_level_description(d_level *);
E const char *endgame_level_name(char *, int);

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
E void newuhs(boolean);
E struct obj *floorfood(const char *, int);
E void vomit(void);
E int eaten_stat(int, struct obj *);
E void food_disappears(struct obj *);
E void food_substitution(struct obj *, struct obj *);
E boolean bite_monster(struct monst *mon);
E void fix_petrification(void);
extern int intrinsic_possible(int, struct permonst *);
E void consume_oeaten(struct obj *, int);
E boolean maybe_finished_meal(boolean);
E void set_tin_variety(struct obj *, int);
E int tin_variety_txt(char *, int *);
E void tin_details(struct obj *, int, char *);
E boolean Popeye(int);

/* ### end.c ### */

E void done1(int);
E int done2(void);
#ifdef USE_TRAMPOLI
E void done_intr(int);
#endif
E void done_in_by(struct monst *);
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E void VDECL(panic, (const char *, ...)) PRINTF_F(1, 2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void done(int);
E void container_contents(struct obj *, boolean, boolean, boolean);
E void nh_terminate(int) NORETURN;
E int num_genocides(void);
E void delayed_killer(int, int, const char *);
E struct kinfo *find_delayed_killer(int);
E void dealloc_killer(struct kinfo *);
E void save_killers(int, int);
E void restore_killers(int);
E char *build_english_list(char *);

/* ### engrave.c ### */

E char *random_engraving(char *);
E void wipeout_text(char *, int, unsigned);
E boolean can_reach_floor(boolean);
E void You_cant_reach_the_floor(int, int, boolean);
E void You_cant_reach_the_ceiling(int, int);
E void cant_reach_floor(int, int, boolean, boolean);
E const char *surface(int, int);
E const char *ceiling(int, int);
E struct engr *engr_at(xchar, xchar);
E int sengr_at(const char *, xchar, xchar);
E void u_wipe_engr(int);
E void wipe_engr_at(xchar, xchar, xchar);
E void read_engr_at(int, int);
E void make_engr_at(int, int, const char *, long, xchar);
E void del_engr_at(int, int);
E int freehand(void);
E int doengrave(void);
E int doengrave_elbereth(void);
E void sanitize_engravings(void);
E void save_engravings(int, int);
E void rest_engravings(int);
E void engr_stats(const char *, char *, long *, long *);
E void del_engr(struct engr *);
E void rloc_engr(struct engr *);
E void make_grave(int, int, const char *);

/* ### exper.c ### */

E long newuexp(int);
E int newpw(void);
E int experience(struct monst *, int);
E void more_experienced(int, int, int);
E void losexp(const char *);
E void newexplevel(void);
E void pluslvl(boolean);
E long rndexp(boolean);

/* ### explode.c ### */

E void explode(int, int, int, int, char, int);
E long scatter(int, int, int, unsigned int, struct obj *);
E void splatter_burning_oil(int, int, boolean);
E void explode_oil(struct obj *, int, int);

/* ### extralev.c ### */

#ifdef REINCARNATION
E void makeroguerooms(void);
E void corr(int, int);
E void makerogueghost(void);
#endif

/* ### files.c ### */

E char *fname_encode(const char *, char, char *, char *, int);
E char *fname_decode(char, char *, char *, int);
E const char *fqname(const char *, int, int);
#ifndef FILE_AREAS
E FILE *fopen_datafile(const char *, const char *, int);
#endif
E boolean uptodate(int, const char *);
E void store_version(int);
#ifdef MFLOPPY
E void set_lock_and_bones(void);
#endif
E void set_levelfile_name(char *, int);
E int create_levelfile(int, char *);
E int open_levelfile(int, char *);
E void delete_levelfile(int);
E void clearlocks(void);
E int create_bonesfile(d_level*, char **, char *);
#ifdef MFLOPPY
E void cancel_bonesfile(void);
#endif
E void commit_bonesfile(d_level *);
E int open_bonesfile(d_level*, char **);
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
E boolean lock_file(const char *, int, int);
E void unlock_file(const char *);
#endif
#ifdef USER_SOUNDS
E boolean can_read_file(const char *);
#endif
extern boolean read_config_file(const char *, int);
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
#ifdef SYSCF_FILE
extern void assure_syscf_file(void);
#endif
E int nhclose(int);
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
E void gush(int, int, genericptr_t);
# endif
E void dryup(xchar, xchar, boolean);
E void drinkfountain(void);
E void dipfountain(struct obj *);
#ifdef SINKS
E void breaksink(int, int);
E void drinksink(void);
#endif

/* ### hack.c ### */

#ifdef DUNGEON_GROWTH
E void catchup_dgn_growths(int);
E void dgn_growths(boolean, boolean);
#endif
extern boolean is_valid_travelpt(int,int);
E anything *uint_to_any(unsigned);
E anything *long_to_any(long);
E anything *monst_to_any(struct monst *);
E anything *obj_to_any(struct obj *);
E boolean revive_nasty(int, int, const char*);
E void movobj(struct obj *, xchar, xchar);
E boolean may_dig(xchar, xchar);
E boolean may_passwall(xchar, xchar);
E boolean bad_rock(struct permonst *, xchar, xchar);
E int cant_squeeze_thru(struct monst *);
E boolean invocation_pos(xchar, xchar);
E boolean test_move(int, int, int, int, int);
E boolean u_rooted(void);
E void domove(void);
extern void runmode_delay_output(void);
E boolean overexertion(void);
E void invocation_message(void);
E void switch_terrain(void);
E boolean pooleffects(boolean);
E void wounds_message(struct monst *);
E char *mon_wounds(struct monst *);
E void spoteffects(boolean);
E char *in_rooms(xchar, xchar, int);
E boolean in_town(int, int);
E void check_special_room(boolean);
E int dopickup(void);
E void lookaround(void);
E boolean crawl_destination(int, int);
E int monster_nearby(void);
E void nomul(int, const char *);
E void unmul(const char *);
E void showdmg(int, boolean);
E void losehp(int, const char *, boolean);
E void losehp_how(int, const char *, boolean, int);
E void set_uhpmax(int, boolean);
E void check_uhpmax(void);
E int weight_cap(void);
E int inv_weight(void);
E int near_capacity(void);
E int calc_capacity(int);
E int max_capacity(void);
E boolean check_capacity(const char *);
E int inv_cnt(boolean);
E long money_cnt(struct obj *);
extern boolean MON_AT(int, int);
extern boolean OBJ_AT(int, int);

/* ### hacklib.c ### */

E boolean digit(char);
E boolean letter(char);
E char highc(char);
E char lowc(char);
E char *lcase(char *);
E char *ucase(char *);
E char *upstart(char *);
E char *mungspaces(char *);
E char *trimspaces(char *);
E char *strip_newline(char *);
E char *eos(char *);
E void sanitizestr(char *);
E boolean str_end_is(const char *, const char *);
E char *strkitten(char *, char);
E void copynchars(char *, const char *, int);
E char *strcasecpy(char *, const char *);
E char *s_suffix(const char *);
E char *xcrypt(const char *, char *);
E boolean onlyspace(const char *);
E char *tabexpand(char *);
E char *visctrl(char);
E char *strsubst(char *, const char *, const char *);
E int strNsubst(char *, const char *, const char *, int);
E const char *ordin(int);
E char *sitoa(int);
E int sgn(int);
E int rounddiv(long, int);
E int dist2(int, int, int, int);
E int isqrt(int);
E int ilog2(int);
E int distmin(int, int, int, int);
E boolean online2(int, int, int, int);
E boolean pmatch(const char *, const char *);
E boolean pmatchi(const char *, const char *);
#ifndef STRNCMPI
E int strncmpi(const char *, const char *, int);
#endif
#ifndef STRSTRI
E char *strstri(const char *, const char *);
#endif
E boolean fuzzymatch(const char *, const char *, const char *, boolean);
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
extern boolean is_june(void);
E char *iso8601(time_t);
E char *iso8601_duration(long);
extern char* format_duration(long);
E char *get_formatted_time(time_t, const char *);
E time_t current_epoch(void);
E void strbuf_init(strbuf_t *);
E void strbuf_append(strbuf_t *, const char *);
E void strbuf_reserve(strbuf_t *, int);
E void strbuf_empty(strbuf_t *);
E void strbuf_nl_to_crlf(strbuf_t *);
extern int swapbits(int, int, int);
extern void strip_brackets(char *);
/* note: the snprintf CPP wrapper includes the "fmt" argument in "..."
   (__VA_ARGS__) to allow for zero arguments after fmt */
#define Snprintf(str, size, ...) \
    nh_snprintf(__func__, __LINE__, str, size, __VA_ARGS__)
extern void nh_snprintf(const char *func, int line, char *str, size_t size,
                        const char *fmt, ...);

/* ### invent.c ### */

E void assigninvlet(struct obj *);
E struct obj *merge_choice(struct obj *, struct obj *);
E int merged(struct obj **, struct obj **);
#ifdef USE_TRAMPOLI
E int ckunpaid(struct obj *);
#endif
E void addinv_core1(struct obj *);
E void addinv_core2(struct obj *);
E struct obj *addinv(struct obj *);
E struct obj *hold_another_object(struct obj *, const char *, const char *, const char *);
E void useupall(struct obj *);
E void useup(struct obj *);
E void consume_obj_charge(struct obj *, boolean);
E void freeinv_core(struct obj *);
E void freeinv(struct obj *);
E void delallobj(int, int);
E void delobj(struct obj *);
E struct obj *sobj_at(int, int, int);
E struct obj *nxtobj(struct obj *, int, boolean);
E struct obj *carrying(int);
E boolean have_lizard(void);
E struct obj *o_on(unsigned int, struct obj *);
E boolean obj_here(struct obj *, int, int);
E boolean wearing_armor(void);
E boolean is_worn(struct obj *);
E struct obj *g_at(int, int);
E struct obj *mkgoldobj(long);
E struct obj *getobj(const char *, const char *);
E int ggetobj(const char *, int (*)(OBJ_P), int, boolean, unsigned *);
E void fully_identify_obj(struct obj *);
E int identify(struct obj *);
E void identify_pack(int, boolean);
E int askchain(struct obj **, const char *, int, int (*)(OBJ_P), int (*)(OBJ_P), int, const char *);
E void prinv(const char *, struct obj *, long);
E char *xprname(struct obj *, const char *, char, boolean, long, long);
E int ddoinv(void);
E char display_inventory(const char *, boolean);
E char dump_inventory(const char *, boolean, boolean);
E int display_binventory(int, int, boolean);
E struct obj *display_cinventory(struct obj *);
E struct obj *display_minventory(struct monst *, int, char *);
E int dotypeinv(void);
E const char *dfeature_at(int, int, char *);
E int look_here(int, boolean);
E int dolook(void);
E boolean will_feel_cockatrice(struct obj *, boolean);
E void feel_cockatrice(struct obj *, boolean);
E void stackobj(struct obj *);
E boolean mergable(struct obj *, struct obj *);
E int doprgold(void);
E int doprwep(void);
E int doprarm(void);
E int doprring(void);
E int dopramulet(void);
E int doprtool(void);
E int doprinuse(void);
E void useupf(struct obj *, long);
E char *let_to_name(char, boolean);
E void free_invbuf(void);
E void reassign(void);
E int doorganize(void);
E int count_objects(struct obj *);
E int count_unidentified(struct obj *);
E void learn_unseen_invent(void);
E void update_inventory(void);
E int count_unpaid(struct obj *);
E int count_buc(struct obj *, int, boolean (*)(OBJ_P));
E void tally_BUCX(struct obj *, boolean, int *, int *, int *, int *, int *);
E long count_contents(struct obj *, boolean, boolean, boolean, boolean);
E void carry_obj_effects(struct obj *);
E const char *currency(long);
E void silly_thing(const char *, struct obj *);
E struct obj *getnextgetobj(void);
#ifdef SORTLOOT
E int sortloot_cmp(struct obj *, struct obj *);
#endif
E boolean is_racial_armor(struct obj *);
E boolean is_racial_weapon(struct obj *);
extern boolean is_dragon_identified(struct permonst *);
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

E void new_light_source(xchar, xchar, int, int, ANY_P *);
E void del_light_source(int, ANY_P *);
E void do_light_sources(char **);
E void show_transient_light(struct obj *, int, int);
E void transient_light_cleanup(void);
E struct monst *find_mid(unsigned, unsigned);
E void save_light_sources(int, int, int);
E void restore_light_sources(int);
E void light_stats(const char *, char *, long *, long *);
E void relink_light_sources(boolean);
E void light_sources_sanity_check(void);
E void obj_move_light_source(struct obj *, struct obj *);
E boolean any_light_source(void);
E void snuff_light_source(int, int);
E boolean obj_sheds_light(struct obj *);
E boolean obj_is_burning(struct obj *);
E void obj_split_light_source(struct obj *, struct obj *);
E void obj_merge_light_sources(struct obj *, struct obj *);
E void obj_adjust_light_radius(struct obj *, int);
E int candle_light_range(struct obj *);
E int arti_light_radius(struct obj *);
E const char *arti_light_description(struct obj *);
#ifdef WIZARD
E int wiz_light_sources(void);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
E int forcelock(void);
E int picklock(void);
#endif
E boolean picking_lock(int *, int *);
E boolean picking_at(int, int);
E void breakchestlock(struct obj *, boolean);
E void reset_pick(void);
E void maybe_reset_pick(struct obj *);
E int pick_lock(struct obj *, int, int, boolean);
E int doforce(void);
E boolean boxlock(struct obj *, struct obj *);
E boolean doorlock(struct obj *, int, int);
E int doopen(void);
E int doclose(void);
E int artifact_door(int, int);
E boolean stumble_on_door_mimic(int, int);
#ifdef AUTO_OPEN
E int doopen_indir(int, int);
#endif

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

E int maccreat(const char *, long);
E int macopen(const char *, int, long);
E int macclose(int);
E int macread(int, void *, unsigned);
E int macwrite(int, void *, unsigned);
E long macseek(int, long, short);
E int macunlink(const char *);

/* ### macsnd.c ### */

E void mac_speaker(struct obj *, char *);

/* ### macunix.c ### */

E void regularize(char *);
E void getlock(void);

/* ### macwin.c ### */

E void lock_mouse_cursor(Boolean);
E int SanePositions(void);

/* ### mttymain.c ### */

E void getreturn(char *);
E void VDECL(msmsg, (const char *, ...));
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

E void mon_sanity_check(void);
E void dealloc_monst(struct monst *);
E boolean is_home_elemental(struct permonst *);
E struct monst *clone_mon(struct monst *, xchar, xchar);
E int monhp_per_lvl(struct monst *);
E void newmonhp(struct monst *, int);
E struct mextra *newmextra(void);
E void copy_mextra(struct monst *, struct monst *);
E void dealloc_mextra(struct monst *);
E struct monst *makemon(struct permonst *, int, int, int);
E boolean create_critters(int, struct permonst *);
E struct permonst *rndmonst(void);
E void reset_rndmonst(int);
E struct permonst *mkclass(char, int);
E struct permonst *mkclass_aligned(char, int, aligntyp);
E int mkclass_poly(int);
E int adj_lev(struct permonst *);
E struct permonst *grow_up(struct monst *, struct monst *);
E int mongets(struct monst *, int);
E int golemhp(int);
E boolean peace_minded(struct permonst *);
E void set_malign(struct monst *);
E void newmcorpsenm(struct monst *);
E void freemcorpsenm(struct monst *);
E void set_mimic_sym(struct monst *);
E int mbirth_limit(int);
E void mimic_hit_msg(struct monst *, short);
E void mkmonmoney(struct monst *, long);
E int bagotricks(struct obj *);
E boolean propagate(int, boolean, boolean);
E void create_camera_demon(struct obj *, int, int);
E int min_monster_difficulty(void);
E int max_monster_difficulty(void);
E boolean prohibited_by_generation_flags(struct permonst *);
E boolean usmellmon(struct permonst *);

/* ### mapglyph.c ### */

E void mapglyph(int, glyph_t *, int *, unsigned *, int, int, unsigned);
E char *encglyph(int);
extern glyph_t get_monsym(int glyph);

/* ### mcastu.c ### */

E int castmu(struct monst *, struct attack *, boolean, boolean);
E int buzzmu(struct monst *, struct attack *);

/* ### mhitm.c ### */

E int fightm(struct monst *);
E int mattackm(struct monst *, struct monst *);
E boolean engulf_target(struct monst *, struct monst *);
E int mdisplacem(struct monst *, struct monst *, boolean);
E void paralyze_monst(struct monst *, int);
E int sleep_monst(struct monst *, int, int);
E void slept_monst(struct monst *);
E long attk_protection(int);
E void rustm(struct monst *, struct obj *);
E void maybe_freeze_m(struct monst *, int, int*);

/* ### mhitu.c ### */

E const char *mpoisons_subj(struct monst *, struct attack *);
E void u_slow_down(void);
E struct monst *cloneu(void);
E void expels(struct monst *, struct permonst *, boolean);
E struct attack *getmattk(struct monst *, struct monst *, int, int *, struct attack *);
E int mattacku(struct monst *);
E int magic_negation(struct monst *);
E boolean gulp_blnd_check(void);
E int gazemu(struct monst *, struct attack *);
E void mdamageu(struct monst *, int);
E int could_seduce(struct monst *, struct monst *, struct attack *);
#ifdef SEDUCE
E int doseduce(struct monst *);
#endif
E void maybe_freeze_u(int*);
E void spore_dies(struct monst *);

/* ### minion.c ### */

E void newemin(struct monst *);
E void free_emin(struct monst *);
E int monster_census(boolean);
E int msummon(struct monst *);
E void summon_minion(aligntyp, boolean);
E int demon_talk(struct monst *);
E long bribe(struct monst *);
E int dprince(aligntyp);
E int dlord(aligntyp);
E int llord(void);
E int ndemon(aligntyp);
E int lminion(void);
E void lose_guardian_angel(struct monst *);
E void gain_guardian_angel(void);

/* ### mklev.c ### */

E void mineralize(int, int, int, int, boolean);
#ifdef USE_TRAMPOLI
E int do_comp(genericptr_t, genericptr_t);
#endif
E void sort_rooms(void);
E void add_room(int, int, int, int, boolean, schar, boolean);
E void add_subroom(struct mkroom *, int, int, int, int, boolean, schar, boolean);
E void makecorridors(int);
E void add_door(int, int, struct mkroom *);
E void mkpoolroom(void);
E void mklev(void);
#ifdef SPECIALIZATION
E void topologize(struct mkroom *, boolean);
#else
E void topologize(struct mkroom *);
#endif
E void place_branch(branch *, xchar, xchar);
E boolean occupied(xchar, xchar);
E int okdoor(xchar, xchar);
E void dodoor(int, int, struct mkroom *);
E void mktrap(int, int, struct mkroom *, coord*);
E void mkstairs(xchar, xchar, char, struct mkroom *);
E void mkinvokearea(void);
E void wallwalk_right(xchar, xchar, schar, schar, schar, int);
#ifdef ADVENT_CALENDAR
E boolean mk_advcal_portal(void);
#endif
extern void mk_knox_vault(int, int, int, int);
extern d_level * get_floating_branch(d_level *, branch *);

/* ### mkmap.c ### */

void flood_fill_rm(int, int, int, boolean, boolean);
void remove_rooms(int, int, int, int);

/* ### mkmaze.c ### */

E void wallification(int, int, int, int);
E void wall_extends(int, int, int, int);
E void walkfrom(int, int, schar);
E void makemaz(const char *);
E void mazexy(coord *);
extern void get_level_extends(int *, int *, int *, int *);
E void bound_digging(void);
E void mkportal(xchar, xchar, xchar, xchar);
E boolean bad_location(xchar, xchar, xchar, xchar, xchar, xchar, xchar);
E int place_lregion(xchar, xchar, xchar, xchar,
                    xchar, xchar, xchar, xchar,
                    xchar, d_level *);
E void fixup_special(void);
E void movebubbles(void);
E void water_friction(void);
E void save_waterlevel(int, int);
E void restore_waterlevel(int);
E const char *waterbody_name(xchar, xchar);

#ifdef ADVENT_CALENDAR
E void fill_advent_calendar(boolean);
#endif

/* ### mkobj.c ### */

E struct oextra *newoextra(void);
E void copy_oextra(struct obj *, struct obj *);
E void dealloc_oextra(struct obj *);
E void newomonst(struct obj *);
E void free_omonst(struct obj *);
E void newomid(struct obj *);
E void free_omid(struct obj *);
E void newolong(struct obj *);
E void free_olong(struct obj *);
E void new_omailcmd(struct obj *, const char *);
E void free_omailcmd(struct obj *);
E struct obj *mkobj_at(char, int, int, boolean);
E void costly_alteration(struct obj *, int);
E struct obj *mksobj_at(int, int, int, boolean, boolean);
E struct obj *mksobj_migr_to_species(int, unsigned, boolean, boolean);
E struct obj *mkobj(char, boolean);
E int rndmonnum(void);
E boolean bogon_is_pname(char);
E struct obj *splitobj(struct obj *, long);
E struct obj *unsplitobj(struct obj *);
E void clear_splitobjs(void);
E void replace_object(struct obj *, struct obj *);
E void bill_dummy_object(struct obj *);
E struct obj *mksobj(int, boolean, boolean);
E int bcsign(struct obj *);
E int weight(struct obj *);
E struct obj *mkgold(long, int, int);
E struct obj *mkcorpstat(int, struct monst *, struct permonst *, int, int, boolean);
E struct obj *obj_attach_mid(struct obj *, unsigned);
E struct monst *get_mtraits(struct obj *, boolean);
E struct obj *mk_tt_object(int, int, int);
E struct obj *mk_named_object(int, struct permonst *, int, int, const char *);
E struct obj *rnd_treefruit_at(int, int);
E void rnd_treesticks_at(int, int);
E void set_corpsenm(struct obj *, int);
E void start_corpse_timeout(struct obj *);
E void bless(struct obj *);
E void unbless(struct obj *);
E void curse(struct obj *);
E void uncurse(struct obj *);
E void blessorcurse(struct obj *, int);
E void set_bknown(struct obj *, unsigned);
E boolean is_flammable(struct obj *);
E boolean is_rottable(struct obj *);
E void place_object(struct obj *, int, int);
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
E void obj_ice_effects(int, int, boolean);
E long peek_at_iced_corpse_age(struct obj *);
E int hornoplenty(struct obj *, boolean);
E int do_stair_travel(char);
#ifdef WIZARD
E void obj_sanity_check(void);
#endif
E struct obj *obj_nexto(struct obj *);
E struct obj *obj_nexto_xy(struct obj *, int, int, boolean);
E struct obj *obj_absorb(struct obj **, struct obj **);
E struct obj *obj_meld(struct obj **, struct obj **);
E void pudding_merge_message(struct obj *, struct obj *);
E struct obj *init_dummyobj(struct obj *, short, long);

/* ### mkroom.c ### */

E struct mkroom * pick_room(boolean);
E void mkroom(int);
E void fill_zoo(struct mkroom *);
E boolean nexttodoor(int, int);
E boolean bydoor(xchar, xchar);
E boolean somexyspace(struct mkroom *, coord *, int);
E boolean has_dnstairs(struct mkroom *);
E boolean has_upstairs(struct mkroom *);
E int somex(struct mkroom *);
E int somey(struct mkroom *);
E boolean inside_room(struct mkroom *, xchar, xchar);
E boolean somexy(struct mkroom *, coord *);
E void mkundead(coord *, boolean, int);
E struct permonst *courtmon(void);
E void save_rooms(int);
E void rest_rooms(int);
E struct mkroom *search_special(schar);
E int cmap_to_type(int);
extern boolean is_rainbow_shop(struct mkroom *);

/* #### mksheol.c ### */

E void mksheol(void*);

/* ### mon.c ### */

extern boolean zombie_maker(struct monst *);
extern int zombie_form(struct permonst *);
E int select_newcham_form(struct monst *);
E void remove_monster(int, int);
E int m_poisongas_ok(struct monst *);
E int undead_to_corpse(int);
E int genus(int, int);
E int pm_to_cham(int);
E int minliquid(struct monst *);
E int movemon(void);
E int meatmetal(struct monst *);
E int meatobj(struct monst *);
E void mpickgold(struct monst *);
E boolean mpickstuff(struct monst *, const char *);
E void mpickup_obj(struct monst *, struct obj *);
E int curr_mon_load(struct monst *);
E int max_mon_load(struct monst *);
E int can_carry(struct monst *, struct obj *);
E int mfndpos(struct monst *, coord *, long *, long);
E boolean monnear(struct monst *, int, int);
E void dmonsfree(void);
E void elemental_clog(struct monst *);
E int mcalcmove(struct monst*);
E void mcalcdistress(void);
E void replmon(struct monst *, struct monst *);
E void relmon(struct monst *, struct monst **);
E struct obj *mlifesaver(struct monst *);
E boolean corpse_chance(struct monst *, struct monst *, boolean);
#ifdef WEBB_DISINT
E void mondead_helper(struct monst *, uchar);
#endif
E void mondead(struct monst *);
E void mondied(struct monst *);
E void mongone(struct monst *);
E void monstone(struct monst *);
E void monkilled(struct monst *, const char *, int);
E void unstuck(struct monst *);
E void killed(struct monst *);
E void xkilled(struct monst *, int);
E void mon_to_stone(struct monst*);
E void m_into_limbo(struct monst *);
E void mnexto(struct monst *);
E void maybe_mnexto(struct monst *);
E int mnearto(struct monst *, xchar, xchar, boolean);
E void poisontell(int);
E void poisoned(const char *, int, const char *, int);
E void m_respond(struct monst *);
E void setmangry(struct monst *, boolean);
E void wakeup(struct monst *, boolean);
E void wake_nearby(void);
E void wake_nearto(int, int, int);
E void seemimic(struct monst *);
E void rescham(void);
E void restartcham(void);
E void restore_cham(struct monst *);
E boolean hideunder(struct monst *);
E void mon_animal_list(boolean);
E boolean validvamp(struct monst *, int *, int);
E void mgender_from_permonst(struct monst *, struct permonst *);
E int newcham(struct monst *, struct permonst *, boolean, boolean);
E int can_be_hatched(int);
E int egg_type_from_parent(int, boolean);
E boolean dead_species(int, boolean);
E void kill_genocided_monsters(void);
E void kill_monster_on_level(int);
E void golemeffects(struct monst *, int, int);
E boolean angry_guards(boolean);
E void pacify_guards(void);
E void decide_to_shapeshift(struct monst *, int);

/* ### mondata.c ### */

E void set_mon_data(struct monst *, struct permonst *);
E struct attack *attacktype_fordmg(struct permonst *, int, int);
E boolean attacktype(struct permonst *, int);
E boolean noattacks(struct permonst *);
E boolean poly_when_stoned(struct permonst *);
E boolean resists_drli(struct monst *);
E boolean resists_magm(struct monst *);
E boolean resists_blnd(struct monst *);
E boolean can_blnd(struct monst *, struct monst *, uchar, struct obj *);
E boolean ranged_attk(struct permonst *);
E boolean hates_silver(struct permonst *);
E boolean mon_hates_silver(struct monst *);
E boolean mon_hates_light(struct monst *);
E boolean passes_bars(struct permonst *);
E boolean can_blow(struct monst *);
E boolean can_chant(struct monst *);
E boolean can_be_strangled(struct monst *);
E boolean can_track(struct permonst *);
E boolean breakarm(struct permonst *);
E boolean sliparm(struct permonst *);
E boolean sticks(struct permonst *);
E boolean cantvomit(struct permonst *);
E int num_horns(struct permonst *);
/* E boolean canseemon(struct monst *); */
E struct attack *dmgtype_fromattack(struct permonst *, int, int);
E boolean dmgtype(struct permonst *, int);
E int max_passive_dmg(struct monst *, struct monst *);
E boolean same_race(struct permonst *, struct permonst *);
E int monsndx(struct permonst *);
E int name_to_mon(const char *);
E int name_to_monclass(const char *, int *);
E int gender(struct monst *);
E int pronoun_gender(struct monst *, boolean);
E boolean levl_follower(struct monst *);
E int little_to_big(int);
E int big_to_little(int);
E const char *locomotion(const struct permonst *, const char *);
E const char *stagger(const struct permonst *, const char *);
E const char *on_fire(struct permonst *, struct attack *);
E const struct permonst *raceptr(struct monst *);
E boolean olfaction(struct permonst *);
extern boolean is_fleshy(const struct permonst *);

/* ### monmove.c ### */

E boolean itsstuck(struct monst *);
E boolean mb_trapped(struct monst *);
E boolean mon_has_key(struct monst *, boolean);
E void mon_regen(struct monst *, boolean);
E int dochugw(struct monst *);
E boolean onscary(int, int, struct monst *);
E void monflee(struct monst *, int, boolean, boolean);
E int dochug(struct monst *);
E boolean m_digweapon_check(struct monst *, xchar, xchar);
E int m_move(struct monst *, int);
extern int m_move_aggress(struct monst *, xchar, xchar);
E boolean closed_door(int, int);
E boolean accessible(int, int);
E void set_apparxy(struct monst *);
E boolean can_ooze(struct monst *);
E boolean can_fog(struct monst *);
E boolean should_displace(struct monst *, coord *, long *, int, xchar, xchar);
E boolean undesirable_disp(struct monst *, xchar, xchar);

/* ### monst.c ### */

E void monst_init(void);

/* ### monstr.c ### */

E void monstr_init(void);

/* ### mplayer.c ### */

E struct monst *mk_mplayer(struct permonst *, xchar, xchar, boolean);
E void create_mplayers(int, boolean);
E void mplayer_talk(struct monst *);
extern const char* dev_name(void);

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
E void gotoxy(int, int);
#  endif
# endif
# ifdef TOS
E int _copyfile(char *, char *);
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
E int (*nt_kbhit)();
E void Delay(int);
# endif /* WIN32 */
#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

E int thitu(int, int, struct obj **, const char *);
E int ohitmon(struct monst *, struct obj *, int, boolean);
E void thrwmu(struct monst *);
E int spitmu(struct monst *, struct attack *);
E int breamu(struct monst *, struct attack *);
E boolean linedup(xchar, xchar, xchar, xchar, int);
E boolean lined_up(struct monst *);
E struct obj *m_carrying(struct monst *, int);
E int thrwmm(struct monst *, struct monst *);
E int spitmm(struct monst *, struct attack *, struct monst *);
E int breamm(struct monst *, struct attack *, struct monst *);
E void m_useupall(struct monst *, struct obj *);
E void m_useup(struct monst *, struct obj *);
E void m_throw(struct monst *, int, int, int, int, int, struct obj *);
E boolean hits_bars(struct obj **, int, int, int, int, int, int);
E void hit_bars(struct obj **, int, int, int, int, boolean, boolean);
E void dissolve_bars(int, int);

/* ### muse.c ### */

E boolean find_defensive(struct monst *);
E int use_defensive(struct monst *);
E int rnd_defensive_item(struct monst *);
E boolean find_offensive(struct monst *);
#ifdef USE_TRAMPOLI
E int mbhitm(struct monst *, struct obj *);
#endif
E int use_offensive(struct monst *);
E int rnd_offensive_item(struct monst *);
E boolean find_misc(struct monst *);
E int use_misc(struct monst *);
E int rnd_misc_item(struct monst *);
E boolean searches_for_item(struct monst *, struct obj *);
E boolean mon_reflects(struct monst *, const char *);
E boolean ureflects(const char *, const char *);
E void mcureblindness(struct monst *, boolean);
E boolean munstone(struct monst *, boolean);
E boolean munslime(struct monst *, boolean);

/* ### music.c ### */

E void awaken_monsters(int);
E void do_earthquake(int);
E void awaken_soldiers(struct monst *);
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

/* ### nhregex.c ### */
E struct nhregex *regex_init(void);
E boolean regex_compile(const char *, struct nhregex *);
E const char *regex_error_desc(struct nhregex *);
E boolean regex_match(const char *, struct nhregex *);
E void regex_free(struct nhregex *);

/* ### nttty.c ### */

#ifdef WIN32CON
E void get_scr_size(void);
E int nttty_kbhit(void);
E void nttty_open(void);
E void nttty_rubout(void);
E int tgetch(void);
E int ntposkey(int *, int *, int *);
E void set_output_mode(int);
E void synch_cursor(void);
#endif

/* ### o_init.c ### */

E void init_objects(void);
E int find_skates(void);
E void oinit(void);
E void savenames(int, int);
E void restnames(int);
E void discover_object(int, boolean, boolean);
E void undiscover_object(int);
E int dodiscovered(void);
E void dragons_init(void);
E void makeknown_msg(int);
E int doclassdisco(void);
E void rename_disco(void);

/* ### objects.c ### */

E void objects_init(void);

/* ### objnam.c ### */

E char *obj_typename(int);
E char *simple_typename(int);
E char *dump_typename(int);
E char *safe_typename(int);
E boolean obj_is_pname(struct obj *);
E char *distant_name(struct obj *, char *(*)(OBJ_P));
E char *fruitname(boolean);
E struct fruit *fruit_from_indx(int);
E char *xname(struct obj *);
E char *mshot_xname(struct obj *);
E boolean the_unique_obj(struct obj *obj);
E boolean the_unique_pm(struct permonst *);
E boolean erosion_matters(struct obj *);
extern long display_weight(struct obj *);
E char *doname(struct obj *);
E char *doname_with_price(struct obj *);
E boolean not_fully_identified(struct obj *);
E char *corpse_xname(struct obj *, const char *, unsigned);
E char *cxname(struct obj *);
#ifdef SORTLOOT
E char *cxname_singular(struct obj *);
#endif
extern char *cxname_unidentified(struct obj *);
E char *killer_xname(struct obj *);
E char *short_oname(struct obj *, char *(*)(OBJ_P), char *(*)(OBJ_P), unsigned);
E const char *singular(struct obj *, char *(*)(OBJ_P));
E char *an(const char *);
E char *An(const char *);
E char *The(const char *);
E char *the(const char *);
E char *aobjnam(struct obj *, const char *);
E char *yobjnam(struct obj *, const char *);
E char *Yobjnam2(struct obj *, const char *);
E char *Tobjnam(struct obj *, const char *);
E char *otense(struct obj *, const char *);
E char *vtense(const char *, const char *);
E char *Doname2(struct obj *);
E char *yname(struct obj *);
E char *Yname2(struct obj *);
E char *ysimple_name(struct obj *);
E char *Ysimple_name2(struct obj *);
E char *simpleonames(struct obj *);
E char *ansimpleoname(struct obj *);
E char *thesimpleoname(struct obj *);
E char *bare_artifactname(struct obj *);
E char *makeplural(const char *);
E char *makesingular(const char *);
extern short name_to_otyp(const char *);
E struct obj *readobjnam(char *, struct obj *);
E int rnd_class(int, int);
E const char *suit_simple_name(struct obj *);
E const char *cloak_simple_name(struct obj *);
E const char *helm_simple_name(struct obj *);
E const char *gloves_simple_name(struct obj *);
E const char *mimic_obj_name(struct monst *);
E char *safe_qbuf(char *, const char *, const char *, struct obj *,
                  char *(*)(OBJ_P), char *(*)(OBJ_P), const char *);
E int shiny_obj(char);

/* ### options.c ### */

E boolean match_optname(const char *, const char *, int, boolean);
E void initoptions(void);
extern boolean parseoptions(char *, boolean, boolean);
E boolean parse_monster_color(char *);
E boolean parse_symbol(const char *);
E boolean parse_monster_symbol(const char *);
E boolean parse_object_symbol(const char *);
E boolean parse_color_definition(const char *);
E int doset(void);
E int dotogglepickup(void);
E void option_help(void);
E void next_opt(winid, const char *);
E int fruitadd(char *);
E int choose_classes_menu(const char *, int, boolean, char *, char *);
extern boolean parsebindings(char *);
E void add_menu_cmd_alias(char, char);
E char map_menu_cmd(char);
E void assign_warnings(uchar *);
E char *nh_getenv(const char *);
E void set_duplicate_opt_detection(int);
E void set_wc_option_mod_status(unsigned long, int);
E void set_wc2_option_mod_status(unsigned long, int);
E void set_option_mod_status(const char *, int);
#ifdef AUTOPICKUP_EXCEPTIONS
E int add_autopickup_exception(const char *);
E void free_autopickup_exceptions(void);
#endif /* AUTOPICKUP_EXCEPTIONS */
#ifdef MENU_COLOR
E boolean add_menu_coloring(char *);
#endif /* MENU_COLOR */
E void free_menu_coloring(void);
E const char * clr2colorname(int);
E int query_color(const char *);
E int query_attr(const char *);
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
extern boolean parse_status_color_options(char *);
extern void free_status_colors(void);
#endif

/* ### pager.c ### */

E void mhidden_description(struct monst *, boolean, char *);
E boolean object_from_map(int,int,int,struct obj **);
E int do_screen_description(coord, boolean, glyph_t, char *, const char **, struct permonst **);
extern int do_look(int, coord *);
E int dowhatis(void);
E int doquickwhatis(void);
E int doidtrap(void);
E int dowhatdoes(void);
E char *dowhatdoes_core(char, char *);
E int dohelp(void);
E int dohistory(void);
E void checkfile(struct obj *, char *, struct permonst *, boolean, boolean, char *);

/* ### pcmain.c ### */

#if defined(MICRO) || defined(WIN32)
# ifdef CHDIR
E void chdirx(char *, boolean);
# endif /* CHDIR */
#endif /* MICRO || WIN32 */

/* ### pcsys.c ### */

#if defined(MICRO) || defined(WIN32)
E void flushout(void);
E int dosh(void);
# ifdef MFLOPPY
E void eraseall(const char *, const char *);
E void copybones(int);
E void playwoRAMdisk(void);
E int saveDiskPrompt(int);
E void gameDiskPrompt(void);
# endif
E void append_slash(char *);
E void getreturn(const char *);
# ifndef AMIGA
E void VDECL(msmsg, (const char *, ...));
# endif
E FILE *fopenp(const char *, const char *);
#endif /* MICRO || WIN32 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
E void gettty(void);
E void settty(const char *);
E void setftty(void);
E void VDECL(error, (const char *, ...));
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

extern int get_most_recently_picked_up_turn(void);
extern int count_most_recently_picked_up(struct obj *);
extern struct obj *find_most_recently_picked_up_obj(struct obj *);
E void observe_quantum_cat(struct obj *, boolean);
E boolean container_gone(int (*)(OBJ_P));
E boolean u_handsy(void);
E void open_coffin(struct obj *, boolean);
extern int collect_obj_classes(char *, struct obj *, boolean, boolean(*)(struct obj *), int *);
E boolean rider_corpse_revival(struct obj *, boolean);
E boolean menu_class_present(int);
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
E int pickup_object(struct obj *, long, boolean);
extern int query_category(const char *, struct obj *, int, menu_item **, int);
extern int query_objlist(const char *, struct obj *, int, menu_item **, int, boolean (*)(OBJ_P));
E struct obj *pick_obj(struct obj *);
E int encumber_msg(void);
E int container_at(int, int, boolean);
E int doloot(void);
E int dotip(void);
E int use_container(struct obj **, int, boolean);
E int loot_mon(struct monst *, int *, boolean *);
E boolean is_autopickup_exception(struct obj *, boolean);
E boolean autopick_testobj(struct obj *, boolean);

/* ### pline.c ### */

E void msgpline_add(int, char *);
E void msgpline_free(void);
E void VDECL(pline, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(custompline, (unsigned, const char *, ...)) PRINTF_F(2, 3);
E void VDECL(Norep, (const char *, ...)) PRINTF_F(1, 2);
E void free_youbuf(void);
E void VDECL(You, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(Your, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(You_feel, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(You_cant, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(You_hear, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(You_see, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(pline_The, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(There, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(verbalize, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(raw_printf, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(impossible, (const char *, ...)) PRINTF_F(1, 2);
E void VDECL(warning, (const char *, ...)) PRINTF_F(1, 2);
E const char *align_str(aligntyp);
E void mstatusline(struct monst *);
E void ustatusline(void);
E void self_invis_message(void);

/* ### polyself.c ### */

E void init_uasmon(void);
E void set_uasmon(void);
E void float_vs_flight(void);
E void change_sex(void);
E void polyself(boolean);
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
E void uunstick(void);
E void skinback(boolean);
E const char *mbodypart(struct monst *, int);
E const char *body_part(int);
E int poly_gender(void);
E void ugolemeffects(int, int);
E boolean is_playermon_genocided(void);
E boolean ugenocided(void);
E const char *udeadinside(void);

/* ### potion.c ### */

E void set_itimeout(long *, long);
E void incr_itimeout(long *, int);
E void make_confused(long, boolean);
E void make_stunned(long, boolean);
E void make_blinded(long, boolean);
E void make_sick(long, const char *, boolean, int);
E void make_slimed(long, const char *);
E void make_stoned(long, const char *, int, const char *);
E void make_vomiting(long, boolean);
E void toggle_blindness(void);
E boolean make_hallucinated(long, boolean, long);
E void make_glib(int);
E int dodrink(void);
E int dopotion(struct obj *);
E int peffects(struct obj *);
E void healup(int, int, boolean, boolean);
E void strange_feeling(struct obj *, const char *);
E void potionhit(struct monst *, struct obj *, int);
E void potionbreathe(struct obj *);
E boolean get_wet(struct obj *);
E int dodip(void);
E void mongrantswish(struct monst **);
E void djinni_from_bottle(struct obj *);
E struct monst *split_mon(struct monst *, struct monst *);
E const char *bottlename(void);
extern boolean is_dark_mix_color(struct obj *);
extern boolean is_colorless_mix_potion(struct obj *);
extern const char* get_base_mix_color(struct obj *);
extern short mixtype(struct obj *, struct obj *);

/* ### pray.c ### */

#ifdef USE_TRAMPOLI
E int prayer_done(void);
#endif
E int dosacrifice(void);
E boolean can_pray(boolean);
E int dopray(void);
E const char *u_gname(void);
E int doturn(void);
E int altarmask_at(int, int);
E const char *a_gname(void);
E const char *a_gname_at(xchar x, xchar y);
E const char *align_gname(aligntyp);
E const char *halu_gname(aligntyp);
E const char *rnd_gname(int);
E const char *align_gtitle(aligntyp);
E void altar_wrath(int, int);
#ifdef ASTRAL_ESCAPE
E int invoke_amulet(struct obj *);
#endif
E int in_trouble(void);
extern void msg_luck_change(int change);


/* ### priest.c ### */

E int move_special(struct monst *, boolean, schar, boolean, boolean,
                   xchar, xchar, xchar, xchar);
E char temple_occupied(char *);
E boolean inhistemple(struct monst *);
E int pri_move(struct monst *);
E void priestini(d_level *, struct mkroom *, int, int, boolean);
E aligntyp mon_aligntyp(struct monst *);
E char *priestname(struct monst *, char *);
E boolean p_coaligned(struct monst *);
E struct monst *findpriest(char);
E void intemple(int);
E void forget_temple_entry(struct monst *);
E void priest_talk(struct monst *);
E struct monst *mk_roamer(struct permonst *, aligntyp,
                          xchar, xchar, boolean);
E void reset_hostility(struct monst *);
E boolean in_your_sanctuary(struct monst *, xchar, xchar);
E void ghod_hitsu(struct monst *);
E void angry_priest(void);
E void clearpriests(void);
E void restpriest(struct monst *, boolean);
E void newepri(struct monst *);
E void free_epri(struct monst *);
E char *piousness(boolean, const char *);

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
E void deliver_splev_message(void);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
E void srandom(unsigned);
E char *initstate(unsigned, char *, int);
E char *setstate(char *);
E long random(void);
#endif /* RANDOM */

/* ### read.c ### */

E void learn_scroll(struct obj *);
E char *tshirt_text(struct obj *, char *);
extern char *hawaiian_motif(struct obj *, char *);
extern char *apron_text(struct obj *, char *);
E int doread(void);
E boolean is_chargeable(struct obj *);
E void recharge(struct obj *, int);
E void forget_objects(int);
E void forget_levels(int);
E void forget_traps(void);
E void forget_map(int);
E int seffects(struct obj *);
#ifdef USE_TRAMPOLI
E void set_lit(int, int, genericptr_t);
#endif
E void litroom(boolean, struct obj *);
E void do_genocide(int, boolean);
E void punish(struct obj *);
E void unpunish(void);
E boolean cant_revive(int *, boolean, struct obj *);
#ifdef WIZARD
E boolean create_particular(void);
#endif
E void drop_boulder_on_player(boolean, boolean, boolean, boolean);
E int drop_boulder_on_monster(int, int, boolean, boolean);
extern boolean create_particular_from_buffer(const char*);

/* ### rect.c ### */

E void init_rect(void);
E NhRect *get_rect(NhRect *);
E NhRect *rnd_rect(void);
E void remove_rect(NhRect *);
E void add_rect(NhRect *);
E void split_rects(NhRect *, NhRect *);

/* ## region.c ### */
E void clear_regions(void);
E void run_regions(void);
E boolean in_out_region(xchar, xchar);
E boolean m_in_out_region(struct monst *, xchar, xchar);
E void update_player_regions(void);
E void update_monster_region(struct monst *);
E NhRegion *visible_region_at(xchar, xchar);
E void show_region(NhRegion*, xchar, xchar);
E void save_regions(int, int);
E void rest_regions(int, boolean);
E NhRegion* create_gas_cloud(xchar, xchar, int, size_t, int);
E NhRegion* create_cthulhu_death_cloud(xchar, xchar, int, size_t, int);
E boolean region_danger(void);
E void region_safety(void);

/* ### restore.c ### */

E void inven_inuse(boolean);
E int dorecover(int);
E void restcemetery(int, struct cemetery **);
E void trickery(char *);
E void getlev(int, int, xchar, boolean);
E void minit(void);
E boolean lookup_id_mapping(unsigned, unsigned *);
#ifdef ZEROCOMP
E int mread(int, genericptr_t, unsigned int);
#else
E void mread(int, genericptr_t, unsigned int);
#endif

/* ### rip.c ### */

E void genl_outrip(winid, int);

/* ### rnd.c ### */

E int rn2(int);
E int rnl(int);
E int rnd(int);
E int d(int, int);
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
extern int str2role(const char *);
extern int str2race(const char *);
extern int str2gend(const char *);
extern int str2align(const char *);
E boolean ok_role(int, int, int, int);
E int pick_role(int, int, int, int);
E boolean ok_race(int, int, int, int);
E int pick_race(int, int, int, int);
E boolean ok_gend(int, int, int, int);
E int pick_gend(int, int, int, int);
E boolean ok_align(int, int, int, int);
E int pick_align(int, int, int, int);
E void role_selection_prolog(int, winid);
E void role_menu_extra(int, winid, boolean);
E void role_init(void);
E void rigid_role_checks(void);
E boolean setrolefilter(const char *);
E boolean gotrolefilter(void);
E void clearrolefilter(void);
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

E char *getrumor(int, char *, boolean);
E void outrumor(int, int);
E void outoracle(boolean, boolean);
E void save_oracles(int, int);
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
E boolean savelev(int, xchar, int);
E boolean swapin_file(int);
E void co_false(void);
#else
E void savelev(int, xchar, int);
#endif
E void bufon(int);
E void bufoff(int);
E void bflush(int);
E void bwrite(int, genericptr_t, unsigned int);
E void bclose(int);
E void savecemetery(int, int, struct cemetery **);
E void savefruitchn(int, int);
E void free_dungeons(void);
E void freedynamicdata(void);

/* ### shk.c ### */

E void neweshk(struct monst *);
E void free_eshk(struct monst *);
E long money2mon(struct monst *, long);
E void money2u(struct monst *, long);
E char *Shknam(struct monst *);
E char *shkname(struct monst *);
E void shkgone(struct monst *);
E void set_residency(struct monst *, boolean);
E void replshk(struct monst *, struct monst *);
E void restshk(struct monst *, boolean);
E char inside_shop(xchar, xchar);
E void u_left_shop(char *, boolean);
E void remote_burglary(xchar, xchar);
E void u_entered_shop(char *);
E void pick_pick(struct obj *);
E boolean same_price(struct obj *, struct obj *);
E void shopper_financial_report(void);
E int inhishop(struct monst *);
E struct monst *shop_keeper(char);
E boolean tended_shop(struct mkroom *);
E boolean is_unpaid(struct obj *);
E void delete_contents(struct obj *);
E void obfree(struct obj *, struct obj *);
E void home_shk(struct monst *, boolean);
E void make_happy_shk(struct monst *, boolean);
E void make_happy_shoppers(boolean);
E void hot_pursuit(struct monst *);
E void make_angry_shk(struct monst *, xchar, xchar);
E int dopay(void);
E boolean paybill(int, boolean);
E void finish_paybill(void);
E struct obj *find_oid(unsigned);
E long contained_cost(struct obj *, struct monst *, long, boolean, boolean);
E long contained_gold(struct obj *);
E void picked_container(struct obj *);
E void alter_cost(struct obj *, long);
E long unpaid_cost(struct obj *, boolean);
E boolean billable(struct monst **, struct obj *, char, boolean);
E void addtobill(struct obj *, boolean, boolean, boolean);
E void append_honorific(char *);
E void splitbill(struct obj *, struct obj *);
E void subfrombill(struct obj *, struct monst *);
E long stolen_value(struct obj *, xchar, xchar, boolean, boolean);
E void sellobj_state(int);
E void sellobj(struct obj *, xchar, xchar);
E int doinvbill(int);
E struct monst *shkcatch(struct obj *, xchar, xchar);
E void add_damage(xchar, xchar, long);
E int repair_damage(struct monst *, struct damage *, boolean);
E int shk_move(struct monst *);
E void after_shk_move(struct monst *);
E boolean is_fshk(struct monst *);
E void shopdig(int);
E void pay_for_damage(const char *, boolean);
E boolean costly_spot(xchar, xchar);
E struct obj *shop_object(xchar, xchar);
E void price_quote(struct obj *);
E void shk_chat(struct monst *);
E void check_unpaid_usage(struct obj *, boolean);
E void check_unpaid(struct obj *);
E void costly_gold(xchar, xchar, long);
E long get_cost_of_shop_item(struct obj *);
E int oid_price_adjustment(struct obj *, unsigned);
E boolean block_door(xchar, xchar);
E boolean block_entry(xchar, xchar);
#ifdef BLACKMARKET
E void blkmar_guards(struct monst *);
E void set_black_marketeer_angry(void);
E void bars_around_portal(boolean);
#endif /* BLACKMARKET */
E char *shk_your(char *, struct obj *);
E char *Shk_Your(char *, struct obj *);
extern void sanity_check_shopkeepers(void);

/* ### shknam.c ### */

E void stock_room(int, struct mkroom *);
E boolean saleable(struct monst *, struct obj *);
E int get_shop_item(int);
E void shop_selection_init(void);
E boolean is_izchak(struct monst *, boolean);

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

E boolean check_room(xchar *, xchar *, xchar *, xchar *, boolean);
E boolean create_room(xchar, xchar, xchar, xchar,
                      xchar, xchar, xchar, xchar);
E void create_secret_door(struct mkroom *, xchar);
E boolean dig_corridor(coord *, coord *, boolean, schar, schar);
E void fill_room(struct mkroom *, boolean);
E boolean load_special(const char *);
E xchar selection_getpoint(int, int, struct opvar *);
E struct opvar *selection_opvar(char *);
E void opvar_free_x(struct opvar *);
E void set_selection_floodfillchk(int(*)(int,int));
E void selection_floodfill(struct opvar *, int, int, boolean);

/* ### spell.c ### */

E void book_cursed(struct obj *);
#ifdef USE_TRAMPOLI
E int learn(void);
#endif
E int study_book(struct obj *);
E void book_disappears(struct obj *);
E void book_substitution(struct obj *, struct obj *);
E void age_spells(void);
E int docast(void);
E int spell_skilltype(int);
E int spelleffects(int, boolean);
E int tport_spell(int);
E void losespells(void);
E int dovspell(void);
E void initialspell(struct obj *);
E void dump_spells(void);
E boolean parse_spellorder(char *);
extern const char* spelltypemnemonic(int);
extern int num_spells(void);

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
E int stealarm(void);
#endif
E long somegold(long);
E void stealgold(struct monst *);
E void remove_worn_item(struct obj *, boolean);
E int steal(struct monst *, char *);
E int mpickobj(struct monst *, struct obj *);
E void stealamulet(struct monst *);
E void maybe_absorb_item(struct monst *, struct obj *, int, int);
E void mdrop_obj(struct monst *, struct obj *, boolean);
E void mdrop_special_objs(struct monst *);
E void relobj(struct monst *, int, boolean);
E struct obj *findgold(struct obj *);

/* ### steed.c ### */

E void rider_cant_reach(void);
E boolean can_saddle(struct monst *);
E int use_saddle(struct obj *);
E void put_saddle_on_mon(struct obj *, struct monst *);
E boolean can_ride(struct monst *);
E int doride(void);
E boolean mount_steed(struct monst *, boolean);
E void exercise_steed(void);
E void kick_steed(void);
E void dismount_steed(int);
E void place_monster(struct monst *, int, int);
E boolean stucksteed(boolean);

/* ### sys.c ### */

extern void sys_early_init(void);

/* ### teleport.c ### */

E boolean goodpos(int, int, struct monst *, unsigned);
E boolean enexto(coord *, xchar, xchar, struct permonst *);
E boolean enexto_core(coord *, xchar, xchar, struct permonst *, unsigned);
E boolean enexto_core_range(coord *, xchar, xchar, struct permonst *, unsigned, int);
E int epathto(coord *, int, xchar, xchar, struct permonst *);
E void teleds(int, int, boolean);
E boolean safe_teleds(boolean);
E boolean teleport_pet(struct monst *, boolean);
E int dotelecmd(void);
E void tele(void);
E int dotele(boolean);
E void level_tele(void);
E void domagicportal(struct trap *);
E void tele_trap(struct trap *);
E void level_tele_trap(struct trap *, unsigned);
E void rloc_to(struct monst *, int, int);
E boolean rloc(struct monst *, boolean);
E boolean tele_restrict(struct monst *);
E void mtele_trap(struct monst *, struct trap *, int);
E int mlevel_tele_trap(struct monst *, struct trap *, boolean, int);
E boolean rloco(struct obj *);
E int random_teleport_level(void);
E boolean u_teleport_mon(struct monst *, boolean);

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
E void fall_asleep(int, boolean);
E void attach_egg_hatch_timeout(struct obj *, long);
E void attach_fig_transform_timeout(struct obj *);
E void kill_egg(struct obj *);
E void hatch_egg(ANY_P *, long);
E void learn_egg_type(int);
E void burn_object(ANY_P *, long);
E void begin_burn(struct obj *, boolean);
E void end_burn(struct obj *, boolean);
E void do_storms(void);
E boolean start_timer(long, short, short, ANY_P *);
E long stop_timer(short, ANY_P *);
E long peek_timer(short, ANY_P *);
E void run_timers(void);
E void obj_move_timers(struct obj *, struct obj *);
E void obj_split_timers(struct obj *, struct obj *);
E void obj_stop_timers(struct obj *);
E boolean obj_has_timer(struct obj *, short);
E void spot_stop_timers(xchar, xchar, short);
E long spot_time_expires(xchar, xchar, short);
E long spot_time_left(xchar, xchar, short);
E boolean obj_is_local(struct obj *);
E void save_timers(int, int, int);
E void restore_timers(int, int, boolean, long);
E void timer_stats(const char *, char *, long *, long *);
E void relink_timers(boolean);
#ifdef WIZARD
E int wiz_timeout_queue(void);
E void timer_sanity_check(void);
#endif

/* ### topten.c ### */

#ifdef RECORD_CONDUCT
E long encodeconduct(void);
#endif
E void topten(int);
E void prscore(int, char **);
E struct obj *tt_oname(struct obj *);

/* ### track.c ### */

E void initrack(void);
E void settrack(void);
E coord *gettrack(int, int);

/* ### trap.c ### */

E boolean burnarmor(struct monst *);
E int erode_obj(struct obj *, const char *, int, int);
E boolean grease_protect(struct obj *, const char *, struct monst *);
E struct trap *maketrap(int, int, int);
E void fall_through(boolean, unsigned);
E struct monst *animate_statue(struct obj *, xchar, xchar, int, int *);
E struct monst *activate_statue_trap(struct trap *, xchar, xchar, boolean);
E void set_utrap(unsigned, unsigned);
E void reset_utrap(boolean);
E void dotrap(struct trap *, unsigned);
E void seetrap(struct trap *);
E void feeltrap(struct trap *);
extern int mintrap(struct monst *, unsigned);
E void instapetrify(const char *);
E void minstapetrify(struct monst *, boolean);
#ifdef WEBB_DISINT
E int instadisintegrate(const char *);
E int minstadisintegrate(struct monst *);
#endif
E void selftouch(const char *);
E void mselftouch(struct monst *, const char *, boolean);
E void float_up(void);
E void fill_pit(int, int);
E int float_down(long, long);
E void climb_pit(void);
E boolean fire_damage(struct obj *, boolean, xchar, xchar);
E int fire_damage_chain(struct obj *, boolean, boolean, xchar, xchar);
E boolean lava_damage(struct obj *, xchar, xchar);
E void acid_damage(struct obj *);
E int water_damage(struct obj *, const char *, boolean);
E void water_damage_chain(struct obj *, boolean);
E boolean drown(void);
E void drain_en(int);
E int dountrap(void);
E void cnv_trap_obj(int, int, struct trap *, boolean);
E int untrap(boolean);
E boolean openholdingtrap(struct monst *, boolean *);
E boolean closeholdingtrap(struct monst *, boolean *);
E boolean openfallingtrap(struct monst *, boolean, boolean *);
E boolean chest_trap(struct obj *, int, boolean);
E void deltrap(struct trap *);
E boolean delfloortrap(struct trap *);
E struct trap *t_at(int, int);
E void b_trapped(const char *, int);
E boolean unconscious(void);
E boolean uteetering_at_seen_pit(struct trap *);
E boolean uescaped_shaft(struct trap *);
E boolean lava_effects(void);
E boolean swamp_effects(void);
E void blow_up_landmine(struct trap *);
E int launch_obj(short, int, int, int, int, int);

/* ### u_init.c ### */

E void u_init(void);

/* ### uhitm.c ### */

E void hurtmarmor(struct monst *, int);
E boolean attack_checks(struct monst *, struct obj *);
E void check_caitiff(struct monst *);
E int find_roll_to_hit(struct monst *, uchar, struct obj *, int *, int *);
E boolean attack(struct monst *);
E boolean hmon(struct monst *, struct obj *, int, int);
E boolean shade_miss(struct monst *, struct monst *, struct obj *, boolean, boolean);
E int damageum(struct monst *, struct attack *, int);
E void missum(struct monst *, struct attack *, boolean);
E int passive(struct monst *, struct obj *, boolean, int, uchar, boolean);
E void passive_obj(struct monst *, struct obj *, struct attack *);
E void stumble_onto_mimic(struct monst *);
E int flash_hits_mon(struct monst *, struct obj *);
E void light_hits_gremlin(struct monst *, int);

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
E void VDECL(error, (const char *, ...)) PRINTF_F(1, 2);
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
E FILE *fopen_datafile_area(const char *, const char *, const char *, boolean);
E FILE *freopen_area(const char *, const char *, const char *, FILE *);
E int chmod_area(const char *, const char *, int);
E int open_area(const char *, const char *, int, int);
E int creat_area(const char *, const char *, int);
E boolean lock_file_area(const char *, const char *, int);
E void unlock_file_area(const char *, const char *);
E int rename_area(const char *, const char *, const char *);
E int remove_area(const char *, const char *);
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS
E int hide_privileges(boolean);
# endif
#endif /* UNIX */

/* ### vault.c ### */

E void newegd(struct monst *);
E void free_egd(struct monst *);
E boolean grddead(struct monst *);
E struct monst *findgd(void);
E void vault_summon_gd(void);
E char vault_occupied(char *);
E void uleftvault(struct monst *);
E void invault(void);
E int gd_move(struct monst *);
E void paygd(boolean);
E long hidden_gold(void);
E boolean gd_sound(void);
E void vault_gd_watching(unsigned int);

/* ### version.c ### */

E char *version_string(char *);
E char *getversionstring(char *);
E int doversion(void);
E int doextversion(void);
#ifdef MICRO
E boolean comp_times(long);
#endif
E boolean check_version(struct version_info *, const char *, boolean);
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
E void tileview(boolean);
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
E int does_block(int, int, struct rm*);
E void vision_reset(void);
E void vision_recalc(int);
E void block_point(int, int);
E void unblock_point(int, int);
E boolean clear_path(int, int, int, int);
E void do_clear_area(int, int, int, void (*)(int, int, genericptr_t), genericptr_t);
E unsigned how_mon_is_seen(struct monst *);

#ifdef VMS

/* ### vmsfiles.c ### */

E int vms_link(const char *, const char *);
E int vms_unlink(const char *);
E int vms_creat(const char *, unsigned int);
E int vms_open(const char *, int, unsigned int);
E boolean same_dir(const char *, const char *);
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
E void chdirx(const char *, boolean);
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
E void VDECL(error, (const char *, ...)) PRINTF_F(1, 2);
#ifdef TIMED_DELAY
E void msleep(unsigned);
#endif

/* ### vmsunix.c ### */

E void getlock(void);
E void regularize(char *);
E int vms_getuid(void);
E boolean file_is_stmlf(int);
E int vms_define(const char *, const char *, int);
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
E int vms_doshell(const char *, boolean);
# endif
# ifdef SUSPEND
E int dosuspend(void);
# endif

#endif /* VMS */

/* ### weapon.c ### */

E const char *weapon_descr(struct obj *);
E int hitval(struct obj *, struct monst *);
E int dmgval(struct obj *, struct monst *);
E int special_dmgval(struct monst *, struct monst *, long, long *);
extern struct damage_info_t dmgval_info(struct obj*);
E void silver_sears(struct monst *, struct monst *, long);
E struct obj *select_rwep(struct monst *);
E boolean mon_might_throw_wep(struct obj *);
E struct obj *select_hwep(struct monst *);
E void possibly_unwield(struct monst *, boolean);
E int mon_wield_item(struct monst *);
E void mwepgone(struct monst *);
E int abon(void);
E int dbon(void);
E void wet_a_towel(struct obj *, int, boolean);
E void dry_a_towel(struct obj *, int, boolean);
extern const char* skill_name(int);
E int enhance_weapon_skill(void);
#ifdef DUMP_LOG
E void dump_weapon_skill(void);
#endif
E void unrestrict_weapon_skill(int);
E void use_skill(int, int);
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
E int counter_were(int);
E int were_beastie(int);
E void new_were(struct monst *);
E int were_summon(struct permonst *, boolean, int *, char *);
E void you_were(void);
E void you_unwere(boolean);
E void set_ulycn(int);

/* ### wield.c ### */

E void setuwep(struct obj *);
E void setuqwep(struct obj *);
E void setuswapwep(struct obj *);
E int dowield(void);
E int doswapweapon(void);
E int dowieldquiver(void);
E int dounwield(void);
E boolean wield_tool(struct obj *, const char *);
E int can_twoweapon(void);
E void drop_uswapwep(void);
E int dotwoweapon(void);
E void uwepgone(void);
E void uswapwepgone(void);
E void uqwepgone(void);
E void untwoweapon(void);
E int chwepon(struct obj *, int);
E int welded(struct obj *);
E void weldmsg(struct obj *);
E void setmnotwielded(struct monst *, struct obj *);
E boolean mwelded(struct obj *);
E void unwield_weapons_silently(void);

/* ### windows.c ### */

E void choose_windows(const char *);
E char genl_message_menu(char, int, const char *);
E void genl_preference_update(const char *);

/* ### wizard.c ### */

E void amulet(void);
E int mon_has_amulet(struct monst *);
E int mon_has_special(struct monst *);
E int tactics(struct monst *);
E boolean has_aggravatables(struct monst *);
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
E void initworm(struct monst *, int);
E void worm_move(struct monst *);
E void worm_nomove(struct monst *);
E void wormgone(struct monst *);
E void wormhitu(struct monst *);
E void cutworm(struct monst *, xchar, xchar, struct obj *);
E void see_wsegs(struct monst *);
E void detect_wsegs(struct monst *, boolean);
E void save_worm(int, int);
E void rest_worm(int);
E void place_wsegs(struct monst *, struct monst *);
E void sanity_check_worm(struct monst *);
E void remove_worm(struct monst *);
E void place_worm_tail_randomly(struct monst *, xchar, xchar);
E int size_wseg(struct monst *);
E int count_wsegs(struct monst *);
E boolean worm_known(struct monst *);
E int wseg_at(struct monst *, int, int);
extern void flip_worm_segs_vertical(struct monst *, int, int);
extern void flip_worm_segs_horizontal(struct monst *, int, int);

/* ### worn.c ### */

E void setworn(struct obj *, long);
E void setnotworn(struct obj *);
E struct obj *wearmask_to_obj(long);
E long wearslot(struct obj *);
E void mon_set_minvis(struct monst *);
E void mon_adjust_speed(struct monst *, int, struct obj *);
E void update_mon_intrinsics(struct monst *, struct obj *, boolean, boolean);
E int find_mac(struct monst *);
E void m_dowear(struct monst *, boolean);
E struct obj *which_armor(struct monst *, long);
E void mon_break_armor(struct monst *, boolean);
E void bypass_obj(struct obj *);
E void clear_bypasses(void);
E void bypass_objlist(struct obj *, boolean);
E struct obj *nxt_unbypassed_obj(struct obj *);
E int racial_exception(struct monst *, struct obj *);

/* ### write.c ### */

extern int ink_cost(struct obj *);
E int dowrite(struct obj *);

/* ### zap.c ### */

E void learn_wand(struct obj *);
E int bhitm(struct monst *, struct obj *);
E void probe_monster(struct monst *);
E boolean get_obj_location(struct obj *, xchar *, xchar *, int);
E boolean get_mon_location(struct monst *, xchar *, xchar *, int);
E struct monst *get_container_location(struct obj *obj, int *, int *);
E struct monst *montraits(struct obj *, coord *, boolean);
E struct monst *revive(struct obj *, boolean);
E int unturn_dead(struct monst *);
E void cancel_item(struct obj *);
E boolean drain_item(struct obj *, boolean);
E struct obj *poly_obj(struct obj *, int);
E boolean obj_resists(struct obj *, int, int);
E boolean obj_shudders(struct obj *);
E void do_osshock(struct obj *);
E int bhito(struct obj *, struct obj *);
E int bhitpile(struct obj *, int (*)(OBJ_P, OBJ_P), int, int, schar);
E int zappable(struct obj *);
E void zapnodir(struct obj *);
E int dozap(void);
E int zapyourself(struct obj *, boolean);
E void ubreatheu(struct attack *);
E int lightdamage(struct obj *, boolean, int);
E boolean flashburn(long);
E boolean cancel_monst(struct monst *, struct obj *, boolean, boolean, boolean);
E void zapsetup(void);
E void zapwrapup(void);
E void weffects(struct obj *);
E int spell_damage_bonus(int);
E const char *exclam(int force);
E void hit(const char *, struct monst *, const char *);
E void miss(const char *, struct monst *);
E struct monst *bhit(int, int, int, int, int (*)(MONST_P, OBJ_P),
                     int (*)(OBJ_P, OBJ_P), struct obj *, boolean *);
E struct monst *boomhit(struct obj *, int, int);
E int zhitm(struct monst *, int, int, struct obj **);
E int burn_floor_objects(int, int, boolean, boolean);
E void buzz(int, int, xchar, xchar, int, int);
E void dobuzz(int, int, xchar, xchar, int, int, boolean);
E void melt_ice(xchar, xchar, const char *);
E void melt_ice_away(ANY_P *, long);
E int zap_over_floor(xchar, xchar, int, boolean *, short);
E void fracture_rock(struct obj *);
E boolean break_statue(struct obj *);
E void destroy_item(int, int);
E int destroy_mitem(struct monst *, int, int);
E int resist(struct monst *, char, int, int);
E void makewish(boolean);
E void remove_corpse(struct obj *);

/* ### livelog.c ### */
#ifdef LIVELOGFILE
E boolean livelog_start(void);
E void livelog_achieve_update(void);
E void livelog_wish(char*);
# ifdef LIVELOG_SHOUT
E int doshout(void);
# endif
# ifdef LIVELOG_BONES_KILLER
E void livelog_bones_killed(struct monst *);
# endif
E void livelog_shoplifting(const char*, const char*, long);
#endif
E void livelog_game_started(const char*, const char*, const char*, const char*);
E void livelog_game_action(const char*);
E void livelog_generic(const char*, const char*);
E void livelog_genocide(const char*, int);
extern void livelog_printf(unsigned int, const char *, ...) PRINTF_F(2, 3);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

/* ### tutorial.c ### */
E boolean check_tutorial_message(int);
E void maybe_tutorial(void);
E int tutorial_redisplay(void);
E void tutorial_redisplay_message(void);
E void check_tutorial_farlook(int, int);
E void check_tutorial_command(char);
E int check_tutorial_location(int, int, boolean);
E int check_tutorial_oclass(int);

/* ### base32.c ### */
long decode_base32(const char*);
const char* encode_base32(unsigned long);

/* ### unicode.c ### */
E glyph_t get_unicode_codepoint(int);
E int pututf8char(glyph_t);

#undef E

#endif /* EXTERN_H */
