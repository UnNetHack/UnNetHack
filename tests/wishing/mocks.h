
#define NEARDATA

#include "objects.c"
#include "monst.c"

struct window_procs windowprocs;

#include "hacklib.c"
#include "decl.c"
#include "drawing.c"
#include "options.c"
#include "rnd.c"
#include "alloc.c"
#include "mkobj.c"
#include "mondata.c"

#include "objnam.c"

#include <stdarg.h>

void raw_printf(const char *str,...) {}
void impossible(const char *str,...) {}
void warning(const char *str,...) {}

unsigned long get_feature_notice_ver(char *str) { return 0; }
unsigned long get_current_feature_ver() { return 0; }

void pline(const char *str,...) {}
void pline_The(const char *str,...) {}
void You_cant(const char *str,...) {}
void panic(const char *str,...) {}
void terminate(int status) {}

int str2role(char *str) { return 0; }
int str2race(char *str) { return 0; }
int str2gend(char *str) { return 0; }
int str2align(char *str) { return 0; }

void read_config_file(const char *str) {}
const char* configfile = "";

const struct Align aligns[] = {};
const struct Gender genders[] = {};
const struct Role roles[] = {};
const struct Race races[] = {};
struct Role urole;
struct Race urace;

struct obj *thrownobj = 0;

int doredraw() { return 0; }
boolean on_level(d_level *a, d_level *b) { return FALSE; }

void vision_recalc(int a) {}
void choose_windows(const char *str) {}
void reassign() {}

boolean Can_fall_thru(d_level *l) { return FALSE; }
const char *align_str(ALIGNTYP_P alignment) { return "unknown"; }

const char* artifact_name(const char *name, short *typ) { return NULL; }
void attach_egg_hatch_timeout(struct obj *obj) {}
void block_point(int x, int y) {}
void unblock_point(int x, int y) {}
const char* body_part(int typ) { return "body_part"; }
int can_be_hatched(int typ) { return typ; }
void consume_oeaten(struct obj *obj, int amount) {}
boolean dead_species(int typ, BOOLEAN_P bool) { return FALSE; }
void del_engr_at(int x, int y) {}
void delete_contents(struct obj *obj) {}
int genus(int typ, int number) { return 0; }
boolean is_quest_artifact(struct obj *obj) { return FALSE; }
void kill_egg(struct obj *obj) {}
void make_grave(int x, int y, const char *engraving) {}
struct trap* maketrap(int x, int y, int typ) { return NULL; }
void newsym(int x, int y) {}
int title_to_mon(const char *str, int* number1, int* number2) { return 0; }

void obfree(struct obj *obj1, struct obj *obj2) {}
struct obj* oname(struct obj *obj, const char *str) { return NULL; }
boolean water_damage(struct obj * obj, BOOLEAN_P bool1, BOOLEAN_P bool2) { return FALSE; }
void reset_remarm() {}
struct permonst* rndmonst() { return NULL; }

boolean worm_known(struct monst *monst) { return FALSE; }
struct obj* which_armor(struct monst *monst, long number) { return NULL; }
int mon_has_amulet(struct monst *monst) { return 0; }



/* ### artifact.c ### */
//E void init_artifacts(void);
//E void save_artifacts(int);
//E void restore_artifacts(int);
//E const char *artiname(int);
struct obj* mk_artifact(struct obj *obj, ALIGNTYP_P typ) { return NULL; }
//E const char *artifact_name(const char *,short *);
boolean exist_artifact(int typ, const char *str) { return FALSE; }
void artifact_exists(struct obj *obj, const char *str, BOOLEAN_P bool) {}
int nartifact_exist() { return 0; }
//E boolean spec_ability(struct obj *,unsigned long);
boolean confers_luck(struct obj *obj) { return FALSE; }
//E boolean arti_reflects(struct obj *);
//E boolean restrict_name(struct obj *,const char *,BOOLEAN_P);
boolean defends(int number1, struct obj *obj) { return FALSE; }
boolean protects(int number2, struct obj *obj) { return FALSE; }
//E void set_artifact_intrinsic(struct obj *,BOOLEAN_P,long);
//E int touch_artifact(struct obj *,struct monst *);
//E int spec_abon(struct obj *,struct monst *);
//E int spec_dbon(struct obj *,struct monst *,int);
//E void discover_artifact(XCHAR_P);
boolean undiscovered_artifact(XCHAR_P typ) { return FALSE; }
//E int disp_artifact_discoveries(winid);
//E boolean artifact_hit(struct monst *,struct monst *,
//                       struct obj *,int *,int);
//E int doinvoke(void);
//E void arti_speak(struct obj *);
//E boolean artifact_light(struct obj *);
//E long spec_m2(struct obj *);
//E boolean artifact_has_invprop(struct obj *,UCHAR_P);
//E long arti_cost(struct obj *);
boolean MATCH_WARN_OF_MON(struct monst *monst) { return FALSE; }
//E const char *get_warned_of_monster(struct obj *);


