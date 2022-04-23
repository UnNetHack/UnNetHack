-- NetHack Valkyrie Val-goal.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_flags("mazelevel", "noteleport", "icedpools", "noflipx", "nommap-boss")

--2345678901234567890123456789012345678901234567890123456789012345678901234567890
des.map([[
LLL   LLLLLL.LL....L       LLL.L..LLL.....LLL     LLL  LLLLL LLL.LLLLLLL  LL
LL    LLL.L....LLLL.    LLLLL...L............L L   LLL L..L  L..L.LLLLLL  LL
LLLL LLLL..LL.LLLLLL.L  LLLL.L.LLLLLL.LLL.L.L.LL     LLLLL..L...L ..LL    LL
LLLLLLL L.LLLLLLLLLLL........LLL-L-L-L-L-L-L...L    LLLL..LL.LLL   L.LLL LLL
L.LL    .LLLLLLLLL.....L..L.LLLL-----------LLLL.  LLLLLL.L..LLLL    L.LLLLLL
LL.LLL  L.LLL...L.....L..LLLLL---.........---L.L........LLLLLLLL   LLL.LLLLL
L...LL.L..LLLL...LL......LLLLLL|...........|LLLLLLL..LLL L LLLLLL LLLL.LLLLL
....L.....LLL........LLLL-L-L-L|...........|L-L-L-L..LL   LLLLLLL LL....L  L
L....L.....L....L  LLLLL--------...........--------L.LL   LLLLLLLLLLL.L     
LLLL....L....L.L   LLLLLL|.....--.........--.....|LLL.   LLLLLLLLLLLL.LLLL  
LLLL.L...L...LL.LLLLLLLL--..L...---+---+---...L..--L.L LLLLLLLL.L..L.LLLLLLL
LL........LLLLLL...LL.LLL|.LLL.......|.......LLL.|LLL..LLLLLLL...L.L.L.LLLLL
L....LLL   LLLLL..LL..LL--..L........|........L..--LLLLLLLL LLL.....L..LLLLL
..L.....   LLLL.L.LL..LLL|......-----------......|LLLLLLLL LLL.L......L.LLLL
.......LL LLLLLLLL.LLLLL--....---.........---....--LLLLL.    L..........LL  
....L..LLLLLLL  L.LL|L|LL|...--...LLLLLLL...--...|LL|L|L.    LLL....L..LLLLL
....LLL..LLLLLL  LL-------...|...LL.....LL...|...-------L.LLLLLL.L..L.....LL
...LLLLLLLLLLLL LLLL|.......--..LL.......LL..--.......|LLLLLLL LL....LL.....
..LLLL   LLLLLLLLLL--.......+........\........+.......--LLLLLL    LLLLLLL.L.
LLLLLLL    LL LLLLLL|.......|-..LL.......LL..-|.......|LLLLL     LLLLLL    L
]])
-- Dungeon Description
local everything = selection.area(00,00,75,19)
local inside = selection.area(32,05,42,08) + selection.area(26,09,48,16) + selection.area(21,17,53,19)
local outside = everything - inside 
des.region(everything, "lit")
-- Stairs
des.stair("up", 00,16)
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))

-- Lava is not solid seas, same as in the lower filler levels.
des.replace_terrain({ selection=outside, fromterrain='L', toterrain='.', chance=15 })
des.replace_terrain({ selection=outside, fromterrain='L', toterrain='.', chance=15 })

-- Interior doors
des.door({ iron=true, x=35, y=10 })
des.door({ iron=true, x=39, y=10 })
des.door({ iron=true, x=28, y=18 })
des.door({ iron=true, x=46, y=18 })

-- Exterior drawbridge
des.drawbridge({ x=37, y=03, dir="south", state="random" })

-- Giants don't need armories when they have boulders 
des.object('boulder', 33,05)
des.object('boulder', 41,05)
des.object('boulder', 32,06)
des.object('boulder', 42,06)
des.object('boulder', 32,07)
des.object('boulder', 42,07)
des.object('boulder', 32,08)
des.object('boulder', 42,08)
des.object('boulder', 33,09)
des.object('boulder', 41,09)
for x = 21,22 do
   for y = 17,19 do
      des.object('boulder', x, y)
   end
end
for x = 52,53 do
   for y = 17,19 do
      des.object('boulder', x, y)
   end
end

-- a little nod to the old Valkyrie quest
des.object('crystal ball', 37,17) 

---- Level-wide traps
for i=1,10+d(4) do
   des.trap("fire")
   if i < 3 then
      des.trap()
   end
end
des.trap("board")

local outsidefloor = outside:filter_mapchar('.')

-- Obligatory random boulders
for i=1,10+d(3) do
   des.object('boulder', outsidefloor:rndcoord())
end

---- Nemesis
des.monster({ id='Lord Surtur', x=66, y=13, waiting=true, inventory=function()
   des.object({ id='two-handed sword', buc='blessed' })
   des.object({ id = "crystal ball", buc="blessed", spe=5, name="The Orb of Fate" })
   des.object({ class='[', buc='not-cursed' })
   des.object({ class='/', id='fire', buc='not-cursed' })
   des.object({ id='Bell of Opening', buc='uncursed' })
end })

-- Other monsters
for i = 1, 6 + d(4) do
   if percent(5) then
      des.monster('fire elemental')
   elseif percent(30) then
      des.monster('fire vortex')
   else
      des.monster('fire ant')
   end
end
function giant_generator(species, sel) 
   -- spawn a fire giant within a certain area of the map, contains safeguards
   -- against placing it in lava or walls, makes sure it isn't peaceful and
   -- gives it some item.
   -- If species is 'H', makes random H, if set, makes an H of that species.
   template = { class = 'H',
                coord = sel:filter_mapchar('.'):rndcoord(),
                peaceful = 0,
                waiting = 1,
                inventory = function()
                   if percent(70) then des.object() end
                   if percent(5) then des.object() end
                end
              }
   if species ~= 'H' then
      template['id'] = species
   end
   des.monster(template)
end

giant_generator('fire giant', selection.area(04,07,09,10))
giant_generator('fire giant', selection.area(18,03,25,06))
giant_generator('fire giant', selection.area(34,00,42,01))
giant_generator('H', selection.area(61,00,67,02))
giant_generator('fire giant', selection.area(68,05,71,09))
for i = 1,5 do
   giant_generator('fire giant', selection.circle(66,13,5,1))
end
giant_generator('H', selection.circle(66,13,5,1))
