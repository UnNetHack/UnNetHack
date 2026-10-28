/*
 * Tests for the .des special-level loader in src/spo_lev.c.
 *
 * The implementation file is included so that its static helpers can be
 * exercised directly.  The build must therefore link spo_lev.o out.
 */

#include <check.h>

#define TEST_SPO_LEV 1
#define IMPLEMENTATION_NAME "spo_lev"

#include "../../src/spo_lev.c"
#include "level_tests.inc"
