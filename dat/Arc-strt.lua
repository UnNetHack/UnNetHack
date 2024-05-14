-- NetHack Archeologist Arc-strt.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your class leader, Lord Carnarvon
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "outdoors", "noflipx")

des.map([[
,,,,,--------,,,,,,,-----....|..|,,,,,,,,..,,,T,,,,}|,,,,,,,,,,--------------
,T,,,|..|...|,,,,,,,|........+..S,,,T,,,,,.,,,,,,,}--,T,,,,,,,,|.....+......|
,,,---S-|...|,,,,,,,--+--------------,,,......,,,,}|,,,,,,,T,,,|.....|......|
,,,|....|...|,,,,,,,.....................,---..,,,}|,,,,,,,,|,,|...-+----+---
.,,|....|...+.,,,....,,,,,,,--------,,.,,,,,,,.,,}},,,,,,}..|,,|...|...|,....
..,----+-+---.....,,,,,,,,,,|......|,..,,,,,,,..,},}},,}}}}.,,,-----...|,,,..
,.............,,,..,,,,T,,,,|......|,.,----,,,,.....,}}}}}..,,,,,,|....|,T,..
,,,,..,-------,,,,.,,,,,,,,,|......|,T,|..|,,,,,,},..,,,}..,,,,,,,---+--,,,.,
,,,..,,,,,T,,,,,,,..,,,--------++--|,,,|..|,,,,,,},,.,,...,,,,,T,,,,..,,,,..,
,,T.,,,,,,,,,,,,,,,.,,,|....+......-----..|,T,,,,},,..,.,,,,,------,.,,,,..,,
,,,.,,T,,,,,,,,,,,,....|....|......S...+..|,,,,,,},,,....,----....|........,,
,,,.,,,,,,,,,T,,,,,....|....------+--------,,,,,},,,,,,,..|..|...K|.,,,,,,.,,
,,...,,,,,,,,,,,,,,....+......|,,,.,,,,,,,,,,,,,},,,,,....+..|...--.--,,,,...
,......................+......|,,,...,,,,,|,,,,,,},,...,..|..S..--...--T,,,,.
...{....,,,,,,,..,,,,,.--------,,,,,......|,,,T,.....,,,,,-------.....--,,,,,
-.........,,----+--,T...,,,,,,,,----+-----|,,,,.....,,,,,,,,,,,|.......|-----
--...--,,...|.|...|,..,.,,,,,T,,|....+....|,,,..,},..,T,,,,........{...+.....
,-----,,---+-.|...|,.,,..,,------S---|....|,,..,}},,........,,,|.......|FFFF,
,,,,,,,,|.....|...--+--,...+.........------,T.,}}}},,,.,,,,,,,,--.....--,.,,,
,,T,,,,,|.....|.......|,.,,|...........|,,,,..}}}}}}}}..,,,,,,,T--...--T,,,.,
]]);
-- Dungeon Description
des.region(selection.area(00,00,76,19), "lit")
-- Stairs
des.stair("down",76,05)
-- Portal arrival point
des.levregion({ region = {06,14,06,14}, type="branch" })
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Doors
des.door("locked",06,02)
des.door("closed",07,05)
des.door("closed",09,05)
des.door("closed",12,04)
des.door("closed",11,17)
des.door("closed",16,15)
des.door("closed",20,18)
des.door("closed",27,18)
des.door("closed",33,17)
des.door("closed",36,15)
des.door("closed",37,16)
des.door("closed",23,12)
des.door("closed",23,13)
des.door("closed",28,09)
des.door("closed",31,08)
des.door("closed",32,08)
des.door("closed",34,11)
des.door("closed",35,10)
des.door("closed",39,10)
des.door("closed",22,02)
des.door("closed",29,01)
des.door("closed",32,01)
des.door("closed",58,12)
des.door("closed",61,13)
des.door("closed",69,01)
des.door("closed",73,03)
des.door("closed",68,03)
des.door("closed",69,07)
des.door("closed",71,16)
-- Lord Carnarvon
des.monster({ id = "Lord Carnarvon", coord = {31, 06}, inventory = function()
   des.object({ id = "fedora", spe = 5 });
   des.object({ id = "bullwhip", spe = 4 });
end })
-- The treasure of the Archeology Department
storeroom = selection.fillrect(40,07,41,10)
des.terrain(storeroom:rndcoord(1), "\\") -- a throne
des.object("?", storeroom:rndcoord(1))
des.object(")", storeroom:rndcoord(1))
des.object("chest", storeroom:rndcoord(1))
for i = 1, 3 + d(4) do
   des.object("*", storeroom:rndcoord(1))
end
-- students milling around
des.monster("student", 06, 04)
des.monster("student", 24, 03)
des.monster("student", 33, 06)
des.monster("student", 25, 10)
des.monster("student", 32, 10)
des.monster("student", 16, 08)
des.monster("student", 59, 11)
des.monster("student", 66, 15)
des.monster("student", 69, 05)
-- objects
des.object({ class="[", x=06, y=01 })
des.object({ class="[", x=07, y=01 })
des.object({ class="!", id="acid", x=64, y=10 })
bookrooms = selection.floodfill(26,00) | selection.floodfill(30,00) |
            selection.floodfill(70,02) | selection.floodfill(64,10) |
            selection.floodfill(40,07) | selection.floodfill(40,16) |
            selection.floodfill(33,18) | selection.floodfill(16,16)
for i = 1,3 do
   des.object("+", bookrooms:rndcoord(1))
end
des.object("wand of detection", bookrooms:rndcoord(1))
for i = 1,2 do
   des.object({ id="boomerang", spe=d(2), coord=bookrooms:rndcoord(1) })
end
des.object("tinning kit", bookrooms:rndcoord(1))
des.object("grappling hook", bookrooms:rndcoord(1))
-- Small cemetery
des.grave(73,18, "Dr. Carter -- Died to a mummy's curse")
des.grave(75,19, "Under NO circumstances are students to dig up this grave!")
-- Mimics in one room
mimicroom = selection.floodfill(66,02)
for i = 1,d(3)+2 do
   des.monster("m", mimicroom:rndcoord(1))
end
-- There are no roaming hostiles on this level at all when it's created (though
-- more may spawn later) or traps. The College is not under attack.
