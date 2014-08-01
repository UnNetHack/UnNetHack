/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern const char *hu_stat[];	/* defined in eat.c */

const char * const enc_stat[] = {
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
};

STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);

extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
extern const struct text_color_option *text_colors;

struct color_option
text_color_of(text, color_options)
const char *text;
const struct text_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	if (strstri(color_options->text, text)
	 || strstri(text, color_options->text))
		return color_options->color_option;
	return text_color_of(text, color_options->next);
}

struct color_option
percentage_color_of(value, max, color_options)
int value, max;
const struct percent_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	switch (color_options->statclrtype) {
		default:
		case STATCLR_TYPE_PERCENT:
			if (100 * value <= color_options->percentage * max)
				return color_options->color_option;
			break;
		case STATCLR_TYPE_NUMBER_EQ:
			if (value == color_options->percentage)
				return color_options->color_option;
			break;
		case STATCLR_TYPE_NUMBER_LT:
			if (value < color_options->percentage)
				return color_options->color_option;
			break;
		case STATCLR_TYPE_NUMBER_GT:
			if (value > color_options->percentage)
				return color_options->color_option;
			break;
	}
	return percentage_color_of(value, max, color_options->next);
}

void
start_color_option(color_option)
struct color_option color_option;
{
#ifdef TTY_GRAPHICS
	int i;
	if (color_option.color != NO_COLOR)
		term_start_color(color_option.color);
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_start_attr(i);
#endif  /* TTY_GRAPHICS */
}

void
end_color_option(color_option)
struct color_option color_option;
{
#ifdef TTY_GRAPHICS
	int i;
	if (color_option.color != NO_COLOR)
		term_end_color();
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_end_attr(i);
#endif  /* TTY_GRAPHICS */
}

static
void
apply_color_option(color_option, newbot2, statusline)
struct color_option color_option;
const char *newbot2;
int statusline; /* apply color on this statusline: 1 or 2 */
{
	if (!iflags.use_status_colors || !iflags.use_color) return;
	curs(WIN_STATUS, 1, statusline-1);
	start_color_option(color_option);
	putstr(WIN_STATUS, 0, newbot2);
	end_color_option(color_option);
}

void
add_colored_text(text, newbot2)
const char *text;
char *newbot2;
{
	char *nb;
	struct color_option color_option;

	if (*text == '\0') return;

	/* don't add anything if it can't be displayed.
	 * Otherwise the color of invisible text may bleed into
	 * the statusline. */
	if (strlen(newbot2) >= min(MAXCO, CO)-1) return;

	if (!iflags.use_status_colors) {
		Sprintf(nb = eos(newbot2), " %s", text);
                return;
        }

	Strcat(nb = eos(newbot2), " ");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Strcat(nb = eos(nb), text);
	curs(WIN_STATUS, 1, 1);
       	color_option = text_color_of(text, text_colors);
	start_color_option(color_option);
	/* Trim the statusline to always have the end color
	 * to have effect. */
	newbot2[min(MAXCO, CO)-1] = '\0';
	putstr(WIN_STATUS, 0, newbot2);
	end_color_option(color_option);
}

STATIC_OVL int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

const char *
rank_of(lev, monnum, female)
	int lev;
	short monnum;
	boolean female;
{
	register struct Role *role;
	register int i;


	/* Find the role */
	for (role = (struct Role *) roles; role->name.m; role++)
	    if (monnum == role->malenum || monnum == role->femalenum)
	    	break;
	if (!role->name.m)
	    role = &urole;

	/* Find the rank */
	for (i = xlev_to_rank((int)lev); i >= 0; i--) {
	    if (female && role->rank[i].f) return (role->rank[i].f);
	    if (role->rank[i].m) return (role->rank[i].m);
	}

	/* Try the role name, instead */
	if (female && role->name.f) return (role->name.f);
	else if (role->name.m) return (role->name.m);
	return ("Player");
}


