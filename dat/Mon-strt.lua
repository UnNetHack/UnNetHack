-- NetHack Monk Mon-strt.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, the Grand Master
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("noteleport", "mazelevel", "hardfloor", "outdoors")

des.map([[
,,,,T,,,,,,,,,T,,,|,,,,,....,|.}}}.|T,,,,,,.........T....-...T.--....T,,,,,,
,,.,,,,,,,,,,,,,,,|,,,....{.,|}}}...........T..T.......-...........-.,,,T,,,
,...,,,,,,,,,,,T,,|,,.......--.....|T,,,T,,,,........T...--..-..--....T,,,,T
..{..,,,T,,,,,,,,--,....-----...............T.....T...-..--.....--.T...,T,,,
,...,,,,,,,,,,,,,|,,....T|.........|T,,,,,,..................--....---------
,,.,,,,,,,T,,,,,,|,....---.....--------------+-----,......--.--..-..,,,,,,,,
,,.,,,,,,,,,,,,T,|,....T|...-.--..................--,.-...--........T.......
,,.,,,,,,,,,,,,,--,....---....|...-..-..-..-..-....--.........-...........,,
,...,,,T,,T,,T,,|,,......|..-.|.....................|.-...,,....-.......,,,,
.........................+....+.....................|....,,,,.......,--.--,,
.........................+....+.....................|....,,,,..,...,--...--,
,...,,,T,,T,,T,,|,,......|..-.|.....................|.-...,,........|..{..|,
,,,,,,,,,,,,,,,,--,....---....|...-..-..-..-..-....--.........,,....--...--,
,,,,,,,,,,,T,,,,,|,....T|...-.--..................--..-...........,.,--.--,,
,,,,T,,,,,,,,,,,,|,....---.....----------+---------....-----+-----,......,,,
,,,,,,,,,,,,,,,,,|,,....T|............................--K.......K--.....,T,,
,,,,,,,,,T,,,,,,,--,....--------...--------...--......+...........|..,,,,,,,
,,T,,,,,,,,,,,,,,,|,,......,,T,-----,,,|--.....--,,,,,|...}}}}}...+.,,,,,,,,
,,,,,,,,,,,,,,,,T,|,,........,,,,,,,,T,-|.......|T,,,,--}}}}}}}}}--,,,,,,,T,
,,,,,,T,,,,,,,,,,,|,,,,........,,,T,,,,,|-.....-|,,,T,,-----------,,,,T,,,,,
]])

-- Full level is lit
des.region(selection.area(00,00,75,19), "lit")

local spacelocs = selection.floodfill(05,04);

-- Portal arrival point
des.levregion({ region = {02,10,02,10}, type="branch" })

-- Stairs
des.stair("down", 75,06)

-- Doors
des.door({ state = "locked", x=25, y=9, iron=1 } )
des.door({ state = "locked", x=25, y=10, iron=1 } )
des.door("locked", 30,09)
des.door("locked", 30,10)
des.door("locked", 41,14)
des.door("locked", 45,05)
des.door("random", 54,16)
des.door("random", 60,14)
des.door("random", 66,17)

-- Unattended Altar - unaligned due to conflict - player must align it.
des.altar({ x=44,y=18, align="noalign", type="altar" })

-- The Grand Master
des.monster({ id = "Grand Master", coord = {49, 09}, inventory = function()
   des.object({ id = "robe", spe = 6 });
end })
-- guards for the audience chamber
local main_hall = selection.fillrect(31,06,51,13)
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())
des.monster("abbot", main_hall:rndcoord())

-- Monks are ascetics: you won't find anything here that they won't give
-- to a Candidate starting out
des.object({ id = "chest", trapped = 0, coord = {51,08},
             contents = function()
                des.object({ id = "robe", spe = 1, buc = "uncursed" })
                des.object({ id = "leather gloves", spe = 2, buc = "uncursed" })
                des.object({ id = "blindfold", buc = "uncursed" })
             end
          })
des.object({ id = "chest", trapped = 0, coord = {51,11},
             contents = function()
                for i=1,5 do
                   des.object("%")
                end
             end
          })
-- next to leader, so possibly tricky to pick up if not ready for quest yet;
-- there's no protection against a xorn eating these tins; BUC state is random
des.object({ id="tin", coord = {29, 10}, quantity=2, montype="spinach" })

-- Non diggable walls - try to hit as few trees as possible
des.non_diggable(selection.area(16,00,18,19))
des.non_diggable(selection.area(25,00,29,16))
des.non_diggable(selection.area(24,05,24,14))
des.non_diggable(selection.area(30,16,40,19))
des.non_diggable(selection.area(30,05,52,14))

-- A few rocks for scenery flavor
des.object("boulder", 30, 00)
des.object("boulder", 75, 16)
des.object("boulder", 72, 19)

-- And finally, the baddies
outside = selection.fillrect(00,00,16,19)
inside_walls = selection.floodfill(26,09)
for i=1,4 do
  des.monster({ id = "earth elemental", coord = outside:rndcoord(), peaceful = 0 })
  des.monster({ class = "E", coord = inside_walls:rndcoord(), peaceful = 0 })
end
des.monster({ id = "xorn", x=21, y=08, peaceful = 0})
des.monster({ id = "xorn", x=21, y=11, peaceful = 0})
