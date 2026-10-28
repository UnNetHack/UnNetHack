all: options/test_base32 \
     hacklib/test_hacklib \
     options/test_options \
     unicode/test_unicode \
     wishing/test_wishing \
     levels/test_sp_lev \
     levels/test_spo_lev

clean:
	rm -f `find */test_* -executable`
	rm -f *.xml */*.xml

TEST_CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable

# ---------------------------------------------------------------------
# Dependencies
#
# Linux distributions package Lua as lua5.4; Homebrew calls it lua.
# Probe both so `make check` works on the CI images and on macOS.
# ---------------------------------------------------------------------

PKG_CONFIG ?= pkg-config

CHECK_CFLAGS = $(shell $(PKG_CONFIG) --cflags check 2>/dev/null)
CHECK_LIBS   = $(shell $(PKG_CONFIG) --libs check 2>/dev/null || echo -lcheck)
LUA_CFLAGS   = $(shell $(PKG_CONFIG) --cflags lua5.4 2>/dev/null || $(PKG_CONFIG) --cflags lua-5.4 2>/dev/null || $(PKG_CONFIG) --cflags lua 2>/dev/null)
LUA_LIBS     = $(shell $(PKG_CONFIG) --libs lua5.4 2>/dev/null || $(PKG_CONFIG) --libs lua-5.4 2>/dev/null || $(PKG_CONFIG) --libs lua 2>/dev/null || echo -llua)
CURSES_LIBS  = $(shell $(PKG_CONFIG) --libs ncurses 2>/dev/null || echo -lncurses)

TEST_INCLUDES = -I../include -I../src $(CHECK_CFLAGS) $(LUA_CFLAGS)

# Existing single-file unit tests: each test includes the source file
# under test through its own mocks.h, so nothing from src/ is linked.
%: %.c
	$(CC) $(TEST_CFLAGS) $(TEST_INCLUDES) -g $(CFLAGS) $@.c \
	    $(LUA_LIBS) $(CHECK_LIBS) -o $@
	./$@

# ---------------------------------------------------------------------
# Level-construction tests.
#
# These link the entire game (minus its main() and the implementation
# under test, which is #included by the test) so that the static helpers
# in sp_lev.c / spo_lev.c can be called directly with real level state.
# ---------------------------------------------------------------------

# Everything the game links except its entry point, the *_util.o helpers
# from other build targets, and the two implementations under test.
LEVEL_OBJS = $(filter-out ../src/unixmain.o %_util.o, $(wildcard ../src/*.o))

LEVEL_CFLAGS = $(TEST_CFLAGS) -Wno-unused-function $(TEST_INCLUDES) \
               $(CFLAGS) -fno-omit-frame-pointer

levels/test_sp_lev: levels/test_sp_lev.c levels/level_tests.inc
	$(CC) $(LEVEL_CFLAGS) -g $< \
	    $(filter-out ../src/sp_lev.o, $(LEVEL_OBJS)) \
	    $(LUA_LIBS) -lm $(CURSES_LIBS) $(CHECK_LIBS) -o $@
	./$@

levels/test_spo_lev: levels/test_spo_lev.c levels/level_tests.inc
	$(CC) $(LEVEL_CFLAGS) -g $< \
	    $(filter-out ../src/spo_lev.o, $(LEVEL_OBJS)) \
	    $(LUA_LIBS) -lm $(CURSES_LIBS) $(CHECK_LIBS) -o $@
	./$@
