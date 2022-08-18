/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursdial.h"
#include "func_tab.h"
#include <ctype.h>

/* Dialog windows for curses interface */


/* Private declarations */

typedef struct nhmi {
    winid wid;                  /* NetHack window id */
    int glyph;                  /* Menu glyphs */
    anything identifier;        /* Value returned if item selected */
    char accelerator;           /* Character used to select item from menu */
    char group_accel;           /* Group accelerator for menu item, if any */
    int attr;                   /* Text attributes for item */
    const char *str;            /* Text of menu item */
    boolean presel;             /* Whether menu item should be preselected */
    boolean selected;           /* Whether item is currently selected */
    int page_num;               /* Display page number for entry */
    int line_num;               /* Line number on page where entry begins */
    int num_lines;              /* Number of lines entry uses on page */
    int count;                  /* Count for selected item */
    struct nhmi *prev_item;     /* Pointer to previous entry */
    struct nhmi *next_item;     /* Pointer to next entry */
} nhmenu_item;

typedef struct nhm {
    winid wid;                  /* NetHack window id */
    const char *prompt;         /* Menu prompt text */
    nhmenu_item *entries;       /* Menu entries */
    int num_entries;            /* Number of menu entries */
    int num_pages;              /* Number of display pages for entry */
    int height;                 /* Window height of menu */
    int width;                  /* Window width of menu */
    boolean reuse_accels;       /* Non-unique accelerators per page */
    struct nhm *prev_menu;      /* Pointer to previous entry */
    struct nhm *next_menu;      /* Pointer to next entry */
} nhmenu;

typedef enum menu_op_type {
    SELECT,
    DESELECT,
    INVERT
} menu_op;

#ifdef MENU_COLOR
extern struct menucoloring *menu_colorings;
#endif

static nhmenu *get_menu(winid wid);
static char menu_get_accel(boolean first);
static void menu_determine_pages(nhmenu *menu);
static boolean menu_is_multipage(nhmenu *menu, int width, int height);
static void menu_win_size(nhmenu *menu);
static void menu_display_page(nhmenu *menu, WINDOW * win, int page_num);
static int menu_get_selections(WINDOW * win, nhmenu *menu, int how);
static void menu_select_deselect(WINDOW * win, nhmenu_item *item,
                                 menu_op operation);
static int menu_operation(WINDOW * win, nhmenu *menu, menu_op operation,
                          int page_num);
static void menu_clear_selections(nhmenu *menu);
static int menu_max_height(void);

static nhmenu *nhmenus = NULL;  /* NetHack menu array */


/* Get a line of text from the player, such as asking for a character name or a wish */

void
curses_line_input_dialog(const char *prompt, char *answer, int buffer)
{
    int map_height, map_width, maxwidth, remaining_buf, winx, winy, count;
    WINDOW *askwin, *bwin;
    char input[buffer];
    char *tmpstr;
    int prompt_width = strlen(prompt) + buffer + 1;
    int prompt_height = 1;
    int height = prompt_height;

    maxwidth = term_cols - 2;

    if (iflags.window_inited) {
        if (!iflags.wc_popup_dialog)
            return curses_message_win_getline(prompt, answer, buffer);
        curses_get_window_size(MAP_WIN, &map_height, &map_width);
        if ((prompt_width + 2) > map_width)
            maxwidth = map_width - 2;
    }

    if (prompt_width > maxwidth) {
        prompt_height = curses_num_lines(prompt, maxwidth);
        height = prompt_height;
        prompt_width = maxwidth;
        tmpstr = curses_break_str(prompt, maxwidth, prompt_height);
        remaining_buf = buffer - (strlen(tmpstr) - 1);
        if (remaining_buf > 0) {
            height += (remaining_buf / prompt_width);
            if ((remaining_buf % prompt_width) > 0) {
                height++;
            }
        }
        free(tmpstr);
    }

    if (iflags.window_inited) {
        bwin = curses_create_window(prompt_width, height, UP);
        wrefresh(bwin);
        getbegyx(bwin, winy, winx);
        askwin = newwin(height, prompt_width, winy + 1, winx + 1);
    } else {
        bwin = curses_create_window(prompt_width, height, CENTER);
        wrefresh(bwin);
        getbegyx(bwin, winy, winx);
        askwin = newwin(height, prompt_width, winy + 1, winx + 1);
    }
    for (count = 0; count < prompt_height; count++) {
        tmpstr = curses_break_str(prompt, maxwidth, count + 1);
        if (count == (prompt_height - 1)) { /* Last line */
            mvwprintw(askwin, count, 0, "%s ", tmpstr);
        } else {
            mvwaddstr(askwin, count, 0, tmpstr);
        }
        free(tmpstr);
    }

    echo();
    curs_set(1);
    wgetnstr(askwin, input, buffer - 1);
    curs_set(0);
    strcpy(answer, input);
    werase(bwin);
    delwin(bwin);
    curses_destroy_win(askwin);
    noecho();
}


/* Get a single character response from the player, such as a y/n prompt */