/* ### attrib.c ### */
//E boolean adjattrib(int,int,int);
//E void change_luck(SCHAR_P);
//E int stone_luck(BOOLEAN_P);
void set_moreluck() {}
//E void gainstr(struct obj *,int);
//E void losestr(int);
//E void restore_attrib(void);
//E void exercise(int,BOOLEAN_P);
//E void exerchk(void);
//E void reset_attribute_clock(void);
//E void init_attr(int);
//E void redist_attr(void);
//E void adjabil(int,int);
//E int newhp(void);
//E schar acurr(int);
//E schar acurrstr(void);
//E void adjalign(int);
//E const char * beautiful(void);
//E int uhp(void);
//E int uhpmax(void);


/* ### dbridge.c ### */
//E boolean is_pool(int,int);
//E boolean is_lava(int,int);
boolean is_ice(int x, int y) { return FALSE; }
//E boolean is_swamp(int,int);
//E boolean is_icewall(int,int);
//E boolean is_any_icewall(int,int);
//E int is_drawbridge_wall(int,int);
//E boolean is_db_wall(int,int);
//E boolean find_drawbridge(int *,int*);
//E boolean create_drawbridge(int,int,int,int);
//E void open_drawbridge(int,int);
//E boolean close_drawbridge(int,int);
//E void destroy_drawbridge(int,int);

/* ### do.c ### */
//#ifdef USE_TRAMPOLI
//E int drop(struct obj *);
//E int wipeoff(void);
//#endif
//E int dodrop(void);
//E boolean boulder_hits_pool(struct obj *,int,int,BOOLEAN_P);
//E boolean flooreffects(struct obj *,int,int,const char *);
//E void doaltarobj(struct obj *);
//E boolean canletgo(struct obj *,const char *);
//E void dropx(struct obj *);
//E void dropy(struct obj *);
void obj_no_longer_held(struct obj *obj) {}
//E int doddrop(void);
//E int dodown(void);
//E int doup(void);
//#ifdef INSURANCE
//E void save_currentstate(void);
//#endif
//E void goto_level(d_level *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
//E void schedule_goto(d_level *,BOOLEAN_P,BOOLEAN_P,int,
//                     const char *,const char *));
//E void deferred_goto(void);
//E boolean revive_corpse(struct obj *);
//E void revive_mon(genericptr_t, long);
//E int donull(void);
//E int dowipe(void);
//E void set_wounded_legs(long,int);
//E void heal_legs(void);

/* ### do_name.c ### */
//E void do_oname(struct obj *);
//E int getpos(coord *,BOOLEAN_P,const char *);
//E struct monst *christen_monst(struct monst *,const char *);
//E int do_mname(void);
//E struct obj *oname(struct obj *,const char *);
//E int ddocall(void);
//E void docall(struct obj *);
//E const char *rndghostname(void);
//E char *x_monnam(struct monst *,int,const char *,int,BOOLEAN_P);
//E char *l_monnam(struct monst *);
char* mon_nam(struct monst *monst) { return "mon_nam"; }
//E char *noit_mon_nam(struct monst *);
//E char *Monnam(struct monst *);
//E char *noit_Monnam(struct monst *);
//E char *m_monnam(struct monst *);
//E char *y_monnam(struct monst *);
//E char *Adjmonnam(struct monst *,const char *);
//E char *Amonnam(struct monst *);
//E char *a_monnam(struct monst *);
//E char *distant_monnam(struct monst *,int,char *);
//E const char *rndmonnam(void);
//E const char *hcolor(const char *);
//E const char *rndcolor(void);
//#ifdef REINCARNATION
//E const char *roguename(void);
//#endif
struct obj* realloc_obj(struct obj *obj, int number1, genericptr_t generic, int number2, const char *str) { return NULL; }
//E char *coyotename(struct monst *,char *);

