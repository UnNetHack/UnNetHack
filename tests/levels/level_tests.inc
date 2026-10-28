/*
 * Shared characterization tests for the special-level construction
 * helpers that exist in both src/sp_lev.c (Lua levels) and
 * src/spo_lev.c (.des levels).
 *
 * This file is included once per implementation by test_sp_lev.c and
 * test_spo_lev.c.  Including the .c file directly makes the otherwise
 * static helpers visible without touching production code.
 *
 * Tests describe the behaviour the two implementations must share.
 * Where the implementations have already diverged, the test documents
 * the divergence explicitly with #ifdef TEST_SP_LEV / TEST_SPO_LEV.
 */

#include <string.h>

/* ------------------------------------------------------------------ */
/* fixtures                                                            */
/* ------------------------------------------------------------------ */

static void
reset_level_state(void)
{
    memset(&level, 0, sizeof level);
    memset(rooms, 0, sizeof rooms);
    memset(doors, 0, sizeof doors);
    memset(SpLev_Map, 0, sizeof SpLev_Map);

    nroom = 0;
    nsubroom = 0;
    doorindex = 0;

    memset(&u, 0, sizeof u);
    u.uz.dnum = 0;
    u.uz.dlevel = 5;

    in_mklev = TRUE; /* same environment as the real level loaders */

    set_random_state(42);
    init_rect();
    vision_init();
    vision_reset(); /* block_point() needs viz_array */
}

/* ------------------------------------------------------------------ */
/* lvlfill_solid                                                       */
/* ------------------------------------------------------------------ */

START_TEST(test_lvlfill_solid_covers_maze_area)
{
    reset_level_state();
    levl[0][0].typ = ROOM;  /* left of x_maze_max: must stay untouched */
    levl[1][5].typ = ROOM;

    lvlfill_solid(VWALL, FALSE);

    ck_assert_int_eq(levl[2][0].typ, VWALL);
    ck_assert_int_eq(levl[x_maze_max][y_maze_max].typ, VWALL);
    ck_assert_int_eq(levl[0][0].typ, ROOM);
    ck_assert_int_eq(levl[1][5].typ, ROOM);
}
END_TEST

START_TEST(test_lvlfill_solid_sets_lit)
{
    reset_level_state();

    lvlfill_solid(STONE, TRUE);

    ck_assert_int_eq(levl[10][10].lit, 1);
}
END_TEST

/* ------------------------------------------------------------------ */
/* lvlfill_maze_grid                                                   */
/* ------------------------------------------------------------------ */

START_TEST(test_lvlfill_maze_grid_pattern)
{
    reset_level_state();
    levl[10][10].typ = ROOM;  /* sentinel outside the filled rectangle */

    lvlfill_maze_grid(3, 3, 9, 9, ROOM);

    ck_assert_int_eq(levl[3][3].typ, STONE);  /* odd/odd: maze wall */
    ck_assert_int_eq(levl[9][9].typ, STONE);
    ck_assert_int_eq(levl[4][4].typ, ROOM);   /* even/even: filling */
    ck_assert_int_eq(levl[3][4].typ, ROOM);   /* odd/even: filling */
    ck_assert_int_eq(levl[10][10].typ, ROOM);
}
END_TEST

START_TEST(test_lvlfill_maze_grid_row_zero_and_one_are_stone)
{
    reset_level_state();

    lvlfill_maze_grid(3, 0, 9, 1, ROOM);

    ck_assert_int_eq(levl[3][0].typ, STONE);
    ck_assert_int_eq(levl[5][1].typ, STONE);
}
END_TEST

START_TEST(test_lvlfill_maze_grid_corrmaze_is_all_stone)
{
    reset_level_state();
    level.flags.corrmaze = 1;

    lvlfill_maze_grid(3, 3, 9, 9, ROOM);

    ck_assert_int_eq(levl[3][3].typ, STONE);
    ck_assert_int_eq(levl[4][4].typ, STONE);
}
END_TEST

/* ------------------------------------------------------------------ */
/* solidify_map                                                        */
/* ------------------------------------------------------------------ */

START_TEST(test_solidify_map_flags_only_unused_rock)
{
    reset_level_state();
    levl[10][10].typ = VWALL;
    levl[11][10].typ = VWALL;
    levl[12][10].typ = ROOM;
    SpLev_Map[11][10] = 1;

    solidify_map();

    ck_assert_int_eq(levl[10][10].wall_info, W_NONDIGGABLE | W_NONPASSWALL);
    ck_assert_int_eq(levl[11][10].wall_info, 0); /* used by the level */
    ck_assert_int_eq(levl[12][10].wall_info, 0); /* not a wall */
}
END_TEST

/* ------------------------------------------------------------------ */
/* set_wall_property                                                   */
/* ------------------------------------------------------------------ */

