%{
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include "editor.h"

	int buffersize = 16;
	int bufferinc = 32;
	char *buffer;
	char *s;

	char *quote;

	char *vargs[32];
	int i = 0;
%}

ws         [[:blank:]]+
optws		{ws}?
digit		[0-9]
number		[+-]?{digit}*"."?{digit}+
uint		{digit}+

punct		[\\\-[\]\.\,/<>?:|{}!@#$%^&*()_`~]
word		([[:alpha:]]|{punct})([[:alnum:]]|{punct})*

delimiter	[;]
quote		[\'\"]

range		^(\{{optws}{uint}?{optws}\,{optws}{uint}?{optws}\})

%x STRING

%%

{quote}		{
			BEGIN STRING;
			buffer = malloc(buffersize * sizeof(char *));
			s = buffer;
			quote = strdup(yytext);
		}
<STRING>\\n	{
			int str_len = s - buffer;	/* because there's no 0-terminator yet */
			if (str_len >= buffersize)
			{
				buffer = realloc(buffer, buffersize + bufferinc);
				buffersize += bufferinc;
				s = buffer + str_len;
			}
			*s++ = '\n';
		}
<STRING>\\t	{
			int str_len = s - buffer;	/* because there's no 0-terminator yet */
			if (str_len >= buffersize)
			{
				buffer = realloc(buffer, buffersize + bufferinc);
				buffersize += bufferinc;
				s = buffer + str_len;
			}
			*s++ = '\t';
		}
<STRING>\\\'	{
			int str_len = s - buffer;	/* because there's no 0-terminator yet */
			if (str_len >= buffersize)
			{
				buffer = realloc(buffer, buffersize + bufferinc);
				buffersize += bufferinc;
				s = buffer + str_len;
			}
			*s++ = '\'';
		}
<STRING>\\\"	{
			int str_len = s - buffer;	/* because there's no 0-terminator yet */
			if (str_len >= buffersize)
			{
				buffer = realloc(buffer, buffersize + bufferinc);
				buffersize += bufferinc;
				s = buffer + str_len;
			}
			*s++ = '\"';
		}
<STRING>{quote}	{
			int str_len = s - buffer;	/* because there's still no 0-terminator */

			if (strcmp(yytext, quote) == 0)
			{
				*s = 0;
				BEGIN 0; /* Back into normal parse mode. */
				debug(" [lex] Found string %s! Spanging it to vargs.", buffer);
				if (i < 32)
				{
					vargs[i++] = strdup(buffer);
				}
				else
				{
					debug(" [lex] Out of tokens.");
				}
				free (buffer);
				free (quote);
			}
			else
			{
				if (str_len >= buffersize)
				{
					buffer = realloc(buffer, buffersize + bufferinc);
					buffersize += bufferinc;
					s = buffer + str_len;
				}
				*s++ = *yytext;
			}
		}
<STRING><<EOF>>	{	debug(" [lex] Quotes don't match.");
			BEGIN 0;
			free (buffer);
		}
<STRING>\n	{	debug(" [lex] Quotes don't match.");
			BEGIN 0;
			free (buffer);
		}
<STRING>.	{
			int str_len = s - buffer;	/* because there's still no 0-terminator */
			if (str_len >= buffersize)
			{
				buffer = realloc(buffer, buffersize + bufferinc);
				buffersize += bufferinc;
				s = buffer + str_len;
			}
			*s++ = *yytext;
		}

{ws}		;

{word}	{	if (i < 32)
			{
				vargs[i++] = strdup(yytext);
			}
			else
			{
				debug(" [lex] Out of tokens.");
			}
		}

{number}	{	if (i < 32)
			{
				vargs[i++] = strdup(yytext);
			}
			else
			{
				debug(" [lex] Out of tokens.");
			}
		}

<<EOF>>		{
			int j;
			if (i != 0)
			{
				debug(" [lex] Calling execute_command...");
				execute_command(i, vargs);
				debug(" [lex] Freeing allocated tokens...");
				for (j = 0; j < i; j++)
				{
					debug(" [lex] Token %d : %s", j, vargs[j]);
					free(vargs[j]);
				}
			}
			else
			{
				debug(" [lex] Nothing to do!");
			}
			i = 0;
			debug(" [lex] Done.");
			yyterminate();
		}
{delimiter}	{
			int j;
			if (i != 0)
			{
				debug(" [lex] Calling execute_command...");
				execute_command(i, vargs);
				debug(" [lex] Freeing allocated tokens...");
				for (j = 0; j < i; j++)
				{
					debug(" [lex] Token %d : %s", j, vargs[j]);
					free(vargs[j]);
				}
			}
			else
			{
				debug(" [lex] Empty command.");
			}
			i = 0;
		}
.		{
			debug (" [lex] *** Unexpected punctuation! Fix this! ***\n");
		}

%%

int yywrap(void)
{
	return 1;
}
