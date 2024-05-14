-- NetHack Archeologist Arc-loca.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "hardfloor", "outdoors", "noteleport", "nommap", "noflipx")
des.map([[
.................................T....|.--T.....--. -------------------------
.................................--.....|....--....HS.|.....|.........|.|.|.|
....................................T......|....-.. |--.|...S..-----..|.....|
......................................|..----T..    ---.|...|.--...---|.....|
.................................-|...|......  ------.---S--|.|.....|.S.....|
...................................-------  H  |..|.....|...|.|.....|.|.|.|.|
........................................|.  ##HS..---...|--S----...--.---S--|
...........................................    ----.|...|......-----......|.|
........................................|.......S#S.|...|......|.|.---...---|
........................................|.....    -----S-......|.|...|...S..|
........................................   ....----..|..S.--S---.---------..|
.................................------  .  .. |.....|S-----..|..S.....|.|..|
.................................T...... .   .HS.....|..|..-----------S-S---|
...................................---...   |  |.....|..|.....|..S...|...|# |
...................................|........|..----..|--|..--------.---.--##|
......................................T.|.......  ----..|---......|..|...| #|
.................................|...|..|...|....   |...|.|.......|.--...--#|
....................................T|..|...|......#S...|.S.......|.|.....S#|
.................................---....|......     -----.|.......|.|.....| |
......................................T...T   #######S....--......-----------
]]);
-- Dungeon Description
local outside = selection.floodfill(00,00)
local inside = outside:clone():negate():filter_mapchar('.')
des.region(outside, "lit")

-- Non diggable walls (but leave all trees diggable)
local nondig = selection.area(00,00,75,19) ~ selection.area(00,00,50,19):filter_mapchar('T')
des.non_diggable(nondig)

-- Jungle filler on the left side
local forest_max_x = 40
local trees = selection.gradient({ type="square", x=forest_max_x, y=00, x2=forest_max_x, y2=19, maxdist=60, limited=false }):negate()
trees = trees & selection.area(00,00,forest_max_x,19)
des.terrain({ selection=trees, typ='T' })

-- Upstair
local ustairy = nh.rn2(16) + 2 -- 2..18
des.stair("up", 00, ustairy)

-- Crude path through the jungle
local path = selection.randline(00, ustairy, 34, 08, 50)
path = path | path:clone():percentage(70):grow("north")
path = path | path:clone():percentage(70):grow("south")
des.replace_terrain({ selection=path, fromterrain='T', toterrain='.' })

-- Also clear off trees and force levelporting to land by the stairs
local stairbox = selection.area(00, ustairy-1, 01, ustairy+1)
des.replace_terrain({ selection=stairbox, fromterrain='T', toterrain='.' })
des.teleport_region({ region={00,ustairy-1, 01, ustairy+1}, dir="down" })

-- Fill in inaccessible pockets in forest
local nostairaccess = selection.floodfill(01, ustairy, true):negate() & selection.area(00,00,31,19)
des.replace_terrain({ selection=nostairaccess, fromterrain='.', toterrain='T' })

-- Randomize the topology somewhat
function pickonepoint(list)
   local tmpsel = selection:new()
   for i=1,#list do
      tmpsel:set(list[i][1], list[i][2])
   end
   return tmpsel:rndcoord()
end
des.terrain({ coord = pickonepoint({{75,06},{74,07},{75,08}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{64,07},{66,07},{68,05}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{61,06},{65,02},{69,03}}), typ='.' }) -- remove wall
des.terrain({ coord = pickonepoint({{62,11},{70,02}}), typ='.' }) -- remove wall
des.terrain({ coord = pickonepoint({{50,05},{53,13}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{64,14},{66,16},{66,17},{66,18}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{61,12},{62,13}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{72,10},{74,12}}), typ='S' }) -- door
des.terrain({ coord = pickonepoint({{68,08},{69,09}}), typ='S' }) -- door

if percent(50) then
   des.terrain(53, 02, '-')
   des.terrain(54, 02, '-')
else
   des.terrain(54, 14, '-')
   des.terrain(55, 14, '-')
end

-- Rolling boulder trap room
-- Original design of this level called for six traps, one on each of the 6
-- spaces with height 5 in this room, with a vertically rolling boulder.
-- We can now specify launch coordinates on rolling boulder traps, but the
-- problem with this is that rolling boulder traps get created with their
-- secondary launch2 point equidistant on the opposite side of the trap as
-- launch1. This would mean in order for this room to behave as desired (and not
-- segfault when a boulder tries to roll off the map), the traps would be in a
-- predictable line down the middle, which isn't interesting.
-- Instead, just put a bunch of traps in the center of the room and hope that
-- most of them will generate appropriate coordinates and a boulder.
local bouldertraps = selection.area(60,16,65,18)
for i=1,8 do
   des.trap("rolling boulder", bouldertraps:rndcoord(1))
end

-- Some loot.
-- Is it a good idea to take items from here? Does it amount to grave-robbing?
-- Or are you just preserving the historical artifacts?
local chestloc = selection:new()
chestloc:set(75,07)
chestloc:set(63,11)
chestloc:set(70,15)
chestloc:set(72,15)
chestloc:iterate(function(x, y)
   local template = { id = 'chest', coord = {x,y} }
   if percent(40) then
      template['material'] = 'gold'
   end
   des.object(template)
end)