START_TEST(test_set_wall_property_applies_to_walls_only)
{
    reset_level_state();
    levl[10][10].typ = VWALL;
    levl[11][10].typ = TREE;
    levl[12][10].typ = IRONBARS;
    levl[13][10].typ = ROOM;

    set_wall_property(5, 5, 20, 20, W_NONDIGGABLE);

    ck_assert_int_eq(levl[10][10].wall_info, W_NONDIGGABLE);
    ck_assert_int_eq(levl[11][10].wall_info, W_NONDIGGABLE);
    ck_assert_int_eq(levl[12][10].wall_info, W_NONDIGGABLE);
    ck_assert_int_eq(levl[13][10].wall_info, 0);
}
END_TEST

START_TEST(test_set_wall_property_clamps_coordinates)
{
    reset_level_state();
    levl[1][0].typ = VWALL;
    levl[COLNO - 1][ROWNO - 1].typ = VWALL;

    /* Must not read or write outside the map. */
    set_wall_property(-10, -10, COLNO + 10, ROWNO + 10, W_NONPASSWALL);

    ck_assert_int_eq(levl[1][0].wall_info, W_NONPASSWALL);
    ck_assert_int_eq(levl[COLNO - 1][ROWNO - 1].wall_info, W_NONPASSWALL);
}
END_TEST

/* ------------------------------------------------------------------ */
/* remove_boundary_syms                                                */
/* ------------------------------------------------------------------ */

START_TEST(test_remove_boundary_syms_only_touched_crosswalls)
{
    reset_level_state();
    levl[10][10].typ = CROSSWALL;
    SpLev_Map[10][10] = 1;
    levl[11][10].typ = CROSSWALL;
    SpLev_Map[11][10] = 0;
    levl[12][10].typ = ROOM;

    remove_boundary_syms();

    ck_assert_int_eq(levl[10][10].typ, ROOM);
    ck_assert_int_eq(levl[11][10].typ, CROSSWALL);
    ck_assert_int_eq(levl[12][10].typ, ROOM);
}
END_TEST

/* ------------------------------------------------------------------ */
/* set_door_orientation                                                */
/* ------------------------------------------------------------------ */

START_TEST(test_set_door_orientation_horizontal_when_walls_left_right)
{
    reset_level_state();
    levl[10][10].typ = SDOOR;
    levl[9][10].typ = HWALL;
    levl[11][10].typ = HWALL;

    set_door_orientation(10, 10);

    ck_assert_int_eq(levl[10][10].horizontal, 1);
}
END_TEST

START_TEST(test_set_door_orientation_vertical_when_walls_up_down)
{
    reset_level_state();
    levl[10][10].typ = SDOOR;
    levl[10][9].typ = VWALL;
    levl[10][11].typ = VWALL;

    set_door_orientation(10, 10);

    ck_assert_int_eq(levl[10][10].horizontal, 0);
}
END_TEST

/* ------------------------------------------------------------------ */
/* get_room_loc                                                        */
/* ------------------------------------------------------------------ */

START_TEST(test_get_room_loc_accepts_fixed_coordinates)
{
    struct mkroom croom;
    coordxy x = 3, y = 4;

    reset_level_state();
    memset(&croom, 0, sizeof croom);
    croom.lx = 10;
    croom.ly = 5;
    croom.hx = 20;
    croom.hy = 15;

    get_room_loc(&x, &y, &croom);

    ck_assert_int_eq(x, 13);
    ck_assert_int_eq(y, 9);
}
END_TEST

START_TEST(test_get_room_loc_random_axis_stays_in_room)
{
    struct mkroom croom;
    coordxy x = -1, y = 4;

    reset_level_state();
    memset(&croom, 0, sizeof croom);
    croom.lx = 10;
    croom.ly = 5;
    croom.hx = 20;
    croom.hy = 15;

    get_room_loc(&x, &y, &croom);

    ck_assert_int_ge(x, 10);
    ck_assert_int_le(x, 20);
    ck_assert_int_eq(y, 9);
}
END_TEST

/* ------------------------------------------------------------------ */
/* build_room                                                          */
/* ------------------------------------------------------------------ */

START_TEST(test_build_room_creates_room_with_requested_size)
{
    room r;
    struct mkroom *created;

    reset_level_state();
    memset(&r, 0, sizeof r);
    r.x = 10;
    r.y = 5;
    r.w = 11;
    r.h = 11;
    r.rtype = OROOM;
    r.rlit = 1;
    r.chance = 100;

    created = build_room(&r, (struct mkroom *) 0);

    ck_assert_ptr_nonnull(created);
    ck_assert_int_eq(nroom, 1);
    ck_assert_int_eq(created->hx - created->lx + 1, 11);
    ck_assert_int_eq(created->hy - created->ly + 1, 11);
    ck_assert_int_eq(created->rtype, OROOM);
    ck_assert_int_eq(created->rlit, 1);
}
END_TEST

/* ------------------------------------------------------------------ */
/* maze1xy                                                             */
/* ------------------------------------------------------------------ */

