-- NetHack Archeologist Arc-goal.lua
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "noflip", "nommap", "noflipx");

des.map([[
                  ---                  -----              -----             
               ---|.|                  |...|         --.  |...|    ..    ...
               |.||.|------        -----...|      .  ...  |.---     ... ....
             ---S--S-|....|---------...S...|      ..-.......| .. .  ........
       ------|.......|....S........S...-----   ---..........   ........... .
       |.S...S.......S....|---S-----...S.| -----.........................   
  -----|.|---|.......|....|  |.|   --.---- |....................... ....    
  |....S.|  ---S--S----S-S- --S--   -S------.................- .. . -.-.--- 
  -----|.|  |.S..|.----.|.|-|...|---|.................... ..-- .    |.....| 
       |.|  ----.|..|.S.|.S.S...+...+...................   |.|      |.....| 
       |.|     |.--.|.-S|.|-|...+...+..................|   |.|      |.....| 
    ----S-------S--S-S-.|.| |...|---|..................|   |.|      --S---- 
    |.......S.S.......|--S---S---   --S-----...........|----S---     |.|    
    |.......--|.......|...S....|     |.|   |...........S..S....|------.|    
    |.......| |.......|---|....|------S---------.......|--|....S.....S.|    
    -----S----|.......|---|....S.......S..|    ------..|  |....|---S----    
        |.....S.......S...S....|-------|..|         ----  |....|  |.|       
        ---S----S---------------       |..|               ---S--  ---       
          |.|  |.|                     ----                 |.|             
          ---  ---                                          ---             
]]);

-- Some map regions
local bigroom_and_tunnel = selection.floodfill(50,09)
local tunnel_only = selection.area(56,00,76,08) & bigroom_and_tunnel
local stairroom = selection.area(05,12,11,14)
local non_hall_areas = selection.area(00,00,42,19) | selection.area(58,00,62,02)
                       | selection.area(56,08,75,19)
non_hall_areas = non_hall_areas ~ (selection.area(56,08,67,08) | selection.area(37,08,42,11))
non_hall_areas = non_hall_areas:filter_mapchar('.')

-- Dungeon Description
des.region(stairroom, "lit")
-- des.region(bigroom_and_tunnel, "lit")
des.region({ region={44,04,54,15}, lit=0, type="temple", filled=2 }) -- abandoned
-- Stairs
des.stair("up", 06, 12)
-- Non diggable walls -- the right is diggable (has already been dug by enemies)
des.non_diggable(selection.area(00,00,43,19))
-- Constrain arrival location by levelport
des.teleport_region({ region={05,12,11,14} })

-- Non-secret doors
des.door({ state="closed", x=32, y=09, iron=0 })
des.door({ state="closed", x=32, y=10, iron=0 })
des.door({ state="locked", x=36, y=09, iron=0 })
des.door({ state="locked", x=36, y=10, iron=0 })

-- Vary the topology a bit by closing off some of the secret doors
fillA = { {09,05},{12,12},{14,12},{09,15},{14,16} }
fillB = { {15,07},{18,07},{16,11},{19,11} }
fillC = { {21,05},{21,11},{22,09},{23,07},{26,16} }
fillD = { {25,07},{25,12} }
fillE = { {35,04},{30,07},{28,09},{29,12},{31,15} }
shuffle(fillA); shuffle(fillB); shuffle(fillC); shuffle(fillD); shuffle(fillE);
des.terrain({ coord=fillA[1], typ='-' })
des.terrain({ coord=fillB[1], typ='-' })
des.terrain({ coord=fillC[1], typ='-' })
des.terrain({ coord=fillD[1], typ='-' })
des.terrain({ coord=fillE[1], typ='-' })

-- Statues, some lining the main hall
local statue_coords = { {39,08},{41,08},{43,08},{45,06},{49,05},{39,11},{41,11},{43,11},{45,13},{49,14},{54,15},{54,12} }
for i=1,#statue_coords do
   -- back statues are gold
   local mat = statue_coords[i][1] > 50 and 'gold' or 'copper'
   des.object({ id='statue', montype='couatl', material=mat, historic=true, coord=statue_coords[i] })
