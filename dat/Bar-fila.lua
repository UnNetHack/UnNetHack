-- NetHack Barbarian Bar-fila.lua	$NHDT-Date: 1652195999 2022/05/10 15:19:59 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_flags("mazelevel", "outdoors", "hardfloor", "inaccessibles", "noflip");

-- TODO: Why does mines-style open up x=1 and x=79 whereas solidfill doesn't?
-- And then, specifying e.g. x=0 in commands in this file actually comes out to x=1 in game...
des.level_init({ style = "mines", fg = ".", bg=".", lit=1, walled=false });

-- let's make some mesa-like rock promontories
local mesa_centers = selection.fillrect(08,00,72,20)
for i=1, 12+d(4) do
   local ctr = mesa_centers:rndcoord()
   local rock = selection.gradient({ type="radial", mindist=0, maxdist=3, limited=true, x=ctr.x, y=ctr.y })
   rock = rock & selection.circle(ctr.x, ctr.y, 3);

   if percent(50) then
      rock = rock:grow("north")
   else
      rock = rock:grow("east")
   end
   rock = rock:grow()
   des.terrain({ selection = rock, typ = ' ', lit = 1 })
end

-- guarantee a way across the level
local leftstair = selection.line(0,0, 0,20):rndcoord()
local rightstair = selection.line(78,1, 78,20):rndcoord()
des.stair({ dir = "up", coord = leftstair })
des.stair({ dir = "down", coord = rightstair })

local path = selection.randline(0, leftstair.y, 78, rightstair.y, 10)
des.terrain({ selection = path:grow("north"):grow("south"), typ = ".", lit = 0 })

for i=1,8 do
  des.object()
end
for i=1,4 do
  des.trap()
end
for i=1,3 do
  des.monster({ id="ogre", peaceful=0 })
end
des.monster({ class="O", peaceful=0 })
des.monster({ class="T", peaceful=0 })

-- wallify the rock promontories
-- we don't want to wallify the stone left and right map edges, so need to limit the range
des.wallify()
