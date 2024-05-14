-- NetHack Archeologist Arc-filb.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--

-- a bit kludgy...
XMIN = 2
XMAX = 76
YMIN = 0
YMAX = 19

-- SHIM -- g.xstart is 1, usually
function getmap(x, y)
   return nh.getmap(x+1, y)
end

des.level_flags("noflip", "noteleport", "hardfloor", "inaccessibles", "nommap")

-- Non diggable walls
des.non_diggable(selection.area(00,00,76,20))

-- initial room containing the upstairs
des.room({ type = "ordinary", lit = 0, x = 28 + nh.rn2(20), y = 1 + nh.rn2(14),
           contents = function(rm)
              des.stair("up")
           end
});
-- find upstairs location for use later
local ustairx, ustairy
selection.area(00,00,75,19):iterate(function(x, y)
   if getmap(x, y)['typ_name'] == 'stairs' then
      ustairx = x
      ustairy = y
   end
end)

-- Return a rectangular selection for a room that can be overlaid entirely
-- onto stone or existing walls.
-- One of dx and dy must be zero and the other must be either 1 or -1.
-- If a room of at least 2x2 won't fit here, return all -1 coordinates.
function findroom(xi, yi, maindir, straightline)
   if xi <= XMIN or yi <= YMIN or xi >= XMAX or yi >= YMAX then
      return nil
   end
   local dirs = { north = true, south = true, east = true, west = true}
   -- don't grow backwards
   if maindir == "north" then dirs["south"] = false end
   if maindir == "south" then dirs["north"] = false end
   if maindir == "east" then dirs["west"] = false end
   if maindir == "west" then dirs["east"] = false end
   if straightline then
      dirs = { }
      dirs[maindir] = true
   end

   function rnddir()
      local elig = 0
      local sel = nil
      for dir, bool in pairs(dirs) do
         if bool then
            elig = elig + 1
            if nh.rn2(elig) == 0 then
               sel = dir
            end
         end
      end
      return sel
   end

   local sel = selection.new()
   sel:set(xi, yi, 1)

   local nexpand = d(2,6)
   local growths = 0

   -- bias against lots of tall vertical corridors
   if straightline and (maindir == "north" or maindir == "south") and percent(60) then
      nexpand = math.ceil(nexpand / 2)
   end

   for i=1,nexpand do
      -- randomly pick a direction to grow
      local currdir = rnddir()
      if currdir == nil then
         break
      end
      if i == 1 then
         -- rooms should be at least 2 deep, so force this on the first attempt
         currdir = maindir
      end

      local newsel = sel:clone():grow(currdir)
      local addition = (newsel ~ sel) -- set subtraction would be better, but xor will do
      local invalid = false
      addition:iterate(function(x, y)
         -- don't allow it to go right up against the edge of the map; leave
         -- room for the wall
         if x == XMIN or y == YMIN or x == XMAX or y == YMAX then
            invalid = true
         end
         if getmap(x, y)["typ_name"] ~= "stone" then
            invalid = true
         end
      end)
      if invalid then
         -- can't grow in this direction
         dirs[currdir] = false
         -- potential extension: in this case, cut down on nexpand
      else
         -- can grow
         sel = newsel
         growths = growths + 1
      end
   end

   -- needs to have grown a certain number of times
   -- possible extension: check whether a !straightline room has grown
   -- perpendicularly (not always in maindir)
   if growths > 3 or straightline then
      return sel
   else
      return nil
   end
end

ALLDIRS = { "north", "south", "east", "west" }

local snakerooms = {}