/* ### dungeon.c ### */
//E void save_dungeon(int,BOOLEAN_P,BOOLEAN_P);
//E void restore_dungeon(int);
//E void insert_branch(branch *,BOOLEAN_P);
//E void init_dungeons(void);
//E s_level *find_level(const char *);
//E s_level *Is_special(d_level *);
//#ifdef RANDOMIZED_PLANES
//E s_level *get_next_elemental_plane(d_level *);
//E d_level *get_first_elemental_plane(void);
//#endif
//E branch *Is_branchlev(d_level *);
//E xchar ledger_no(d_level *);
//E xchar maxledgerno(void);
//E schar depth(d_level *);
//E xchar dunlev(d_level *);
//E xchar dunlevs_in_dungeon(d_level *);
//E xchar ledger_to_dnum(XCHAR_P);
//E xchar ledger_to_dlev(XCHAR_P);
//E xchar deepest_lev_reached(BOOLEAN_P);
//E boolean on_level(d_level *,d_level *);
//E void next_level(BOOLEAN_P);
//E void prev_level(BOOLEAN_P);
//E void u_on_newpos(int,int);
//E void u_on_sstairs(void);
//E void u_on_upstairs(void);
//E void u_on_dnstairs(void);
//E boolean On_stairs(XCHAR_P,XCHAR_P);
//E void get_level(d_level *,int);
//E boolean Is_botlevel(d_level *);
//E boolean Can_fall_thru(d_level *);
//E boolean Can_dig_down(d_level *);
//E boolean Can_rise_up(int,int,d_level *);
boolean In_quest(d_level *level) { return FALSE; }
//E boolean In_mines(d_level *);
//E boolean In_sheol(d_level *);
//E branch *dungeon_branch(const char *);
//E boolean at_dgn_entrance(const char *);
boolean In_hell(d_level *level) { return FALSE; }
//E boolean In_V_tower(d_level *);
//E boolean On_W_tower_level(d_level *);
//E boolean In_W_tower(int,int,d_level *);
//E void find_hell(d_level *);
//E void goto_hell(BOOLEAN_P,BOOLEAN_P);
//E void assign_level(d_level *,d_level *);
//E void assign_rnd_level(d_level *,d_level *,int);
//E int induced_align(int);
//E boolean Invocation_lev(d_level *);
xchar level_difficulty() { return 1; }
//E schar lev_by_name(const char *);
//#ifdef WIZARD
//E schar print_dungeon(BOOLEAN_P,schar *,xchar *);
//#endif
//E int donamelevel(void);
//E int dooverview(void);
//E void forget_mapseen(int);
//E void init_mapseen(d_level *);
//E void recalc_mapseen(void);
//E void recbranch_mapseen(d_level *, d_level *);
//E void remdun_mapseen(int);
//E char *get_annotation(d_level *);
//E const char *get_generic_level_description(d_level *);

/* ### eat.c ### */
//#ifdef USE_TRAMPOLI
//E int eatmdone(void);
//E int eatfood(void);
//E int opentin(void);
//E int unfaint(void);
//#endif
//E boolean is_edible(struct obj *);
//E void init_uhunger(void);
//E int Hear_again(void);
//E void reset_eat(void);
//E int doeat(void);
//E void gethungry(void);
//E void morehungry(int);
//E void lesshungry(int);
//E boolean is_fainted(void);
//E void reset_faint(void);
//E void violated_vegetarian(void);
//#if 0
//E void sync_hunger(void);
//#endif
//E void newuhs(BOOLEAN_P);
//E struct obj *floorfood(const char *,int);
//E void vomit(void);
int eaten_stat(int number1, struct obj *obj) { return 0; }
//E void food_disappears(struct obj *);
//E void food_substitution(struct obj *,struct obj *);
//E boolean bite_monster(struct monst *mon);
//E void fix_petrification(void);
//E void consume_oeaten(struct obj *,int);
//E boolean maybe_finished_meal(BOOLEAN_P);