/*
 * maze1xy must return a coordinate that has not been touched by the
 * level loader (SpLev_Map == 0), is odd/odd and lies inside the maze.
 *
 * With seed 42 the first candidate is (11,17).  Marking only that cell
 * as used makes the two loop guards pick different results, so the
 * assertions are exact and deterministic rather than probabilistic.
 */
START_TEST(test_maze1xy_returns_untouched_odd_coordinate)
{
    coord m;

    reset_level_state();
    memset(levl, ROOM, sizeof levl);
    memset(SpLev_Map, 0, sizeof SpLev_Map);
    SpLev_Map[11][17] = 1; /* the first candidate is already used */

#ifdef TEST_SP_LEV
    maze1xy(&m, DRY);

    ck_assert_int_ge(m.x, 3);
    ck_assert_int_le(m.x, x_maze_max - 1);
    ck_assert_int_ge(m.y, 3);
    ck_assert_int_le(m.y, y_maze_max - 1);
    ck_assert_int_eq(m.x % 2, 1);
    ck_assert_int_eq(m.y % 2, 1);
    /* Rejected (11,17), accepted the next candidate. */
    ck_assert_int_eq(m.x, 59);
    ck_assert_int_eq(m.y, 7);
    ck_assert_int_eq(SpLev_Map[m.x][m.y], 0);
#else
    /*
     * Known divergence: spo_lev.c still has the pre-2022 test inverted
     * and accepts a cell the loader has already used.  It therefore
     * returns the first candidate instead of skipping it.  This branch
     * pins the current (wrong) behaviour and must be deleted when the
     * implementations are merged.
     */
    maze1xy(&m, DRY);

    ck_assert_int_eq(m.x, 11);
    ck_assert_int_eq(m.y, 17);
    ck_assert_int_eq(SpLev_Map[m.x][m.y], 1);
#endif
}
END_TEST

/* ------------------------------------------------------------------ */
/* fill_empty_maze                                                     */
/* ------------------------------------------------------------------ */

/*
 * A maze where every cell is already used by the special level must not
 * be populated with random objects, monsters, gold or traps.
 */
START_TEST(test_fill_empty_maze_skips_fully_used_maze)
{
    reset_level_state();
    init_objects();
    memset(levl, ROOM, sizeof levl);
    memset(SpLev_Map, 1, sizeof SpLev_Map);

    fill_empty_maze();

#ifdef TEST_SP_LEV
    ck_assert_ptr_null(fobj);
    ck_assert_ptr_null(fmon);
#else
    /*
     * Known divergence: spo_lev.c counts used cells with the opposite
     * test and therefore believes the maze is empty.  This assertion
     * documents the bug; it must be replaced by the sp_lev.c behaviour
     * once the implementations are merged.
     */
    ck_assert_ptr_nonnull(fobj);
#endif
}
END_TEST

/* ------------------------------------------------------------------ */
/* suite                                                               */
/* ------------------------------------------------------------------ */

static Suite *
level_suite(void)
{
    Suite *s = suite_create("level-" IMPLEMENTATION_NAME);
    TCase *tc = tcase_create("primitives");

    suite_add_tcase(s, tc);

    tcase_add_test(tc, test_lvlfill_solid_covers_maze_area);
    tcase_add_test(tc, test_lvlfill_solid_sets_lit);
    tcase_add_test(tc, test_lvlfill_maze_grid_pattern);
    tcase_add_test(tc, test_lvlfill_maze_grid_row_zero_and_one_are_stone);
    tcase_add_test(tc, test_lvlfill_maze_grid_corrmaze_is_all_stone);
    tcase_add_test(tc, test_solidify_map_flags_only_unused_rock);
    tcase_add_test(tc, test_set_wall_property_applies_to_walls_only);
    tcase_add_test(tc, test_set_wall_property_clamps_coordinates);
    tcase_add_test(tc, test_remove_boundary_syms_only_touched_crosswalls);
    tcase_add_test(tc, test_set_door_orientation_horizontal_when_walls_left_right);
    tcase_add_test(tc, test_set_door_orientation_vertical_when_walls_up_down);
    tcase_add_test(tc, test_get_room_loc_accepts_fixed_coordinates);
    tcase_add_test(tc, test_get_room_loc_random_axis_stays_in_room);
    tcase_add_test(tc, test_build_room_creates_room_with_requested_size);
    tcase_add_test(tc, test_maze1xy_returns_untouched_odd_coordinate);
    tcase_add_test(tc, test_fill_empty_maze_skips_fully_used_maze);

    return s;
}

int
main(int argc, char *argv[])
{
    int failed;
    Suite *s = level_suite();
    SRunner *sr = srunner_create(s);

    (void) argc;
    (void) argv;

    /*
     * Each test runs in its own forked process, so the mutable game
     * globals are pristine when the test starts.  Do not disable
     * forking: the level helpers write to level, rooms and SpLev_Map.
     */
    srunner_run_all(sr, CK_VERBOSE);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
