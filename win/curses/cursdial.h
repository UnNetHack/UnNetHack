#ifndef CURSDIAL_H
#define CURSDIAL_H

# ifdef MENU_COLOR_REGEX
#  include <regex.h>
# endif


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



#endif  /* CURSDIAL_H */