/* ### hack.c ### */
//#ifdef DUNGEON_GROWTH
//E void catchup_dgn_growths(int);
//E void dgn_growths(BOOLEAN_P,BOOLEAN_P);
//#endif
//E boolean revive_nasty(int,int,const char*);
//E void movobj(struct obj *,XCHAR_P,XCHAR_P);
//E boolean may_dig(XCHAR_P,XCHAR_P);
//E boolean may_passwall(XCHAR_P,XCHAR_P);
//E boolean bad_rock(struct permonst *,XCHAR_P,XCHAR_P);
//E boolean invocation_pos(XCHAR_P,XCHAR_P);
//E boolean test_move(int, int, int, int, int);
//E void domove(void);
//E void invocation_message(void);
//E void wounds_message(struct monst *));
//E char *mon_wounds(struct monst *));
//E void spoteffects(BOOLEAN_P);
char* in_rooms(XCHAR_P x, XCHAR_P y, int number) { return NULL; }
//E boolean in_town(int,int);
//E void check_special_room(BOOLEAN_P);
//E int dopickup(void);
//E void lookaround(void);
//E int monster_nearby(void);
//E void nomul(int, const char *);
//E void unmul(const char *);
//E void showdmg(int, BOOLEAN_P);
//E void losehp(int,const char *,BOOLEAN_P);
//E void losehp_how(int,const char *,BOOLEAN_P,int);
//E void set_uhpmax(int,BOOLEAN_P);
//E void check_uhpmax(void);
//E int weight_cap(void);
//E int inv_weight(void);
//E int near_capacity(void);
//E int calc_capacity(int);
//E int max_capacity(void);
//E boolean check_capacity(const char *);
//E int inv_cnt(void);
//#ifdef GOLDOBJ
//E long money_cnt(struct obj *);
//#endif

/* ### invent.c ### */
int merged(struct obj **obj1, struct obj **obj2) { return 0; }
const char* currency(long number) { return "currency"; }
struct obj* g_at(int x, int y) { return NULL; }
void freeinv(struct obj *obj) {}

/* ### light.c ### */
//E void new_light_source(XCHAR_P, XCHAR_P, int, int, genericptr_t);
void del_light_source(int number, genericptr_t generic) {}
//E void do_light_sources(char **);
//E struct monst *find_mid(unsigned, unsigned);
//E void save_light_sources(int, int, int);
//E void restore_light_sources(int);
//E void relink_light_sources(BOOLEAN_P);
//E void obj_move_light_source(struct obj *, struct obj *);
//E boolean any_light_source(void);
//E void snuff_light_source(int, int);
boolean obj_sheds_light(struct obj *obj) { return FALSE; }
//E boolean obj_is_burning(struct obj *);
void obj_split_light_source(struct obj *obj1, struct obj *obj2) {}
//E void obj_merge_light_sources(struct obj *,struct obj *);
//E int candle_light_range(struct obj *);
//#ifdef WIZARD
//E int wiz_light_sources(void);
//#endif
//
/* ### makemon.c ### */
//E boolean is_home_elemental(struct permonst *);
//E struct monst *clone_mon(struct monst *,XCHAR_P,XCHAR_P);
//E struct monst *makemon(struct permonst *,int,int,int);
//E boolean create_critters(int,struct permonst *);
//E struct permonst *rndmonst(void);
//E void reset_rndmonst(int);
//E struct permonst *mkclass(CHAR_P,int);
//E int adj_lev(struct permonst *);
//E struct permonst *grow_up(struct monst *,struct monst *);
//E int mongets(struct monst *,int);
//E int golemhp(int);
//E boolean peace_minded(struct permonst *);
//E void set_malign(struct monst *);
//E void set_mimic_sym(struct monst *);
//E int mbirth_limit(int);
//E void mimic_hit_msg(struct monst *, SHORT_P);
//#ifdef GOLDOBJ
//E void mkmonmoney(struct monst *, long);
//#endif
//E int bagotricks(struct obj *);
//E boolean propagate(int, BOOLEAN_P,BOOLEAN_P);
//E void create_camera_demon(struct obj *,int,int);
//E int min_monster_difficulty(void);
//E int max_monster_difficulty(void);
boolean prohibited_by_generation_flags(struct permonst *permonst) { return FALSE; }


