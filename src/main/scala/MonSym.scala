/*	Monster symbols and creation information rev 1.0	  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object MonSym {

/*
 * Monster classes.  Below, are the corresponding default characters for
 * them.  Monster class 0 is not used or defined so we can use it as a
 * NULL character.
 */
val S_ANT = 1
val S_BLOB = 2
val S_COCKATRICE = 3
val S_DOG = 4
val S_EYE = 5
val S_FELINE = 6
val S_GREMLIN = 7
val S_HUMANOID = 8
val S_IMP = 9
val S_JELLY = 10
val S_KOBOLD = 11
val S_LEPRECHAUN = 12
val S_MIMIC = 13
val S_NYMPH = 14
val S_ORC = 15
val S_PIERCER = 16
val S_QUADRUPED = 17
val S_RODENT = 18
val S_SPIDER = 19
val S_TRAPPER = 20
val S_UNICORN = 21
val S_VORTEX = 22
val S_WORM = 23
val S_XAN = 24
val S_LIGHT = 25
val S_ZRUTY = 26
val S_ANGEL = 27
val S_BAT = 28
val S_CENTAUR = 29
val S_DRAGON = 30
val S_ELEMENTAL = 31
val S_FUNGUS = 32
val S_GNOME = 33
val S_GIANT = 34
val S_JABBERWOCK = 36
val S_KOP = 37
val S_LICH = 38
val S_MUMMY = 39
val S_NAGA = 40
val S_OGRE = 41
val S_PUDDING = 42
val S_QUANTMECH = 43
val S_RUSTMONST = 44
val S_SNAKE = 45
val S_TROLL = 46
val S_UMBER = 47
val S_VAMPIRE = 48
val S_WRAITH = 49
val S_XORN = 50
val S_YETI = 51
val S_ZOMBIE = 52
val S_HUMAN = 53
val S_GHOST = 54
val S_GOLEM = 55
val S_DEMON = 56
val S_EEL = 57
val S_LIZARD = 58

val S_WORM_TAIL = 59
val S_MIMIC_DEF = 60

val S_STATUE = 61

val MAXMCLASSES = 62	/* number of monster classes */

/*
 * Default characters for monsters.  These correspond to the monster classes
 * above.
 */
val DEF_ANT = 'a'
val DEF_BLOB = 'b'
val DEF_COCKATRICE = 'c'
val DEF_DOG = 'd'
val DEF_EYE = 'e'
val DEF_FELINE = 'f'
val DEF_GREMLIN = 'g'
val DEF_HUMANOID = 'h'
val DEF_IMP = 'i'
val DEF_JELLY = 'j'
val DEF_KOBOLD = 'k'
val DEF_LEPRECHAUN = 'l'
val DEF_MIMIC = 'm'
val DEF_NYMPH = 'n'
val DEF_ORC = 'o'
val DEF_PIERCER = 'p'
val DEF_QUADRUPED = 'q'
val DEF_RODENT = 'r'
val DEF_SPIDER = 's'
val DEF_TRAPPER = 't'
val DEF_UNICORN = 'u'
val DEF_VORTEX = 'v'
val DEF_WORM = 'w'
val DEF_XAN = 'x'
val DEF_LIGHT = 'y'
val DEF_ZRUTY = 'z'
val DEF_ANGEL = 'A'
val DEF_BAT = 'B'
val DEF_CENTAUR = 'C'
val DEF_DRAGON = 'D'
val DEF_ELEMENTAL = 'E'
val DEF_FUNGUS = 'F'
val DEF_GNOME = 'G'
val DEF_GIANT = 'H'
val DEF_JABBERWOCK = 'J'
val DEF_KOP = 'K'
val DEF_LICH = 'L'
val DEF_MUMMY = 'M'
val DEF_NAGA = 'N'
val DEF_OGRE = 'O'
val DEF_PUDDING = 'P'
val DEF_QUANTMECH = 'Q'
val DEF_RUSTMONST = 'R'
val DEF_SNAKE = 'S'
val DEF_TROLL = 'T'
val DEF_UMBER = 'U'
val DEF_VAMPIRE = 'V'
val DEF_WRAITH = 'W'
val DEF_XORN = 'X'
val DEF_YETI = 'Y'
val DEF_ZOMBIE = 'Z'
val DEF_HUMAN = '@'
val DEF_GHOST = ' '
val DEF_GOLEM = '\''
val DEF_DEMON = '&'
val DEF_EEL = ';'
val DEF_LIZARD = ':'

val DEF_INVISIBLE = 'I'
val DEF_WORM_TAIL = '~'
val DEF_MIMIC_DEF = ']'

val DEF_STATUE = '`'

}
