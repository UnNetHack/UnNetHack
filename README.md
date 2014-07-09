# UnNetHackButLess

UnNetHackButLess is a port of UnNetHack to Scala with additional modernization.  

[UnNetHack](https://github.com/UnNetHack/UnNetHack UnNetHack) is used as a
basis for the project because it keeps the flavor of vanilla 
[NetHack](http://nethack.org) while still being actively maintained and enhanced.  
It a fork of 
NetHack and originally based on NetHack version 3.4.3.  It features more randomness, 
more levels, more challenges and more fun than vanilla NetHack.  Patric Mueller, 
the creator of UnNetHack describes it as "how NetHack would look today if the 
DevTeam didn't stop releasing".

## Conversion Methodology

The source for UnNetHack will be gradually converted to Scala on a file-by-file basis.

### Phase 1

The first pass will the keep the code as similar to the C as possible.  This 
will help preserve the lineage of the code and may simplify incorporating future 
improvements to UnNetHack.  This will create some truly abhorrent Scala in the 
short term, a credit to Scala for allowing such horrible abuse of it's type system
and features (such as implicit conversions).  Importantly, gradual conversion 
will also reduce risks caused by making too many simultaneous modifications.

There is only minor conversion to better structures at this point.  All filenames and 
identifiers will be as similar to the original as possible, except where those 
identifiers drastically break with Scala coding conventions. 

### Phase 2

Once the initial conversion is complete the process of converting code will be refactored 
into idiomatic modern Scala will begin.  This is when the payoff begins and the result
should be an application that is much easier to maintain and can grow gracefully.

### Phase 3

Make improvements and additions that would have otherwise been difficult, such as 
refactoring into a client/server design.

## Coding Conventions

NetHack was written to early C standards so the basic data types are assumed to be the minimum
required by the specification, such as a 16 bit "int" and 8 bit "char".  The Java Virtual Machine
lacks unsigned types so those are promoted up to the next higher type.  Types will be simplified
in a future version.

There are markers used to indicate comments that are used only during conversion to Scala:

 * MOTODO - An item to complete, often used when commenting out code that may not be required.
 * MONOTE - A note about the conversion.

# Community

For discussion, join the IRC channel #unnethack on irc.freenode.net, post to
rec.games.roguelike.nethack.

# History

The process of modernizing NetHack began in June 2014 by Sheldon Young.

 -- Good luck, and happy Hacking