/* ### mon.c ### */
//E int select_newcham_form(struct monst *);
//E void remove_monster(int, int);
int undead_to_corpse(int number) { return NON_PM; }


/* ### o_init.c ### */
//E void init_objects(void);
//E int find_skates(void);
//E void oinit(void);
//E void savenames(int,int);
//E void restnames(int);
void discover_object(int number, BOOLEAN_P bool1, BOOLEAN_P bool2) {}
//E void undiscover_object(int);
//E int dodiscovered(void);
//E void dragons_init(void);

/* ### shk.c ### */
//#ifdef GOLDOBJ
//E long money2mon(struct monst *, long);
//E void money2u(struct monst *, long);
//#endif
//E char *shkname(struct monst *);
//E void shkgone(struct monst *);
//E void set_residency(struct monst *,BOOLEAN_P);
//E void replshk(struct monst *,struct monst *);
//E void restshk(struct monst *,BOOLEAN_P);
//E char inside_shop(XCHAR_P,XCHAR_P);
//E void u_left_shop(char *,BOOLEAN_P);
//E void remote_burglary(XCHAR_P,XCHAR_P);
//E void u_entered_shop(char *);
//E boolean same_price(struct obj *,struct obj *);
//E void shopper_financial_report(void);
//E int inhishop(struct monst *);
struct monst* shop_keeper(CHAR_P c) { return NULL; }
//E boolean tended_shop(struct mkroom *);
//E void delete_contents(struct obj *);
//E void obfree(struct obj *,struct obj *);
//E void home_shk(struct monst *,BOOLEAN_P);
//E void make_happy_shk(struct monst *,BOOLEAN_P);
//E void hot_pursuit(struct monst *);
//E void make_angry_shk(struct monst *,XCHAR_P,XCHAR_P);
//E int dopay(void);
//E boolean paybill(int);
//E void finish_paybill(void);
//E struct obj *find_oid(unsigned);
long contained_cost(struct obj *obj, struct monst *monst, long number, BOOLEAN_P bool1, BOOLEAN_P bool2) { return 0; }
//E long contained_gold(struct obj *);
//E void picked_container(struct obj *);
long unpaid_cost(struct obj *obj) { return 0; }
void addtobill(struct obj *obj, BOOLEAN_P bool1, BOOLEAN_P bool2, BOOLEAN_P bool3) {}
void splitbill(struct obj *obj1, struct obj *obj2) {}
void subfrombill(struct obj *obj, struct monst *monst) {}
//E long stolen_value(struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P);
//E void sellobj_state(int);
//E void sellobj(struct obj *,XCHAR_P,XCHAR_P);
//E int doinvbill(int);
//E struct monst *shkcatch(struct obj *,XCHAR_P,XCHAR_P);
//E void add_damage(XCHAR_P,XCHAR_P,long);
//E int repair_damage(struct monst *,struct damage *,BOOLEAN_P);
//E int shk_move(struct monst *);
//E void after_shk_move(struct monst *);
boolean is_fshk(struct monst *monst) { return FALSE; }
//E void shopdig(int);
//E void pay_for_damage(const char *,BOOLEAN_P);
boolean costly_spot(XCHAR_P x, XCHAR_P y) { return FALSE; }
//E struct obj *shop_object(XCHAR_P,XCHAR_P);
//E void price_quote(struct obj *);
//E void shk_chat(struct monst *);
//E void check_unpaid_usage(struct obj *,BOOLEAN_P);
//E void check_unpaid(struct obj *);
//E void costly_gold(XCHAR_P,XCHAR_P,long);
long get_cost_of_shop_item(struct obj *obj) { return 0; }
//E boolean block_door(XCHAR_P,XCHAR_P);
//E boolean block_entry(XCHAR_P,XCHAR_P);
//#ifdef BLACKMARKET
//E void blkmar_guards(struct monst *);
//E void set_black_marketeer_angry(void);
//E void bars_around_portal(BOOLEAN_P);
//#endif /* BLACKMARKET */
char* shk_your(char *buf, struct obj *obj) { return "shk_your"; }
char* Shk_Your(char *buf, struct obj *obj) { return "Shk_Your"; }