const char *
rank()
{
	return(rank_of(u.ulevel, Role_switch, flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
	register int i, j;


	/* Loop through each of the roles */
	for (i = 0; roles[i].name.m; i++)
	    for (j = 0; j < 9; j++) {
	    	if (roles[i].rank[j].m && !strncmpi(str,
	    			roles[i].rank[j].m, strlen(roles[i].rank[j].m))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].m);
	    	    return roles[i].malenum;
	    	}
	    	if (roles[i].rank[j].f && !strncmpi(str,
	    			roles[i].rank[j].f, strlen(roles[i].rank[j].f))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].f);
	    	    return ((roles[i].femalenum != NON_PM) ?
	    	    		roles[i].femalenum : roles[i].malenum);
	    	}
	    }
	return NON_PM;
}

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	for (i = 0; i < 9; i++) {
	    if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) maxr = r;
	    if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) maxr = r;
	}
	mrank_sz = maxr;
	return;
}

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    int deepest = deepest_lev_reached(FALSE);
    long ugold = u.ugold + hidden_gold();

    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
    return ugold + u.urscore + (long)(50 * (deepest - 1))
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#ifdef DUMP_LOG
void bot1str(char *newbot1)
#else
STATIC_OVL void
bot1()
#endif
{
#ifndef DUMP_LOG
	char newbot1[MAXCO];
#endif
	char *nb;
	int i=0,j;
	int save_botlx = flags.botlx;

	Strcpy(newbot1, "");
	if (flags.hitpointbar) {
		flags.botlx = 0;
		curs(WIN_STATUS, 1, 0);
		putstr(WIN_STATUS, 0, newbot1);
		Strcat(newbot1, "[");
		i = 1; /* don't overwrite the string in front */
		curs(WIN_STATUS, 1, 0);
		putstr(WIN_STATUS, 0, newbot1);
	}
	Strcat(newbot1, plname);
	if('a' <= newbot1[i] && newbot1[i] <= 'z') newbot1[i] += 'A'-'a';
	newbot1[10] = '\0';
	Sprintf(nb = eos(newbot1)," the ");

	if (Upolyd) {
		char mbot[BUFSZ];
		int k = 0;

		Strcpy(mbot, mons[u.umonnum].mname);
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}
		Sprintf(nb = eos(nb), "%s", mbot);
	} else
		Sprintf(nb = eos(nb), "%s", rank());

	if (flags.hitpointbar) {
		int bar_length = strlen(newbot1)-1;
		char tmp[MAXCO];
		char *p = tmp;
		/* filledbar >= 0 and < MAXCO */
		int hp = (uhp() < 0) ? 0 : uhp();
		int filledbar = hp * bar_length / uhpmax();
		if (filledbar >= MAXCO) { filledbar = MAXCO-1; }
		Strcpy(tmp, newbot1);
		p++;

		/* draw hp bar */
		if (iflags.use_inverse) term_start_attr(ATR_INVERSE);
		p[filledbar] = '\0';
		if (iflags.use_color) {
			/* draw in color mode */
			apply_color_option(percentage_color_of(uhp(), uhpmax(), hp_colors), tmp, 1);
		} else {
			/* draw in inverse mode */
			curs(WIN_STATUS, 1, 0);
			putstr(WIN_STATUS, 0, tmp);
		}
		term_end_color();
		if (iflags.use_inverse) term_end_attr(ATR_INVERSE);

		Strcat(newbot1, "]");
	}

	Sprintf(nb = eos(nb),"  ");
	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > STR18(100))
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < STR18(100))
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(nb = eos(nb),"St:18/** ");
	} else
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
	Sprintf(nb = eos(nb),
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
	if (flags.showscore)
	    Sprintf(nb = eos(nb), " S:%ld", botl_score());
#endif
#ifdef DUMP_LOG
}
STATIC_OVL void
bot1()
{
	char newbot1[MAXCO];

	int save_botlx = flags.botlx;
	bot1str(newbot1);
#endif
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
	flags.botlx = save_botlx;
}

/* provide the name of the current level for display by various ports */
int
describe_level(buf)
char *buf;
{
	int ret = 1;

	/* TODO:	Add in dungeon name */
	if (Is_knox(&u.uz))
		Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
	else if (In_quest(&u.uz))
		Sprintf(buf, "Home %d ", dunlev(&u.uz));
	else if (In_endgame(&u.uz))
		Sprintf(buf,
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
	else if (Is_blackmarket(&u.uz))
		Sprintf(buf, "Blackmarket ");
	else if (Is_town_level(&u.uz))
		Sprintf(buf, "Town ");
	else if (Is_minetown_level(&u.uz))
		Sprintf(buf, "Mine Town:%-2d ", depth(&u.uz));
	else {
		char *dgn_name = dungeons[u.uz.dnum].dname;
		if (!strncmpi(dgn_name, "The ", 4)) { dgn_name += 4; }
		/* ports with more room may expand this one */
		Sprintf(buf, "%s:%-2d ", 
		        iflags.show_dgn_name ? dgn_name : "Dlvl",
		        depth(&u.uz));
		ret = 0;
	}
	return ret;
}

#ifdef DUMP_LOG
void bot2str(newbot2)
char* newbot2;
#else
STATIC_OVL void
bot2()
#endif
{
#ifndef DUMP_LOG
	char  newbot2[MAXCO];
#endif
	register char *nb;
	int hp, hpmax;
	int cap = near_capacity();
	int save_botlx = flags.botlx;

	hp = Upolyd ? u.mh : u.uhp;
	hpmax = Upolyd ? u.mhmax : u.uhpmax;

	if(hp < 0) hp = 0;
	(void) describe_level(newbot2);
	Sprintf(nb = eos(newbot2), "%c:%-2ld", oc_syms[COIN_CLASS],
		u.ugold
	       );

	Strcat(nb = eos(newbot2), " HP:");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
	flags.botlx = 0;

	Sprintf(nb = eos(nb), "%d(%d)", hp, hpmax);
	apply_color_option(percentage_color_of(hp, hpmax, hp_colors), newbot2, 2);
	Strcat(nb = eos(nb), " Pw:");
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);

	Sprintf(nb = eos(nb), "%d(%d)", u.uen, u.uenmax);
	apply_color_option(percentage_color_of(u.uen, u.uenmax, pw_colors), newbot2, 2);
	Sprintf(nb = eos(nb), " AC:%-2d", u.uac);
	if (Upolyd)
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
	else if(flags.showexp)
		Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
	else
		Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);

    if (flags.showweight)
        Sprintf(nb = eos(nb), " Wt:%ld/%ld", (long)(inv_weight()+weight_cap()),
                (long)weight_cap());


	if(flags.time)
		Sprintf(nb = eos(nb), " T:%ld", moves);

#ifdef REALTIME_ON_BOTL
	if(iflags.showrealtime) {
		time_t currenttime = get_realtime();
		Sprintf(nb = eos(nb), " %ld:%2.2ld", currenttime / 3600, 
		        (currenttime % 3600) / 60);
	}
#endif

	if(strcmp(hu_stat[u.uhs], "        "))
		add_colored_text(hu_stat[u.uhs], newbot2);
	if(Confusion)
		add_colored_text("Conf", newbot2);
	if(Sick) {
		if (u.usick_type & SICK_VOMITABLE)
			add_colored_text("FoodPois", newbot2);
		if (u.usick_type & SICK_NONVOMITABLE)
			add_colored_text("Ill", newbot2);
	}
	if(Blind)
	     	add_colored_text("Blind", newbot2);
	if(Stunned)
	     	add_colored_text("Stun", newbot2);
	if(Hallucination)
	     	add_colored_text("Hallu", newbot2);
	if(Slimed)
	     	add_colored_text("Slime", newbot2);
	if(cap > UNENCUMBERED)
		add_colored_text(enc_stat[cap], newbot2);
	if (u.ufeetfrozen > 0) {
	     	add_colored_text("Frozen", newbot2);
	}
	if(!Blind && sengr_at("Elbereth", u.ux, u.uy))
	     	add_colored_text("Elbereth", newbot2);
#ifdef DUMP_LOG
}
STATIC_OVL void
bot2()
{
	char newbot2[MAXCO];
	bot2str(newbot2);
	int save_botlx = flags.botlx;
#endif
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
	flags.botlx = save_botlx;
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

/*botl.c*/
