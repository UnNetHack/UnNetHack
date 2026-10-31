/*
 * Tests for the Lua special-level builder in src/sp_lev.c.
 *
 * The implementation file is included so that its static helpers can be
 * exercised directly.  The build must therefore link sp_lev.o out.
 */

#include <check.h>

#define TEST_SP_LEV 1
#define IMPLEMENTATION_NAME "sp_lev"

#include "../../src/sp_lev.c"
#include "level_tests.inc"
