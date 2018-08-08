-- NetHack Monk Mon-fila.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = ",", lit=1 });

des.level_flags("mazelevel", "hardfloor", "outdoors")

-- Scatter a few trees
des.replace_terrain({ region={00,00,79,20}, fromterrain=",", toterrain="T", chance=4 })

-- Stairs on opposite edges of the level
local leftedge = selection.line(01,00, 01,21)
local rightedge = selection.line(77,00, 77,21)
local leftstair = leftedge:rndcoord()
local rightstair = rightedge:rndcoord()

-- A few rivers running across top to bottom
-- TODO: sort the x and y coordinates so they probably won't intersect
local topedge = selection.line(01,00, 77,01)
local botedge = selection.line(01,20, 77,21)
for i=1,math.random(1,3) do
   local toppt = topedge:rndcoord()
   local botpt = botedge:rndcoord()
   local river = selection.randline(toppt.x, 0, botpt.x, 21, 8)
   des.terrain(river, "}")
end

-- make a path from one stair to the other
local lstairy = leftstair.y
local rstairy = rightstair.y
local path = selection.randline(1, lstairy, 77, rstairy, 20)
des.terrain(path, ".")

-- Now place stairs, to avoid overwriting them with the path
des.stair({ dir = "up", coord = leftstair })
des.stair({ dir = "down", coord = rightstair })

-- scatter a few rocks
for i=1,8 + d(2,4) do
  des.object("boulder")
end
-- Some random traps
for i=1,5 + d(3) do
  des.trap()
end

-- Monsters
for i=1,2 + d(3) do
  des.monster({ class="E", peaceful=0 })
end
for i=1,3 do
  des.monster({ class="xorn", peaceful=0 })
end

-- Objects
for i=1,2 + d(2) do
  des.object()
end

