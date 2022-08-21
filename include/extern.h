/* Copyright (c) Steve Creps, 1988.               */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#define E extern

/* ### alloc.c ### */

#if 0
extern long *alloc(unsigned int);
#endif
extern char *fmt_ptr(const genericptr);

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

extern void early_init(void);
extern void moveloop(boolean);
extern void stop_occupation(void);
extern void display_gamewindows(void);
extern void newgame(void);
extern void welcome(boolean);
extern boolean can_regenerate(void);
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
extern time_t get_realtime(void);
#endif

/* ### apply.c ### */

extern int doapply(void);
extern int dorub(void);
extern int dojump(void);
extern int jump(int);
extern int number_leashed(void);
extern void o_unleash(struct obj *);
extern void m_unleash(struct monst *, boolean);
extern void unleash_all(void);
extern boolean leashable(struct monst *);
extern boolean next_to_u(void);
extern struct obj *get_mleash(struct monst *);
extern void check_leash(coordxy, coordxy);
extern boolean um_dist(coordxy, coordxy, xint16);
extern boolean snuff_candle(struct obj *);
extern boolean snuff_lit(struct obj *);
extern boolean catch_lit(struct obj *);
extern void fix_attributes_and_properties(struct obj *, int);
extern boolean tinnable(struct obj *);
extern void reset_trapset(void);
extern void fig_transform(ANY_P *, long);
extern int unfixable_trouble_count(boolean);

/* ### artifact.c ### */

extern void init_artifacts(void);
extern void save_artifacts(int);
extern void restore_artifacts(int);
extern const char *artiname(int);
extern struct obj *mk_artifact(struct obj *, aligntyp);
extern const char *artifact_name(const char *, short *);
extern boolean exist_artifact(int, const char *);
extern void artifact_exists(struct obj *, const char *, boolean);
extern int nartifact_exist(void);
extern boolean spec_ability(struct obj *, unsigned long);
extern boolean confers_luck(struct obj *);
extern boolean arti_reflects(struct obj *);
extern boolean shade_glare(struct obj *);
extern boolean restrict_name(struct obj *, const char *, boolean);
extern boolean defends(int, struct obj *);
extern boolean defends_when_carried(int, struct obj *);
extern boolean protects(struct obj *, boolean);
extern void set_artifact_intrinsic(struct obj *, boolean, long);
extern int touch_artifact(struct obj *, struct monst *);
extern int spec_abon(struct obj *, struct monst *);
extern int spec_dbon(struct obj *, struct monst *, int);
extern void discover_artifact(xint16);
extern boolean undiscovered_artifact(xint16);
extern int disp_artifact_discoveries(winid);
extern boolean artifact_hit(struct monst *, struct monst *, struct obj *, int *, int);
extern int doinvoke(void);
extern void arti_speak(struct obj *);
extern boolean artifact_light(struct obj *);
extern long spec_m2(struct obj *);
extern boolean artifact_has_invprop(struct obj *, uchar);
extern long arti_cost(struct obj *);
extern boolean MATCH_WARN_OF_MON(struct monst *);
extern struct obj *what_gives(long *);
extern const char *get_warned_of_monster(struct obj *);
extern const char *glow_color(int);
extern const char *glow_verb(int, boolean);
extern void Sting_effects(int);
extern int retouch_object(struct obj **, boolean);
extern void retouch_equipment(int);
extern void mkot_trap_warn(void);
extern boolean is_magic_key(struct monst *, struct obj *);
extern struct obj *has_magic_key(struct monst *);

/* ### attrib.c ### */

extern boolean adjattrib(int, int, int);
extern void change_luck(schar);
extern int stone_luck(boolean);
extern boolean has_luckitem(void);
extern void set_moreluck(void);
extern void gainstr(struct obj *, int);
extern void losestr(int);
extern void restore_attrib(void);
extern void exercise(int, boolean);
extern void exerchk(void);
extern void reset_attribute_clock(void);
extern void init_attr(int);
extern void redist_attr(void);
extern void adjabil(int, int);
extern int newhp(void);
extern schar acurr(int);
extern schar acurrstr(void);
extern void adjalign(int);
extern const char * beautiful(void);
extern int uhp(void);
extern int uhpmax(void);
extern boolean extremeattr(int);
extern void uchangealign(int, int);

/* ### ball.c ### */

extern void ballrelease(boolean);
extern void ballfall(void);
extern void placebc(void);
extern void unplacebc(void);
extern void set_bc(int);
extern void move_bc(int, int, coordxy, coordxy, coordxy, coordxy);
extern boolean drag_ball(coordxy, coordxy, int *, coordxy *, coordxy *, coordxy *, coordxy *, boolean *, boolean);
extern void drop_ball(coordxy, coordxy);
extern void drag_down(void);
extern void bc_sanity_check(void);

/* ### bones.c ### */

extern void sanitize_name(char *);
extern void drop_upon_death(struct monst *, struct obj *, coordxy, coordxy);
extern boolean can_make_bones(void);
extern void savebones(struct obj *);
extern int getbones(void);

/* ### botl.c ### */

extern int xlev_to_rank(int);
extern int title_to_mon(const char *, int *, int *);
extern void max_rank_sz(void);
#ifdef SCORE_ON_BOTL
extern long botl_score(void);
#endif
extern int describe_level(char *);
extern const char *rank(void);
extern const char *rank_of(int, short, boolean);
extern const char* botl_realtime(void);
extern void bot(void);
#ifdef DUMP_LOG
extern void bot1str(char *);
extern void bot2str(char *);
extern void bot3str(char *);
#endif

/* ### cmd.c ### */

extern char randomkey(void);
extern boolean redraw_cmd(char);
extern char cmd_from_func(int(*)(void));
#ifdef USE_TRAMPOLI
extern int doextcmd(void);
extern int domonability(void);
extern int doprev_message(void);
extern int timed_occupation(void);
extern int wiz_attributes(void);
extern int enter_explore_mode(void);
# ifdef WIZARD
extern int wiz_detect(void);
extern int wiz_genesis(void);
extern int wiz_identify(void);
extern int wiz_level_tele(void);
extern int wiz_map(void);
extern int wiz_where(void);
extern int wiz_wish(void);
# endif /* WIZARD */
#endif /* USE_TRAMPOLI */
extern void reset_occupations(void);
extern void set_occupation(int (*)(void), const char *, int);
#ifdef REDO
extern char pgetchar(void);
extern void pushch(char);
extern void savech(char);
#endif
#ifdef WIZARD
extern void add_debug_extended_commands(void);
#endif /* WIZARD */
extern boolean bind_specialkey(uchar, const char *);
extern char txt2key(char *);
extern void parseautocomplete(char *, boolean);
extern void reset_commands(boolean);
extern void rhack(char *);
extern int doextlist(void);
extern int extcmd_via_menu(void);
extern boolean bind_key(uchar, const char *);
extern void enlightenment(int, boolean);
extern void show_conduct(int, boolean);
extern int xytod(schar, schar);
extern void dtoxy(coord *, int);
extern int movecmd(char);
extern int dxdy_moveok(void);
extern int getdir(const char *);
extern void confdir(void);
extern const char *directionname(int);
extern int isok(coordxy, coordxy);
extern int get_adjacent_loc(const char *, const char *, coordxy, coordxy, coord *);
extern const char *click_to_cmd(coordxy, coordxy, int);
extern char readchar(void);
extern int do_stair_travel(char);
#ifdef WIZARD
extern void sanity_check(void);
#endif
extern char* key2txt(uchar, char *);
extern char yn_function(const char *, const char *, char);
extern char paranoid_yn(const char *, boolean);
extern void sokoban_trickster(void);
extern void list_vanquished(char, boolean);
extern void list_vanquishedonly(void);
#ifdef TTY_GRAPHICS
extern int tty_debug_show_colors(void);
#endif
#ifdef CURSES_GRAPHICS
extern int curses_debug_show_colors(void);
#endif

/* ### dbridge.c ### */

extern boolean is_pool(coordxy, coordxy);
extern boolean is_lava(coordxy, coordxy);
extern boolean is_pool_or_lava(coordxy, coordxy);
extern boolean is_ice(coordxy, coordxy);
extern boolean is_moat(coordxy, coordxy);
extern boolean is_swamp(coordxy, coordxy);
extern boolean is_icewall(coordxy, coordxy);
extern boolean is_any_icewall(coordxy, coordxy);
extern schar db_under_typ(struct rm *lev);
extern int is_drawbridge_wall(coordxy, coordxy);
extern boolean is_db_wall(coordxy, coordxy);
extern boolean find_drawbridge(coordxy *, coordxy *);
extern boolean create_drawbridge(coordxy, coordxy, int, int);
extern void open_drawbridge(coordxy, coordxy);
extern boolean close_drawbridge(coordxy, coordxy);
extern void destroy_drawbridge(coordxy, coordxy);

/* ### decl.c ### */

extern void decl_init(void);

/* ### detect.c ### */

extern boolean trapped_chest_at(int, coordxy, coordxy);
extern boolean trapped_door_at(int, coordxy, coordxy);
extern struct obj *o_in(struct obj*, char);
extern struct obj *o_material(struct obj*, unsigned);
extern int gold_detect(struct obj *);
extern int food_detect(struct obj *);
extern int object_detect(struct obj *, int, boolean);
extern int monster_detect(struct obj *, int);
extern int trap_detect(struct obj *);
extern const char *level_distance(d_level *);
extern void use_crystal_ball(struct obj **);
extern void do_mapping(void);
extern void do_vicinity_map(void);
extern void cvt_sdoor_to_door(struct rm *);
#ifdef USE_TRAMPOLI
extern void findone(coordxy, coordxy, genericptr_t);
extern void openone(coordxy, coordxy, genericptr_t);
#endif
extern int findit(void);
extern int openit(void);
extern boolean detecting(void(*)(coordxy, coordxy, void *));
extern void find_trap(struct trap *);
extern int dosearch0(int);
extern int dosearch(void);
extern void sokoban_detect(void);
extern void reveal_terrain(int, int);

/* ### dig.c ### */

extern int dig_typ(struct obj *, coordxy, coordxy);
extern boolean is_digging(void);
#ifdef USE_TRAMPOLI
extern int dig(void);
#endif
extern int holetime(void);
extern boolean dig_check(struct monst *, boolean, coordxy, coordxy);
extern void digactualhole(coordxy, coordxy, struct monst *, int);
extern boolean dighole(boolean, boolean, coord *);
extern int use_pick_axe(struct obj *);
extern int use_pick_axe2(struct obj *);
extern boolean mdig_tunnel(struct monst *);
extern void draft_message(boolean);
extern void watch_dig(struct monst *, coordxy, coordxy, boolean);
extern void zap_dig(void);
extern struct obj *bury_an_obj(struct obj *, boolean *);
extern void bury_objs(coordxy, coordxy);
extern void unearth_objs(coordxy, coordxy);
extern void rot_organic(ANY_P *, long);
extern void rot_corpse(ANY_P *, long);
extern struct obj *buried_ball(coord *);
extern void buried_ball_to_punishment(void);
extern void buried_ball_to_freedom(void);
extern schar fillholetyp(int, int, boolean);
extern void liquid_flow(coordxy, coordxy, schar, struct trap *, const char *);
extern boolean conjoined_pits(struct trap *, struct trap *, boolean);
#if 0
extern void bury_monst(struct monst *);
extern void bury_you(void);
extern void unearth_you(void);
extern void escape_tomb(void);
extern void bury_obj(struct obj *);
#endif

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
extern struct obj * vobj_at(coordxy, coordxy);
#endif /* INVISIBLE_OBJECTS */
extern int is_safemon(struct monst *);
extern void magic_map_background(coordxy, coordxy, int);
extern void map_background(coordxy, coordxy, int);
extern void map_trap(struct trap *, int);
extern void map_object(struct obj *, int);
extern void map_invisible(coordxy, coordxy);
extern boolean unmap_invisible(coordxy, coordxy);
extern void unmap_object(coordxy, coordxy);
extern struct obj *vobj_at(coordxy, coordxy);
extern void map_location(coordxy, coordxy, int);
extern void feel_newsym(coordxy, coordxy);
extern void feel_location(coordxy, coordxy);
extern void newsym(coordxy, coordxy);
extern void newsym_force(coordxy, coordxy);
extern void shieldeff(coordxy, coordxy);
extern void tmp_at(coordxy, coordxy);
extern void flash_glyph_at(coordxy, coordxy, int, int);
extern void swallowed(int);
extern void under_ground(int);
extern void under_water(int);
extern void see_monsters(void);
extern void set_mimic_blocking(void);
extern void see_objects(void);
extern void see_traps(void);
extern void curs_on_u(void);
extern int doredraw(void);
extern void docrt(void);
extern void show_glyph(coordxy, coordxy, int);
extern void clear_glyph_buffer(void);
extern void row_refresh(int, int, int);
extern void cls(void);
extern void flush_screen(int);
#ifdef DUMP_LOG
extern void dump_screen(void);
#endif
extern int back_to_glyph(coordxy, coordxy);
extern int zapdir_to_glyph(int, int, int);
extern int glyph_at(coordxy, coordxy);
extern void set_wall_state(void);
extern void unset_seenv(struct rm *, int, int, int, int);
extern int warning_of(struct monst *);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
extern int drop(struct obj *);
extern int wipeoff(void);
#endif
extern int dodrop(void);
extern boolean boulder_hits_pool(struct obj *, int, int, boolean);
extern boolean flooreffects(struct obj *, coordxy, coordxy, const char *);
extern void doaltarobj(struct obj *);
extern boolean canletgo(struct obj *, const char *);
extern void dropx(struct obj *);
extern void dropy(struct obj *);
extern void dropz(struct obj *, boolean);
extern void obj_no_longer_held(struct obj *);
extern int doddrop(void);
extern int dodown(void);
extern int doup(void);
#ifdef INSURANCE
extern void save_currentstate(void);
#endif
extern void goto_level(d_level *, boolean, boolean, boolean);
extern void schedule_goto(d_level *, boolean, boolean, int, const char *, const char *);
extern void deferred_goto(void);
extern boolean revive_corpse(struct obj *);
extern void revive_mon(ANY_P *, long);
extern void zombify_mon(union any *, long);
extern int donull(void);
extern int dowipe(void);
extern void set_wounded_legs(long, int);
extern void heal_legs(int);

