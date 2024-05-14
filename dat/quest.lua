-- NetHack may be freely redistributed.  See license for details.
-- TODO:
--  - output = "verbalize"
--  - export the quest string replacements to lua, instead of %H etc
--  - allow checking if hero is carrying item (see comments for %Cp Arc 00042)
--  - fold quest_portal, quest_portal_again, quest_portal_demand into one
--  - write tests to check questtext validity?
--  - qt_pager hack(?): if (qt_msg->delivery == 'p' && strcmp(windowprocs.name, "X11"))




-- text = "something"
-- Text is shown to the user.

-- synopsis = "something"
-- Synopsis is inserted into the message history.
--
-- output = "pline" | "menu" | "text"
-- The output can be manually set by using output = "menu"
-- Valid values for output are "pline", "text", and "menu, defaulting to
-- pline, unless the text contains newlines, or is too long to fit a message buffer,
-- then will be shown as a text window instead.



questtext = {
   -- If a role doesn't have a specific message, try a fallback
   msg_fallbacks = {
      goal_alt = "goal_next"
   },
   common = {
      TEST_PATTERN = {
         output = "text",
         text = [[%p:	return(plname);
 %c:	return(pl_character);
 %r:	return((char *)rank_of(u.ulevel));
 %R:	return((char *)rank_of(MIN_QUEST_LEVEL));
 %s:	return((flags.female) ? "sister" : "brother" );
 %S:	return((flags.female) ? "daughter" : "son" );
 %l:	return((char *)ldrname());
 %i:	return(intermed());
 %o:	return(artiname());
 %O:	return(shortened(artiname()));
 %m:	return((char *)creatorname());
 %n:	return((char *)neminame());
 %g:	return((char *)guardname());
 %G:	return((char *)align_gtitle(u.ualignbase[1]));
 %H:	return((char *)homebase());
 %a:	return(Alignnam(u.ualignbase[1]));
 %A:	return(Alignnam(u.ualign.type));
 %d:	return((char *)align_gname(u.ualignbase[1]));
 %D:	return((char *)align_gname(A_LAWFUL));
 %C:	return("chaotic");
 %N:	return("neutral");
 %L:	return("lawful");
 %x:	return((Blind) ? "sense" : "see");
 %Z:	return("The Dungeons of Doom");
 %%:	return(percent_sign);
 a suffix:	return an(root);
 A suffix:	return An(root);
 C suffix:	return capitalized(root);
 h suffix:	return pronoun(he_or_she, mon_of(root)); /* for %l,%n,%d,%o */
 H suffix:	return capitalized(pronoun(he_or_she, mon_of(root)));
 i suffix:	return pronoun(him_or_her, mon_of(root));
 I suffix:	return capitalized(pronoun(him_or_her, mon_of(root)));
 j suffix:	return pronoun(his_or_her, mon_of(root));
 J suffix:	return capitalized(pronoun(his_or_her, mon_of(root)));
 p suffix:	return makeplural(root);
 P suffix:	return makeplural(capitalized(root));
 s suffix:	return s_suffix(root);
 S suffix:	return s_suffix(capitalized(root));
 t suffix:	return strip_the_prefix(root);]],
      },
      angel_cuss = {
         "\"Repent, and thou shalt be saved!\"",
         "\"Thou shalt pay for thine insolence!\"",
         "\"Very soon, my child, thou shalt meet thy maker.\"",
         "\"The great %D has sent me to make you pay for your sins!\"",
         "\"The wrath of %D is now upon you!\"",
         "\"Thy life belongs to %D now!\"",
         "\"Dost thou wish to receive thy final blessing?\"",
         "\"Thou art but a godless void.\"",
         "\"Thou art not worthy to seek the Amulet.\"",
         "\"No one expects the Spanish Inquisition!\"",
         "\"Judgment hath been passed upon thee, %p.\"",
         "\"Thy reckoning is at hand, %p.\"",
         "\"Thou shalt be brought before %D for thy crimes!\"",
         "\"With %D as my witness, I shall strike thee down.\"",
      },
      banished = {
         synopsis = "[You are banished from %H for betraying your allegiance to %d.]",
         output = "text",
         text = [["You have betrayed all those who hold allegiance to %d, as you once did.
My allegiance to %d holds fast and I cannot condone or accept what you
have done.

Leave this place.  You shall never set foot beyond %H again as long as I
draw breath... and without the Bell of Opening, you will never be able to
enter the place where he who has the Amulet resides.

Go now!  You are banished from this place.]],
      },
      demon_cuss = {
         "\"I first mistook thee for a statue, when I regarded thy head of stone.\"",
         "\"Come here often?\"",
         "\"Doth pain excite thee?  Wouldst thou prefer the whip?\"",
         "\"Thinkest thou it shall tickle as I rip out thy lungs?\"",
         "\"Eat slime and die!\"",
         "\"Go ahead, fetch thy mama!  I shall wait.\"",
         "\"Go play leapfrog with a herd of unicorns!\"",
         "\"Hast thou been drinking, or art thou always so clumsy?\"",
         "\"This time I shall let thee off with a spanking, but let it not happen again.\"",
         "\"I've met smarter (and prettier) acid blobs.\"",
         "\"Look!  Thy bootlace is undone!\"",
         "\"Mercy!  Dost thou wish me to die of laughter?\"",
         "\"Run away!  Live to flee another day!\"",
         "\"Thou hadst best fight better than thou canst dress!\"",
         "\"Twixt thy cousin and thee, Medusa is the prettier.\"",
         "\"Methinks thou wert unnaturally stirred by yon corpse back there, eh, varlet?\"",
         "\"Up thy nose with a rubber hose!\"",
         "\"Verily, thy corpse could not smell worse!\"",
         "\"Wait!  I shall polymorph into a grid bug to give thee a fighting chance!\"",
         "\"Why search for the Amulet?  Thou wouldst but lose it, cretin.\"",
         "\"Thou ought to be a comedian, thy skills are so laughable!\"",
         "\"Thy gaze is so vacant, I thought thee a floating eye!\"",
         "\"Thy head is unfit for a mind flayer to munch upon!\"",
         "\"Only thy reflection could love thee!\"",
         "\"Hast thou considered masking thine odour?\"",
         "\"Hold! Thy face is a most exquisite torture!\"",
         "\"I wouldst fart in thy direction, but it might improve thy smell!\"",
      },
      legacy = {
         synopsis = "[%dC has chosen you to recover the Amulet of Yendor for %dI.]",
         output = "menu",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of %m the Creator.
    Moloch stole from %m the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, a newly trained %r, have been heralded
from birth as the instrument of %d.  You are destined
to recover the Amulet for your deity, or die in the
attempt.  Your hour of destiny has come.  For the sake
of us all:  Go bravely with %d!]],
      },
      legacy_convict = {
         output = "text",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of %m the Creator.
    Moloch stole from %m the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, a newly escaped %r, have chosen to redeem
yourself by recovering the Amulet for %d.  You are
determined to recover the Amulet for your deity, or die
in the attempt.  Your hour of destiny has come.  For
the sake of us all:  Go bravely with %d!]],
      },
      quest_complete_no_bell = {
         text = [["The silver bell which was hoarded by %n will be
essential in locating the Amulet of Yendor."]],
      },
      quest_portal = {
         output = "pline",
         text = [[You receive a faint telepathic message from %l:
Your help is urgently needed at %H!
Look for a ...ic transporter.
You couldn't quite make out that last message.]],
      },
      quest_portal_again = {
         text = "You again sense %l pleading for help.",
      },
      quest_portal_demand = {
         text = "You again sense %l demanding your attendance.",
      },
      qt_tutorial_welcome = {
         output = "text",
         text = [[
Tutorial: WELCOME

Welcome to UnNetHack! These tutorial messages will help to teach you
how to play the game. (Use ? for more in-depth help on the game.)

The screen generally contains (non-tutorial) messages at the top (if
any), information about your character at the bottom (this will be
explained later), and a map in between.  Each symbol on the screen
represents something; you are the @ sign (which might be hidden behind
this message at the moment, but you'll be able to see it most of the
time), and the screen will show what you can see and what you can
remember.  In general, letters (and a few symbols like ' : ; & and @)
represent creatures (which could be friendly or hostile); and most
punctuation marks represent either dungeon features (such as - and |
for walls and doors, # for corridors, and . for the floor), or items
(such as $ for gold).  Eventually you'll know the symbols off by heart;
while you're learning, you can press / for a reminder of what any
symbol means, or ; to find out what any object on the map is.

Let's start by looking around a bit; close this message with the
spacebar, then press ; so that you can look at nearby objects.]],
      },
      qt_tutorial_cursor_numpad = {
         output = "text",
         text = [[
Tutorial: CURSOR MOVEMENT (with the number pad)

In order to specify what you want to look at, you have to be able to
move the cursor.  You may be used to thinking of four directions in
which an object can move; but NetHack has eleven:

north-west north north-east          upwards
       7     8     9                    <

           here
  west 4     .     6 east
          at self

       1     2     3                    >
south-west south south-east         downwards

In the case of looking at an object on-screen, we can only look
sideways, not upwards or downwards, so only bother with the numbers
for the time being.  Use the numbers in order to move the cursor over
the location to look at; then press the . button to select it.  If you
aren't sure what to look at, look at a monster (a letter on the map).]],
      },
      qt_tutorial_cursor_vikeys = {
         output = "text",
         text = [[
Tutorial: CURSOR MOVEMENT (with vi-style controls)

In order to specify what you want to look at, you have to be able to
move the cursor.  You may be used to thinking of four directions in
which an object can move; but NetHack has eleven:

north-west north north-east      upwards     Note: for orthogonal
       y     k     u                <        movement, hjkl are in a
                                             row on a QWERTY keyboard;
           here                              j's descender goes down,
  west h     .     l east                    k's ascender goes up.  The
          at self                            diagonals are the letters
                                             diagonally adjacent to h,
       b     j     n                >        in the corresponding
south-west south south-east     downwards    directions.

In the case of looking at an object on-screen, you can only look
sideways, not upwards or downwards, so only bother with the letters
for the time being.  Use the letters in order to move the cursor over
the location to look at; then press the . button to select it.  If you
aren't sure what to look at, look at a monster (a letter on the map).]],
      },
      qt_tutorial_look_tame = {
         output = "text",
         text = [[
Tutorial: PETS

The Mazes of Menace may be a dangerous and hostile place to adventure
in, but luckily, you don't have to do it alone.  You start each game
with a tame kitten (f), little dog (d), or pony (u); they will follow
you around the dungeon, and fight monsters with you.  Your pet can
mostly take care of itself; for instance, it will try to feed itself
with the corpses of enemy monsters to prevent itself becoming hungry,
and back out of fights it thinks it will lose.  Over time, as the pet
gets more practice, it will get better at combat, and may grow into a
larger animal; and to start with, your pet may well be better at
combat than you are; a useful friend for an inexperienced character.

Note that your pet can't follow you everywhere; if you go up or down
stairs, your pet will only be able to follow if it's next to you at
the time, and will otherwise become stranded and may go wild.  In
general, if you want to make sure you keep track of your pet, keep it
close by whenever you can.

If you aren't sure what to do next, use the cursor movement keys to
walk around, or use ; to look at more things around you.]],
      },
      qt_tutorial_look_hostile = {
         output = "text",
         text = [[
Tutorial: MONSTERS

That's a hostile monster, one of the huge number of threats facing you
in the Mazes of Menace.  Note that monsters are only displayed onscreen
if your character can see them; therefore, it can be important to try
to get the best view of what's ahead that you can, and be especially
wary in dark corridors!

There are several ways to deal with enemy monsters.  Although often you
can simply run away, that can leave you backed into a corner, and some
monsters will be able to outrun you; even so, that option is often
worth considering.  More usually, you'll want to kill the monster;
common methods are fighting it hand-to-hand ('in melee'), using
missiles, and casting spells.

To attack a monster in melee, simply walk into it, and you'll hit the
monster with whatever you're holding in your hands.  Movement in the
dungeon is done the same way as moving the cursor; you can even move
diagonally like this. (Melee should be good enough to beat most early
monsters, although many characters will have a better method of
fighting.) Try fighting that monster now!]],
      },
      qt_tutorial_look_peaceful = {
         output = "text",
         text = [[
Tutorial: PEACEFUL MONSTERS AND ALIGNMENT

Not all monsters are out to kill everyone they meet; many monsters will
have more sense than that.  Many monsters can be very dangerous when
angered; so it's often a good idea to leave peaceful monsters be.  If a
peaceful monster is blocking your way, try waiting a few turns (press
the . key in order to wait a turn) to see if they move out of the way.

Amongst the factors that determine how a monster will react to you,
one of the most important is alignment (Lawful, Neutral, or Chaotic);
monsters which have a similar outlook on life to you are more likely
to be non-aggressive.  You can see your alignment in the status area
near the bottom of the screen, or by pressing control-X; this also
lets you know about the connected area of religion (sharing an
alignment implies sharing a god).  Note that although lawful behaviour
tends to be good, and evil behaviour tends to be chaotic, legal and
ethical do not always coincide.

One useful form of interaction with certain peaceful and tame monsters
is to chat to them using #chat; you'll see the outcome of the
conversation at the top of the screen.]],
      },
      qt_tutorial_look_invisible = {
         output = "text",
         text = [[
Tutorial: INVISIBLE MONSTERS

It looks like there's something around here you can't see! This might
be because the monster is invisible, or you might not be able to see
it for some other reason.  Either way, fighting it could be difficult.

Unlike all other letters, the letter I on the map is special; instead
of indicating a monster you can see right now, it indicates where an
invisible monster was the last time you determined its location.  You
can fight an I the same way you fight any other monster (e.g. you can
fight it in melee by walking into it), but the monster might have
moved since you discovered where it was, in which case your attack
will miss altogether.

If you think an invisible monster has moved, you can use the F and m
commands; F followed by a movement direction attacks an adjacent
square even if there's apparently nothing there, and m followed by a
movement direction walks onto a square even if you remember a monster
was there.  Searching around you by pressing s may also be helpful;
it's considerably easier to feel an invisible monster next to you than
it is to find a secret door or trap.]],
      },
      qt_tutorial_doors = {
         output = "text",
         text = [[
Tutorial: DOORS AND DOORWAYS

Closed doors are represented by the + character; open doors are by
default represented by a | or - character, perpendicular to the wall
that they're on (although your options may be set to show it in a more
stylish way, to avoid it getting muddled with a wall).  A closed door
is a rather obvious obstacle to any adventurer, but there are several
ways to deal with one.  Probably the most obvious method of getting
past a door is to try to open it; to do that, just try to walk into it
or stand adjacent to the door, press o, and specify the direction that
the door is in.  Some doors are locked; in this case, you'll have to
either pick the lock (by using the a command to use a lock-picking
tool, if you have one), or kick the door down using control-D.

Open doors are less obviously obstacles, but they still hamper
movement to some extent, as it's impossible to move diagonally into or
out of an open doorway; you have to approach it straight.  You can also
close an open door by pressing c when standing adjacent to it; because
most monsters don't know how to, or physically can't, open doors, this
can be useful for shutting out monsters.]],
      },
      qt_tutorial_corridor = {
         output = "text",
         text = [[
Tutorial: CORRIDORS AND LIGHTING

Up here near the top of the dungeon, the vast majority of rooms are
going to be well-lit; you can see anything that's on a lit square,
unless there's a wall or other equally solid object in the way,
blocking your view.  When a square does go out of sight, your character
won't be able to see it, but your memory of it will continue to be
displayed on the map; monsters shown on the square will be removed,
because monsters tend to walk around, but any items or dungeon
features on the square will continue to be displayed (and you can
use ; to find out what they were, if you've forgotten).

Corridors are generally represented by the # symbol (your options may
be set to use a slightly more artistic representation); the solid rock
that borders them is just displayed with blank space.  Corridors are
generally dark; unless you illuminate them yourself somehow, you'll
only be able to fully see the squares adjacent to you (as well as any
lit squares in the distance, if there's no solid object in the way).
Take extra care in corridors; you might suddenly end up next to a
monster you couldn't previously see, due to being too far away!]],
      },
      qt_tutorial_secretdoor = {
         output = "text",
         text = [[
Tutorial: SECRET DOORS AND CORRIDORS

Not everything in these dungeons is quite as it seems.  Amongst dungeon
designers, it's a common habit to hide some of the connections between
one room and the next, and this dungeon is no exception.

A secret door or corridor just looks like normal rock or a normal
wall, until it's discovered; the usual way to discover one is to
carefully search the area around it.  If you think you're standing
adjacent to such a secret passage, you can use the s command to start
looking for it; however, it can take a beginning character quite a lot
of searching to find anything even if there is something there, so
it's often necessary to press s several times. (And of course, all
that searching will be in vain if there is in fact nothing there!)
Good places to search are in apparent dead-ends, and similar
situations where it seems likely that there's a way forward, but you
can't see what it is.

This is a good place to try searching, because there is a door or
corridor hidden adjacent to your current location; try pressing s
until you find it, although note that searching takes time.]],
      },
      qt_tutorial_poolormoat = {
         output = "text",
         text = [[
Tutorial: WATER

Water (by default represented as a } character, but this is often
customized to show as something else) is yet another of the hazards
you may end up facing.  Generally speaking, a typical adventurer won't
be able to cross large bodies of water without magical aid; although
in some cases, it may be possible to carefully move through a swamp by
avoiding water squares.  Diagonal movement might be helpful here.

Be very careful when moving near water; falling in may cause your
character severe problems, although it isn't normally instantly fatal
unless you're so burdened you can't climb back out.  It's often a good
strategy to find an alternative route through this level, if there is
one.]],
      },
      qt_tutorial_lava = {
         output = "text",
         text = [[
Tutorial: LAVA

Lava, like water, is represented by the } character (or other custom
character representing liquid); you can use the ; command to tell
water and lava apart, or (if playing in colour) by looking at the
colour difference.  You need to be even more careful around lava than
you are around water; few adventurers have the levels of magical
protection needed to be able to swim through lava safely, so a typo
near lava could be instantly fatal.  Because NetHack is a turn-based
game, there's no need to react to anything quickly in real-time; so
take your time over your moves!]],
      },
      qt_tutorial_stairs = {
         output = "text",
         text = [[
Tutorial: STAIRS

The > and < characters represent flights of stairs (going down and up
respectively); you can climb down stairs in order to go deeper in the
dungeon.  The commands for traversing a flight of stairs are the same
as the characters representing the stairs themselves; and likewise,
the same as the keys representing the vertical directions.  Note that
climbing up and down stairs can be difficult if you're holding too
much equipment; for instance, excess weight may cause you to fall down
the stairs, taking damage in the process.

If you can't see the stairs that let you move down from any given
level, try looking underneath items; failing that, bear in mind the
possibility that there may be a secret door or corridor somewhere
leading to another part of the current level. (Of course, the dungeon
isn't infinitely deep; so it's possible you'll find the bottom level,
which has no downstairs.  But that isn't something to worry about just
now.)

Note that the stairs that go up from level 1 lead out of the dungeon;
the game will end if you use them.]],
      },
      qt_tutorial_fountain = {
         output = "text",
         text = [[
Tutorial: FOUNTAINS

The { character represents a fountain; it's possible to walk through a
square with a fountain on, but also to interact with the fountain.
Most commonly, an adventurer who chooses to use a fountain will either
want to drink from it, or dip something into it; in either case, this
is accomplished by standing on the square of the fountain, and using
one of the following commands:

    q   Drink (quaff) from the fountain
 #dip   Dip something into the fountain

Note that not all fountains are quite what they seem to be!]],
      },
      qt_tutorial_throne = {
         output = "text",
         text = [[
Tutorial: THRONES

A backslash character (\) represents an opulent throne.  If there's
nothing else on the same square as the throne, you can sit on it by
moving onto the same square and using the #sit command; if there are
items on top of the throne, you'll need to clear them off first, say
by picking them up and dropping them elsewhere.]],
      },
      qt_tutorial_sink = {
         output = "text",
         text = [[
Tutorial: KITCHEN SINKS

The # character normally represents a corridor; but inside a room, it
has a different use, representing a kitchen sink.  Sinks don't have
many obvious uses, although if you feel brave, you could try drinking
from one by standing on the same square as the sink, and pressing q in
order to 'quaff' from it.]],
      },
      qt_tutorial_grave = {
         output = "text",
         text = [[
Tutorial: GRAVES

The lure of a treasure-filled dungeon is great, and as a result there
tend to be many adventurers seeking their fortune in any given
dungeon.  Even though you're the only person here today, there will
have been many other adventurers who sought their fortune, and died
here, in the past; and you just found the grave of one.

Graves are signified by the | symbol inside a room; they don't block
your movement.  If the adventurer in question died very recently, then
their body and possessions will likely still exist in the same
location; and quite possibly, so will their ghost, and whatever killed
them! Otherwise, the corpse, along with some of the possessions, will
be buried underground, and if you feel like grave-robbing, you'll have
to dig them up somehow.

Note that it's very common for the possessions of dead adventurers to
be cursed, so be careful with things that you loot from the dead.]],
      },
      qt_tutorial_altar = {
         output = "text",
         text = [[
Tutorial: ALTARS

Religion is an important feature of NetHack; although the gods take
note of your actions all the time, an altar (_) is a good place to go
to contact them more directly.  It's important to know who an altar is
dedicated to; to do this, stand on the altar and press the : key.
(Using : will tell you more information about the square you're on.)
As a reminder: your %G is %d.

One common use of an altar is to sacrifice the corpse of a monster to
the appropriate deity; to do this, stand on the altar, with a corpse
in your inventory or on the altar, and use the #offer command.  As long
as your %G is not offended by the sacrifice, this will generally
have a pleasing effect; sometimes, your %G will give you gifts in
return for your sacrifices.

Prayer (using #pray) is also more effective when standing on your own
%G's altar; prayer is used to request help from the gods, usually
in emergencies.  Deities don't like being pestered too much, so try not
to request divine intervention too often; most of the time, you'll
want to save your prayers for when you're in mortal danger.]],
      },
      qt_tutorial_drawbridge = {
         output = "text",
         text = [[
Tutorial: DRAWBRIDGES

That's a drawbridge; while open, you can walk over it, but when shut,
it will block your path.  Drawbridges are basically just really large
doors; but they tend to have a more complicated mechanism for opening
and closing.  You almost certainly don't want to be anywhere near a
drawbridge if it gets destroyed!]],
      },
      qt_tutorial_trap = {
         output = "text",
         text = [[
Tutorial: TRAPS

The ^ symbol represents a trap.  Traps can have a variety of effects,
but generally speaking you want to avoid them; you can look at a trap
with ; to see what it does (or use ^ and a direction when next to it,
if it's buried under items and you can't see the ^ as a result).  Some
traps can be dismantled using the #untrap command when adjacent to the
trap; although scavenging the parts that make up a trap can be useful,
it's entirely possible that your character will set the trap off by
mistake when doing so, so be careful and make sure you aren't badly
injured before you try.  The #untrap command can also be used to check
chests and doors for traps, although not with a perfect success rate.

Walking round a trap is normally the best option; although if it's
blocking your way and you can't dismantle it, there may be some more
ingenious way to either avoid its effects, or to prevent it
functioning correctly when stepped on.]],
      },
      qt_tutorial_l1upstairs = {
         output = "text",
         text = [[
Tutorial: ESCAPING THE DUNGEON

That < is a flight of stairs going upwards (likewise, a > is a flight
of stairs going downwards); to be precise, those are the stairs you
used to enter the dungeon.  If you go up those stairs (by using the <
command), you'll walk straight out of the dungeon and end the game.

Leaving the dungeon via the stairs is known as 'escaping'; escaping
the dungeon is not nearly as bad as death (especially if you get out
with a lot of treasure!), but it isn't really an ideal end to the
game, as you haven't achieved the major aim of the game (offering the
Amulet of Yendor to your god).  Of course, if you somehow manage to get
the Amulet of Yendor, there's nothing wrong with taking it out of the
dungeon in an attempt to find an altar to your god somewhere on the
surface, so you can offer the Amulet; the game will continue if you do
this.

Note that once you've left the dungeon wih a character, either with or
without the Amulet, it's impossible to re-enter the dungeon; in order
to play there again, you'll have to start a new game, with a new
character, if you escape.]],
      },
      qt_tutorial_farmove_vikeys = {
         output = "text",
         text = [[
Tutorial: FASTER MOVEMENT (with vi-style controls)

Moving around one square at a time is useful for tactical reasons
during combat; but it can get boring quickly when trying to explore a
level.  Therefore, there are some commands which cause you to continue
moving in the same general direction until something happens:

    g <direction>
        Keep moving in the given direction until something interesting
        happens, or the corridor forks.
    G <direction> or control-<direction>
        Keep moving in the given direction until something interesting
        happens (not including a fork in the corridor).
    M <direction> or   shift-<direction>
        Keep moving in the given direction until no further progress
        is possible (such as bumping into a monster or dead end).

It's up to you to decide how aggressive you want to be in terms of
automatically repeating moves; note that none of these save you time
in-game, just your own time playing.  Of course, you need not use these
commands at all; you can just continue moving one square at a time.]],
      },
      qt_tutorial_farmove_numpad = {
         output = "text",
         text = [[
Tutorial: FASTER MOVEMENT (with the number pad)

Moving around one square at a time is useful for tactical reasons
during combat; but it can get boring quickly when trying to explore a
level.  Therefore, there are some commands which cause you to continue
moving in the same general direction until something happens:

    g <direction> or 5 <direction>
        Keep moving in the given direction until something interesting
        happens, or the corridor forks.
    G <direction>
        Keep moving in the given direction until something interesting
        happens (not including a fork in the corridor).
    M <direction>
        Keep moving in the given direction until no further progress
        is possible (such as bumping into a monster or dead end).

It's up to you to decide how aggressive you want to be in terms of
automatically repeating moves; note that none of these save you time
in-game, just your own time playing.  Of course, you need not use these
commands at all; you can just continue moving one square at a time.]],
      },
      qt_tutorial_travel = {
         output = "text",
         text = [[
Tutorial: AUTOMATIC TRAVEL

Moving back to places in the dungeon that you've already explored can
be a tedious task; luckily, it's possible to automate your movement
somewhat.

The _ command will prompt you for a location anywhere on the current
level; it'll then automatically move you along the shortest path you
know to that location.  This doesn't save you any time in-game, but it
can save a lot of real-life time pressing the keys to move back to the
known location one at a time.

In addition, you can aim for a particular location by map symbol; for
instance, you can press _>. to automatically travel to a visible
downstairs, or __. to automatically travel to an altar.  This is
particularly useful when going back through several already-explored
levels of the dungeon to reach something higher up.

Travelling will stop automatically if it comes across something
interesting, such as an engraving or a monster.]],
      },
      qt_tutorial_diagonals_vi = {
         output = "text",
         text = [[
Tutorial: DIAGONAL MOVEMENT (with vi-style controls)

It seems that you haven't been moving diagonally much recently; this
is a reminder about what diagonal movement is all about.  Here's the
cursor movement diagram again, as a reminder:

north-west north north-east      upwards     Note: for orthogonal
       y     k     u                <        movement, hjkl are in a
                                             row on a QWERTY keyboard;
           here                              j's descender goes down,
  west h     .     l east                    k's ascender goes up.  The
          at self                            diagonals are the letters
                                             diagonally adjacent to h,
       b     j     n                >        in the corresponding
south-west south south-east     downwards    directions.

Moving diagonally can be an important tactic, as it's generally faster
than moving orthogonally for moving quickly along diagonal corridors;
you can also use it to move round obstacles such as traps without
taking any extra time.  It's especially important to use diagonal moves
correctly when trying to outrun a monster.]],
      },
      qt_tutorial_diagonals_num = {
         output = "text",
         text = [[
Tutorial: DIAGONAL MOVEMENT (with the number pad)

It seems that you haven't been moving diagonally much recently; this
is a reminder about what diagonal movement is all about.  Here's the
cursor movement diagram again, as a reminder:

north-west north north-east      upwards
       7     8     9                <

           here
  west 4     .     6 east
          at self

       1     2     3                >
south-west south south-east     downwards

Moving diagonally can be an important tactic, as it's generally faster
than moving orthogonally for moving quickly along diagonal corridors;
you can also use it to move round obstacles such as traps without
taking any extra time.  It's especially important to use diagonal moves
correctly when trying to outrun a monster.]],
      },
      qt_tutorial_repeat_vikeys = {
         output = "text",
         text = [[
Tutorial: REPEATING COMMANDS (with vi-style controls)

Getting bored of pressing s repeatedly to search that square? Not only
can repeating a command be boring, it can also be dangerous; holding
down a key, or pressing a button repeatedly without thinking, can both
lead to you failing to react to a monster that turns up, meaning that
it can sometimes get several free hits on you before you stop
repeating and start fighting.

There's a way to automatically repeat commands, however.  You can type
a number before more or less any command to automatically repeat the
command that many times; for instance, 20s will search around you 20
times, or until something interesting happens (such as finding what
you were searching for, or until a monster turns up).  This sort of
mechanized repetition both saves you time, and reduces the chance of
making a mistake due to distraction.

It's also possible to repeat commands after the fact; pressing
control-A will repeat whatever you just did, which can be useful when
repeatedly trying to open a stuck door, for instance.  Generally
speaking, though, control-A is used less than repeat counts.]],
      },
      qt_tutorial_repeat_numpad = {
         output = "text",
         text = [[
Tutorial: REPEATING COMMANDS (with the numeric keypad)

Getting bored of pressing s repeatedly to search that square? Not only
can repeating a command be boring, it can also be dangerous; holding
down a key, or pressing a button repeatedly without thinking, can both
lead to you failing to react to a monster that turns up, meaning that
it can sometimes get several free hits on you before you stop
repeating and start fighting.

There's a way to automatically repeat commands, however.  You can type
n and a number before more or less any command to automatically repeat
the command that many times; for instance, n20s will search around you
20 times, or until something interesting happens (such as finding what
you were searching for, or until a monster turns up).  This sort of
mechanized repetition both saves you time, and reduces the chance of
making a mistake due to distraction.

It's also possible to repeat commands after the fact; pressing
control-A will repeat whatever you just did, which can be useful when
repeatedly trying to open a stuck door, for instance.  Generally
speaking, though, control-A is used less than repeat counts.]],
      },
      qt_tutorial_chooseitem = {
         output = "text",
         text = [[
Tutorial: ITEM PROMPTS

You're about to be asked to choose an item; this probably happened
because you wanted to do something like wear armor or read a scroll,
and the game needed to know which piece of armor to wear, or which
scroll to read.  Each item in your inventory is given a letter (case
matters), that you can use to represent the item; so for instance, if
you wanted to wield a weapon in 'slot' a, you could press a in order
to choose that weapon.  The prompt will let you know which slots
contain plausible items for the command you selected.

What if you can't remember what's in which slot? There are two more
options for the prompt: ? will give you a list of all the items that
the game thinks are likely choices (such as weapons for a wield
command), and * will give you a list of all the items you have, even
if it would apparently make no sense. (This lets you try out unusual
tactics, such as throwing items which aren't normally thrown, or
wielding things other than weapons.) You can choose the item you want
to use from the resulting menu.

Sometimes, - is also accepted, meaning no item, or bare hands.]],
      },
      qt_tutorial_massunequip = {
         output = "text",
         text = [[
Tutorial: UNEQUIPPING MULTIPLE ITEMS

Sometimes, you need to remove multiple pieces of armor in order to
get at the armor underneath; for instance, you might have to take off
your cloak and jacket in order to put on a T-shirt.  Likewise, you
might want to take all your armor off at once so you can drop it to
save weight, or remove both your rings so you can put two different
rings on.

The A command allows you to take off armor, remove rings and amulets,
unwield weapons, empty your quiver, and unready a secondary weapon;
therefore, if you need to do two or more of those actions at once for
any reason, using A will often be simpler than typing in each of the
commands manually.  It can also be useful if you forget what the
command is to perform a particular sort of unequip action.]],
      },
      qt_tutorial_callmonster = {
         output = "text",
         text = [[
Tutorial: NAMING MONSTERS

When there are a lot of similar-looking monsters around, it can be
hard to keep track of what's going on; a lot of messages saying just
"the jackal" or "the soldier ant" tend not to be particularly
informative.  In such cases, knowing which monster is which can help
you concentrate on attacking just one monster, rather than spreading
the damage amongst multiple monsters by mistake.  It's good practice to
give your pet a name, so you can recognise it if you somehow become
separated from it and it goes wild.

The C command lets you call a monster a particular name; the game's
messages will from then on use that name to refer to the monster,
rather than its species.  You can't rename monsters that have a name
already, though, unless it was you who gave them their name in the
first place.]],
      },
      qt_tutorial_multidrop = {
         output = "text",
         text = [[
Tutorial: DROPPING MULTIPLE ITEMS

Dropping commands one at a time with d is tedious, slow, and prone to
mistakes when you want to drop a lot of items all at once; it also
takes more game time than is actually needed.  The D command works like
d, except that you can specify more than one item to drop; but no
matter how many items you're dropping, it only takes one turn.

Note that in a multiple-select menu, you can use . to automatically
select all options of the menu; combined with the ability to show,
say, only items with unknown blessed/cursed state, or only items that
have been picked up in a shop but not paid for, you can quickly select
entire sets of items you want to get rid of, and drop them.  This trick
works in all multiple-select menus, not just d's. (The other menu
commands: - deselects all items, \ deselects items on this page, ^
goes to the first page, ~ inverts all items on this page, | goes to
the last page, > goes to the next page, < goes to the previous page,
. selects all items on the menu, , selects all items on the page, a
number before an item selects that many items from a stack. < > . and
numbers are probably the only commands that are worth memorizing,
though.)]],
      },
      qt_tutorial_massinventory = {
         output = "text",
         text = [[
Tutorial: QUICK INVENTORY

A quick interface tip: you can use i, rather than I, to show the whole
inventory at once rather than going via sections.  Note that the
inventory list can get so long that it needs scrolling, unless you
have an unusually large terminal; you can use < and > to scroll
through the list, the same way that any other menu works.]],
      },
      qt_tutorial_secondwield = {
         output = "text",
         text = [[
Tutorial: SECONDARY WEAPONS

For characters who repeatedly switch between their main weapon and a
secondary weapon (for instance, switching between a melee weapon and a
launcher is rather common), there's a shortcut which can save on
keystrokes and on typos.  There's a secondary weapon slot which you can
put an item in to be able to wield it rapidly; to swap your wielded
weapon and secondary weapon, press x. (This is how you fill your
secondary weapon slot in the first place; by wielding a weapon and
swapping it into the slot.)]],
      },
      qt_tutorial_look_reminder = {
         output = "text",
         text = [[
Tutorial: LOOKING AT THINGS

You haven't used the ; command to look at anything for ages; try using
it more often.  Knowing what the symbols on the screen mean is rather
important to be able to play effectively; and even experienced players
need to check now and again, because some symbols can mean more than
one thing. (If your terminal shows colours, that can make it easier to
tell things apart; but even so, some colour/symbol combinations are
ambiguous; is a brown + a closed door or a brown spellbook, for
instance?)

Here's a quick tip about using the ; command; once you've placed the
cursor on whatever you want to look at, you can press : rather than
pressing . in order to view flavour text about whatever you're looking
at; normally this will be a quote from literature, to give you a
better idea of whatever what you're looking at actually is, although
sometimes it's more of a factual description.  Have fun!]],
      },
      qt_tutorial_fire = {
         output = "text",
         text = [[
Tutorial: THE QUIVER

If you're repeatedly throwing the same thrown weapon, or repeatedly
firing the same ammunition from a launcher, you can put whatever
you're throwing in your quiver in order to be able to fire it with
fewer keystrokes.

The Q command allows you to put an item into your quiver; from then
on, you can fire it by pressing f, which works identically to throwing
using t, except that you don't have to specify which item to throw or
fire; instead, the item chosen is the one in your quiver. (Some
characters will start out with their quiver already filled.) The
quiver is simply a convenience option; it doesn't actually do anything
other than selecting an item to throw each time, as everything in your
main inventory is readily accessible (backpack manufacturers really
know how to cater to adventurers well).  Still, saving keystrokes
reduces the chance of accidentally throwing the wrong item to quite an
extent.]],
      },
      qt_tutorial_dgn_overview = {
         output = "text",
         text = [[
Tutorial: DUNGEON OVERVIEW

You've already ventured quite far into the dungeon.  You passed several
dungeon levels by now and it might be hard for you to remember each
and every level.

Use #overview or press control-O for an overview of the dungeon.  The
list shows specific dungeon features like fountains, sinks and altars
but also stairs to other branches.

You can also name the current level by using #annotate.  The name will
appear in the dungeon overview next to the dungeon level number.]],
      },
      qt_tutorial_viewtutorial = {
         output = "text",
         text = [[
Tutorial: GETTING HELP

It can be hard to remember everything that goes on in NetHack;
luckily, there are several commands that you can use in order to get
help on how to play.

Most important is probably the ? command, which shows a menu of the
various help possibilities; for instance, ?b shows a list of all the
commands in the game, ?d tells you what the various symbols mean (it
does the same thing as /), and ?e explains what a given key does.

You might also forget the contents of a tutorial message; if so, you
can use the key combination control-V to look again at any tutorial
message you've seen this game (although they may make slightly less
sense out of context).  Reviewing tutorials can also be accomplished
via the help menu, ?.

Note that the tutorial menu can get rather long; if it (or any other
menu) doesn't fit on your screen, you can use the < and > keys to page
up and down, or the space key to page through the menu.  All menus in
NetHack use these controls, by the way.]],
      },
      qt_tutorial_check_items = {
         output = "text",
         text = [[
Tutorial: ITEMS

It would be unreasonable to expect you to go through the dungeon with
no equipment to help, and no treasure to find.  Your inventory contains
items, which you can use to help you through the dungeon; and you can
find items in the dungeon itself, too.  Items are represented by
punctuation marks:

$ gold         In order to see what items you have at the moment, use
) weapon       the I command to list a section of your inventory; this
[ armor        will also show you a tutorial message explaining how to
%% edible item  use the items in that section (different items can be
? scroll       used in different ways).  As for items in the dungeon,
/ wand         there are two possible ways to pick them up; if the
= ring         autopickup option is on, you'll pick up items when you
! potion       step on them, and if it's off, you'll need to press ,
( tool         in order to pick up items on that square.  You can turn
" amulet       autopickup on or off by pressing @. (Note that you can
* rock or gem  have no more than 52 stacks of items in your inventory;
+ spellbook    so be selective about what you leave behind, and what
` statue       you take with you; gold doesn't take up space, though.)]],
      },
      qt_tutorial_objective = {
         output = "text",
         text = [[
Tutorial: OBJECTIVE

So, why are you here anyway? Like any adventurer, you might just be
here for the treasure; there are plenty of coins and gems in the
dungeon, and you could quite possibly gather them up and retire rich.
Your score will depend mainly on how much treasure you gather, and how
many monsters you kill, although other things affect it too.

More importantly, though, there's a rather significant artifact in the
dungeon; somewhere, probably rather deep down, is the fabled Amulet of
Yendor, allegedly desired even by the gods.  You would almost certainly
be rewarded greatly, maybe even with immortality, if you could find it
and offer it to your %G %d.

However, to start with, you should just worry about survival and
exploration.  Staying alive, gathering equipment to make yourself more
powerful, and practicing combat are all good aims early on; and you'll
want to explore the dungeon in order to gain an idea of where you want
to go.  Even though you'll eventually want to go for the Amulet, you'll
need to discover where it is and how to reach it first; so keep your
aim lower for the time being.]],
      },
      qt_tutorial_saveload = {
         output = "text",
         text = [[
Tutorial: SAVING AND QUITTING

Unlike many other games, in NetHack, death is final; you have to start
a new game from the start (there's no way to save a game and load the
save file more than once).  However, if you need to take a break, you
can save the game by pressing S; that will save and end the game.  To
reload a save file, just run NetHack again (and if applicable, specify
the same name that you specified last time); the game will load, but
your save file will be deleted.

If you really get fed up of a game, you can use #quit in order to give
up and end the game; quitting a game will end it permanently, though,
because there's no way to go back to an earlier state in a game.  So
only do this when you want to abandon the game forever.  Quitting does
give a slightly higher final score than death, though.

You can also effectively quit simply by escaping the dungeon using the
stairs that you entered by; leaving the dungeon without the Amulet of
Yendor counts as "escaping", and although you survive with all your
equipment, and your life, intact, the game will end there, and you'll
have to start a new one.]],
      },
      qt_tutorial_messagerecall = {
         output = "text",
         text = [[
Tutorial: REVIEWING MESSAGES

By now, you're likely to have seen many messages appear at the top of
the screen.  That space is used to convey any information that can't
easily be shown on the map in the middle; for instance, it reveals the
results of looking at something using the ; command, explains what's
going on during a fight, and lets you know about things your character
hears while walking around in the dungeon.

However, sometimes it's possible to play too quickly and miss a
message.  You can press control-P in order to review messages that
you've heard recently; depending on your options, it might either
display lots of recent messages, or only the most recent (in which
case repeating control-P will tell you about older messages).

If you want to do the opposite, and skip large numbers of messages,
then you can use escape at the --More-- prompt in order to skip the
remaining messages that turn (they can still be reviewed via
control-P, however). (In order to go on to the next message at a
--More-- prompt, use the spacebar or press Return.)]],
      },
      qt_tutorial_spells = {
         output = "text",
         text = [[
Tutorial: SPELLCASTING

Although so far you've probably been using melee in combat, most
spellcaster characters won't be good enough at fighting to beat
everything in melee; against more powerful opponents, a different
option will be needed.  Therefore, it's probably a good time to get
some practice casting spells.

To cast a spell, use the Z command; you can only cast spells in the
eight compass directions (and up, down, and at yourself).  For this
combat, "force bolt" is probably a good spell to use; it's easy to
cast, and can do decent amounts of damage even when used by weak
spellcasters.  Why not try it on a nearby hostile monster?

In order to power a spell, you'll need to use up some of your personal
magical power; the effort also tends to make you burn food faster.
Your magical power is listed as Pw near the bottom of the screen; it's
depleted by casting spells, and fills up again over time.  To start
with, it'll be too low to cast any but the simplest spells, but as
time goes on, you'll be able to handle more powerful ones.  Good luck
spellcasting!]],
      },
      qt_tutorial_thrownweapons = {
         output = "text",
         text = [[
Tutorial: THROWN WEAPONS

Although fighting monsters in melee is all very well, it helps if you
can get some hits in before you close in for the kill; a weakened
monster will die faster, and so will deal less damage to you in the
process.

You can throw items at a monster using the t command; good weapons to
throw include darts (which are designed for throwing), and daggers
(which work just as well thrown as when wielded).  Some characters will
be better with certain weapons than others; you can use #enhance to
see how skilled you are with various weapons (if a weapon is listed as
'unskilled', you don't know how to use it yet but can learn with
practice; if it isn't listed at all, you won't be able to learn to use
it no matter how much you practice).

Note that even spellcasters can benefit from thrown weapons; because
your magical power (Pw) can run out quickly if you rely on spells as
your major method of combat, using darts or daggers as a backup can be
an important tactic while you're waiting for your magical power to
recharge.]],
      },
      qt_tutorial_projectiles = {
         output = "text",
         text = [[
Tutorial: PROJECTILE WEAPONS

Using bows, crossbows, and slings is slightly more complicated than
fighting in melee; but the ability to hit monsters from a distance is
an important one, because it allows you to hit many monsters several
times before they can hit back, meaning you can kill them faster and
take less damage as a result.  In order to fire a projectile weapon,
you must wield the weapon (with w), then throw the projectile (using
t); the combination is recognised as firing the projectile with the
weapon, rather than throwing it by hand.

As bashing monsters with a bow or sling is unlikely to do much damage,
it's probably a bad idea to try to use a wielded projectile in melee;
instead, when the opposing monster has almost caught up to you, use
your turn to wield a melee weapon (using w) rather than trying to get
in a final shot.

You can use the #enhance command to see which weapons (including
projectile weapons) your character is good with ('basic' or better),
which they need practice to use ('unskilled'), and which they can
never learn to use effectively (unlisted).]],
      },
      qt_tutorial_elbereth = {
         output = "text",
         text = [[
Tutorial: ENGRAVING "ELBERETH"

As you go deeper into the dungeon, the monsters get more and more
dangerous, meaning you'll have to get better at combat yourself; and
if you aren't careful, you might easily be overwhelmed.  However,
there's a way to give yourself a bit of breathing space when you're
being swarmed with monsters.

You can write on the floor using a wand or your fingers, using the E
command (E- writes with your fingers); and there is one word that the
vast majority of monsters fear, the word "Elbereth".  Writing it on the
ground will therefore cause many monsters to flee from you in terror,
giving you a breathing space to recover from your wounds.

However, there are several problems with this method, too.  For one
thing, writing in the dust with your fingers is not the most accurate
method of inscription, so it's possible that one or more of the
letters will be malformed.  Also, engravings tend to get scuffed up a
lot by movement; fighting from an Elbereth square will almost
certainly wipe out the engraving and make it ineffective.  Finally,
despite the word's power, some monsters simply aren't scared of it.]],
      },
      qt_tutorial_melee = {
         output = "text",
         text = [[
Tutorial: MORE ABOUT MELEE

When attacking monsters in melee, you hit them with whatever you're
wielding in your hands.  However, some weapons are going to work better
than others; it helps to use a weapon your character has had lots of
practice with.

Your ability scores also have an effect on how effective melee can be.
Your Strength (St:) and Dexterity (Dx:) are shown at the bottom of the
screen; higher ability scores are better, and different abilities
affect different things.  Dexterity is imporant in being able to hit
enemies; a good dexterity helps you aim better both with melee and
with thrown and projectile weapons.  Strength is also important; it
helps to be strong to be able to aim a melee weapon accurately in
combat, and the stronger you are, the more damage you can do with it
if you connect.

Finally, some weapons will be more powerful than others, especially
against specific opponents.  For instance, you can expect swords to
generally hurt your enemies more than daggers do.  However, note that
your ability with the weapons in question will be just as important.]],
      },
      qt_tutorial_item_gold = {
         output = "text",
         text = [[
Tutorial: GOLD

Every adventurer loves gold; and it can be quite useful even within
the dungeon itself.  Gold is represented by the $ symbol, and takes up
no space in your inventory (but its weight can still be significant,
even though coins are quite light); the amount of gold you have is
constantly displayed near the bottom of the screen, rather than in
your inventory.

Gold adds to your score at the end of the game; but even within the
dungeon, it can still be used to buy things.  Not every monster is
hostile, and some will want to trade with you; and gold is the perfect
medium to trade with.  In particular, you may find shops which will buy
and sell items, in which case you'll want gold to be able to afford
things; but shopkeepers are not the only monsters who care about how
rich you are.

You can get information about your financial situation by using the $
command; normally, this just depends on the gold in your wallet, but
if you owe, or are owed, money, that information will also be
available that way.]],
      },
      qt_tutorial_item_weapon = {
         output = "text",
         text = [[
Tutorial: WEAPONS

With the number of hostile monsters in the dungeon, weapons tend to be
rather important.  Weapons on the map are represented with the ) symbol
(note that ammunition also counts as a weapon for this purpose).

You can use the w command to hold an object in your hands (or w- to
hold nothing); by far the most common use of this is to hold a weapon.
The object in your hands is what you'll use to hit monsters with in
melee, so it should be something that your character is skilled with
attacking with; you can use the #enhance command to see which weapons
your character is good at using ('unskilled' means your character
doesn't yet know how to use the weapon but can learn; if a weapon
isn't mentioned, your character will never know how to use it).

Other uses of weapons include using t to throw them at things (darts
are best used as thrown weapons, daggers are also good for this);
using a to hit targets two squares away (if you have a long weapon
like a lance); and using E to scratch into the floor with the weapon
(note that this damages the weapon).]],
      },
      qt_tutorial_item_armor = {
         output = "text",
         text = [[
Tutorial: ARMOR

Armor (represented by the [ character) is a useful method of
protecting yourself from the blows of your enemies; armor reduces the
chance that an enemy attack will hit, and may also reduce the damage
it deals.  Your vulnerability to attacks is called your Armor Class
(AC, shown at the bottom of the screen); an unarmored human has an AC
of 10, but you'll want to drive it much lower than that.

You can wear armor with W, and take it off again with T; you'll need a
free hand to be able to wear a shield, and likewise you can't wear two
pieces of armor that fit on the same part of the body.  Some armor is
magical, and has either a magical bonus (such as +1) to defending you,
or some other property; armor can also be inferior to normal, and have
a penalty (such as -1).  The manufacturer of armor is also relevant;
elves are good at making armor, and dwarves are even better, but
orcish armor tends to be rather low in quality.

Note that although anyone can wear armor, it hampers spellcasting;
metal armor in particular can mess up magic, and shields also make
spellcasting rather difficult.]],
      },
      qt_tutorial_item_food = {
         output = "text",
         text = [[
Tutorial: FOOD

The %% character represents an edible item; note that just because you
can eat something, that doesn't necessarily mean it's a good idea.  The
term 'comestibles' is used to represent food and other things that can
be eaten.  Eating is rather important; a character who doesn't eat for
too long will become hungry, grow weak, then eventually start fainting
from hunger and starve to death. (If you're getting hungry, this will
be shown near the bottom of the screen.) However, try not to eat when
'satiated'; you can choke to death, as well as starving.  You can eat
things (either in your inventory, or on the floor) with the e
command.  Fresh food tends to be considerably healthier than food
that's been left to rot; various types of food will spoil over time
and become dangerous, even if they were safe to eat when they first
appeared.

Note that killing monsters also often leaves a %% behind, which is the
monster's corpse.  Corpses can be eaten just the same way food can be
(as they tend to be heavy, eating from the floor is usually the best
option).  Not all are healthy, but some could give you special powers
when eaten; to some extent, you truly are what you eat.]],
      },
      qt_tutorial_item_scroll = {
         output = "text",
         text = [[
Tutorial: SCROLLS

Scrolls (?) are magical items that store spells bound up on paper.  By
reading a scroll (using the r command), you can unleash the magic
within; but it's rather wise to know what a scroll does, first;
unexpected magical effects can be rather bad for your health.  Anyone
can use a scroll, regardless of if they know any magic or not.

If you don't know what a scroll does, you'll see the scroll's label
rather than its function; within any given game, scrolls with the same
label do the same thing, but which label goes on which sort of scroll
can change from one game to the next. (You can use the \ command to
see the label/scroll mappings you've already discovered.) There may be
magical methods of determining what a given scroll does; in fact, one
of the spells commonly bound up in scroll form is a magical means of
determining the properties of an unknown item.

Magical scrolls are strictly single-use; because the scroll is being
used to capture a spell, the process of casting the spell also
destroys the scroll.]],
      },
      qt_tutorial_item_wand = {
         output = "text",
         text = [[
Tutorial: WANDS

Wands (/) are magical objects, which are generally designed to cast a
specific spell (depending on appearance; wand/appearance combinations
change from game to game).  They generally store enough magical energy
to cast that spell multiple times, but will eventually run out; if
somehow you discover the amount of energy in a wand, it will be shown
in the wand's description (e.g. (3:5) means that the wand has been
recharged 3 times, and can be used 5 more times before it next needs
to be recharged).  As wands are designed to cast spells by themselves,
you need not know magic yourself to be able to use them; just point
them in the desired direction and fire! You can use the z command to
'zap' a wand like this, to unleash its magic; note that you can only
aim in compass directions, upwards, downwards, and at yourself).

Other things you can do with wands include writing on the floor with
them (use the E command to do this), and breaking the wand with the a
command; note that breaking a wand often causes an enormous and rather
dangerous release of magical energy.  Wands also sometimes explode of
their own accord upon being recharged; the more a wand has been
recharged, the less likely it is to survive another recharging.]],
      },
      qt_tutorial_item_ring = {
         output = "text",
         text = [[
Tutorial: RINGS

Unlike wands, scrolls, and potions, rings (=) are a more permanent
sort of magical item; instead of being powered from an in-built
magical source (and therefore running out relatively quickly), most
are powered from your own body's food supply, causing you to get
hungry slightly more quickly (although some powerful rings need more
power than others).

You can put a ring on each ring finger, but only one on each hand; as
a result, you can wear up to two rings.  The put-on command, P, is used
to put on a ring; you can remove one using R.

Rings generally have an effect either on you, or on those around you;
note that not all effects are beneficial, and so you should be careful
when trying on rings whose effects you don't know. (The same way as
wands, scrolls, and potions work, rings with the same appearance will
always do the same thing in any given game, but which appearance has
which effect will differ from game to game.) You can use the \ command
to see which ring appearances you already know this game; note that
if you recognise a ring's purpose, your inventory will list it.]],
      },
      qt_tutorial_item_potion = {
         output = "text",
         text = [[
Tutorial: POTIONS

Potions (!) are magic in liquid form; being single-use magical items,
they work rather similarly to scrolls.  You can drink a potion using
the q ('quaff') command.  The main difference from scrolls is that as
potions are drunk not read, they affect the person who drank them.

Just as with most other magical items, you may not know what a potion
does upon encountering it; two potions which are the same colour will
do the same thing in any given game, but which colour of potion has
which effect will differ from one game to the next.  As some potions
can be very dangerous, be careful when quaffing potions of unknown
enchantment! Clear potions contain water; although water is normally
nonmagical, it might also be holy or unholy water, and because all
three look the same, it might be hard to tell them apart.

Other things you can do with potions include using #dip to dip items
into them, and using t to throw them at things. (Holy water is rather
effective as a thrown weapon against undead, for instance; but
splashing a potion against something normally gives a much weaker
magical effect than drinking it.)]],
      },
      qt_tutorial_item_tool = {
         output = "text",
         text = [[
Tutorial: TOOLS

The term 'tool' is used for a broad range of miscellaneous items,
shown with the ( symbol.  Although mostly nonmagical, some may have
magical powers; and even the ones that don't are nearly always useful
in some more mundane way.  Magical tools often, but not always, will
masquerade as a nonmagical tool, but there will often be some method
of telling the magical tool apart from the ordinary one.

You can use a tool using the a command; what using a tool does depends
on the tool.  Some tools can be used an unlimited number of times;
others will wear out, or run out of magic or other supplies. (For
instance, a lamp can run out of oil after a while.) There may be other
commands that make sense in relation to tools, based on their shape;
for instance, some tools are usable as weapons, and wielding them
using w so you can hit monsters with them makes sense.  Luckily, it's
normally obvious what the purpose of a tool is, so you can normally
guess what will happen when you press a and use it.]],
      },
      qt_tutorial_item_container = {
         output = "text",
         text = [[
Tutorial: CONTAINERS

Containers are a special subset of tools (and likewise, are
represented with the ( character), that can be used to store items;
bags, boxes, chests and safes are all generally types of containers.
You can use containers to carry more than 52 stacks of items around
with you, by storing in the container; however, the weight of the
items inside will still be significant, and the weight of the
container itself can be too.

To put items into or take them out of a container in your inventory,
you can use the a command to use the container (just like with any
other tool); if the container is on the floor, you can likewise use
the #loot command to do the same thing.  Boxes and chests are often
locked or trapped; you could try using a lock-picking tool on them,
kicking them (using control-D from an adjacent square) in the hope of
breaking open the lock, or trying to force the lock with your wielded
weapon using the #force command.  If you're afraid of traps, the
command #untrap can be used to search for traps on the container
(although this is not guaranteed to succeed), and to try to deactivate
them if you find them.]],
      },
      qt_tutorial_item_amulet = {
         output = "text",
         text = [[
Tutorial: AMULETS

Amulets (") are like rings in more or less every respect; they have
constant magical effects powered from your own body's food supply,
amulets of the same appearance do the same thing in one game, but may
differ from game to game, and you put them on and remove them with the
P and R commands.  Amulets are often slightly more powerful than rings,
though; and you wear them around your neck, rather than on your ring
fingers. (You can only wear one amulet at a time.)

As usual, not all magical items are guaranteed to be beneficial; so be
careful trying on amulets you don't recognise.]],
      },
      qt_tutorial_item_gem = {
         output = "text",
         text = [[
Tutorial: GEMS AND ROCKS

Gems and rocks (*) are a varied class of items; some are somewhat
valuable, whereas some are worthless, although they are rarely
magical.  Unlike other classes of items, two gems of the same colour
are not necessarily the same sort of gem; also, note that even if you
do find a valuable gem, you're unlikely to be able to sell it for very
much unless you can prove its identity to the shopkeeper.  Like gold,
valuable gems are useful in increasing your score.

Things you can do with gems and rocks include throwing them (using the
t command; this works better with non-valuable gems such as ordinary
rocks), and rubbing things on them using #rub. (Note that rocks do
more damage as sling ammunition than as thrown weapons.) Mostly,
though, what gems and rocks you do pick up are likely to be carried
around rather than used.]],
      },
      qt_tutorial_item_statue = {
         output = "text",
         text = [[
Tutorial: STATUES AND BOULDERS

The ` symbol represents stone objects like statues; your options may
also display boulders using `, although 0 is also a common choice.
Statues tend to be both useless and rather heavy, so make sure you
turn autopickup off when walking over them (you can turn autopickup on
and off using @); however, some statues may not be what they seem.

As opposed to statues, boulders are not only too heavy for almost
anything to lift (although giants and similarly large creatures have
been known to use them as weapons), but too large for the average
character to slip past.  Most likely, you'll have to push them out of
your way, or somehow destroy them, if they block your path; you can
push a boulder by walking into it (using the usual movement keys), and
you will try to squeeze past it if you can, and the boulder's movement
is somehow blocked.]],
      },
      qt_tutorial_item_book = {
         output = "text",
         text = [[
Tutorial: SPELLBOOKS

The + character represents a book, generally a spellbook; this is how
characters learn spells.  Pretty much anyone can try to learn a spell
by studying a spellbook (use the r command to read a spellbook), using
some of the book's magic to teach them the spell; but if the spell is
too difficult to learn, the attempt can backfire in a rather dangerous
manner.  As magic from the book is required, spellbooks will eventually
run out of it, fading and becoming unreadable in the process; and
similarly, you will not be able to retain the memory of how to cast a
spell forever.

Once you learn a spell, you can use the Z command to cast it (or + to
list the spells you know); some spells are easier to cast than others,
and unless your character is particularly good with magic, spells will
have a chance of going wrong when cast.  Armor also interferes with
spellcasting, and spells need a source of power; your magical energy
(Pw) is shown at the bottom of the screen, and is used to power spells
(it regenerates over time), and casting spells generally also makes
you hungrier.  Lower-level spells are less powerful, but easier and
cheaper to cast, so they may be good spells to practice casting with.]],
      },
      qt_tutorial_callitem = {
         output = "text",
         text = [[
Tutorial: CALLING ITEMS

With all the items around with unknown purposes, it can be very useful
to keep notes on your beliefs about the function of various items.
You are about to be given the chance to give a name to a certain
appearance of item (probably because one was destroyed and you saw the
effects of it); this will replace the appearance from now on, thus
reminding you of what you wrote.

An example: suppose you saw an orange potion break, and got a glimpse
of the sort of effects it had; the game might prompt you with "Call an
orange potion:".  This lets you rename orange potions; so, if for
instance you suspected that the potion was dangerous, you could write
"dangerous" at the prompt, and "orange potion" would from then on be
known as "potion called dangerous".  As two orange potions always have
the same effect, you'd know to avoid quaffing those potions in future.

If you have nothing to say about what you think the item in question
does, you can just leave the prompt blank, to leave the item with its
old name. (Also, note that you can use #name then n to call items
yourself, rather than waiting for the game to prompt you.)]],
      },
      qt_tutorial_artifact = {
         output = "text",
         text = [[
Tutorial: ARTIFACTS

Some items are one of a kind! Artifacts are items with special powers,
often crafted long ago with magic that can no longer be duplicated, or
made by the gods or some similarly powerful beings.

An artifact is identified by its proper name, which cannot be changed;
it will function perfectly well as the item it was based on (e.g. an
artifact longsword can do everything a longsword can), but will often
do other things as well; so for instance, artifact weapons often do
much more damage than a regular weapon would, and artifact tools
normally have one or more permanent magical effects.

Some artifacts tend to be rather picky about who uses them, however;
if you're not the sort of person the artifact wants as its owner, it
might damage you when you touch it, or even refuse to be used
altogether.  As always, then, it helps to be wary around artifacts of
unknown function. (Using the / command to ask about the artifact can
be a useful source of information, though; artifacts are sufficiently
famous that your character may have heard of them before ever even
going into the dungeon.)]],
      },
      qt_tutorial_randappearance = {
         output = "text",
         text = [[
Tutorial: RANDOM-APPEARANCE ITEMS

A large number of magical items have appearances that vary from game
to game; although within any one game two items with the same
appearance (apart from tools, gems and rocks) will have the same
function, knowledge of what an item does in one game will not help in
the next.  Therefore, one of the goals in each game is to try to
determine which item does what.

To avoid having to remember this information, you can use the #name
commands to make notes about the items.  If you have an idea of what
one particular appearance of item does, you can use #name followed by
n to produce an alternative name for the appearance; e.g. you could
change "orange potion" to "potion called dangerous" if you decided
that all orange potions in the game were dangerous.

You might also get information about one particular item; for
instance, a curse on one orange potion does not mean that all others
are cursed, but if you found out about it, you'd want to remember.
Using #name followed by y lets you write on an individual item to make
notes about it; this will not affect other items which look the same.]],
      },
      qt_tutorial_levelup = {
         output = "text",
         text = [[
Tutorial: LEVELING UP

Your character just gained an experience level! Going up in level
means you can survive more damage before death, store up more magical
power to cast spells with, heal faster, and become better at casting
spells; some races and all classes will also gain various ongoing
powers as they level up, such as sleep resistance or the ability to
move faster than normal.  Many other things also become better for you
the more experienced you are.

It's important to keep on killing monsters so that you can continue
going up in level; as you go deeper, the enemies will get more
powerful, and you'll likely need the benefits that higher levels bring
in order to survive in the deeper parts of the dungeon.]],
      },
      qt_tutorial_rankup = {
         output = "text",
         text = [[
Tutorial: RANKS

You just leveled up again; and as a result, your rank has changed;
you're now have the rank "%r".

Although your rank is mostly irrelevant, a better rank does give you a
somewhat better professional status; a good rank lets you prove to
other people in the game that you have what it takes to perform well
at your chosen job.]],
      },
      qt_tutorial_abilup = {
         output = "text",
         text = [[
Tutorial: ABILITIES

One of your abilities just increased! Demonstrating the use of an
ability helps increase your abilities, making your character more
powerful.

There are six abilities, shown at the bottom of the screen:

Strength     (St:) Affects melee combat and carrying capacity.
Dexterity    (Dx:) Affects your ability to aim at and hit things.
Constitution (Co:) Affects your ability to survive injury.
Intelligence (In:) Affects the power of your spells.
Wisdom       (Wi:) Affects the rate at which you regain magic.
Charisma     (Ch:) Affects enemy's reactions towards you.

Unlike most other stats, abilities are rather hard to change; either
you need to continuously exercise the use of the ability to increase
it, or you need to use some magical means of increasing your abilities
to higher values.  You might also find items that boost one or more of
your abilities, letting you function better for as long as you're
using them.]],
      },
      qt_tutorial_abildown = {
         output = "text",
         text = [[
Tutorial: ABILITY ABUSE

One of your abilities (Strength, Dexterity, Constitution,
Intelligence, Wisdom, or Charisma) just decreased.  This happens when
you abuse your mind or body; it can be the result of unwise actions,
of not paying attention to training in a particular sort of activity,
or of actions that monsters or traps tried against you (for instance,
poison tends to seriously abuse abilities, making them very likely to
drop).  Unless you can find a magical way to restore your abilities,
you'll need to train in the appropriate area to help to bring your
abilities back up:

Strength     (St:) Affects melee combat and carrying capacity.
Dexterity    (Dx:) Affects your ability to aim at and hit things.
Constitution (Co:) Affects your ability to survive injury.
Intelligence (In:) Affects the power of your spells.
Wisdom       (Wi:) Affects the rate at which you regain magic.
Charisma     (Ch:) Affects enemy's reactions towards you.]],
      },
      qt_tutorial_dlevelchange = {
         output = "text",
         text = [[
Tutorial: GOING DEEPER

Welcome to a deeper level of the dungeon! As you go deeper into the
dungeon, it becomes deadlier and more dangerous; the monsters will be
better at fighting, the traps will be nastier, and you'll need to come
up with ever more ingenious methods to be able to continue onwards.

Note that it's possible to return to previous levels; although most
levels are generated randomly, levels don't change just because you
leave them; the layout will stay the same when you return.  Monsters
on a level will stay in stasis while you aren't on the level, meaning
that you can use stairs to run from a fight; but time on the level
still passes, so for instance, food will continue to spoil while you
aren't there.  As a result, the main reasons to return to a previous
level are to continue exploring from elsewhere on that level, to pick
up items that you left behind, and to run away from problems on deeper
levels, in favour of healing somewhere that's already been cleared
out.

The dungeon level (Dlvl:) you're on is shown in the bottom-left corner
of your screen. (Some levels have names rather than numbers).]],
      },
      qt_tutorial_damaged = {
         output = "text",
         text = [[
Tutorial: HEALTH

Ouch! You just lost some of your health.  Your health is measured in
Hit Points (HP), and is visible at the bottom of the screen; the first
number counts your current hit points, and the number in parentheses
is the maximum number of hit points you can have, when you're fully
healed.

You should be careful to try to conserve your HP, because when you run
out of them, you die.  Luckily, your wounds will slowly heal over time;
so you can regain your HP simply by avoiding being hit for a while.  If
you end up critically low on HP, you'll probably want to find some way
to accelerate the healing process; if you find a reliable way to do
this, make sure you remember what it is, to help you in emergencies.

Your Constitution (Co:) ability is also shown at the bottom of the
screen; it reflects how naturally good you are at surviving injury.
A high Constitution will help you to heal faster, and give you more hit
points to start with; so improving your Constitution is a helpful way
to survive better.]],
      },
      qt_tutorial_pwused = {
         output = "text",
         text = [[
Tutorial: MAGICAL POWER

It looks like you just used some of your magical power.  There are
several uses for it, although the main use is to cast spells.  The
amount of magical power you have is shown as Pw at the bottom of the
screen; the first number counts your current power reserves, and the
number in parentheses shows the maximum amount of power your body can
store at one time.  You regenerate magical power slowly over time;
nothing particularly bad happens if you run out, but if you don't have
enough power, you can't cast spells, or perform other actions that
need you to power them with personal magic.

The amount of power you have depends on your Wisdom ability (shown as
Wi: at the bottom of the screen); gaining more Wisdom will give you
more maximum power, and help you regenerate it faster.  There's another
ability relevant to magic; your Intelligence ability (shown as In: at
the bottom of the screen) also helps regenerate your magical power,
makes it easier to understand spellbooks you might find, and makes
your spells more powerful.]],
      },
      qt_tutorial_pwempty = {
         output = "text",
         text = [[
Tutorial: ALTERNATIVES TO MAGIC

It seems that your magical power has now run so low that you can't
cast any more spells, until it regenerates.  If your character relies
on magic, then you could be in trouble.

There are several things you could do here.  If there are no monsters
around, you might want to retreat to an area of the dungeon you've
already cleared out, to wait for your power to regenerate.  You might
also want to let your pet do the fighting for a while, if you have
one; although you probably don't want to expose it to too much danger.

Alternatively, you could try some non-magical combat yourself.  Even
spellcasting classes should practice with a weapon so that they can
fight in melee if they have to; thrown or projectile weapons are also
very useful for weakening monsters without exposing yourself to danger
in return.  It helps to get some practice with weapons against weaker
monsters, so that you aren't completely helpless against stronger
monsters when you run out of magic against them.]],
      },
      qt_tutorial_acimproved = {
         output = "text",
         text = [[
Tutorial: USING ARMOR

Your Armor Class (AC) has just decreased; as Armor Class measures how
vulnerable you are to attacks, this is a good thing.  Being better
armored reduces the chance that monsters will be able to hit you (as
armor helps to parry attacks); if you manage to get sufficiently well
armored, your body will be so well covered that the armor will be hard
to avoid, meaning that even when the attacks do connect, their damage
will be reduced.

Of course, there are always tradeoffs.  One potential problem with
wearing a lot of metal armor is that it makes spellcasting
difficult, although this is not a problem unless you intend to cast
spells.  Another potential issue is that the weight of armor can
encumber you, making you move more slowly and making you more
vulnerable as a result.  Shields take up a hand that you could be using
to hold a weapon.  Finally, there are several magical items that need
to be worn on parts of your body normally used for armor; so there may
eventually be a trade-off between armoring yourself well and gaining
magical benefits.  However, especially early on, the value of armor is
often large enough to outweigh all this.]],
      },
      qt_tutorial_gainedexp = {
         output = "text",
         text = [[
Tutorial: EXPERIENCE

You just gained some experience! By far, killing monsters is the most
common way to gain experience, although there are a few weirder
methods (some experienced NetHack players have completed the game
without a single kill, although this is incredibly difficult to
achieve).

Accumulating experience is important, because it will eventually allow
you to level up, giving you a boost in maximum health and magical
power, making you better at spellcasting, and gaining you new special
abilities.  As a result, early on in the game you probably want to kill
more or less everything hostile, for the practice; later on, though,
some monsters will be better avoided than fought.

Pets can get experience too; it's getting the final blow in that gives
the experience, so if you're trying to keep a pet, you'll need to
think carefully about which of you should get the experience from any
given death.]],
      },
      qt_tutorial_hunger = {
         output = "text",
         text = [[
Tutorial: HUNGER

You're getting hungry; now would be an especially good time to eat
something.  When you're hungry, your hunger status (such as "Hungry",
"Weak", or "Fainting") will be displayed at the bottom of the screen;
merely being hungry isn't disastrous, but becoming too hungry can kill
you easily, either directly by starving to death, or indirectly by
making you too weak to defend yourself.

Therefore, you'd better try to find something to eat.  The corpses of
monsters are often edible; although many sorts of food spoil over
time, so fresh corpses are more likely to be healthy.  An important
skill is learning what's safe to eat, and what will kill you.

You can also try eating food from your inventory; you can use I to see
which "comestibles" you're carrying around.  Some sorts of food are
best saved for when you can't find any monsters around to kill and
eat; food designed for people to eat is often especially nutritious
and can get you out of starvation trouble quickly.

Use the e command to eat food, from the floor or your inventory.]],
      },
      qt_tutorial_satiation = {
         output = "text",
         text = [[
Tutorial: SATIATION

There's an opposite problem to hunger; eating too much can make you
satiated, which can be bad for your health.  When satiated, eating yet
more food could be a bad idea, as there's a limit to how much your
stomach can hold; you might end up choking to death!

If there are things you really feel you need to eat, you could try
burning off some of those calories first; several sorts of magic
(spells, rings, and amulets) power themselves from your body's food
supplies, so you could try using those.  You might also find other
calorie-burning actions as the game progresses.

Mostly, though, the easiest way to burn off the calories will simply
be to continue adventuring until you naturally end up hungry again;
after all, food doesn't last forever.]],
      },
      qt_tutorial_status = {
         output = "text",
         text = [[
Tutorial: STATUS

Status effects are effects like confusion or blindness that affect
your character negatively, making it harder to act correctly.  They're
displayed at the bottom of the screen.  Some status effects can kill
you directly, as you die from illness or food poisoning; others are
dangerous more indirectly, as they impair your senses or your actions,
making it considerably easier for monsters to kill you.  Therefore,
it's probably advisable to try to cure the status.

If the status is not fatal in itself, you may be able to cure yourself
simply by waiting somewhere where there are fewer monsters, trying to
survive until your body recovers; but this will just make things worse
in some cases.  Your main alternatives are to try to find an item or
spell that will cure you.  Also, although it's too late now, it's often
possible to avoid the status in the first place; for instance, it's
advisable not to eat monster corpses that will give you status
effects (you can find out which these are by experimenting over the
course of several games).  Asking for divine intervention with #pray
may save your life, too, in the most serious cases; but this should be
treated as a last resort, as deities dislike being bothered too much.]],
      },
      qt_tutorial_enhance = {
         output = "text",
         text = [[
Tutorial: ENHANCING SKILLS

Practicing a particular area of fighting, weaponry, or magic, will
allow you to get better in it, to some extent.  Your character now has
enough practice to be able to 'enhance' a skill, becoming better at
it; to do this, use the #enhance command.

Note that the total amount of skill enhancement you can manage depends
on your experience level; as a result, if you somehow get enough
practice to enhance a skill you aren't planning to use, it may be a
good idea to decide not to enhance that skill, so as to save your
skill slots for some other skill.  Likewise, if there's some skill you
are planning to use in the future, it may be worth practicing it now,
so that it's enhanced ready for you to be able to use it.

Only skills listed in the #enhance list are enhanceable at all; most
characters will have some skills they'll never be able to get good
at.  Similarly, the maximum competence you can gain in a skill (basic,
skilled, expert, master, grand master) will depend on your class; for
instance, a competence of 'master' or better can only be achieved in
martial arts or unarmed combat, and even then, only by some classes.]],
      },
      qt_tutorial_polyself = {
         output = "text",
         text = [[
Tutorial: POLYMORPHED

It seems that you've just turned into a monster! Monsters tend to have
their own strengths and weaknesses, which are likely quite different
from the ones you have in your original form; it may help to
experiment a bit to see what you can and can't do now.  It's entirely
possible that some things you took for granted, such as the ability to
open doors, are not possible in this form.

Some monsters have a special attack or ability; if you do, you'll have
been told what it was, and you can use the #monster command to use it.
Some female monsters can also lay eggs, which you can do with the #sit
command.  If you're stuck in monster form, you may as well try to make
the most of it!]],
      },
      qt_tutorial_engraving = {
         output = "text",
         text = [[
Tutorial: ENGRAVINGS

There seems to have been something written on the square you stepped
on.  Although it's rare for writing to survive a long time in a
dungeon, writing on the floor can be a useful technique to avoid
getting lost, or to record information; other denizens of the dungeon
might also be using it to try to communicate with you.

You can write on the floor yourself, too, using the E command.  You can
write in the dust with your fingers using E-, but such writing tends
to erode incredibly quickly; scratching into the ground with a metal
weapon or similarly hard object will last longer, but note that this
will damage the weapon.  Some magical wands can be used for writing,
too.]],
      },
      qt_tutorial_majortrouble = {
         output = "text",
         text = [[
Tutorial: PRAYER

You seem to be in serious trouble! Sometimes, you end up badly
injured, starving to death, have both hands welded to a cursed weapon,
or have some equally serious issue; and in such cases, there's a last
resort you can try.

As long as you've been careful not to do anything too sacrilegious,
and have been advancing the cause of your religion (killing hostile
monsters is generally a good way to do this), calling upon your
%G (%d) may help.

Prayer is accomplished by the #pray command; although you can't rely
on divine intervention too often, it can get you out of problematic
situations reliably, if you don't do it so much that your %G
becomes annoyed.  Care is needed; an annoyed deity can do all sorts of
drastic things to you! If you do annoy your %G by mistake, or
you want to please them so you can pray more quickly, you might want
to find an altar dedicated to them and use #offer to sacrifice a
recently killed monster; as long as the offer isn't offensive, you
can gain standing in the eye of your %G this way.]],
      },
      qt_tutorial_burden = {
         output = "text",
         text = [[
Tutorial: ENCUMBERANCE

One problem that every adventurer seems to face is the problem of
hauling all that treasure around.  Your Strength ability (shown as St:
at the bottom of the screen) is important in influencing how much you
can carry (and also has a large effect on the effectiveness of your
fighting in melee); you generally want to be strong to be able to
carry a lot of gear. (Constitution (Co:) is also relevant.)

The more you have on you, the more and more burdened you become; if
you're having trouble carrying your equipment, the bottom of the
screen will show your encumberance status, ranging from "Burdened"
through "Stressed", "Strained", and "Overtaxed" up to "Overloaded".
The more encumbered you are, the slower you'll perform all actions,
and the more often you'll need to eat; being slower means that the
movements of monsters will seem faster by comparison, which could be
rather dangerous for you.  As a result, try discarding items to save
weight; you can come back to them later.

You can use the d command in order to drop an unwanted item; dropping
several items now is probably a good idea.]],
      },
      qt_tutorial_equipcurse = {
         output = "text",
         text = [[
Tutorial: CURSES

Some items bear curses, which make them considerably worse; you seem
to have come across such an item.  Curses are generally not incredibly
bad; mostly they make an object less effective than it would be
otherwise, although sometimes they have other negative or detrimental
effects.  Cursed equipment also tends not to want to be unequipped; you
normally need to find some magical method of lifting the curse to be
able to get rid of it.

There's an opposite to curses, too; an item can be blessed, which will
make it more beneficial (or in the case of dangerous items, less
bad).  For instance, demons are injured more badly by blessed weapons.
A blessing or a curse on water is what makes it holy or unholy water,
respectively.

Your inventory will list a blessing or a curse on any objects for
which you find out that information; if you know for certain an object
is neither blessed nor cursed, it'll be marked as "uncursed".  There
won't be a notation at all if you aren't sure, though.]],
      },
      qt_tutorial_mailscroll = {
         output = "text",
         text = [[
Tutorial: MAIL

You've got mail! If someone sends you a message, a mail daemon will
turn up and give it to you; it'll appear in the form of a scroll, and
reading the scroll will allow you to check what the message was.  The
form of the message will depend on where and how you're playing; if
you're on your own computer, then the scroll will normally be a
response to the receipt of email, in which case you can read the
scroll to read the email; but when playing on a public NetHack server,
the mail will probably be related to your game, sent by someone
watching.  In this case, you can effectively send a reply by pressing
#, then space, then typing in your message and leaving it on the
screen for a while; whoever's watching your game (and sent the mail)
will be able to read the response on your screen.

If you get fed up of being sent mail all the time, you can turn this
option off, by using O to enter the options screen, and turning off
the 'mail' option.]],
      },
      qt_tutorial_caster_armor = {
         output = "text",
         text = [[
Tutorial: SPELLCASTING AND ARMOR

Spellcasting is always a matter of trade-offs; using up magical power,
and burning calories, are things that cannot be kept up forever, so
you generally need to think carefully about when to use spells and
when to use more mundane methods.  There's another issue that causes
problems when spellcasting, though; whether to wear armor.

Metal armor tends to block magic, and shields get in the way of the
complex hand motions needed to cast a spell properly; as a result,
wearing metal armor or a shield can really mess up your chances of
casting spells correctly.  You can use + to see your spell failure
chances, to help you make an informed decision about whether the
armor is worth it or not.  If you're particularly good at magic, it's
possible that easier spells will be 100%% successful despite the
armor; in this case, you might want to confine yourself to low-level
spells, and wear the armor for extra protection.  Later in the game,
though, you'll probably want to cast higher-level spells, and may have
to ditch the armor (or change to non-metallic armor) as a result.  If
you aren't that good at magic, then it may also be worth considering
just ignoring magic altogether and using whatever armor you can find.]],
      },
      qt_tutorial_weapon_skill = {
         output = "text",
         text = [[
Tutorial: WEAPON CHOICE

The weapon you're currently wielding isn't one that your character is
particularly good with.  Choosing an appropriate weapon can be quite a
problem, especially if none of the weapons that you've learnt so far
are particularly effective.

You can use the #enhance command to see which weapons your character
knows how to use, and which ones you can learn; any skill that's
listed can be learnt, but you'll never be any good with weapons that
aren't mentioned.  If you do decide to learn a new weapon, make sure
it's one that it's possible to learn, and then practice it against
weaker enemies; with a weapon you aren't trained in, you're going to
find it hard to hit and to deal significant damage, until you've had a
lot of practice. (It may be worth going higher up in the dungeon to
find weaker opponents to train against, as a result.)

Alternatively, you could just switch back to a weapon you already know
how to use; most characters will be able to become highly proficient
with their starting weapon, over time, so that can be a good choice.]],
      },
      qt_tutorial_engulfed = {
         output = "text",
         text = [[
Tutorial: ENGULFED

It seems that you've been swallowed by a monster! Various monsters
prefer to engulf, swallow, or otherwise absorb their prey whole,
rather than relying on the usual methods of combat.  As a result,
you're going to have to fight your way out from the inside.

When engulfed, all 8 compass directions count as attacks against the
monster; so for instance, moving in any direction will melee it.  You
can use spells inside a monster, too (casting in any direction); and
likewise, zapping wands at it may be a useful way to escape.

Generally speaking, you need to get out before the monster does
whatever unpleasant things it's planning to do to you; being digested
is a rather final end to a game, and the insides of most monsters will
deal damage to you, or injure you some other way.  However, you're
unlikely to have much trouble hitting the monster from here!]],
      },
      qt_tutorial_death = {
         output = "text",
         text = [[
Tutorial: DEATH

Every game comes to an end, and unfortunately, this one has ended with
your character's death.  It looks like someone else is going to have to
retrieve the Amulet.

When you die, your score is calculated, based mostly on the treasure
you had and the monsters you killed.  You're also told what items you
had, and what your character was like; the information you're shown
this way can help you to figure out what went wrong.  Death in NetHack
is fatal; you'll need to start again with a new character.  Learn from
your death, and try to do better next time!

If you keep on dying, there's an easier mode you can play in to gain
some practice, which doesn't count for the high score tables but in
which death is not fatal, so you can keep on trying things out.  To
enter "explore mode", as it's called, type X whilst playing a game;
that will convert the game into a non-scoring explore-mode game.  Once
you get the hang of playing, though, go back to normal play for your
games after that; wins in explore-mode don't count.]],
      },
      qt_tutorial_shopentry = {
         output = "text",
         text = [[
Tutorial: SHOPPING

A dungeon may seem a rather strange place to set up a shop, but some
hardy shopkeepers attempt to make the best of the commercial potential
that a dungeon has to offer.  You can use your money to buy the items
on offer in the shop; or if you have items that the shopkeeper is
interested in, you can try selling them.

Apart from the square just inside the door (which doesn't count as
part of the shop), items in the shop are the shopkeeper's property;
you can pick them up to examine them, but the shopkeeper will get
annoyed if you damage them.  Once you're holding an item you want to
buy, you can use the p command to pay for it, and it'll become yours.
Similarly, you can drop items inside the shop, and if the shopkeeper
is interested he or she will offer to pay for them (if you decline the
offer, you can simply pick them back up again).

Note that leaving autopickup on in a shop is generally a bad idea; you
can use the @ command to turn autopickup on or off. (You don't want to
end up buying everything in the shop by mistake!)]],
      },
      qt_tutorial_shopbuy = {
         output = "text",
         text = [[
Tutorial: PURCHASING ITEMS

Well, you're now holding one of the shopkeeper's items! Try not to
damage it, or you'll have to pay for it. (Of course, if you're
planning to buy it anyway, you could use it now and pay for it later;
but paying first is a prudent strategy, as shopkeepers are used to
having to take their produce back by force.)

There are three commands that you may find useful in relation to
unpaid items; you can use p to pay for an item, $ to check how much
money you owe the shopkeeper (or vice versa), and d to drop the item
if you decide you don't want it after all.

Note that as an alternative to picking up items to find out how much
they cost, you can use the #chat command while standing on the item,
to talk to the shopkeeper; this can help to reduce the chance that you
damage an item that you can't afford to pay for.

Your Charisma ability (shown as Ch: at the bottom of the screen) is
useful when shopping; a stronger personality and more attractive
manner can help you to haggle better prices.]],
      },
      qt_tutorial_mines = {
         output = "text",
         text = [[
Tutorial: THE GNOMISH MINES

It's long been rumoured that the gnomes have built a mine complex
somewhere in the dungeons; and it looks like you just found it.  There
are several side-branches to the main dungeon, and the mines are the
first one; although branches are mostly optional, they can be
rewarding. (What adventurer wouldn't want to explore a working gem
mine?)

In general, levels only have two sets of stairs (one going up, one
going down); but the entries to branches are different, having another
set of stairs that leads to the branch. (The level before the Mines,
for instance, has some stairs leading to the Mines, and some stairs
continuing along the main dungeon.) The ends of branches are also
special; eventually, if you explore right the way to the bottom of the
mines, you'll hit a dead end which has stairs up but no stairs down.
Most players do choose to (immediately or eventually) explore side
branches that they find, though; the riches to be found there are
often similar or better to those in the main dungeon.]],
      },
      qt_tutorial_sokoban = {
         output = "text",
         text = [[
Tutorial: SOKOBAN

Welcome to a dungeon side-branch! Although mostly optional, branches
of the dungeon can be rewarding to explore, even if they do eventually
lead to dead ends.

You're now in Sokoban, a rather unusual area of the dungeon full of
boulder puzzles.  The aim here is to push the boulders into the pits or
holes; that way, you can complete a path leading to the next level.
It's possible to complete all the puzzles here according to the
traditional Sokoban rules; however, by taking NetHack physics, and the
possibility to use magic, into account, you may find certain rather
more ingenious ways to solve the puzzle.

In order to help you solve the puzzles in an informed manner, you'll
be magically informed of the map on each level of Sokoban, so the aim
here is mostly to solve the puzzles.  Items and monsters do not show up
until they're in your vision range, though; so exploring the levels
can still be useful.  There's no undoing moves; so think carefully
before moving any of the boulders!]],
      },
      qt_tutorial_oracle = {
         output = "text",
         text = [[
Tutorial: THE ORACLE

Welcome to the Oracle! Although most of the levels in NetHack are
generated at random, some will have predetermined areas, or be unusual
in various other ways; for instance, the Oracle of Delphi has a
specially designed room all to herself, from which she makes
prophecies.

You can chat to the Oracle using the #chat command; she'll be willing
to give you advice in return for money.  The Oracle's consultations are
full of useful, if potentially obscure, information about the game;
you can gain a lot of information that way.

There are more 'special levels' deeper in the dungeon; but you'll have
to figure out how those ones work for yourself.]],
      },
   },
   Arc = {
      assignquest = {
         synopsis = "[%nC is after %o.  Locate %i and try to get to %O first.]",
         output = "text",
         text = [[
"No sooner had we published our latest research on locating the fabled
%o, than %n announced that %nh and %nj team were heading
to %i to claim it first.

"%nS methods of archeology are abhorrent.  %nH will blast %nj
way into the temple with no regard for anything but %nj prize,
destroying priceless pieces of history.  They are well-funded and outnumber
us.  You are the only one who might stand a chance of beating them to it.

"Go through the great jungle and locate the ancient entrance to
%i.  Somewhere deep within it, you will find
%o.  They will not be watching that way, I think.

"Make haste.  %nC must not be allowed to make off with %o.

"May the wisdom of %d be your guide."]],
      },
      badalign = {
         synopsis = "[\"%pC, you have strayed from the %a path.  Purify yourself!\"]",
         output = "text",
         text = [["%pC!  I've heard that you've been using sloppy techniques.  Your
results lately can hardly be called suitable for %ra!

"How could you have strayed from the %a path?  Go from here, and come
back only when you have brushed up on your basics."]],
      },
      badlevel = {
         synopsis = "[%pC, a mere %r is too inexperienced.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to undertake such a demanding
quest.  A mere %r could not possibly face the rigors demanded and
survive.  Go forth, and come here again when your adventures have further
taught you."]],
      },
      discourage = {
         "\"%lC is weak! %lH is blind to the success of my methods.\"",
         "\"Ah, this bell? An ancient tool that has opened the way to many discoveries!\"",
         "\"Surrender to me, %p.  I will let you return to %H in shame.\"",
         "\"If you are the best %l can send, I have nothing to fear.\"",
         "\"What are a few walls worth, compared to the treasure behind them?\"",
         "\"%lC names me chaotic.  I consider myself pragmatic.\"",
         "\"Do you mean to kill me? Are we not professionals?\"",
         "\"Ancient artifacts don't belong in a museum! They're meant to be used!\"",
         "\"Soon, all the pupils at your college will want to study under me!\"",
         "\"Why do you strive against me? Abandon %l and we can work together.\"",
         "\"You'll never get anywhere gingerly dusting in the dirt.\"",
         "\"Oh, you had trouble with mummies? I just blew them up.\"",
         "\"I will blast you to the heavens!\"",
         "\"I did not expect you to demand death if you could not have %o.\"",
         "\"I got here first! Go find some other dig site!\"",
         "\"Is this what it's come to? Taking someone's find by force?\"",
      },
      encourage = {
         "\"To delve into %i, you must pass many traps.\"",
         "\"The jungle trek is dangerous.  You may need to cut your way through it.\"",
         "\"Beware the wild beasts you will surely encounter on your journey.\"",
         "\"The tomb has never been opened.  None can say for sure what lies within.\"",
         "\"You may encounter mummies.  Beware their withering touch.\"",
         "\"%nC has always had a penchant for using explosives.\"",
         "\"Remember not to stray from the true %a path.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"%nS henchmen are just as ruthless as %nh is.\"",
         "\"Do not listen to %ns lies.  The end does not justify the means.\"",
         "\"%o is a mighty scrying device.  %nC will see you coming if %nh gets it.\"",
         "\"Wearing %o should defend against many types of rays.\"",
         "\"Expect %i to be warded against magical mapping.\"",
      },
      firsttime = {
         synopsis = "[You arrive at %H.]",
         output = "text",
         text = [[
You are suddenly in familiar surroundings.  The buildings nearby seem
to be those of your old alma mater.

Everything looks fine on the surface, but you remember %ls plea
for help, and know that something must be wrong.]],
      },
      goal_alt = {
         text = "You have returned to the lowest level of the tomb.",
      },
      goal_first = {
         synopsis = "[Explosions continue deep in the temple.]",
         output = "text",
         text = [[
As you press into the depths of the temple, you notice cracks in the
walls, and the blasts of explosions grow ever more powerful.  You realize
that %n and %nj team must already be nearby.

You hope the continued blasts mean that they have not found
%o yet, and you are not too late.]],
      },
      goal_next = {
         text = "You return to the deepest depths of the temple.",
      },
      gotit = {
         synopsis = "[%o is what you were looking for.  Take it back to %l.]",
         output = "text",
         text = [[
As you hold %o, you feel the weight of its long and storied
history, and feel a profound sense of respect.  You know you must take
it to %l for analysis.]],
      },
      guardtalk_after = {
         "\"Truth be told, I don't actually like fedoras that much.\"",
         "\"Hey, got any spare potions of booze?\"",
         "\"I guess you are guaranteed to make full professor now.\"",
         "\"So, what was worse, %n or your entrance exams?\"",
         "\"%oC is impressive, but nothing like the bones I dug up!\"",
         "\"Snakes.  Why did it have to be snakes?\"",
      },
      guardtalk_before = {
         "\"Did you see Lash LaRue in 'Song of Old Wyoming' the other night?\"",
         "\"Hey, got any potions of hallucination for sale?\"",
         "\"Did you see the bracelet %l brought back from the last dig?\"",
         "\"So what species do *you* think we evolved from?\"",
         "\"So you're %ls prize pupil!  I don't know what he sees in you.\"",
         "\"Ever since we dug up that burial ground, I think we've been cursed.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and sacrifice it at the altar of %d.]",
         output = "text",
         text = [["Congratulations, %p.  I wondered if anyone could prevail against
the Wizard and the minions of Moloch.  Now, you must embark on one
final adventure.

"Take the Amulet, and find your way onto the Astral Plane.
There you must find the altar of %d and sacrifice the
Amulet on that altar to fulfill your destiny.

"Remember, your path now should always be upwards."]],
      },
      killed_nemesis = {
         synopsis = "[%nC collapses to the ground and dies.]",
         output = "text",
         text = [[
%nC collapses to the ground, wounded beyond any possibility of
healing.  For a moment, it seems as though %nh is trying to say
something, but the life leaves %ni before %nh can say it.

You are unhappy that it had to come to this, and wonder if you went too
far by killing %ni.]],
      },
      leader_first = {
         synopsis = "[\"You have returned, %p, to a difficult task.\"]",
         output = "text",
         text = [["Finally you have returned, %p.  You were always
my most promising student.  Allow me to see if you are ready for the
most difficult task of your career."]],
      },
      leader_last = {
         synopsis = "[\"%pC, you have failed us.  Begone!\"]",
         output = "text",
         text = [["%p, you have failed us.  All of my careful training has been in
vain.  Begone!  Your tenure at this college has been revoked!

"You are a disgrace to the profession!"]],
      },
      leader_next = {
         text = [["Again, %p, you stand before me.
Let me see if you have gained experience in the interim."]],
      },
      leader_other = {
         text = [["Once more, %p, you have returned from the field.
Are you finally ready for the task that must be accomplished?"]],
      },
      locate_first = {
         synopsis = "[You arrive at the ruined edifice, and feel an explosion.]",
         output = "text",
         text = [[
The trees give way ahead, as the ground abruptly steepens into a
mountainside.  Carved into the mountainside are the clear signs of ruins
of a once-great edifice -- %i.  It's not clear
where the entrance is, if indeed there still is one.

A short tremor runs through the ground as you stand there.  Alarmed, you
realize that it was a detonation.  %nS team must already be here
and blasting their way into the other side of the mountain.]],
      },
      locate_next = {
         text = "Once again, you are near the entrance to %i.",
      },
      nemesis_first = {
         synopsis = "[\"The treasure is mine.  You dispute this?  Come, then!\"]",
         output = "text",
         text = [[
%nC levels a piercing gaze at you.  A slow smile comes across %nh
face.

%nH briefly tips %nj hat to you.  "So %l did send someone
after all.  Alas, it seems that I was faster, so the treasure is mine."

%nH lifts %nj arm, admiring the gleam of %o on it in the dim
light, then glances at you again.  "Ah.  I see you will not accept your
loss gracefully.  I should not have expected better from one of
%l's pupils.  Very well then: come!  And you shall see what
I am made of."]],
      },
      nemesis_next = {
         synopsis = "[\"You are foolish.  Accept that %o is not yours.\"]",
         output = "text",
         text = [[
"You are entirely too bloodthirsty, %p.  This is foolishness.

"If you insist on being beaten into the dirt until you accept that
%o belongs to me now, though, I am happy to oblige."]],
      },
      nemesis_other = {
         text = "\"Your misguided persistence is growing tiresome.\"",
      },
      nemesis_wantsit = {
         text = "\"You have no claim to %o!  Return it!\"",
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[%lC instructs you to take %o with you.]",
         output = "text",
         text = [[
You briefly tell the details of your journey to %l.

%lH takes %o and holds it briefly, gazing into its surface.
"So, %n is defeated.  I regret the conflict it caused, though I
cannot say I sympathize with a man who so grievously destroyed the
archaeological record in order to get at one piece of treasure.

"One thing though that I now realize %nh was right about:  %o
cannot sit for decades gathering dust in some storeroom, or even a
museum.  It yearns to be wielded by one who is worthy.  I can think of
none more worthy than you.

"Now that we have photographed %o and recorded its
characteristics, it seems fitting that you should keep it for further
study.  I expect that you will have a fine treatise published soon.

"May the blessings of %d follow you and guard you."]],
      },
      offeredit2 = {
         synopsis = "[\"Resume your search for the Amulet beyond the magic portal to %Z.\"]",
         output = "text",
         text = [["
Lord Carnarvon raises an eyebrow and hands Itlachiayaque back to you.
"We've already documented this as thoroughly as we can right now.
Perhaps some day when your great quest for the Amulet is complete, you
can return it here.

"For now, take it back with you through the magic portal to
%Z, with my blessing."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         synopsis = "[\"Have you progressed with your quest to regain the Amulet of Yendor for %d?\"]",
         output = "text",
         text = [["Welcome back, %p.  Have you progressed with your quest to
regain the Amulet of Yendor for %d?"]],
      },
   },
   Bar = {
      assignquest = {
         synopsis = "[\"Find %n, defeat %ni, and return %o to us.\"]",
         output = "text",
         text = [["The world is in great need of your assistance, %p.

"About six months ago, I learned that a mysterious sorcerer, known
as %n, had begun to gather a large group of cutthroats and brigands
about %ni.

"At about the same time, these people you once rode with `liberated' a
potent magical talisman, %o, from a Turanian caravan.

"%nC and %nj Black Horde swept down upon %i and defeated
the people there, driving them out into the desert.  He has taken
%o, and seeks to bend it to %nj will.  I detected the
subtle changes in the currents of fate, and joined these people.
Then I sent forth a summons for you.

"If %n can bend %o to %nj will, he will become
almost indestructible.  He will then be able to enslave the minds of
men across the world.  You are the only hope.  The gods smile upon you,
and with %d behind you, you alone can defeat %n.

"You must go to %i.  From there, you can track down
%n, defeat %ni, and return %o to us.  Only
then will the world be safe."]],
      },
      badalign = {
         synopsis = "[\"You have wandered from the path of the %a.  Come back when you have atoned.\"]",
         output = "text",
         text = [["%pC!  You have wandered from the path of the %a!
If you attempt to overcome %n in this state, he will surely
enslave your soul.  Your only hope, and ours, lies in your purification.
Go forth, and return when you feel ready."]],
      },
      badlevel = {
         synopsis = "[\"You are too inexperienced.  Come back when you are %Ra.\"]",
         output = "text",
         text = [["%p, I fear that you are as yet too inexperienced to face
%n.  Only %Ra with the help of %d could ever hope to
defeat %ni."]],
      },
      discourage = {
         "\"My pets will dine on your carcass tonight!\"",
         "\"You are a sorry excuse for %ra.\"",
         "\"Run while you can, %c.  My next spell will be your last.\"",
         "\"I shall use your very skin to bind my next grimoire.\"",
         "\"%d cannot protect you now.  Here, you die.\"",
         "\"Your %a nature makes you weak.  You cannot defeat me.\"",
         "\"Come, %c.  I shall kill you, then unleash the horde on your tribe.\"",
         "\"Once you are dead, my horde shall finish off %l, and your tribe.\"",
         "\"Fight, %c, or are you afraid of the mighty %n?\"",
         "\"You have failed, %c.  Now, my victory is complete.\"",
      },
      encourage = {
         "\"%nC is strong in the dark arts, but not immune to cold steel.\"",
         "\"Remember that %n is a great sorcerer.  He lived in the time of Atlantis.\"",
         "\"If you fail, %p, I will not be able to protect these people long.\"",
         "\"To enter %i, you must be very stealthy.  The horde will be on guard.\"",
         "\"Call upon %d in your time of need.\"",
         "\"May %d protect you, and guide your steps.\"",
         "\"If you can lay hands upon %o, carry it for good fortune.\"",
         "\"I cannot stand against %ns sorcery.  But %d will help you.\"",
         "\"Do not fear %n.  I know you can defeat %ni.\"",
         "\"You have a great road to travel, %p, but only after you defeat %n.\"",
      },
      firsttime = {
         synopsis = "[You reach the vicinity of %H, but sense evil magic nearby.]",
         output = "text",
         text = [[Warily you scan your surroundings, all of your senses alert for signs
of possible danger.  Off in the distance, you can %x the familiar shapes
of %H.

But why, you think, should %l be there?

Suddenly, the hairs on your neck stand on end as you detect the aura of
evil magic in the air.

Without thought, you ready your weapon, and mutter under your breath:

    "By %d, there will be blood spilt today."]],
      },
      goal_first = {
         synopsis = "[This is surely the lair of %n.]",
         output = "text",
         text = [[
Ahead, you can make out the glow of fire and the dark shapes of
buildings.  The hairs on the nape of your neck lift as you sense an
energy in the very air around you.  You fight down a primordial panic
that seeks to make you turn and run.  This is surely the lair of %n.
         ]],
      },
      goal_next = {
         text = "Yet again you feel the air around you heavy with malevolent magical energy.",
      },
      gotit = {
         synopsis = "[You feel the power of %o flowing through your hands.]",
         output = "text",
         text = [[As you pick up %o, you feel the power of it
flowing through your hands.  It seems to be in two or more places
at once, even though you are holding it.]],
      },
      guardtalk_after = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"Times will be good again, now that the horde is vanquished.\"",
         "\"You have brought our clan much honor in defeating %n.\"",
         "\"You will be a worthy successor to %l.\"",
      },
      guardtalk_before = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"There has been little treasure to loot, since the horde arrived.\"",
         "\"The horde is mighty in numbers, but they have little courage.\"",
         "\"%lC is a strange one, but he has helped defend us.\"",
      },
      hasamulet = {
         synopsis = "[\"Take the Amulet to the altar of %d on the Astral Plane and offer it.\"]",
         output = "text",
         text = [["This is wondrous, %p.  I feared that you could not possibly
succeed in your quest, but here you are in possession of the Amulet
of Yendor!

"I have studied the texts of the magi constantly since you left.  In
the Book of Skelos, I found this:

    %d will cause a child to be sent into the world.  This child is to
    be made strong by trial of battle and magic, for %d has willed it so.
    It is said that the child of %d will recover the Amulet of Yendor
    that was stolen from the Creator at the beginning of time.

"As you now possess the amulet, %p, I suspect that the Book
speaks of you.

    The child of %d will take the Amulet, and travel to the Astral
    Plane, where the Great Temple of %d is to be found.  The Amulet
    will be sacrificed to %d, there on %dJ altar.  Then the child will
    stand by %d as champion of all %cP for eternity.

"This is all I know, %p.  I hope it will help you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you, but you feel the overpowering aura of magic fading.]",
         output = "text",
         text = [[%nC falls to the ground, and utters a last curse at you.  Then %nj
body fades slowly, seemingly dispersing into the air around you.  You
slowly become aware that the overpowering aura of magic in the air has
begun to fade.]],
      },
      leader_first = {
         synopsis = "[\"At last you have returned.  There is a great quest you must undertake.\"]",
         output = "text",
         text = [["Ah, %p.  You have returned at last.  The world is in dire
need of your help.  There is a great quest you must undertake.

"But first, I must see if you are ready to take on such a challenge."]],
      },
      leader_last = {
         synopsis = "[\"You have betrayed %d; soon %n will destroy us.  Begone!\"]",
         output = "text",
         text = [["Pah!  You have betrayed the gods, %p.  You will never attain
the glory which you aspire to.  Your failure to follow the true path has
closed this future to you.

"I will protect these people as best I can, but soon %n will overcome
me and destroy all who once called you %s.  Now begone!"]],
      },
      leader_next = {
         text = "\"%p, you are back.  Are you ready now for the challenge?\"",
      },
      leader_other = {
         text = "\"Again, you stand before me, %p.  Surely you have prepared yourself.\"",
      },
      locate_first = {
         synopsis = "[You have located %i.]",
         output = "text",
         text = [[
As the sun sinks below the horizon, the scent of water comes to you in
the desert breeze.  You know that you have located %i.
         ]],
      },
      locate_next = {
         text = "Yet again you have a chance to infiltrate %i.",
      },
      nemesis_first = {
         synopsis = "[%nC boasts that %nh has slain many.  \"Prepare to die, %c.\"]",
         output = "text",
         text = [["So.  This is what that second rate sorcerer %l sends to do %lj bidding.
I have slain many before you.  You shall give me little sport.

"Prepare to die, %c."]],
      },
      nemesis_next = {
         text = "\"I have wasted too much time on you already.  Now, you shall die.\"",
      },
      nemesis_other = {
         text = "\"You return yet again, %c!  Are you prepared for death now?\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o back, you pitiful excuse for %ca.
And your life as well."]],
      },
      nexttime = {
         text = [[Once again, you near %H.  You know that %l
will be waiting.]],
      },
      offeredit = {
         synopsis = "[%lC tells you to guard %o, and to return when you have triumphed.]",
         output = "text",
         text = [[When %l sees %o, he smiles, and says:

    Well done, %p.  You have saved the world from certain doom.
    What, now, should be done with %o?

    These people, brave as they are, cannot hope to guard it from
    other sorcerers who will detect it, as surely as %n did.

    Take %o with you, %p.  It will guard you in
    your adventures, and you can best guard it.  You embark on a
    quest far greater than you realize.

    Remember me, %p, and return when you have triumphed.  I
    will tell you then of what you must do.  You will understand when the
    time comes.]],
      },
      offeredit2 = {
         synopsis = "[\"You keep %o.  Return to %Z to search for the Amulet.\"]",
         output = "text",
         text = [[%l gazes reverently at %o, then back at you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  %Z await your return through the
magic portal which brought you here."]],
      },
      othertime = {
         text = [[Again, and you think possibly for the last time, you approach
%H.]],
      },
      posthanks = {
         text = "\"Tell us, %p, have you fared well on your great quest?\"",
      },
   },
   Cav = {
      assignquest = {
         synopsis = "[Find and defeat %n, recover %o, and return with it.]",
         output = "text",
         text = [["You are indeed ready now, %p.  I shall tell you a tale of
great suffering among your people:

"Shortly after you left on your vision quest, the caves were invaded by
the creatures sent against us by %n.

"She, herself, could not attack us due to her great size, but her minions
have harassed us ever since.  In the first attacks, many died, and the
minions of %n managed to steal %o.
They took it to %i and there, none of our
%g warriors have been able to go.

"You must find %i, and within it wrest
%o from %n.  She guards it as
jealously as she guards all treasures she attains.  But with it,
we can make our caves safe once more.

"Please, %p, recover %o for us, and return it here."]],
      },
      badalign = {
         synopsis = "[\"You no longer follow the path of the %a.  Go, and purify yourself.\"]",
         output = "text",
         text = [["%pC!  You have deviated from my teachings.  You no longer follow
the path of the %a as you should.  I banish you from these caves, to
go forth and purify yourself.  Then, you might be able to accomplish this
quest."]],
      },
      badlevel = {
         synopsis = "[\"%rA is too inexperienced.  Come back when you have progressed.\"]",
         output = "text",
         text = [["Alas, %p, you are as yet too inexperienced to embark upon such
a difficult quest as that I propose to give you.

"%rA could not possibly survive the rigors demanded to find
%i, never mind to confront %n herself.

"Adventure some more, and you will learn the skills you will require.
%d decrees it."]],
      },
      discourage = {
         "\"You are weak, %c.  No challenge for the Mother of all Dragons.\"",
         "\"I grow hungry, %r.  You look like a nice appetizer!\"",
         "\"Join me for lunch?  You're the main course, %c.\"",
         "\"With %o, I am invincible!  You cannot succeed.\"",
         "\"Your mentor, %l has failed.  You are nothing to fear.\"",
         "\"You shall die here, %c.  %rA cannot hope to defeat me.\"",
         "\"You, a mere %r challenge the might of %n?  Hah!\"",
         "\"I am the Mother of all Dragons!  You cannot hope to defeat me.\"",
         "\"My claws are sharp now.  I shall rip you to shreds!\"",
         "\"%d has deserted you, %c.  This is my domain.\"",
      },
      encourage = {
         "\"%nC is immune to her own breath weapons.  You should use magic upon her that she does not use herself.\"",
         "\"When you encounter %n, call upon %d for assistance.\"",
         "\"There will be nowhere to hide inside %ns inner sanctum.\"",
         "\"Your best chance with %n will be to keep moving.\"",
         "\"Do not be distracted by the great treasures in %ns lair.  Concentrate on %o.\"",
         "\"%oC is the only object that %n truly fears.\"",
         "\"Do not be fooled by %ns size.  She is fast, and it is rumored that she uses magic.\"",
         "\"I would send a party of %gP with you, but we will need all of our strength to defend ourselves.\"",
         "\"Remember, be %a at all times.  This is your strength.\"",
         "\"If only we had an amulet of reflection, this would not have happened.\"",
      },
      firsttime = {
         synopsis = "[You arrive back at %H, but something is wrong here.]",
         output = "text",
         text = [[You descend through a barely familiar stairwell that you remember
%l showing you when you embarked upon your vision quest.

You arrive back at %H, but something seems
wrong here.  The usual smoke and glowing light of the fires of the
outer caves are absent, and an uneasy quiet fills the damp air.]],
      },
      goal_first = {
         synopsis = "[You enter a large cavern.  %nC is present.]",
         output = "text",
         text = [[You find yourself in a large cavern, with neatly polished walls, that
nevertheless show signs of being scorched by fire.

Bones litter the floor, and there are objects scattered everywhere.
The air is close with the stench of sulphurous fumes.

%nC is clearly visible, but %nh seems to be asleep.]],
      },
      goal_next = {
         text = "Once again, you find yourself in the lair of %n.",
      },
      gotit = {
         synopsis = "[%oC fills you with a feeling of power.]",
         output = "text",
         text = [[As you pick up %o it seems heavy at first, but as you
hold it strength flows into your arms.

You suddenly feel full of power, as if nothing could possibly stand
in your path.]],
      },
      guardtalk_after = {
         "\"The rains have returned and the land grows lush again.\"",
         "\"Peace has returned, give thanks to %d!\"",
         "\"Welcome back!  Did you find %o?\"",
         "\"So, %p, tell us the story of your fight with %n.\"",
         "\"%lC grows old.  Perhaps you will guide us after he ascends.\"",
      },
      guardtalk_before = {
         "\"We have not been able to gather as much food since the Giants sealed off our access to the outer world.\"",
         "\"Since %n sent her minions, we have been constantly fighting.\"",
         "\"I have heard your vision quest was successful.  Is this so?\"",
         "\"So, tell me, %p, how have you fared?\"",
         "\"%lC grows old.  We know not who will guide us after he ascends.\"",
      },
      hasamulet = {
         synopsis = "[\"Take the Amulet to the altar of %d on the Astral Plane and offer it.\"]",
         output = "text",
         text = [["You have been successful, I see, %p.

"Now that the Amulet of Yendor is yours, here is what you must do:

"Journey upwards to the open air.  The Amulet you carry will then
take you into the Astral Planes, where the Great Temple of %d
casts its influence throughout our world.

"Sacrifice the Amulet on the altar.  Thus shall %d become supreme!"]],
      },
      killed_nemesis = {
         text = [[%nC sinks to the ground, her heads flailing about.
As she dies, a cloud of noxious fumes billows about her.]],
      },
      leader_first = {
         synopsis = "[\"You have returned.  We are in dire need of your help.\"]",
         output = "text",
         text = [["You have returned from your vision quest, %p.  Thank %d.

"We are in dire need of your help, my %S.

"But first, I must see if you are yet capable of the quest I would
ask you to undertake."]],
      },
      leader_last = {
         synopsis = "[\"You have betrayed the %L.  Begone!\"]",
         output = "text",
         text = [["%pC!  You have sealed our fate.  You seem unable to reform yourself,
so I must select another to take your place.

"Begone from %H!  You have betrayed us by choosing
the path of the %C over the true path of the %L.

"You no longer live in our eyes."]],
      },
      leader_next = {
         text = "\"Again, you return to us, %p.  Let me see if you are ready now.\"",
      },
      leader_other = {
         text = "\"Ah, %p.  Are you finally ready?\"",
      },
      locate_first = {
         synopsis = "[You %x many large claw marks, smell carrion, and notice bones.]",
         output = "text",
         text = [[You %x many large claw marks on the ground.  The tunnels ahead
of you are larger than most of those in any cave complex you have
ever been in before.

Your nose detects the smell of carrion from within, and bones litter
the sides of the tunnels.]],
      },
      locate_next = {
         text = "Once again, you approach %i.",
      },
      nemesis_first = {
         synopsis = "[%nC threatens to eat you.]",
         output = "text",
         text = [["So, follower of %l, you seek to invade the lair of
%n.  Only my meals are allowed down here.  Prepare
to be eaten!"]],
      },
      nemesis_next = {
         text = [["So, again you face me, %c.  No one has ever before escaped me.
Now I shall kill you."]],
      },
      nemesis_other = {
         text = "\"You are getting annoying, %c.  Prepare to die.\"",
      },
      nemesis_wantsit = {
         text = "\"I'll have %o from you, %c.  You shall die.\"",
      },
      nexttime = {
         text = "Once again, you arrive back at %H.",
      },
      offeredit = {
         synopsis = "[\"Take %o with you.  It will help in your quest for the Amulet of Yendor.\"]",
         output = "text",
         text = [[%lC glimpses %o in your possession.
He smiles and says:

    You have done it!  We are saved.  But I fear that %o
    will always be a target for %C forces who will want it for their
    own.

    To prevent further trouble, I would like you, %p,
    to take %o away with you.  It will help you as you
    quest for the Amulet of Yendor.]],
      },
      offeredit2 = {
         synopsis = "[\"You are the keeper of %o now.  Return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%l grasps %o proudly for a moment, then looks at you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  %Z await your return through the
magic portal which brought you here."]],
      },
      othertime = {
         text = [[For some reason, you think that this may be the last time you will
enter %H.]],
      },
      posthanks = {
         text = [["%pC!  Welcome back.
How goes your quest to recover the Amulet for %d?"]],
      },
   },
   Con = {
      assignquest = {
         output = "text",
         text = [["Yes, %p, it looks like you are now ready to
help us.  Things have become much worse since you
left.  The dungeons are overrun by vermin, guards
have become brutal and corrupt, and I believe that
%n has had dealings with dark powers.

"I had spent many years enchanting my iron ball
with powers to help lead an escape from this place,
turning it into %o.
Unfortunately, %n stole it from me,
likely using it for dark, twisted purposes.

"We need your help to go down to the warden's level
to defeat the corrupt %n and recover
%o for us so that
%H will be safe again."]],
      },
      badalign = {
         output = "text",
         text = [["%p, I see that you are not yet a loyal servant of
%d.  Leave this place for now, and only
return when you have cleansed your spirit.  Only then
will you be ready to stand up to %n and
recover %o."]],
      },
      badlevel = {
         output = "text",
         text = [["%p, I am afraid that a %r will be no
match for %n.  If I sent you now,
she would dispatch you all too easily.

"Continue your other quest, and practice your
skills.  Return to us once you have achieved the rank
of %R."]],
      },
      discourage = {
         "\"You should never come back.  You will not escape here alive again!\"",
         "\"Even all of your pathetic lot put together would not be enough to defeat me.\"",
         "\"You are no more than lowly prison scum.  You are unworthy to even challenge me.\"",
         "\"Even %l could not stand up to me, so you have no chance at all.\"",
         "\"Even %o would not be enough to make you my equal.\"",
         "\"Your pathetic devotion to %d will not save you, either.\"",
         "\"After I kill you, I will go after %l next, and any that dare to defy me again!\"",
         "\"Die with dishonor, lowly %c!\"",
         "\"Once a %c, always a %c.\"",
         "\"How about you find yourself a cosy little cell, and hope that I forgive your insolence?\"",
      },
      encourage = {
         "\"The sooner you are able to recover %o, the better off we will be.\"",
         "\"I am not sure what has happened, but even without %o, %n will make a formidable opponent.\"",
         "\"Stay true to the teachings of %d, and good fortune will be with you.\"",
         "\"I am afraid without the power of %o to protect us, we will soon be overwhelmed.\"",
         "\"%n used to be a woman of honor before turning to dark ways.  It is much too late for redemption, however.\"",
         "\"%n is arrogant, and will try to diminish your confidence.  Do not let yourself be discouraged!\"",
         "\"You will have to be very alert, and will need to use all of your cunning if you hope to defeat %n.\"",
         "\"Call upon the power of %d to protect you when you encounter %n.\"",
         "\"If you remain true to your faith, you should be able to sense the power of %o when you are near.\"",
         "\"You should be able to defeat %n easily enough in a fair fight.  Unfortunately, she doesn't fight fair.\"",
      },
      firsttime = {
         output = "text",
         text = [[Somehow, you have been captured and teleported back to
%H!  But wait, the portal you went through is still
here.  You had heard your old mentor %l,
the prison chaplain who taught you the ways of
%d calling for your help.  Also, %H
is not as you had left it.  There seems to be damage
to the walls, and you hear sounds of strife all
around you.]],
      },
      goal_first = {
         output = "text",
         text = [[You sense the presence of %o as soon as you
enter this level.  If %o is here, then
%n must not be far away.]],
      },
      goal_next = {
         text = [[Once again, you find yourself in the abode of
%n, and feel the presence of
%o.]],
      },
      gotit = {
         output = "text",
         text = [[As you pick up %o, you feel its power
flow through you, protecting you, and making you
more aware of your surroundings.  You know that
you need to get it back to %l as soon
as possible now.]],
      },
      guardtalk_after = {
         "\"Thank you for ridding us of %n.  I hope the new warden will be better.\"",
         "\"Take me with you!  I will be glad to be rid of this awful place.\"",
         "\"Don't forget to come back for us once you have completed your quest!\"",
         "\"Thank you for coming back to help us in our hour of need!\"",
         "\"Leave while you still can!  If you can escape, maybe there is hope for the rest of us.\"",
      },
      guardtalk_before = {
         "\"How did you manage to escape?\"",
         "\"How is life on the outside?\"",
         "\"Between the abusive guards, and the nasty vermin, I don't know how we will hold out.\"",
         "\"This place used to be so much nicer before %n became corrupt.\"",
         "\"You are really back?  I thought you were gone for good.\"",
      },
      hasamulet = {
         output = "text",
         text = [["Congratulations, %p!  You have redeemed yourself
and proven yourself a worthy servant of %d,
and in doing so have made all of us proud.

"One final task remains for you now.  You must take
the Amulet up to the Great Temple of %d, on
the Astral plane.  There you must offer the Amulet
to %d."]],
      },
      killed_nemesis = {
         output = "text",
         text = [[As %n approaches death, you see sudden
clarity in her eyes.

"What have I done?  Those demons have been clouding my mind.
Please forgive me, %p, although I know that I
probably do not deserve it.  Take 
%o back to %l
with my blessing."

With a final coughing fit, %n spasms, and her
eyes see no more.]],
      },
      leader_first = {
         output = "text",
         text = [["%p, I am very happy to see that you are
alive and well!  We were all proud of your daring
escape, and hoped that you were doing well.
Things have become much worse in your absence,
however, and we desperately need your help.  Let
me have a look at you, and see if you are ready."]],
      },
      leader_last = {
         output = "text",
         text = [["You have betrayed us all, %p!  Begone from
here, and don't come back!  You are not even worthy
to share a cell with the least of us now."]],
      },
      leader_next = {
         text = [["I see that you are back, %p.  I hope that
you are now ready to help us?  Let me see if you
are."]],
      },
      leader_other = {
         text = [["Once again, you have returned.  Let me see if you
are finally ready to help us."]],
      },
      locate_first = {
         output = "text",
         text = [[This must be the level that leads to the warden's
area.  You must be getting closer to
%o now!]],
      },
      locate_next = {
         text = [[Once again, you find yourself near the entrance to
the warden's area.  Hopefully, you can acquire
%o before it's too late!]],
      },
      nemesis_first = {
         output = "text",
         text = [["So, %p.  You have returned, and are here to get
%o for %l?
What makes you think you can get it if
%l couldn't?  I have many
allies that have made me even stronger."]],
      },
      nemesis_next = {
         text = [["I see that you have returned, %p.  I
suggest you leave and save yourself while you still
have a chance."]],
      },
      nemesis_other = {
         text = [["Back again, %p?  You will certainly die in
your attempt to defeat me."]],
      },
      nemesis_wantsit = {
         text = [["I will get %o from you, just as I did
%l!  You can not defeat me."]],
      },
      nexttime = {
         text = [[You have returned to %H.  Somehow, things are even
worse now than when you were on your previous visit.  Has
%l managed to still hold out?]],
      },
      offeredit = {
         output = "text",
         text = [["Congratulations, %p!  You have returned with
your life, and with %o!  I had
planned to use %o to lead an
escape from here, but with %n defeated,
life should go back to normal before long.

"Whether I like it or not, I know my place is here.  I
see that %o has already attuned
itself to you.  I offer it to you in hopes that
it will aid in your quest to recover the Amulet
of Yendor."]],
      },
      offeredit2 = {
         output = "text",
         text = [[%l looks upon %o
with fondness.
"You are its keeper now.  Take it with you back to
%Z, through the magic portal
that brought you here."]],
      },
      othertime = {
         text = [[The situation seems to degrade, and you wonder how long
your fellow %gp can hold out against the corrupt
guards and creatures of the Underdark that pervade the
formerly tranquil %H.]],
      },
      posthanks = {
         output = "text",
         text = [["Welcome back, %p.  We have done well in your
absence.  How far have you come with your quest to
regain the Amulet of Yendor for %d?"]],
      },
   },
   Hea = {
      assignquest = {
         synopsis = "[Travel to %i on your way to recover %o from %n.]",
         output = "text",
         text = [[For the first time, you sense a smile on %ls face.

    "You have indeed learned as much as we can teach you in preparation
    for this task.  Let me tell you what I know of the symptoms and hope
    that you can provide a cure.

    "A short while ago, the dreaded %nt was fooled by the gods
    into thinking that %nh could use %o to find a
    cure for old age.  Think of it, eternal youth!  But %nj good
    health is accomplished by drawing the health from those around %ni.

    "He has exhausted %nj own supply of healthy people and now %nh seeks to
    extend %nj influence into our world.  You must recover from %ni
    %o and break the spell.

    "You must travel into the swamps to %i, and from there
    follow the trail to %ns island lair.  Be careful."]],
      },
      badalign = {
         synopsis = "[Return when you are more %a.]",
         output = "text",
         text = [["You have learned much of the remedies that benefit, but you must also
know which physic for which ail.  That is why %ds teachings are a
part of your training.

"Return to us when you have healed thyself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you are yet too inexperienced to deal with the rigors
of such a task.  You must be able to draw on the knowledge of botany,
alchemy and veterinary practices before I can send you on this quest 
with good conscience.

"Return when you wear %Ra's caduceus."]],
      },
      discourage = {
         "\"They might as well give scalpels to wizards as to let you try to use %o!\"",
         "\"If I could strike %l, surrounded by %lj %gP, imagine what I can do to you here by yourself.\"",
         "\"I will put my %Rp to work making a physic out of your ashes.\"",
         "\"As we speak, Hades gathers your patients to join you.\"",
         "\"After I'm done with you, I'll destroy %l as well.\"",
         "\"You will have to kill me if you ever hope to leave this place.\"",
         "\"I will impale your head on my caduceus for all to see.\"",
         "\"There is no materia medica in your sack which will cure you of me!\"",
         "\"Do not fight too hard, I want your soul strong, not weakened!\"",
         "\"You should have stopped studying at veterinary.\"",
      },
      encourage = {
         "\"Remember, %p, to always wash your hands before operating.\"",
         "\"%nC has no real magic of %nj own.  To this %nh is vulnerable.\"",
         "\"If you have been true to %d, you can draw on the power of %o.\"",
         "\"Bring with you antidotes for poisons.\"",
         "\"Remember this, %n can twist the powers of %o to hurt instead of heal.\"",
         "\"I have sent for Chiron, but I am afraid he will come too late.\"",
         "\"Maybe when you return the snakes will once again begin to shed.\"",
         "\"The plague grows worse as we speak.  Hurry, %p!\"",
         "\"Many times %n has caused trouble in these lands.  It is time that %nh was eradicated like the diseases %nh has caused.\"",
         "\"With but one eye, %n should be easy to blind.  Remember this.\"",
      },
      firsttime = {
         synopsis = "[You arrive back at %H and must find %l.]",
         output = "text",
         text = [[What sorcery has brought you back to %H?  The smell
of fresh funeral pyres tells you that something is amiss with the healing
powers that used to practice here.

No rhizotomists are tending the materia medica gardens, and where are the
common folk who used to come for the cures?

You know that you must quickly make your way to the collegium, and
%ls iatreion, and find out what has happened in your absence.]],
      },
      goal_first = {
         synopsis = "[You have reached the lair of %n.  Take %o away from %ni.]",
         output = "text",
         text = [[You stand within sight of the infamous Isle of %n.  Even
the words of %l had not prepared you for this.

Steeling yourself against the wails of the ill that pierce your ears,
you hurry on your task.  Maybe with %o you can
heal them on your return, but not now.]],
      },
      goal_next = {
         text = "Once again, you %x the Isle of %n in the distance.",
      },
      gotit = {
         synopsis = "[You feel the healing power of %o and should return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel its healing begin to
warm your soul.  You curse Zeus for taking it from its rightful owner,
but at least you hope that %l can put it to good use once
again.]],
      },
      guardtalk_after = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"How are you feeling?  Perhaps a good bleeding will improve your spirits.\"",
         "\"Have you heard the absurd new theory that diseases are caused by microscopic organisms, and not ill humors?\"",
         "\"I see that you bring %o, now you can cure this plague!\"",
      },
      guardtalk_before = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"I passed handwriting so they are demoting me a rank.\"",
         "\"I've heard that even %l has not been able to cure Chiron.\"",
         "\"We think %n has used %nj alchemists, and %o, to unleash a new disease we call 'the cold' on Gehennom.\"",
      },
      hasamulet = {
         synopsis = "[\"You have recovered the Amulet.  Travel to the Astral Plane and return it to %d.\"]",
         output = "text",
         text = [["Ah, you have recovered the Amulet, %p.  Well done!

"Now, you should know that you must travel through the Elemental Planes
to the Astral, and there return the Amulet to %d.  Go forth and
may our prayers be as a wind upon your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[The battered body of %n slumps to the ground and gasps
out one last curse:

    "You have defeated me, %p, but I shall have my revenge.
    How, I shall not say, but this curse shall be like a cancer
    on you."

With that %n dies.]],
      },
      leader_first = {
         synopsis = "[%l is weak from the struggle with %n.  %lH wants to examine you.]",
         output = "text",
         text = [[Feebly, %l raises %lj head to look at you.

"It is good to see you again, %p.  I see the concern in your
eyes, but do not worry for me.  I am not ready for Hades yet.  We have
exhausted much of our healing powers holding off %n.
I need your fresh strength to carry on our work.

"Come closer and let me lay hands on you, and determine if you have
the skills necessary to accomplish this mission."]],
      },
      leader_last = {
         synopsis = "[You are a failure as a healer.]",
         output = "text",
         text = [["You have failed us, %p.  You are a quack!  A charlatan!

"Hades will be happy to hear that you are once again practicing your
arts on the unsuspecting."]],
      },
      leader_next = {
         text = [["Again you return to me, %p.  I sense that each trip back
the pleurisy and maladies of our land begin to infect you.  Let us
hope and pray to %d that you become ready for your task before
you fall victim to the bad humors."]],
      },
      leader_other = {
         text = [["Chiron has fallen, Hermes has fallen, what else must I tell you to
impress upon you the importance of your mission!  I hope that you
have come prepared this time."]],
      },
      locate_first = {
         synopsis = "[You have reached %i but all is not well.]",
         output = "text",
         text = [[You stand before the entrance to %i.  Strange
scratching noises come from within the building.

The swampy ground around you seems to stink with disease.]],
      },
      locate_next = {
         text = "Once again you stand at the entrance to %i.",
      },
      nemesis_first = {
         synopsis = "[\"I will take your life, then defeat %l.\"]",
         output = "text",
         text = [["They have made a mistake in sending you, %p.

"When I add your youth to mine, it will just make it easier for me
to defeat %l."]],
      },
      nemesis_next = {
         text = "\"Unlike your patients, you seem to keep coming back, %p!\"",
      },
      nemesis_other = {
         text = "\"Which would you like, %p?  Boils, pleurisy, convulsions?\"",
      },
      nemesis_wantsit = {
         text = [["I'll have %o back from you, %r.  You are
not going to live to escape this place."]],
      },
      nexttime = {
         text = [[After your last experience you expected to be here, but you certainly
did not expect to see things so much worse.  This time you must succeed.]],
      },
      offeredit = {
         synopsis = "[%l touches %o and tells %lj %gP to do so too, then tells you to take it with you.]",
         output = "text",
         text = [[As soon as %l sees %o %lh summons %lj
%gP.

Gently, %l reaches out and touches %o.
He instructs each of the assembled to do the same.  When everyone
has finished %lh speaks to you.

    "Now that we have been replenished we can defeat this plague.  You must
    take %o with you and replenish the worlds you have
    been called upon to travel next.  I wish you could ride Chiron to the
    end of your journey, but I need him to help me spread the cure.  Go
    now and continue your journey."]],
      },
      offeredit2 = {
         synopsis = "[%l tells you to keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%l cautiously handles %o while watching you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  %Z await your return through the
magic portal which brought you here."]],
      },
      othertime = {
         text = [[Again, you %x %H in the distance.

The smell of death and disease permeates the air.  You do not have
to be %Ra to know that %n is on the verge of victory.]],
      },
      posthanks = {
         text = [["You have again returned to us, %p.  We have done well in your
absence, yes?  How fare you upon your quest for the Amulet?"]],
      },
   },
   Kni = {
      assignquest = {
         synopsis = "[Pass through %i to reach %n.  Destroy %ni and return with %o.]",
         output = "text",
         text = [["Ah, %p.  Thou art truly ready, as no %c before thee hath
been.  Hear now Our words:

"As thou noticed as thou approached %H, a great battle hath
been fought recently in these fields.  Know thou that Merlin himself
came to aid Us here as We battled the foul %n.  In the midst of that
battle, %n struck Merlin a great blow, felling him.  Then, as Our
forces were pressed back, %n stole %o.

"We eventually turned the tide, but lost many %cP in doing so.
Merlin was taken off by his apprentice, but hath not recovered.  We have
been told that so long as %n possesseth %o,
Merlin will not regain his health.

"We hereby charge thee with this most important of duties:

"Go forth from this place, to the fens, and there thou wilt find
%i.  From there, thou must track down %n.  Destroy the
beast, and return to Us %o.  Only then can
We restore Merlin to health."]],
      },
      badalign = {
         synopsis = "[Go and do penance.  Return when you are truly %a.]",
         output = "text",
         text = [["Thou dishonourest Us, %p!  Thou hast strayed from the path of
chivalry! Go from Our presence and do penance.  Only when thou art again
pure mayst thou return hence."]],
      },
      badlevel = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [["Verily, %p, thou hast done well.  That thou hast survived thus
far is a credit to thy valor, but thou art yet unprepared for
the demands required as Our Champion.  %rA, no matter how
pure, could never hope to defeat the foul %n.

"Journey forth from this place, and hone thy skills.  Return to
Our presence when thou hast attained the noble title of %R."]],
      },
      discourage = {
         "\"A mere %r can never withstand me!\"",
         "\"I shall kill thee now, and feast!\"",
         "\"Puny %c.  What manner of death dost thou wish?\"",
         "\"First thee, %p, then I shall feast upon %l.\"",
         "\"Hah!  Thou hast failed, %r.  Now thou shalt die.\"",
         "\"Die, %c.  Thou art as nothing against my might.\"",
         "\"I shall suck the marrow from thy bones, %c.\"",
         "\"Let's see...  Baked?  No.  Fried?  Nay.  Broiled?  Yea verily, that is the way I like my %c for dinner.\"",
         "\"Thy strength waneth, %p.  The time of thy death draweth near.\"",
         "\"Call upon thy precious %d, %p.  It shall not avail thee.\"",
      },
      encourage = {
         "\"Remember, %p, follow always the path of %d.\"",
         "\"Though %n is verily a mighty foe, We have confidence in thy victory.\"",
         "\"Beware, for %n hath surrounded %niself with hordes of foul creatures.\"",
         "\"Great treasure, 'tis said, is hoarded in the lair of %n.\"",
         "\"If thou possessest %o, %p, %ns magic shall therewith be thwarted.\"",
         "\"The gates of %i are guarded by forces unseen, %p.  Go carefully.\"",
         "\"Return %o to Us quickly, %p.\"",
         "\"Destroy %n, %p, else %H shall surely fall.\"",
         "\"Call upon %d when thou art in need.\"",
         "\"To find %i, thou must keep thy heart pure.\"",
      },
      firsttime = {
         synopsis = "[Signs of battle include long gouges in the walls of %H.]",
         output = "text",
         text = [[You materialize in the shadows of %H.  Immediately, you notice
that something is wrong.  The fields around the castle are trampled and
withered, as if some great battle has been recently fought.

Exploring further, you %x long gouges in the walls of %H.
You know of only one creature that makes those kinds of marks...]],
      },
      goal_first = {
         synopsis = "[You %x the entrance to a cavern inside a hill.]",
         output = "text",
         text = [[As you exit the swamps, you %x before you a huge, gaping hole in the
side of a hill.  From within, you smell the foul stench of carrion.

The pools on either side of the entrance are fouled with blood, and
pieces of rusted metal and broken weapons show above the surface.]],
      },
      goal_next = {
         text = "Again, you stand at the entrance to %ns lair.",
      },
      gotit = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you pick up %o, you feel its protective fields
form around your body.  You also feel a faint stirring in your mind, as
if you are in two places at once, and in the second, you are waking from
a long sleep.]],
      },
      guardtalk_after = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"So, %p, didst thou find %n in the fens near %i?\"",
         "\"Worthy %p, hast thou proven thy right purpose on the body of %n?\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Hast thou indeed recovered %o?\"",
      },
      guardtalk_before = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"There is word, %p, that %n hath been sighted in the fens near %i.\"",
         "\"Thou art our only hope now, %p.\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Many brave %cP died when %n attacked.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [["Thou hast succeeded, We see, %p!  Now thou art commanded to take
the Amulet to be sacrificed to %d in the Plane of the Astral.

"Merlin hath counseled Us that thou must travel always upwards through
the Planes of the Elements, to achieve this goal.

"Go with %d, %p."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[As %n sinks to the ground, blood gushing from %nj open mouth, %nh
defiantly curses you and %l:

    "Thou hast not won yet, %r.  By the gods, I shall return
    and dog thy steps to the grave!"

%nJ tail flailing madly, %n tries to crawl towards you, but slumps
to the ground and dies in a pool of %nj own blood.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are ready for a great undertaking.]",
         output = "text",
         text = [["Ah, %p.  We see thou hast received Our summons.
We are in dire need of thy prowess.  But first, We must needs
decide if thou art ready for this great undertaking."]],
      },
      leader_last = {
         synopsis = "[You are a disgrace as %ca.]",
         output = "text",
         text = [["Thou disgracest this noble court with thine impure presence.  We have been
lenient with thee, but no more.  Thy name shall be spoken no more.  We
hereby strip thee of thy title, thy lands, and thy standing as %ca.
Begone from Our sight!"]],
      },
      leader_next = {
         text = "\"Welcome again, %p.  We hope thou art ready now.\"",
      },
      leader_other = {
         text = "\"Once again, thou standest before Us, %p.  Art thou ready now?\"",
      },
      locate_first = {
         synopsis = "[You have reached %i and can %x a shrine.]",
         output = "text",
         text = [[You stand at the foot of %i.  Atop, you can %x a shrine.
Strange energies seem to be focused here, and the hair on the back of
your neck stands on end.]],
      },
      locate_next = {
         text = "Again, you stand at the foot of %i.",
      },
      nemesis_first = {
         synopsis = "[%nC taunts you and issues a threat against %H.]",
         output = "text",
         text = [["Hah!  Another puny %c seeks death.  I shall dine well tonight,
then tomorrow, %H shall fall!"]],
      },
      nemesis_next = {
         text = "\"Again, thou challengest me, %r?  So be it.  Thou wilt die here.\"",
      },
      nemesis_other = {
         text = "\"Thou art truly foolish, %r.  I shall dispatch thee anon.\"",
      },
      nemesis_wantsit = {
         text = [["So, thou darest touch MY property!  I shall have that bauble back,
puny %r.  Thou wilt die in agony!"]],
      },
      nexttime = {
         text = "Once again you stand in the shadows of %H.",
      },
      offeredit = {
         synopsis = "[%oC is yours now.  It will aid in your search for the Amulet.]",
         output = "text",
         text = [[As you approach %l, %lh beams at you and says:

    "Well done!  Thou art truly the Champion of %H.  We
    have received word that Merlin is recovering, and shall soon
    rejoin Us.

    "He hath instructed Us that thou art now to be the guardian of
    %o.  He feeleth that thou mayst have need of
    its powers in thine adventures.  It is Our wish that thou keepest
    %o with thee as thou searchest for the fabled
    Amulet of Yendor."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [["Careful, %p!  %oC might break, and that would
be a tragic loss.  Thou art its keeper now, and the time hath come
to resume thy search for the Amulet.  %Z await thy
return through the magic portal that brought thee here."]],
      },
      othertime = {
         text = [[Again, you stand before %H.  You vaguely sense that this
may be the last time you stand before %l.]],
      },
      posthanks = {
         text = "\"Well met, %p.  How goeth thy search for the Amulet of Yendor?\"",
      },
   },
   Mon = {
      assignquest = {
         synopsis = "[Find %i, then continue to %ns lair.  Defeat %ni and return with %o.]",
         output = "text",
         text = [[
"Yes, %p.  You are truly ready now.  Listen now to the story of our
woes:

"A few nights ago, %n led a surprise assault on
%H, along with dozens of elementals.
Our %gP fought bravely, but too many of them were slaughtered,
including the one bearing %o.
The %gP that remain are barely enough to defend this hall.

"Travel through the wilderness until you reach
%i, then, from there, descend into
%ns lair.
Defeat %n and bring back %o, and with them
we will be able to reestablish %H as a center of
spiritual guidance.

"Go bravely with %d, %p."
         ]],
      },
      badalign = {
         synopsis = "[You are losing your way.  Come back when your harmony is restored.]",
         output = "text",
         text = [[
"You seem troubled, %p.  It looks as though you are letting worldly
concerns begin to turn you away from enlightenment and the %a way.

There is still some hope for you, though.  Go from here and meditate on
your transgressions, and return once you have restored harmony with
yourself and with %d."
         ]],
      },
      badlevel = {
         synopsis = "[You are not ready to face %n.  Come back when you are %Ra.]",
         output = "text",
         text = [[
"Though your progress is adequate, no %r can stand up to %n.
You must continue your martial and mental training.  Return to us when
you have attained the post of %R."
         ]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"Though unenlightened, %n is an incredibly dangerous martial artist.\"",
         "\"In addition to his physical threats, %n is also a master of dark magic.\"",
         "\"Keep you mind clear, my %S.\"",
         "\"The path to %i is steep, and you will face many foes there.\"",
         "\"Remember your studies, and you will prevail!\"",
         "\"Acquire and wear %o if you can.  They will aid you against %n.\"",
         "\"It will not do for you to be unfocused.  Meditate on your mission if you must.\"",
         "\"We have some small amount of supplies here; take any that you may need.\"",
         "\"%n may try to trick and distract you.  Be steadfast.\"",
         "\"I must continue to hold this hall against the elementals while you are gone.\"",
      },
      firsttime = {
         synopsis = "[You have reached %H but something is wrong.  %lC needs your aid.]",
         output = "text",
         text = [[
You find yourself standing in sight of %H.

But what is this? The great gates are sealed shut, and ominous noises
echo from the grounds around you.

You must consult with %l at once!
         ]],
      },
      goal_first = {
         synopsis = "[You are surrounded by brimstone, lava, and elementals.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the elementals close in
from all sides!

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel the essence of %d and realize that you should take %o to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole %oi from
%H, for with %oi, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      guardtalk_after = {
         "\"Greetings, honorable %r.  It is good to see you again.\"",
         "\"%l teaches us not to focus on earthly beauty.\"",
         "\"With this test behind you, may %d bring you enlightenment.\"",
         "\"Good day, %p.  The repair efforts have been going well.\"",
         "\"Got any wands of enlightenment to spare?\"",
      },
      guardtalk_before = {
         "\"Greetings, honorable %r.  It is good to see you.\"",
         "\"Our beautiful monastery lies in shambles, %p.\"",
         "\"Welcome back, %p.  Will you join me in meditation?\"",
         "\"It will take long to repair the damage from the elementals.\"",
         "\"Got any potions of enlightenment to spare?\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [[
"You have prevailed, %p!  %d is surely with you.  Now,
you must bring the Amulet up out of the dungeon, and sacrifice it on
%ds altar on the Astral Plane, where you may reach true nirvana at last.
All of us here will continue to keep you in our thoughts."
         ]],
      },
      killed_nemesis = {
         synopsis = "[As %n dies, %nh threatens to return.]",
         output = "text",
         text = [[%nC gasps:

    "You have only defeated this mortal body.  Know this: my spirit
    is strong.  I shall return and reclaim what is mine!"

With that, %n expires.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are adequately prepared to help.]",
         output = "text",
         text = [[
"Ah, %p, my %S.
Hard times have fallen on our peaceful monastery, but you may be able to
help.  But first, let us see how your training has continued while you
have been away from us."
         ]],
      },
      leader_last = {
         synopsis = "[You are unenlightened and are expelled from the monastery.]",
         output = "text",
         text = [[
"You are a disgrace, %p!  I can see that your mind has turned away from
the path of enlightenment.  We will never defeat %n if you are our
champion.

You are no longer %sa here.  Return to the dungeon from whence you came,
and never again return."
         ]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      locate_first = {
         synopsis = "[You are near %i.  %nC lurks further ahead.]",
         output = "text",
         text = [[
A high mountain path stretches upwards in front of you.
Somewhere at its end, you will find %i,
and in its depths %n surely lurks.
         ]],
      },
      locate_next = {
         text = "Again, you near %i.",
      },
      lowlevel = {
         synopsis = "[Are you sure you are ready?]",
         output = "text",
         text = [[
"Oddly, %p, though you seem unready, I find myself wondering
whether the experience I have looked for in you is perhaps not the true
preparation you require.  Sending you forth now is a great risk,
however.  Are you sure you are ready?""]],
      },
      nemesis_first = {
         synopsis = "[You are no %g.  You shall never regain %o.]",
         output = "text",
         text = [["Ah, so %l has sent another %g to retrieve
%o.

"No, I see you are no %g.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Keep %o.  They will help you recover the Amulet of Yendor.]",
         output = "text",
         text = [[
"Amazing work, %p! You have recovered %o!

"I have meditated on the use to which %o could best be put.
The choice is clear: you must keep %o, and use them on your
quest for the Amulet of Yendor.

"I could not be more proud of you, %p.  Go with my blessing."
         ]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%lC studies %o for a moment,
then returns his gaze to you.

"%oC must remain with you.  Use %oi
as you resume your search for the Amulet.
As you know, you will find it somewhere in %Z, back through
the magic portal.]],
      },
      othertime = {
         text = [[Again you face %H.  Your intuition hints that this
may be the final time you come here.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  Is there any small assistance we can provide you?\"",
      },
   },
   Pri = {
      assignquest = {
         synopsis = "[%nC invaded %H and captured %o.  Defeat %ni and retrieve %oh.]",
         output = "text",
         text = [["Yes, %p.  You are truly ready now.  Attend to me and I shall
tell you of what has transpired:

"At one of the Great Festivals a short time ago, %n and a legion
of undead invaded %H.  Many %gP were killed, including
the one carrying %o.

"As a final act of vengefulness, %n desecrated the altar here.
Without it, we could not mount a counter-attack.  Now, there are
barely enough %gP left to keep the undead at bay.

"We need you to find %i, then, from there, travel
to %ns lair.  If you can manage to defeat %n and return
%o here, we can then drive off the legions of
undead that befoul the land.

"Go with %d as your guide, %p."]],
      },
      badalign = {
         synopsis = "[You have deviated from the path.  Return when you have purified yourself.]",
         output = "text",
         text = [["This is terrible, %p.  You have deviated from the true path!
You know that %d requires the most strident devotion of this
order.  The %shood must stand for utmost piety.

"Go from here, atone for your sins against %d.  Return only when
you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[%rA cannot withstand %n.  Come back when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, it is not yet to be.  A mere %r could never
withstand the might of %n.  Go forth, again into the world, and return
when you have attained the post of %R."]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"You can prevail, if you rely on %d.\"",
         "\"Remember that %n has great magic at his command.\"",
         "\"Be pure, my %S.\"",
         "\"Beware, %i is surrounded by a great graveyard.\"",
         "\"You may be able to affect %n with magical cold.\"",
         "\"Acquire and wear %o if you can.  It will aid you against %n.\"",
         "\"Call upon %d when your need is greatest.  You will be answered.\"",
         "\"The undead legions are weakest during the daylight hours.\"",
         "\"Do not lose faith, %p.  If you do so, %n will grow stronger.\"",
         "\"Wear %o.  It will assist you against the undead.\"",
      },
      firsttime = {
         synopsis = "[You are at %H; the doors are closed.  %lC needs your help!]",
         output = "text",
         text = [[You find yourself standing in sight of %H.  Something
is obviously wrong here.  The doors to %H, which usually
stand open, are closed.  Strange human shapes shamble around
outside.

You realize that %l needs your assistance!]],
      },
      goal_first = {
         synopsis = "[The stench of brimstone surrounds you, the shrieks and moans are endless.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the shrieks and moans
of tortured souls assault your psyche.

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel %d as you pick up %o; return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole it from
%H, for with it, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      guardtalk_after = {
         "\"Greetings, %r.  It is good to see you again.\"",
         "\"Ah, %p!  Our deepest gratitude for all of your help.\"",
         "\"Welcome back, %s!  With %o, no undead can stand against us.\"",
         "\"Praise be to %d, for delivering us from %n.\"",
         "\"May %d be with you, %s.\"",
      },
      guardtalk_before = {
         "\"Greetings, honored %r.  It is good to see you.\"",
         "\"Ah, %p!  Surely you can help us in our hour of need.\"",
         "\"Greetings, %s.  %lC has great need of your help.\"",
         "\"Alas, it seems as if even %d has deserted us.\"",
         "\"May %d be with you, %s.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and offer it on %ds altar.]",
         output = "text",
         text = [["You have prevailed, %p!  %d is surely with you.  Now,
you must take the amulet, and sacrifice it on %ds altar on
the Astral Plane.  I suspect that I shall never see you again in this
life, but I hope to at %ds feet."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.  Moloch is aware of you and angry at %n.]",
         output = "text",
         text = [[You feel a wrenching shift in the ether as %ns body dissolves
into a cloud of noxious gas.

Suddenly, a voice booms out:

    "Thou hast defeated the least of my minions, %r.
    Know now that Moloch is aware of thy presence.
    As for thee, %n, I shall deal with thy failure
    at my leisure."

You then hear the voice of %n, screaming in terror...]],
      },
      leader_first = {
         synopsis = "[You have returned and we need your help.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to us at last.
A great blow has befallen our order; perhaps you can help us.
First, however, I must determine if you are prepared for this
great challenge."]],
      },
      leader_last = {
         synopsis = "[You are a heretic who has deviated from the teachings of %d.]",
         output = "text",
         text = [["You are a heretic, %p!  How can you, %ra, deviate so from the
teachings of %d?  Begone from this temple.  You are no longer
%sa to this order.  We will pray to %d for other assistance,
as you have failed us utterly."]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      locate_first = {
         synopsis = "[You have found %i.  The trail to %n lies ahead.]",
         output = "text",
         text = [[You stand facing a large graveyard.  The sky above is filled with clouds
that seem to get thicker closer to the center.  You sense the presence of
undead in larger numbers than you have ever encountered before.

You remember the descriptions of %i, given to you by
%l.  It is ahead that you will find %ns trail.]],
      },
      locate_next = {
         text = "Again, you stand before %i.",
      },
      nemesis_first = {
         synopsis = "[%lC has sent you, but you are no %gC.  I shall destroy you.]",
         output = "text",
         text = [["Ah, so %l has sent another %gC to retrieve
%o.

"No, I see you are no %gC.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Congratulations, %p.  Keep %o; go and recover the Amulet.]",
         output = "text",
         text = [["You have returned, %p.  And with %o, I see.
Congratulations.

"I have been in meditation, and have received direction from
a minion of %d.  %d commands that you retain
%o.  With it, you must recover the Amulet
of Yendor.

"Go forth, and let %d guide your steps."]],
      },
      offeredit2 = {
         synopsis = "[%oC is yours now.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [[%lC reiterates that %o is yours now.

"The time has come to resume your search for the Amulet.
%Z await your return through the magic portal
that brought you here."]],
      },
      othertime = {
         text = [[Again you face %H.  Your intuition hints that this may be
the final time you come here.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  How is your quest for the Amulet going?\"",
      },
   },
   Ran = {
      assignquest = {
         synopsis = "[%nC has stolen %o.  Infiltrate %i and retrieve %oh for us.]",
         output = "text",
         text = [["You are indeed ready, %p.  I shall tell you what has transpired,
and why we so desperately need your help:

"A short time ago, the mountain centaurs to the east invaded
and enslaved the plains centaurs in this area.  The local
leader is now only a figurehead, and serves %n.

"During our last gathering of worship here, we were beset by hordes of
hostile centaurs, as you witnessed.  In the first onslaught a group,
headed by %n %niself, managed to breach the grove and steal
%o.

"Since then, we have been besieged.  We do not know how much longer
we will be able to maintain our magical barriers.

"If we are to survive, you, %p, must infiltrate
%i.  There, you will find a pathway down, to the
underground cavern of %n.  He has always coveted
%o, and will surely keep it.

"Recover %o for us, %p!  Only then will %d be safe."]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Come back when you have purified yourself.]",
         output = "text",
         text = [["You have strayed, %p!  You know that %d requires that
we maintain a pure devotion to things %a!

"You must go from us.  Return when you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Come back when you are %Ra.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to withstand the demands of that
which we need you to do.  %RA might just be able to do this thing.

"Return to us when you have learned more, my %S."]],
      },
      discourage = {
         "\"Your %d is nothing, %c.  You are mine now!\"",
         "\"Run away little %c!  You can never hope to defeat %n!\"",
         "\"My servants will rip you to shreds!\"",
         "\"I shall display your head as a trophy.  What do you think about that wall?\"",
         "\"I shall break your %ls grove, and destroy all the %gP!\"",
         "\"%d has abandoned you, %c.  You are doomed.\"",
         "\"%rA?  %lC sends a mere %r against me?  Hah!\"",
         "\"%lC has failed, %c.  %oC will never leave here.\"",
         "\"You really think you can defeat me, eh %c?  You are wrong!\"",
         "\"You weaken, %c.  I shall kill you now.\"",
      },
      encourage = {
         "\"It is rumored that the Forest and Mountain Centaurs have resolved their ancient feud and now band together against us.\"",
         "\"%nC is strong, and very smart.\"",
         "\"Use %o, when you find it.  It will help you survive to reach us.\"",
         "\"Remember, let %d be your guide.\"",
         "\"Call upon %d when you face %n.  The very act of doing so will infuriate him, and give you advantage.\"",
         "\"%n and his kind have always hated us.\"",
         "\"We cannot hold the grove much longer, %p.  Hurry!\"",
         "\"To infiltrate %i, you must be very stealthy.\"",
         "\"Remember that %n is a braggart.  Trust not what he says.\"",
         "\"You can triumph, %p, if you trust in %d.\"",
      },
      firsttime = {
         synopsis = "[The ancient forest grove is surrounded by centaurs.]",
         output = "text",
         text = [[You arrive in familiar surroundings.  In the distance, you %x the
ancient forest grove, the place of worship to %d.

Something is wrong, though.  Surrounding the grove are centaurs!
And they've noticed you!]],
      },
      goal_first = {
         synopsis = "[You descend into a subterranean complex.  Hooves clatter in the distance.]",
         output = "text",
         text = [[You descend into a weird place, in which roughly cut cave-like walls
join with smooth, finished ones, as if someone was in the midst of
finishing off the construction of a subterranean complex.

Off in the distance, you hear a sound like the clattering of many
hooves on rock.]],
      },
      goal_next = {
         text = "Once again, you enter the distorted castle of %n.",
      },
      gotit = {
         synopsis = "[You pick up %o and feel power.  It's time to return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, it seems to glow, and a warmth
fills you completely.  You realize that its power is what has protected
your %sp against their enemies for so long.

You must now return it to %l without delay -- their lives depend
on your speed.]],
      },
      guardtalk_after = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"Birdsong has returned to the grove, surely this means you have defeated %n.\"",
         "\"%lC seems to have regained some of his strength.\"",
         "\"So, tell us how you entered %i, in case some new evil arises there.\"",
         "\"Is that truly %o that I see you carrying?\"",
      },
      guardtalk_before = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"%nC continues to threaten the grove.  But we hold fast.\"",
         "\"%lC is growing weak.  The magic required to defend the grove drains us.\"",
         "\"Remember %i is hard to enter.  Beware the distraction of leatherwings.\"",
         "\"We must regain %o.  Without it we will be overrun.\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet!  Take it to the Astral Plane and offer it to %d.]",
         output = "text",
         text = [["You have it!  You have recovered the Amulet of Yendor!
Now attend to me, %p, and I will tell you what must be done:

"The Amulet has within it magic, the capability to transport you to
the Astral Plane, where the primary circle of %d resides.

"To activate this magic, you must travel upwards as far as you can.
When you reach the temple, sacrifice the Amulet to %d.

"Thus will you fulfill your destiny."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC collapses to the ground, cursing you and %l, then says:

    "You have defeated me, %r!  But I curse you one final time, with
    my dying breath!  You shall die before you leave my castle!"]],
      },
      leader_first = {
         synopsis = "[You have returned, %p.  We need your help.  Are you ready?]",
         output = "text",
         text = [["%pC!  You have returned!  Thank %d.

"We have great need of you.  But first, I must see if you have the
required abilities to take on this responsibility."]],
      },
      leader_last = {
         synopsis = "[You are not sufficiently %a.  We renounce your %shood.]",
         output = "text",
         text = [["%pC!  You have doomed us all.  You fairly radiate %L influences
and weaken the power we have raised in this grove as a result!

"Begone!  We renounce your %shood with us!  You are an outcast now!"]],
      },
      leader_next = {
         text = "\"Once again, %p, you stand in our midst.  Are you ready now?\"",
      },
      leader_other = {
         text = "\"Ah, you are here again, %p.  Allow me to determine your readiness...\"",
      },
      locate_first = {
         synopsis = "[This is %i.  There are bats nearby.  Beware the wumpus!]",
         output = "text",
         text = [[This must be %i.

You are in a cave built of many different rooms, all interconnected
by tunnels.  Your quest is to find and shoot the evil wumpus that
resides elsewhere in the cave without running into any bottomless
pits or using up your limited supply of arrows.  Good luck.

You are in room 9 of the cave.  There are tunnels to rooms
5, 8, and 10.
*rustle* *rustle* (must be bats nearby)
*sniff* (I can smell the evil wumpus nearby!)]],
      },
      locate_next = {
         synopsis = "[You are in %i.  There are pits.  There are bats nearby.]",
         output = "text",
         text = [[Once again, you descend into %i.

*whoosh* (I feel a draft from some pits).
*rustle* *rustle* (must be bats nearby)]],
      },
      nemesis_first = {
         synopsis = "[You have come to recover %o, but I shall keep %oh and you shall die.]",
         output = "text",
         text = [["So, %c.  %lC has sent you to recover %o.

"Well, I shall keep that bauble.  It pleases me.  You, %c, shall die."]],
      },
      nemesis_next = {
         text = "\"Back again, eh?  Well, a mere %r is no threat to me!  Die, %c!\"",
      },
      nemesis_other = {
         text = "\"You haven't learned your lesson, %c.  You can't kill me!  You shall die now.\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o from you, %r.  Then I shall
kill you."]],
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[You have succeeded.  Take %o with you as you go to find the Amulet.]",
         output = "text",
         text = [["%pC!  You have succeeded!  I feared it was not possible!

"You have returned with %o!

"I fear, now, that the Centaurs will regroup and plot yet another raid.
This will take some time, but if you can recover the Amulet of Yendor
for %d before that happens, we will be eternally safe.

"Take %o with you.  It will aid in your quest for
the Amulet."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o now.  Go and find the Amulet.]",
         output = "text",
         text = [[%l flexes %o reverently.

"With this wondrous bow, one need never run out of arrows.
You are its keeper now, and the time has come to resume your
search for the Amulet.  %Z await your return
through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[You have the oddest feeling that this may be the last time you
are to enter %H.]],
      },
      posthanks = {
         text = [["Welcome, %p.  How have you fared on your quest for the Amulet
of Yendor?"]],
      },
   },
   Rog = {
      assignquest = {
         synopsis = "[Get %o from %n and bring it to %l.]",
         output = "text",
         text = [["Will everyone not going to retrieve %o from that
jerk, %n, take one step backwards.  Good choice,
%p, because I was going to send you anyway.  My other %gp
are too valuable to me.

"Here's the deal.  I want %o, %n
has %o.  You are going to get %o
and bring it back to me.  So simple an assignment even you can understand
it."]],
      },
      badalign = {
         synopsis = "[Come back when you are really %a.]",
         output = "text",
         text = [["Maybe I should chain you to my perch here for a while.  Perhaps watching
real %a men at work will bring some sense back to you.  I don't
think I could stand the sight of you for that long though.  Come back
when you can be trusted to act properly."]],
      },
      badlevel = {
         synopsis = "[%rA is not adequately trained to handle this job.]",
         output = "text",
         text = [["In the time that you've been gone you've only been able to master the
arts of %ra?  I've trained ten times again as many %Rp
in that time.  Maybe I should send one of them, no?  Where would that
leave you, %p?  Oh yeah, I remember, I was going to kill you!"]],
      },
      discourage = {
         "\"May I suggest a compromise.  Are you interested in gold or gems?\"",
         "\"Please don't force me to kill you.\"",
         "\"Grim times are upon us all.  Will you not see reason?\"",
         "\"I knew %l, and you're no %lt, thankfully.\"",
         "\"It is a shame that we are not meeting under more pleasant circumstances.\"",
         "\"I was once like you are now, %p.  Believe in me -- our way is better.\"",
         "\"Stay with me, and I will make you %os guardian.\"",
         "\"When you return, with or without %o, %l will have you killed.\"",
         "\"Do not be fooled; I am prepared to kill to defend %o.\"",
         "\"I can reunite you with the Twain.  Oh, the stories you can swap.\"",
      },
      encourage = {
         "\"You don't seem to understand, %o isn't here so neither should you be!\"",
         "\"May %d curse you with lead fingers.  Get going!\"",
         "\"We don't have all year.  GET GOING!\"",
         "\"How would you like a scar necklace?  I'm just the jeweler to do it!\"",
         "\"Lazy S.O.B.  Maybe I should call up someone else...\"",
         "\"Maybe I should open your skull and see if my instructions are inside?\"",
         "\"This is not a task you can complete in the afterlife, you know.\"",
         "\"Inside every living person is a dead person trying to get out, and I have your key!\"",
         "\"We're almost out of hell-hound chow, so why don't you just get moving!\"",
         "\"You know, %o isn't going to come when you whistle.  You must get it yourself.\"",
      },
      firsttime = {
         synopsis = "[You are in Ransmannsby, where you trained.  Find %l.]",
         output = "text",
         text = [[Unexpectedly, you find yourself back in Ransmannsby, where you trained to
be a thief.  Quickly you make the guild sign, hoping that you AND word
of your arrival reach %ls den.]],
      },
      goal_first = {
         synopsis = "[You sense %o.]",
         output = "text",
         text = [[You feel a great swelling up of courage, sensing the presence of
%o.  Or is it fear?]],
      },
      goal_next = {
         text = "The hairs on the back of your neck whisper -- it's fear.",
      },
      gotit = {
         synopsis = "[You pick up %o and know that %l should not have it.]",
         output = "text",
         text = [[As you pick up %o, the hairs on the back of your
neck fall out.  At once you realize why %n was
willing to die to keep it out of %ls hands.  Somehow
you know that you must do likewise.]],
      },
      guardtalk_after = {
         "\"I was sure wrong about Lady Tyvefelle's house; I barely got away with my life and lost my lock pick in the process.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      guardtalk_before = {
         "\"I hear that Lady Tyvefelle's household is lightly guarded.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and find %ds temple.]",
         output = "text",
         text = [["I see that with your abilities, and my brains, we could rule this world.

"All that we would need to be all-powerful is for you to take that little
trinket you've got there up to the Astral Plane.  From there, %d will
show you what to do with it.  Once that's done, we will be invincible!"]],
      },
      killed_nemesis = {
         synopsis = "[Before dying, %n tells you to use the %o wisely.]",
         output = "text",
         text = [["I know what you are thinking, %p.  It is not too late for you
to use %o wisely.  For the sake of your guild
%sp, do what is right."

You sit and wait for death to come for %n, and then you
brace yourself for your next meeting with %l!]],
      },
      leader_first = {
         synopsis = "[You owe back dues to your guild.  You can pay them off if you're up to the job.]",
         output = "text",
         text = [["Well, look who it is boys -- %p has come home.  You seem to have
fallen behind in your dues.  I should kill you as an example to these
other worthless cutpurses, but I have a better plan.  If you are ready
maybe you could work off your back dues by performing a little job for
me.  Let us just see if you are ready..."]],
      },
      leader_last = {
         synopsis = "[You must go.]",
         output = "text",
         text = [["Well %gp, it looks like our friend has forgotten who is the boss
around here.  Our friend seems to think that %rp have been put in
charge.  Wrong.  DEAD WRONG!"

Your sudden shift in surroundings prevents you from hearing the end
of %ls curse.]],
      },
      leader_next = {
         synopsis = "[Are you stupid or are you ready?]",
         output = "text",
         text = [["Well, I didn't expect to see you back.  It shows that you are either stupid,
or you are finally ready to accept my offer.  Let us hope for your sake it
isn't stupidity that brings you back."]],
      },
      leader_other = {
         text = [["Did you perhaps mistake me for some other %lt?  You must
think me as stupid as your behavior.  I warn you not to try my patience."]],
      },
      locate_first = {
         text = "Those damn little hairs tell you that you are nearer to %o.",
      },
      locate_next = {
         text = "Not wanting to face %l without having stolen %o, you continue.",
      },
      nemesis_first = {
         text = "\"Ah!  You must be %ls ... er, `hero'.  A pleasure to meet you.\"",
      },
      nemesis_next = {
         text = "\"We meet again.  Please reconsider your actions.\"",
      },
      nemesis_other = {
         synopsis = "[You cannot trust %l.]",
         output = "text",
         text = [["Surely, %p, you have learned that you cannot trust any bargains
that %l has made.  I can show you how to continue on
your quest without having to run into him again."]],
      },
      nemesis_wantsit = {
         synopsis = "[%lC should not have %o.]",
         output = "text",
         text = [["Please, think for a moment about what you are doing.  Do you truly
believe that %d would want %l to have
%o?"]],
      },
      nexttime = {
         text = [[Once again, you find yourself back in Ransmannsby.  Fond memories are
replaced by fear, knowing that %l is waiting for you.]],
      },
      offeredit = {
         synopsis = "[Take %o with you and go.]",
         output = "text",
         text = [["Well, I'll be damned.  You got it.  I am proud of you, a fine %r
you've turned out to be.

"While you were gone I got to thinking, you and %o
together could bring me more treasure than either of you apart, so why don't
you take it with you.  All I ask is a cut of whatever loot you come by.
That is a better deal than I offered %n.

"But, you see what happened to %n when he refused.
Don't make me find another to send after you this time."]],
      },
      offeredit2 = {
         synopsis = "[Take %o and acquire the Amulet.]",
         output = "text",
         text = [[%lC seems tempted to swap %o for
the mundane one you detect in his pocket, but noticing your alertness,
evidently chickens out.

"Go filch the Amulet before someone else beats you to it.
%Z are back the way you came, through the magic portal."]],
      },
      othertime = {
         text = [[You rub your hands through your hair, hoping that the little ones on
the back of your neck stay down, and prepare yourself for your meeting
with %l.]],
      },
      posthanks = {
         synopsis = "[How about trading %o for something?]",
         output = "text",
         text = [["Quite the little thief, aren't we, %p.  Can I interest you in a
swap for %o?  Look around, anything in the keep
is yours for the asking."]],
      },
   },
   Sam = {
      assignquest = {
         synopsis = "[You must enter %i, then regain %o from %n.]",
         output = "text",
         text = [["Domo %p-san, indeed you are ready.  I can now tell you what
it is that I require of you.

"The daimyo, %n, has betrayed us.  He has stolen from us
%o and taken it to his donjon deep within
%i.

"If I cannot show the emperor %o when he comes
for the festival he will know that I have failed in my duty, and
request that I commit seppuku.

"You must gain entrance to %i and retrieve the
emperor's property.  Be quick!  The emperor will be here for the
cha-no-you in 5 sticks.

"Wakarimasu ka?"]],
      },
      badalign = {
         synopsis = "[When you can think %a and act %a then return.]",
         output = "text",
         text = [["%p-san, you would do better to join the kyokaku.

"You have skills, but until you can call upon the bushido to know when and
how to use them you are not samurai.  When you can think %a and
act %a then return."]],
      },
      badlevel = {
         synopsis = "[\"I require %Ra to defeat %n.  Return when you are ready.\"]",
         output = "text",
         text = [["%p-san, you have learned well and honored your family.
I require the skills of %Ra in order to defeat %n.
Go and seek out teachers.  Learn what they have learned.  When you
are ready, return to me."]],
      },
      discourage = {
         "\"Ahh, I finally meet the daimyo of the kyokaku!\"",
         "\"There is no honor for me in your death.\"",
         "\"You know that I cannot resash my swords until they have killed.\"",
         "\"Your presence only compounds the dishonor of %l in not coming %liself.\"",
         "\"I will make tea with your hair and serve it to %l.\"",
         "\"Your fear shows in your eyes, coward!\"",
         "\"I have not heard of you, %p-san; has your life been that unworthy?\"",
         "\"If you will not obey me, you will die.\"",
         "\"Kneel now and make the two cuts of honor.  I will tell your %sp of your honorable death.\"",
         "\"Your master was a poor teacher.  You will pay for his mistakes in your teaching.\"",
      },
      encourage = {
         "\"To defeat %n you must overcome the seven emotions: hate, adoration, joy, anxiety, anger, grief, and fear.\"",
         "\"Remember your honor is my honor, you perform in my name.\"",
         "\"I will go to the temple and burn incense for your safe return.\"",
         "\"Sayonara.\"",
         "\"There can be honor in defeat, but no gain.\"",
         "\"Your kami must be strong in order to succeed.\"",
         "\"You are indeed a worthy %R, but now you must be a worthy samurai.\"",
         "\"If you fail, %n will be like a tai-fun on the land.\"",
         "\"If you are truly %a, %d will listen.\"",
         "\"Sharpen your swords and your wits for the task before you.\"",
      },
      firsttime = {
         synopsis = "[The banner of %n flies above town.  What has happened to %l?]",
         output = "text",
         text = [[Even before your senses adjust, you recognize the kami of
%H.

You %x the standard of your teki, %n, flying above
the town.  How could such a thing have happened?  Why are ninja
wandering freely; where are the samurai of your daimyo, %l?

You quickly say a prayer to Izanagi and Izanami and walk towards
town.]],
      },
      goal_alt = {
         text = "As you arrive once again at the home of %n.",
      },
      goal_first = {
         synopsis = "[You feel the taunts %n, but after offering a prayer to %d, you proceed.]",
         output = "text",
         text = [[In your mind, you hear the taunts of %n.

You become like the rice plant and bend to the ground, offering a
prayer to %d.  But when the wind has passed, you stand
proudly again.  Putting your kami in the hands of fate, you advance.]],
      },
      goal_next = {
         text = [[As you arrive once again at the home of %n, your thoughts
turn only to %o.]],
      },
      gotit = {
         synopsis = "[You feel the power of %o and are humbled.]",
         output = "text",
         text = [[As you pick up %o, you feel the strength of its karma.
You realize at once why so many good samurai had to die to defend it.
You are humbled knowing that you hold one of the artifacts of the
sun goddess.]],
      },
      guardtalk_after = {
         "\"Come, join us in celebrating with some sake.\"",
         "\"Ikaga desu ka?\"",
         "\"You have brought our clan and %l much honor.\"",
         "\"Please %r, sit for a while and tell us how you overcame the Ninja.\"",
         "\"%lC still lives!  You have saved us from becoming ronin.\"",
      },
      guardtalk_before = {
         "\"To succeed, you must walk like a butterfly on the wind.\"",
         "\"Ikaga desu ka?\"",
         "\"I fear for The Land of The Gods.\"",
         "\"%nC has hired the Ninja -- be careful.\"",
         "\"If %o is not returned, we will all be ronin.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane to finish your task.]",
         output = "text",
         text = [["Ah, %p-sama.  You have wasted your efforts returning home.
Now that you are in possession of the Amulet, you are honor-bound to
finish the quest you have undertaken.  There will be plenty of time
for saki and stories when you have finished.

"Go now, and may our prayers be a wind at your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies without honor.]",
         output = "text",
         text = [[Your healing skills tell you that %ns wounds are mortal.

You know that the bushido tells you to finish him and let his kami
die with honor, but the thought of so many samurai dead due to this
man's dishonor prevents you from giving the final blow.

You order that his unwashed head be given to the crows and his body
thrown into the sea.]],
      },
      leader_first = {
         synopsis = "[%lC needs someone to lead %lj samurai against %n.  Are you ready?]",
         output = "text",
         text = [["Ah, %p-san, it is good to see you again.  I need someone who can
lead my samurai against %n.  If you are ready, you will be
that person."]],
      },
      leader_last = {
         synopsis = "[Leave and do not come back.]",
         output = "text",
         text = [["You are no longer my samurai, %p.

"Hara-kiri is denied.  You are ordered to shave your head and then to
become a monk.  Your fief and family are forfeit.  Wakarimasu ka?"]],
      },
      leader_next = {
         text = [["Once again, %p-san, you kneel before me.  Are you yet capable of
being my vassal?"]],
      },
      leader_other = {
         synopsis = "[Are you truly a samurai?]",
         output = "text",
         text = [["You begin to test my matsu, %p-san.
If you cannot determine what I want in a samurai, how can I rely on you
to figure out what I need from a samurai?"]],
      },
      locate_first = {
         text = [[You instinctively reach for your swords.  You do not recognize the
lay of this land, but you know that your teki are everywhere.]],
      },
      locate_next = {
         text = [[Thankful that your %sp at %H cannot see
your fear, you prepare again to advance.]],
      },
      nemesis_first = {
         text = [["Ah, so it is to be you, %p-san.  I offer you seppuku.
I will be your second if you wish."]],
      },
      nemesis_next = {
         text = [["I have offered you the honorable exit.  Now I will have your
head to send unwashed to %l."]],
      },
      nemesis_other = {
         text = "\"After I have dispatched you, I will curse your kami.\"",
      },
      nemesis_wantsit = {
         text = [["You have fought my samurai; surely you must know that you
will not be able to take %o back to
%H."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[The emperor wants you to take %o and recover the Amulet.]",
         output = "text",
         text = [[As you bow before %l, he welcomes you:

    "You have brought your family great honor, %p-sama.

    "While you have been gone the emperor's advisors have discovered in
    the ancient texts that the karma of the samurai who seeks to recover
    the Amulet and the karma of %o are joined
    as the seasons join to make a year.

    "Because you have shown such fidelity, the emperor requests
    that you take leave of other obligations and continue on the
    road that fate has set your feet upon.  I would consider it
    an honor if you would allow me to watch your household until
    you return with the Amulet."

With that, %l bows, and places his sword atop
%o.]],
      },
      offeredit2 = {
         synopsis = "[Take %o, return to %Z, and recover the Amulet.]",
         output = "text",
         text = [[%l holds %o tightly for a moment, then returns
his gaze to you.

"The time is ripe to recover the Amulet.  Return to %Z
through the magic portal that transported you here so that you may
achieve the destiny which awaits you."]],
      },
      othertime = {
         synopsis = "[%HC is threatened by %n.]",
         output = "text",
         text = [[You are back at %H.

Instantly you sense a subtle change in your karma.  You seem to know that
if you do not succeed in your quest, %n will have destroyed
the kami of %H before you return again.]],
      },
      posthanks = {
         text = "%lC bows.  \"%p-sama, tell us of your search for the Amulet.\"",
      },
   },
   Tou = {
      assignquest = {
         synopsis = "[Enter %i and recover %o from %n.]",
         output = "text",
         text = [["You have indeed proven yourself a worthy %c, %p.

"But now your kinfolk and I must ask you to put aside your travels and
help us in our time of need.  After you left us we elected a new mayor,
%n.  He proved to be a most heinous and vile creature.

"Soon after taking office he absconded with %o
and fled town, leaving behind his henchmen to rule over us.  In order
for us to regain control of our town, you must enter %i
and recover %o.

"Do not be distracted on your quest.  If you do not return quickly I fear
that all will be lost.  Let us both pray now that %d will guide you
and keep you safe."]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Return when you are.]",
         output = "text",
         text = [["It would be an affront to %d to have one not true to the
%a path undertake her bidding.

"You must not return to us until you have purified yourself of these
bad influences on your actions.  Remember, only by following the %a
path can you hope to overcome the obstacles you will face."]],
      },
      badlevel = {
         synopsis = "[Return when you are %Ra.]",
         output = "text",
         text = [["There is still too much that you have to learn before you can undertake
the next step.  Return to us as a proven %R, and perhaps then
you will be ready.

"Go back now, and may the teachings of %d serve you well."]],
      },
      discourage = {
         "\"I defeated %l and I will defeat you, %p.\"",
         "\"Where is %d now!  You must realize no one can help you here.\"",
         "\"Beg for mercy now and I may be lenient on you.\"",
         "\"If you were not so %a, you might have stood a chance.\"",
         "\"Vengeance is mine at last, %p.\"",
         "\"I only wish that %l had a more worthy %r to send against me.\"",
         "\"With %o in my possession you cannot hope to defeat me.\"",
         "\"%nC has never been defeated, NEVER!\"",
         "\"Are you truly the best %H has to send against me?  I pity %l.\"",
         "\"How do you spell %p?  I want to ensure the marker on your grave is correct as a warning to your %sp.\"",
      },
      encourage = {
         "\"Do not be fooled by the false promises of %n.\"",
         "\"To enter %i you must pass many traps.\"",
         "\"If you do not return with %o, your quest will be in vain.\"",
         "\"Do not be afraid to call upon %d if you truly need help.\"",
         "\"If you do not destroy %n, he will follow you back here!\"",
         "\"Take %o from %n and you may be able to defeat him.\"",
         "\"You must hurry, %p!\"",
         "\"You are like %Sa to me, %p.  Do not let me down.\"",
         "\"If you are %a at all times you may succeed, %p.\"",
         "\"Let all who meet you on your journey know that you are on a quest for %l and grant safe passage.\"",
      },
      firsttime = {
         synopsis = "[You find yourself back at %H, but the quiet is ominous.]",
         output = "text",
         text = [[You breathe a sigh of relief as you find yourself back in the familiar
surroundings of %H.

You quickly notice that things do not appear the way they did when you
left.  The town is dark and quiet.  There are no sounds coming from
behind the town walls, and no campfires burning in the fields.  As a
matter of fact, you do not %x any movement in the fields at all, and
the crops seem as though they have been untended for many weeks.]],
      },
      goal_alt = {
         text = "You have returned to %ns lair.",
      },
      goal_first = {
         text = "You sense the presence of %o.",
      },
      goal_next = {
         text = [[You gain confidence, knowing that you may soon be united with
%o.]],
      },
      gotit = {
         synopsis = "[You pick up %o and feel relief.  Return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel a great
weight has been lifted from your shoulders.  Your only thoughts are
to quickly return to %H and find %l.]],
      },
      guardtalk_after = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"If you stick around, I'll show you the pictures from my latest trip.\"",
         "\"Did you bring me back any souvenirs?\"",
      },
      guardtalk_before = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"Don't stay at the Inn, I hear the food is terrible and it has rats.\"",
         "\"They told me that this was the off season!\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet.  Take it to the Astral Plane to finish your task.]",
         output = "text",
         text = [["Stand back and let me look at you, %p.
Now that you have recovered the Amulet of Yendor, I'm afraid living
out your days in %H would seem pretty tame.

"You have come too far to stop now, for there are still more tasks that
our oral history foretells for you.  Forever more, though, your name shall
be spoken by the %gP with awe.  You are truly an inspiration to your
%sp!"]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses at you as %nh dies.]",
         output = "text",
         text = [[You turn in the direction of %n.  As his earthly body begins
to vanish before your eyes, you hear him curse:

    "You shall never be rid of me, %p!
    I will find you where ever you go and regain what is rightly mine."]],
      },
      leader_first = {
         synopsis = "[Someone must defeat %n.  Are your ready?]",
         output = "text",
         text = [["Is it really you, %p!  I had given up hope for your return.
As you can %x, we are desperately in need of your talents.  Someone must
defeat %n if our town is to become what it once was.

"Let me see if you are ready to be that someone."]],
      },
      leader_last = {
         synopsis = "[Leave %H and never return.]",
         output = "text",
         text = [["It is too late, %p.  You are not even worthy to die amongst us.
Leave %H and never return."]],
      },
      leader_next = {
         text = "\"Things are getting worse, %p.  I hope that this time you are ready.\"",
      },
      leader_other = {
         text = "\"I hope that for the sake of %H you have prepared yourself this time.\"",
      },
      locate_first = {
         synopsis = "[You %x the handiwork of %ns henchlings.]",
         output = "text",
         text = [[Only your faith in %d keeps you from trembling.  You %x
the handiwork of %ns henchlings everywhere.]],
      },
      locate_next = {
         text = "You know that this time you must find and destroy %n.",
      },
      nemesis_first = {
         synopsis = "[%rA will not defeat me.]",
         output = "text",
         text = [["So, %p, %l thinks that you can wrest
%o from me!

"It only proves how desperate he has become that he sends %ra to
try to defeat me.  When this day is over, I will have you enslaved
in the mines where you will rue the day that you ever entered
%i."]],
      },
      nemesis_next = {
         text = [["I let you live the last time because it gave me pleasure.
This time I will destroy you, %p."]],
      },
      nemesis_other = {
         synopsis = "[Run away or you will suffer severely.]",
         output = "text",
         text = [["These meetings come to bore me.  You disturb my workings with
%o.

"If you do not run away now, I will inflict so much suffering on you that
%l will feel guilty for ever having sent his %S to me!"]],
      },
      nemesis_wantsit = {
         synopsis = "[\"Return %o to me and we will rule %H.\"]",
         output = "text",
         text = [["You fool.  You do not know how to call upon the powers of
%o.

"Return it to me and I will teach you how to use it, and together we
will rule %H.  But do so now, as my patience grows thin."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o and with %ds guidance, recover the Amulet.]",
         output = "text",
         text = [[As %l detects the presence of %o,
he almost smiles for the first time in many a full moon.

As he looks up from %o he says:

    "You have recovered %o.  You are its
    owner now, but not its master.  Let it work with you as you continue
    your journey.  With its help, and %d to guide you on the
    %a path, you may yet recover the Amulet of Yendor."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z through the portal.]",
         output = "text",
         text = [["%oC is yours now.  %Z
await your return through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
Things appear to have become so bad that you fear that soon
%H will not be here to return to.]],
      },
      posthanks = {
         text = [["I could not be more proud than if you were my own %S, %p!
Tell me of your adventures in quest of the Amulet of Yendor."]],
      },
   },
   Val = {
      assignquest = {
         synopsis = "[Find %i; defeat %n; return with %o.]",
         output = "text",
         text = [["It is not clear, %p, for my sight is limited without our relic.
But it is now likely that you can defeat %n, and recover
%o.

"A short time ago, %n and his minions attacked this place.  They
opened the huge volcanic vents you %x about the hill, and attacked.  I knew
that this was to come to pass, and had asked %d for a group of %gP
to help defend this place.  The few you %x here are the mightiest of
Valhalla's own, and are all that are left of one hundred %d sent.

"Despite the great and glorious battle we fought, %n managed at
last to steal %o.  This has upset the balance of the universe,
and unless %oh is returned into my care, %n may start Ragnarok.

"You must find the entrance to %i.  Travel downward
from there and you will find %ns lair.  Defeat him and
return %o to me."]],
      },
      badalign = {
         synopsis = "[You have strayed from the %a path.  Return after you purify yourself.]",
         output = "text",
         text = [["NO!  This is terrible.  I see you becoming an ally of %n, and
leading his armies in the final great battles.  This must not come to
pass!  You have strayed from the %a path.  You must purge yourself,
and return here only when you have regained a state of purity."]],
      },
      badlevel = {
         synopsis = "[Come back when you are %Ra.]",
         output = "text",
         text = [["I see you and %n fighting, %p.  But you are not prepared and
shall die at %ns hand if you proceed.  No.  This will not do.
Go back out into the world, and grow more experienced at the ways of war.
Only when you have returned %Ra will you be able to defeat %n."]],
      },
      discourage = {
         "\"I am your death, %c.\"",
         "\"You cannot prevail, %r.  I have foreseen your every move.\"",
         "\"With you out of the way, Valhalla will be mine for the taking.\"",
         "\"I killed scores of %ds best when I took %o.  Do you really think that one %c can stand against me?\"",
         "\"Who bears the souls of %cP to Valhalla, %r?\"",
         "\"No, %d cannot help you here.\"",
         "\"Some instrument of %d you are, %p.  You are a weakling!\"",
         "\"Never have I seen %ca so clumsy in battle.\"",
         "\"You die now, little %s.\"",
         "\"Your body I destroy now, your soul when my hordes overrun Valhalla!\"",
      },
      encourage = {
         "\"Go with the blessings of %d.\"",
         "\"Call upon %d when you are in need.\"",
         "\"Use %o if you can.  It will protect you.\"",
         "\"Magical cold is very effective against %n.\"",
         "\"To face %n, you will need to be immune to fire.\"",
         "\"May %d strengthen your sword-arm.\"",
         "\"Trust in %d.  He will not desert you.\"",
         "\"It becomes more likely that Ragnarok will come with every passing moment.  You must hurry, %p.\"",
         "\"If %n can master %o, he will be powerful enough to face %d far earlier than is fated.  This must not be!\"",
         "\"Remember your training, %p.  You can succeed.\"",
      },
      firsttime = {
         synopsis = "[You arrive below %H.  Something is wrong; there is lava present.]",
         output = "text",
         text = [[You materialize at the base of a snowy hill.  Atop the hill sits
a place you know well, %H.  You immediately realize
that something here is very wrong!

In places, the snow and ice have been melted into steaming pools of
water.  Fumaroles and pools of bubbling lava surround the hill.
The stench of sulphur is carried through the air, and you %x creatures
that should not be able to live in this environment moving towards you.]],
      },
      goal_first = {
         synopsis = "[This is the lair of %n.]",
         output = "text",
         text = [[Through clouds of sulphurous gases, you %x a wall of great stones
surrounded with a moat of bubbling lava.  This can only be the fortress
of %n.  From somewhere ahead, you hear a great deep voice
shouting orders, and you silently hope you have not arrived too late.]],
      },
      goal_next = {
         text = "Once again, you stand in sight of %ns fortress.",
      },
      gotit = {
         synopsis = "[You must return %o to %l.]",
         output = "text",
         text = [[As you pick up %o, your mind is suddenly filled with images,
and you perceive all of the possibilities of each potential choice you
could make.  As you begin to control and channel your thoughts, you
realize that you must return %o to %l immediately.]],
      },
      guardtalk_after = {
         "\"Hail, and well met, brave %c.\"",
         "\"May %d guide your steps, %p.\"",
         "\"%lC told us you had succeeded!\"",
         "\"You recovered %o just in time, %p.\"",
         "\"Hail %d, for delivering %o back to us.\"",
      },
      guardtalk_before = {
         "\"Hail, and well met, brave %c.\"",
         "\"May %d guide your steps, %p.\"",
         "\"%lC weakens.  Without %o, her foresight is dim.\"",
         "\"You must hurry, %p, else Ragnarok may well come.\"",
         "\"I would deal with this foul %n myself, but %d forbids it.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds temple on the Astral Plane and offer it.]",
         output = "text",
         text = [["Excellent, %p.  I see you have recovered the Amulet!

"You must take the Amulet to the Great Temple of %d, on the Astral
Plane.  There you must offer the Amulet to %d.

"Go now, my %S.  I cannot tell you your fate, as the power of the
Amulet interferes with mine.  I hope for your success."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.]",
         output = "text",
         text = [[A look of surprise and horror appears on %ns face.

    "No!!!  %o has lied to me!  I have been misled!"

Suddenly, %n grasps his head and screams in agony, then dies.]],
      },
      leader_first = {
         synopsis = "[We need your aid.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to %H
at last.  We are in dire need of your aid, but I must determine if you
are yet ready for such an undertaking.

"Let me read your fate..."]],
      },
      leader_last = {
         synopsis = "[\"Begone from my presence and never return.\"]",
         output = "text",
         text = [["No, %p.  Your fate is sealed.  I must cast about for another
champion.  Begone from my presence, and never return.  Know this, that
you shall never succeed in this life, and Valhalla is denied to you."]],
      },
      leader_next = {
         text = [["Let me read the future for you now, %p, perhaps you have managed to
change it enough..."]],
      },
      leader_other = {
         text = [["Again, I shall read your fate, my %S.  Let us both hope that you have
made changes to become ready for this task..."]],
      },
      locate_first = {
         synopsis = "[This is %i.]",
         output = "text",
         text = [[
Your pursuit of %n brings you to an area you have visited only
once or twice before in your life, but there's no way you have forgotten
it -- here is the majestic rainbow bridge Bifrost, connecting this realm
to Muspelheim far below.

But what is this?  As you draw closer, you %x that it is shattered
and broken.  Cursing, you remember that this too was prophesied: that it
would be sundered by the weight of %n and %nj forces
marching across it.

Before you can think about crossing, you realize you have been spotted.
A grim giant with hatred in his eyes points at you with a gleaming sword
and says something to his compatriots.  They start moving towards you.]]
      },
      locate_next = {
         text = "Once again, you stand before %i.",
      },
      nemesis_first = {
         synopsis = "[\"%oC has shown me that I must kill you.\"]",
         output = "text",
         text = [["So!  %lC has finally sent %ca to challenge me!

"I thought that mastering %o would enable me to challenge
%d, but it has shown me that first I must kill you!  So come, little
%s.  Once I defeat you, I can at last begin the final battle with %d."]],
      },
      nemesis_next = {
         text = "\"Again you challenge me, %r.  Good.  I will kill you now.\"",
      },
      nemesis_other = {
         text = "\"Have you not learned yet?  You cannot defeat %n!\"",
      },
      nemesis_wantsit = {
         text = "\"I will kill you, %c, and wrest %o from your mangled hands.\"",
      },
      nexttime = {
         text = "Once again, you are near the abode of %l.",
      },
      offeredit = {
         synopsis = "[Take %o.  Search for the Amulet.]",
         output = "text",
         text = [[As you approach, %l rises and touches %o.

"You may take %o with you, %p.  I have removed from
it the power to foretell the future, for that power no mortal should
have.  Its other abilities, however, you have at your disposal.

"You must now begin in %ds name to search for the Amulet of Yendor.
May your steps be guided by %d, my %S."]],
      },
      offeredit2 = {
         synopsis = "[You are %os keeper now.  Return through the portal and find the Amulet.]",
         output = "text",
         text = [["Careful, %p!  %oC might break, and that would be
a tragic loss.  You are its keeper now, and the time has come to
resume your search for the Amulet.  %Z await your
return through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[Again you materialize near %ls abode.  You have a nagging feeling
that this may be the last time you come here.]],
      },
      posthanks = {
         text = [["Greetings, %p.  I have not been able to pay as much attention to
your search for the Amulet as I have wished.  How do you fare?"]],
      },
   },
   Wiz = {
      assignquest = {
         synopsis = "[Travel to %i; overcome %n; return with %o.]",
         output = "text",
         text = [["Yes, %p, you truly are ready for this dire task.  Listen,
carefully, for what I tell you now will be of vital importance.

"Since you left us to hone your skills in the world, we unexpectedly came
under attack by the forces of %n.  As you know, we thought
%n had perished at the end of the last age, but, alas, this was
not the case.

"%nC sent an army of abominations against us.  Among them was a
minion, mindless and ensorcelled, and thus, in the confusion, it was
able to penetrate our defenses.  Alas, this creature has stolen
%o and I fear has delivered %oh to %n.

"Over the years, I had woven most of my power into this amulet, and thus,
without it, I have but a shadow of my former power, and I fear that I
shall soon perish.

"You must travel to %i, and within its dungeons,
find and overcome %n, and return %o to me.

"Go now, with %d, and complete this quest before it is too late."]],
      },
      badalign = {
         synopsis = "[Go; come back when you are worthy of %d.]",
         output = "text",
         text = [["You amaze me, %p!  How many times did I tell you that the way of a mage
is an exacting one.  One must use the world with care, lest one leave it
in ruins and simplify the task of %n.

"You must go back and show your worthiness.  Do not return until you are
truly ready for this quest.  May %d guide you in this task."]],
      },
      badlevel = {
         synopsis = "[Go; return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you have not yet shown your proficiency as a worthy
spellcaster.  As %ra, you would surely be overcome in the challenge
ahead.  Go, now, expand your horizons, and return when you have attained
renown as %Ra."]],
      },
      discourage = {
         "\"Your puny powers are no match for me, fool!\"",
         "\"When you are defeated, your torment will last for a thousand years.\"",
         "\"After your downfall, %p, I shall devour %l for dessert!\"",
         "\"Are you ready yet to beg for mercy?  I could be lenient...\"",
         "\"Your soul shall join the enslaved multitude I command!\"",
         "\"Your lack of will is evident, and you shall die as a result.\"",
         "\"Your faith in %d is for naught!  Come, submit to me now!\"",
         "\"A mere %r is nothing compared to my skill!\"",
         "\"So, you are the best hope of %l?  How droll.\"",
         "\"Feel my power, %c!  My victory is imminent!\"",
      },
      encourage = {
         "\"Beware, for %n is immune to most magical attacks.\"",
         "\"To enter %i you must pass many traps.\"",
         "\"%nC may be vulnerable to physical attacks.\"",
         "\"%d will come to your aid when you call.\"",
         "\"You must utterly destroy %n.  He will pursue you otherwise.\"",
         "\"%oC is a mighty artifact.  With it you can destroy %n.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"Feel free to take any items in that chest that might aid you.\"",
         "\"You will know when %o is near.  Proceed with care!\"",
      },
      firsttime = {
         synopsis = "[You have arrived at %ls tower but something is very wrong.]",
         output = "text",
         text = [[You are suddenly in familiar surroundings.  You notice what appears to
be a large, squat stone structure nearby.  Wait!  That looks like the
tower of your former teacher, %l.

However, things are not the same as when you were last here.  Mists and
areas of unexplained darkness surround the tower.  There is movement in
the shadows.

Your teacher would never allow such unaesthetic forms to surround the
tower...  unless something were dreadfully wrong!]],
      },
      goal_alt = {
         text = "You have returned to %ns lair.",
      },
      goal_first = {
         text = "You feel your mentor's presence; perhaps %o is nearby.",
      },
      goal_next = {
         text = "The aura of %o tingles at the edge of your perception.",
      },
      gotit = {
         synopsis = "[You feel %os power and know you should return %oh to %l.]",
         output = "text",
         text = [[As you touch %o, its comforting power infuses you
with new energy.  You feel as if you can detect others' thoughts flowing
through it.  Although you yearn to wear %o and
attack the Wizard of Yendor, you know you must return it to its rightful
owner, %l.]],
      },
      guardtalk_after = {
         "\"I have some eye of newt to trade, do you have a spare blind-worm's sting?\"",
         "\"The magic portal now seems like it will remain stable for quite some time.\"",
         "\"Have you noticed how much stronger %l is since %o was recovered?\"",
         "\"Thank %d!  We weren't positive you would defeat %n.\"",
         "\"I, too, will venture into the world, because %n was but one of many evils to be vanquished.\"",
      },
      guardtalk_before = {
         "\"Would you happen to have some eye of newt in that overstuffed pack, %s?\"",
         "\"Ah, the spell to create the magic portal worked.  Outstanding!\"",
         "\"Hurry!  %lC may not survive that casting of the portal spell!\"",
         "\"The spells of %n were just too powerful for us to withstand.\"",
         "\"I, too, will venture into the world, because %n is but one of many evils to be vanquished.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds altar on the Astral Plane.]",
         output = "text",
         text = [["Congratulations, %p.  I always knew that if anyone could succeed
in defeating the Wizard of Yendor and his minions, it would be you.

"Go now, and take the Amulet to the Astral Plane.  Once there, present
the Amulet on the altar of %d.  Along the way you shall pass through
the four Elemental Planes.  These planes are like nothing you have ever
experienced before, so be prepared!

"For this you were born, %s!  I am very proud of you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC, whose body begins to shrivel up, croaks out:

    "I shall haunt your progress until the end of time.  A thousand
    curses on you and %l."

Then, the body bursts into a cloud of choking dust, and blows away.]],
      },
      leader_first = {
         synopsis = "[You have come a long way, but are you ready for the task I require?]",
         output = "text",
         text = [["Come closer, %p, for my voice falters in my old age.
Yes, I see that you have come a long way since you went out into the
world, leaving the safe confines of this tower.  However, I must first
determine if you have all of the skills required to take on the task
I require of you."]],
      },
      leader_last = {
         synopsis = "[\"Get out of here!\"]",
         output = "text",
         text = [["You fool, %p!  Why did I waste all of those years teaching you
the esoteric arts?  Get out of here!  I shall find another."]],
      },
      leader_next = {
         text = "\"Well, %p, you have returned.  Perhaps you are now ready...\"",
      },
      leader_other = {
         text = [["This is getting tedious, %p, but perseverance is a sign of a true mage.
I certainly hope that you are truly ready this time!"]],
      },
      locate_first = {
         text = "Wisps of fog swirl nearby.  You feel that %ns lair is close.",
      },
      locate_next = {
         text = "You believe that you may once again invade %i.",
      },
      nemesis_first = {
         synopsis = "[\"Your destruction should make for good sport.\"]",
         output = "text",
         text = [["Ah, I recognize you, %p.  So, %l has sent you to steal
%o from me, hmmm?  Well, %lh is a fool to send such
a mental weakling against me.

"Your destruction, however, should make for good sport.  In the end, you
shall beg me to kill you!"]],
      },
      nemesis_next = {
         synopsis = "[\"Your soul shall soon be mine to command.\"]",
         output = "text",
         text = [["How nice of you to return, %p!  I enjoyed our last meeting.  Are you
still hungry for more pain?

"Come!  Your soul, like %o, shall soon be mine to command."]],
      },
      nemesis_other = {
         text = [["I'm sure that your perseverance shall be the subject of innumerable
ballads, but you shall not be around to hear them, I fear!"]],
      },
      nemesis_wantsit = {
         text = [["Thief!  %oC belongs to me, now.  I shall feed
your living flesh to my minions."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o with you in your quest for the Amulet.]",
         output = "text",
         text = [[%lC notices %o in your possession,
beams at you and says:

    "I knew you could defeat %n and retrieve
    %o.  We shall never forget this
    brave service.

    "Take %oh with you in your quest for the Amulet of Yendor.
    I can sense that it has attuned %oiself to you already.

    "May %d guide you in your quest, and keep you from harm."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o, return through the portal to %Z; find the other Amulet.]",
         output = "text",
         text = [["You are the keeper of %o now.  It is time to
recover the /other/ Amulet.  %Z await your return through
the magic portal which brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         text = [["Come near, my %S, and share your adventures with me.
So, have you succeeded in your quest for the Amulet of Yendor?"]],
      },
   },
}
