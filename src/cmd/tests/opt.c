/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <debug.h>

#if OPT_VERSION >= 20000101L
#define NEW	1
#else
#define NEW	0
#endif

#if !NEW
#define name	option
#endif

#if NEW
static int
infof(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	if (streq(s, "options"))
		return sfprintf(sp, "[Z:zoom?Do it as fast as possible.]\fmore#1\f\fmore#2\f[B:boom?Dump into \afile\a.]:[file]");
	if (streq(s, "zero"))
		return sfprintf(sp, "[+yabba?dabba][+doo?aroni]");
	if (streq(s, "more#1"))
		return sfprintf(sp, "[C:cram?Cram as much as possible.]\fmore#3\f");
	if (streq(s, "more#2"))
		return sfprintf(sp, "\fmore#4\f[D:dump?Dump as much as possible.]");
	if (streq(s, "more#3"))
		return sfprintf(sp, "[K:kill?kill all processes.]");
	if (streq(s, "more#4"))
		return sfprintf(sp, "[F:fudge?Fudge the statistics to satisfy everyone.]");
	return sfprintf(sp, "<* %s info ok *>", s);
}

static char*
translate(const char* locale, const char* id, const char* catalog, const char* msg)
{
	register int	c;
	register int	i;
	register char*	s;
	register char*	t;
	register char*	e;

	static char	buf[8 * 1024];

	sfprintf(sfstdout, "id=%s catalog=%s text=\"%s\"\n", id, catalog, msg);
	i = !catalog;
	s = (char*)msg;
	t = buf;
	e = buf + sizeof(buf) - 1;
	while ((c = *s++) && t < e)
	{
		switch (c)
		{
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M':
			c += 13;
			if (i)
				c = tolower(c);
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm':
			c += 13;
			if (i)
				c = toupper(c);
			break;
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S':
		case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
			c -= 13;
			if (i)
				c = tolower(c);
			break;
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's':
		case 't': case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
			c -= 13;
			if (i)
				c = toupper(c);
			break;
		case '\\':
			if (t < e)
				*t++ = c;
			if (!(c = *s++))
				s--;
			break;
		case '%':
			do
			{
				if (t >= e)
					break;
				*t++ = c;
				if (!(c = *s++))
				{
					s--;
					break;
				}
			} while (!isalpha(c) || (!islower(c) || c == 'h' || c == 'l') && (t < e) && isalpha(*s));
			break;
		case '\b':
			do
			{
				if (t >= e)
					break;
				*t++ = c;
				if (!(c = *s++))
				{
					s--;
					break;
				}
			} while (c != '\b');
			break;
		}
		*t++ = c;
	}
	*t = 0;
	return streq(buf, msg) ? (char*)msg : buf;
}
#endif

main(int argc, char** argv)
{
	int		n;
	int		ext;
	int		str;
	char*		command;
	char*		usage;
	char**		extra;
#if NEW
	Optdisc_t	disc;
#endif

	error_info.id = "opt";
	setlocale(LC_ALL, "");
	error(-1, "test");
	extra = 0;
	ext = 0;
	str = 0;
	while (command = *(argv + 1))
	{
		if (streq(command, "-"))
		{
			argv++;
			str = NEW;
		}
		else if (streq(command, "-+"))
		{
			argv++;
#if NEW
			ast.locale.set |= (1<<AST_LC_MESSAGES);
			error_info.translate = translate;
#endif
		}
		else if (streq(command, "+") && *(argv + 2))
		{
			ext += 2;
			argv += 2;
			if (!extra)
				extra = argv;
		}
		else
			break;
	}
	if (!(command = *++argv) || !(usage = *++argv))
		error(ERROR_USAGE|4, "[ - | + usage ... ] command-name usage-string [ arg ... ]");
	argv += str;
	error_info.id = command;
#if NEW
	memset(&disc, 0, sizeof(disc));
	disc.version = OPT_VERSION;
	disc.infof = infof;
	opt_info.disc = &disc;
#else
	memset(&opt_info, 0, sizeof(opt_info));
#endif
	for (;;)
	{
		if (!str)
		{
			if (!(n = optget(argv, usage)))
				break;
		}
		else if (!(n = optstr(*argv, usage)))
		{
			if (!*++argv)
				break;
			continue;
		}
		if (n == '?')
		{
			sfprintf(sfstdout, "return=%c option=%s name=%s num=%d\n", n, opt_info.option, opt_info.name, opt_info.num);
			error(ERROR_USAGE|4, "%s", opt_info.arg);
		}
		else if (n == ':')
		{
			sfprintf(sfstdout, "return=%c option=%s name=%s num=%d", n, opt_info.option, opt_info.name, opt_info.num);
			if (!opt_info.option[0])
				sfprintf(sfstdout, " str=%s", argv[opt_info.index - 1]);
			sfputc(sfstdout, '\n');
			error(2, "%s", opt_info.arg);
		}
		else if (n > 0)
			sfprintf(sfstdout, "return=%c option=%s name=%s arg=%s num=%d\n", n, opt_info.option, opt_info.name, opt_info.arg, opt_info.num);
		else
			sfprintf(sfstdout, "return=%d option=%s name=%s arg=%s num=%d\n", n, opt_info.option, opt_info.name, opt_info.arg, opt_info.num);
		if (extra)
		{
			for (n = 0; n < ext; n += 2)
				optget(NiL, extra[n]);
			extra = 0;
		}
	}
	if (!str && *(argv += opt_info.index))
		while (command = *argv++)
			sfprintf(sfstdout, "argument=%d value=\"%s\"\n", ++str, command);
	return error_info.errors != 0;
}