/* ### teleport.c ### */
//E boolean goodpos(int,int,struct monst *,unsigned);
//E boolean enexto(coord *,XCHAR_P,XCHAR_P,struct permonst *);
//E boolean enexto_core(coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned);
//E boolean enexto_core_range(coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned,int);
//E int epathto(coord *,int,XCHAR_P,XCHAR_P,struct permonst *);
//E void teleds(int,int,BOOLEAN_P);
//E boolean safe_teleds(BOOLEAN_P);
//E boolean teleport_pet(struct monst *,BOOLEAN_P);
//E void tele(void);
//E int dotele(void);
//E void level_tele(void);
//E void domagicportal(struct trap *);
//E void tele_trap(struct trap *);
//E void level_tele_trap(struct trap *);
//E void rloc_to(struct monst *,int,int);
//E boolean rloc(struct monst *, BOOLEAN_P);
//E boolean tele_restrict(struct monst *);
//E void mtele_trap(struct monst *, struct trap *,int);
//E int mlevel_tele_trap(struct monst *, struct trap *,BOOLEAN_P,int);
void rloco(struct obj *obj) {}
//E int random_teleport_level(void);
//E boolean u_teleport_mon(struct monst *,BOOLEAN_P);

/* ### timeout.c ### */
//E void burn_away_slime(void);
//E void nh_timeout(void);
//E void fall_asleep(int, BOOLEAN_P);
//E void attach_egg_hatch_timeout(struct obj *);
void attach_fig_transform_timeout(struct obj *obj) {}
//E void kill_egg(struct obj *);
//E void hatch_egg(genericptr_t, long);
//E void learn_egg_type(int);
//E void burn_object(genericptr_t, long);
void begin_burn(struct obj *obj, BOOLEAN_P bool) {}
//E void end_burn(struct obj *, BOOLEAN_P);
//E void do_storms(void);
boolean start_timer(long timeleft, SHORT_P typ, SHORT_P action, genericptr_t generic) { return TRUE; }
long stop_timer(SHORT_P typ, genericptr_t generic) { return 0; }
//E void run_timers(void);
//E void obj_move_timers(struct obj *, struct obj *);
void obj_split_timers(struct obj *obj1, struct obj *obj2) {}
void obj_stop_timers(struct obj *obj) {}
//E boolean obj_is_local(struct obj *);
//E void save_timers(int,int,int);
//E void restore_timers(int,int,BOOLEAN_P,long);
//E void relink_timers(BOOLEAN_P);

/* ### topten.c ### */
//#ifdef RECORD_CONDUCT
//E long encodeconduct(void);
//#endif
//E void topten(int);
//E void prscore(int,char **);
struct obj* tt_oname(struct obj *obj) { return NULL; }