-- Altars of every god
local altars = { {73,02}, {71,17}, {68,13}, {52,12}, {64,05} }
shuffle(altars)
des.altar({ coord=altars[1], align=align[1] })
des.altar({ coord=altars[2], align=align[2] })
des.altar({ coord=altars[3], align=align[3] })

-- The downstair occupies one of the unused altar spots
des.stair({ coord=altars[4], dir='down' })

-- Mummy-trapped chests (note the spe=3 on them)
local box_coords={ {57,13}, {54,16}, {71,08}, altars[5] }
shuffle(box_coords)
des.object({ id='chest', material='gold', coord=box_coords[1], spe=3, trapped = 1,
             contents = function()
                des.object({ class='"', material='gold' })
             end
})
des.object({ id='chest', material='gold', coord=box_coords[2], spe=3, trapped = 1,
             contents = function()
                -- otyp = percent(40) and 'magic flute' or 'flute'
                des.object({ id='flute', material='gold' })
             end
})
des.object({ id='chest', material='gold', coord=box_coords[3], spe=3, trapped = 1,
             contents = function()
                des.object({ id='etched helmet', material='gold' })
             end
})
des.object({ id='chest', material='gold', coord=box_coords[4], spe=3, trapped = 1,
             contents = function()
                des.object({ id='gold piece', quantity=d(3,500) })
                for i=4,d(4,4) do
                   des.object('*')
                end
             end
})

-- Trap room
selection.area(57,07,62,09):iterate(function(xx,yy)
   if percent(50) then
      des.trap({ x=xx, y=yy, spider_on_web=false })
   end
end)

-- More traps in general
for i=1,8 do
   des.trap("spiked pit")
end
interior_spaces = selection.area(48,00,76,19):filter_mapchar('.')
for i=1,11 do
   des.trap({ coord = interior_spaces:rndcoord(1), spider_on_web = false })
end
for i=1,d(3) do
   des.trap('fire')
end

-- Cave-ins, or possibly deliberate sealed corridors
local boulders = selection.area(53,06,55,08) | selection.area(68,07,70,07)
boulders:iterate(function(x,y)
   des.object('boulder', x, y)
end)

-- A few random other boulders, could be outside or inside
for i=1,4 do
   des.object("boulder")
end

-- Obligatory mummies
des.monster({ id='human mummy', x=55, y=03, waiting=1 })
des.monster({ id='human mummy', x=53, y=04, waiting=1 })
des.monster({ id='human mummy', x=52, y=10, waiting=1 })
des.monster({ id='human mummy', x=52, y=14, waiting=1 })
des.monster({ id='human mummy', x=57, y=16, waiting=1 })
des.monster({ id='human mummy', x=63, y=05, waiting=1 })
des.monster({ id='human mummy', x=67, y=04, waiting=1 })
des.monster({ id='human mummy', x=66, y=08, waiting=1 })
des.monster({ id='human mummy', x=71, y=01, waiting=1 })
des.monster({ id='human mummy', x=75, y=01, waiting=1 })
des.monster({ id='human mummy', x=71, y=03, waiting=1 })
des.monster({ id='human mummy', x=75, y=03, waiting=1 })
des.monster({ id='human mummy', x=71, y=05, waiting=1 })
des.monster({ id='human mummy', x=75, y=05, waiting=1 })
des.monster({ id='human mummy', x=72, y=11, waiting=1 })
des.monster({ id='human mummy', x=75, y=09, waiting=1 })
des.monster({ id='human mummy', x=75, y=10, waiting=1 })
des.monster({ id='human mummy', x=75, y=11, waiting=1 })
des.monster({ id='human mummy', x=68, y=15, waiting=1 })
des.monster({ id='human mummy', x=69, y=18, waiting=1 })
des.monster({ id='human mummy', x=71, y=18, waiting=1 })
des.monster({ id='human mummy', x=73, y=18, waiting=1 })

-- Other monsters
local valid_outside = outside:clone():filter_mapchar('.')
for i=1,d(3,2) do
   des.monster('S', valid_outside:rndcoord(1))
end
des.monster('f', valid_outside:rndcoord(1))
des.monster('Y', valid_outside:rndcoord(1))
for i=1,d(5) do
   des.monster('giant bat', inside:rndcoord())
end

-- Loot cache buried not very deep
des.object({ id='gold piece', quantity=d(3,50), coord={41,11} })

-- Statues, mostly outside, some inside. Stone golems because there aren't
-- really specific monster types for these things.
local statue_coords = {{34,07}, {36,07}, {38,07}, {34,09}, {36,09}, {38,09},
                       {37,00}, {39,04}, {41,08}, {36,15}, {43,10}, {42,15},
                       {42,17}, {49,11}, {49,13}, {51,10}, {51,14}, {59,01},
                       {59,03}, {64,03}, {65,03}, {66,03}, {64,06}, {65,06},
                       {66,06}}
for i=1,#statue_coords do
   local mon = percent(90) and 'stone golem' or 'clay golem'
   des.object({ id='statue', montype=mon, material='stone', historic=true, coord=statue_coords[i] })
end

-- Treasure?
des.engraving({ type="engrave", coord=outside:filter_mapchar('.'):rndcoord(), text="X marks the spot." })