/* ### do_name.c ### */

extern void do_oname(struct obj *);
extern char *coord_desc(coordxy, coordxy, char *, char);
extern boolean getpos_menu(coord *, int);
extern int getpos(coord *, boolean, const char *);
extern void getpos_sethilite(void (*f)(int), boolean (*d)(coordxy, coordxy));
extern void new_mgivenname(struct monst *, int);
extern void free_mgivenname(struct monst *);
extern void new_oname(struct obj *, int);
extern void free_oname(struct obj *);
extern const char *safe_oname(struct obj *);
extern struct monst *christen_monst(struct monst *, const char *);
extern int do_mname(void);
extern struct obj *oname(struct obj *, const char *);
extern boolean objtyp_is_callable(int);
extern int ddocall(void);
extern void docall(struct obj *);
extern void docall_input(int);
void namefloorobj(void);
extern const char *rndghostname(void);
extern char *x_monnam(struct monst *, int, const char *, int, boolean);
extern char *l_monnam(struct monst *);
extern char *mon_nam(struct monst *);
extern char *noit_mon_nam(struct monst *);
extern char *Monnam(struct monst *);
extern char *noit_Monnam(struct monst *);
extern char *noname_monnam(struct monst *, int);
extern char *m_monnam(struct monst *);
extern char *y_monnam(struct monst *);
extern char *Adjmonnam(struct monst *, const char *);
extern char *Amonnam(struct monst *);
extern char *a_monnam(struct monst *);
extern char *distant_monnam(struct monst *, int, char *);
extern char *mon_nam_too(struct monst *, struct monst *);
extern char *minimal_monnam(struct monst *, boolean);
extern const char *rndmonnam(void);
extern const char *hcolor(const char *);
extern const char *rndcolor(void);
extern const char *hliquid(const char *);
#ifdef REINCARNATION
extern const char *roguename(void);
#endif
extern struct obj *realloc_obj(struct obj *, int, genericptr_t, int, const char *);
extern char *coyotename(struct monst *, char *);
extern char *rndorcname(char *);
extern struct monst *christen_orc(struct monst *, const char *, const char *);
extern const char *noveltitle(int *);
extern const char *lookup_novel(const char *, int *);
extern char *mon_wounds(struct monst *);
extern void print_mon_wounded(struct monst *, int);

/* ### do_wear.c ### */

extern int lucky_fedora(void);
#ifdef USE_TRAMPOLI
extern int Armor_on(void);
extern int Boots_on(void);
extern int Gloves_on(void);
extern int Helmet_on(void);
extern int select_off(struct obj *);
extern int take_off(void);
#endif
extern const char *fingers_or_gloves(boolean);
extern void off_msg(struct obj *);
extern void set_wear(void);
extern boolean donning(struct obj *);
extern boolean doffing(struct obj *);
extern void cancel_doff(struct obj *, long);
extern void cancel_don(void);
extern int stop_donning(struct obj *);
extern int Armor_off(void);
extern int Armor_gone(void);
extern int Helmet_off(void);
extern int Gloves_off(void);
extern int Boots_on(void);
extern int Boots_off(void);
extern int Cloak_off(void);
extern int Shield_off(void);
extern int Shirt_off(void);
extern void Amulet_off(void);
extern void Ring_on(struct obj *);
extern void Ring_off(struct obj *);
extern void Ring_gone(struct obj *);
extern void Blindf_on(struct obj *);
extern void Blindf_off(struct obj *);
extern int dotakeoff(void);
extern int doremring(void);
extern int cursed(struct obj *);
extern int armoroff(struct obj *);
extern int canwearobj(struct obj *, long *, boolean);
extern int dowear(void);
extern int doputon(void);
extern void find_ac(void);
extern void glibr(void);
extern struct obj *some_armor(struct monst *);
extern void erode_armor(struct monst *, int);
extern struct obj *stuck_ring(struct obj *, int);
extern struct obj *unchanger(void);
extern void reset_remarm(void);
extern int doddoremarm(void);
extern int destroy_arm(struct obj *);
extern void adj_abon(struct obj *, schar);
extern boolean inaccessible_equipment(struct obj *, const char *, boolean);

/* ### dog.c ### */

extern void newedog(struct monst *);
extern void free_edog(struct monst *);
extern void initedog(struct monst *);
extern struct monst *make_familiar(struct obj *, coordxy, coordxy, boolean);
extern struct monst *makedog(void);
extern void update_mlstmv(void);
extern void losedogs(void);
extern void mon_arrive(struct monst *, boolean);
extern void mon_catchup_elapsed_time(struct monst *, long);
extern void keepdogs(boolean);
extern void migrate_to_level(struct monst *, xint16, xint16, coord *);
extern int dogfood(struct monst *, struct obj *);
extern boolean tamedog(struct monst *, struct obj *);
extern void abuse_dog(struct monst *);
extern void wary_dog(struct monst *, boolean);

/* ### dogmove.c ### */

extern struct obj *droppables(struct monst *);
extern int dog_nutrition(struct monst *, struct obj *);
extern int dog_eat(struct monst *, struct obj *, coordxy, coordxy, boolean);
extern int dog_move(struct monst *, int);
#ifdef USE_TRAMPOLI
extern void wantdoor(coordxy, coordxy, genericptr_t);
#endif
extern void finish_meating(struct monst *);
extern boolean cursed_object_at(coordxy, coordxy);

/* ### dokick.c ### */

extern boolean ghitm(struct monst *, struct obj *);
extern void container_impact_dmg(struct obj *, coordxy, coordxy);
extern int dokick(void);
extern boolean ship_object(struct obj *, coordxy, coordxy, boolean);
extern void obj_delivery(boolean);
extern void deliver_obj_to_mon(struct monst *mtmp, int, unsigned long);
extern schar down_gate(coordxy, coordxy);
extern void impact_drop(struct obj *, coordxy, coordxy, xint16);

/* ### dothrow.c ### */

extern int dothrow(void);
extern int dofire(void);
extern void endmultishot(boolean);
extern void hitfloor(struct obj *, boolean);
extern void hurtle(int, int, int, boolean);
extern void mhurtle(struct monst *, int, int, int);
extern boolean throwing_weapon(struct obj *);
extern void throwit(struct obj *, long, boolean);
extern int omon_adj(struct monst *, struct obj *, boolean);
extern int thitmonst(struct monst *, struct obj *);
extern int hero_breaks(struct obj *, coordxy, coordxy, boolean);
extern int breaks(struct obj *, coordxy, coordxy);
extern void breakobj(struct obj *, coordxy, coordxy, boolean, boolean);
extern boolean breaktest(struct obj *);
extern boolean walk_path(coord *, coord *, boolean (*)(genericptr_t, coordxy, coordxy), genericptr_t);
extern boolean hurtle_jump(void *, coordxy, coordxy);
extern boolean hurtle_step(void *, coordxy, coordxy);

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern int def_char_to_objclass(char);
extern int def_char_to_monclass(char);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void assign_graphics(glyph_t *, int, int, int);
extern void switch_graphics(int);
#ifdef REINCARNATION
extern void assign_rogue_graphics(boolean);
#endif
extern void assign_utf8graphics_symbol(int, glyph_t);
extern void assign_moria_graphics(boolean);

/* ### dump.c ### */

extern void dump(const char *, const char *);
extern void dump_blockquote_start(void);
extern void dump_blockquote_end(void);
extern void dump_text(const char *, const char *);
extern void dump_header_html(const char *);
extern void dump_html(const char *, const char *);
extern void dump_init(void);
extern void dump_exit(void);
extern void dump_object(const char, const struct obj *, const char *);
extern void dump_title(char *);
extern void dump_subtitle(const char *);
extern void dump_line(const char *, const char *);
extern void dump_list_start(void);
extern void dump_list_item(const char *);
extern void dump_list_item_object(struct obj *);
extern void dump_list_item_link(const char *, const char *);
extern void dump_list_end(void);
extern void dump_definition_list_start(void);
extern void dump_definition_list_dd(const char *);
extern void dump_definition_list_dt(const char *);
extern void dump_definition_list_end(void);
extern void dump_containerconts(struct obj *, boolean, boolean, boolean);
extern const char* html_escape_character(const char);
extern char* html_link(const char *, const char *);
#ifdef DUMP_LOG
extern int dump_screenshot(void);
extern int dumpoverview(void);
#endif

/* ### dungeon.c ### */

extern void save_dungeon(int, boolean, boolean);
extern void restore_dungeon(int);
extern void insert_branch(branch *, boolean);
extern void init_dungeons(void);
extern s_level *find_level(const char *);
extern s_level *Is_special(d_level *);
#ifdef RANDOMIZED_PLANES
extern s_level *get_next_elemental_plane(d_level *);
extern d_level *get_first_elemental_plane(void);
#endif
extern branch *Is_branchlev(d_level *);
extern boolean builds_up(d_level *);
extern xint16 ledger_no(d_level *);
extern xint16 maxledgerno(void);
extern schar depth(d_level *);
extern xint16 dunlev(d_level *);
extern xint16 dunlevs_in_dungeon(d_level *);
extern xint16 ledger_to_dnum(xint16);
extern xint16 ledger_to_dlev(xint16);
extern xint16 deepest_lev_reached(boolean);
extern boolean on_level(d_level *, d_level *);
extern void next_level(boolean);
extern void prev_level(boolean);
extern void u_on_newpos(coordxy, coordxy);
extern void u_on_rndspot(int);
extern void u_on_sstairs(int);
extern void u_on_upstairs(void);
extern void u_on_dnstairs(void);
extern boolean On_stairs(coordxy, coordxy);
extern void get_level(d_level *, int);
extern boolean Is_botlevel(d_level *);
extern boolean Can_fall_thru(d_level *);
extern boolean Can_dig_down(d_level *);
extern boolean Can_rise_up(coordxy, coordxy, d_level *);
extern boolean has_ceiling(d_level *);
extern boolean In_quest(d_level *);
extern boolean In_mines(d_level *);
extern boolean In_sheol(d_level *);
extern branch *dungeon_branch(const char *);
extern boolean at_dgn_entrance(const char *);
extern boolean In_hell(d_level *);
extern boolean In_V_tower(d_level *);
extern boolean On_W_tower_level(d_level *);
extern boolean In_W_tower(coordxy, coordxy, d_level *);
extern void find_hell(d_level *);
extern void goto_hell(boolean, boolean);
extern void assign_level(d_level *, d_level *);
extern void assign_rnd_level(d_level *, d_level *, int);
extern int induced_align(int);
extern boolean Invocation_lev(d_level *);
extern xint16 level_difficulty(void);
extern schar lev_by_name(const char *);
#ifdef WIZARD
extern schar print_dungeon(boolean, schar *, xint16 *);
#endif
extern int donamelevel(void);
extern int dooverview(void);
extern void show_overview(int, int);
extern void forget_mapseen(int);
extern void init_mapseen(d_level *);
extern void recalc_mapseen(void);
extern void mapseen_temple(struct monst *);
extern void recbranch_mapseen(d_level *, d_level *);
extern void room_discovered(int);
extern void overview_stats(winid, const char *, long *, long *);
extern void remdun_mapseen(int);
extern char *get_annotation(d_level *);
extern const char *get_generic_level_description(d_level *);
extern const char *endgame_level_name(char *, int);

/* ### eat.c ### */

#ifdef USE_TRAMPOLI
extern int eatmdone(void);
extern int eatfood(void);
extern int opentin(void);
extern int unfaint(void);
#endif
extern boolean is_edible(struct obj *);
extern void init_uhunger(void);
extern int Hear_again(void);
extern void reset_eat(void);
extern int doeat(void);
extern void gethungry(void);
extern void morehungry(int);
extern void lesshungry(int);
extern boolean is_fainted(void);
extern void reset_faint(void);
extern void violated_vegetarian(void);
#if 0
extern void sync_hunger(void);
#endif
extern void newuhs(boolean);
extern struct obj *floorfood(const char *, int);
extern void vomit(void);
extern int eaten_stat(int, struct obj *);
extern void food_disappears(struct obj *);
extern void food_substitution(struct obj *, struct obj *);
extern boolean bite_monster(struct monst *mon);
extern void fix_petrification(void);
extern int intrinsic_possible(int, struct permonst *);
extern void consume_oeaten(struct obj *, int);
extern boolean maybe_finished_meal(boolean);
extern void set_tin_variety(struct obj *, int);
extern int tin_variety_txt(char *, int *);
extern void tin_details(struct obj *, int, char *);
extern boolean Popeye(int);

/* ### end.c ### */