for i=1,60 do
   -- Find an available spot on a wall where a door can be placed leading into a
   -- corridor or another room.
   local wallsN = selection.match(" \nw\n.")
   local wallsS = selection.match(".\nw\n ")
   local wallsE = selection.match(".w ")
   local wallsW = selection.match(" w.")
   local allwalls = wallsN | wallsS | wallsE | wallsW
   local wallpt = allwalls:rndcoord(1) -- remove this spot from allwalls
   local wx, wy = wallpt.x, wallpt.y

   -- des.terrain({ selection = wallsN, typ="}" })
   local north, south, east, west = false, false, false, false
   local dx, dy = 0, 0
   if wallsN:get(wx, wy) > 0 then
      dir, dx, dy = "north", 0, -1
   elseif wallsS:get(wx, wy) > 0 then
      dir, dx, dy = "south", 0, 1
   elseif wallsE:get(wx, wy) > 0 then
      dir, dx, dy = "east", 1, 0
   elseif wallsW:get(wx, wy) > 0 then
      dir, dx, dy = "west", -1, 0
   end

   local tryroom = nil
   if percent(25) then
      tryroom = findroom(wx+dx, wy+dy, dir, false)
   end
   if tryroom == nil then
      -- room failed. try a line?
      tryroom = findroom(wx+dx, wy+dy, dir, true)
   else
      -- room succeeded; mark it as a candidate for being filled with snakes
      snakerooms[#snakerooms+1] = tryroom:clone()
   end
   if tryroom ~= nil then
      -- to make walls, grow in all directions
      local expanded = tryroom:clone():grow("all")
      des.terrain({ selection=expanded, typ='-' }) -- this will overwrite wx,wy but that's fine
      des.terrain({ selection=tryroom, typ='.' })

      local doorstate = percent(50) and "secret" or "random"
      des.door(doorstate, wx, wy)
   end
end

-- possible extension: match things like
-- |.|
-- ---
-- x.x
-- to create a door on them in post-proc

local deadends = get_deadends()

-- guard against very rare case of no dead ends
local n_ends = 0
deadends:iterate(function(x, y)
   n_ends = n_ends + 1
end)

if n_ends > 0 then
   -- Attempt to place the downstair sufficiently far away from the upstair.
   local stairell = selection.ellipse(ustairx, ustairy, 20, 10, 1):negate()
   local far_ends = stairell & deadends
   local dstair = far_ends:rndcoord()
   if dstair.x < 0 or dstair.y < 0 then
      -- pick any dead end; potential future extension is to find the furthest
      -- one away
      dstair = deadends:rndcoord()
   end
   des.stair({ dir = "down", coord = dstair })
else
   des.stair("down")
end

-- traps
for i=1,30 do
   des.trap({ spider_on_web = false })
end

-- mummies and other enemies
local n_mummies = 10 + d(4,4)
local mummies_in_ends = math.min(math.floor(n_ends * 3 / 4), n_mummies)
function mkmummy(deadend)
   local template = { class='M', id='human mummy', waiting=1, coord=deadends:rndcoord() }
   if not deadend then
      template['coord'] = nil
   end
   if percent(10) then
      template['id'] = nil -- generate any M randomly
   end
   des.monster(template)
end

for i=1, mummies_in_ends do
   mkmummy(true)
   mummies_in_ends = mummies_in_ends - 1
   n_mummies = n_mummies - 1
end
-- then any remaining mummies
for i=1,n_mummies do
   mkmummy(false)
end

-- "Indy, why does the floor move?"
local snaketypes = { 'pit viper', 'cobra' }
local snk_room = #snakerooms > 0 and d(#snakerooms) or nil
-- des.terrain({ selection = snakerooms[snk_room], typ='L' })
for i=1,4+d(6) do
   local snkid = snaketypes[d(#snaketypes)]
   if snk_room == nil then
      des.monster(snkid)
   else
      des.monster({ id=snkid, coord=snakerooms[snk_room]:rndcoord() })
   end
end

bats = { "bat", "giant bat", "vampire bat" }
for i=1,3 do
   des.monster(bats[d(#bats)])
end

-- treasure!
for i = 1,10+d(10) do
   des.object({ id="gold piece", quantity = d(250) })
end
for i=1,2+d(3) do
   local template = { id="chest", coord=deadends:rndcoord() }
   if percent(50) then
      -- make it a mummy trap
      template['trapped'] = 1
      template['spe'] = 3
      template['material'] = 'gold'
   elseif percent(30) then
      template['material'] = 'gold'
   end
   des.object(template)
end
local wpn_mats = { "copper", "silver", "gold", "bone" }
local wpns = { "short sword", "dagger", "knife", "spear", "javelin", "quarterstaff", "axe", "flail" }
for i=1,d(2,4) do
   des.object({ id=wpns[d(#wpns)], material=wpn_mats[d(#wpn_mats)], coord=deadends:rndcoord() })
end