int
curses_character_input_dialog(const char *prompt, const char *choices,
                              char def)
{
    WINDOW *askwin = NULL;
    int answer, count, maxwidth, map_height, map_width;
    char *linestr;
    char askstr[BUFSZ + QBUFSZ];
    char choicestr[QBUFSZ];
    int prompt_width = strlen(prompt);
    int prompt_height = 1;
    boolean any_choice = FALSE;
    boolean accept_count = FALSE;

    if (invent || (moves > 1)) {
        curses_get_window_size(MAP_WIN, &map_height, &map_width);
    } else {
        map_height = term_rows;
        map_width = term_cols;
    }

    maxwidth = map_width - 2;

    if (choices != NULL) {
        for (count = 0; choices[count] != '\0'; count++) {
            if (choices[count] == '#') { /* Accept a count */
                accept_count = TRUE;
            }
        }
        choicestr[0] = ' ';
        choicestr[1] = '[';
        for (count = 0; choices[count] != '\0'; count++) {
            if (choices[count] == DOESCAPE) { /* Escape */
                break;
            }
            choicestr[count + 2] = choices[count];
        }
        choicestr[count + 2] = ']';
        if (((def >= 'A') && (def <= 'Z')) || ((def >= 'a') && (def <= 'z'))) {
            choicestr[count + 3] = ' ';
            choicestr[count + 4] = '(';
            choicestr[count + 5] = def;
            choicestr[count + 6] = ')';
            choicestr[count + 7] = '\0';
        } else {                /* No usable default choice */

            choicestr[count + 3] = '\0';
            def = '\0';         /* Mark as no default */
        }
        strcpy(askstr, prompt);
        strcat(askstr, choicestr);
    } else {
        strcpy(askstr, prompt);
        any_choice = TRUE;
    }

    prompt_width = strlen(askstr);

    if ((prompt_width + 2) > maxwidth) {
        prompt_height = curses_num_lines(askstr, maxwidth);
        prompt_width = map_width - 2;
    }

    if (iflags.wc_popup_dialog || curses_stupid_hack) {
        askwin = curses_create_window(prompt_width, prompt_height, UP);
        for (count = 0; count < prompt_height; count++) {
            linestr = curses_break_str(askstr, maxwidth, count + 1);
            mvwaddstr(askwin, count + 1, 1, linestr);
            free(linestr);
        }

        wrefresh(askwin);
    } else {
        linestr = curses_copy_of(askstr);
        pline("%s", linestr);
        free(linestr);
        curs_set(1);
    }

    curses_stupid_hack = 0;

    while (1) {
        answer = getch();

        if (answer == ERR) {
            answer = def;
            break;
        }

        answer = curses_convert_keys(answer);

        if (answer == KEY_ESC) {
            if (choices == NULL) {
                break;
            }
            answer = def;
            for (count = 0; choices[count] != '\0'; count++) {
                if (choices[count] == 'q') {    /* q is preferred over n */
                    answer = 'q';
                } else if ((choices[count] == 'n') && answer != 'q') {
                    answer = 'n';
                }
            }
            break;
        } else if ((answer == '\n') || (answer == '\r') || (answer == ' ')) {
            if ((choices != NULL) && (def != '\0')) {
                answer = def;
            }
            break;
        }

        if (digit(answer)) {
            if (accept_count) {
                if (answer != '0') {
                    yn_number = curses_get_count(answer - '0');
                    touchwin(askwin);
                    refresh();
                }

                answer = '#';
                break;
            }
        }

        if (any_choice) {
            break;
        }

        if (choices != NULL) {
            for (count = 0; count < strlen(choices); count++) {
                if (choices[count] == answer) {
                    break;
                }
            }
            if (choices[count] == answer) {
                break;
            }
        }
    }

    if (iflags.wc_popup_dialog) {
        /* Kludge to make prompt visible after window is dismissed
           when inputting a number */
        if (digit(answer)) {
            linestr = curses_copy_of(askstr);
            pline("%s", linestr);
            free(linestr);
            curs_set(1);
        }

        curses_destroy_win(askwin);
    } else {
        curses_clear_unhighlight_message_window();
        curs_set(0);
    }

    return answer;
}


/* Return an extended command from the user */

