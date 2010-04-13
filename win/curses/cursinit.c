#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursinit.h"

#include <ctype.h>

/* Initialization functions for curses interface */


/* Create the "main" nonvolitile windows used by nethack */

void curses_create_main_windows()
{
    int message_x = 0;
    int message_y = 0;
    int status_x = 0;
    int status_y = 0;
    int map_x = 0;
    int map_y = 0;
    int message_height = 0;
    int message_width = 0;
    int status_height = 0;
    int status_width = 0;
    int map_height = 0;
    int map_width = 0;
    int min_message_height = 1;
    int message_orientation = 0;
    int status_orientation = 0;
    int border_space = 0;
    int hspace = term_cols - 80;
    boolean borders = FALSE;

    switch (iflags.wc2_windowborders)
    {
        case 1: /* On */
        {
            borders = TRUE;
            break;
        }
        case 2: /* Off */
        {
            borders = FALSE;
            break;
        }
        case 3: /* Auto */
        {
            if ((term_cols > 81) && (term_rows > 25))
            {
                borders = TRUE;
            }
            break;
        }
        default:
        {
            borders = FALSE;
        }
    }

    
    if (borders)
    {
        border_space = 2;
        hspace -= border_space;
    }
    
    if ((term_cols - border_space) < COLNO)
    {
        min_message_height++;
    }
    
    /* Determine status window orientation */    
    if (!iflags.wc_align_status || (iflags.wc_align_status == ALIGN_TOP)
     || (iflags.wc_align_status == ALIGN_BOTTOM))
    {
        if (!iflags.wc_align_status)
        {
            iflags.wc_align_status = ALIGN_BOTTOM;
        }
        status_orientation = iflags.wc_align_status;
    }
    else    /* left or right alignment */
    {
        /* Max space for player name and title horizontally */
        if ((hspace >= 26) && (term_rows >= 24))
        {
            status_orientation = iflags.wc_align_status;
            hspace -= (26 + border_space);
        }
        else
        {
            status_orientation = ALIGN_BOTTOM;
        }
    }
    
    /* Determine message window orientation */    
    if (!iflags.wc_align_message || (iflags.wc_align_message == ALIGN_TOP)
     || (iflags.wc_align_message == ALIGN_BOTTOM))
    {
        if (!iflags.wc_align_message)
        {
            iflags.wc_align_message = ALIGN_TOP;
        }
        message_orientation = iflags.wc_align_message;
    }
    else    /* left or right alignment */
    {
        if ((hspace - border_space) >= 25)   /* Arbitrary */
        {
            message_orientation = iflags.wc_align_message;
        }
        else
        {
            message_orientation = ALIGN_TOP;
        }
    }
    
    /* Determine window placement and size - 16 possible combos
       If anyone wants to try to generalize this, be my guest! */
    if ((status_orientation == ALIGN_TOP) &&
     (message_orientation == ALIGN_TOP))
    {
        status_x = 0;
        status_y = 0;
        status_width = (term_cols - border_space);
        status_height = 2;
        message_x = 0;
        message_y = status_y + (status_height + border_space);
        message_width = (term_cols - border_space);
        message_height = term_rows - (status_height + ROWNO + (border_space * 3));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_x = 0;
        map_y = message_y + (message_height + border_space);
        map_width = (term_cols - border_space);
        map_height = term_rows - (status_height + message_height + (border_space * 3));
    }
    else if ((status_orientation == ALIGN_TOP) &&
     (message_orientation == ALIGN_RIGHT))
    {
        status_x = 0;
        status_y = 0;
        status_height = 2;
        message_height = (term_rows - border_space);
        message_width = term_cols - (COLNO + (border_space * 2));
        status_width = term_cols - (message_width + (border_space * 2));
        message_x = status_x + (status_width + border_space);
        message_y = 0;
        map_x = 0;
        map_y = status_y + (status_height + border_space);
        map_width = status_width;
        map_height = term_rows - (status_height + (border_space * 2));
    }
    else if ((status_orientation == ALIGN_TOP) &&
     (message_orientation == ALIGN_BOTTOM))
    {
        status_x = 0;
        status_y = 0;
        status_width = (term_cols - border_space);
        status_height = 2;
        map_x = 0;
        map_y = status_y + (status_height + border_space);
        map_width = (term_cols - border_space);
        message_height = term_rows - (status_height + ROWNO + (border_space * 3));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_height = term_rows - (status_height + message_height + (border_space * 3));
        message_x = 0;
        message_y = map_y + (map_height + border_space);
        message_width = (term_cols - border_space);
    }
    else if ((status_orientation == ALIGN_TOP) &&
     (message_orientation == ALIGN_LEFT))
    {
        message_x = 0;
        message_y = 0;
        message_height = (term_rows - border_space);
        message_width = term_cols - (COLNO + (border_space * 2));
        status_x = message_x + (message_width + border_space);
        status_y = 0;
        status_height = 2;
        status_width = term_cols - (message_width + (border_space * 2));
        map_x = status_x;
        map_y = status_y + (status_height + border_space);
        map_height = term_rows - (status_height + (border_space * 2));
        map_width = status_width;
    }
    if ((status_orientation == ALIGN_RIGHT) &&
     (message_orientation == ALIGN_TOP))
    {
        status_width = 26;
        status_height = (term_rows - border_space);
        status_x = term_cols - (status_width + border_space);
        status_y = 0;
        message_x = 0;
        message_y = 0;
        message_width = term_cols - (status_width + (border_space * 2));
        message_height = term_rows - (ROWNO + (border_space * 2));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_x = 0;
        map_y = message_y + (message_height + border_space);
        map_width = term_cols - (status_width + (border_space * 2));
        map_height = term_rows - (message_height + (border_space * 2));
    }
    else if ((status_orientation == ALIGN_RIGHT) &&
     (message_orientation == ALIGN_RIGHT))
    {
        map_x = 0;
        map_y = 0;
        map_height = (term_rows - border_space);
        status_width = 26;
        message_width = term_cols - (COLNO + status_width + (border_space * 3));
        map_width = term_cols - (status_width + message_width + (border_space * 3));
        message_x = map_x + (map_width + border_space);
        message_y = 0;
        message_height = (term_rows - border_space);
        status_x = message_x + (message_width + border_space);
        status_y = 0;
        status_height = (term_rows - border_space);
    }
    else if ((status_orientation == ALIGN_RIGHT) &&
     (message_orientation == ALIGN_BOTTOM))
    {
        map_x = 0;
        map_y = 0;
        status_width = 26;
        map_width = term_cols - (status_width + (border_space * 2));
        message_height = term_rows - (ROWNO + (border_space * 2));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_height = term_rows - (message_height + (border_space * 2));
        message_x = 0;
        message_y = map_y + (map_height + border_space);
        message_width = map_width;
        status_x = map_x + (map_width + border_space);
        status_y = 0;
        status_height = (term_rows - border_space);
    }
    else if ((status_orientation == ALIGN_RIGHT) &&
     (message_orientation == ALIGN_LEFT))
    {
        status_x = 0;
        status_y = 0;
        status_height = (term_rows - border_space);
        status_width = 26;
        message_width = term_cols - (status_width + COLNO + (border_space * 3));
        map_x = status_x + (status_width + border_space);
        map_y = 0;
        map_height = (term_rows - border_space);
        map_width = term_cols - (status_width + message_width + (border_space * 3));
        message_x = map_x + (map_width + border_space);
        message_y = 0;
        message_height = (term_rows - border_space);
    }
    if ((status_orientation == ALIGN_BOTTOM) &&
     (message_orientation == ALIGN_TOP))
    {
        message_x = 0;
        message_y = 0;
        message_width = (term_cols - border_space);
        status_height = 2;
        message_height = term_rows - (status_height + ROWNO + (border_space * 3));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_x = 0;
        map_y = message_y + (message_height + border_space);
        map_width = (term_cols - border_space);
        map_height = term_rows - (status_height + message_height + (border_space * 3));
        status_x = 0;
        status_y = map_y + (map_height + border_space);
        status_width = (term_cols - border_space);
    }
    else if ((status_orientation == ALIGN_BOTTOM) &&
     (message_orientation == ALIGN_RIGHT))
    {
        map_x = 0;
        map_y = 0;
        status_height = 2;
        map_height = term_rows - (status_height + (border_space * 2));
        message_width = term_cols - (COLNO + (border_space * 2));
        map_width = term_cols - (message_width + (border_space * 2));
        status_x = 0;
        status_y = map_y + (map_height + border_space);
        status_width = map_width;
        message_x = map_x + (map_width + border_space);
        message_y = 0;
        message_height = (term_rows - border_space);
    }
    else if ((status_orientation == ALIGN_BOTTOM) &&
     (message_orientation == ALIGN_BOTTOM))
    {
        map_x = 0;
        map_y = 0;
        message_x = 0;
        status_x = 0;
        message_width = (term_cols - border_space);
        status_height = 2;
        message_height = term_rows - (status_height + ROWNO + (border_space * 3));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_width = (term_cols - border_space);
        map_height = term_rows - (status_height + message_height + (border_space * 3));
        message_y = map_y + (map_height + border_space);
        status_y = message_y + (message_height + border_space);
        status_width = (term_cols - border_space);
    }
    else if ((status_orientation == ALIGN_BOTTOM) &&
     (message_orientation == ALIGN_LEFT))
    {
        message_x = 0;
        message_y = 0;
        message_height = (term_rows - border_space);
        message_width = term_cols - (COLNO + (border_space * 2));
        status_height = 2;
        map_x = message_x + (message_width + border_space);
        map_y = 0;
        map_height = term_rows - (status_height + (border_space * 2));
        map_width = term_cols - (message_width + (border_space * 2));
        status_x = map_x;
        status_y = map_y + (map_height + border_space);
        status_width = term_cols - (message_width + (border_space * 2));
    }
    if ((status_orientation == ALIGN_LEFT) &&
     (message_orientation == ALIGN_TOP))
    {
        status_x = 0;
        status_y = 0;
        status_height = (term_rows - border_space);
        status_width = 26;
        message_x = status_x + (status_width + border_space);
        message_y = 0;
        message_height = term_rows - (ROWNO + (border_space * 2));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        message_width = term_cols - (status_width + (border_space * 2));
        map_x = message_x;
        map_y = message_y + (message_height + border_space);
        map_height = term_rows - (message_height + (border_space * 2));
        map_width = term_cols - (status_width + (border_space * 2));
    }
    else if ((status_orientation == ALIGN_LEFT) &&
     (message_orientation == ALIGN_RIGHT))
    {
        message_x = 0;
        message_y = 0;
        message_height = (term_rows - border_space);
        status_width = 26;
        message_width = term_cols - (status_width + COLNO + (border_space * 3));
        map_x = message_x + (message_width + border_space);
        map_y = 0;
        map_height = (term_rows - border_space);
        map_width = term_cols - (status_width + message_width + (border_space * 3));
        status_x = map_x + (map_width + border_space);
        status_y = 0;
        status_height = (term_rows - border_space);
    }
    else if ((status_orientation == ALIGN_LEFT) &&
     (message_orientation == ALIGN_BOTTOM))
    {
        status_x = 0;
        status_y = 0;
        status_height = (term_rows - border_space);
        status_width = 26;
        map_x = status_x + (status_width + border_space);
        map_y = 0;
        message_height = term_rows - (ROWNO + (border_space * 2));
        if (message_height < min_message_height)
        {
            message_height = min_message_height;
        }
        map_height = term_rows - (message_height + (border_space * 2));
        map_width = term_cols - (status_width + (border_space * 2));
        message_x = status_x + (status_width + border_space);
        message_y = map_y + (map_height + border_space);
        message_width = map_width;
    }
    else if ((status_orientation == ALIGN_LEFT) &&
     (message_orientation == ALIGN_LEFT))
    {
        status_x = 0;
        status_y = 0;
        status_height = (term_rows - border_space);
        status_width = 26;
        message_x = status_x + (status_width + border_space);
        message_y = 0;
        message_height = status_height;
        message_width = term_cols - (COLNO + status_width + (border_space * 3));
        map_x = message_x + (message_width + border_space);
        map_y = 0;
        map_height = message_height;
        map_width = term_cols - (status_width + message_width + (border_space * 3));
    }
    
    if (map_width > COLNO)
    {
        map_width = COLNO;
    }
    
    if (map_height > ROWNO)
    {
        map_height = ROWNO;
    }
    
    if (curses_window_exists(STATUS_WIN))
    {
        curses_del_nhwin(STATUS_WIN);
        curses_del_nhwin(MESSAGE_WIN);
        curses_del_nhwin(MAP_WIN);
        clear();
    }
    
    curses_add_nhwin(STATUS_WIN, status_height, status_width, status_y,
     status_x, status_orientation, borders);

    curses_add_nhwin(MESSAGE_WIN, message_height, message_width, message_y,
     message_x, message_orientation, borders);

    if (borders)
    {
        mapborderwin = newwin(map_height + border_space, map_width
         + border_space, map_y, map_x);
        wrefresh(mapborderwin);
        curses_add_nhwin(MAP_WIN, map_height, map_width, map_y + 1, map_x + 1,
         0, borders);
    }
    else
    {
        curses_add_nhwin(MAP_WIN, map_height, map_width, map_y, map_x, 0,
         borders);
    }

    refresh();
    curses_refresh_nethack_windows();

    if (iflags.window_inited)
    {
        curses_update_stats();
    }
    else
    {
        iflags.window_inited = TRUE;
    }
}


