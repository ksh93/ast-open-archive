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
 *	<del>program<del>pattern<del>read-command<del>write-command<del>
 * % in unzip or zip expands to the command name
 * %% for literal %
 * program is matched against error_info.id
 * pattern is matched against the file path
 */

#define sfopen	_sfopen

#include <ast.h>
#include <error.h>

#undef	sfopen

struct Match_s; typedef struct Match_s Match_t;

#define program		command[0]
#define pattern		command[1]

struct Match_s
{
	Match_t*	next;
	const char*	command[4];
};

static struct State_s
{
	Match_t*	match;
	Sfio_t*		cmd;
	int		init;
} state;

static Match_t*
initialize(void)
{
	register const char*	s;
	register char*		t;
	register int		n;
	register int		d;
	Match_t*		x;
	Match_t*		v;
	const char*		arg[5];

	x = 0;
	if (s = getenv("SFOPEN_INTERCEPT"))
	{
		while (d = *s++)
		{
			for (n = 0; n < elementsof(arg) - 1; n++)
			{
				for (arg[n] = s; *s && *s != d; s++);
				if (*s)
					s++;
			}
			arg[n] = s;
			if (!(v = newof(0, Match_t, 1, arg[elementsof(arg)-1] - arg[0])))
				return x;
			t = (char*)(v + 1);
			for (n = 0; n < elementsof(v->command); n++)
			{
				v->command[n] = (const char*)t;
				if ((d = arg[n+1] - arg[n]) > 1)
					memcpy(t, arg[n], d - 1);
				t += d;
				if (!*v->command[n])
					v->command[n] = 0;
			}
			v->next = x;
			x = v;
		}
	}
	return x;
}

Sfio_t*
sfopen(Sfio_t* f, const char* path, const char* mode)
{
	register Match_t*	x;
	register const char*	s;
	register int		n;

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
			do
			{
				if ((!x->program || strmatch(error_info.id, x->program)) && (!x->pattern || strmatch(path, x->pattern)))
				{
					n = 0;
					s = mode;
					for (;;)
					{
						switch (*s++)
						{
						case 0:
							break;
						case 'b':
							n = -1;
							break;
						case 'w':
						case '+':
							n = 1;
							continue;
						default:
							continue;
						}
						break;
					}
					if (n >= 0 && (s = x->command[n+2]))
					{
						while (n = *s++)
						{
							if (n == '%')
							{
								if (*s != '%')
								{
									sfputr(state.cmd, path, -1);
									continue;
								}
								s++;
							}
							sfputc(state.cmd, n);
						}
						return sfpopen(f, sfstruse(state.cmd), mode);
					}
					break;
				}
			} while (x = x->next);
		}
	}
	return _sfopen(f, path, mode);
}
