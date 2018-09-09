-- NetHack Barbarian Bar-goal.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "outdoors");

des.map([[
.................................................|.........|................
.................................................---.....---BB-----.........
...................................................--....|....|...|.........
...............................---..................---..|..---...+.........
...............................|.|........----------B.-+--..|.+...|.........
...............................|.---+--...|........|........-------.........
...............................|......|...|........|.............B..........
...............................|......|...|.-+----+-............------......
...............................--------..----..|................|....--+-...
.........................................|.....+................+...--..|...
.........................................-------................|...|...|...
..........................................B........--+-..-----..---------...
......................................-----------BB|..|...+..--....B........
......................................|.........|..|..---.|...--+-----......
......................................|.........+..|....|B---........|......
......................................|.........|..------...|....-----......
......................................----...----...........------..........
.........................................|...|..............................
.........................................--+--..............................
............................................................................
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,19), "unlit")

-- Don't levelport or fall into the interesting part of the level
des.teleport_region({ region = {00,00,75,19}, exclude = {06,00,75,19}, dir="down" })

-- This is a bit of a hack to prevent the hero from walking beyond x=6 and then
-- being able to teleport to Thoth Amon straight away. You can't enter this
-- level from below, but having dir=up will somehow keep both regions distinct
-- and active at the same time.
des.teleport_region({ region = {00,00,75,19}, exclude = {41,00,75,19}, dir="up" })

-- Stair
local leftedge = selection.line(00,00, 00,19)
des.stair("up", leftedge:rndcoord(1))

-- Define a selection containing all the building interiors for later use
local inbuildings = selection.floodfill(32,05) | selection.floodfill(50,00) |
                    selection.floodfill(45,06) | selection.floodfill(45,09) | 
                    selection.floodfill(45,13) | selection.floodfill(53,13) |
                    selection.floodfill(60,13) | selection.floodfill(65,02) |
                    selection.floodfill(65,08) | selection.floodfill(70,09) |
                    selection.floodfill(61,04)

-- Grass... none in the village or buildings, more outside it
local grass = selection.gradient({ type="radial", mindist=6, maxdist=60,
                                   x=55, y=8, x2=59, y2=8 }):negate() - inbuildings
              -- & selection.negate(inbuildings)
des.replace_terrain({ selection = grass, fromterrain=".", toterrain="," })

-- Make some trees
des.replace_terrain({ region = {00,00,31,19}, fromterrain=",", toterrain="T", chance = 2 })

-- Interior doors. Exterior doors are all gone, but still have + on the map to
-- be a doorway.
des.door("closed",62,04)
des.door("closed",45,07)

-- Thoth Amon and his treasure
des.monster("Thoth Amon", 57,08)
des.object({ id = "luckstone", coord = {57,08}, buc="blessed", name = "The Heart of Ahriman" })

-- Thoth Amon's attendants
local towncenter = selection.floodfill(57,08)
for i=1,3 do
  des.monster({ id = "ogre king", coord = towncenter:rndcoord(1), peaceful = 0 })
end
for i=1,2 do
  des.monster({ id = "orc-captain", coord = towncenter:rndcoord(1), peaceful = 0 })
  des.monster({ id = "Olog-hai", coord = towncenter:rndcoord(1), peaceful = 0 })
end
des.monster({ id = "kobold lord", coord = towncenter:rndcoord(1), peaceful = 0 })

-- Wish there were some higher-level rampaging-type monsters that are also spellcasters...
des.monster({ id = "orc shaman", coord = {57,05}, asleep = 1, peaceful = 0 })
des.monster({ id = "orc shaman", coord = {53,07}, asleep = 1, peaceful = 0 })
des.monster({ id = "orc shaman", coord = {61,07}, asleep = 1, peaceful = 0 })
des.monster({ id = "orc shaman", coord = {54,10}, asleep = 1, peaceful = 0 })
des.monster({ id = "orc shaman", coord = {60,10}, asleep = 1, peaceful = 0 })

-- Thoth Amon's horde
local outsidetown = selection.floodfill(01,01)
local hordestart = selection.fillrect(31,00,41,19) & outsidetown
for i=1,8 do
  des.monster({ id = "ogre", coord = hordestart:rndcoord(1), peaceful = 0 })
  des.monster({ id = "rock troll", coord = hordestart:rndcoord(1), peaceful = 0 })
end
for i=1,6 do
  des.monster({ class = "O", coord = hordestart:rndcoord(1), peaceful = 0 })
  des.monster({ class = "T", coord = hordestart:rndcoord(1), peaceful = 0 })
end
for i=1,4 do
  des.monster({ class = "o", coord = hordestart:rndcoord(1), peaceful = 0 })
end
for i=1,3 do
  des.monster({ class = "k", coord = hordestart:rndcoord(1), peaceful = 0 })
end

-- Ominous candles on the ground to see their approach
local candleline = selection.area(26,00, 30,19)
for i=1,1 + d(5) do
  des.object({ id = "tallow candle", coord = candleline:rndcoord(1), lit = 1 })
end

-- Random objects, scattered through the buildings.
for i=1,14 do
  des.object(inbuildings:rndcoord(1))
end

-- Traps. Thoth Amon has invested heavily in intruder detection
local interiors = selection.new()
interiors = interiors | towncenter
interiors = interiors | inbuildings
for i=1,10 do
  des.trap("board", interiors:rndcoord(1))
  des.trap("board", outsidetown:rndcoord(1))
  des.trap("bear")
  des.trap("spiked pit")
end

-- Fire ring in the village center
des.object({ id = "rock", x=56, y=06, quantity = d(3,2) })
des.object({ id = "rock", x=57, y=06, quantity = d(3,2) })
des.object({ id = "rock", x=58, y=06, quantity = d(3,2) })
des.object({ id = "rock", x=55, y=07, quantity = d(3,2) })
des.object({ id = "rock", x=59, y=07, quantity = d(3,2) })
des.object({ id = "rock", x=54, y=08, quantity = d(3,2) })
des.object({ id = "rock", x=60, y=08, quantity = d(3,2) })
des.object({ id = "rock", x=55, y=09, quantity = d(3,2) })
des.object({ id = "rock", x=59, y=09, quantity = d(3,2) })
des.object({ id = "rock", x=56, y=10, quantity = d(3,2) })
des.object({ id = "rock", x=57, y=10, quantity = d(3,2) })
des.object({ id = "rock", x=58, y=10, quantity = d(3,2) })

-- Barricades blocking access to the buildings
des.object("boulder", 60,01)
des.object("boulder", 61,01)
des.object("boulder", 59,02)
des.object("boulder", 60,02)
des.object("boulder", 61,02)
des.object("boulder", 52,04)
des.object("boulder", 53,04)
des.object("boulder", 52,05)
des.object("boulder", 53,05)
des.object("boulder", 64,06)
des.object("boulder", 65,06)
des.object("boulder", 39,08)
des.object("boulder", 40,08)
des.object("boulder", 42,11)
des.object("boulder", 43,11)
des.object("boulder", 44,11)
des.object("boulder", 49,12)
des.object("boulder", 50,12)
des.object("boulder", 49,13)
des.object("boulder", 50,13)
des.object("boulder", 57,14)
des.object("boulder", 57,15)
des.object("boulder", 65,12)
des.object("boulder", 66,12)
des.object("boulder", 67,12)

-- Some more random scattered boulders.
for i=1,8 + d(2,4) do
  des.object("boulder");
end