int
curses_ext_cmd(void)
{
    int count, letter, prompt_width, startx, starty, winx, winy;
    int messageh, messagew, maxlen = BUFSZ - 1;
    int ret = -1;
    char cur_choice[BUFSZ];
    int matches = 0;
    WINDOW *extwin = NULL, *extwin2 = NULL;

    if (iflags.extmenu) {
        return extcmd_via_menu();
    }

    startx = 0;
    starty = 0;
    if (iflags.wc_popup_dialog) { /* Prompt in popup window */
        int x0, y0, w, h; /* bounding coords of popup */

        extwin2 = curses_create_window(25, 1, UP);
        wrefresh(extwin2);
        /* create window inside window to prevent overwriting of border */
        getbegyx(extwin2,y0,x0);
        getmaxyx(extwin2,h,w);
        extwin = newwin(1, w-2, y0+1, x0+1);
        if (w - 4 < maxlen) maxlen = w - 4;
        nhUse(h); /* needed only to give getmaxyx three arguments */
    } else {
        curses_get_window_xy(MESSAGE_WIN, &winx, &winy);
        curses_get_window_size(MESSAGE_WIN, &messageh, &messagew);

        if (curses_window_has_border(MESSAGE_WIN)) {
            winx++;
            winy++;
        }

        winy += messageh - 1;
        extwin = newwin(1, messagew-2, winy, winx);
        if (messagew - 4 < maxlen) maxlen = messagew - 4;
        pline("#");
    }

    cur_choice[0] = '\0';

    while (1) {
        wmove(extwin, starty, startx);
        waddstr(extwin, "# ");
        wmove(extwin, starty, startx + 2);
        waddstr(extwin, cur_choice);
        wmove(extwin, starty, strlen(cur_choice) + startx + 2);
        wprintw(extwin, "             ");

        /* if we have an autocomplete command, AND it matches uniquely */
        if (matches == 1) {
            curses_toggle_color_attr(extwin, NONE, A_UNDERLINE, ON);
            wmove(extwin, starty, strlen(cur_choice) + startx + 2);
            wprintw(extwin, "%s", extcmdlist[ret].ef_txt + strlen(cur_choice));
            curses_toggle_color_attr(extwin, NONE, A_UNDERLINE, OFF);
            mvwprintw(extwin, starty,
                      strlen(extcmdlist[ret].ef_txt) + 2, "          ");
        }

        wrefresh(extwin);
        letter = getch();
        prompt_width = strlen(cur_choice);
        matches = 0;

        if (letter == DOESCAPE || letter == ERR) {
            ret = -1;
            break;
        }

        if ((letter == '\r') || (letter == '\n')) {
            if (ret == -1) {
                for (count = 0; extcmdlist[count].ef_txt; count++) {
                    if (!strcasecmp(cur_choice, extcmdlist[count].ef_txt)) {
                        ret = count;
                        break;
                    }
                }
            }
            break;
        }

        /* DEL/Rubout */
        if (letter == '\177') {
            letter = '\b';
        }
        if ((letter == '\b') || (letter == KEY_BACKSPACE)) {
            if (prompt_width == 0) {
                ret = -1;
                break;
            } else {
                cur_choice[prompt_width - 1] = '\0';
                letter = '*';
                prompt_width--;
            }
        }
        if (letter != '*' && prompt_width < maxlen) {
            cur_choice[prompt_width] = letter;
            cur_choice[prompt_width + 1] = '\0';
            ret = -1;
        }
        for (count = 0; extcmdlist[count].ef_txt; count++) {
            if (!wizard && (extcmdlist[count].flags & WIZMODECMD)) {
                continue;
            }
            if (!(extcmdlist[count].flags & AUTOCOMPLETE)) {
                continue;
            }
            if (strlen(extcmdlist[count].ef_txt) > prompt_width) {
                if (strncasecmp(cur_choice, extcmdlist[count].ef_txt,
                                prompt_width) == 0) {
                    if ((extcmdlist[count].ef_txt[prompt_width] ==
                         lowc(letter)) || letter == '*') {
                        if (matches == 0) {
                            ret = count;
                        }

                        matches++;
                    }
                }
            }
        }
    }

    curses_destroy_win(extwin);
    if (extwin2) curses_destroy_win(extwin2);
    return ret;
}


/* Initialize a menu from given NetHack winid */

void
curses_create_nhmenu(winid wid)
{
    nhmenu *new_menu = NULL;
    nhmenu *menuptr = nhmenus;
    nhmenu_item *menu_item_ptr = NULL;
    nhmenu_item *tmp_menu_item = NULL;

    new_menu = get_menu(wid);

    if (new_menu != NULL) {
        /* Reuse existing menu, clearing out current entries */
        menu_item_ptr = new_menu->entries;

        if (menu_item_ptr != NULL) {
            while (menu_item_ptr->next_item != NULL) {
                tmp_menu_item = menu_item_ptr->next_item;
                free((char *) menu_item_ptr->str);
                free(menu_item_ptr);
                menu_item_ptr = tmp_menu_item;
            }
            free((char *) menu_item_ptr->str);
            free(menu_item_ptr); /* Last entry */
            new_menu->entries = NULL;
        }
        if (new_menu->prompt != NULL) { /* Reusing existing menu */
            free((char *) new_menu->prompt);
            new_menu->prompt = NULL;
        }
        new_menu->num_pages = 0;
        new_menu->height = 0;
        new_menu->width = 0;
        new_menu->reuse_accels = FALSE;
        return;
    }

    new_menu = malloc(sizeof (nhmenu));
    new_menu->wid = wid;
    new_menu->prompt = NULL;
    new_menu->entries = NULL;
    new_menu->num_pages = 0;
    new_menu->height = 0;
    new_menu->width = 0;
    new_menu->reuse_accels = FALSE;
    new_menu->next_menu = NULL;

    if (nhmenus == NULL) {      /* no menus in memory yet */
        new_menu->prev_menu = NULL;
        nhmenus = new_menu;
    } else {
        while (menuptr->next_menu != NULL) {
            menuptr = menuptr->next_menu;
        }
        new_menu->prev_menu = menuptr;
        menuptr->next_menu = new_menu;
    }
}


/* Add a menu item to the given menu window */

void
curses_add_nhmenu_item(winid wid, int glyph, const ANY_P * identifier,
                       char accelerator, char group_accel, int attr,
                       const char *str, boolean presel)
{
    char *new_str;
    nhmenu_item *new_item, *current_items, *menu_item_ptr;
    nhmenu *current_menu = get_menu(wid);

    if (current_menu == NULL) {
        impossible
            ("curses_add_nhmenu_item: attempt to add item to nonexistent menu");
        return;
    }

    if (str == NULL) {
        return;
    }

    new_str = curses_copy_of(str);
    curses_rtrim((char *) new_str);
    new_item = malloc(sizeof (nhmenu_item));
    new_item->wid = wid;
    new_item->glyph = glyph;
    new_item->identifier = *identifier;
    new_item->accelerator = accelerator;
    new_item->group_accel = group_accel;
    new_item->attr = attr;
    new_item->str = new_str;
    new_item->presel = presel;
    new_item->selected = FALSE;
    new_item->page_num = 0;
    new_item->line_num = 0;
    new_item->num_lines = 0;
    new_item->count = -1;
    new_item->next_item = NULL;

    current_items = current_menu->entries;
    menu_item_ptr = current_items;

    if (current_items == NULL) {
        new_item->prev_item = NULL;
        current_menu->entries = new_item;
    } else {
        while (menu_item_ptr->next_item != NULL) {
            menu_item_ptr = menu_item_ptr->next_item;
        }
        new_item->prev_item = menu_item_ptr;
        menu_item_ptr->next_item = new_item;
    }
}


