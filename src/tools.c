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
 *  tools.c
 *  purpose : various utility functions
 *  authors : James Garlick
 */
#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "editor.h"

void    yy_scan_string (char *);
int     yylex (void);

void debug (char *str, ...)
{
	va_list argp;

	if (!e || e->flags & DEBUG)
	{
		va_start (argp, str);
		fprintf (stderr, "[stderr] ");
		vfprintf (stderr, str, argp);
		fprintf (stderr, "\n");
		va_end (argp);
	}
}

void parse (char *format, ...)
{
	char   *str;
	va_list argp;

	va_start (argp, format);
	vasprintf (&str, format, argp);
	va_end (argp);

	debug ("sending %s", str);
	yy_scan_string (str);
	yylex ();
	debug ("sent");
	free (str);
}

void output_message (char *str, ...)
{
	output_msg_t *new;

	va_list argp;

	new = ALLOC (output_msg_t);

	va_start (argp, str);

	vasprintf (&(new->message), str, argp);

	new->next = e->messages;
	e->messages = new;

	e->redraw |= OUTPUT;
	debug ("OUTPUT %s", new->message);

	va_end (argp);
}

void output_message_c (char *title, char *format, ...)
{
	char   *str;
	va_list argp;

	va_start (argp, format);
	vasprintf (&str, format, argp);
	va_end (argp);

	output_message ("(%s) %s (%s)", title, str, title);
	free (str);
}

#ifdef HPUX

/* 
Memory allocating vsprintf (from FreeBSD)
*/

#ifdef __STDC__
#define PTR void *
#else
#define PTR char *
#endif

unsigned long strtoul ();
//char *malloc ();

static int
int_vasprintf (result, format, args)
	 char  **result;
	 const char *format;
	 va_list *args;
{
	const char *p = format;
	/* Add one to make sure that it is never zero, which might cause malloc
	   to return NULL.  */
	int     total_width = strlen (format) + 1;
	va_list ap;

	memcpy ((PTR) & ap, (PTR) args, sizeof (va_list));

	while (*p != '\0')
	{
		if (*p++ == '%')
		{
			while (strchr ("-+ #0", *p))
				++p;
			if (*p == '*')
			{
				++p;
				total_width += abs (va_arg (ap, int));
			}
			else
				total_width += strtoul ((char *) p, (char **) &p, 10);
			if (*p == '.')
			{
				++p;
				if (*p == '*')
				{
					++p;
					total_width += abs (va_arg (ap, int));
				}
				else
					total_width += strtoul ((char *) p, (char **) &p, 10);
			}
			while (strchr ("hlL", *p))
				++p;
			/* Should be big enough for any format specifier except %s.  */
			total_width += 30;
			switch (*p)
			{
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			case 'c':
				(void) va_arg (ap, int);
				break;
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				(void) va_arg (ap, double);
				break;
			case 's':
				total_width += strlen (va_arg (ap, char *));
				break;
			case 'p':
			case 'n':
				(void) va_arg (ap, char *);
				break;
			}
		}
	}
	*result = (char *) malloc (total_width);
	if (*result != NULL)
		return vsprintf (*result, format, *args);
	else
		return 0;
}

int
vasprintf (result, format, args)
	 char  **result;
	 const char *format;
	 va_list args;
{
	return int_vasprintf (result, format, &args);
}
#endif

int parse_commandfile (char *path)
{
	FILE   *f;
	char    str[4096];
	char   *c;
	int     lines;

	f = fopen (path, "r");
	if (!f)
		return -1;

	lines = 0;
	while (fgets (str, 4096, f))
	{
		c = str;
		while (*c != '\n' && *c)
			c++;
		*c = 0;

		c = str;
		while (*c == ' ')
			c++;

		if (*c && *c != '#')
		{
			parse ("%s", c);
			lines++;
		}
	}

	fclose (f);

	return lines;
}

void create_dir (char *name)
{
	struct stat buf;

	if (stat (name, &buf) == -1)
	{
		if (errno == ENOENT)
		{
			mkdir (name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		}
		else
		{
			perror (name);
			exit (1);
		}
	}
}

void create_local_config ()
{
	char  temp[256];
    char *home;

	home = getenv ("HOME");

	if (home == NULL)
		return;


	sprintf (temp, "%s/.gronda", home);
	create_dir (temp);

	sprintf (temp, "%s/.gronda/buffer", home);
	create_dir (temp);
}

/* get actual string position on a line in characters,   */
/* from cursor co-ordinate and pad offset, accounting    */
/* for the posibility of tabs                            */
/* if position x is inside a tab, intab is set to the    */
/* number of characters offset from the start of the tab */
int get_string_pos (int x, char *str, int *intab)
{
	int  a;
	char *c;
	int  correction;

	x--;

	a = 0;
	c = str;
	correction = 0;

	*intab = 0;

	while (a < x && *c)
	{
		if (*c == '\t')
		{
			(*intab)++;
			if (*intab == 4) /* replace with next tab stop */
			{
				*intab = 0;
				correction += 3;
				c++;
			}
		}
		else
			c++;

		a++;
	}

	return (x - correction) - *intab;
}

/* get the horizontal cursor position for the given string */
/* position on a line, accounting for tabs etc             */
int get_curs_pos (int x, line_t *l)
{
	int  len;
	char *c;
	int  a;

	if (l == NULL || l->str == NULL)
		return x;

	c   = l->str->data;
	a   = 0;
	len = 0;

	while (a < x)
	{
		if (*c == '\t')
			len += 4; /* this is temporary, replace with tabstops */
		else
			len++;

		a++;
		if (*c)
			c++;
	}

	return len + 1;
}

/* attempt to set the cursor position to the given y, x position */
/* if we end up in the middle of a wide character such as a tab, */
/* adjust the cursor position either left or right depending on  */
/* the value of 'adjust'                                         */
void cursor_set_pos (pad_t *p, int curs_y, int curs_x, int adjust)
{
	line_t *l;
	char   *c;
	int    a;
	int    width;
	int    offset;
	int    x;

	l = LINE_get_line_at (p, p->offset_y + curs_y);

	p->curs_y = curs_y;
	p->curs_x = curs_x;

	if (l != NULL && l->str != NULL)
	{
		c = l->str->data;
		x = p->offset_x + curs_x;

		a = 0;
		while (*c && a < x - 1)
		{
			if (*c == '\t')
			{
				width = 4;
				a += width; /* TODO: tab stops */
			}
			else
				a++;

			if (a < x) c++;
		}

		if (*c)
		{
			offset = x - a - 1;
			if (offset != 0)
			{
				offset += width;
				if (adjust == ADJUST_LEFT)
					p->curs_x -= offset;
				else /* adjust == ADJUST_RIGHT */
					p->curs_x += (width - offset);
			}
		}
	}

}


void pad_modified (pad_t *pad)
{
	if (!(pad->flags & MODIFIED))
	{
		pad->flags |= MODIFIED;
		e->redraw |= STATS;
	}
}


