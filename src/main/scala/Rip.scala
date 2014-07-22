/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Hack._
import WinType._

object Rip {

/* A normal tombstone for end of game display. */
val rip_txt: List[String] = List(
"                       ----------",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |", /* Name of player */
"                  |                  |", /* Amount of $ */
"                  |                  |", /* Type of death */
"                  |                  |", /* . */
"                  |                  |", /* . */
"                  |                  |", /* . */
"                  |       1001       |", /* Real year of death */
"                 *|     *  *  *      | *",
"        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
null
)
val STONE_LINE_CENT = 28	/* char[] element of center of stone face */
val STONE_LINE_LEN = 16		/* # chars that fit on one line
				 * (note 1 ' ' border)
				 */
val NAME_LINE = 6		/* *char[] line # for player name */
val GOLD_LINE = 7		/* *char[] line # for amount of gold */
val DEATH_LINE = 8		/* *char[] line # for death description */
val YEAR_LINE = 12		/* *char[] line # for year */

var rip: Array[Array[Char]] = null

private def center(line: Int, text: String): Unit =
{
/*** MOTODOO: Convert
	var ip = text
	var op = &rip[line][STONE_LINE_CENT - ((strlen(text)+1)>>1)]
	while(*ip) *op++ = *ip++
***/
}


private def genl_outrip(tmpwin: winid, how: Int): Unit =
{
/*** MOTODO: Convert
	register char **dp;
	register char *dpx;
	char buf[BUFSZ];
	var x = 0
	int line;

	rip = dp = (char **) alloc(sizeof(rip_txt));
	for (x = 0; rip_txt(x); x++) {
		dp[x] = (char *) alloc((unsigned int)(strlen(rip_txt[x]) + 1));
		Strcpy(dp[x], rip_txt[x]);
	}
	dp(x) = null

	/* Put name on stone */
	Sprintf(buf, "%s", plname);
	buf[STONE_LINE_LEN] = 0;
	center(NAME_LINE, buf);

	/* Put $ on stone */
	Sprintf(buf, "%ld Au", u.ugold);
	buf[STONE_LINE_LEN] = 0; /* It could be a *lot* of gold :-) */
	center(GOLD_LINE, buf);

	/* Put together death description */
	switch (killer_format) {
		default: warning("bad killer format?");
		case KILLED_BY_AN:
			Strcpy(buf, killed_by_prefix[how]);
			Strcat(buf, an(killer));
			break;
		case KILLED_BY:
			Strcpy(buf, killed_by_prefix[how]);
			Strcat(buf, killer);
			break;
		case NO_KILLER_PREFIX:
			Strcpy(buf, killer);
			break;
	}

	/* Put death type on stone */
	for (line=DEATH_LINE, dpx = buf; line<YEAR_LINE; line++) {
		register int i,i0;
		char tmpchar;

		if ( (i0=strlen(dpx)) > STONE_LINE_LEN) {
				for(i = STONE_LINE_LEN;
				    ((i0 > STONE_LINE_LEN) && i); i--)
					if(dpx[i] == ' ') i0 = i;
				if(!i) i0 = STONE_LINE_LEN;
		}
		tmpchar = dpx[i0];
		dpx[i0] = 0;
		center(line, dpx);
		if (tmpchar != ' ') {
			dpx[i0] = tmpchar;
			dpx= &dpx[i0];
		} else  dpx= &dpx[i0+1];
	}

	/* Put year on stone */
	Sprintf(buf, "%4d", getyear());
	center(YEAR_LINE, buf);

	putstr(tmpwin, 0, "");
	for(; *dp; dp++)
		putstr(tmpwin, 0, *dp);

	putstr(tmpwin, 0, "");
	putstr(tmpwin, 0, "");
***/
	rip = null
}
}
