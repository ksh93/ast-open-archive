/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                   Phong Vo <kpv@research.att.com>                    *
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfopen file suffix match intercept
 * export SFOPEN_INTERCEPT with one or more suffix map entries
 *	<del>program<del>read-pattern<del>read-command<del>write-pattern<del>write-command<del>[<newline>>]
 *	<del><del>option<del>[<newline>]
 * program is matched against error_info.id
 * *-pattern is matched against the file path
 * \0 in *-command expands to the file path
 * \<n> in *-command expands to the <n>th subexpression in the file path pattern match
 */

#define sfopen	_sfopen

#include <ast.h>
#include <error.h>

#undef	sfopen

struct Match_s; typedef struct Match_s Match_t;

struct Match_s
{
	Match_t*	next;
	const char*	arg[5];
};

static struct State_s
{
	Match_t*	match;
	Sfio_t*		cmd;
	int		init;
	int		verbose;
} state;

static Match_t*
initialize(void)
{
	register char*	s;
	register char*	t;
	register int	n;
	register int	d;
	Match_t*	x;
	Match_t*	v;

	x = 0;
	if ((s = getenv("SFOPEN_INTERCEPT")) && (s = strdup(s)))
	{
		while (d = *s++)
		{
			if (d == '\r' || d == '\n')
				continue;
			if (*s == d)
			{
				for (t = (char*)++s; *s && *s != d; s++);
				if (*t == 'n' && *(t + 1) == 'o')
				{
					t += 2;
					n = 0;
				}
				else
					n = 1;
				if (*t == 'v')
					state.verbose = n;
				if (*s)
					s++;
			}
			else if (!(v = newof(0, Match_t, 1, 0)))
				break;
			else
			{
				for (n = 0; n < elementsof(v->arg); n++)
				{
					for (v->arg[n] = s; *s && *s != d; s++);
					if (*s)
						*s++ = 0;
					if (!*v->arg[n])
						v->arg[n] = 0;
				}
				v->next = x;
				x = v;
			}
		}
	}
	return x;
}

Sfio_t*
sfopen(Sfio_t* f, const char* path, const char* mode)
{
	register Match_t*	x;
	register const char*	s;
	register int		c;
	register int		n;
	register int		r;
	register int		m;

	int			sub[20];

	if (path && error_info.id)
	{
		if (!state.init)
		{
			state.init = 1;
			if (state.cmd = sfstropen())
				state.match = initialize();
		}
		if (x = state.match)
		{
			n = 1;
			s = mode;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case 'b':
					n = 0;
					break;
				case 'w':
				case '+':
					n = 3;
					continue;
				default:
					continue;
				}
				break;
			}
			if (n > 1 || n == 1 && !access(path, R_OK))
				do
				{
					if ((!x->arg[0] || strmatch(error_info.id, x->arg[0])) &&
					    (!x->arg[n] && !(m = 0) || (m = strgrpmatch(path, x->arg[n], sub, elementsof(sub) / 2, STR_MAXIMAL|STR_LEFT|STR_RIGHT))))
					{
						if (s = x->arg[n+1])
						{
							m *= 2;
							r = 1;
							while (c = *s++)
							{
								if (c == '\\' && *s && (c = *s++) >= '0' && c <= '9')
								{
									c = 2 * (c - '0');
									if (c < m && (r = sub[c+1] - sub[c]))
										sfwrite(state.cmd, path + sub[c], r);
									r = 0;
								}
								else
									sfputc(state.cmd, c);
							}
							if (r)
								sfprintf(state.cmd, " %c %s", n == 1 ? '<' : '>', path);
							s = sfstruse(state.cmd);
							if (state.verbose)
								error(0, "%s %s", error_info.id, s);
							if (f = sfpopen(f, s, mode))
								sfset(f, SF_SHARE, 0);
							return f;
						}
						break;
					}
				} while (x = x->next);
		}
	}
	return _sfopen(f, path, mode);
}