/* Initialize curses colors to colors used by NetHack */

void curses_init_nhcolors()
{
#ifdef TEXTCOLOR
    if (has_colors())
    {
        use_default_colors();
        init_pair(1, COLOR_BLACK, -1);
        init_pair(2, COLOR_RED, -1);
        init_pair(3, COLOR_GREEN, -1);
        init_pair(4, COLOR_YELLOW, -1);
        init_pair(5, COLOR_BLUE, -1);
        init_pair(6, COLOR_MAGENTA, -1);
        init_pair(7, COLOR_CYAN, -1);
        init_pair(8, -1, -1);
        init_pair(9, COLOR_WHITE, -1);

        if (can_change_color())
        {
            init_color(COLOR_YELLOW, 500, 300, 0);
            init_color(COLOR_WHITE, 600, 600, 600);
        }
    }
#endif
}


/* Allow player to pick character's role, race, gender, and alignment.
Borrowed from the Gnome window port. */

void curses_choose_character()
{
    int n, i, sel;
    const char** choices;
    int* pickmap;

    /* prevent an unnecessary prompt */
    rigid_role_checks();

    if (!flags.randomall && flags.initrole < 0) {

	/* select a role */
	for (n = 0; roles[n].name.m; n++) continue;
	choices = (const char **)alloc(sizeof(char *) * (n+1));
	pickmap = (int*)alloc(sizeof(int) * (n+1));
	for (;;) {
	    for (n = 0, i = 0; roles[i].name.m; i++) {
		if (ok_role(i, flags.initrace,
			    flags.initgend, flags.initalign)) {
		    if (flags.initgend >= 0 && flags.female && roles[i].name.f)
			choices[n] = roles[i].name.f;
		    else
			choices[n] = roles[i].name.m;
		    pickmap[n++] = i;
		}
	    }
	    if (n > 0) break;
	    else if (flags.initalign >= 0) flags.initalign = -1;    /* reset */
	    else if (flags.initgend >= 0) flags.initgend = -1;
	    else if (flags.initrace >= 0) flags.initrace = -1;
	    else panic("no available ROLE+race+gender+alignment combinations");
	}
	choices[n] = (const char *) 0;
	if (n > 1)
        sel = curses_character_dialog(choices, "Choose one of the following roles:");
	else sel = 0;
	if (sel >= 0) sel = pickmap[sel];
	else if (sel == ROLE_NONE) {		/* Quit */
	    clearlocks();
	    curses_bail(0);
	}
	free(choices);
	free(pickmap);
    } else if (flags.initrole < 0) sel = ROLE_RANDOM;
    else sel = flags.initrole;
  
    if (sel == ROLE_RANDOM) {	/* Random role */
	sel = pick_role(flags.initrace, flags.initgend,
			  flags.initalign, PICK_RANDOM);
	if (sel < 0) sel = randrole();
    }

    flags.initrole = sel;

    /* Select a race, if necessary */
    /* force compatibility with role, try for compatibility with
     * pre-selected gender/alignment */
    if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
	if (flags.initrace == ROLE_RANDOM || flags.randomall) {
	    flags.initrace = pick_race(flags.initrole, flags.initgend,
				       flags.initalign, PICK_RANDOM);
	    if (flags.initrace < 0) flags.initrace = randrace(flags.initrole);
	} else {
	    /* Count the number of valid races */
	    n = 0;	/* number valid */
	    for (i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend, flags.initalign))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; races[i].noun; i++) {
		    if (validrace(flags.initrole, i)) n++;
		}
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend,
			    flags.initalign)) {
		    choices[n] = races[i].noun;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = curses_character_dialog(choices, "Choose one of the following races:");
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		curses_bail(0);
	    }
	    flags.initrace = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initrace == ROLE_RANDOM) {	/* Random role */
	    sel = pick_race(flags.initrole, flags.initgend,
			    flags.initalign, PICK_RANDOM);
	    if (sel < 0) sel = randrace(flags.initrole);
	    flags.initrace = sel;
	}
    }

    /* Select a gender, if necessary */
    /* force compatibility with role/race, try for compatibility with
     * pre-selected alignment */
    if (flags.initgend < 0 ||
	!validgend(flags.initrole, flags.initrace, flags.initgend)) {
	if (flags.initgend == ROLE_RANDOM || flags.randomall) {
	    flags.initgend = pick_gend(flags.initrole, flags.initrace,
				       flags.initalign, PICK_RANDOM);
	    if (flags.initgend < 0)
		flags.initgend = randgend(flags.initrole, flags.initrace);
	} else {
	    /* Count the number of valid genders */
	    n = 0;	/* number valid */
	    for (i = 0; i < ROLE_GENDERS; i++) {
		if (ok_gend(flags.initrole, flags.initrace, i, flags.initalign))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; i < ROLE_GENDERS; i++) {
		    if (validgend(flags.initrole, flags.initrace, i)) n++;
		}
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; i < ROLE_GENDERS; i++) {
		if (ok_gend(flags.initrole, flags.initrace, i,
				flags.initalign)) {
		    choices[n] = genders[i].adj;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = curses_character_dialog(choices, "Choose one of the following genders:");
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		curses_bail(0);
	    }
	    flags.initgend = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initgend == ROLE_RANDOM) {	/* Random gender */
	    sel = pick_gend(flags.initrole, flags.initrace,
			    flags.initalign, PICK_RANDOM);
	    if (sel < 0) sel = randgend(flags.initrole, flags.initrace);
	    flags.initgend = sel;
	}
    }

    /* Select an alignment, if necessary */
    /* force compatibility with role/race/gender */
    if (flags.initalign < 0 ||
	!validalign(flags.initrole, flags.initrace, flags.initalign)) {
	if (flags.initalign == ROLE_RANDOM || flags.randomall) {
	    flags.initalign = pick_align(flags.initrole, flags.initrace,
					 flags.initgend, PICK_RANDOM);
	    if (flags.initalign < 0)
		flags.initalign = randalign(flags.initrole, flags.initrace);
	} else {
	    /* Count the number of valid alignments */
	    n = 0;	/* number valid */
	    for (i = 0; i < ROLE_ALIGNS; i++) {
		if (ok_align(flags.initrole, flags.initrace, flags.initgend, i))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; i < ROLE_ALIGNS; i++)
		    if (validalign(flags.initrole, flags.initrace, i)) n++;
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; i < ROLE_ALIGNS; i++) {
		if (ok_align(flags.initrole,
			     flags.initrace, flags.initgend, i)) {
		    choices[n] = aligns[i].adj;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = curses_character_dialog(choices, "Choose one of the following alignments:");
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		curses_bail(0);
	    }
	    flags.initalign = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initalign == ROLE_RANDOM) {
	    sel = pick_align(flags.initrole, flags.initrace,
			     flags.initgend, PICK_RANDOM);
	    if (sel < 0) sel = randalign(flags.initrole, flags.initrace);
	    flags.initalign = sel;
	}
    }
}


