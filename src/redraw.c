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
 *  redraw.c
 *  purpose : Setting up display modifiers and high-level redraw
 *            Basic implementation of syntax highlighting for testing purposes
 *  authors : James Garlick
 */
#include <string.h>

#include "editor.h"

void syntax_highlight_test ();

void redraw ()
{
	pad_t  *p = e->cpad;
	int     a;
	line_t *l;

	int start_y;
	int start_x;
	int end_y;
	int end_x;
	int len;
	int end;

	if (p->echo)
	{
		get_region (p->echo, &start_y, &start_x, &end_y, &end_x);

		start_y -= p->offset_y + 1;
		start_x -= p->offset_x + 1;
		end_y   -= p->offset_y + 1;
		end_x   -= p->offset_x + 1;
	}

	/* redraw title */
	if (e->redraw & TITLE)
		display_redraw_title ();

	/* redraw stats */
	if (e->redraw & STATS)
		display_redraw_stats ();

	/* redraw output window */
	if (e->redraw & OUTPUT)
		display_redraw_output ();

	/* redraw command window */
	if (e->redraw & COMMAND)
		display_redraw_command ();

	/* redraw lines */
	l = LINE_get_line_at (p, p->offset_y + 1);
	if (l == NULL)
		l = p->line_head;

	dm_clear_all ();
	/* syntax_highlight_test (); */

	for (a = 0; a < p->height; a++)
	{
		if (l != p->line_head)
		{
			if (l->str != NULL)
				len = get_curs_pos (strlen (l->str->data), l) - p->offset_x;
			else
				len = 1;
		}
		else
			len = 1;

		/* region highlight */
		if (p->echo == REGION_LINEAR)
		{
			if (a == start_y)
			{
				if (!(start_y == end_y && start_x == end_x))
				{
					if (start_x < len)
					{
						dm_set (a, start_x, INVERT_ON);
	
						if (start_y == end_y)
						{
							end = (len < end_x) ? len : end_x;
							dm_set (a, end, INVERT_OFF);
						}
						else
							dm_set (a, len, INVERT_OFF);
					}
				}
			} 
			else if (a > start_y && a < end_y)
			{
				dm_set (a, 0, INVERT_ON);
				dm_set (a, len, INVERT_OFF);

			}
			else if (a == end_y)
			{
				if (end_x != 0)
				{
					dm_set (a, 0, INVERT_ON);
	
					end = (len < end_x) ? len : end_x;
					dm_set (a, end, INVERT_OFF);
				}
			}
		}
		else if (p->echo == REGION_RECT)
		{
			if (!(start_x == end_x) && a >= start_y && a <= end_y)
			{
				dm_set (a, start_x, INVERT_ON);
				dm_set (a, end_x,   INVERT_OFF);
			}
		}

		display_redraw_line (a, l);

		if (l != p->line_head)
			l = l->next;
	}

	e->redraw = 0;
	memset (p->line_redraw, p->height, CLEAN);

	display_redraw_curs ();
}


void dm_set (int row, int col, int mod)
{
	if (row < 0) row = 0;
	if (row > e->pad_head->height) row = e->pad_head->height;
	if (col < 0) col = 0;
	if (col > e->pad_head->width) col = e->pad_head->width;

	*(e->dm + (row * (e->pad_head->width + 1)) + col) |= mod;
}

void dm_clear_all ()
{
	memset (e->dm, 0, (e->cpad->height + 1) * (e->cpad->width + 1) * 4);
}

/*
Types        1
Keywords     2
Literals     3
Operands     4
Precompiler  5
Comment      6
*/

char   *c_s1[] =
	{ "auto", "const", "enum", "extern", "register", "static", "struct",
	"typedef", "union", "volatile", "char", "double", "float", "int", "long",
	"short", "signed", "unsigned", "void", 0
};

char   *c_s2[] =
	{ "break", "case", "continue", "default", "do", "else", "for", "goto",
	"if", "return", "sizeof", "switch", "while", 0
};

static void highlight_words (char *line, int line_no, char **search, int highlight)
{
	pad_t  *p = e->cpad;
	char   *ptr;

	while (*search)
	{
		ptr = strstr (line, *search);

		while (ptr && ptr < line + p->width)
		{
			dm_set (line_no, (int) (ptr - line), highlight);
			dm_set (line_no, (int) (ptr - line) + strlen (*search), SYNTAX0);

			ptr = strstr (ptr + 1, *search);
		}
		search++;
	}
}

void syntax_highlight_test ()
{
	pad_t  *p = e->cpad;
	int     a;
	line_t *l;
	char   *s;

	/* syntax highlighting test */
	l = LINE_get_line_at (p, p->offset_y + 1);
	if (l == NULL)
		l = p->line_head;

	for (a = 0; a < p->height; a++)
	{
		if (l != p->line_head)
		{
			if (l->str == NULL || strlen (l->str->data) < (size_t)p->offset_x)
				continue;

			s = l->str->data;
			s += p->offset_x;

			highlight_words (s, a, c_s1, SYNTAX1);
			highlight_words (s, a, c_s2, SYNTAX2);

			l = l->next;
		}
	}
}
