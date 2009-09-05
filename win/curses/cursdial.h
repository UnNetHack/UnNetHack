#ifndef CURSDIAL_H
#define CURSDIAL_H


/* Global declarations */

void curses_line_input_dialog(const char *prompt, char *answer, int buffer);

int curses_character_input_dialog(const char *prompt, const char *choices, CHAR_P def);

int curses_ext_cmd(void);

void curses_create_nhmenu(winid wid);

void curses_add_nhmenu_item(winid wid, const ANY_P *identifier,
 CHAR_P accelerator, CHAR_P group_accel, int attr, const char *str,
 BOOLEAN_P presel);

void curses_finalize_nhmenu(winid wid, const char *prompt);

int curses_display_nhmenu(winid wid, int how, MENU_ITEM_P **_selected);

boolean curses_menu_exists(winid wid);

void curses_del_menu(winid wid);


/* Private declarations */

typedef struct nhmi
{
    winid wid;  /* NetHack window id */
    anything identifier; /* Value returned if item selected */
    CHAR_P accelerator;  /* Character used to select item from menu */
    CHAR_P group_accel; /* Group accelerator for menu item, if any */
    int attr;  /* Text attributes for item */
    const char *str;  /* Text of menu item */
    BOOLEAN_P presel; /* Whether menu item should be preselected */
    boolean selected;   /* Whether item is currently selected */
    int page_num;   /* Display page number for entry */
    int line_num;   /* Line number on page where entry begins */
    int num_lines;  /* Number of lines entry uses on page */
    struct nhmi *prev_item;    /* Pointer to previous entry */
    struct nhmi *next_item;    /* Pointer to next entry */
} nhmenu_item;

typedef struct nhm
{
    winid wid;  /* NetHack window id */
    const char *prompt;  /* Menu prompt text */
    nhmenu_item *entries;   /* Menu entries */
    int num_entries;          /* Number of menu entries */
    int num_pages;            /* Number of display pages for entry */
    int height;               /* Window height of menu */
    int width;                /* Window width of menu */
    struct nhm *prev_menu;    /* Pointer to previous entry */
    struct nhm *next_menu;    /* Pointer to next entry */
} nhmenu;

typedef enum menu_op_type
{
    SELECT,
    DESELECT,
    INVERT
} menu_op;

static nhmenu *get_menu(winid wid);

static char menu_get_accel(boolean first);

static void menu_determine_pages(nhmenu *menu);

static boolean menu_is_multipage(nhmenu *menu, int width, int height);

static void menu_win_size(nhmenu *menu);

static void menu_display_page(nhmenu *menu, WINDOW *win, int page_num);

static int menu_get_choices(nhmenu *menu, int how);

static int menu_get_selections(WINDOW *win, nhmenu *menu, int num_choices, int how);

static void menu_select_deselect(WINDOW *win, nhmenu_item *item, menu_op operation);

static int menu_operation(WINDOW *win, nhmenu *menu, menu_op operation,
 int page_num);


#endif  /* CURSDIAL_H */
