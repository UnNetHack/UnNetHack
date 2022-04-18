-- NetHack Valkyrie Val-filb.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_flags("mazelevel", "icedpools", "noflip")

-- having a map of solid lava rather than level_init is a hack to set the
-- relative coordinate origin to something sane rather than off the map at the
-- in-game 0,0
des.map([[
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
]])

local everything = selection.area(00,00,75,19)
des.region(everything, "lit")
des.replace_terrain({ selection=everything, fromterrain='L', toterrain='.', chance=50 })

-- Stairs on opposite ends
ustairy = nh.rn2(20)
dstairy = nh.rn2(20)
-- & selection.area is needed to keep these from running outside the map...
des.terrain(selection.circle(00, ustairy, 2, 1) & selection.area(00,00,05,19), '.')
des.terrain(selection.circle(75, dstairy, 2, 1) & selection.area(70,00,75,19), '.')
des.stair({ coord={00,ustairy}, dir="up" })
des.stair({ coord={75,dstairy}, dir="down" })

for i=1,10 do
   x1 = nh.rn2(76)
   x2 = nh.rn2(76)
   y1 = nh.rn2(20)
   y2 = nh.rn2(20)
   des.replace_terrain({ selection=selection.randline(x1,y1,x2,y2,30), fromterrain='L', toterrain='.' })
   if percent(10) then
      if percent(50) then
         des.replace_terrain({ selection=selection.circle(x1,y1,d(2),1), fromterrain='L', toterrain='.' })
      else
         des.replace_terrain({ selection=selection.circle(x2,y2,d(2),1), fromterrain='L', toterrain='.' })
      end
   end
end

-- path guaranteed between stairs: as long as stairs are not connected, put down
-- random path between them
local fromustair = selection.new()
while true do
   -- circles of floor around the stairs mean that the points immediately next
   -- to the stairs are guaranteed to be floor
   fromustair = selection.floodfill(01,ustairy,true)
   if fromustair:get(74,dstairy) == 1 then
      break
   end
   -- if not...
   des.replace_terrain({ selection=selection.randline(00,ustairy,75,dstairy,50), fromterrain='L', toterrain='.' })
end

-- now remove disconnected bits of floor that the hero might teleport onto
des.replace_terrain({ selection=fromustair:negate(), fromterrain='.', toterrain='L' })

-- Lots of giants live here, so naturally there will be plenty of boulders (also
-- helps if the hero wants to fill some lava spaces)
for i = 1, 18+d(4) do
   des.object('boulder')
end

-- Monsters (no frost giants here)
for i = 1, 6+d(4) do
   if percent(5) then
      des.monster('fire elemental')
   elseif percent(30) then
      des.monster('fire vortex')
   else
      des.monster('fire ant')
   end
end
des.monster('a')
-- if percent(4) then
--   des.monster('phoenix')
-- end
for i = 1, 2+d(3) do
   des.monster({ id='fire giant', peaceful=false, inventory=function()
      -- Objects are carried by giants.
      for j = 1, d(2) do
         des.object()
      end
   end })
end

-- Random traps
for i = 1, 8 do
   des.trap("fire")
end
for i = 1, 3 do
   des.trap()
end