/* ### trap.c ### */
//E boolean burnarmor(struct monst *));
//E boolean rust_dmg(struct obj *,const char *,int,BOOLEAN_P,struct monst *);
//E void grease_protect(struct obj *,const char *,struct monst *);
//E struct trap *maketrap(int,int,int);
//E void fall_through(BOOLEAN_P);
//E struct monst *animate_statue(struct obj *,XCHAR_P,XCHAR_P,int,int *);
//E struct monst *activate_statue_trap(struct trap *,XCHAR_P,XCHAR_P,BOOLEAN_P);
//E void dotrap(struct trap *, unsigned);
//E void seetrap(struct trap *);
//E int mintrap(struct monst *);
//E void instapetrify(const char *);
//E void minstapetrify(struct monst *,BOOLEAN_P);
//#ifdef WEBB_DISINT
//E int instadisintegrate(const char *);
//E int minstadisintegrate(struct monst *);
//#endif
//E void selftouch(const char *);
//E void mselftouch(struct monst *,const char *,BOOLEAN_P);
//E void float_up(void);
//E void fill_pit(int,int);
//E int float_down(long, long);
//E int fire_damage(struct obj *,BOOLEAN_P,BOOLEAN_P,XCHAR_P,XCHAR_P);
//E boolean water_damage(struct obj *,BOOLEAN_P,BOOLEAN_P);
//E boolean drown(void);
//E void drain_en(int);
//E int dountrap(void);
//E int untrap(BOOLEAN_P);
//E boolean chest_trap(struct obj *,int,BOOLEAN_P);
//E void deltrap(struct trap *);
//E boolean delfloortrap(struct trap *);
//E struct trap *t_at(int,int);
//E void b_trapped(const char *,int);
//E boolean unconscious(void);
boolean lava_effects() { return FALSE; }
boolean swamp_effects() { return FALSE; }
//E void blow_up_landmine(struct trap *);
//E int launch_obj(SHORT_P,int,int,int,int,int));

/* ### wield.c ### */
//E void setuwep(struct obj *);
//E void setuqwep(struct obj *);
//E void setuswapwep(struct obj *);
//E int dowield(void);
//E int doswapweapon(void);
//E int dowieldquiver(void);
//E boolean wield_tool(struct obj *,const char *);
//E int can_twoweapon(void);
void drop_uswapwep() {}
//E int dotwoweapon(void);
//E void uwepgone(void);
//E void uswapwepgone(void);
//E void uqwepgone(void);
//E void untwoweapon(void);
//E void erode_obj(struct obj *,BOOLEAN_P,BOOLEAN_P);
//E int chwepon(struct obj *,int);
//E int welded(struct obj *);
//E void weldmsg(struct obj *);
//E void setmnotwielded(struct monst *,struct obj *);
//E void unwield_weapons_silently(void);

/* ### zap.c ### */
//E int bhitm(struct monst *,struct obj *);
//E void probe_monster(struct monst *);
boolean get_obj_location(struct obj *obj, xchar *x, xchar *y, int number) { return FALSE; }
//E boolean get_mon_location(struct monst *,xchar *,xchar *,int);
//E struct monst *get_container_location(struct obj *obj, int *, int *);
//E struct monst *montraits(struct obj *,coord *);
//E struct monst *revive(struct obj *);
//E int unturn_dead(struct monst *);
//E void cancel_item(struct obj *);
//E boolean drain_item(struct obj *);
//E struct obj *poly_obj(struct obj *, int);
//E boolean obj_resists(struct obj *,int,int);
//E boolean obj_shudders(struct obj *);
//E void do_osshock(struct obj *);
//E int bhito(struct obj *,struct obj *);
//E int bhitpile(struct obj *,int (*)(OBJ_P,OBJ_P),int,int);
//E int zappable(struct obj *);
//E void zapnodir(struct obj *);
//E int dozap(void);
//E int zapyourself(struct obj *,BOOLEAN_P);
//E boolean cancel_monst(struct monst *,struct obj *, BOOLEAN_P,BOOLEAN_P,BOOLEAN_P);
//E void weffects(struct obj *);
//E int spell_damage_bonus(void);
//E const char *exclam(int force);
//E void hit(const char *,struct monst *,const char *);
//E void miss(const char *,struct monst *);
//E struct monst *bhit(int,int,int,int,int (*)(MONST_P,OBJ_P),
//                             int (*)(OBJ_P,OBJ_P),struct obj *, boolean *));
//E struct monst *boomhit(int,int);
//E int burn_floor_paper(int,int,BOOLEAN_P,BOOLEAN_P);
//E void buzz(int,int,XCHAR_P,XCHAR_P,int,int);
//E void melt_ice(XCHAR_P,XCHAR_P);
//E int zap_over_floor(XCHAR_P,XCHAR_P,int,boolean *);
//E void fracture_rock(struct obj *);
//E boolean break_statue(struct obj *);
//E void destroy_item(int,int);
//E int destroy_mitem(struct monst *,int,int);
//E int resist(struct monst *,CHAR_P,int,int);
//E void makewish(BOOLEAN_P);

