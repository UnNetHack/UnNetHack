-- NetHack Valkyrie Val-strt.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, the Norn,
--	and receive your quest assignment.
--

des.level_flags("mazelevel", "noteleport", "hardfloor", "icedpools", "outdoors", "noflipx")
des.level_init({ style = "solidfill", fg = "I" })

des.map([[
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIII...IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII...IIIIIIIII
IIIIIIIIIIIIIII.....IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII.......IIIIIIII
IIIIIIIIIIIIIII....IIIIIIIIIIIIIIIIIII-----IIIIIIIIIIIIII.....II...IIIIIIIII
IIIIIIIIIIIIIII.IIIIIIIIIIIIII--------|...|IIIIIIIIII.....IIIIIII.IIIIIIIIII
IIIIIIIIIIIIIII.IIIIIIIIIIIIII|.|.|.|.|...|IIIIIIIIII.IIIIIIIIIII.IIIIIIIIII
IIIIIIIIIIIIIII..IIIIIIIIIIIII|.......|...|IIIIIIIIII.IIIIIIIIIII..IIIIIIIII
IIIIIIIIIIIIIIII.IIIIIIIIIIII-----+-----+----IIIIIIII.IIIIIIIIIIII.IIIIIIIII
IIIIIIIIIIIIIIII..IIIIIIIII.--..............--IIIIIII..IIIIIIIIIII.IIIIIIIII
IIIIIIIIIIIIIIIII.IIIIIIII..|................|IIIIIIII.IIIIIIIIIII..IIIIIIII
IIIIIIIIIIIIIIIII.IIIII.....+................|IIIIIIII.IIIIIIIIIIII.IIIIIIII
IIIIIIIIIIIIIIIII.III...II..|................|IIIII...........IIIII.IIIIIIII
IIIIIIIIIIIIIIIII.....IIIII.--..............--......IIIIIIIII.......IIIIIIII
IIIIIIIIIIIIIII...IIIIIIIIIII--------.-----+-..IIIIIIIIIIIIIIIIIIII...IIIIII
IIIIIIIIIIIII...IIIIIIIIIIIIIIIIIII--.--II....IIIIIIIIIIIIIIIIIIIIIII...IIII
IIIIIIIIIII...IIIIIIIIIIIIIIIIIIII--...--IIIIIIIIIIIIIIIIIIIIIIIIIIIIII.....
I.......I...IIIIIIIIIIIIIIIIIIIIII|..T..|IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
..........IIIIIIIIIIIIIIIIIIIIIIII--...--IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
I........IIIIIIIIIIIIIIIIIIIIIIIIII-----IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
]]);
-- Dungeon Description
local everything = selection.area(00,00,75,19)
local alltheice = everything:filter_mapchar('I')
local inside = selection.floodfill(34,10) + selection.floodfill(34,06) + selection.floodfill(40,06)
local outside = (everything:filter_mapchar('.') + alltheice) - inside
local init_monster_area = outside & selection.area(17,00,75,19):grow()
des.region(everything, "lit")
-- Lava vents surrounded by water
local pools = selection.new()
local function growths()
   -- Compute the number of times a random lava vent should grow in size.
   if percent(40) then
      return 1 + growths()
   end
   return 0
end
for i = 1,13 do
   local pt = alltheice:rndcoord()
   local newpool = selection.new():set(pt.x, pt.y)
   -- maybe turn into a bigger one
   for j=1,growths() do
      newpool = newpool:grow('random')
   end
   pools = pools | newpool
end
-- Only let them affect ice.
local water = pools:clone():grow("all")
des.terrain(water & alltheice, "P")
des.terrain(pools & alltheice, "L")
-- And now there's less ice.
alltheice = alltheice - water
init_monster_area = init_monster_area - water

-- Portal arrival point
des.levregion({ region = {02,17,02,17}, type="branch" })
-- Stairs
des.stair("down", 75, 15)
-- One fountain
if percent(50) then
   des.feature("fountain", 65,02)
else
   des.feature("fountain", 17,02)
end
-- Doors
des.door({ state="locked", coord={28,10}, iron=1 })
des.door("closed",34,07)
des.door("locked",40,07)
des.door({ state="locked", coord={43,13}, iron=1 })
-- Norn
des.monster({ id = "Norn", coord = {42, 10}, inventory = function()
   des.object({ id = "banded mail", spe = 5 });
   des.object({ id = "long sword", spe = 4 });
end })
-- The treasure of the Norn
des.object({ id="chest", x=43, y=08, contents = function()
   des.object({ class='+', id='cone of cold' })
end})
des.object({ id="spear", coord={39,04}, spe=d(3) })
des.object(")", 39, 05)
des.object(")", 39, 06)
des.object({ id="small shield", coord={41,04}, spe=3 })
des.object("[", 41, 05)
des.object("[", 41, 06)
-- Valkyrie guards posted outside
des.monster({ id="warrior", coord={27,09}, waiting=1 })
des.monster({ id="warrior", coord={27,11}, waiting=1 })
-- Valkyrie guards inside the hall
des.monster("warrior", 34, 09)
des.monster("warrior", 34, 11)
des.monster("warrior", 40, 09)
des.monster("warrior", 40, 11)
des.monster({ id="warrior", coord={44,09}, waiting=1 })
des.monster({ id="warrior", coord={44,11}, waiting=1 })
-- Guards off shift in the bunkroom
des.monster({ id="warrior", coord={33,05}, asleep=1 })
des.monster({ id="warrior", coord={37,05}, asleep=1 })
-- 
-- Non diggable walls
des.non_diggable(selection.area(26,03,45,18))
-- Random traps
for i=1,8 do
   des.trap("fire", alltheice:rndcoord(1))
end
-- Scatter a few boulders too (extra ammo for giants!)
for i=1,d(3)+2 do
   des.object("boulder", alltheice:rndcoord(1))
end
-- Monsters on siege duty.
for i=1,d(2) do
   des.monster('fire ant', init_monster_area:rndcoord(1))
end
for i=1,d(2)-1 do
   des.monster('fire vortex', init_monster_area:rndcoord(1))
end
for i=1,d(3) do
   des.monster('fire giant', init_monster_area:rndcoord(1))
end
-- Ragnarok doesn't only feature fire giants.
for i=1,d(3) do
   des.monster('frost giant', init_monster_area:rndcoord(1))
end
