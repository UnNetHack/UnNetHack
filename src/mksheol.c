/* This file contains the functions that create Sheol filler levels. */

#include "hack.h"
#include "sp_lev.h"

#define HEIGHT	(ROWNO - 1)
#define WIDTH	(COLNO - 2)

/* Minimum distance between points when using voronoi diagram to place
 * water, ice and solid ground.
 *
 * This is euclidean distance to second power. 
 * (i.e. 2 == 4) */
#define MINIMUM_VORONOI_DISTANCE 4

/* See sheol.txt for Sheol description.
 * This file deals with generating the levels. */

/* init_level_base_voronoi() uses this. */
typedef struct spatchcoord
{
	coord c;
	schar typ;
} patchcoord;

/* probability of making a floor:
 * prob / out_of or if guaranteed is 1 then it's 100% chance */
typedef struct sfloorprob
{
	int prob;
	int out_of;

	int guaranteed;
} floorprob;

/* These are chances which type to use for each tile in a voronoi diagram. */
static schar typs[10] = { ICEWALL, 
			  CRYSTALICEWALL, CRYSTALICEWALL, CRYSTALICEWALL,
			  POOL, POOL,
			  STONE, STONE, STONE, STONE };

static void init_level_base_voronoi(schar* typs, int numtyps);
static int check_voronoi_winner(patchcoord* coords, int num_coords,
								int x, int y);
static void carve_path(floorprob* probs);
static void fuzzy_circle(int x, int y, 
			 int guaranteed_passage_radius, int fallout,
			 floorprob* floorprobs);
static void
wallify_map();

/* Return values from plug_unreachable_places */
#define STAT_REJECT 0       /* "please reject the map, it's too bad" */
#define STAT_SEMIPLUGGED  1 /* Some places are only reachable behind ice */
#define STAT_ALLREACHABLE 2 /* Every place is reachable by walking or 
			       levitating */

static int plug_unreachable_places();

void
mksheol(init_lev)
	lev_init *init_lev;
{
	int i1, i2;
	int testval;
	floorprob* probs;

	probs = (floorprob*) alloc(sizeof(floorprob) * COLNO * ROWNO);

	again:
	memset(probs, 0, sizeof(floorprob) * COLNO * ROWNO);

	init_level_base_voronoi(typs, 10);

	/* Then, carve a "path" from somewhere left of the level to the right
	 * of the level. */
	carve_path(probs);
	
	/* Sometimes, put a lot of clouds at one side of the level. */

	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			testval = 1;
			if (probs[i1 + i2 * COLNO].out_of != 0) {
				testval = rn2(probs[i1+i2*COLNO].out_of);	
			}
			if (testval < probs[i1+i2*COLNO].prob ||
			    probs[i1+i2*COLNO].guaranteed)
			{
				if (levl[i1][i2].typ == ICEWALL)
					levl[i1][i2].typ = ICE;
				else if (levl[i1][i2].typ != POOL)
					levl[i1][i2].typ = ROOM;

			}
			else if (levl[i1][i2].typ == POOL)
				levl[i1][i2].typ = ICEWALL;
		}

	if (plug_unreachable_places() == STAT_REJECT)
		goto again;

	/* Finalization */
	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			levl[i1][i2].lit = TRUE; /* lit things up */
			if (IS_ROCK(levl[i1][i2].typ) &&
			    !IS_ANY_ICEWALL(levl[i1][i2].typ))
				levl[i1][i2].wall_info |= W_NONDIGGABLE;
		}
	
	wallify_map();

	level.flags.is_maze_lev = FALSE;
	level.flags.is_cavernous_lev = TRUE;

	free(probs);
}

static void 
init_level_base_voronoi(schar* typs, int numtyps)
{
	int patches, i1, i2, i3;
	int winner_patch, winner_distance, tmpdist;
	patchcoord points[150];

	/* We use a voronoi diagram to put ice and solid ground on the level.
	 * Maybe it looks interesting? I hope so. That's kind of the point. */

	patches = rn1(145, 5);

	for (i1 = 0; i1 < patches; ++i1) {
		points[i1].c.x = rn1(COLNO-1, 1);
		points[i1].c.y = rn2(ROWNO);
		points[i1].typ = typs[rn2(numtyps)];

		/* Don't want them to be too close to other points... */
		for (i2 = 0; i2 < i1; ++i2) {
			if (dist2(points[i1].c.x, points[i1].c.y,
				  points[i2].c.x, points[i2].c.y) < 
				  MINIMUM_VORONOI_DISTANCE) {
				--i1;
				break;
			}
		}
	}

	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			winner_patch = check_voronoi_winner(points, patches, i1, i2);
			levl[i1][i2].typ = points[winner_patch].typ;
		}
}

