-- NetHack Monk Mon-loca.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "hardfloor", "outdoors", "noteleport")
--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
....TTT    .}}}...T...     T...      ..  T....      T..T      ...           
......T  ...}}.......    .......T ...........     ......    .......  .....  
T.....   ..}....T..........................      ... T...   .----.....----. 
TT....    } ...   .........  ....... ....     ...........  ..|..-------..|. 
TT...T   }            ...   .......  ...      ....  ....T  ..|...........|..
TT...  {}    ....T.        .....T   ...     T....    ...  ...--..-----..--..
T....       .......T      .....     ...   .......     ..  ....|..|...|..|...
T...T     .... ....T     ....      ....   ......      ..   ...|..|...|..|...
T...T    T...  .....     T..     ....T.   ......      ..  ....|..|...|..|...
....     ...  T....     ...     .......    ....       ...  ..--..-----..--..
...     ...   ....    T....   ..T.....     ....      T...  ..|...........|. 
...T   ....  .....   ......   .......      T....     ...   ..|..-------..|. 
...T   ...   T...    .....     .....      .......    ....   .----.....----. 
....  ....   ....     .....    .....      .......   .....   ..............  
.... T....   ....     ....    .......{   ......T    .......     ... ......  
T... .....   ....T   .....   .......}.T  .......    T......           ....  
T...B......   .....  ....T   ......}.............     .........   ......... 
T...B....T.   ..... .....     ..T..}..........          T.................. 
TTT.B.}.       }}......}}       ..}......T.             ................    
]]);

-- Dungeon Description
des.region(selection.area(00,00,75,18), "lit")
-- darken the fortress
des.region(selection.area(62,03,72,11), "unlit")

-- Constrain arrival location by levelport
des.teleport_region({ region={00,00,06,04}, dir="down" })
des.teleport_region({ region={62,03,72,11}, dir="up" })

-- Stairs
des.stair("up", 00,00)
des.stair("down", 67,07)

-- Non diggable walls
des.non_diggable(selection.area(00,00,75,20))

-- Make one door on a random side of the inner and outer walls of the fort
innerwalls = { {67,05},{65,07},{69,07},{67,09} }
outerwalls = { {67,03},{62,07},{72,07},{67,11} }
local innerwallsidx = math.random(1, #innerwalls);
local outerwallsidx = math.random(1, #outerwalls);
des.door({ state = "locked", coord = innerwalls[innerwallsidx], iron = 1 })
des.door({ state = "secret", coord = outerwalls[outerwallsidx] })

-- Objects
des.object("chest", 62, 03)
des.object("chest", 62, 11)
des.object("chest", 72, 03)
des.object("chest", 72, 11)
des.object()
des.object()
des.object()
des.object()
for i=1,5 do
  des.object("boulder")
end
-- since vegetarian monks shouldn't eat giant corpses, give a chance for
-- Str boost that isn't throttled by exercise restrictions;
-- in vanilla there is an Elbereth that attempts to partially prevent xorns from
-- eating the tins, but this isn't actually functional, so ignore it
local tinplace = selection.negate():filter_mapchar('.')
local tinloc = tinplace:rndcoord(0)
des.object({ id="tin", coord=tinloc, quantity=2, buc="blessed",
             montype="spinach" })

-- Traps
forest = selection.floodfill(01, 01)
for i=1, 2 + math.random(1, 2) do
  des.trap("bear", forest:rndcoord())
end
des.trap("pit")
des.trap("spiked pit")
des.trap("spiked pit")
des.trap("land mine")
des.trap("rolling boulder")
des.trap("bear")
fortress = selection.floodfill(62,03)
for i=1,3 + math.random(1 - 1,1*3) do
  des.trap(fortress:rndcoord())
end

-- Random monsters.
des.monster({ id="earth elemental", coord={05,17}, peaceful=0 })
des.monster({ id="earth elemental", coord={13,02}, peaceful=0 })
des.monster({ id="earth elemental", coord={28,01}, peaceful=0 })
des.monster({ id="earth elemental", coord={24,11}, peaceful=0 })
des.monster({ id="earth elemental", coord={70,17}, peaceful=0 })
des.monster({ id="earth elemental", coord={72,16}, peaceful=0 })
des.monster({ class="E", coord={15,01}, peaceful=0 })
des.monster({ class="E", coord={45,07}, peaceful=0 })
des.monster({ id="xorn", coord={07,16}, peaceful=0 })
des.monster({ id="xorn", coord={42,17}, peaceful=0 })
des.monster({ id="xorn", coord={44,16}, peaceful=0 })
des.monster({ id="xorn", coord={63,04}, peaceful=0 })
des.monster({ id="xorn", coord={71,04}, peaceful=0 })
des.monster({ id="xorn", coord={63,10}, peaceful=0 })
des.monster({ id="xorn", coord={71,10}, peaceful=0 })
local fortressarea = selection.area(58,00,75,13):filter_mapchar('.')
for i=1,4 do
  des.monster({ class = "E", coord = fortressarea:rndcoord(), peaceful=0 })
end

