/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1994-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * ncsl [file ...]
 *
 * ncsl -- list the number of non-comment source lines for each C file
 */

static const char usage[] =
"[-?\n@(#)$Id: ncsl (AT&T Labs Research) 1996-10-11 $\n]"
USAGE_LICENSE
"[+NAME?ncsl - list the number of non-comment source lines for each C file]"
"[+DESCRIPTION?\bncsl\b lists the number of non-comment source lines for"
"	each C \afile\a argument. If no \afile\a arguments are specified then"
"	the standard input is read. /*...*/ and // style comments are"
"	recognized.]"

"[s:summary?If more than one \afile\a is specified then list a summary count"
"	for all files.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bwc\b(1)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>

#define JUNK(c)		(iscntrl(c)||!isprint(c))

/*
 * return number of ncsl in sp
 */

static int
ncsl(register Sfio_t* sp)
{
	register int		c;
	register int		data;
	register unsigned long	line;
	int			quote;

	line = 0;
	data = 0;
 again:
	for (;;) switch (c = sfgetc(sp))
	{
	case EOF:
		return(line);
	case ' ':
	case '\f':
	case '\t':
		break;
	case '\n':
	newline:
		if (data)
		{
			if (data > 0) line++;
			data = 0;
		}
		break;
	case '/':
		if (data < 0) goto again;
		switch (c = sfgetc(sp))
		{
		case '/':
			for (;;) switch (c = sfgetc(sp))
			{
			case EOF:
				goto again;
			case '\n':
				goto newline;
			default:
				if (JUNK(c))
				{
					data = -1;
					goto again;
				}
				break;
			}
			break;
		case '*':
			for (;;) switch (c = sfgetc(sp))
			{
			case EOF:
			case '*':
				for (;;)
				{
					switch (c = sfgetc(sp))
					{
					case EOF:
					case '/':
						goto again;
					case '*':
						continue;
					default:
						if (JUNK(c))
						{
							data = -1;
							goto again;
						}
						break;
					}
					break;
				}
				break;
			default:
				if (JUNK(c))
				{
					data = -1;
					goto again;
				}
				break;
			}
			break;
		case EOF:
			break;
		default:
			if (!data) data = 1;
			sfungetc(sp, c);
			break;
		}
		break;
	case '"':
	case '\'':
		if (data < 0) goto again;
		data = 1;
		quote = c;
		for (;;) switch (c = sfgetc(sp))
		{
		case EOF:
			goto again;
		case '"':
		case '\'':
			if (c == quote) goto again;
			break;
		case '\n':
			goto newline;
		case '\\':
			switch (c = sfgetc(sp))
			{
			case EOF:
				goto again;
			case '\n':
				line++;
				continue;
			}
			/*FALLTHROUGH*/
		default:
			if (JUNK(c))
			{
				data = -1;
				goto again;
			}
			break;
		}
		break;
	default:
		if (JUNK(c)) data = -1;
		else if (!data) data = 1;
		break;
	}
	/*NOTREACHED*/
}

main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	sp;

	unsigned long		n;
	int			summary = 0;
	unsigned long		total = 0;
	int			trailer;

	NoP(argc);
	error_info.id = "ncsl";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 's':
			summary = 1;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (*argv)
	{
		trailer = *(argv + 1) != 0;
		while (s = *argv++)
		{
			if (!(sp = sfopen(NiL, s, "r")))
				error(2, "%s: cannot open for reading", s);
			else
			{
				n = ncsl(sp);
				sfclose(sp);
				if (!summary)
					sfprintf(sfstdout, "%s: %lu\n", s, n);
				total += n;
			}
		}
		if (summary || trailer)
			sfprintf(sfstdout, "%lu\n", total);
	}
	else
		sfprintf(sfstdout, "%d\n", ncsl(sfstdin));
	return error_info.errors != 0;
}
