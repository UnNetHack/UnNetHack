#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursstat.h"

/* Status window functions for curses interface */

/* Private declarations */

typedef struct nhs
{
    long value;
    char *txt;
    aligntyp alignment;
    boolean display;
    int highlight_turns;
    int highlight_color;
    int x;
    int y;
    char *label;
} nhstat;

static void init_stats(void);

static void set_labels(int label_width);

static nhstat prevname;
static nhstat prevdepth;
static nhstat prevstr;
static nhstat prevint;
static nhstat prevwis;
static nhstat prevdex;
static nhstat prevcon;
static nhstat prevcha;
static nhstat prevalign;  
static nhstat prevau;
static nhstat prevhp;
static nhstat prevmhp;
static nhstat prevlevel;
static nhstat prevpow;
static nhstat prevmpow;
static nhstat prevac;
static nhstat prevexp;
static nhstat prevtime;
#ifdef SCORE_ON_BOTL
static nhstat prevscore;
#endif
static nhstat prevhunger;
static nhstat prevconf;
static nhstat prevblind;
static nhstat prevstun;
static nhstat prevhallu;
static nhstat prevsick;
static nhstat prevslime;
static nhstat prevencumb;

#define COMPACT_LABELS  1
#define NORMAL_LABELS   2
#define WIDE_LABELS     3

extern const char *hu_stat[]; /* from eat.c */
extern const char *enc_stat[]; /* from botl.c */

/* Update the status win - this is called when NetHack would normally
write to the status window, so we know somwthing has changed.  We
override the write and update what needs to be updated ourselves. */

void curses_update_stats()
{
    char buf[BUFSZ];
    int count, enc, orient, sx_start;
    WINDOW *win = curses_get_nhwin(STATUS_WIN);
    static boolean first = TRUE;
    static boolean prev_horiz;
    boolean horiz;
    int sx = 0;
    int sy = 0;
    boolean border = curses_window_has_border(STATUS_WIN);
    
    if (border)
    {
        sx++;
        sy++;
    }
    
    sx_start = sx;
        
    orient = curses_get_window_orientation(STATUS_WIN);

    if ((orient == ALIGN_RIGHT) || (orient == ALIGN_LEFT))
    {
        horiz = FALSE;
    }
    else
    {
        horiz = TRUE;
    }
    
    if (first)
    {
        init_stats();
        prev_horiz = !horiz;
        first = FALSE;
    }
    
    if (prev_horiz != horiz)    /* Status window orientation changed */
    {
        if (horiz)
        {
            set_labels(NORMAL_LABELS);
    }
    else
    {
            set_labels(WIDE_LABELS);
        }
    }

    prev_horiz = horiz;
    
    curses_clear_nhwin(STATUS_WIN);
    
    /* Line 1 */
    
    /* Player name and title */
    strcpy(buf, plname);
    if ('a' <= buf[0] && buf[0] <= 'z') buf[0] += 'A'-'a';
    strcat(buf, " the ");
    if (u.mtimedone) {
        char mname[BUFSZ];
        int k = 0;

        strcpy(mname, mons[u.umonnum].mname);
        while(mname[k] != 0) {
            if ((k == 0 || (k > 0 && mname[k-1] == ' '))
             && 'a' <= mname[k] && mname[k] <= 'z')
            {
                mname[k] += 'A' - 'a';
            }
            k++;
        }
        strcat(buf, mname);
    } else {
        strcat(buf, rank_of(u.ulevel, pl_character[0], flags.female));
    }
    
    if (strcmp(buf, prevname.txt) != 0) /* Title changed */
    {
        prevname.highlight_turns = 5;
        prevname.highlight_color = HIGHLIGHT_COLOR;
        free(prevname.txt);
        prevname.txt = curses_copy_of(buf);
    }
    
    if (prevname.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevname.label);
        sx += strlen(prevname.label);
    }
    
    if (prevname.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevname.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevname.txt);
        curses_toggle_color_attr(win, prevname.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevname.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevname.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }
    
    /* Add dungeon name and level if status window is vertical */
    if (!horiz)
    {
        sprintf(buf, "%s", dungeons[u.uz.dnum].dname);
        mvwaddstr(win, sy, sx, buf);
        sy += 2;
    }

    /* Strength */
    if (ACURR(A_STR) != prevstr.value)  /* Strength changed */
    {
        
        if (ACURR(A_STR) > prevstr.value)
        {
            prevstr.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevstr.highlight_color = STAT_DOWN_COLOR;
        }
        prevstr.value = ACURR(A_STR);
        if (ACURR(A_STR) > 118)
        {
            sprintf(buf, "%d", ACURR(A_STR) - 100);
        }
        else if (ACURR(A_STR)==118)
        {
            sprintf(buf, "18/**");
        }
        else if(ACURR(A_STR) > 18)
        {
            sprintf(buf, "18/%02d", ACURR(A_STR) - 18);
        }
        else
        {
            sprintf(buf, "%d", ACURR(A_STR));
        }
        free(prevstr.txt);
        prevstr.txt = curses_copy_of(buf);
        prevstr.highlight_turns = 5;
    }

    if (prevstr.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevstr.label);
        sx += strlen(prevstr.label);
    }
    
    if (prevstr.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevstr.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevstr.txt);
        curses_toggle_color_attr(win, prevstr.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevstr.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevstr.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Intelligence */
    if (ACURR(A_INT) != prevint.value)  /* Intelligence changed */
    {
        
        if (ACURR(A_INT) > prevint.value)
        {
            prevint.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevint.highlight_color = STAT_DOWN_COLOR;
        }
        prevint.value = ACURR(A_INT);
        sprintf(buf, "%d", ACURR(A_INT)); 
        free(prevint.txt);
        prevint.txt = curses_copy_of(buf);
        prevint.highlight_turns = 5;
    }

    if (prevint.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevint.label);
        sx += strlen(prevint.label);
    }
    
    if (prevint.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevint.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevint.txt);
        curses_toggle_color_attr(win, prevint.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevint.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevint.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Wisdom */
    if (ACURR(A_WIS) != prevwis.value)  /* Wisdom changed */
    {
        
        if (ACURR(A_WIS) > prevwis.value)
        {
            prevwis.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevwis.highlight_color = STAT_DOWN_COLOR;
        }
        prevwis.value = ACURR(A_WIS);
        sprintf(buf, "%d", ACURR(A_WIS)); 
        free(prevwis.txt);
        prevwis.txt = curses_copy_of(buf);
        prevwis.highlight_turns = 5;
    }

    if (prevwis.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevwis.label);
        sx += strlen(prevwis.label);
    }
    
    if (prevwis.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevwis.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevwis.txt);
        curses_toggle_color_attr(win, prevwis.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevwis.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevwis.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Dexterity */
    if (ACURR(A_DEX) != prevdex.value)  /* Dexterity changed */
    {
        
        if (ACURR(A_DEX) > prevdex.value)
        {
            prevdex.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevdex.highlight_color = STAT_DOWN_COLOR;
        }
        prevdex.value = ACURR(A_DEX);
        sprintf(buf, "%d", ACURR(A_DEX));
        free(prevdex.txt);
        prevdex.txt = curses_copy_of(buf);
        prevdex.highlight_turns = 5;
    }

    if (prevdex.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevdex.label);
        sx += strlen(prevdex.label);
    }
    
    if (prevdex.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevdex.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevdex.txt);
        curses_toggle_color_attr(win, prevdex.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevdex.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevdex.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Constitution */
    if (ACURR(A_CON) != prevcon.value)  /* Constitution changed */
    {
        
        if (ACURR(A_CON) > prevcon.value)
        {
            prevcon.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevcon.highlight_color = STAT_DOWN_COLOR;
        }
        prevcon.value = ACURR(A_CON);
        sprintf(buf, "%d", ACURR(A_CON));
        free(prevcon.txt);
        prevcon.txt = curses_copy_of(buf);
        prevcon.highlight_turns = 5;
    }

    if (prevcon.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevcon.label);
        sx += strlen(prevcon.label);
    }
    
    if (prevcon.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevcon.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevcon.txt);
        curses_toggle_color_attr(win, prevcon.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevcon.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevcon.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Charisma */
    if (ACURR(A_CHA) != prevcha.value)  /* Charisma changed */
    {
        if (ACURR(A_CHA) > prevcha.value)
        {
            prevcha.highlight_color = STAT_UP_COLOR;
        }
        else
        {
            prevcha.highlight_color = STAT_DOWN_COLOR;
        }
        prevcha.value = ACURR(A_CHA);
        sprintf(buf, "%d", ACURR(A_CHA));
        free(prevcha.txt);
        prevcha.txt = curses_copy_of(buf);
        prevcha.highlight_turns = 5;
    }

    if (prevcha.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevcha.label);
        sx += strlen(prevcha.label);
    }
    
    if (prevcha.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevcha.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevcha.txt);
        curses_toggle_color_attr(win, prevcha.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevcha.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevcha.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy ++;
    }
    
    /* Alignment */
    if (prevalign.alignment != u.ualign.type)   /* Alignment changed */
    {
        prevalign.highlight_color = HIGHLIGHT_COLOR;
        prevalign.highlight_turns = 10; /* This is a major change! */
        prevalign.alignment = u.ualign.type;
        free(prevalign.txt);
        switch (u.ualign.type)
        {
            case A_LAWFUL:
            {
                prevalign.txt = curses_copy_of("Lawful");
                break;
            }
            case A_NEUTRAL:
            {
                prevalign.txt = curses_copy_of("Neutral");
                break;
            }
            case A_CHAOTIC:
            {
                prevalign.txt = curses_copy_of("Chaotic");
                break;
            }
        }
    }

    if (prevalign.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevalign.label);
        sx += strlen(prevalign.label);
    }
    
    if (prevalign.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevalign.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevalign.txt);
        curses_toggle_color_attr(win, prevalign.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevalign.txt);
    }
    
    /* Line 2 */
    
    sx = sx_start;
    sy++;
    
    /* Dungeon Level */
    if (depth(&u.uz) != prevdepth.value)    /* Dungeon level changed */
    {
        prevdepth.highlight_color = HIGHLIGHT_COLOR;
        prevdepth.highlight_turns = 5;
        prevdepth.value = depth(&u.uz);
        free(prevdepth.txt);
        if (In_endgame(&u.uz))
        {
            strcpy(buf, (Is_astralevel(&u.uz) ? "Astral Plane":"End Game"));
        }
        else
        {
            sprintf(buf, "%d", depth(&u.uz));
        }
        prevdepth.txt = curses_copy_of(buf);
    }
    
    if (prevdepth.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevdepth.label);
        sx += strlen(prevdepth.label);
    }
    
    if (prevdepth.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevdepth.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevdepth.txt);
        curses_toggle_color_attr(win, prevdepth.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevdepth.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevdepth.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }
    
    /* Gold */
