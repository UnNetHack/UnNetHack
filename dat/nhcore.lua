-- NetHack may be freely redistributed.  See license for details.
-- This file contains lua code used by NetHack core.
-- Is it loaded once, at game start, and kept in memory until game exit.

-- Callback functions
nhcore = {
    -- start_new_game called once, when starting a new game
    -- after "Welcome to NetHack" message has been given.
    -- start_new_game = function() nh.pline("NEW GAME!"); end,

    -- restore_old_game called once, when restoring a saved game
    -- after "Welcome back to NetHack" message has been given.
    -- restore_old_game = function() nh.pline("RESTORED OLD GAME!"); end,

    -- moveloop_turn is called once per turn.
    -- moveloop_turn = mk_dgl_extrainfo,

    -- game_exit is called when the game exits (quit, saved, ...)
    -- game_exit = function() end,
};