/* No more entries are to be added to menu, so details of the menu can be
 finalized in memory */

void
curses_finalize_nhmenu(winid wid, const char *prompt)
{
    int count = 0;
    nhmenu *current_menu = get_menu(wid);

    if (current_menu == NULL) {
        impossible("curses_finalize_nhmenu: attempt to finalize nonexistent menu");
        return;
    }

    nhmenu_item *menu_item_ptr = current_menu->entries;
    while (menu_item_ptr != NULL) {
        menu_item_ptr = menu_item_ptr->next_item;
        count++;
    }

    current_menu->num_entries = count;
    current_menu->prompt = curses_copy_of(prompt);
}


/* Display a nethack menu, and return a selection, if applicable */

int
curses_display_nhmenu(winid wid, int how, MENU_ITEM_P ** _selected)
{
    nhmenu *current_menu = get_menu(wid);
    nhmenu_item *menu_item_ptr;
    int num_chosen, count;
    WINDOW *win;
    MENU_ITEM_P *selected = NULL;

    *_selected = NULL;

    if (current_menu == NULL) {
        impossible("curses_display_nhmenu: attempt to display nonexistent menu");
        return '\033';
    }

    menu_item_ptr = current_menu->entries;

    if (menu_item_ptr == NULL) {
        impossible("curses_display_nhmenu: attempt to display empty menu");
        return '\033';
    }

    /* Reset items to unselected to clear out selections from previous
       invocations of this menu, and preselect appropriate items */
    while (menu_item_ptr != NULL) {
        menu_item_ptr->selected = menu_item_ptr->presel;
        menu_item_ptr = menu_item_ptr->next_item;
    }

    menu_win_size(current_menu);
    menu_determine_pages(current_menu);

    /* Display pre and post-game menus centered */
    if (((moves <= 1) && !invent) || program_state.gameover) {
        win = curses_create_window(current_menu->width,
                                   current_menu->height, CENTER);
    } else { /* Display during-game menus on the right out of the way */
        win = curses_create_window(current_menu->width,
                                   current_menu->height, RIGHT);
    }

    num_chosen = menu_get_selections(win, current_menu, how);
    curses_destroy_win(win);

    if (num_chosen > 0) {
        selected = (MENU_ITEM_P *) alloc((unsigned) (num_chosen * sizeof (MENU_ITEM_P)));
        count = 0;

        menu_item_ptr = current_menu->entries;

        while (menu_item_ptr != NULL) {
            if (menu_item_ptr->selected) {
                if (count == num_chosen) {
                    impossible("curses_display_nhmenu: Selected items "
                          "exceeds expected number");
                     break;
                }
                selected[count].item = menu_item_ptr->identifier;
                selected[count].count = menu_item_ptr->count;
                count++;
            }
            menu_item_ptr = menu_item_ptr->next_item;
        }

        if (count != num_chosen) {
            impossible("curses_display_nhmenu: Selected items less than expected number");
        }
    }

    *_selected = selected;

    return num_chosen;
}


