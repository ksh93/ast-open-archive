/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <ast.h>

#undef	_BLD_ast	/* enable ast imports since we're user static */

#include <error.h>
#include <option.h>
#include <getopt.h>
#include <ctype.h>

static const char*		lastoptstring;
static const struct option*	lastlongopts;
static char*			usage;
static Sfio_t*			up;

static int			lastoptind;

static int
golly(int argc, char* const* argv, const char* optstring, const struct option* longopts, int* longindex, int flags)
{
	register char*			s;
	register const struct option*	o;
	register int			c;
	char*				t;

	if (!up || optstring != lastoptstring || longopts != lastlongopts)
	{
		if (!up && !(up = sfstropen()))
			return -1;
		sfprintf(up, "[-1p%d]", flags);
		t = strdup(optstring);
		for (o = longopts; o->name; o++)
		{
			if (o->flag || o->val <= 0 || o->val > UCHAR_MAX || !isalnum(o->val))
				sfprintf(up, "\n[%d:%s]", UCHAR_MAX + 1 + (o - longopts), o->name);
			else
			{
				sfprintf(up, "\n[%c:%s]", o->val, o->name);
				if (s = strchr(t, o->val))
				{
					*s++ = ' ';
					if (*s == ':')
					{
						*s++ = ' ';
						if (*s == ':')
							*s = ' ';
					}
				}
			}
			if (o->has_arg)
			{
				sfputc(up, ':');
				if (o->has_arg == optional_argument)
					sfputc(up, '?');
				sfprintf(up, "[string]");
			}
		}
		s = t;
		while (c = *s++)
			if (c != ' ')
			{
				sfprintf(up, "\n[%c]", c);
				if (*s == ':')
				{
					sfputc(up, *s);
					if (*++s == ':')
					{
						sfputc(up, '?');
						s++;
					}
					sfputc(up, '[');
					sfputc(up, ']');
				}
			}
		sfputc(up, '\n');
		usage = sfstruse(up);
		lastoptstring = optstring;
		lastlongopts = longopts;
	}
	opt_info.index = (optind > 1 || optind == lastoptind) ? optind : 0;
	if (opt_info.index >= argc || !(c = optget((char**)argv, usage)))
	{
		sfstrclose(up);
		up = 0;
		c = -1;
	}
	else
	{
		if (c == ':' || c == '?')
		{
			if (opterr && (!optstring || *optstring != ':'))
			{
				if (!error_info.id)
					error_info.id = argv[0];
				errormsg(NiL, c == '?' ? (ERROR_USAGE|4) : 2, "%s", opt_info.arg);
			}
			optopt = opt_info.option[1];
			c = '?';
		}
		optarg = opt_info.arg;
		if (c < 0)
		{
			o = longopts - c - UCHAR_MAX - 1;
			if (o->flag)
			{
				*o->flag = o->val;
				c = 0;
			}
			else
				c = o->val;
		}
	}
	lastoptind = optind = opt_info.index;
	return c;
}

extern int
getopt_long(int argc, char* const* argv, const char* optstring, const struct option* longopts, int* longindex)
{
	return golly(argc, argv, optstring, longopts, longindex, 2);
}

extern int
getopt_long_only(int argc, char* const* argv, const char* optstring, const struct option* longopts, int* longindex)
{
	return golly(argc, argv, optstring, longopts, longindex, 1);
}
