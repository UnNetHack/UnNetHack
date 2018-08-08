-- NetHack Ranger Ran-strt.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Orion,
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = "." });

des.level_flags("mazelevel");

des.level_flags("noteleport", "hardfloor", "outdoors")
des.map({ halign = "left", map = [[
TTTTT..TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT.TT............}...................
TTTT....TTTTTTTT..TTT.........TT......TTTT.T............}...................
TTT..TTTTTT..TTTT..T..TT...TT....TTT....TT...............}..................
T.TT....TTT....TTT...TT.......TT.TT.....TTTT.............}..................
TT...TT.....TT..TT..TTT..TT..TTT.......TTTT.............}.}.................
TTT..TTTT.T.............TTTT......T...TTTTT.............}.}.................
TT....TT..TTTTTT........TTT.....TT......TTTT.............}..................
T..TT..T...TTTT..TTT...TTT....TTTTT......TTT..............}.................
T..TT..T.TTTTTTFFTTTFFFTTTFFTTTTTTTTT.....TT...............}................
TT.TTTTTTTT...................TTT.TTTT.....................}................
TT.....TTT........................TTTTTT..TT...............}................
TT..TTTTTTT...................TT...TTTTTTTTT................}...............
TTT...TTTTTTTTTFFTTTFFFTTTFFTTTT....TTT...TT.................}..............
TTTT....TTTTTTT..TTTT...TT...TTT.....TT...TT.................}..............
TTT..........T...TTT....T..T..T...T...TT..T..................}..............
TT...TT..TTT...T..T..T.....TT...TTTT..TT.TT.................}...............
TT..TTTTTTTT.TTTT....TT...TTT....TTTT....TTT...............}.}..............
TT.....TTT..TTTT.....TTTTTTT.......T....TT.T..............}...}}............
TTT.....TT..TTTT..TT....TT....TT......TTTTT...............}.....}}..........
TTTTTT..TTTTTTTTTTTTTT......TTTTT...TTTTT.T..............}........}}}.......
TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT..T............}............}......
]] })
des.region(selection.area(00,00,77,21), "lit")
des.replace_terrain({ region = {43,00,47,21}, fromterrain=".", toterrain="T", chance=30 })
des.replace_terrain({ region = {43,00,47,21}, fromterrain=".", toterrain="T", chance=30 })
des.replace_terrain({ region = {48,00,52,20}, fromterrain=".", toterrain="T", chance=15 })
des.replace_terrain({ region = {53,00,77,20}, fromterrain=".", toterrain="T", chance=4 })

-- Stairs
local rightedge = selection.line(77,00, 77,21)
local dnstair = rightedge:rndcoord()
des.stair("down", dnstair)

-- Guarantee a treeless path from the stair to the forest entry
des.terrain({ selection = selection.randline(77, dnstair.y, 43,09, 6), typ=".", lit=1 })

-- Portal arrival point; just about anywhere on the right hand side of the map
des.levregion({ region = {51,2,77,18}, exclude = {0,0,40,20}, region_islev = 1, type="branch" })

-- Altar (entry text says there's one here)
des.altar({ x=11, y=10, align="coaligned", type="altar" })

-- -- Random grass patches
local grass = selection.new()
local everything = selection.area(00, 00, 77, 21)
for i=1,d(4,3) do
   local ctr = everything:rndcoord()
   local fairyring = percent(1)
   local newpatch = selection.circle(ctr.x, ctr.y, 3, fairyring and 0 or 1)
   while percent(50) do
      newpatch:grow("random")
   end
   grass = grass | newpatch
end
des.replace_terrain({ selection=grass, fromterrain='.', toterrain=',' })

-- Orion and his faithful dog
des.monster({ id = "Orion", coord = {20, 10}, inventory = function()
   des.object({ id = "leather armor", spe = 4 });
   des.object({ id = "yumi", spe = 4 });
   des.object({ id = "arrow", spe = 4, quantity = 50 });
end })
des.monster({ id = "large dog", x=20, y=11, name="Sirius", peaceful=1 })
-- The treasure of Orion
des.object({ id = "chest", trapped = 0, x=20, y=10,
             contents = function()
                des.object({ id = "bow", buc = "blessed" })
                des.object({ id = "arrow", spe = 2, quantity = 40 })
                des.object()
                des.object()
                des.object()
                des.object()
                des.object()
                des.object()
             end
})

-- Guards for the audience chamber
des.monster("hunter", 19, 09)
des.monster("hunter", 20, 09)
des.monster("hunter", 21, 09)
des.monster("hunter", 19, 10)
des.monster("hunter", 21, 10)
des.monster("hunter", 19, 11)
des.monster("hunter", 20, 12)
des.monster("hunter", 21, 11)

-- Non diggable trees
des.non_diggable(selection.area(00,00,40,20))

-- Traps
des.trap("arrow",30,10)
des.trap("pit",40,09)
des.trap("spiked pit")
des.trap("bear")
des.trap("bear")

-- A few other scattered objects
for i=1,2 + d(3) do
  des.object()
end

-- Monsters on siege duty.
des.monster({ id = "forest centaur", x=19, y=03, peaceful = 0 })
des.monster({ id = "forest centaur", x=19, y=04, peaceful = 0 })
des.monster({ id = "forest centaur", x=19, y=05, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=03, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=04, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=05, peaceful = 0 })
des.monster({ id = "forest centaur", x=01, y=09, peaceful = 0 })
des.monster({ id = "forest centaur", x=02, y=09, peaceful = 0 })
des.monster({ id = "forest centaur", x=03, y=09, peaceful = 0 })
des.monster({ id = "forest centaur", x=01, y=11, peaceful = 0 })
des.monster({ id = "forest centaur", x=02, y=11, peaceful = 0 })
des.monster({ id = "forest centaur", x=03, y=11, peaceful = 0 })
des.monster({ id = "forest centaur", x=19, y=15, peaceful = 0 })
des.monster({ id = "forest centaur", x=19, y=16, peaceful = 0 })
des.monster({ id = "forest centaur", x=19, y=17, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=15, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=16, peaceful = 0 })
des.monster({ id = "forest centaur", x=21, y=17, peaceful = 0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "plains centaur", peaceful=0 })
des.monster({ id = "scorpion", peaceful=0 })
des.monster({ id = "scorpion", peaceful=0 })
