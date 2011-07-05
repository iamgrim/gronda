#ifndef __MENU_H
#define __MENU_H

typedef struct menu_item_s
{
	char   *text;
	char    quick_key;
	void    (*handler) ();

	struct menu_item_s *next;
}
menu_item_t;

typedef struct menu_s
{
	char   *title;
	menu_item_t *item_head;
}
menu_t;

menu_t *menu_alloc (char *, ...);
void    menu_free (menu_t *);
void    menu_add_item (menu_t *, char *, char, void (*)());

#endif