static int 
check_voronoi_winner(coords, num_coords, x, y)
	patchcoord* coords;
	int num_coords;
	int x, y;
{
	int i1;
	int winner_distance;
	int winner;
	int d;

	winner_distance = 10000;

	for (i1 = 0; i1 < num_coords; ++i1)
	{
		d = dist2(coords[i1].c.x, coords[i1].c.y, x, y);
		if (d < winner_distance) {
			winner_distance = d;
			winner = i1;
		}
	}

	return winner;
}

typedef struct sbezcoord
{
	int x, y;         /* coordinate */
} bezcoord;

static void 
carve_path(floorprobs)
	floorprob* floorprobs;
{
	/*
	 * We'll draw a bezier curve from the left of the level to the right.
	 * We first pick a rough path from left to right by picking points.
	 * Then we'll make a smooth path using a bezier curve by going through
	 * those points.
	 *
	 * We use every second coordinate as a control point.
	 */

	bezcoord points[100];
	int i1, i2, num_points, c1_x, c1_y, c2_x, c2_y;
	int r_i2;
	int x, y;

	int sample_points;
	
	int path_x, path_y;
	int tries;

	/* Attempt to make ~random point paths to the end. */
	while(1) {
		path_x = -2;
		path_y = ROWNO / 2;

		for (i1 = 0; i1 < 100; ++i1) {
			tries = 0;
			while(tries < 100) {
				tries++;
				points[i1].x = path_x;
				points[i1].y = path_y;

				if (!rn2(2))
					points[i1].x += rn2(11)-5;
				else
					points[i1].x += rn2(6)-2;
				points[i1].y += rn2(11)-5;

				if (points[i1].y < 0 ||
				    points[i1].y >= ROWNO ||
				    points[i1].x < 1 ||
				    points[i1].x >= COLNO)
					continue;
				for (i2 = 0; i2 < i1; ++i2)
					if (dist2(points[i2].x, points[i2].y,
						  points[i1].x, points[i1].y) <=
						16)
						break;
				if (i2 < i1)
					continue;
				break;
			}
			if (tries >= 100)
			{
				path_x = -2;
				path_y = ROWNO / 2;
				i1 = -1;
				continue;
			}
			if (i1 > 0 && points[i1-1].x >= COLNO-5)
				break;
			path_x = points[i1].x;
			path_y = points[i1].y;
		}
		if (i1 >= 100)
			continue;
		break;
	}

	num_points = i1;

	for (i1 = 0; i1 < num_points - 2; i1 += 2) {
		sample_points = isqrt(dist2(points[i1].x, points[i1].y,
					    points[i1+1].x, points[i1+1].y)+
				    dist2(points[i1+1].x, points[i1+1].y,
				    	  points[i1+2].x, points[i1+2].y))*20;
		for (i2 = 0; i2 <= sample_points; ++i2)
		{
			r_i2 = sample_points - i2;
			#define BEZNUMERICAL(targ, m, idx1, idx2) \
				targ = points[idx1].m * i2 / sample_points; \
				targ += points[idx2].m * r_i2 / sample_points;

			BEZNUMERICAL(c1_x, x, i1, i1+1);
			BEZNUMERICAL(c1_y, y, i1, i1+1);
			BEZNUMERICAL(c2_x, x, i1+1, i1+2);
			BEZNUMERICAL(c2_y, y, i1+1, i1+2);

			x = c1_x * i2 / sample_points;
			x += c2_x * r_i2 / sample_points;
			y = c1_y * i2 / sample_points;
			y += c2_y * r_i2 / sample_points;

			#undef BEZNUMERICAL

			fuzzy_circle(x, y, 1, 2, floorprobs);
		}
	}
}


/* Makes a passable circle centered at x, y.
 * To radius guaranteed_passage, guarantees passability.
 *
 * After that, linearly makes it less likely that a passable square is
 * made. This makes a sort of "rough" edge. */
static void 
fuzzy_circle(x, y, guaranteed_passage_radius, fallout, floorprobs)
	int x, y;
	int guaranteed_passage_radius, fallout;
	floorprob* floorprobs;
{
	int i1, i2;
	int fallout_2 = fallout * fallout;
	int guaranteed_2 = guaranteed_passage_radius * 
			   guaranteed_passage_radius;
	int prob, d;

	if (fallout < guaranteed_passage_radius)
		fallout = guaranteed_passage_radius;

	for (i1 = x - fallout; i1 <= x + fallout; ++i1)
		for (i2 = y - fallout; i2 <= y + fallout; ++i2) {
			if (i1 < 1 || i1 >= COLNO ||
			    i2 < 0 || i2 >= ROWNO)
				continue;
			d = dist2(i1, i2, x, y);
			if (d > fallout_2)
				continue;
			if (d <= guaranteed_passage_radius)
				floorprobs[i1 + i2 * COLNO].guaranteed = 1;
			else {
				d -= guaranteed_passage_radius;
				d = (d * 100) / 
					((fallout_2 - 
					  guaranteed_passage_radius));
				d = 100 - d;
				floorprobs[i1+i2*COLNO].prob += d;
				floorprobs[i1+i2*COLNO].out_of += 100;
			}
		}
}