boolean
curses_menu_exists(winid wid)
{
    if (get_menu(wid) != NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Delete the menu associated with the given NetHack winid from memory */

void
curses_del_menu(winid wid)
{
    nhmenu_item *tmp_menu_item;
    nhmenu_item *menu_item_ptr;
    nhmenu *tmpmenu;
    nhmenu *current_menu = get_menu(wid);

    if (current_menu == NULL) {
        return;
    }

    menu_item_ptr = current_menu->entries;

    /* First free entries associated with this menu from memory */
    if (menu_item_ptr != NULL) {
        while (menu_item_ptr->next_item != NULL) {
            tmp_menu_item = menu_item_ptr->next_item;
            free((void *)menu_item_ptr->str);
            free(menu_item_ptr);
            menu_item_ptr = tmp_menu_item;
        }
        free((void *)menu_item_ptr->str);
        free(menu_item_ptr);    /* Last entry */
        current_menu->entries = NULL;
    }

    /* Now unlink the menu from the list and free it as well */
    if (current_menu->prev_menu != NULL) {
        tmpmenu = current_menu->prev_menu;
        tmpmenu->next_menu = current_menu->next_menu;
    } else {
        nhmenus = current_menu->next_menu;      /* New head mode or NULL */
    }
    if (current_menu->next_menu != NULL) {
        tmpmenu = current_menu->next_menu;
        tmpmenu->prev_menu = current_menu->prev_menu;
    }

    if (current_menu->prompt) {
        free((char *) current_menu->prompt);
    }
    free(current_menu);

    curses_del_wid(wid);
}


/* return a pointer to the menu associated with the given NetHack winid */

static nhmenu *
get_menu(winid wid)
{
    nhmenu *menuptr = nhmenus;

    while (menuptr != NULL) {
        if (menuptr->wid == wid) {
            return menuptr;
        }
        menuptr = menuptr->next_menu;
    }

    return NULL;                /* Not found */
}


static char
menu_get_accel(boolean first)
{
    char ret;
    static char next_letter = 'a';

    if (first) {
        next_letter = 'a';
    }

    ret = next_letter;

    if (((next_letter < 'z') && (next_letter >= 'a')) || ((next_letter < 'Z')
                                                          && (next_letter >=
                                                              'A')) ||
        ((next_letter < '9') && (next_letter >= '0'))) {
        next_letter++;
    } else if (next_letter == 'z') {
        next_letter = 'A';
    } else if (next_letter == 'Z') {
        next_letter = '0';
    }

    return ret;
}


/* Determine if menu will require multiple pages to display */

static boolean
menu_is_multipage(nhmenu *menu, int width, int height)
{
    int num_lines;
    int curline = 0;
    nhmenu_item *menu_item_ptr = menu->entries;

    if (strlen(menu->prompt) > 0) {
        curline += curses_num_lines(menu->prompt, width) + 1;
    }

    if (menu->num_entries <= (height - curline)) {
        while (menu_item_ptr != NULL) {
            menu_item_ptr->line_num = curline;
            if (menu_item_ptr->identifier.a_void == NULL) {
                num_lines = curses_num_lines(menu_item_ptr->str, width);
            } else {
                /* Add space for accelerator */
                num_lines = curses_num_lines(menu_item_ptr->str, width - 4);
            }
            menu_item_ptr->num_lines = num_lines;
            curline += num_lines;
            menu_item_ptr = menu_item_ptr->next_item;
            if ((curline > height) || ((curline > height - 2) &&
                                       (height == menu_max_height()))) {
                break;
            }
        }
        if (menu_item_ptr == NULL) {
            return FALSE;
        }
    }
    return TRUE;
}


/* Determine which entries go on which page, and total number of pages */

static void
menu_determine_pages(nhmenu *menu)
{
    int tmpline, num_lines;
    int curline = 0;
    int page_num = 1;
    nhmenu_item *menu_item_ptr = menu->entries;
    int width = menu->width;
    int height = menu->height;
    int page_end = height;


    if (strlen(menu->prompt) > 0) {
        curline += curses_num_lines(menu->prompt, width) + 1;
    }

    tmpline = curline;

    if (menu_is_multipage(menu, width, height)) {
        page_end -= 2;          /* Room to display current page number */
    }

    /* Determine what entries belong on which page */
    menu_item_ptr = menu->entries;

    while (menu_item_ptr != NULL) {
        menu_item_ptr->page_num = page_num;
        menu_item_ptr->line_num = curline;
        if (menu_item_ptr->identifier.a_void == NULL) {
            num_lines = curses_num_lines(menu_item_ptr->str, width);
        } else {
            /* Add space for accelerator */
            num_lines = curses_num_lines(menu_item_ptr->str, width - 4);
        }
        menu_item_ptr->num_lines = num_lines;
        curline += num_lines;
        if (curline > page_end) {
            page_num++;
            curline = tmpline;
            /* Move ptr back so entry will be reprocessed on new page */
            menu_item_ptr = menu_item_ptr->prev_item;
        }
        menu_item_ptr = menu_item_ptr->next_item;
    }

    menu->num_pages = page_num;
}


/* Determine dimensions of menu window based on term size and entries */

static void
menu_win_size(nhmenu *menu)
{
    int width, height, maxwidth, maxheight, curentrywidth, lastline;
    int maxentrywidth = 0;
    int maxheaderwidth = menu->prompt ? (int) strlen(menu->prompt) : 0;
    nhmenu_item *menu_item_ptr;

    if (program_state.gameover) {
        /* for final inventory disclosure, use full width */
        maxwidth = term_cols - 2; /* +2: borders assumed */
    } else {
        /* this used to be 38, which is 80/2 - 2 (half a 'normal' sized
           screen minus room for a border box), but some data files
           have been manually formatted for 80 columns (usually limited
           to 78 but sometimes 79, rarely 80 itself) and using a value
           less that 40 meant that a full line would wrap twice:
           1..38, 39..76, and 77..80 */
        maxwidth = 40; /* Reasonable minimum usable width */
        if ((term_cols / 2) > maxwidth) {
            maxwidth = (term_cols / 2); /* Half the screen */
        }
    }
    maxheight = menu_max_height();

    /* First, determine the width of the longest menu entry */
    for (menu_item_ptr = menu->entries; menu_item_ptr != NULL;
         menu_item_ptr = menu_item_ptr->next_item) {
        curentrywidth = (int) strlen(menu_item_ptr->str);
        if (menu_item_ptr->identifier.a_void == NULL) {
            if (curentrywidth > maxheaderwidth) {
                maxheaderwidth = curentrywidth;
            }
        } else {
            /* Add space for accelerator (selector letter) */
            curentrywidth += 4;
            if (menu_item_ptr->glyph != NO_GLYPH
                        && !iflags.vanilla_ui_behavior)
                curentrywidth += 2;
        }
        if (curentrywidth > maxentrywidth) {
            maxentrywidth = curentrywidth;
        }
    }

    /*
     * 3.6.3: This used to set maxwidth to maxheaderwidth when that was
     * bigger but only set it to maxentrywidth if the latter was smaller,
     * so entries wider than the default would always wrap unless at
     * least one header or separator line was long, even when there was
     * lots of space available to display them without wrapping.  The
     * reason to force narrow menus isn't known.  It may have been to
     * reduce the amount of map rewriting when menu is dismissed, but if
     * so, that was an issue due to excessive screen writing for the map
     * (output was flushed for each character) which was fixed long ago.
     */
    maxwidth = max(maxentrywidth, maxheaderwidth);
    if (maxwidth > term_cols - 2) {
        /* -2: space for left and right borders */
        maxwidth = term_cols - 2;
    }

    /* Possibly reduce height if only 1 page */
    if (!menu_is_multipage(menu, maxwidth, maxheight)) {
        menu_item_ptr = menu->entries;

        while (menu_item_ptr->next_item != NULL) {
            menu_item_ptr = menu_item_ptr->next_item;
        }

        lastline = (menu_item_ptr->line_num) + menu_item_ptr->num_lines;

        if (lastline < maxheight) {
            maxheight = lastline;
        }
    }

    /* avoid a tiny popup window; when it's shown over the endings of
       old messages rather than over the map, it is fairly easy for
       the player to overlook it, particularly when walking around and
       stepping on a pile of 2 items; also, multi-page menus need enough
       room for "(Page M of N) => " even if all entries are narrower
       than that; we specify same minimum width even when single page */
    menu->width = max(maxwidth, 25);
    menu->height = max(maxheight, 5);
}


/* Displays menu selections in the given window */

static void
menu_display_page(nhmenu *menu, WINDOW * win, int page_num)
{
    nhmenu_item *menu_item_ptr;
    int count, curletter, entry_cols, start_col, num_lines, footer_x;
    char *tmpstr;
    boolean first_accel = TRUE;

    int color = NO_COLOR;
#ifdef MENU_COLOR
    attr_t attr = A_NORMAL;
    boolean menu_color = FALSE;
#endif /* MENU_COLOR */

    /* Cycle through entries until we are on the correct page */

    menu_item_ptr = menu->entries;

    while (menu_item_ptr != NULL) {
        if (menu_item_ptr->page_num == page_num) {
            break;
        }
        menu_item_ptr = menu_item_ptr->next_item;
    }

    if (menu_item_ptr == NULL) {        /* Page not found */
        impossible("menu_display_page: attempt to display nonexistent page");
        return;
    }

    werase(win);

    if (menu->prompt && *menu->prompt) {
        num_lines = curses_num_lines(menu->prompt, menu->width);

        for (count = 0; count < num_lines; count++) {
            tmpstr = curses_break_str(menu->prompt, menu->width, count + 1);
            mvwprintw(win, count + 1, 1, "%s", tmpstr);
            free(tmpstr);
        }
    }

    /* Display items for current page */

    while (menu_item_ptr != NULL) {
        if (menu_item_ptr->page_num != page_num) {
            break;
        }
        if (menu_item_ptr->identifier.a_void != NULL) {
            if (menu_item_ptr->accelerator != 0) {
                curletter = menu_item_ptr->accelerator;
            } else {
                if (first_accel) {
                    curletter = menu_get_accel(TRUE);
                    first_accel = FALSE;
                    if (!menu->reuse_accels && (menu->num_pages > 1)) {
                        menu->reuse_accels = TRUE;
                    }
                } else {
                    curletter = menu_get_accel(FALSE);
                }
                menu_item_ptr->accelerator = curletter;
            }

            if (menu_item_ptr->selected) {
                curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, ON);
                mvwaddch(win, menu_item_ptr->line_num + 1, 1, '<');
                mvwaddch(win, menu_item_ptr->line_num + 1, 2, curletter);
                mvwaddch(win, menu_item_ptr->line_num + 1, 3, '>');
                curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, OFF);
            } else {
                curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
                mvwaddch(win, menu_item_ptr->line_num + 1, 2, curletter);
                curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
                mvwprintw(win, menu_item_ptr->line_num + 1, 3, ") ");
            }
        }
        entry_cols = menu->width;
        start_col = 1;

        if (menu_item_ptr->identifier.a_void != NULL) {
            entry_cols -= 4;
            start_col += 4;
        }
        if (menu_item_ptr->glyph != NO_GLYPH && !iflags.vanilla_ui_behavior) {
            unsigned special;   /*notused */

            mapglyph(menu_item_ptr->glyph, &curletter, &color, &special, 0, 0, 0);
            curses_toggle_color_attr(win, color, NONE, ON);
            mvwaddch(win, menu_item_ptr->line_num + 1, start_col, curletter);
            curses_toggle_color_attr(win, color, NONE, OFF);
            mvwaddch(win, menu_item_ptr->line_num + 1, start_col + 1, ' ');
            entry_cols -= 2;
            start_col += 2;
        }
#ifdef MENU_COLOR
        color = NONE;
        menu_color = iflags.use_menu_color && iflags.use_color &&
                     curses_get_menu_coloring(menu_item_ptr->str, &color, &attr);
        if (menu_color) {
            attr = curses_convert_attr(attr);
            if (color != NONE || attr != A_NORMAL) {
                curses_menu_color_attr(win, color, attr, ON);
            }
        } else {
            attr = menu_item_ptr->attr;
            if (color != NONE || attr != A_NORMAL) {
                curses_toggle_color_attr(win, color, attr, ON);
            }
        }
#endif /* MENU_COLOR */

        num_lines = curses_num_lines(menu_item_ptr->str, entry_cols);
        for (count = 0; count < num_lines; count++) {
            if (menu_item_ptr->str && *menu_item_ptr->str) {
                tmpstr = curses_break_str(menu_item_ptr->str, entry_cols, count + 1);
                mvwprintw(win, menu_item_ptr->line_num + count + 1, start_col, "%s", tmpstr);
                free(tmpstr);
            }
        }
#ifdef MENU_COLOR
        if (color != NONE || attr != A_NORMAL) {
            if (menu_color) {
                curses_menu_color_attr(win, color, attr, OFF);
            } else {
                curses_toggle_color_attr(win, color, attr, OFF);
            }
        }
#endif /* MENU_COLOR */

        menu_item_ptr = menu_item_ptr->next_item;
    }

    if (menu->num_pages > 1) {
        footer_x = menu->width - strlen("<- (Page X of Y) ->");
        if (menu->num_pages > 9) {      /* Unlikely */
            footer_x -= 2;
        }
        mvwprintw(win, menu->height, footer_x + 3, "(Page %d of %d)",
                  page_num, menu->num_pages);
        if (page_num != 1) {
            curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
            mvwaddstr(win, menu->height, footer_x, "<=");
            curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
        }
        if (page_num != menu->num_pages) {
            curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
            mvwaddstr(win, menu->height, menu->width - 2, "=>");
            curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
        }
    }
    curses_toggle_color_attr(win, DIALOG_BORDER_COLOR, NONE, ON);
    box(win, 0, 0);
    curses_toggle_color_attr(win, DIALOG_BORDER_COLOR, NONE, OFF);
    wrefresh(win);
}


