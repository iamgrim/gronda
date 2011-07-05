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
 *  keydefs.c
 *  purpose : storage and retreival of key definitions
 *  authors : James Garlick
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"

keydef_t *root;

char   *special_keynames[] =
	{ "up", "down", "left", "right", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
"f8", "f9", "f10", "f11", "f12", "ins", "del", "bs", "home", "end", "pgup", "pgdown", "esc",
"enter", "tab", "squote", "dquote", 0 };

void
insert (keydef_t * root, int code, char *def)
{
	keydef_t **mover;
	int     r, exists = 0;
	int     nleft;

	nleft = code << ((sizeof (int) * 8) - 5);
	code = code >> 5;
	code = code | nleft;

	mover = &root;

	if (root->code == 0)
	{
		exists = 1;
	}

	while (*mover && exists == 0)
	{
		r = code - (*mover)->code;

		if (r > 0)
		{
			mover = &(*mover)->r;
		}
		else if (r < 0)
		{
			mover = &(*mover)->l;
		}
		else
		{
			exists = 1;
		}
	}

	if (!exists)
	{
		if ((*mover = ALLOC (keydef_t)) == NULL)
		{
			debug ("malloc error");
			exit (1);
		}
	}
	else if ((*mover)->def)
		FREE ((*mover)->def);

	(*mover)->def = (char *) calloc (1, strlen (def) + 2);
	if ((*mover)->def == NULL)
	{
		debug ("malloc error");
		exit (1);
	}

	strcpy ((*mover)->def, def);

	(*mover)->code = code;
	(*mover)->l = (*mover)->r = NULL;
}

void
KEY_init ()
{
	char    k[10];
	char    es[80];
	int     a;

	if ((root = ALLOC (keydef_t)) == NULL)
	{
		debug ("malloc error\n");
		exit (1);
	}

	root->code = 0;
	root->l = root->r = NULL;
	root->def = NULL;

	/* default key bindings */

	for (a = 32; a < 127; a++)
	{
		sprintf (k, "%c", (char) a);

		if (a == 92)
			sprintf (es, "er 92");
		else
			sprintf (es, "es\'%c\'", (char) a);

		KEY_define (k, es);
	}

	KEY_define ("squote", "es\"\'\"");
	KEY_define ("dquote", "es\'\"\'");

	KEY_define ("esc", "tdm");
	KEY_define ("enter", "en");
	KEY_define ("tab", "er 9");

	KEY_define ("up", "au");
	KEY_define ("down", "ad");
	KEY_define ("left", "al");
	KEY_define ("right", "ar");

}

void
KEY_debug (keydef_t * node)
{
	if (!node)
		return;

	KEY_debug (node->l);

	debug ("Node value : %d %s\n", node->code, node->def);

	KEY_debug (node->r);
}

int
KEY_define (char *key, char *def)
{
	int     key_code, a, done;

	if (strlen (key) > 1)
	{
		a = done = 0;

		while (special_keynames[a] && !done)
		{
			if (!strcasecmp (key, special_keynames[a]))
			{
				insert (root, 500 + a, def);
				done = 1;
			}
			a++;

		}
		if (!done)
		{
/*                        set_error( G_ERROR_NO_SUCH_KEY );*/
			debug ("Unknown key %s", key);
			return -1;
		}

	}
	else
	{
		key_code = (int) *key;
		insert (root, key_code, def);
	}

	return 0;
}

char   *
KEY_find (int code)
{
	keydef_t *mover;
	int     r, nleft;

	nleft = code << ((sizeof (int) * 8) - 5);
	code = code >> 5;
	code = code | nleft;

	for (mover = root; mover != NULL;)
	{
		r = code - ((mover->code));

		if (r < 0)
			mover = mover->l;
		else if (r > 0)
			mover = mover->r;
		else
			return mover->def;
	}

	return NULL;
}
