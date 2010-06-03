The homepage for this project may be found at:

http://nethack-curses.wikia.com/wiki/NetHack_Curses_Interface_Wiki

INTRO
=====

The "curses" windowport is a new text-based interface for NetHack,
using high-level curses routines to control the display.  Currently, it
has been compiled and tested on Linux and Windows, but it should also
be portable to a number of other systems, such as other forms of UNIX,
Mac OS X, MSDOS, and OS/2.

Some features of this interface compared to the traditional tty
interface include:

 * Dynamic window resizing (e.g. maximizing a terminal window)
 * Dynamic configurable placement of status and message windows,
 relative to the map
 * Makes better use of larger and smaller terminal windows
 * Fancier display (e.g. window borders, optional popup dialogs,
 splash screen, and better menus)
 * "cursesgraphics" option for fancier line-drawing characters for
 drawing the dungeon - this should work on most terminals/platforms


GAMEPLAY
========

Gameplay should be similar to the tty interface for NetHack; the 
differences are primarily visual.  This windowport supports dymanic
resizing of the terminal window, so you can play with it to see how it
looks best to you during a game.  Also, the align_status and
align_message options may be set during the game or in the config file
to place the status and message windows where you like, so you can
experiment to see what arraingement looks best to you.

For menus, in addition to the normal configurable keybindings for menu
navigation described in the Guidebook, you can use the right and left
arrows to to forward or backward one page, respectively, and the home
and end keys to go to the first and last pages, respectively.

Some configuration options that are specific to or relevant to the
curses windowport are shown below.  Copy any of these that you like to
your nethack configuration file (e.g. .nethackrc for UNIX or
NetHack.cnf for Windows):
#
# Use this if the binary was compiled with multiple window interfaces,
# and curses is not the default
OPTIONS=windowtype:curses
#
# Set this for Windows systems, or for PDCurses for SDL on any system.
# The latter uses a cp437 font, which works with this option
#OPTIONS=IBMgraphics
#
# Set this if IBMgraphics above won't work for your system.  Mutually
# exclusive with the above option, and should work on nearly any
# system.
OPTIONS=cursesgraphics
#
# Optionally specify the alignment of the message and status windows
# relative to the map window.  If not specified, the code will default
# to the locations used in the tty interface: message window on top,
# and status window on bottom.  Placing either of these on the right or
# left really only works well for winder terminal windows.
OPTIONS=align_message:bottom,align_status:right
#
# Use a small popup "window" for short prompts, e.g. "Really save?".
# If this is not set, the message window will be used for these as is
# done for the tty interface.
OPTIONS=popup_dialog
#
# Specify the initial window size for NetHack in units of characters.
# This is supported on PDCurses for SDL as well as PDCurses for
# Windows.
OPTIONS=term_cols:110,term_rows:32
#
# Controls the usage of window borders for the main NetHack windows
# (message, map, and status windows).  A value of 1 forces the borders
# to be drawn, a value of 2 forces them to be off, and a value of 3
# allows the code to decide if they should be drawn based on the size
# of the terminal window.
OPTIONS=windowborders:3
#
# Whether or not to display a fancy ascii-graphics splash screen on
# startup.
OPTIONS=splash_screen
#
# This controls what text attributes are used to highlight any pets.
# Any combination of single letters representing each text attribute
# may be used.  Not all attributes work on all terminals.
#
# Attributes may be one or more of: Bold, Inverse, Underline, blinK,
# iTalic, Rightline, or Leftline.  The default is underline.
OPTIONS=petattr:u
#
# Controls whether color and attributes are used for menu headers,
# window borders, message text, etc.  This is separate from the color
# option, which only controls the color of the map itself.
OPTIONS=guicolor


BUILDING
========

As of this writing code has been compiled on Linux, Windows and Mac OS
X.

UNIX/Linux build instructions: Follow the instructions in
sys/unix/Install.unx.  By default, the Makefile is setup to compile
against ncurses.  Edit Makefile.src if you wish to compile against a
different curses library, such as PDCurses for SDL.

Windows build instructions: If you are using Mingw32 as your compiler,
then follow the instructions in sys/winnt/Install.nt with the following
changes:

 * Patch the NetHack source code with an appropriate Windows patch
 utility
 * Download and install SDL and PDCurses-SDL, or download winlibs.zip
 from the same place you got the curses interface patch, and unzip it
 into the root NetHack source directory
 * Copy curses.h from the PDCurses distribution into the nethack
 include directory, or get it from the aforementioned winlibs.zip.
 * Instead of typing "mingw32-make -f Makefile.gcc install" you will
 type "mingw32-make -f cursmake.gcc install"

If you are using a different compiler, you will have to manually modify
the appropriate Makefile to include the curses windowport files.

Mac OS X Build instructions: These instructions assume you are using
PDCurses for SDL.  I have not yet compiled with NCurses on the Mac due
to a couple of link errors:

 * Install the SDL library via fink or the like.
 * Download PDCurses from http://pdcurses.sourceforge.net/ unpack it,
 change to the sdl1 subdirectory, and type "make"
 * Copy the generated pdcurses.a somewhere where it can be found by the
 compiler, e.g. /usr/local/lib
 * Run the following command from the sys/unix directory: sh ./setup.sh
 * Edit src/Makefile, comment-out the CFLAGS line (152), and uncomment
 the CFLAGS line for Max OS X above (line 148)
 * Comment the WINCURSESLIB line for ncurses (line 257), and uncomment
 the WINCURSESLIB for PDCurses for SDL (line 259)
 * Follow steps 4 and 7-11 on this page:
 http://nethack.wikia.com/wiki/Compiling#On_Mac_OS_X


CONTACT
=======

Please send any bug reports, suggestions, patches, or miscellaneous
feedback to me (Karl Garrison) at: kgarrison@pobox.com.

Happy Hacking!

Karl Garrison
April, 2010

http://nethack-curses.wikia.com/wiki/NetHack_Curses_Interface_Wiki

