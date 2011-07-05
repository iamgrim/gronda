/*  Gronda/Ce
 *  Copyright (C) 2001-2003 James Garlick and Graeme Jefferis
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  cmd_navigation.c
 *  purpose : Window and pane movement commands
 *  authors : James Garlick
 */

#include "editor.h"

void wc_menu_handler (char *text, int index)
{
	if (index == 1)
	{
		cmd_pw (0, 0);
	}

	if (index != 3)
		sig_cleanexit ("%s %s\n", EDITOR_NAME, EDITOR_VERSION);
}

void cmd_wc (int argc, char *argv[])
{
	menu_t *m;

	if (e->cpad->flags & MODIFIED)
	{
		/* TODO: use a pre-allocated menu */
		m = menu_alloc ("File modified; okay to quit?");

		menu_add_item (m, "Save and quit",        's', wc_menu_handler);
		menu_add_item (m, "Quit without saving",  'q', wc_menu_handler);
		menu_add_item (m, "Return to the editor", 'r', wc_menu_handler);

		display_do_menu (m);

		menu_free (m);
	}
	else
		sig_cleanexit ("%s %s\n", EDITOR_NAME, EDITOR_VERSION);
}

void cmd_tdm (int argc, char *argv[])
{
	e->occupied_window = COMMAND_WINDOW_REQUEST;
}


