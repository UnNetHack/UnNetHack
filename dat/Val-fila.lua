-- NetHack Valkyrie Val-fila.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_flags("mazelevel", "outdoors", "icedpools", "noflipx")

-- having a map of solid ice rather than level_init is a hack to set the
-- relative coordinate origin to something sane rather than off the map at the
-- in-game 0,0
des.map([[
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
]])

local everything = selection.area(00,00,75,19)
des.region(everything, "lit")
des.replace_terrain({ selection=everything:percentage(20), fromterrain='I', toterrain='.' })

-- Stairs on opposite ends
des.stair("up", selection.area(00,00,00,19):rndcoord())
des.stair("down", selection.area(75,00,75,19):rndcoord())

-- More random lava vents like in home level
local middleground = selection.area(08,00,68,19)
local pools = selection.new()
for i = 1,15 do
   local pt = middleground:rndcoord()
   local newpool = selection.new():set(pt.x, pt.y)
   -- maybe turn into a bigger one
   for j = 1, d(8)-5 do
      newpool = newpool:grow('random')
   end
   pools = pools | newpool
end
local water = pools:clone():grow("all")
des.replace_terrain({ selection=water, fromterrain='I', toterrain='P' })
des.terrain(pools, "L")

-- Unfrozen lake
lakex = 20 + nh.rn2(25)
lakey = 19
xrad = 6
yrad = 2
lake = selection.ellipse(lakex, lakey, xrad, yrad, 1)
lake = lake + selection.ellipse(lakex, lakey, xrad + 1, yrad + 1, 1):percentage(70)
lake = lake + selection.ellipse(lakex, lakey, xrad + 2, yrad + 2, 1):percentage(40)
des.terrain(lake, '}')
-- remove random separated patches of water
des.terrain(lake - selection.floodfill(lakex, lakey), 'I')

-- A few random boulders (giants will probably pick some up)
for i = 1,5+d(3) do
   des.object('boulder')
end

-- Monsters
des.monster("fire ant")
for i = 1,2+d(3) do
   -- No objects scattered on the floor here, instead they are in the giants'
   -- inventories.
   template = { peaceful = 0, inventory = function()
      for j = 1,d(2) do
         des.object()
      end
   end }
   local v = math.random(0,99)
   if v < 45 then
      template['id'] = 'fire giant'
   elseif v < 90 then
      template['id'] = 'frost giant'
   else
      template['class'] = 'H'
   end
   des.monster(template)
end

-- Random traps
for i=1,6+d(3) do
   des.trap({ spider_on_web = false })
end