end
local rndstatue_mons = { 'couatl', 'human mummy', 'stone golem' }
for i=1,d(2,3) do
   des.object({ id='statue', montype=rndstatue_mons[d(#rndstatue_mons)], historic=true, coord=non_hall_areas:rndcoord() })
end

-- Nemesis
des.monster({ id="Schliemann", x=53, y=09, inventory = function()
   des.object({ id="fedora", spe=1 })
   des.object({ id="leather jacket", spe=2 })
   des.object({ id="pick-axe", spe=4 })
   des.object({ id="oil lamp", lit=1 })
   -- nethack doesn't have explosives; this is the closest substitute...
   des.object({ id="oil", cursed=0, quantity=7+d(5) })
   des.object({ id="shield of reflection",buc="blessed",material="gold",spe=1,name="Itlachiayaque" })
   des.object({ id="Bell of Opening", buc="uncursed" })
end })

-- His crew of archeologists
function henchman_inventory()
   if percent(80) then des.object("fedora") end
   if percent(50) then des.object({ id="leather jacket" }) end
   if percent(50) then
      if percent(50) then des.object("high boots") else des.object("low boots") end
   end
   if percent(30) then des.object("leather gloves") end
   if percent(40) then des.object("bullwhip") end
   if percent(70) then
      des.object("pick-axe")
   else
      des.object("dagger")
   end
   if percent(20) then
      des.object({ id='brass lantern', lit=1 })
   else
      des.object({ id='wax candle', lit=1 })
   end
   if percent(40) then des.object({ id="gold piece", quantity=100+d(200) }) end
   if percent(15) then des.object({ class="!", id="oil" }) end
   if percent(10) then
      des.object("/")
   elseif percent(10) then
      des.object("!")
   end
   if percent(5) then des.object({ class="?", id="earth" }) end
end
des.monster({ id="archeologist", x=30, y=10, peaceful=0, waiting=1, inventory=henchman_inventory })
des.monster({ id="archeologist", x=45, y=07, peaceful=0, waiting=1, inventory=henchman_inventory })
des.monster({ id="archeologist", x=50, y=14, peaceful=0, waiting=1, inventory=henchman_inventory })
des.monster({ id="archeologist", x=58, y=08, peaceful=0, waiting=1, inventory=henchman_inventory })

-- Rubble from the dig operation
for i=1,20 do
   des.object({ id="rock", coord=bigroom_and_tunnel:rndcoord(), quantity=d(10) })
end
for i=1,d(2,3) do
   des.object("boulder", tunnel_only:rndcoord())
end

-- Pits from the dig operation
for i=1,d(3,2) do
   des.trap('pit', bigroom_and_tunnel:rndcoord())
   des.trap('pit', tunnel_only:rndcoord())
end

-- Identify dead ends for putting stuff in
local deadends = get_deadends()
-- add in some points that are in dead-end rooms but aren't one-space dead ends
deadends:set(42,02, 1)
deadends:set(60,09, 1)
deadends:set(41,17, 1)

-- Mummies and other monsters
for i=1,d(6,2) do
   -- These ones aren't waiting for approach. They've already been disturbed by
   -- the dig option.
   des.monster('human mummy', non_hall_areas:rndcoord())
end
for i=1,d(3) do
   des.monster('M', deadends:rndcoord())
end
for i=1,d(2,3) do
   des.monster('S', non_hall_areas:rndcoord())
end

-- Traps
for i=1,10+d(5) do
   des.trap({ coord=non_hall_areas:rndcoord(), spider_on_web=false })
end
des.trap("rolling boulder")

-- Objects
for i=1,10 do
   des.object({ id='gold piece', quantity = d(250) })
end
for i=1,d(2,2) do
   local box = { id='chest', material='gold', coord=deadends:rndcoord(), trapped=1, spe=3 }
   if percent(50) then
      box['trapped'] = 1
      box['spe'] = 3
   end
   des.object(box)
end
-- duplicate of Arc-filb code but not important enough to put in nhlib.lua
local wpn_mats = { "copper", "silver", "gold", "bone" }
local wpns = { "short sword", "dagger", "knife", "spear", "javelin", "quarterstaff", "axe", "flail" }
for i=1,d(2,3) do
   des.object({ id=wpns[d(#wpns)], material=wpn_mats[d(#wpn_mats)], coord=non_hall_areas:rndcoord() })
end