#ifndef GOLDOBJ
    if (prevau.value != u.ugold)    /* Gold changed */
    {
        if (u.ugold > prevau.value)
        {
#else
    if (prevau.value != money_cnt(invent))  /* Gold changed */
    {
        if (money_cnt(invent) > prevau.value)
        {
#endif
            prevau.highlight_color = HI_GOLD;
        }
        else
        {
            prevau.highlight_color = STAT_DOWN_COLOR;
        }
#ifndef GOLDOBJ
        prevau.value = u.ugold;
        sprintf(buf,"%ld", u.ugold);
#else
        prevau.value = money_cnt(invent);
        sprintf(buf,"%ld", money_cnt(invent));
#endif
        free(prevau.txt);
        prevau.txt = curses_copy_of(buf);
        prevau.highlight_turns = 5;
    }
    
    if (prevau.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevau.label);
        sx += strlen(prevau.label);
    }
    
    if (prevau.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevau.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevau.txt);
        curses_toggle_color_attr(win, prevau.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevau.txt);
    }

    if (horiz)
    {
        sx += strlen(prevau.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }
    
    /* Hit Points */
    if (u.mtimedone)    /* Currently polymorphed - show monster HP */
    {
	    if (u.mh != prevhp.value)
	    {
	        if (u.mh > prevhp.value)
	        {
	            prevhp.highlight_color = STAT_UP_COLOR;
	        }
	        else
	        {
	            prevhp.highlight_color = STAT_DOWN_COLOR;
	        }
            prevhp.highlight_turns = 3;
            prevhp.value = u.mh;
            sprintf(buf, "%d", u.mh);
            free(prevhp.txt);
            prevhp.txt = curses_copy_of(buf);
	    }
	}
	else if (u.uhp != prevhp.value)  /* Not polymorphed */
	{
	    if (u.uhp > prevhp.value)
	    {
	        prevhp.highlight_color = STAT_UP_COLOR;
	    }
	    else
	    {
            prevhp.highlight_color = STAT_DOWN_COLOR;
	    }
        prevhp.value = u.uhp;
        sprintf(buf, "%d", u.uhp);
        free(prevhp.txt);
        prevhp.txt = curses_copy_of(buf);
        prevhp.highlight_turns = 3;
	}

    if (prevhp.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevhp.label);
        sx += strlen(prevhp.label);
    }

    if (prevhp.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevhp.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevhp.txt);
        curses_toggle_color_attr(win, prevhp.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevhp.txt);
    }
    
    sx += strlen(prevhp.txt);

    /* Max Hit Points */
    if (u.mtimedone)    /* Currently polymorphed - show monster HP */
    {
	    if (u.mhmax != prevmhp.value)
	    {
	        if (u.mhmax > prevmhp.value)
	        {
	            prevmhp.highlight_color = STAT_UP_COLOR;
	        }
	        else
	        {
	            prevmhp.highlight_color = STAT_DOWN_COLOR;
	        }
            prevmhp.value = u.mhmax;
            sprintf(buf, "%d", u.mhmax);
            free(prevmhp.txt);
            prevmhp.txt = curses_copy_of(buf);
            prevmhp.highlight_turns = 3;
	    }
	}
	else if (u.uhpmax != prevmhp.value)  /* Not polymorphed */
	{
	    if (u.uhpmax > prevmhp.value)
	    {
	        prevmhp.highlight_color = STAT_UP_COLOR;
	    }
	    else
	    {
            prevmhp.highlight_color = STAT_DOWN_COLOR;
	    }
        prevmhp.value = u.uhpmax;
        sprintf(buf, "%d", u.uhpmax);
        free(prevmhp.txt);
        prevmhp.txt = curses_copy_of(buf);
        prevmhp.highlight_turns = 3;
	}

    if (prevmhp.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevmhp.label);
        sx += strlen(prevmhp.label);
    }

    if (prevmhp.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevmhp.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevmhp.txt);
        curses_toggle_color_attr(win, prevmhp.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevmhp.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevmhp.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Power */
    if (u.uen != prevpow.value)
	{
	    if (u.uen > prevpow.value)
	    {
	        prevpow.highlight_color = STAT_UP_COLOR;
	    }
	    else
	    {
            prevpow.highlight_color = STAT_DOWN_COLOR;
	    }
        prevpow.value = u.uen;
        sprintf(buf, "%d", u.uen);
        free(prevpow.txt);
        prevpow.txt = curses_copy_of(buf);
        prevpow.highlight_turns = 3;
	}

    if (prevpow.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevpow.label);
        sx += strlen(prevpow.label);
    }

    if (prevpow.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevpow.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevpow.txt);
        curses_toggle_color_attr(win, prevpow.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevpow.txt);
    }
    
    sx += strlen(prevpow.txt);

    /* Max Power */
    if (u.uenmax != prevmpow.value)
	{
	    if (u.uenmax > prevmpow.value)
	    {
	        prevmpow.highlight_color = STAT_UP_COLOR;
	    }
	    else
	    {
            prevmpow.highlight_color = STAT_DOWN_COLOR;
	    }
        prevmpow.value = u.uenmax;
        sprintf(buf, "%d", u.uenmax);
        free(prevmpow.txt);
        prevmpow.txt = curses_copy_of(buf);
        prevmpow.highlight_turns = 3;
	}

    if (prevmpow.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevmpow.label);
        sx += strlen(prevmpow.label);
    }

    if (prevmpow.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevmpow.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevmpow.txt);
        curses_toggle_color_attr(win, prevmpow.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevmpow.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevmpow.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }


    /* Armor Class */
    if (u.uac != prevac.value)
	{
	    if (u.uac > prevac.value)   /* Lower is better for AC */
	    {
	        prevac.highlight_color = STAT_DOWN_COLOR;
	    }
	    else
	    {
            prevac.highlight_color = STAT_UP_COLOR;
	    }
        prevac.value = u.uac;
        sprintf(buf, "%d", u.uac);
        free(prevac.txt);
        prevac.txt = curses_copy_of(buf);
        prevac.highlight_turns = 5;
	}

    if (prevac.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevac.label);
        sx += strlen(prevac.label);
    }

    if (prevac.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevac.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevac.txt);
        curses_toggle_color_attr(win, prevac.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevac.txt);
    }
    
    if (horiz)
    {
        sx += strlen(prevac.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Experience */
#ifdef EXP_ON_BOTL
    if (prevexp.display != flags.showexp)   /* Setting has changed */
    {
        prevexp.display = flags.showexp;
        free(prevlevel.label);
        if (prevexp.display)
        {
            prevlevel.label = curses_copy_of("/");
        }
        else
        {
            if (horiz)
            {
            prevlevel.label = curses_copy_of("Lvl:");
        }
            else
            {
                prevlevel.label = curses_copy_of("Level:         ");
            }
        }
    }

    if (prevexp.display && !u.mtimedone)
    {
        if (u.uexp != prevexp.value)
	    {
	        if (u.uexp > prevexp.value)
	        {
	            prevexp.highlight_color = STAT_UP_COLOR;
	        }
	        else
	        {
                prevexp.highlight_color = STAT_DOWN_COLOR;
	        }
            sprintf(buf, "%ld", u.uexp);
            free(prevexp.txt);
            prevexp.txt = curses_copy_of(buf);
            prevexp.highlight_turns = 3;
	    }

        if (prevexp.label != NULL)
        {
            mvwaddstr(win, sy, sx, prevexp.label);
            sx += strlen(prevexp.label);
        }

        if (prevexp.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevexp.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevexp.txt);
            curses_toggle_color_attr(win, prevexp.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevexp.txt);
        }

        sx += strlen(prevexp.txt);
    }
    
    prevexp.value = u.uexp; /* Track it even when it's not displayed */
#endif  /* EXP_ON_BOTL */

    /* Level */
    if (u.mtimedone)    /* Currently polymorphed - show monster HD */
    {
        if ((strncmp(prevlevel.label, "HP:", 3) != 0) ||
         (strncmp(prevlevel.label, "Hit Points:", 11) != 0))
        {
            free(prevlevel.label);
            if (horiz)
            {
            prevlevel.label = curses_copy_of("HD:");
        }
            else
            {
                prevlevel.label = curses_copy_of("Hit Dice:      ");
            }
        }
        if (mons[u.umonnum].mlevel != prevlevel.value)
        {
            if (mons[u.umonnum].mlevel > prevlevel.value)
            {
                prevlevel.highlight_color = STAT_UP_COLOR;
            }
            else
            {
                prevlevel.highlight_color = STAT_DOWN_COLOR;
            }
            prevlevel.highlight_turns = 5;
        }
        prevlevel.value = mons[u.umonnum].mlevel;
        sprintf(buf, "%d", mons[u.umonnum].mlevel);
        free(prevlevel.txt);
        prevlevel.txt = curses_copy_of(buf);
    }
    else    /* Not polymorphed */
    {
        if ((strncmp(prevlevel.label, "HD:", 3) != 0) ||
         (strncmp(prevlevel.label, "Hit Dice:", 9) != 0))
        {
            free(prevlevel.label);
            if (prevexp.display)
            {
                prevlevel.label = curses_copy_of("/");
            }
            else
            {
                if (horiz)
                {
                prevlevel.label = curses_copy_of("Lvl:");
            }
                else
                {
                    prevlevel.label = curses_copy_of("Level:         ");
                }
            }
        }
        if (u.ulevel > prevlevel.value)
        {
            prevlevel.highlight_color = STAT_UP_COLOR;
            prevlevel.highlight_turns = 5;
        }
        else if (u.ulevel < prevlevel.value)
        {
            prevlevel.highlight_color = STAT_DOWN_COLOR;
            prevlevel.highlight_turns = 5;
        }
        prevlevel.value = u.ulevel;
        sprintf(buf, "%d", u.ulevel);
        free(prevlevel.txt);
        prevlevel.txt = curses_copy_of(buf);
    }

    if (prevlevel.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevlevel.label);
        sx += strlen(prevlevel.label);
    }

    if (prevlevel.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevlevel.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevlevel.txt);
        curses_toggle_color_attr(win, prevlevel.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevlevel.txt);
    }

    if (horiz)
    {
        sx += strlen(prevlevel.txt) + 1;
    }
    else
    {
        sx = sx_start;
        sy++;
    }

    /* Time */
    if (prevtime.display != flags.time)   /* Setting has changed */
    {
        prevtime.display = flags.time;
    }
    if (prevtime.display)
    {
        if (moves != prevtime.value)
	    {
            sprintf(buf, "%ld", moves);
            free(prevtime.txt);
            prevtime.txt = curses_copy_of(buf);
	    }

        if (prevtime.label != NULL)
        {
            mvwaddstr(win, sy, sx, prevtime.label);
            sx += strlen(prevtime.label);
        }

        mvwaddstr(win, sy, sx, prevtime.txt);

        if (horiz)
        {
            sx += strlen(prevtime.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }
    
    /* Score */
#ifdef SCORE_ON_BOTL
    if (prevscore.display != flags.showscore)   /* Setting has changed */
    {
        prevscore.display = flags.showscore;
    }
    if (prevscore.display)
    {
        if (botl_score() != prevscore.value)
	    {
	        if (botl_score() > prevscore.value)
	        {
	            prevscore.highlight_color = STAT_UP_COLOR;
	        }
	        else    /* Not sure this is possible */
	        {
                prevscore.highlight_color = STAT_DOWN_COLOR;
	        }
            sprintf(buf, "%ld", botl_score());
            free(prevscore.txt);
            prevscore.txt = curses_copy_of(buf);
            prevscore.highlight_turns = 3;
	    }

        if (prevscore.label != NULL)
        {
            mvwaddstr(win, sy, sx, prevscore.label);
            sx += strlen(prevscore.label);
        }

        if (prevscore.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevscore.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevscore.txt);
            curses_toggle_color_attr(win, prevscore.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevscore.txt);
        }

        if (horiz)
        {
            sx += strlen(prevscore.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }
    
    prevscore.value = botl_score(); /* Track it even when it's not displayed */
#endif  /* SCORE_ON_BOTL */

    /* Hunger */
    if (u.uhs != prevhunger.value)
	{
	    if ((u.uhs > prevhunger.value) || (u.uhs > 3))
	    {
	        prevhunger.highlight_color = STAT_DOWN_COLOR;
	    }
	    else
	    {
            prevhunger.highlight_color = STAT_UP_COLOR;
	    }
        prevhunger.value = u.uhs;
        for (count = 0; count < strlen(hu_stat[u.uhs]); count++)
        {
            if ((hu_stat[u.uhs][count]) == ' ')
            {
                break;
            }
            buf[count] = hu_stat[u.uhs][count];
        }

        buf[count] = '\0';
        free(prevhunger.txt);
        prevhunger.txt = curses_copy_of(buf);
        prevhunger.highlight_turns = 5;
	}

    if (prevhunger.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevhunger.label);
        sx += strlen(prevhunger.label);
    }

    if (prevhunger.highlight_turns > 0)
    {
        curses_toggle_color_attr(win, prevhunger.highlight_color, NONE, ON);
        mvwaddstr(win, sy, sx, prevhunger.txt);
        curses_toggle_color_attr(win, prevhunger.highlight_color, NONE, OFF);
    }
    else
    {
        mvwaddstr(win, sy, sx, prevhunger.txt);
    }
    
    if (strlen(prevhunger.txt) > 0)
    {
        if (horiz)
        {
            sx += strlen(prevhunger.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Confusion */
    if (Confusion != prevconf.value)
	{
	    prevconf.highlight_color = STAT_DOWN_COLOR;
        if (prevconf.txt != NULL)
        {
            free(prevconf.txt);
        }
        if (Confusion)
        {
            prevconf.txt = curses_copy_of("Conf");
        }
        else
        {
            prevconf.txt = NULL;
        }
        if (prevconf.value == 0)
        {
            prevconf.highlight_turns = 5;
	    }
        prevconf.value = Confusion;
	}

    if (prevconf.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevconf.label);
        sx += strlen(prevconf.label);
    }

    if (prevconf.txt != NULL)
    {
        if (prevconf.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevconf.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevconf.txt);
            curses_toggle_color_attr(win, prevconf.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevconf.txt);
        }
    }

    if (prevconf.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevconf.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Blindness */
    if (Blind != prevblind.value)
	{
	    prevblind.highlight_color = STAT_DOWN_COLOR;
        if (prevblind.txt != NULL)
        {
            free(prevblind.txt);
        }
        if (Blind)
        {
            prevblind.txt = curses_copy_of("Blind");
        }
        else
        {
            prevblind.txt = NULL;
        }
        if (prevblind.value == 0)
        {
            prevblind.highlight_turns = 5;
	    }
        prevblind.value = Blind;
	}

    if (prevblind.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevblind.label);
        sx += strlen(prevblind.label);
    }

    if (prevblind.txt != NULL)
    {
        if (prevblind.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevblind.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevblind.txt);
            curses_toggle_color_attr(win, prevblind.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevblind.txt);
        }
    }

    if (prevblind.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevblind.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Stun */
    if (Stunned != prevstun.value)
	{
	    prevstun.highlight_color = STAT_DOWN_COLOR;
        if (prevstun.txt != NULL)
        {
            free(prevstun.txt);
        }
        if (Stunned)
        {
            prevstun.txt = curses_copy_of("Stun");
        }
        else
        {
            prevstun.txt = NULL;
        }
        if (prevstun.value == 0)
        {
            prevstun.highlight_turns = 5;
	    }
        prevstun.value = Stunned;
	}

    if (prevstun.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevstun.label);
        sx += strlen(prevstun.label);
    }

    if (prevstun.txt != NULL)
    {
        if (prevstun.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevstun.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevstun.txt);
            curses_toggle_color_attr(win, prevstun.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevstun.txt);
        }
    }

    if (prevstun.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevstun.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Hallucination */
    if (Hallucination != prevhallu.value)
	{
	    prevhallu.highlight_color = STAT_DOWN_COLOR;
        if (prevhallu.txt != NULL)
        {
            free(prevhallu.txt);
        }
        if (Hallucination)
        {
            prevhallu.txt = curses_copy_of("Hallu");
        }
        else
        {
            prevhallu.txt = NULL;
        }
        if (prevhallu.value == 0)
        {
            prevhallu.highlight_turns = 5;
	    }
        prevhallu.value = Hallucination;
	}

    if (prevhallu.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevhallu.label);
        sx += strlen(prevhallu.label);
    }

    if (prevhallu.txt != NULL)
    {
        if (prevhallu.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevhallu.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevhallu.txt);
            curses_toggle_color_attr(win, prevhallu.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevhallu.txt);
        }
    }

    if (prevhallu.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevhallu.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Sick */
    if (Sick != prevsick.value)
	{
	    prevsick.highlight_color = STAT_DOWN_COLOR;
        if (prevsick.txt != NULL)
        {
            free(prevsick.txt);
        }
        if (Sick)
        {
            if (u.usick_type & SICK_VOMITABLE)
            {
                prevsick.txt = curses_copy_of("FoodPois");
            }
            else      
            {
                prevsick.txt = curses_copy_of("Ill");
            }
        }
        else
        {
            prevsick.txt = NULL;
        }
        if (prevsick.value == 0)
        {
            prevsick.highlight_turns = 5;
	    }
        prevsick.value = Sick;
	}

    if (prevsick.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevsick.label);
        sx += strlen(prevsick.label);
    }

    if (prevsick.txt != NULL)
    {
        if (prevsick.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevsick.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevsick.txt);
            curses_toggle_color_attr(win, prevsick.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevsick.txt);
        }
    }

    if (prevsick.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevsick.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Slime */
    if (Slimed != prevslime.value)
	{
	    prevslime.highlight_color = STAT_DOWN_COLOR;
        if (prevslime.txt != NULL)
        {
            free(prevslime.txt);
        }
        if (Slimed)
        {
            prevslime.txt = curses_copy_of("Slime");
        }
        else
        {
            prevslime.txt = NULL;
        }
        if (prevslime.value == 0)
        {
            prevslime.highlight_turns = 5;
	    }
        prevslime.value = Slimed;
	}

    if (prevslime.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevslime.label);
        sx += strlen(prevslime.label);
    }

    if (prevslime.txt != NULL)
    {
        if (prevslime.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevslime.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevslime.txt);
            curses_toggle_color_attr(win, prevslime.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevslime.txt);
        }
    }

    if (prevslime.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevslime.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    /* Encumberance */
    enc = near_capacity();
    
    if (enc != prevencumb.value)
	{
	    if (enc < prevencumb.value)
	    {
	        prevencumb.highlight_color = STAT_UP_COLOR;
	    }
	    else
	    {
	        prevencumb.highlight_color = STAT_DOWN_COLOR;
        }
        if (prevencumb.txt != NULL)
        {
            free(prevencumb.txt);
        }
        if (enc > UNENCUMBERED)
        {
            sprintf(buf, "%s", enc_stat[enc]);
            prevencumb.txt = curses_copy_of(buf);
            prevencumb.highlight_turns = 5;
        }
        else
        {
            prevencumb.txt = NULL;
        }
        prevencumb.value = enc;
	}

    if (prevencumb.label != NULL)
    {
        mvwaddstr(win, sy, sx, prevencumb.label);
        sx += strlen(prevencumb.label);
    }

    if (prevencumb.txt != NULL)
    {
        if (prevencumb.highlight_turns > 0)
        {
            curses_toggle_color_attr(win, prevencumb.highlight_color, NONE, ON);
            mvwaddstr(win, sy, sx, prevencumb.txt);
            curses_toggle_color_attr(win, prevencumb.highlight_color, NONE, OFF);
        }
        else
        {
            mvwaddstr(win, sy, sx, prevencumb.txt);
        }
    }

    if (prevencumb.txt != NULL)
    {
        if (horiz)
        {
            sx += strlen(prevencumb.txt) + 1;
        }
        else
        {
            sx = sx_start;
            sy++;
        }
    }

    wrefresh(win);
}


