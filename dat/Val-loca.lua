-- NetHack Valkyrie Val-loca.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_flags("mazelevel", "hardfloor", "icedpools", "outdoors", "noteleport", "noflip")

--        1         2         3         4         5         6         7         
--23456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
IIII|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCCAAAAAAAAAAAAAAAAAAAAAAAA...LL|....
IIII--AAAAAAAAAAAACCAAAAAAAAAAAAAAAACCCCCCCAAAAAAAAAAAAAAAAAAAA.....LL--....
IIIII--AAAAAAAAAACCCCAAAAAAAAAAAAAAAACCACCAAAAAAAAAAAAAAAAAAA.......LL|.....
IIIIII|AAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......LLL|.....
IIIIII----AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......LL---.....
IIIIIIIII-----AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......LL|.......
IIIIIIIIIII..---A--AA-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA........LL---.....
IIIIIIIIII...pppppppppppppAppAApppAAApAApAppppAppppApppppp........LLLL|.....
IIIIIIIII....pppppppApppppAppAAAAAAAAAApppppApAApppppAppp........LLLLL|.....
IIIIIIIII....pppppppppppAppApppAAApAAAAApAppApppApApppppp........LLLLL+.....
IIIIIIIII....pppppppppApppppAAApAAAAApApppAAAAApppppApppp........LLLLL+.....
IIIIIIIII....pppppppApppppAppAApAAAAAApApAppppAppAppppppp........LLLLL|.....
IIIIIIIIII...pppppppAppppppAAppAAppAApAppAAApppppppppppApp........LLLL|.....
IIIIIIIIIII..---AA-AAA-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA........LL---.....
IIIIIIIII-----AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......LL|.......
IIIIII----AAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAACCAAAAAAAAAA.......LL---.....
IIIIII|AAAAAAAAAAACCCCAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.......LLL|.....
IIIII--AAAAAAAAAACCCCCCCCAACCCAAAAAAAAAAAAAAAACAAAAAAAAAAAAAA.......LL|.....
IIII--AAAAAAAAAAAAACCCCCCCCCCAAAAAAAAAAAAAAACCCCAAAAAAAAAAAAAAA.....LL--....
IIII|AAAAAAAAAAAAAAAAAAACCCCCAAAAAAAAAAAAAACCCCCCCCAAAAAAAAAAAAAAA...LL|....
]]);
-- Define areas
local everything = selection.area(00,00,75,19)
local rightside = selection.area(58,00,75,19)
local muspell = selection.floodfill(75,00)
-- Dungeon Description
des.region(everything, "lit")
-- Stairs
des.stair("up", 00,00)
des.stair("down", 75,19)
-- Non diggable walls only on the right
des.non_diggable(rightside)
-- Force incoming levelporters to land by the stairs.
des.teleport_region({ region={00,00,03,02}, dir="down" })
-- Great gates (not closed, Surtur isn't playing defense)
des.door({ state="open", x=70, y=09, iron=1 })
des.door({ state="open", x=70, y=10, iron=1 })
-- Lava inside the wall.
des.replace_terrain({ selection=muspell:percentage(10), fromterrain='.', toterrain='L' })
muspell = muspell:filter_mapchar('.')
-- Random path of non-ice from stairs to bridge. The approach is basically since
-- the line won't reliably not stray off the ice, keep trying as long as there's
-- stuff in the line that shouldn't be in the final product.
local nopath = true
local line = selection.new()
while nopath do
  nopath = false
  line = selection.randline(00,00,09,09,30)
  line:iterate(function(x, y)
     local typ_name = nh.getmap(x, y)['typ_name']
     if typ_name ~= "ice" and typ_name ~= "room" and typ_name ~= "stairs" then
        nopath = true
     end
  end)
end
des.replace_terrain({ selection=line, fromterrain='I', toterrain='.' })
-- Objects
for i=1,5 do
   des.object(muspell:rndcoord())
end
for i=1,2 do
   des.object()
end
-- Traps
local rightland = rightside:filter_mapchar('.') -- after extra lava is added
for i=1,13 do
   des.trap("fire", rightland:rndcoord(1))
end
for i=1,4 do
   des.trap(rightland:rndcoord(1))
end
-- Monsters
local right_outside = rightland - muspell
for i=1,2 do
   des.monster({ id="fire giant", peaceful=0, coord=right_outside:rndcoord(1), asleep=1 })
end
for i=1,5 do
   des.monster({ id="fire giant", peaceful=0, coord=muspell:rndcoord(1) })
end
-- A bit of an experimental miniboss. Hrymr is the leader of the frost giants,
-- and most versions of Ragnarok have him captaining a ship to transport the
-- Aesir's enemies to battle; however, other versions have Loki doing it, so
-- assume that's why he can show up here.
des.monster({ coord={12,09}, id="frost giant", peaceful=0, name="Hrymr", female=false, inventory=function()
   -- give him gear that is very good in terms of attack/defense, but which the
   -- hero probably doesn't want to use themselves due to its sheer weight
   des.object({ id="two-handed sword", material="platinum", spe=4, buc="not-cursed" })
   des.object({ class="!", id="speed", buc="not-cursed" })
   des.object({ class="!", id="extra healing", buc="not-cursed" })
   des.object("boulder")
   des.object("boulder")
end })
des.monster({ id="frost giant", peaceful=0, coord={12,08} })
des.monster({ id="frost giant", peaceful=0, coord={12,10} })