extern void done1(int);
extern int done2(void);
#ifdef USE_TRAMPOLI
extern void done_intr(int);
#endif
extern void done_in_by(struct monst *);
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern void VDECL(panic, (const char *, ...)) PRINTF_F(1, 2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void done(int);
extern void container_contents(struct obj *, boolean, boolean, boolean);
extern void nh_terminate(int) NORETURN;
extern int num_genocides(void);
extern void delayed_killer(int, int, const char *);
extern struct kinfo *find_delayed_killer(int);
extern void dealloc_killer(struct kinfo *);
extern void save_killers(int, int);
extern void restore_killers(int);
extern char *build_english_list(char *);

/* ### engrave.c ### */

extern char *random_engraving(char *);
extern void wipeout_text(char *, int, unsigned);
extern boolean can_reach_floor(boolean);
extern void You_cant_reach_the_floor(coordxy, coordxy, boolean);
extern void You_cant_reach_the_ceiling(coordxy, coordxy);
extern void cant_reach_floor(coordxy, coordxy, boolean, boolean);
extern const char *surface(coordxy, coordxy);
extern const char *ceiling(coordxy, coordxy);
extern struct engr *engr_at(coordxy, coordxy);
extern int sengr_at(const char *, coordxy, coordxy);
extern void u_wipe_engr(int);
extern void wipe_engr_at(coordxy, coordxy, xint16);
extern void read_engr_at(coordxy, coordxy);
extern void make_engr_at(coordxy, coordxy, const char *, long, xint16);
extern void del_engr_at(coordxy, coordxy);
extern int freehand(void);
extern int doengrave(void);
extern int doengrave_elbereth(void);
extern void sanitize_engravings(void);
extern void save_engravings(int, int);
extern void rest_engravings(int);
extern void engr_stats(const char *, char *, long *, long *);
extern void del_engr(struct engr *);
extern void rloc_engr(struct engr *);
extern void make_grave(coordxy, coordxy, const char *);

/* ### exper.c ### */

extern long newuexp(int);
extern int newpw(void);
extern int experience(struct monst *, int);
extern void more_experienced(int, int, int);
extern void losexp(const char *);
extern void newexplevel(void);
extern void pluslvl(boolean);
extern long rndexp(boolean);

/* ### explode.c ### */

extern void explode(coordxy, coordxy, int, int, char, int);
extern long scatter(int, int, int, unsigned int, struct obj *);
extern void splatter_burning_oil(coordxy, coordxy, boolean);
extern void explode_oil(struct obj *, coordxy, coordxy);

/* ### extralev.c ### */

#ifdef REINCARNATION
extern void makeroguerooms(void);
extern void corr(coordxy, coordxy);
extern void makerogueghost(void);
#endif

/* ### files.c ### */

extern char *fname_encode(const char *, char, char *, char *, int);
extern char *fname_decode(char, char *, char *, int);
extern const char *fqname(const char *, int, int);
#ifndef FILE_AREAS
extern FILE *fopen_datafile(const char *, const char *, int);
#endif
extern boolean uptodate(int, const char *);
extern void store_version(int);
#ifdef MFLOPPY
extern void set_lock_and_bones(void);
#endif
extern void set_levelfile_name(char *, int);
extern int create_levelfile(int, char *);
extern int open_levelfile(int, char *);
extern void delete_levelfile(int);
extern void clearlocks(void);
extern int create_bonesfile(d_level*, char **, char *);
#ifdef MFLOPPY
extern void cancel_bonesfile(void);
#endif
extern void commit_bonesfile(d_level *);
extern int open_bonesfile(d_level*, char **);
extern int delete_bonesfile(d_level*);
extern void compress_bonesfile(void);
extern void set_savefile_name(void);
#ifdef INSURANCE
extern void save_savefile_name(int);
#endif
#if defined(WIZARD) && !defined(MICRO)
extern void set_error_savefile(void);
#endif
extern int create_savefile(void);
extern int open_savefile(void);
extern int delete_savefile(void);
extern int restore_saved_game(void);
extern void compress(const char *);
extern void uncompress(const char *);
extern void compress_area(const char *, const char *);
extern void uncompress_area(const char *, const char *);
#ifndef FILE_AREAS
extern boolean lock_file(const char *, int, int);
extern void unlock_file(const char *);
#endif
#ifdef USER_SOUNDS
extern boolean can_read_file(const char *);
#endif
extern boolean read_config_file(const char *, int);
extern void check_recordfile(const char *);
#if defined(WIZARD)
extern void read_wizkit(void);
#endif
extern void paniclog(const char *, const char *);
extern int validate_prefix_locations(char *);
extern char** get_saved_games(void);
extern void free_saved_games(char**);
#ifdef SELF_RECOVER
extern boolean recover_savefile(void);
#endif
#ifdef SYSCF_FILE
extern void assure_syscf_file(void);
#endif
extern int nhclose(int);
#ifdef HOLD_LOCKFILE_OPEN
extern void really_close(void);
#endif
#ifdef WHEREIS_FILE
extern void touch_whereis(void);
extern void delete_whereis(void);
extern void signal_whereis(int);
#endif

/* ### fountain.c ### */

extern void floating_above(const char *);
extern void dogushforth(int);
# ifdef USE_TRAMPOLI
extern void gush(int, int, genericptr_t);
# endif
extern void dryup(coordxy, coordxy, boolean);
extern void drinkfountain(void);
extern void dipfountain(struct obj *);
#ifdef SINKS
extern void breaksink(coordxy, coordxy);
extern void drinksink(void);
#endif

/* ### hack.c ### */

#ifdef DUNGEON_GROWTH
extern void catchup_dgn_growths(int);
extern void dgn_growths(boolean, boolean);
#endif
extern boolean is_valid_travelpt(coordxy, coordxy);
extern anything *uint_to_any(unsigned);
extern anything *long_to_any(long);
extern anything *monst_to_any(struct monst *);
extern anything *obj_to_any(struct obj *);
extern boolean revive_nasty(coordxy, coordxy, const char*);
extern void movobj(struct obj *, coordxy, coordxy);
extern boolean may_dig(coordxy, coordxy);
extern boolean may_passwall(coordxy, coordxy);
extern boolean bad_rock(struct permonst *, coordxy, coordxy);
extern int cant_squeeze_thru(struct monst *);
extern boolean invocation_pos(coordxy, coordxy);
extern boolean test_move(int, int, int, int, int);
extern boolean u_rooted(void);
extern void domove(void);
extern void runmode_delay_output(void);
extern boolean overexertion(void);
extern void invocation_message(void);
extern void switch_terrain(void);
extern boolean pooleffects(boolean);
extern void wounds_message(struct monst *);
extern char *mon_wounds(struct monst *);
extern void spoteffects(boolean);
extern char *in_rooms(coordxy, coordxy, int);
extern boolean in_town(coordxy, coordxy);
extern void check_special_room(boolean);
extern int dopickup(void);
extern void lookaround(void);
extern boolean crawl_destination(coordxy, coordxy);
extern int monster_nearby(void);
extern void nomul(int, const char *);
extern void unmul(const char *);
extern void showdmg(int, boolean);
extern void losehp(int, const char *, boolean);
extern void losehp_how(int, const char *, boolean, int);
extern void set_uhpmax(int, boolean);
extern void check_uhpmax(void);
extern int weight_cap(void);
extern int inv_weight(void);
extern int near_capacity(void);
extern int calc_capacity(int);
extern int max_capacity(void);
extern boolean check_capacity(const char *);
extern int inv_cnt(boolean);
extern long money_cnt(struct obj *);
extern boolean MON_AT(coordxy, coordxy);
extern boolean OBJ_AT(coordxy, coordxy);

/* ### hacklib.c ### */

extern boolean digit(char);
extern boolean letter(char);
extern char highc(char);
extern char lowc(char);
extern char *lcase(char *);
extern char *ucase(char *);
extern char *upstart(char *);
extern char *mungspaces(char *);
extern char *trimspaces(char *);
extern char *strip_newline(char *);
extern char *eos(char *);
extern void sanitizestr(char *);
extern boolean str_end_is(const char *, const char *);
extern char *strkitten(char *, char);
extern void copynchars(char *, const char *, int);
extern char *strcasecpy(char *, const char *);
extern char *s_suffix(const char *);
extern char *xcrypt(const char *, char *);
extern boolean onlyspace(const char *);
extern char *tabexpand(char *);
extern char *visctrl(char);
extern char *strsubst(char *, const char *, const char *);
extern int strNsubst(char *, const char *, const char *, int);
extern const char *ordin(int);
extern char *sitoa(int);
extern int sgn(int);
extern int rounddiv(long, int);
extern int dist2(int, int, int, int);
extern int isqrt(int);
extern int ilog2(int);
extern int distmin(int, int, int, int);
extern boolean online2(int, int, int, int);
extern boolean pmatch(const char *, const char *);
extern boolean pmatchi(const char *, const char *);
#ifndef STRNCMPI
extern int strncmpi(const char *, const char *, int);
#endif
#ifndef STRSTRI
extern char *strstri(const char *, const char *);
#endif
extern boolean fuzzymatch(const char *, const char *, const char *, boolean);
extern void init_random(unsigned int);
extern void reseed_random(void);
extern void set_random_state(unsigned int);
extern int getyear(void);
extern int getmonth(void);
extern int getmday(void);
#if 0
extern char *yymmdd(time_t);
#endif
extern long yyyymmdd(time_t);
extern int phase_of_the_moon(void);
extern boolean friday_13th(void);
extern boolean towelday(void);
extern boolean pirateday(void);
extern int night(void);
extern int midnight(void);
extern boolean piday(void);
extern boolean aprilfoolsday(void);
extern boolean discordian_holiday(void);
extern boolean is_june(void);
extern char *iso8601(time_t);
extern char *iso8601_duration(long);
extern char* format_duration(long);
extern char *get_formatted_time(time_t, const char *);
extern time_t current_epoch(void);
extern void strbuf_init(strbuf_t *);
extern void strbuf_append(strbuf_t *, const char *);
extern void strbuf_reserve(strbuf_t *, int);
extern void strbuf_empty(strbuf_t *);
extern void strbuf_nl_to_crlf(strbuf_t *);
extern int swapbits(int, int, int);
extern void strip_brackets(char *);
/* note: the snprintf CPP wrapper includes the "fmt" argument in "..."
   (__VA_ARGS__) to allow for zero arguments after fmt */
#define Snprintf(str, size, ...) \
    nh_snprintf(__func__, __LINE__, str, size, __VA_ARGS__)
extern void nh_snprintf(const char *func, int line, char *str, size_t size,
                        const char *fmt, ...);

/* ### invent.c ### */

extern void assigninvlet(struct obj *);
extern struct obj *merge_choice(struct obj *, struct obj *);
extern int merged(struct obj **, struct obj **);
#ifdef USE_TRAMPOLI
extern int ckunpaid(struct obj *);
#endif
extern void addinv_core1(struct obj *);
extern void addinv_core2(struct obj *);
extern struct obj *addinv(struct obj *);
extern struct obj *hold_another_object(struct obj *, const char *, const char *, const char *);
extern void useupall(struct obj *);
extern void useup(struct obj *);
extern void consume_obj_charge(struct obj *, boolean);
extern void freeinv_core(struct obj *);
extern void freeinv(struct obj *);
extern void delallobj(coordxy, coordxy);
extern void delobj(struct obj *);
extern struct obj *sobj_at(int, coordxy, coordxy);
extern struct obj *nxtobj(struct obj *, int, boolean);
extern struct obj *carrying(int);
extern boolean have_lizard(void);
extern struct obj *o_on(unsigned int, struct obj *);
extern boolean obj_here(struct obj *, coordxy, coordxy);
extern boolean wearing_armor(void);
extern boolean is_worn(struct obj *);
extern struct obj *g_at(coordxy, coordxy);
extern struct obj *mkgoldobj(long);
extern struct obj *getobj(const char *, const char *);
extern int ggetobj(const char *, int (*)(OBJ_P), int, boolean, unsigned *);
extern void fully_identify_obj(struct obj *);
extern int identify(struct obj *);
extern void identify_pack(int, boolean);
extern int askchain(struct obj **, const char *, int, int (*)(OBJ_P), int (*)(OBJ_P), int, const char *);
extern void prinv(const char *, struct obj *, long);
extern char *xprname(struct obj *, const char *, char, boolean, long, long);
extern int ddoinv(void);
extern char display_inventory(const char *, boolean);
extern char dump_inventory(const char *, boolean, boolean);
extern int display_binventory(coordxy, coordxy, boolean);
extern struct obj *display_cinventory(struct obj *);
extern struct obj *display_minventory(struct monst *, int, char *);
extern int dotypeinv(void);
extern const char *dfeature_at(coordxy, coordxy, char *);
extern int look_here(int, boolean);
extern int dolook(void);
extern boolean will_feel_cockatrice(struct obj *, boolean);
extern void feel_cockatrice(struct obj *, boolean);
extern void stackobj(struct obj *);
extern boolean mergable(struct obj *, struct obj *);
extern int doprgold(void);
extern int doprwep(void);
extern int doprarm(void);
extern int doprring(void);
extern int dopramulet(void);
extern int doprtool(void);
extern int doprinuse(void);
extern void useupf(struct obj *, long);
extern char *let_to_name(char, boolean);
extern void free_invbuf(void);
extern void reassign(void);
extern int doorganize(void);
extern int count_objects(struct obj *);
extern int count_unidentified(struct obj *);
extern void learn_unseen_invent(void);
extern void update_inventory(void);
extern int count_unpaid(struct obj *);
extern int count_buc(struct obj *, int, boolean (*)(OBJ_P));
extern void tally_BUCX(struct obj *, boolean, int *, int *, int *, int *, int *);
extern long count_contents(struct obj *, boolean, boolean, boolean, boolean);
extern void carry_obj_effects(struct obj *);
extern const char *currency(long);
extern void silly_thing(const char *, struct obj *);
extern struct obj *getnextgetobj(void);
#ifdef SORTLOOT
extern int sortloot_cmp(struct obj *, struct obj *);
#endif
extern boolean is_racial_armor(struct obj *);
extern boolean is_racial_weapon(struct obj *);
extern boolean is_dragon_identified(struct permonst *);
extern void identify_dragon(int);

/* ### ioctl.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void getwindowsz(void);
extern void getioctls(void);
extern void setioctls(void);
# ifdef SUSPEND
extern int dosuspend(void);
# endif /* SUSPEND */
#endif /* UNIX || __BEOS__ */

/* ### light.c ### */

extern void new_light_source(coordxy, coordxy, int, int, ANY_P *);
extern void del_light_source(int, ANY_P *);
extern void do_light_sources(char **);
extern void show_transient_light(struct obj *, coordxy, coordxy);
extern void transient_light_cleanup(void);
extern struct monst *find_mid(unsigned, unsigned);
extern void save_light_sources(int, int, int);
extern void restore_light_sources(int);
extern void light_stats(const char *, char *, long *, long *);
extern void relink_light_sources(boolean);
extern void light_sources_sanity_check(void);
extern void obj_move_light_source(struct obj *, struct obj *);
extern boolean any_light_source(void);
extern void snuff_light_source(coordxy, coordxy);
extern boolean obj_sheds_light(struct obj *);
extern boolean obj_is_burning(struct obj *);
extern void obj_split_light_source(struct obj *, struct obj *);
extern void obj_merge_light_sources(struct obj *, struct obj *);
extern void obj_adjust_light_radius(struct obj *, int);
extern int candle_light_range(struct obj *);
extern int arti_light_radius(struct obj *);
extern const char *arti_light_description(struct obj *);
#ifdef WIZARD
extern int wiz_light_sources(void);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
extern int forcelock(void);
extern int picklock(void);
#endif
extern boolean picking_lock(int *, int *);
extern boolean picking_at(coordxy, coordxy);
extern void breakchestlock(struct obj *, boolean);
extern void reset_pick(void);
extern void maybe_reset_pick(struct obj *);
extern int pick_lock(struct obj *, int, int, boolean);
extern int doforce(void);
extern boolean boxlock(struct obj *, struct obj *);
extern boolean doorlock(struct obj *, coordxy, coordxy);
extern int doopen(void);
extern int doclose(void);
extern int artifact_door(coordxy, coordxy);
extern boolean stumble_on_door_mimic(coordxy, coordxy);
#ifdef AUTO_OPEN
extern int doopen_indir(coordxy, coordxy);
#endif

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

extern int maccreat(const char *, long);
extern int macopen(const char *, int, long);
extern int macclose(int);
extern int macread(int, void *, unsigned);
extern int macwrite(int, void *, unsigned);
extern long macseek(int, long, short);
extern int macunlink(const char *);

/* ### macsnd.c ### */

extern void mac_speaker(struct obj *, char *);

/* ### macunix.c ### */

extern void regularize(char *);
extern void getlock(void);

/* ### macwin.c ### */

extern void lock_mouse_cursor(Boolean);
extern int SanePositions(void);

/* ### mttymain.c ### */

extern void getreturn(char *);
extern void VDECL(msmsg, (const char *, ...));
extern void gettty(void);
extern void setftty(void);
extern void settty(const char *);
extern int tgetch(void);
extern void cmov(int x, int y);
extern void nocmov(int x, int y);

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
extern void getmailstatus(void);
# endif
extern void ckmailstatus(void);
extern void read_hint(struct obj *);
extern void readmail(struct obj *);
extern void maybe_hint(void);
#endif /* MAIL */

/* ### makemon.c ### */

extern void mon_sanity_check(void);
extern void dealloc_monst(struct monst *);
extern boolean is_home_elemental(struct permonst *);
extern struct monst *clone_mon(struct monst *, coordxy, coordxy);
extern int monhp_per_lvl(struct monst *);
extern void newmonhp(struct monst *, int);
extern struct mextra *newmextra(void);
extern void copy_mextra(struct monst *, struct monst *);
extern void dealloc_mextra(struct monst *);
extern struct monst *makemon(struct permonst *, coordxy, coordxy, int);
extern boolean create_critters(int, struct permonst *);
extern struct permonst *rndmonst(void);
extern void reset_rndmonst(int);
extern struct permonst *mkclass(char, int);
extern struct permonst *mkclass_aligned(char, int, aligntyp);
extern int mkclass_poly(int);
extern int adj_lev(struct permonst *);
extern struct permonst *grow_up(struct monst *, struct monst *);
extern int mongets(struct monst *, int);
extern int golemhp(int);
extern boolean peace_minded(struct permonst *);
extern void set_malign(struct monst *);
extern void newmcorpsenm(struct monst *);
extern void freemcorpsenm(struct monst *);
extern void set_mimic_sym(struct monst *);
extern int mbirth_limit(int);
extern void mimic_hit_msg(struct monst *, short);
extern void mkmonmoney(struct monst *, long);
extern int bagotricks(struct obj *);
extern boolean propagate(int, boolean, boolean);
extern void create_camera_demon(struct obj *, coordxy, coordxy);
extern int min_monster_difficulty(void);
extern int max_monster_difficulty(void);
extern boolean prohibited_by_generation_flags(struct permonst *);
extern boolean usmellmon(struct permonst *);

/* ### mapglyph.c ### */

extern void mapglyph(int, glyph_t *, int *, unsigned *, coordxy, coordxy, unsigned);
extern char *encglyph(int);
extern glyph_t get_monsym(int glyph);

/* ### mcastu.c ### */

extern int castmu(struct monst *, struct attack *, boolean, boolean);
extern int buzzmu(struct monst *, struct attack *);

/* ### mhitm.c ### */

extern int fightm(struct monst *);
extern int mattackm(struct monst *, struct monst *);
extern boolean engulf_target(struct monst *, struct monst *);
extern int mdisplacem(struct monst *, struct monst *, boolean);
extern void paralyze_monst(struct monst *, int);
extern int sleep_monst(struct monst *, int, int);
extern void slept_monst(struct monst *);
extern long attk_protection(int);
extern void rustm(struct monst *, struct obj *);
extern void maybe_freeze_m(struct monst *, int, int*);

/* ### mhitu.c ### */

extern const char *mpoisons_subj(struct monst *, struct attack *);
extern void u_slow_down(void);
extern struct monst *cloneu(void);
extern void expels(struct monst *, struct permonst *, boolean);
extern struct attack *getmattk(struct monst *, struct monst *, int, int *, struct attack *);
extern int mattacku(struct monst *);
extern int magic_negation(struct monst *);
extern boolean gulp_blnd_check(void);
extern int gazemu(struct monst *, struct attack *);
extern void mdamageu(struct monst *, int);
extern int could_seduce(struct monst *, struct monst *, struct attack *);
#ifdef SEDUCE
extern int doseduce(struct monst *);
#endif
extern void maybe_freeze_u(int*);
extern void spore_dies(struct monst *);

/* ### minion.c ### */

extern void newemin(struct monst *);
extern void free_emin(struct monst *);
extern int monster_census(boolean);
extern int msummon(struct monst *);
extern void summon_minion(aligntyp, boolean);
extern int demon_talk(struct monst *);
extern long bribe(struct monst *);
extern int dprince(aligntyp);
extern int dlord(aligntyp);
extern int llord(void);
extern int ndemon(aligntyp);
extern int lminion(void);
extern void lose_guardian_angel(struct monst *);
extern void gain_guardian_angel(void);

/* ### mklev.c ### */

extern void mineralize(int, int, int, int, boolean);
#ifdef USE_TRAMPOLI
extern int do_comp(genericptr_t, genericptr_t);
#endif
extern void sort_rooms(void);
extern void add_room(int, int, int, int, boolean, schar, boolean);
extern void add_subroom(struct mkroom *, int, int, int, int, boolean, schar, boolean);
extern void makecorridors(int);
extern void add_door(coordxy, coordxy, struct mkroom *);
extern void mkpoolroom(void);
extern void mklev(void);
#ifdef SPECIALIZATION
extern void topologize(struct mkroom *, boolean);
#else
extern void topologize(struct mkroom *);
#endif
extern void place_branch(branch *, coordxy, coordxy);
extern boolean occupied(coordxy, coordxy);
extern int okdoor(coordxy, coordxy);
extern void dodoor(coordxy, coordxy, struct mkroom *);
extern void mktrap(int, int, struct mkroom *, coord*);
extern void mkstairs(coordxy, coordxy, char, struct mkroom *);
extern void mkinvokearea(void);
extern void wallwalk_right(coordxy, coordxy, schar, schar, schar, int);
#ifdef ADVENT_CALENDAR
extern boolean mk_advcal_portal(void);
#endif
extern void mk_knox_vault(coordxy, coordxy, int, int);
extern d_level * get_floating_branch(d_level *, branch *);

/* ### mkmap.c ### */

void flood_fill_rm(int, int, int, boolean, boolean);
void remove_rooms(int, int, int, int);

/* ### mkmaze.c ### */

extern void wallification(int, int, int, int);
extern void wall_extends(int, int, int, int);
extern void walkfrom(coordxy, coordxy, schar);
extern void makemaz(const char *);
extern void mazexy(coord *);
extern void get_level_extends(int *, int *, int *, int *);
extern void bound_digging(void);
extern void mkportal(coordxy, coordxy, xint16, xint16);
extern boolean bad_location(coordxy, coordxy, coordxy, coordxy, coordxy, coordxy, coordxy);
extern int place_lregion(coordxy, coordxy, coordxy, coordxy,
                    coordxy, coordxy, coordxy, coordxy,
                    xint16, d_level *);
extern void fixup_special(void);
extern void movebubbles(void);
extern void water_friction(void);
extern void save_waterlevel(int, int);
extern void restore_waterlevel(int);
extern const char *waterbody_name(coordxy, coordxy);

#ifdef ADVENT_CALENDAR
extern void fill_advent_calendar(boolean);
#endif

/* ### mkobj.c ### */

extern struct oextra *newoextra(void);
extern void copy_oextra(struct obj *, struct obj *);
extern void dealloc_oextra(struct obj *);
extern void newomonst(struct obj *);
extern void free_omonst(struct obj *);
extern void newomid(struct obj *);
extern void free_omid(struct obj *);
extern void newolong(struct obj *);
extern void free_olong(struct obj *);
extern void new_omailcmd(struct obj *, const char *);
extern void free_omailcmd(struct obj *);
extern struct obj *mkobj_at(char, coordxy, coordxy, boolean);
extern void costly_alteration(struct obj *, int);
extern struct obj *mksobj_at(int, coordxy, coordxy, boolean, boolean);
extern struct obj *mksobj_migr_to_species(int, unsigned, boolean, boolean);
extern struct obj *mkobj(char, boolean);
extern int rndmonnum(void);
extern boolean bogon_is_pname(char);
extern struct obj *splitobj(struct obj *, long);
extern struct obj *unsplitobj(struct obj *);
extern void clear_splitobjs(void);
extern void replace_object(struct obj *, struct obj *);
extern void bill_dummy_object(struct obj *);
extern struct obj *mksobj(int, boolean, boolean);
extern int bcsign(struct obj *);
extern int weight(struct obj *);
extern struct obj *mkgold(long, coordxy, coordxy);
extern struct obj *mkcorpstat(int, struct monst *, struct permonst *, coordxy, coordxy, boolean);
extern struct obj *obj_attach_mid(struct obj *, unsigned);
extern struct monst *get_mtraits(struct obj *, boolean);
extern struct obj *mk_tt_object(int, coordxy, coordxy);
extern struct obj *mk_named_object(int, struct permonst *, coordxy, coordxy, const char *);
extern struct obj *rnd_treefruit_at(coordxy, coordxy);
extern void rnd_treesticks_at(coordxy, coordxy);
extern void set_corpsenm(struct obj *, int);
extern void start_corpse_timeout(struct obj *);
extern void bless(struct obj *);
extern void unbless(struct obj *);
extern void curse(struct obj *);
extern void uncurse(struct obj *);
extern void blessorcurse(struct obj *, int);
extern void set_bknown(struct obj *, unsigned);
extern boolean is_flammable(struct obj *);
extern boolean is_rottable(struct obj *);
extern void place_object(struct obj *, coordxy, coordxy);
extern void remove_object(struct obj *);
extern void discard_minvent(struct monst *);
extern void obj_extract_self(struct obj *);
extern void extract_nobj(struct obj *, struct obj **);
extern void extract_nexthere(struct obj *, struct obj **);
extern int add_to_minv(struct monst *, struct obj *);
extern struct obj *add_to_container(struct obj *, struct obj *);
extern void add_to_migration(struct obj *);
extern void add_to_buried(struct obj *);
extern void dealloc_obj(struct obj *);
extern void obj_ice_effects(coordxy, coordxy, boolean);
extern long peek_at_iced_corpse_age(struct obj *);
extern int hornoplenty(struct obj *, boolean);
extern int do_stair_travel(char);
#ifdef WIZARD
extern void obj_sanity_check(void);
#endif
extern struct obj *obj_nexto(struct obj *);
extern struct obj *obj_nexto_xy(struct obj *, coordxy, coordxy, boolean);
extern struct obj *obj_absorb(struct obj **, struct obj **);
extern struct obj *obj_meld(struct obj **, struct obj **);
extern void pudding_merge_message(struct obj *, struct obj *);
extern struct obj *init_dummyobj(struct obj *, short, long);

/* ### mkroom.c ### */

extern struct mkroom * pick_room(boolean);
extern void mkroom(int);
extern void fill_zoo(struct mkroom *);
extern boolean nexttodoor(int, int);
extern boolean bydoor(coordxy, coordxy);
extern boolean somexyspace(struct mkroom *, coord *, int);
extern boolean has_dnstairs(struct mkroom *);
extern boolean has_upstairs(struct mkroom *);
extern int somex(struct mkroom *);
extern int somey(struct mkroom *);
extern boolean inside_room(struct mkroom *, coordxy, coordxy);
extern boolean somexy(struct mkroom *, coord *);
extern void mkundead(coord *, boolean, int);
extern struct permonst *courtmon(void);
extern void save_rooms(int);
extern void rest_rooms(int);
extern struct mkroom *search_special(schar);
extern int cmap_to_type(int);
extern boolean is_rainbow_shop(struct mkroom *);

/* #### mksheol.c ### */

extern void mksheol(void*);

/* ### mon.c ### */

extern boolean zombie_maker(struct monst *);
extern int zombie_form(struct permonst *);
extern int select_newcham_form(struct monst *);
extern void remove_monster(coordxy, coordxy);
extern int m_poisongas_ok(struct monst *);
extern int undead_to_corpse(int);
extern int genus(int, int);
extern int pm_to_cham(int);
extern int minliquid(struct monst *);
extern int movemon(void);
extern int meatmetal(struct monst *);
extern int meatobj(struct monst *);
extern void mpickgold(struct monst *);
extern boolean mpickstuff(struct monst *, const char *);
extern void mpickup_obj(struct monst *, struct obj *);
extern int curr_mon_load(struct monst *);
extern int max_mon_load(struct monst *);
extern int can_carry(struct monst *, struct obj *);
extern int mfndpos(struct monst *, coord *, long *, long);
extern boolean monnear(struct monst *, coordxy, coordxy);
extern void dmonsfree(void);
extern void elemental_clog(struct monst *);
extern int mcalcmove(struct monst*);
extern void mcalcdistress(void);
extern void replmon(struct monst *, struct monst *);
extern void relmon(struct monst *, struct monst **);
extern struct obj *mlifesaver(struct monst *);
extern boolean corpse_chance(struct monst *, struct monst *, boolean);
#ifdef WEBB_DISINT
extern void mondead_helper(struct monst *, uchar);
#endif
extern void mondead(struct monst *);
extern void mondied(struct monst *);
extern void mongone(struct monst *);
extern void monstone(struct monst *);
extern void monkilled(struct monst *, const char *, int);
extern void unstuck(struct monst *);
extern void killed(struct monst *);
extern void xkilled(struct monst *, int);
extern void mon_to_stone(struct monst*);
extern void m_into_limbo(struct monst *);
extern void mnexto(struct monst *);
extern void maybe_mnexto(struct monst *);
extern int mnearto(struct monst *, coordxy, coordxy, boolean);
extern void poisontell(int);
extern void poisoned(const char *, int, const char *, int);
extern void m_respond(struct monst *);
extern void setmangry(struct monst *, boolean);
extern void wakeup(struct monst *, boolean);
extern void wake_nearby(void);
extern void wake_nearto(coordxy, coordxy, int);
extern void seemimic(struct monst *);
extern void rescham(void);
extern void restartcham(void);
extern void restore_cham(struct monst *);
extern boolean hideunder(struct monst *);
extern void mon_animal_list(boolean);
extern boolean validvamp(struct monst *, int *, int);
extern void mgender_from_permonst(struct monst *, struct permonst *);
extern int newcham(struct monst *, struct permonst *, boolean, boolean);
extern int can_be_hatched(int);
extern int egg_type_from_parent(int, boolean);
extern boolean dead_species(int, boolean);
extern void kill_genocided_monsters(void);
extern void kill_monster_on_level(int);
extern void golemeffects(struct monst *, int, int);
extern boolean angry_guards(boolean);
extern void pacify_guards(void);
extern void decide_to_shapeshift(struct monst *, int);

/* ### mondata.c ### */

extern void set_mon_data(struct monst *, struct permonst *);
extern struct attack *attacktype_fordmg(struct permonst *, int, int);
extern boolean attacktype(struct permonst *, int);
extern boolean noattacks(struct permonst *);
extern boolean poly_when_stoned(struct permonst *);
extern boolean resists_drli(struct monst *);
extern boolean resists_magm(struct monst *);
extern boolean resists_blnd(struct monst *);
extern boolean can_blnd(struct monst *, struct monst *, uchar, struct obj *);
extern boolean ranged_attk(struct permonst *);
extern boolean hates_silver(struct permonst *);
extern boolean mon_hates_silver(struct monst *);
extern boolean mon_hates_light(struct monst *);
extern boolean passes_bars(struct permonst *);
extern boolean can_blow(struct monst *);
extern boolean can_chant(struct monst *);
extern boolean can_be_strangled(struct monst *);
extern boolean can_track(struct permonst *);
extern boolean breakarm(struct permonst *);
extern boolean sliparm(struct permonst *);
extern boolean sticks(struct permonst *);
extern boolean cantvomit(struct permonst *);
extern int num_horns(struct permonst *);
/* E boolean canseemon(struct monst *); */
extern struct attack *dmgtype_fromattack(struct permonst *, int, int);
extern boolean dmgtype(struct permonst *, int);
extern int max_passive_dmg(struct monst *, struct monst *);
extern boolean same_race(struct permonst *, struct permonst *);
extern int monsndx(struct permonst *);
extern int name_to_mon(const char *);
extern int name_to_monclass(const char *, int *);
extern int gender(struct monst *);
extern int pronoun_gender(struct monst *, boolean);
extern boolean levl_follower(struct monst *);
extern int little_to_big(int);
extern int big_to_little(int);
extern const char *locomotion(const struct permonst *, const char *);
extern const char *stagger(const struct permonst *, const char *);
extern const char *on_fire(struct permonst *, struct attack *);
extern const struct permonst *raceptr(struct monst *);
extern boolean olfaction(struct permonst *);
extern boolean is_fleshy(const struct permonst *);

/* ### monmove.c ### */

extern boolean itsstuck(struct monst *);
extern boolean mb_trapped(struct monst *);
extern boolean mon_has_key(struct monst *, boolean);
extern void mon_regen(struct monst *, boolean);
extern int dochugw(struct monst *);
extern boolean onscary(coordxy, coordxy, struct monst *);
extern void monflee(struct monst *, int, boolean, boolean);
extern int dochug(struct monst *);
extern boolean m_digweapon_check(struct monst *, coordxy, coordxy);
extern int m_move(struct monst *, int);
extern int m_move_aggress(struct monst *, coordxy, coordxy);
extern boolean closed_door(coordxy, coordxy);
extern boolean accessible(coordxy, coordxy);
extern void set_apparxy(struct monst *);
extern boolean can_ooze(struct monst *);
extern boolean can_fog(struct monst *);
extern boolean should_displace(struct monst *, coord *, long *, int, coordxy, coordxy);
extern boolean undesirable_disp(struct monst *, coordxy, coordxy);

/* ### monst.c ### */

extern void monst_init(void);

/* ### monstr.c ### */

extern void monstr_init(void);

/* ### mplayer.c ### */

extern struct monst *mk_mplayer(struct permonst *, coordxy, coordxy, boolean);
extern void create_mplayers(int, boolean);
extern void mplayer_talk(struct monst *);
extern const char* dev_name(void);

#if defined(MICRO) || defined(WIN32)

/* ### msdos.c,os2.c,tos.c,winnt.c ### */

#  ifndef WIN32
extern int tgetch(void);
#  endif
#  ifndef TOS
extern char switchar(void);
#  endif
# ifndef __GO32__
extern long freediskspace(char *);
#  ifdef MSDOS
extern int findfirst_file(char *);
extern int findnext_file(void);
extern long filesize_nh(char *);
#  else
extern int findfirst(char *);
extern int findnext(void);
extern long filesize(char *);
#  endif /* MSDOS */
extern char *foundfile_buffer(void);
# endif /* __GO32__ */
extern void chdrive(char *);
# ifndef TOS
extern void disable_ctrlP(void);
extern void enable_ctrlP(void);
# endif
# if defined(MICRO) && !defined(WINNT)
extern void get_scr_size(void);
#  ifndef TOS
extern void gotoxy(int, int);
#  endif
# endif
# ifdef TOS
extern int _copyfile(char *, char *);
extern int kbhit(void);
extern void set_colors(void);
extern void restore_colors(void);
#  ifdef SUSPEND
extern int dosuspend(void);
#  endif
# endif /* TOS */
# ifdef WIN32
extern char *get_username(int *);
extern void nt_regularize(char *);
extern int (*nt_kbhit)(void);
extern void Delay(int);
# endif /* WIN32 */
#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

extern int thitu(int, int, struct obj **, const char *);
extern int ohitmon(struct monst *, struct obj *, int, boolean);
extern void thrwmu(struct monst *);
extern int spitmu(struct monst *, struct attack *);
extern int breamu(struct monst *, struct attack *);
extern boolean linedup(coordxy, coordxy, coordxy, coordxy, int);
extern boolean lined_up(struct monst *);
extern struct obj *m_carrying(struct monst *, int);
extern int thrwmm(struct monst *, struct monst *);
extern int spitmm(struct monst *, struct attack *, struct monst *);
extern int breamm(struct monst *, struct attack *, struct monst *);
extern void m_useupall(struct monst *, struct obj *);
extern void m_useup(struct monst *, struct obj *);
extern void m_throw(struct monst *, coordxy, coordxy, int, int, int, struct obj *);
extern boolean hits_bars(struct obj **, int, int, int, int, int, int);
extern void hit_bars(struct obj **, int, int, int, int, boolean, boolean);
extern void dissolve_bars(coordxy, coordxy);

/* ### muse.c ### */

extern boolean find_defensive(struct monst *);
extern int use_defensive(struct monst *);
extern int rnd_defensive_item(struct monst *);
extern boolean find_offensive(struct monst *);
#ifdef USE_TRAMPOLI
extern int mbhitm(struct monst *, struct obj *);
#endif
extern int use_offensive(struct monst *);
extern int rnd_offensive_item(struct monst *);
extern boolean find_misc(struct monst *);
extern int use_misc(struct monst *);
extern int rnd_misc_item(struct monst *);
extern boolean searches_for_item(struct monst *, struct obj *);
extern boolean mon_reflects(struct monst *, const char *);
extern boolean ureflects(const char *, const char *);
extern void mcureblindness(struct monst *, boolean);
extern boolean munstone(struct monst *, boolean);
extern boolean munslime(struct monst *, boolean);

/* ### music.c ### */

extern void awaken_monsters(int);
extern void do_earthquake(int);
extern void awaken_soldiers(struct monst *);
extern int do_play_instrument(struct obj *);

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
extern void init_lan_features(void);
extern char *lan_username(void);
# ifdef LAN_MAIL
extern boolean lan_mail_check(void);
extern void lan_mail_read(struct obj *);
extern void lan_mail_init(void);
extern void lan_mail_finish(void);
extern void lan_mail_terminate(void);
# endif
#endif

/* ### nhregex.c ### */
extern struct nhregex *regex_init(void);
extern boolean regex_compile(const char *, struct nhregex *);
extern const char *regex_error_desc(struct nhregex *);
extern boolean regex_match(const char *, struct nhregex *);
extern void regex_free(struct nhregex *);

/* ### nttty.c ### */

#ifdef WIN32CON
extern void get_scr_size(void);
extern int nttty_kbhit(void);
extern void nttty_open(void);
extern void nttty_rubout(void);
extern int tgetch(void);
extern int ntposkey(int *, int *, int *);
extern void set_output_mode(int);
extern void synch_cursor(void);
#endif

/* ### o_init.c ### */

extern void init_objects(void);
extern int find_skates(void);
extern void oinit(void);
extern void savenames(int, int);
extern void restnames(int);
extern void discover_object(int, boolean, boolean);
extern void undiscover_object(int);
extern int dodiscovered(void);
extern void dragons_init(void);
extern void makeknown_msg(int);
extern int doclassdisco(void);
extern void rename_disco(void);

/* ### objects.c ### */

extern void objects_init(void);

/* ### objnam.c ### */

extern char *obj_typename(int);
extern char *simple_typename(int);
extern char *dump_typename(int);
extern char *safe_typename(int);
extern boolean obj_is_pname(struct obj *);
extern char *distant_name(struct obj *, char *(*)(OBJ_P));
extern char *fruitname(boolean);
extern struct fruit *fruit_from_indx(int);
extern char *xname(struct obj *);
extern char *mshot_xname(struct obj *);
extern boolean the_unique_obj(struct obj *obj);
extern boolean the_unique_pm(struct permonst *);
extern boolean erosion_matters(struct obj *);
extern long display_weight(struct obj *);
extern char *doname(struct obj *);
extern char *doname_with_price(struct obj *);
extern boolean not_fully_identified(struct obj *);
extern char *corpse_xname(struct obj *, const char *, unsigned);
extern char *cxname(struct obj *);
#ifdef SORTLOOT
extern char *cxname_singular(struct obj *);
#endif
extern char *cxname_unidentified(struct obj *);
extern char *killer_xname(struct obj *);
extern char *short_oname(struct obj *, char *(*)(OBJ_P), char *(*)(OBJ_P), unsigned);
extern const char *singular(struct obj *, char *(*)(OBJ_P));
extern char *an(const char *);
extern char *An(const char *);
extern char *The(const char *);
extern char *the(const char *);
extern char *aobjnam(struct obj *, const char *);
extern char *yobjnam(struct obj *, const char *);
extern char *Yobjnam2(struct obj *, const char *);
extern char *Tobjnam(struct obj *, const char *);
extern char *otense(struct obj *, const char *);
extern char *vtense(const char *, const char *);
extern char *Doname2(struct obj *);
extern char *yname(struct obj *);
extern char *Yname2(struct obj *);
extern char *ysimple_name(struct obj *);
extern char *Ysimple_name2(struct obj *);
extern char *simpleonames(struct obj *);
extern char *ansimpleoname(struct obj *);
extern char *thesimpleoname(struct obj *);
extern char *bare_artifactname(struct obj *);
extern char *makeplural(const char *);
extern char *makesingular(const char *);
extern short name_to_otyp(const char *);
extern struct obj *readobjnam(char *, struct obj *);
extern int rnd_class(int, int);
extern const char *suit_simple_name(struct obj *);
extern const char *cloak_simple_name(struct obj *);
extern const char *helm_simple_name(struct obj *);
extern const char *gloves_simple_name(struct obj *);
extern const char *mimic_obj_name(struct monst *);
extern char *safe_qbuf(char *, const char *, const char *, struct obj *,
                  char *(*)(OBJ_P), char *(*)(OBJ_P), const char *);
extern int shiny_obj(char);

/* ### options.c ### */

extern boolean match_optname(const char *, const char *, int, boolean);
extern void initoptions(void);
extern boolean parseoptions(char *, boolean, boolean);
extern boolean parse_monster_color(char *);
extern boolean parse_symbol(const char *);
extern boolean parse_monster_symbol(const char *);
extern boolean parse_object_symbol(const char *);
extern boolean parse_color_definition(const char *);
extern int doset(void);
extern int dotogglepickup(void);
extern void option_help(void);
extern void next_opt(winid, const char *);
extern int fruitadd(char *);
extern int choose_classes_menu(const char *, int, boolean, char *, char *);
extern boolean parsebindings(char *);
extern void add_menu_cmd_alias(char, char);
extern char map_menu_cmd(char);
extern void assign_warnings(uchar *);
extern char *nh_getenv(const char *);
extern void set_duplicate_opt_detection(int);
extern void set_wc_option_mod_status(unsigned long, int);
extern void set_wc2_option_mod_status(unsigned long, int);
extern void set_option_mod_status(const char *, int);
#ifdef AUTOPICKUP_EXCEPTIONS
extern int add_autopickup_exception(const char *);
extern void free_autopickup_exceptions(void);
#endif /* AUTOPICKUP_EXCEPTIONS */
#ifdef MENU_COLOR
extern boolean add_menu_coloring(char *);
#endif /* MENU_COLOR */
extern void free_menu_coloring(void);
extern const char * clr2colorname(int);
extern int query_color(const char *);
extern int query_attr(const char *);
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
extern boolean parse_status_color_options(char *);
extern void free_status_colors(void);
#endif

/* ### pager.c ### */

extern void mhidden_description(struct monst *, boolean, char *);
extern boolean object_from_map(int, coordxy, coordxy, struct obj **);
extern int do_screen_description(coord, boolean, glyph_t, char *, const char **, struct permonst **);
extern int do_look(int, coord *);
extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *dowhatdoes_core(char, char *);
extern int dohelp(void);
extern int dohistory(void);
extern void checkfile(struct obj *, char *, struct permonst *, boolean, boolean, char *);

/* ### pcmain.c ### */

#if defined(MICRO) || defined(WIN32)
# ifdef CHDIR
extern void chdirx(char *, boolean);
# endif /* CHDIR */
#endif /* MICRO || WIN32 */

/* ### pcsys.c ### */

#if defined(MICRO) || defined(WIN32)
extern void flushout(void);
extern int dosh(void);
# ifdef MFLOPPY
extern void eraseall(const char *, const char *);
extern void copybones(int);
extern void playwoRAMdisk(void);
extern int saveDiskPrompt(int);
extern void gameDiskPrompt(void);
# endif
extern void append_slash(char *);
extern void getreturn(const char *);
# ifndef AMIGA
extern void VDECL(msmsg, (const char *, ...));
# endif
extern FILE *fopenp(const char *, const char *);
#endif /* MICRO || WIN32 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
extern void VDECL(error, (const char *, ...));
#if defined(TIMED_DELAY) && defined(_MSC_VER)
extern void msleep(unsigned);
#endif
#endif /* MICRO || WIN32 */

/* ### pcunix.c ### */

#if defined(MICRO)
extern void regularize(char *);
#endif /* MICRO */
#if defined(PC_LOCKING)
extern void getlock(void);
#endif

/* ### pickup.c ### */

extern int get_most_recently_picked_up_turn(void);
extern int count_most_recently_picked_up(struct obj *);
extern struct obj *find_most_recently_picked_up_obj(struct obj *);
extern void observe_quantum_cat(struct obj *, boolean);
extern boolean container_gone(int (*)(OBJ_P));
extern boolean u_handsy(void);
extern void open_coffin(struct obj *, boolean);
extern int collect_obj_classes(char *, struct obj *, boolean, boolean(*)(struct obj *), int *);
extern boolean rider_corpse_revival(struct obj *, boolean);
extern boolean menu_class_present(int);
extern void add_valid_menu_class(int);
extern boolean allow_all(struct obj *);
extern boolean allow_category(struct obj *);
extern boolean is_worn_by_type(struct obj *);
#ifdef USE_TRAMPOLI
extern int ck_bag(struct obj *);
extern int in_container(struct obj *);
extern int out_container(struct obj *);
#endif
extern int pickup(int);
extern int pickup_object(struct obj *, long, boolean);
extern int query_category(const char *, struct obj *, int, menu_item **, int);
extern int query_objlist(const char *, struct obj *, int, menu_item **, int, boolean (*)(OBJ_P));
extern struct obj *pick_obj(struct obj *);
extern int encumber_msg(void);
extern int container_at(coordxy, coordxy, boolean);
extern int doloot(void);
extern int dotip(void);
extern int use_container(struct obj **, int, boolean);
extern int loot_mon(struct monst *, int *, boolean *);
extern boolean is_autopickup_exception(struct obj *, boolean);
extern boolean autopick_testobj(struct obj *, boolean);

/* ### pline.c ### */

extern void msgpline_add(int, char *);
extern void msgpline_free(void);
extern void VDECL(pline, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(custompline, (unsigned, const char *, ...)) PRINTF_F(2, 3);
extern void urgent_pline(const char *, ...) PRINTF_F(1, 2);
extern void VDECL(Norep, (const char *, ...)) PRINTF_F(1, 2);
extern void free_youbuf(void);
extern void VDECL(You, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(Your, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(You_feel, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(You_cant, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(You_hear, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(You_see, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(pline_The, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(There, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(verbalize, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(raw_printf, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(impossible, (const char *, ...)) PRINTF_F(1, 2);
extern void VDECL(warning, (const char *, ...)) PRINTF_F(1, 2);
extern const char *align_str(aligntyp);
extern void mstatusline(struct monst *);
extern void ustatusline(void);
extern void self_invis_message(void);

/* ### polyself.c ### */

extern void init_uasmon(void);
extern void set_uasmon(void);
extern void float_vs_flight(void);
extern void change_sex(void);
extern void polyself(boolean);
extern int polymon(int);
extern void rehumanize(void);
extern int dobreathe(void);
extern int dospit(void);
extern int doremove(void);
extern int dospinweb(void);
extern int dosummon(void);
extern int dogaze(void);
extern int dohide(void);
extern int domindblast(void);
extern void uunstick(void);
extern void skinback(boolean);
extern const char *mbodypart(struct monst *, int);
extern const char *body_part(int);
extern int poly_gender(void);
extern void ugolemeffects(int, int);
extern boolean is_playermon_genocided(void);
extern boolean ugenocided(void);
extern const char *udeadinside(void);

/* ### potion.c ### */

extern void set_itimeout(long *, long);
extern void incr_itimeout(long *, int);
extern void make_confused(long, boolean);
extern void make_stunned(long, boolean);
extern void make_blinded(long, boolean);
extern void make_sick(long, const char *, boolean, int);
extern void make_slimed(long, const char *);
extern void make_stoned(long, const char *, int, const char *);
extern void make_vomiting(long, boolean);
extern void toggle_blindness(void);
extern boolean make_hallucinated(long, boolean, long);
extern void make_glib(int);
extern int dodrink(void);
extern int dopotion(struct obj *);
extern int peffects(struct obj *);
extern void healup(int, int, boolean, boolean);
extern void strange_feeling(struct obj *, const char *);
extern void potionhit(struct monst *, struct obj *, int);
extern void potionbreathe(struct obj *);
extern boolean get_wet(struct obj *);
extern int dodip(void);
extern void mongrantswish(struct monst **);
extern void djinni_from_bottle(struct obj *);
extern struct monst *split_mon(struct monst *, struct monst *);
extern const char *bottlename(void);
extern boolean is_dark_mix_color(struct obj *);
extern boolean is_colorless_mix_potion(struct obj *);
extern const char* get_base_mix_color(struct obj *);
extern short mixtype(struct obj *, struct obj *);

/* ### pray.c ### */

#ifdef USE_TRAMPOLI
extern int prayer_done(void);
#endif
extern int dosacrifice(void);
extern boolean can_pray(boolean);
extern int dopray(void);
extern const char *u_gname(void);
extern int doturn(void);
extern int altarmask_at(coordxy, coordxy);
extern const char *a_gname(void);
extern const char *a_gname_at(coordxy x, coordxy y);
extern const char *align_gname(aligntyp);
extern const char *halu_gname(aligntyp);
extern const char *rnd_gname(int);
extern const char *align_gtitle(aligntyp);
extern void altar_wrath(coordxy, coordxy);
#ifdef ASTRAL_ESCAPE
extern int invoke_amulet(struct obj *);
#endif
extern int in_trouble(void);
extern void msg_luck_change(int change);


/* ### priest.c ### */

extern int move_special(struct monst *, boolean, schar, boolean, boolean,
                   coordxy, coordxy, coordxy, coordxy);
extern char temple_occupied(char *);
extern boolean inhistemple(struct monst *);
extern int pri_move(struct monst *);
extern void priestini(d_level *, struct mkroom *, int, int, boolean);
extern aligntyp mon_aligntyp(struct monst *);
extern char *priestname(struct monst *, char *);
extern boolean p_coaligned(struct monst *);
extern struct monst *findpriest(char);
extern void intemple(int);
extern void forget_temple_entry(struct monst *);
extern void priest_talk(struct monst *);
extern struct monst *mk_roamer(struct permonst *, aligntyp,
                          coordxy, coordxy, boolean);
extern void reset_hostility(struct monst *);
extern boolean in_your_sanctuary(struct monst *, coordxy, coordxy);
extern void ghod_hitsu(struct monst *);
extern void angry_priest(void);
extern void clearpriests(void);
extern void restpriest(struct monst *, boolean);
extern void newepri(struct monst *);
extern void free_epri(struct monst *);
extern char *piousness(boolean, const char *);

/* ### quest.c ### */

extern void onquest(void);
extern void nemdead(void);
extern void artitouch(void);
extern boolean ok_to_quest(void);
extern void leader_speaks(struct monst *);
extern void nemesis_speaks(void);
extern void quest_chat(struct monst *);
extern void quest_talk(struct monst *);
extern void quest_stat_check(struct monst *);
extern void finish_quest(struct obj *);

/* ### questpgr.c ### */

extern void load_qtlist(void);
extern void unload_qtlist(void);
extern short quest_info(int);
extern const char *ldrname(void);
extern boolean is_quest_artifact(struct obj*);
extern void com_pager(int);
extern void qt_pager(int);
extern char *string_subst(char *);
extern void qt_com_firstline(int, char*);
extern void deliver_splev_message(void);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
extern void srandom(unsigned);
extern char *initstate(unsigned, char *, int);
extern char *setstate(char *);
extern long random(void);
#endif /* RANDOM */

/* ### read.c ### */

extern void learn_scroll(struct obj *);
extern char *tshirt_text(struct obj *, char *);
extern char *hawaiian_motif(struct obj *, char *);
extern char *apron_text(struct obj *, char *);
extern int doread(void);
extern boolean is_chargeable(struct obj *);
extern void recharge(struct obj *, int);
extern void forget_objects(int);
extern void forget_levels(int);
extern void forget_traps(void);
extern void forget_map(int);
extern int seffects(struct obj *);
#ifdef USE_TRAMPOLI
extern void set_lit(int, int, genericptr_t);
#endif
extern void litroom(boolean, struct obj *);
extern void do_genocide(int, boolean);
extern void punish(struct obj *);
extern void unpunish(void);
extern boolean cant_revive(int *, boolean, struct obj *);
#ifdef WIZARD
extern boolean create_particular(void);
#endif
extern void drop_boulder_on_player(boolean, boolean, boolean, boolean);
extern int drop_boulder_on_monster(int, int, boolean, boolean);
extern boolean create_particular_from_buffer(const char*);

/* ### rect.c ### */

extern void init_rect(void);
extern NhRect *get_rect(NhRect *);
extern NhRect *rnd_rect(void);
extern void remove_rect(NhRect *);
extern void add_rect(NhRect *);
extern void split_rects(NhRect *, NhRect *);

/* ## region.c ### */
extern void clear_regions(void);
extern void run_regions(void);
extern boolean in_out_region(coordxy, coordxy);
extern boolean m_in_out_region(struct monst *, coordxy, coordxy);
extern void update_player_regions(void);
extern void update_monster_region(struct monst *);
extern NhRegion *visible_region_at(coordxy, coordxy);
extern void show_region(NhRegion*, coordxy, coordxy);
extern void save_regions(int, int);
extern void rest_regions(int, boolean);
extern NhRegion* create_gas_cloud(coordxy, coordxy, int, size_t, int);
extern NhRegion* create_cthulhu_death_cloud(coordxy, coordxy, int, size_t, int);
extern boolean region_danger(void);
extern void region_safety(void);

/* ### restore.c ### */

extern void inven_inuse(boolean);
extern int dorecover(int);
extern void restcemetery(int, struct cemetery **);
extern void trickery(char *);
extern void getlev(int, int, xint8, boolean);
extern void minit(void);
extern boolean lookup_id_mapping(unsigned, unsigned *);
#ifdef ZEROCOMP
extern int mread(int, genericptr_t, unsigned int);
#else
extern void mread(int, genericptr_t, unsigned int);
#endif

/* ### rip.c ### */

extern void genl_outrip(winid, int);

/* ### rnd.c ### */

extern int rn2(int);
extern int rnl(int);
extern int rnd(int);
extern int d(int, int);
extern int rne(int);
extern int rnz(int);
extern int rnf(int, int);

/* ### role.c ### */

extern boolean validrole(int);
extern boolean validrace(int, int);
extern boolean validgend(int, int, int);
extern boolean validalign(int, int, int);
extern int randrole(void);
extern int randrace(int);
extern int randgend(int, int);
extern int randalign(int, int);
extern int str2role(const char *);
extern int str2race(const char *);
extern int str2gend(const char *);
extern int str2align(const char *);
extern boolean ok_role(int, int, int, int);
extern int pick_role(int, int, int, int);
extern boolean ok_race(int, int, int, int);
extern int pick_race(int, int, int, int);
extern boolean ok_gend(int, int, int, int);
extern int pick_gend(int, int, int, int);
extern boolean ok_align(int, int, int, int);
extern int pick_align(int, int, int, int);
extern void role_selection_prolog(int, winid);
extern void role_menu_extra(int, winid, boolean);
extern void role_init(void);
extern void rigid_role_checks(void);
extern boolean setrolefilter(const char *);
extern boolean gotrolefilter(void);
extern void clearrolefilter(void);
extern void plnamesuffix(void);
extern const char *Hello(struct monst *);
extern const char *Goodbye(void);
extern char *build_plselection_prompt(char *, int, int, int, int, int);
extern char *root_plselection_prompt(char *, int, int, int, int, int);
extern void violated(int);
extern boolean successful_cdt(int);
extern boolean intended_cdt(int);
extern boolean superfluous_cdt(int);
extern boolean failed_cdt(int);

/* ### rumors.c ### */

extern char *getrumor(int, char *, boolean);
extern void outrumor(int, int);
extern void outoracle(boolean, boolean);
extern void save_oracles(int, int);
extern void restore_oracles(int);
extern int doconsult(struct monst *);

/* ### save.c ### */

extern int dosave(void);
#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
extern void hangup(int);
#endif
extern int dosave0(void);
#ifdef INSURANCE
extern void savestateinlock(void);
#endif
#ifdef MFLOPPY
extern boolean savelev(int, xint8, int);
extern boolean swapin_file(int);
extern void co_false(void);
#else
extern void savelev(int, xint8, int);
#endif
extern void bufon(int);
extern void bufoff(int);
extern void bflush(int);
extern void bwrite(int, genericptr_t, unsigned int);
extern void bclose(int);
extern void savecemetery(int, int, struct cemetery **);
extern void savefruitchn(int, int);
extern void free_dungeons(void);
extern void freedynamicdata(void);

/* ### shk.c ### */

extern void neweshk(struct monst *);
extern void free_eshk(struct monst *);
extern long money2mon(struct monst *, long);
extern void money2u(struct monst *, long);
extern char *Shknam(struct monst *);
extern char *shkname(struct monst *);
extern void shkgone(struct monst *);
extern void set_residency(struct monst *, boolean);
extern void replshk(struct monst *, struct monst *);
extern void restshk(struct monst *, boolean);
extern char inside_shop(coordxy, coordxy);
extern void u_left_shop(char *, boolean);
extern void remote_burglary(coordxy, coordxy);
extern void u_entered_shop(char *);
extern void pick_pick(struct obj *);
extern boolean same_price(struct obj *, struct obj *);
extern void shopper_financial_report(void);
extern int inhishop(struct monst *);
extern struct monst *shop_keeper(char);
extern boolean tended_shop(struct mkroom *);
extern boolean is_unpaid(struct obj *);
extern void delete_contents(struct obj *);
extern void obfree(struct obj *, struct obj *);
extern void home_shk(struct monst *, boolean);
extern void make_happy_shk(struct monst *, boolean);
extern void make_happy_shoppers(boolean);
extern void hot_pursuit(struct monst *);
extern void make_angry_shk(struct monst *, coordxy, coordxy);
extern int dopay(void);
extern boolean paybill(int, boolean);
extern void finish_paybill(void);
extern struct obj *find_oid(unsigned);
extern long contained_cost(struct obj *, struct monst *, long, boolean, boolean);
extern long contained_gold(struct obj *);
extern void picked_container(struct obj *);
extern void alter_cost(struct obj *, long);
extern long unpaid_cost(struct obj *, boolean);
extern boolean billable(struct monst **, struct obj *, char, boolean);
extern void addtobill(struct obj *, boolean, boolean, boolean);
extern void append_honorific(char *);
extern void splitbill(struct obj *, struct obj *);
extern void subfrombill(struct obj *, struct monst *);
extern long stolen_value(struct obj *, coordxy, coordxy, boolean, boolean);
extern void sellobj_state(int);
extern void sellobj(struct obj *, coordxy, coordxy);
extern int doinvbill(int);
extern struct monst *shkcatch(struct obj *, coordxy, coordxy);
extern void add_damage(coordxy, coordxy, long);
extern int repair_damage(struct monst *, struct damage *, boolean);
extern int shk_move(struct monst *);
extern void after_shk_move(struct monst *);
extern boolean is_fshk(struct monst *);
extern void shopdig(int);
extern void pay_for_damage(const char *, boolean);
extern boolean costly_spot(coordxy, coordxy);
extern struct obj *shop_object(coordxy, coordxy);
extern void price_quote(struct obj *);
extern void shk_chat(struct monst *);
extern void check_unpaid_usage(struct obj *, boolean);
extern void check_unpaid(struct obj *);
extern void costly_gold(coordxy, coordxy, long);
extern long get_cost_of_shop_item(struct obj *);
extern int oid_price_adjustment(struct obj *, unsigned);
extern boolean block_door(coordxy, coordxy);
extern boolean block_entry(coordxy, coordxy);
#ifdef BLACKMARKET
extern void blkmar_guards(struct monst *);
extern void set_black_marketeer_angry(void);
extern void bars_around_portal(boolean);
#endif /* BLACKMARKET */
extern char *shk_your(char *, struct obj *);
extern char *Shk_Your(char *, struct obj *);
extern void sanity_check_shopkeepers(void);

/* ### shknam.c ### */

extern void stock_room(int, struct mkroom *);
extern boolean saleable(struct monst *, struct obj *);
extern int get_shop_item(int);
extern void shop_selection_init(void);
extern boolean is_izchak(struct monst *, boolean);

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern void attrcurse(void);

/* ### sounds.c ### */

extern void dosounds(void);
extern const char *growl_sound(struct monst *);
extern void growl(struct monst *);
extern void yelp(struct monst *);
extern void whimper(struct monst *);
extern void beg(struct monst *);
extern int dotalk(void);
#ifdef USER_SOUNDS
extern int add_sound_mapping(const char *);
extern void play_sound_for_message(const char *);
#endif

/* ### sys/msdos/sound.c ### */

#ifdef MSDOS
extern int assign_soundcard(char *);
#endif

/* ### sp_lev.c ### */

extern boolean check_room(coordxy *, coordxy *, coordxy *, coordxy *, boolean);
extern boolean create_room(coordxy, coordxy, coordxy, coordxy,
                      coordxy, coordxy, xint16, xint16);
extern void create_secret_door(struct mkroom *, coordxy);
extern boolean dig_corridor(coord *, coord *, boolean, schar, schar);
extern void fill_room(struct mkroom *, boolean);
extern boolean load_special(const char *);
extern coordxy selection_getpoint(coordxy, coordxy, struct opvar *);
extern struct opvar *selection_opvar(char *);
extern void opvar_free_x(struct opvar *);
extern void set_selection_floodfillchk(int(*)(coordxy, coordxy));
extern void selection_floodfill(struct opvar *, coordxy, coordxy, boolean);

/* ### spell.c ### */

extern void book_cursed(struct obj *);
#ifdef USE_TRAMPOLI
extern int learn(void);
#endif
extern int study_book(struct obj *);
extern void book_disappears(struct obj *);
extern void book_substitution(struct obj *, struct obj *);
extern void age_spells(void);
extern int docast(void);
extern int spell_skilltype(int);
extern int spelleffects(int, boolean);
extern int tport_spell(int);
extern void losespells(void);
extern int dovspell(void);
extern void initialspell(struct obj *);
extern void dump_spells(void);
extern boolean parse_spellorder(char *);
extern const char* spelltypemnemonic(int);
extern int num_spells(void);

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
extern int stealarm(void);
#endif
extern long somegold(long);
extern void stealgold(struct monst *);
extern void remove_worn_item(struct obj *, boolean);
extern int steal(struct monst *, char *);
extern int mpickobj(struct monst *, struct obj *);
extern void stealamulet(struct monst *);
extern void maybe_absorb_item(struct monst *, struct obj *, int, int);
extern void mdrop_obj(struct monst *, struct obj *, boolean);
extern void mdrop_special_objs(struct monst *);
extern void relobj(struct monst *, int, boolean);
extern struct obj *findgold(struct obj *);

/* ### steed.c ### */

extern void rider_cant_reach(void);
extern boolean can_saddle(struct monst *);
extern int use_saddle(struct obj *);
extern void put_saddle_on_mon(struct obj *, struct monst *);
extern boolean can_ride(struct monst *);
extern int doride(void);
extern boolean mount_steed(struct monst *, boolean);
extern void exercise_steed(void);
extern void kick_steed(void);
extern void dismount_steed(int);
extern void place_monster(struct monst *, coordxy, coordxy);
extern boolean stucksteed(boolean);

/* ### sys.c ### */

extern void sys_early_init(void);

/* ### teleport.c ### */

extern boolean goodpos(coordxy, coordxy, struct monst *, unsigned);
extern boolean enexto(coord *, coordxy, coordxy, struct permonst *);
extern boolean enexto_core(coord *, coordxy, coordxy, struct permonst *, unsigned);
extern boolean enexto_core_range(coord *, coordxy, coordxy, struct permonst *, unsigned, int);
extern int epathto(coord *, int, coordxy, coordxy, struct permonst *);
extern void teleds(coordxy, coordxy, boolean);
extern boolean safe_teleds(boolean);
extern boolean teleport_pet(struct monst *, boolean);
extern int dotelecmd(void);
extern void tele(void);
extern int dotele(boolean);
extern void level_tele(void);
extern void domagicportal(struct trap *);
extern void tele_trap(struct trap *);
extern void level_tele_trap(struct trap *, unsigned);
extern void rloc_to(struct monst *, coordxy, coordxy);
extern boolean rloc(struct monst *, boolean);
extern boolean tele_restrict(struct monst *);
extern void mtele_trap(struct monst *, struct trap *, int);
extern int mlevel_tele_trap(struct monst *, struct trap *, boolean, int);
extern boolean rloco(struct obj *);
extern int random_teleport_level(void);
extern boolean u_teleport_mon(struct monst *, boolean);

/* ### termcap.c ### */
extern void term_start_color(int);
extern void term_end_color(void);
extern void term_start_attr(int);
extern void term_end_attr(int);

/* ### tile.c ### */
#ifdef USE_TILES
extern void substitute_tiles(d_level *);
#endif

/* ### timeout.c ### */

extern void burn_away_slime(void);
extern void nh_timeout(void);
extern void fall_asleep(int, boolean);
extern void attach_egg_hatch_timeout(struct obj *, long);
extern void attach_fig_transform_timeout(struct obj *);
extern void kill_egg(struct obj *);
extern void hatch_egg(ANY_P *, long);
extern void learn_egg_type(int);
extern void burn_object(ANY_P *, long);
extern void begin_burn(struct obj *, boolean);
extern void end_burn(struct obj *, boolean);
extern void do_storms(void);
extern boolean start_timer(long, short, short, ANY_P *);
extern long stop_timer(short, ANY_P *);
extern long peek_timer(short, ANY_P *);
extern void run_timers(void);
extern void obj_move_timers(struct obj *, struct obj *);
extern void obj_split_timers(struct obj *, struct obj *);
extern void obj_stop_timers(struct obj *);
extern boolean obj_has_timer(struct obj *, short);
extern void spot_stop_timers(coordxy, coordxy, short);
extern long spot_time_expires(coordxy, coordxy, short);
extern long spot_time_left(coordxy, coordxy, short);
extern boolean obj_is_local(struct obj *);
extern void save_timers(int, int, int);
extern void restore_timers(int, int, boolean, long);
extern void timer_stats(const char *, char *, long *, long *);
extern void relink_timers(boolean);
#ifdef WIZARD
extern int wiz_timeout_queue(void);
extern void timer_sanity_check(void);
#endif

/* ### topten.c ### */

#ifdef RECORD_CONDUCT
extern long encodeconduct(void);
#endif
extern void topten(int);
extern void prscore(int, char **);
extern struct obj *tt_oname(struct obj *);

/* ### track.c ### */

extern void initrack(void);
extern void settrack(void);
extern coord *gettrack(coordxy, coordxy);

/* ### trap.c ### */

extern boolean burnarmor(struct monst *);
extern int erode_obj(struct obj *, const char *, int, int);
extern boolean grease_protect(struct obj *, const char *, struct monst *);
extern struct trap *maketrap(coordxy, coordxy, int);
extern void fall_through(boolean, unsigned);
extern struct monst *animate_statue(struct obj *, coordxy, coordxy, int, int *);
extern struct monst *activate_statue_trap(struct trap *, coordxy, coordxy, boolean);
extern void set_utrap(unsigned, unsigned);
extern void reset_utrap(boolean);
extern void dotrap(struct trap *, unsigned);
extern void seetrap(struct trap *);
extern void feeltrap(struct trap *);
extern int mintrap(struct monst *, unsigned);
extern void instapetrify(const char *);
extern void minstapetrify(struct monst *, boolean);
#ifdef WEBB_DISINT
extern int instadisintegrate(const char *);
extern int minstadisintegrate(struct monst *);
#endif
extern void selftouch(const char *);
extern void mselftouch(struct monst *, const char *, boolean);
extern void float_up(void);
extern void fill_pit(coordxy, coordxy);
extern int float_down(long, long);
extern void climb_pit(void);
extern boolean fire_damage(struct obj *, boolean, coordxy, coordxy);
extern int fire_damage_chain(struct obj *, boolean, boolean, coordxy, coordxy);
extern boolean lava_damage(struct obj *, coordxy, coordxy);
extern void acid_damage(struct obj *);
extern int water_damage(struct obj *, const char *, boolean);
extern void water_damage_chain(struct obj *, boolean);
extern boolean drown(void);
extern void drain_en(int);
extern int dountrap(void);
extern void cnv_trap_obj(int, int, struct trap *, boolean);
extern int untrap(boolean);
extern boolean openholdingtrap(struct monst *, boolean *);
extern boolean closeholdingtrap(struct monst *, boolean *);
extern boolean openfallingtrap(struct monst *, boolean, boolean *);
extern boolean chest_trap(struct obj *, int, boolean);
extern void deltrap(struct trap *);
extern boolean delfloortrap(struct trap *);
extern struct trap *t_at(coordxy, coordxy);
extern void b_trapped(const char *, int);
extern boolean unconscious(void);
extern boolean uteetering_at_seen_pit(struct trap *);
extern boolean uescaped_shaft(struct trap *);
extern boolean lava_effects(void);
extern boolean swamp_effects(void);
extern void blow_up_landmine(struct trap *);
extern int launch_obj(short, coordxy, coordxy, coordxy, coordxy, int);

/* ### u_init.c ### */

extern void u_init(void);

/* ### uhitm.c ### */

extern void hurtmarmor(struct monst *, int);
extern boolean attack_checks(struct monst *, struct obj *);
extern void check_caitiff(struct monst *);
extern int find_roll_to_hit(struct monst *, uchar, struct obj *, int *, int *);
extern boolean attack(struct monst *);
extern boolean hmon(struct monst *, struct obj *, int, int);
extern boolean shade_miss(struct monst *, struct monst *, struct obj *, boolean, boolean);
extern int damageum(struct monst *, struct attack *, int);
extern void missum(struct monst *, struct attack *, boolean);
extern int passive(struct monst *, struct obj *, boolean, int, uchar, boolean);
extern void passive_obj(struct monst *, struct obj *, struct attack *);
extern void stumble_onto_mimic(struct monst *);
extern int flash_hits_mon(struct monst *, struct obj *);
extern void light_hits_gremlin(struct monst *, int);

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
extern void port_help(void);
# endif
#endif /* UNIX */


/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void VDECL(error, (const char *, ...)) PRINTF_F(1, 2);
#endif /* UNIX || __BEOS__ */

/* ### unixunix.c ### */

#ifdef UNIX
extern void getlock(void);
extern void regularize(char *);
# if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
extern void msleep(unsigned);
# endif
# ifdef SHELL
extern int dosh(void);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
extern int child(int);
# endif
#ifdef FILE_AREAS
extern char *make_file_name(const char *, const char *);
extern FILE *fopen_datafile_area(const char *, const char *, const char *, boolean);
extern FILE *freopen_area(const char *, const char *, const char *, FILE *);
extern int chmod_area(const char *, const char *, int);
extern int open_area(const char *, const char *, int, int);
extern int creat_area(const char *, const char *, int);
extern boolean lock_file_area(const char *, const char *, int);
extern void unlock_file_area(const char *, const char *);
extern int rename_area(const char *, const char *, const char *);
extern int remove_area(const char *, const char *);
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS
extern int hide_privileges(boolean);
# endif
#endif /* UNIX */

/* ### vault.c ### */

extern void newegd(struct monst *);
extern void free_egd(struct monst *);
extern boolean grddead(struct monst *);
extern struct monst *findgd(void);
extern void vault_summon_gd(void);
extern char vault_occupied(char *);
extern void uleftvault(struct monst *);
extern void invault(void);
extern int gd_move(struct monst *);
extern void paygd(boolean);
extern long hidden_gold(void);
extern boolean gd_sound(void);
extern void vault_gd_watching(unsigned int);

/* ### version.c ### */

extern char *version_string(char *);
extern char *getversionstring(char *);
extern int doversion(void);
extern int doextversion(void);
#ifdef MICRO
extern boolean comp_times(long);
#endif
extern boolean check_version(struct version_info *, const char *, boolean);
extern unsigned long get_feature_notice_ver(char *);
extern unsigned long get_current_feature_ver(void);
#ifdef RUNTIME_PORT_ID
extern void append_port_id(char *);
#endif

/* ### video.c ### */

#ifdef MSDOS
extern int assign_video(char *);
# ifdef NO_TERMS
extern void gr_init(void);
extern void gr_finish(void);
# endif
extern void tileview(boolean);
#endif
#ifdef VIDEOSHADES
extern int assign_videoshades(char *);
extern int assign_videocolors(char *);
#endif

/* ### vis_tab.c ### */

#ifdef VISION_TABLES
extern void vis_tab_init(void);
#endif

/* ### vision.c ### */

extern void vision_init(void);
extern int does_block(coordxy, coordxy, struct rm*);
extern void vision_reset(void);
extern void vision_recalc(int);
extern void block_point(coordxy, coordxy);
extern void unblock_point(coordxy, coordxy);
extern boolean clear_path(int, int, int, int);
extern void do_clear_area(coordxy, coordxy, int, void (*)(coordxy, coordxy, genericptr_t), genericptr_t);
extern unsigned how_mon_is_seen(struct monst *);

#ifdef VMS

/* ### vmsfiles.c ### */

extern int vms_link(const char *, const char *);
extern int vms_unlink(const char *);
extern int vms_creat(const char *, unsigned int);
extern int vms_open(const char *, int, unsigned int);
extern boolean same_dir(const char *, const char *);
extern int c__translate(int);
extern char *vms_basename(const char *);

/* ### vmsmail.c ### */

extern unsigned long init_broadcast_trapping(void);
extern unsigned long enable_broadcast_trapping(void);
extern unsigned long disable_broadcast_trapping(void);
# if 0
extern struct mail_info *parse_next_broadcast(void);
# endif /*0*/

/* ### vmsmain.c ### */

extern int main(int, char **);
# ifdef CHDIR
extern void chdirx(const char *, boolean);
# endif /* CHDIR */

/* ### vmsmisc.c ### */

extern void vms_abort(void);
extern void vms_exit(int);

/* ### vmstty.c ### */

extern int vms_getchar(void);
extern void gettty(void);
extern void settty(const char *);
extern void shuttty(const char *);
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void VDECL(error, (const char *, ...)) PRINTF_F(1, 2);
#ifdef TIMED_DELAY
extern void msleep(unsigned);
#endif

/* ### vmsunix.c ### */

extern void getlock(void);
extern void regularize(char *);
extern int vms_getuid(void);
extern boolean file_is_stmlf(int);
extern int vms_define(const char *, const char *, int);
extern int vms_putenv(const char *);
extern char *verify_termcap(void);
# if defined(CHDIR) || defined(SHELL) || defined(SECURE)
extern void privoff(void);
extern void privon(void);
# endif
# ifdef SHELL
extern int dosh(void);
# endif
# if defined(SHELL) || defined(MAIL)
extern int vms_doshell(const char *, boolean);
# endif
# ifdef SUSPEND
extern int dosuspend(void);
# endif

#endif /* VMS */

/* ### weapon.c ### */

extern const char *weapon_descr(struct obj *);
extern int hitval(struct obj *, struct monst *);
extern int dmgval(struct obj *, struct monst *);
extern int special_dmgval(struct monst *, struct monst *, long, long *);
extern struct damage_info_t dmgval_info(struct obj*);
extern void silver_sears(struct monst *, struct monst *, long);
extern struct obj *select_rwep(struct monst *);
extern boolean mon_might_throw_wep(struct obj *);
extern struct obj *select_hwep(struct monst *);
extern void possibly_unwield(struct monst *, boolean);
extern int mon_wield_item(struct monst *);
extern void mwepgone(struct monst *);
extern int abon(void);
extern int dbon(void);
extern void wet_a_towel(struct obj *, int, boolean);
extern void dry_a_towel(struct obj *, int, boolean);
extern const char* skill_name(int);
extern int enhance_weapon_skill(void);
#ifdef DUMP_LOG
extern void dump_weapon_skill(void);
#endif
extern void unrestrict_weapon_skill(int);
extern void use_skill(int, int);
extern void add_weapon_skill(int);
extern void lose_weapon_skill(int);
extern int weapon_type(struct obj *);
extern int uwep_skill_type(void);
extern int weapon_hit_bonus(struct obj *);
extern int weapon_dam_bonus(struct obj *);
extern void skill_init(const struct def_skill *);
extern boolean can_advance_something(void);

/* ### were.c ### */

extern void were_change(struct monst *);
extern int counter_were(int);
extern int were_beastie(int);
extern void new_were(struct monst *);
extern int were_summon(struct permonst *, boolean, int *, char *);
extern void you_were(void);
extern void you_unwere(boolean);
extern void set_ulycn(int);

/* ### wield.c ### */

extern void setuwep(struct obj *);
extern void setuqwep(struct obj *);
extern void setuswapwep(struct obj *);
extern int dowield(void);
extern int doswapweapon(void);
extern int dowieldquiver(void);
extern int dounwield(void);
extern boolean wield_tool(struct obj *, const char *);
extern int can_twoweapon(void);
extern void drop_uswapwep(void);
extern int dotwoweapon(void);
extern void uwepgone(void);
extern void uswapwepgone(void);
extern void uqwepgone(void);
extern void untwoweapon(void);
extern int chwepon(struct obj *, int);
extern int welded(struct obj *);
extern void weldmsg(struct obj *);
extern void setmnotwielded(struct monst *, struct obj *);
extern boolean mwelded(struct obj *);
extern void unwield_weapons_silently(void);

/* ### windows.c ### */

extern void choose_windows(const char *);
extern char genl_message_menu(char, int, const char *);
extern void genl_preference_update(const char *);

/* ### wizard.c ### */

extern void amulet(void);
extern int mon_has_amulet(struct monst *);
extern int mon_has_special(struct monst *);
extern int tactics(struct monst *);
extern boolean has_aggravatables(struct monst *);
extern void aggravate(void);
extern void clonewiz(void);
extern int pick_nasty(void);
extern int nasty(struct monst*);
extern void resurrect(void);
extern void intervene(void);
extern void wizdead(void);
extern void cuss(struct monst *);

/* ### worm.c ### */

extern int get_wormno(void);
extern void initworm(struct monst *, int);
extern void worm_move(struct monst *);
extern void worm_nomove(struct monst *);
extern void wormgone(struct monst *);
extern void wormhitu(struct monst *);
extern void cutworm(struct monst *, coordxy, coordxy, struct obj *);
extern void see_wsegs(struct monst *);
extern void detect_wsegs(struct monst *, boolean);
extern void save_worm(int, int);
extern void rest_worm(int);
extern void place_wsegs(struct monst *, struct monst *);
extern void sanity_check_worm(struct monst *);
extern void remove_worm(struct monst *);
extern void place_worm_tail_randomly(struct monst *, coordxy, coordxy);
extern int size_wseg(struct monst *);
extern int count_wsegs(struct monst *);
extern boolean worm_known(struct monst *);
extern int wseg_at(struct monst *, coordxy, coordxy);
extern void flip_worm_segs_vertical(struct monst *, int, int);
extern void flip_worm_segs_horizontal(struct monst *, int, int);

/* ### worn.c ### */

extern void setworn(struct obj *, long);
extern void setnotworn(struct obj *);
extern struct obj *wearmask_to_obj(long);
extern long wearslot(struct obj *);
extern void mon_set_minvis(struct monst *);
extern void mon_adjust_speed(struct monst *, int, struct obj *);
extern void update_mon_intrinsics(struct monst *, struct obj *, boolean, boolean);
extern int find_mac(struct monst *);
extern void m_dowear(struct monst *, boolean);
extern struct obj *which_armor(struct monst *, long);
extern void mon_break_armor(struct monst *, boolean);
extern void bypass_obj(struct obj *);
extern void clear_bypasses(void);
extern void bypass_objlist(struct obj *, boolean);
extern struct obj *nxt_unbypassed_obj(struct obj *);
extern int racial_exception(struct monst *, struct obj *);

/* ### write.c ### */

extern int ink_cost(struct obj *);
extern int dowrite(struct obj *);

/* ### zap.c ### */

extern void learn_wand(struct obj *);
extern int bhitm(struct monst *, struct obj *);
extern void probe_monster(struct monst *);
extern boolean get_obj_location(struct obj *, coordxy *, coordxy *, int);
extern boolean get_mon_location(struct monst *, coordxy *, coordxy *, int);
extern struct monst *get_container_location(struct obj *obj, int *, int *);
extern struct monst *montraits(struct obj *, coord *, boolean);
extern struct monst *revive(struct obj *, boolean);
extern int unturn_dead(struct monst *);
extern void cancel_item(struct obj *);
extern boolean drain_item(struct obj *, boolean);
extern struct obj *poly_obj(struct obj *, int);
extern boolean obj_resists(struct obj *, int, int);
extern boolean obj_shudders(struct obj *);
extern void do_osshock(struct obj *);
extern int bhito(struct obj *, struct obj *);
extern int bhitpile(struct obj *, int (*)(OBJ_P, OBJ_P), int, int, schar);
extern int zappable(struct obj *);
extern void zapnodir(struct obj *);
extern int dozap(void);
extern int zapyourself(struct obj *, boolean);
extern void ubreatheu(struct attack *);
extern int lightdamage(struct obj *, boolean, int);
extern boolean flashburn(long);
extern boolean cancel_monst(struct monst *, struct obj *, boolean, boolean, boolean);
extern void zapsetup(void);
extern void zapwrapup(void);
extern void weffects(struct obj *);
extern int spell_damage_bonus(int);
extern const char *exclam(int force);
extern void hit(const char *, struct monst *, const char *);
extern void miss(const char *, struct monst *);
extern struct monst *bhit(int, int, int, int, int (*)(MONST_P, OBJ_P),
                     int (*)(OBJ_P, OBJ_P), struct obj *, boolean *);
extern struct monst *boomhit(struct obj *, int, int);
extern int zhitm(struct monst *, int, int, struct obj **);
extern int burn_floor_objects(coordxy, coordxy, boolean, boolean);
extern void buzz(int, int, coordxy, coordxy, int, int);
extern void dobuzz(int, int, coordxy, coordxy, int, int, boolean);
extern void melt_ice(coordxy, coordxy, const char *);
extern void melt_ice_away(ANY_P *, long);
extern int zap_over_floor(coordxy, coordxy, int, boolean *, short);
extern void fracture_rock(struct obj *);
extern boolean break_statue(struct obj *);
extern void destroy_item(int, int);
extern int destroy_mitem(struct monst *, int, int);
extern int resist(struct monst *, char, int, int);
extern void makewish(boolean);
extern void remove_corpse(struct obj *);

/* ### livelog.c ### */
#ifdef LIVELOGFILE
extern boolean livelog_start(void);
extern void livelog_achieve_update(void);
extern void livelog_wish(char*);
# ifdef LIVELOG_SHOUT
extern int doshout(void);
# endif
# ifdef LIVELOG_BONES_KILLER
extern void livelog_bones_killed(struct monst *);
# endif
extern void livelog_shoplifting(const char*, const char*, long);
#endif
extern void livelog_game_started(const char*, const char*, const char*, const char*);
extern void livelog_game_action(const char*);
extern void livelog_generic(const char*, const char*);
extern void livelog_genocide(const char*, int);
extern void livelog_printf(unsigned int, const char *, ...) PRINTF_F(2, 3);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

/* ### tutorial.c ### */
extern boolean check_tutorial_message(int);
extern void maybe_tutorial(void);
extern int tutorial_redisplay(void);
extern void tutorial_redisplay_message(void);
extern void check_tutorial_farlook(coordxy, coordxy);
extern void check_tutorial_command(char);
extern int check_tutorial_location(coordxy, coordxy, boolean);
extern int check_tutorial_oclass(int);

/* ### base32.c ### */
long decode_base32(const char*);
const char* encode_base32(unsigned long);

/* ### unicode.c ### */
extern glyph_t get_unicode_codepoint(int);
extern int pututf8char(glyph_t);

#undef E

#endif /* EXTERN_H */