/* Decrement the highlight_turns for all stats.  Call curses_update_stats
if needed to unhighlight a stat */

void curses_decrement_highlight()
{
    boolean unhighlight = FALSE;
    
    if (prevname.highlight_turns > 0)
    {
        prevname.highlight_turns--;
        if (prevname.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevdepth.highlight_turns > 0)
    {
        prevdepth.highlight_turns--;
        if (prevdepth.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevstr.highlight_turns > 0)
    {
        prevstr.highlight_turns--;
        if (prevstr.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevint.highlight_turns > 0)
    {
        prevint.highlight_turns--;
        if (prevint.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevwis.highlight_turns > 0)
    {
        prevwis.highlight_turns--;
        if (prevwis.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevdex.highlight_turns > 0)
    {
        prevdex.highlight_turns--;
        if (prevdex.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevcon.highlight_turns > 0)
    {
        prevcon.highlight_turns--;
        if (prevcon.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevcha.highlight_turns > 0)
    {
        prevcha.highlight_turns--;
        if (prevcha.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevalign.highlight_turns > 0)
    {
        prevalign.highlight_turns--;
        if (prevalign.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevau.highlight_turns > 0)
    {
        prevau.highlight_turns--;
        if (prevau.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevhp.highlight_turns > 0)
    {
        prevhp.highlight_turns--;
        if (prevhp.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevmhp.highlight_turns > 0)
    {
        prevmhp.highlight_turns--;
        if (prevmhp.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevlevel.highlight_turns > 0)
    {
        prevlevel.highlight_turns--;
        if (prevlevel.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevpow.highlight_turns > 0)
    {
        prevpow.highlight_turns--;
        if (prevpow.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevmpow.highlight_turns > 0)
    {
        prevmpow.highlight_turns--;
        if (prevmpow.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevac.highlight_turns > 0)
    {
        prevac.highlight_turns--;
        if (prevac.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
#ifdef EXP_ON_BOTL
    if (prevexp.highlight_turns > 0)
    {
        prevexp.highlight_turns--;
        if (prevexp.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
#endif
    if (prevtime.highlight_turns > 0)
    {
        prevtime.highlight_turns--;
        if (prevtime.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
#ifdef SCORE_ON_BOTL
    if (prevscore.highlight_turns > 0)
    {
        prevscore.highlight_turns--;
        if (prevscore.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
#endif
    if (prevhunger.highlight_turns > 0)
    {
        prevhunger.highlight_turns--;
        if (prevhunger.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevconf.highlight_turns > 0)
    {
        prevconf.highlight_turns--;
        if (prevconf.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevblind.highlight_turns > 0)
    {
        prevblind.highlight_turns--;
        if (prevblind.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevstun.highlight_turns > 0)
    {
        prevstun.highlight_turns--;
        if (prevstun.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevhallu.highlight_turns > 0)
    {
        prevhallu.highlight_turns--;
        if (prevhallu.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevsick.highlight_turns > 0)
    {
        prevsick.highlight_turns--;
        if (prevsick.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevslime.highlight_turns > 0)
    {
        prevslime.highlight_turns--;
        if (prevslime.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    if (prevencumb.highlight_turns > 0)
    {
        prevencumb.highlight_turns--;
        if (prevencumb.highlight_turns == 0)
        {
            unhighlight = TRUE;
        }
    }
    
    if (unhighlight)
    {
        curses_update_stats();
    }
}


/* Initialize the stats with beginning values. */

static void init_stats()
{
    char buf[BUFSZ];
    int count;

    /* Player name and title */
    strcpy(buf, plname);
    if ('a' <= buf[0] && buf[0] <= 'z') buf[0] += 'A'-'a';
    strcat(buf, " the ");
    if (u.mtimedone) {
        char mname[BUFSZ];
        int k = 0;

        strcpy(mname, mons[u.umonnum].mname);
        while(mname[k] != 0) {
            if ((k == 0 || (k > 0 && mname[k-1] == ' '))
             && 'a' <= mname[k] && mname[k] <= 'z')
            {
                mname[k] += 'A' - 'a';
            }
            k++;
        }
        strcat(buf, mname);
    } else {
        strcat(buf, rank_of(u.ulevel, pl_character[0], flags.female));
    }

    prevname.txt = curses_copy_of(buf);
    prevname.display = TRUE;
    prevname.highlight_turns = 0;
    prevname.label = NULL;
    
    /* Strength */
    if (ACURR(A_STR) > 118)
    {
        sprintf(buf, "%d", ACURR(A_STR) - 100);
    }
    else if (ACURR(A_STR)==118)
    {
        sprintf(buf, "18/**");
    }
    else if(ACURR(A_STR) > 18)
    {
        sprintf(buf, "18/%02d", ACURR(A_STR) - 18);
    }
    else
    {
        sprintf(buf, "%d", ACURR(A_STR));
    }

    prevstr.value = ACURR(A_STR);
    prevstr.txt = curses_copy_of(buf);
    prevstr.display = TRUE;
    prevstr.highlight_turns = 0;
    prevstr.label = NULL;

    /* Intelligence */
    sprintf(buf, "%d", ACURR(A_INT));
    prevint.value = ACURR(A_INT);
    prevint.txt = curses_copy_of(buf);
    prevint.display = TRUE;
    prevint.highlight_turns = 0;
    prevint.label = NULL;

    /* Wisdom */
    sprintf(buf, "%d", ACURR(A_WIS));
    prevwis.value = ACURR(A_WIS);
    prevwis.txt = curses_copy_of(buf);
    prevwis.display = TRUE;
    prevwis.highlight_turns = 0;
    prevwis.label = NULL;

    /* Dexterity */
    sprintf(buf, "%d", ACURR(A_DEX));
    prevdex.value = ACURR(A_DEX);
    prevdex.txt = curses_copy_of(buf);
    prevdex.display = TRUE;
    prevdex.highlight_turns = 0;
    prevdex.label = NULL;

    /* Constitution */
    sprintf(buf, "%d", ACURR(A_CON));
    prevcon.value = ACURR(A_CON);
    prevcon.txt = curses_copy_of(buf);
    prevcon.display = TRUE;
    prevcon.highlight_turns = 0;
    prevcon.label = NULL;

    /* Charisma */
    sprintf(buf, "%d", ACURR(A_CHA));
    prevcha.value = ACURR(A_CHA);
    prevcha.txt = curses_copy_of(buf);
    prevcha.display = TRUE;
    prevcha.highlight_turns = 0;
    prevcha.label = NULL;

    /* Alignment */
    switch (u.ualign.type)
    {
        case A_LAWFUL:
        {
            prevalign.txt = curses_copy_of("Lawful");
            break;
        }
        case A_NEUTRAL:
        {
            prevalign.txt = curses_copy_of("Neutral");
            break;
        }
        case A_CHAOTIC:
        {
            prevalign.txt = curses_copy_of("Chaotic");
            break;
        }
    }
    
    prevalign.alignment = u.ualign.type;
    prevalign.display = TRUE;
    prevalign.highlight_turns = 0;
    prevalign.label = NULL;
    
    /* Dungeon level */
    if (In_endgame(&u.uz))
    {
        strcpy(buf, (Is_astralevel(&u.uz) ? "Astral Plane":"End Game"));
    }
    else
    {
        sprintf(buf, "%d", depth(&u.uz));
    }

    prevdepth.value = depth(&u.uz);
    prevdepth.txt = curses_copy_of(buf);
    prevdepth.display = TRUE;
    prevdepth.highlight_turns = 0;
    prevdepth.label = NULL;
    
    /* Gold */
#ifndef GOLDOBJ
    sprintf(buf,"%ld", u.ugold);
    prevau.value = u.ugold;
#else
    sprintf(buf,"%ld", money_cnt(invent));
    prevau.value = money_cnt(invent);
#endif
    prevau.txt = curses_copy_of(buf);
    prevau.display = TRUE;
    prevau.highlight_turns = 0;
    prevau.label = NULL;

    /* Hit Points */
    if (u.mtimedone)    /* Currently polymorphed - show monster HP */
    {
        prevhp.value = u.mh;
        sprintf(buf, "%d", u.mh);
        prevhp.txt = curses_copy_of(buf);
	}
	else if (u.uhp != prevhp.value)  /* Not polymorphed */
	{
	    prevhp.value = u.uhp;
        sprintf(buf, "%d", u.uhp);
        prevhp.txt = curses_copy_of(buf);
	}
	prevhp.display = TRUE;
	prevhp.highlight_turns = 0;
    prevhp.label = NULL;

    /* Max Hit Points */
    if (u.mtimedone)    /* Currently polymorphed - show monster HP */
    {
        prevmhp.value = u.mhmax;
        sprintf(buf, "%d", u.mhmax);
        prevmhp.txt = curses_copy_of(buf);
	}
	else    /* Not polymorphed */
	{
	    prevmhp.value = u.uhpmax;
        sprintf(buf, "%d", u.uhpmax);
        prevmhp.txt = curses_copy_of(buf);
	}
	prevmhp.display = TRUE;
	prevmhp.highlight_turns = 0;
    prevmhp.label = curses_copy_of("/");

    /* Power */
    prevpow.value = u.uen;
    sprintf(buf, "%d", u.uen);
    prevpow.txt = curses_copy_of(buf);
	prevpow.display = TRUE;
	prevpow.highlight_turns = 0;
    prevpow.label = NULL;

    /* Max Power */
    prevmpow.value = u.uenmax;
    sprintf(buf, "%d", u.uenmax);
    prevmpow.txt = curses_copy_of(buf);
	prevmpow.display = TRUE;
	prevmpow.highlight_turns = 0;
    prevmpow.label = curses_copy_of("/");

    /* Armor Class */
    prevac.value = u.uac;
    sprintf(buf, "%d", u.uac);
    prevac.txt = curses_copy_of(buf);
	prevac.display = TRUE;
	prevac.highlight_turns = 0;
    prevac.label = NULL;

    /* Experience */
#ifdef EXP_ON_BOTL
    prevexp.value = u.uexp;
    sprintf(buf, "%ld", u.uexp);
    prevexp.txt = curses_copy_of(buf);
	prevexp.display = flags.showexp;
	prevexp.highlight_turns = 0;
    prevexp.label = NULL;
#endif

    /* Level */
    prevlevel.label = NULL;
    if (u.mtimedone)    /* Currently polymorphed - show monster HP */
    {
        prevlevel.value = mons[u.umonnum].mlevel;
        sprintf(buf, "%d", mons[u.umonnum].mlevel);
        prevlevel.txt = curses_copy_of(buf);
	}
	else if (u.ulevel != prevlevel.value)  /* Not polymorphed */
	{
	    prevlevel.value = u.ulevel;
        sprintf(buf, "%d", u.ulevel);
        prevlevel.txt = curses_copy_of(buf);
	}
	prevlevel.display = TRUE;
	prevlevel.highlight_turns = 0;

    /* Time */
    prevtime.value = moves;
    sprintf(buf, "%ld", moves);
    prevtime.txt = curses_copy_of(buf);
	prevtime.display = flags.time;
	prevtime.highlight_turns = 0;
    prevtime.label = NULL;

    /* Score */
#ifdef SCORE_ON_BOTL
    prevscore.value = botl_score();
    sprintf(buf, "%ld", botl_score());
    prevscore.txt = curses_copy_of(buf);
	prevscore.display = flags.showscore;
	prevscore.highlight_turns = 0;
    prevscore.label = NULL;
#endif

    /* Hunger */
    prevhunger.value = u.uhs;
    for (count = 0; count < strlen(hu_stat[u.uhs]); count++)
    {
        if ((hu_stat[u.uhs][count]) == ' ')
        {
            break;
        }
        buf[count] = hu_stat[u.uhs][count];
    }

    buf[count] = '\0';
    prevhunger.txt = curses_copy_of(buf);
    prevhunger.display = TRUE;
    prevhunger.highlight_turns = 0;
    prevhunger.label = NULL;

    /* Confusion */
    prevconf.value = Confusion;
    if (Confusion)
    {
        prevconf.txt = curses_copy_of("Conf");
    }
    else
    {
        prevconf.txt = NULL;
    }
    prevconf.display = TRUE;
    prevconf.highlight_turns = 0;
    prevconf.label = NULL;

    /* Blindness */
    prevblind.value = Blind;
    if (Blind)
    {
        prevblind.txt = curses_copy_of("Blind");
    }
    else
    {
        prevblind.txt = NULL;
    }
    prevblind.display = TRUE;
    prevblind.highlight_turns = 0;
    prevblind.label = NULL;

    /* Stun */
    prevstun.value = Stunned;
    if (Stunned)
    {
        prevstun.txt = curses_copy_of("Stun");
    }
    else
    {
        prevstun.txt = NULL;
    }
    prevstun.display = TRUE;
    prevstun.highlight_turns = 0;
    prevstun.label = NULL;

    /* Hallucination */
    prevhallu.value = Hallucination;
    if (Hallucination)
    {
        prevhallu.txt = curses_copy_of("Hallu");
    }
    else
    {
        prevhallu.txt = NULL;
    }
    prevhallu.display = TRUE;
    prevhallu.highlight_turns = 0;
    prevhallu.label = NULL;

    /* Sick */
    prevsick.value = Sick;
    if (Sick)
    {
        if (u.usick_type & SICK_VOMITABLE)
        {
            prevsick.txt = curses_copy_of("Sick");
        }
        else     
        {
            prevsick.txt = curses_copy_of("Ill");
        }
    }
    else
    {
        prevsick.txt = NULL;
    }
    prevsick.display = TRUE;
    prevsick.highlight_turns = 0;
    prevsick.label = NULL;

    /* Slimed */
    prevslime.value = Slimed;
    if (Slimed)
    {
        prevslime.txt = curses_copy_of("Slime");
    }
    else
    {
        prevslime.txt = NULL;
    }
    prevslime.display = TRUE;
    prevslime.highlight_turns = 0;
    prevslime.label = NULL;

    /* Encumberance */
    prevencumb.value = near_capacity();
    if (prevencumb.value > UNENCUMBERED)
    {
        sprintf(buf, "%s", enc_stat[prevencumb.value]);
        prevencumb.txt = curses_copy_of(buf);
    }
    else
    {
        prevencumb.txt = NULL;
    }
    prevencumb.display = TRUE;
    prevencumb.highlight_turns = 0;
    prevencumb.label = NULL;
}

/* Set labels based on orientation of status window.  If horizontal,
we want to compress this info; otherwise we know we have a width of at
least 26 characters. */

static void set_labels(int label_width)
{
    char buf[BUFSZ];

    switch (label_width)
    {
        case COMPACT_LABELS:
        {
            if (prevstr.label)
            {
                free (prevstr.label);
            }
            prevstr.label = curses_copy_of("S:");
            break;
        }
        case NORMAL_LABELS:
        {
            /* Strength */
            if (prevstr.label)
            {
                free (prevstr.label);
            }
            prevstr.label = curses_copy_of("Str:");
            /* Intelligence */
            if (prevint.label)
            {
                free (prevint.label);
            }
            prevint.label = curses_copy_of("Int:");

            /* Wisdom */
            if (prevwis.label)
            {
                free (prevwis.label);
            }
            prevwis.label = curses_copy_of("Wis:");
            
            /* Dexterity */
            if (prevdex.label)
            {
                free (prevdex.label);
            }
            prevdex.label = curses_copy_of("Dex:");
            
            /* Constitution */
            if (prevcon.label)
            {
                free (prevcon.label);
            }
            prevcon.label = curses_copy_of("Con:");
            
            /* Charisma */
            if (prevcha.label)
            {
                free (prevcha.label);
            }
            prevcha.label = curses_copy_of("Cha:");
            
            /* Alignment */
            if (prevalign.label)
            {
                free (prevalign.label);
            }
            prevalign.label = NULL;
            
            /* Dungeon level */
            if (prevdepth.label)
            {
                free (prevdepth.label);
            }
            prevdepth.label = curses_copy_of("Dlvl:");
           
            /* Gold */
            if (prevau.label)
            {
                free (prevau.label);
            }
            sprintf(buf, "%c:", GOLD_SYM);
            prevau.label = curses_copy_of(buf);
            
            /* Hit points */
            if (prevhp.label)
            {
                free (prevhp.label);
            }
            prevhp.label = curses_copy_of("HP:");
            
            /* Power */
            if (prevpow.label)
            {
                free (prevpow.label);
            }
            prevpow.label = curses_copy_of("Pw:");
            
            /* Armor Class */
            if (prevac.label)
            {
                free (prevac.label);
            }
            prevac.label = curses_copy_of("AC:");
            
#ifdef EXP_ON_BOTL            
            /* Experience */
            if (prevexp.label)
            {
                free (prevexp.label);
            }
            prevexp.label = curses_copy_of("XP:");
#endif            

            /* Level */            
            if (prevlevel.label)
            {
                free (prevlevel.label);
                prevlevel.label = NULL;
            }
            if (u.mtimedone)    /* Currently polymorphed - show monster HP */
            {
                prevlevel.label = curses_copy_of("HD:");
        	}
        	else    /* Not polymorphed */
        	{
                if (prevexp.display)
                {
                    prevlevel.label = curses_copy_of("/");
                }
                else
                {    
                    prevlevel.label = curses_copy_of("Lvl:");
                }
            }
            
            /* Time */
            if (prevtime.label)
            {
                free (prevtime.label);
            }
            prevtime.label = curses_copy_of("T:");
            
#ifdef SCORE_ON_BOTL
            /* Score */
            if (prevscore.label)
            {
                free (prevscore.label);
            }
            prevscore.label = curses_copy_of("S:");
#endif
            break;
        }
        case WIDE_LABELS:
        {
            /* Strength */
            if (prevstr.label)
            {
                free (prevstr.label);
            }
            prevstr.label = curses_copy_of("Strength:      ");
            
            /* Intelligence */
            if (prevint.label)
            {
                free (prevint.label);
            }
            prevint.label = curses_copy_of("Intelligence:  ");
            
            /* Wisdom */
            if (prevwis.label)
            {
                free (prevwis.label);
            }
            prevwis.label = curses_copy_of("Wisdom:        ");
            
            /* Dexterity */
            if (prevdex.label)
            {
                free (prevdex.label);
            }
            prevdex.label = curses_copy_of("Dexterity:     ");
            
            /* Constitution */
            if (prevcon.label)
            {
                free (prevcon.label);
            }
            prevcon.label = curses_copy_of("Constitution:  ");
            
            /* Charisma */
            if (prevcha.label)
            {
                free (prevcha.label);
            }
            prevcha.label = curses_copy_of("Charisma:      ");
            
            /* Alignment */
            if (prevalign.label)
            {
                free (prevalign.label);
            }
            prevalign.label = curses_copy_of("Alignment:     ");
            
            /* Dungeon level */
            if (prevdepth.label)
            {
                free (prevdepth.label);
            }
            prevdepth.label = curses_copy_of("Dungeon Level: ");
           
            /* Gold */
            if (prevau.label)
            {
                free (prevau.label);
            }
            prevau.label = curses_copy_of("Gold:          ");
            
            /* Hit points */
            if (prevhp.label)
            {
                free (prevhp.label);
            }
            prevhp.label = curses_copy_of("Hit Points:    ");
            
            /* Power */
            if (prevpow.label)
            {
                free (prevpow.label);
            }
            prevpow.label = curses_copy_of("Magic Power:   ");
            
            /* Armor Class */
            if (prevac.label)
            {
                free (prevac.label);
            }
            prevac.label = curses_copy_of("Armor Class:   ");
            
#ifdef EXP_ON_BOTL            
            /* Experience */
            if (prevexp.label)
            {
                free (prevexp.label);
            }
            prevexp.label = curses_copy_of("Experience:   ");
#endif            

            /* Level */            
            if (prevlevel.label)
            {
                free (prevlevel.label);
            }
            if (u.mtimedone)    /* Currently polymorphed - show monster HP */
            {
                prevlevel.label = curses_copy_of("Hit Dice:      ");
        	}
        	else    /* Not polymorphed */
        	{
                if (prevexp.display)
                {
                    prevlevel.label = curses_copy_of(" / ");
                }
                else
                {    
                    prevlevel.label = curses_copy_of("Level:         ");
                }
            }
            
            /* Time */
            if (prevtime.label)
            {
                free (prevtime.label);
            }
            prevtime.label = curses_copy_of("Time:          ");
            
#ifdef SCORE_ON_BOTL
            /* Score */
            if (prevscore.label)
            {
                free (prevscore.label);
            }
            prevscore.label = curses_copy_of("Score:         ");
#endif
            break;
        }
        default:
        {
            panic( "set_labels(): Invalid lavel_width %d\n",
             label_width );
            break;
        }
    }
}