/* Prompt user for character race, role, alignment, or gender */

int curses_character_dialog(const char** choices, const char *prompt)
{
    int count, count2, ret, curletter;
    char used_letters[52];
    anything identifier;
    menu_item *selected = NULL;
    winid wid = curses_get_wid(NHW_MENU);

    identifier.a_void = 0;
    curses_start_menu(wid);

    for (count=0; choices[count]; count++)
    {
        curletter=tolower(choices[count][0]);
        for (count2=0; count2<count; count2++)
        {
            if (curletter==used_letters[count2])
            {
                curletter=toupper(curletter);
            }
        }

        identifier.a_int = (count + 1); /* Must be non-zero */
        curses_add_menu(wid, NO_GLYPH, &identifier, curletter, 0,
         A_NONE, choices[count], FALSE);
        used_letters[count] = curletter;
    }

    /* Random Selection */
    identifier.a_int = ROLE_RANDOM;
    curses_add_menu(wid, NO_GLYPH, &identifier, '*', 0, A_NONE, "Random",
     FALSE);    
    
    /* Quit prompt */
    identifier.a_int = ROLE_NONE;
    curses_add_menu(wid, NO_GLYPH, &identifier, 'q', 0, A_NONE, "Quit",
     FALSE);    
    curses_end_menu(wid, prompt);
    ret = curses_select_menu(wid, PICK_ONE, &selected);
    ret = (selected->item.a_int);

    if (ret > 0)
    {
        ret--;
    }
    
    free(selected);
    return ret;
}


