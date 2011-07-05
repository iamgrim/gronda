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
 *  main.c
 *  purpose : main program loop 
 *            editor initialisation and finalisation
 *  authors : James Garlick
 */
#include <sys/time.h>
#include <string.h>

#include "editor.h"

editor_t *e;

void editor_init ()
{
	e = ALLOC (editor_t);

	if (e == NULL)
	{
		debug ("(main.c) Unable to allocate memory for editor");
		exit (1);
	}

	e->occupied_window = EDIT_WINDOW;

	/* new pad */
	e->pad_head = ALLOC (pad_t);
	if (e->pad_head == NULL)
	{
		debug ("(main.c) Unable to allocate memory for e->pad_head");
		exit (1);
	}
	e->pad_head->line_head = LINE_new_lines ();
	e->pad_head->curs_x = 1;
	e->pad_head->curs_y = 1;
	e->pad_head->flags |= FILE_WRITE;

	display_max_pad_dim (&(e->pad_head->width), &(e->pad_head->height));

	e->cpad = e->pad_head;

	e->input = ALLOC (input_t);
	if (e->input == NULL)
	{
		debug ("(main.c) Unable to allocate memory for e->input");
		exit (1);
	}

	e->input->offset = 0;
	e->input->curs_x = 0;
	e->flags |= INSERT;

	KEY_init ();
}

void
editor_fini ()
{
	/* TODO */
	FREE (e);
}


void
resize ()
{
	pad_t  *mover;

	for (mover = e->pad_head; mover; mover = mover->next)
	{
		display_max_pad_dim (&(mover->width), &(mover->height));

		if (mover->curs_x > mover->width)
			mover->curs_x = mover->width;

		if (mover->curs_y > mover->height)
			mover->curs_y = mover->height;
	}

	if (e->dm)
		free (e->dm);

	e->dm = (int *) calloc ((e->pad_head->width + 1) * (e->pad_head->height + 1), 4);
	if (!e->dm)
	{
		debug ("Out of memory");
		/* TODO */
	}

	e->redraw = DIRTY_ALL | TITLE | STATS | COMMAND | OUTPUT;
	redraw ();
}


int
main (int argc, char **argv)
{
	int     key, mods;
	struct itimerval value;
	char   *s;
	char   *home;
	char    tmp[256];
	int     lines;
	int     a;
	char   *args[2];

	sig_init ();

	editor_init ();

	create_local_config ();

	display_init ();

	add_base_commands ();

	/* handle command line args before plugin load */
	for (a = 1; a < argc; a++)
	{
		s = argv[a];

		if (*s == '-')
		{
			s++;
			if (*s == 'd' || !strcasecmp (s, "-debug"))
				e->flags |= DEBUG;
		}
	}

	/* read a config file */
	home = getenv ("HOME");

	if (home != NULL)
	{
		sprintf (tmp, "%s/.gronda/startup", home);

		lines = parse_commandfile (tmp);
	}

	if (lines == -1)
	{
		lines = parse_commandfile ("grondarc");

		if (lines == -1)
			sig_cleanexit
				("Can't load configuration file grondarc\nThis is essential to the functioning of %s.\nAborting.\n",
				 EDITOR_NAME);
	}

	output_message ("Config file: %d lines processed", lines);

	/* handle command line args after plugin load */
	for (a = 1; a < argc; a++)
	{
		s = argv[a];

		if (*s == '-')
		{
			s++;
			if (*s == 'v' || !strcasecmp (s, "-version"))
			{
				sig_cleanexit ("%s (v%s)\nhttp://gronda.sourceforge.net\n\n",
							   EDITOR_NAME, EDITOR_VERSION);
			}
			else if (*s == 'h' || !strcasecmp (s, "-help"))
			{
				sig_cleanexit
					("Usage: ge [--debug] [--version] [--help] [filename]\n");
			}
		}
		else
		{
			args[0] = "ce";
			args[1] = s;

			cmd_ce (2, args);
		}
	}

	e->redraw |= OUTPUT;
	redraw ();

	/* start timer */
	setitimer (ITIMER_REAL, &value, NULL);

	/* main loop */
	while (1)
	{
		key = mods = 0;

		/* blocks until event or timer tick */
		display_nextevent (&key, &mods);

		if (key > 0 && key != 410)
		{
			if (key < 500)
				debug ("KEY %d (%c) MODS %d", key, (char) key, mods);

			if (e->occupied_window == COMMAND_WINDOW)
			{
				switch (key)
				{
				case K_ESC:
					e->occupied_window = EDIT_WINDOW;
					redraw ();
					break;
				case K_ENTER:
					if (*(e->input->buffer))
					{
						debug ("To Parser: %s", e->input->buffer);
						parse ("%s", e->input->buffer);
						memset (e->input->buffer, 0, BUFFER_SIZE);
						e->input->curs_x = 0;
						e->input->offset = 0;
						e->redraw |= COMMAND;

						redraw ();
					}
					else
					{
						e->occupied_window = EDIT_WINDOW;
						redraw ();
					}

					break;
					/* backspace */
				case K_BS:
					if (e->input->curs_x)
					{
						e->input->curs_x--;
						e->input->buffer[e->input->curs_x + e->input->offset] =
							0;
						e->redraw |= COMMAND;
						redraw ();
					}
					break;
				case K_SQUOTE:
					e->input->buffer[e->input->curs_x + e->input->offset] =
						'\'';
					e->input->curs_x++;
					e->redraw |= COMMAND;
					redraw ();
					break;
				case K_DQUOTE:
					e->input->buffer[e->input->curs_x + e->input->offset] =
						'\"';
					e->input->curs_x++;
					e->redraw |= COMMAND;
					redraw ();
					break;
				default:
					if (key >= 32 && key <= 126)
					{
						e->input->buffer[e->input->curs_x + e->input->offset] =
							(char) key;
						e->input->curs_x++;
						e->redraw |= COMMAND;
						redraw ();
					}
				}
			}
			else if (e->occupied_window == EDIT_WINDOW)
			{
				s = KEY_find (key);

				if (s)
				{
					debug ("Found keydef for %d : '%s'", key, s);
					parse ("%s", s);

					redraw ();
				}
				else
					debug ("No keydef defined for (%d)\n", key);
			}

			if (key == -1)
				debug ("An unknown key was pressed");
			else if (key > 0 && key < 32)
				debug ("Key press ( %d )", key);
			else if (key > 500)
				debug ("%s was pressed", special_keynames[key - 500]);

		}
	}

	return 0;
}
