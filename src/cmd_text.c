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
 *  cmd_text.c
 *  purpose : Text manipulation commands
 *  authors : Graeme Jefferis, James Garlick
 */
#include <signal.h>
#include <string.h>
#include <math.h>

#include "editor.h"

#define PADSTEP (8)


/* ES: Enter String */
void cmd_es (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int     xpos;
	int     ypos;
	line_t *cline;
	int     intab;

	if (!(e->cpad->flags & FILE_WRITE))
	{
		output_message_c ("es", "Pad is read-only.");
		return;
	}

	if (argc < 2)
	{
		output_message_c ("es", "Missing string argument");
		return;
	}

	ypos = pad->curs_y + pad->offset_y;

	if (ypos > pad->line_count)
		line_grow_pad (pad, ypos);

	cline = LINE_get_line_at (pad, ypos);
	if (cline->str == NULL)
		cline->str = string_alloc ("");

	xpos = get_string_pos (pad->curs_x + pad->offset_x, cline->str->data, &intab);

	if (e->flags & INSERT)
		string_insert (cline->str, xpos, "%s", argv[1]);
	else
		string_overwrite (cline->str, xpos, "%s", argv[1]);

/*	string_debug (cline->str);*/

	/* horizontal cursor movement */
	pad->curs_x = get_curs_pos ((xpos - pad->offset_x) + strlen (argv[1]), cline);

	while (pad->curs_x > pad->width)
	{
		pad->offset_x += PADSTEP;
		pad->curs_x   -= PADSTEP;
	}

	pad_modified (pad);
}

void cmd_er (int argc, char *argv[])
{
	int  i;
	char *args[2];
	char tmp[2];

	if (!(e->cpad->flags & FILE_WRITE))
	{
		output_message_c ("er", "Pad is read-only.");
		return;
	}

	if (argc < 2)
	{
		output_message_c ("er", "Missing argument");
		return;
	}

	i = atoi (argv[1]);

	if (i < 1 || i > 255)
		output_message_c ("er", "Value out of the range 1 to 255");
	else
	{
		tmp[0] = (char) i;
		tmp[1] = '\0';

		args[0] = "es";
		args[1] = tmp;
		cmd_es (2, args);
	}
}

/* EN: Enter Newline */
void cmd_en (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int     xpos;
	int     ypos;
	line_t *cline;
	line_t *newline;
	char   *extratext;
	int    intab;

	if (!(e->cpad->flags & FILE_WRITE))
	{
		output_message_c ("en", "Pad is read-only.");
		return;
	}

	ypos = pad->curs_y + pad->offset_y;

	if (ypos > pad->line_count)
		line_grow_pad (pad, ypos);

	LINE_insert (pad, ypos, "");
	cline = LINE_get_line_at (pad, ypos);

	if (cline->str != NULL)
	{
		xpos = get_string_pos (pad->curs_x + pad->offset_x, cline->str->data, &intab);

		if ((size_t)xpos < strlen (cline->str->data))
		{
			/* We're 'inside' the string. Get the text on the
			 * right, and truncate the current string. Then put
			 * the 'extra' text on the line we've inserted or
			 * allocated.
			 */

			extratext = cline->str->data + xpos;
			newline = LINE_get_line_at (pad, ypos + 1);
			newline->str = string_alloc ("%s", extratext);
			string_truncate (cline->str, xpos);
		}
	}

	pad->curs_x   = 1;
	pad->offset_x = 0;
	if (pad->curs_y == pad->height)
	{
		pad->offset_y++;
	}
	else
	{
		pad->curs_y++;
	}

	pad_modified (pad);
}

/* ED: Edit Delete ('delete') */
void cmd_ed (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int    xpos;
	int    ypos;
	line_t *cline;
	line_t *cline2;
	int    intab;
	int    len;

	if (!(e->cpad->flags & FILE_WRITE))
	{
		output_message_c ("ed", "Pad is read-only.");
		return;
	}

	ypos = pad->curs_y + pad->offset_y;

	if (ypos > pad->line_count)
	{
		line_grow_pad (pad, ypos);
		pad_modified (pad);
		return;
	}

	cline = LINE_get_line_at (pad, ypos);

	len = 0;

	if (cline->str != NULL)
	{
		xpos = get_string_pos (pad->curs_x + pad->offset_x, cline->str->data, &intab);

		len = strlen (cline->str->data);
	}

	if (xpos < len)
	{
		string_remove (cline->str, xpos, 1);
	}
	else
	{
		cline2 = LINE_get_line_at (pad, ypos + 1);
		if (cline2 != NULL)
		{
			if (cline2->str != NULL)
			{
				if (cline->str == NULL)
					cline->str = string_alloc ("");

				if (cline->str)
					string_insert (cline->str, xpos, "%s", cline2->str->data);
			}

			LINE_remove (pad, cline2);
		}
	}

	pad_modified (pad);
}

/* EE: Edit Erase ('backspace') */
void cmd_ee (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int wordwrap = 0;

	if (!(e->cpad->flags & FILE_WRITE))
	{
		output_message_c ("ee", "Pad is read-only.");
		return;
	}

	if (argc > 1 && strcmp (argv[1], "-w") == 0)
		wordwrap = 1;

	if ((pad->curs_x + pad->offset_x) == 1 
		&& (!wordwrap || (pad->curs_y + pad->offset_y == 1)))
		return;

	cmd_al (argc, argv);
	cmd_ed (0, NULL);
}


/* EI: Edit Insert (toggle insert mode) */
void cmd_ei (int argc, char *argv[])
{
	int     i;
	e->flags ^= INSERT;
	if (argc > 1)
	{
		for (i = 1; i <= argc; i++)
		{
			if (strcmp (argv[i], "-on") == 0)
			{
				e->flags |= INSERT;
				return;
			}
			else if (strcmp (argv[i], "-off") == 0)
			{
				e->flags &= (~INSERT);
				return;
			}
		}
	}
	e->redraw |= STATS;
}