/* Initialize and display options appropriately */

void curses_init_options()
{
    set_wc_option_mod_status(WC_ALIGN_MESSAGE|WC_ALIGN_STATUS|WC_COLOR|
     WC_HILITE_PET|WC_POPUP_DIALOG, SET_IN_GAME);

    set_wc2_option_mod_status(WC2_GUICOLOR, SET_IN_GAME);

    /* Remove a few options that are irrelevant to this windowport */
    set_option_mod_status("DECgraphics", SET_IN_FILE);
    set_option_mod_status("perm_invent", SET_IN_FILE);
    set_option_mod_status("eight_bit_tty", SET_IN_FILE);

    /* Make sure that DECgraphics is not set to true via the config
    file, as this will cause display issues.  We can't disable it in
    options.c in case the game is compiled with both tty and curses.*/
    if (iflags.DECgraphics)
    {
        switch_graphics(CURS_GRAPHICS);
    }
	
#ifdef PDCURSES
    /* PDCurses for SDL, win32 and OS/2 has the ability to set the
     terminal size programatically.  If the user does not specify a
     size in the config file, we will set it to a nice big 110x32 to
     take advantage of some of the nice features of this windowport. */
    if (iflags.wc2_term_cols == 0)
    {
        iflags.wc2_term_cols = 110;
    }
    
    if (iflags.wc2_term_rows == 0)
    {
        iflags.wc2_term_rows = 32;
    }
    
    resize_term(iflags.wc2_term_rows, iflags.wc2_term_cols);
    getmaxyx(base_term, term_rows, term_cols);
#endif
    if (!iflags.wc2_windowborders)
    {
        iflags.wc2_windowborders = 3; /* Set to auto if not specified */
    }
}