static int
menu_get_selections(WINDOW * win, nhmenu *menu, int how)
{
    int curletter;
    int count = -1;
    int count_letter = '\0';
    int curpage = 1;
    int num_selected = 0;
    boolean dismiss = FALSE;
    char search_key[BUFSZ];
    nhmenu_item *menu_item_ptr = menu->entries;

    menu_display_page(menu, win, 1);

    while (!dismiss) {
        curletter = getch();

        if (curletter == ERR) {
            num_selected = -1;
            dismiss = TRUE;
        }

        if (curletter == '\033') {
            curletter = curses_convert_keys(curletter);
        }

        switch (how) {
        case PICK_NONE:
            if (menu->num_pages == 1) {
                if (curletter == KEY_ESC) {
                    num_selected = -1;
                } else {
                    num_selected = 0;
                }
                dismiss = TRUE;
                break;
            }
            break;
        case PICK_ANY:
            switch (curletter) {
            case MENU_SELECT_PAGE:
                (void) menu_operation(win, menu, SELECT, curpage);
                break;
            case MENU_SELECT_ALL:
                curpage = menu_operation(win, menu, SELECT, 0);
                break;
            case MENU_UNSELECT_PAGE:
                (void) menu_operation(win, menu, DESELECT, curpage);
                break;
            case MENU_UNSELECT_ALL:
                curpage = menu_operation(win, menu, DESELECT, 0);
                break;
            case MENU_INVERT_PAGE:
                (void) menu_operation(win, menu, INVERT, curpage);
                break;
            case MENU_INVERT_ALL:
                curpage = menu_operation(win, menu, INVERT, 0);
                break;
            }
            /* fall through */
        default:
            if (isdigit(curletter)) {
                count = curses_get_count(curletter - '0');
                touchwin(win);
                refresh();
                curletter = getch();
                if (count > 0) {
                    count_letter = curletter;
                }
            }
        }

        switch (curletter) {
        case KEY_ESC:
            num_selected = -1;
            dismiss = TRUE;
            break;
        case '\n':
        case '\r':
            dismiss = TRUE;
            break;
        case KEY_RIGHT:
        case KEY_NPAGE:
        case MENU_NEXT_PAGE:
        case ' ':
            if (curpage < menu->num_pages) {
                curpage++;
                menu_display_page(menu, win, curpage);
            } else if (curletter == ' ') {
                dismiss = TRUE;
                break;
            }
            break;
        case KEY_LEFT:
        case KEY_PPAGE:
        case MENU_PREVIOUS_PAGE:
            if (curpage > 1) {
                curpage--;
                menu_display_page(menu, win, curpage);
            }
            break;
        case KEY_END:
        case MENU_LAST_PAGE:
            if (curpage != menu->num_pages) {
                curpage = menu->num_pages;
                menu_display_page(menu, win, curpage);
            }
            break;
        case KEY_HOME:
        case MENU_FIRST_PAGE:
            if (curpage != 1) {
                curpage = 1;
                menu_display_page(menu, win, curpage);
            }
            break;
        case MENU_SEARCH:
            curses_line_input_dialog("Search for:", search_key, BUFSZ);

            refresh();
            touchwin(win);
            wrefresh(win);

            if (strlen(search_key) == 0) {
                break;
            }

            menu_item_ptr = menu->entries;

            while (menu_item_ptr != NULL) {
                if ((menu_item_ptr->identifier.a_void != NULL) &&
                    (strstri(menu_item_ptr->str, search_key))) {
                    if (how == PICK_ONE) {
                        menu_clear_selections(menu);
                        menu_select_deselect(win, menu_item_ptr, SELECT);
                        num_selected = 1;
                        dismiss = TRUE;
                        break;
                    } else {
                        menu_select_deselect(win, menu_item_ptr, INVERT);
                    }
                }

                menu_item_ptr = menu_item_ptr->next_item;
            }

            menu_item_ptr = menu->entries;
            break;
        default:
            if (how == PICK_NONE) {
                num_selected = 0;
                dismiss = TRUE;
                break;
            }
        }
        menu_item_ptr = menu->entries;
        while (menu_item_ptr != NULL) {
            if (menu_item_ptr->identifier.a_void != NULL) {
                if (((curletter == menu_item_ptr->accelerator) &&
                     ((curpage == menu_item_ptr->page_num) ||
                      (!menu->reuse_accels))) || ((menu_item_ptr->group_accel)
                                                  && (curletter ==
                                                      menu_item_ptr->
                                                      group_accel))) {
                    if (curpage != menu_item_ptr->page_num) {
                        curpage = menu_item_ptr->page_num;
                        menu_display_page(menu, win, curpage);
                    }

                    if (how == PICK_ONE) {
                        menu_clear_selections(menu);
                        menu_select_deselect(win, menu_item_ptr, SELECT);
                        if (count)
                            menu_item_ptr->count = count;
                        num_selected = 1;
                        dismiss = TRUE;
                        break;
                    } else if ((how == PICK_ANY) && (curletter == count_letter)) {
                        menu_select_deselect(win, menu_item_ptr, SELECT);
                        menu_item_ptr->count = count;
                        count = 0;
                        count_letter = '\0';
                    } else {
                        menu_select_deselect(win, menu_item_ptr, INVERT);
                    }
                }
            }
            menu_item_ptr = menu_item_ptr->next_item;
        }
    }

    if ((how == PICK_ANY) && (num_selected != -1)) {
        num_selected = 0;
        menu_item_ptr = menu->entries;

        while (menu_item_ptr != NULL) {
            if (menu_item_ptr->identifier.a_void != NULL) {
                if (menu_item_ptr->selected) {
                    num_selected++;
                }
            }
            menu_item_ptr = menu_item_ptr->next_item;
        }
    }

    return num_selected;
}