static void
wallify_map()
{

    int x, y, xx, yy;

    for(x = 1; x < COLNO; x++)
	for(y = 0; y < ROWNO; y++)
	    if(levl[x][y].typ == STONE) {
		for(yy = y - 1; yy <= y+1; yy++)
		    for(xx = x - 1; xx <= x+1; xx++)
			if(isok(xx,yy) && levl[xx][yy].typ == ROOM) {
			    if(yy != y)	levl[x][y].typ = HWALL;
			    else	levl[x][y].typ = VWALL;
			}
	    }
}

#define VALID_PASSABLE(x, y) (levl[x][y].typ == ICE || \
			      levl[x][y].typ == POOL || \
			      levl[x][y].typ == ROOM)
#define VALID_PASSABLE2(x, y) (VALID_PASSABLE(x, y) || \
			       IS_ANY_ICEWALL(levl[x][y].typ))

static int 
plug_unreachable_places()
{
	char fillmap[COLNO][ROWNO];
	int not_passable;
	int x, y;
	int flood_x, flood_y;
	int tries;
	int done;

	not_passable = 0;

	memset(fillmap, 0, sizeof(fillmap));

	tries = 100;
	while(tries > 0) {
		tries--;
		x = rn1(COLNO-2, 1);
		y = rn2(ROWNO);
		if (levl[x][y].typ != ROOM &&
		    levl[x][y].typ != ICE)
			continue;
		break;
	}
	if (tries <= 0)
		return STAT_REJECT;
	
	/* flood fill */
	done = 0;
	flood_x = x;
	flood_y = y;
	fillmap[flood_x][flood_y] = 1;

	while(!done) {
		done = 1;
		for (x = 2; x < COLNO-1; ++x)
			for (y = 1; y < ROWNO-1; ++y)
			{
				if (fillmap[x+1][y] == 0 &&
				    VALID_PASSABLE(x+1, y)) {
				    fillmap[x+1][y] = 1;
				    done = 0;
				}
				if (fillmap[x-1][y] == 0 &&
				    VALID_PASSABLE(x-1, y)) {
				    fillmap[x-1][y] = 1;
				    done = 0;
				}
				if (fillmap[x][y-1] == 0 &&
				    VALID_PASSABLE(x, y-1)) {
				    fillmap[x][y-1] = 1;
				    done = 0;
				}
				if (fillmap[x][y+1] == 0 &&
				    VALID_PASSABLE(x, y+1)) {
				    fillmap[x][y+1] = 1;
				    done = 0;
				}
			}
	}

	for (x = 1; x < COLNO; ++x)
		for (y = 0; y < ROWNO; ++y)
			if (VALID_PASSABLE(x, y) &&
			    fillmap[x][y] != 1)
			{
			    not_passable = 1;
			    break;
			}

	if (!not_passable)
		return STAT_ALLREACHABLE;

	/* flood fill again, but go through ice this time */
	done = 0;
	not_passable = 0;
	memset(fillmap, 0, sizeof(fillmap));
	fillmap[flood_x][flood_y] = 1;

	while(!done) {
		done = 1;
		for (x = 2; x < COLNO-1; ++x)
			for (y = 1; y < ROWNO-1; ++y)
			{
				if (fillmap[x+1][y] == 0 &&
				    VALID_PASSABLE2(x+1, y)) {
				    fillmap[x+1][y] = 1;
				    done = 0;
				}
				if (fillmap[x-1][y] == 0 &&
				    VALID_PASSABLE2(x-1, y)) {
				    fillmap[x-1][y] = 1;
				    done = 0;
				}
				if (fillmap[x][y-1] == 0 &&
				    VALID_PASSABLE2(x, y-1)) {
				    fillmap[x][y-1] = 1;
				    done = 0;
				}
				if (fillmap[x][y+1] == 0 &&
				    VALID_PASSABLE2(x, y+1)) {
				    fillmap[x][y+1] = 1;
				    done = 0;
				}
			}
	}

	for (x = 1; x < COLNO; ++x)
		for (y = 0; y < ROWNO; ++y)
			if (VALID_PASSABLE(x, y) &&
			    fillmap[x][y] != 1)
			{
			    not_passable = 1;
			    break;
			}
	
	if (not_passable)
		return STAT_REJECT;
	return STAT_SEMIPLUGGED;
}

