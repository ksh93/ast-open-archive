/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * what
 * my what is faster than yours.
 * your who is what?
 * no, they're different.
 */

static const char usage[] =
"[-?\n@(#)$Id: what (AT&T Labs Research) 2000-12-01 $\n]"
USAGE_LICENSE
"[+NAME?what - display binary identification strings]"
"[+DESCRIPTION?\bwhat\b searches the given files for all occurrences of"
"	the pattern \b@(#)\b or \b$Id:\b and writes a line to the standard"
"	output containing the text that follows until the first occurrence"
"	of one of the following: \b\" > \\ $ newline NUL\b.]"

"[s:first|single?Find only the first occurrence of the pattern in each file.]"

"\n"
"\nfile ...\n"
"\n"

"[+EXIT STATUS]{"
"	[+0?Some matches were found.]"
"	[+1?Otherwise.]"
"	[+2?Option error.]"
"}"
"[+SEE ALSO?\bident\b(1), \bgrep\b(1), \bstrings\b(1)]"
;

#include <cmd.h>
#include <ctype.h>

#define HIT		SSIZE_MAX

static struct
{
	int		hit;
	int		single;
	size_t		skip[UCHAR_MAX+1];
	unsigned char	prev[3];
} state;

static void
what(const char* file, Sfio_t* ip, Sfio_t* op)
{
	register unsigned char*	buf;
	register size_t*	skip;
	register unsigned char*	s;
	register unsigned char*	e;
	register size_t		index;
	register size_t		mid;
	unsigned char*		b;
	char*			t;

	sfprintf(op, "%s:\n", file);
	if (buf = (unsigned char*)sfreserve(ip, SF_UNBOUND, 0))
	{
		skip = state.skip;
		if ((mid = sfvalue(ip)) <= (index = 3))
			goto next;
		e = buf + mid;
		for (;;)
		{
			while ((index += skip[buf[index]]) < mid);
			if (index < HIT)
			{
			next:
				s = state.prev;
				s[0] = s[1] = s[2] = 0;
				switch (mid)
				{
				default:
					s[0] = buf[mid - 3];
					/*FALLTHROUGH*/
				case 2:
					s[1] = buf[mid - 2];
					/*FALLTHROUGH*/
				case 1:
					s[2] = buf[mid - 1];
					/*FALLTHROUGH*/
				case 0:
					break;
				}
				if (!(buf = (unsigned char*)sfreserve(ip, SF_UNBOUND, 0)))
					goto done;
				if ((mid = sfvalue(ip)) <= (index = 3))
					goto next;
				e = buf + mid;
				switch (skip[buf[0]])
				{
				case HIT:
					if (buf[0] == ')' && s[0] == '@' && s[1] == '(' && s[2] == '#' || buf[0] == ':' && s[0] == '$' && s[1] == 'I' && s[2] == 'd')
					{
						index = 0;
						s = buf + 1;
						goto hit;
					}
					break;
				case 1:
					if (buf[0] == ')' && s[1] == '@' && s[2] == '(' && buf[1] == '#' || buf[0] == ':' && s[1] == '$' && s[2] == 'I' && s[3] == 'd')
					{
						index = 1;
						s = buf + 2;
						goto hit;
					}
					break;
				case 2:
					if (buf[0] == ')' && s[2] == '@' && buf[1] == '(' && buf[2] == '#' || buf[0] == ':' && s[2] == '$' && s[3] == 'I' && s[3] == 'd')
					{
						index = 2;
						s = buf + 3;
						goto hit;
					}
					break;
				}
			}
			else
			{
				index -= HIT;
				s = buf + index;
				if (s[0] == ')' && s[-1] == '#' && s[-2] == '(' && s[-3] == '@' || s[0] == ':' && s[-1] == 'd' && s[-2] == 'I' && s[-3] == '$')
				{
					s++;
				hit:
					for (;;)
					{
						while (*s == ' ' || *s == '\t')
							s++;
						if (s[0] == '@' && s[1] == '(' && s[2] == '#' && s[3] == ')')
							s += 4;
						else if (s[0] == '$' && s[1] == 'I' && s[2] == 'd' && s[3] == ':')
							s += 4;
						else
							break;
					}
					b = s;
					t = "\t";
					for (;;)
					{
						if (s >= e)
						{
							sfprintf(op, "%s%-.*s", t, s - b, b);
							t = "";
							if (!(buf = (unsigned char*)sfreserve(ip, SF_UNBOUND, 0)))
								goto list;
							e = (s = b = buf) + (mid = sfvalue(ip));
						}
						else
						{
							switch (*s)
							{
							case 0:
							case '>':
							case '"':
							case '\\':
							case '\n':
							list:
								if ((s - b) > 2 && *(s - 1) == '$' && *(s - 2) == ' ')
									s -= 2;
								if (s > b)
								{
									sfprintf(op, "%s%-.*s\n", t, s - b, b);
									state.hit = 1;
									if (state.single)
										return;
									if (!buf)
										goto done;
								}
								break;
							default:
								s++;
								continue;
							}
							break;
						}
					}
					index = s - buf;
				}
				if ((index += 4) >= mid)
					goto next;
			}
		}
	}
 done:
	if (sfvalue(ip))
		error(ERROR_system(0), "%s: read error", file);
}

int
b_what(int argc, char** argv, void* context)
{
	register int		n;
	register char*		s;
	register Sfio_t*	sp;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG);
	state.hit = state.single = 0;
	for (n = 0; n <= UCHAR_MAX; n++)
		state.skip[n] = 4;
	state.skip['@'] = state.skip['$'] = 3;
	state.skip['('] = state.skip['I'] = 2;
	state.skip['#'] = state.skip['d'] = 1;
	state.skip[')'] = state.skip[':'] = HIT;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 's':
			state.single = 1;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (s = *argv)
		argv++;
	do
	{
		if (!s || streq(s, "-"))
		{
			s = "/dev/stdin";
			sp = sfstdin;
		}
		else if (!(sp = sfopen(NiL, s, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", s);
			continue;
		}
		what(s, sp, sfstdout);
		if (sp != sfstdin)
			sfclose(sp);
	} while (s = *argv++);
	return error_info.errors != 0 && state.hit;
}