/* Select, deselect, or toggle selected for the given menu entry */

static void
menu_select_deselect(WINDOW * win, nhmenu_item *item, menu_op operation)
{
    int curletter = item->accelerator;

    if ((operation == DESELECT) || (item->selected && (operation == INVERT))) {
        item->selected = FALSE;
        mvwaddch(win, item->line_num + 1, 1, ' ');
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
        mvwaddch(win, item->line_num + 1, 2, curletter);
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
        mvwaddch(win, item->line_num + 1, 3, ')');
    } else {
        item->selected = TRUE;
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, ON);
        mvwaddch(win, item->line_num + 1, 1, '<');
        mvwaddch(win, item->line_num + 1, 2, curletter);
        mvwaddch(win, item->line_num + 1, 3, '>');
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, OFF);
    }

    wrefresh(win);
}


/* Perform the selected operation (select, unselect, invert selection)
on the given menu page.  If menu_page is 0, then perform opetation on
all pages in menu.  Returns last page displayed.  */

static int
menu_operation(WINDOW * win, nhmenu *menu, menu_op
               operation, int page_num)
{
    int first_page, last_page, current_page;
    nhmenu_item *menu_item_ptr = menu->entries;

    if (page_num == 0) {        /* Operation to occur on all pages */
        first_page = 1;
        last_page = menu->num_pages;
    } else {
        first_page = page_num;
        last_page = page_num;
    }

    /* Cycle through entries until we are on the correct page */

    while (menu_item_ptr != NULL) {
        if (menu_item_ptr->page_num == first_page) {
            break;
        }
        menu_item_ptr = menu_item_ptr->next_item;
    }

    current_page = first_page;

    if (page_num == 0) {
        menu_display_page(menu, win, current_page);
    }

    if (menu_item_ptr == NULL) {        /* Page not found */
        impossible("menu_display_page: attempt to display nonexistent page");
        return 0;
    }

    while (menu_item_ptr != NULL) {
        if (menu_item_ptr->page_num != current_page) {
            if (menu_item_ptr->page_num > last_page) {
                break;
            }

            current_page = menu_item_ptr->page_num;
            menu_display_page(menu, win, current_page);
        }

        if (menu_item_ptr->identifier.a_void != NULL) {
            menu_select_deselect(win, menu_item_ptr, operation);
        }

        menu_item_ptr = menu_item_ptr->next_item;
    }

    return current_page;
}


/* Set all menu items to unselected in menu */

static void
menu_clear_selections(nhmenu *menu)
{
    nhmenu_item *menu_item_ptr = menu->entries;

    while (menu_item_ptr != NULL) {
        menu_item_ptr->selected = FALSE;
        menu_item_ptr = menu_item_ptr->next_item;
    }
}


/* Get the maximum height for a menu */

static int
menu_max_height(void)
{
    return term_rows - 2;
}

#ifdef MENU_COLOR
boolean
curses_get_menu_coloring(const char *line, int *color, attr_t*attr)
{
    struct menucoloring *tmpmc;
    boolean foundcolor = FALSE, foundattr = FALSE;
    char str[BUFSZ];

    strcpy(str, line);
    strip_brackets(str);

    if (iflags.use_menu_color && iflags.use_color) {
        for (tmpmc = menu_colorings; tmpmc; tmpmc = tmpmc->next) {
            if (regex_match(str, tmpmc->match)) {
                *color = tmpmc->color;
                *attr = tmpmc->attr;
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif
