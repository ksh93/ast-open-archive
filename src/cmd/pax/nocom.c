/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2001 AT&T Corp.                *
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
 * AT&T Labs Research
 *
 * nocom [file ...]
 *
 * nocom -- strip comments from C source files
 */

static const char usage[] =
"[-?\n@(#)$Id: nocom (AT&T Labs Research) 1994-01-11 $\n]"
USAGE_LICENSE
"[+NAME?nocom - strip comments from C source files]"
"[+DESCRIPTION?\bnocom\b strips \b// ...\b and \b/* ... */\b comments from"
"	each C source \afile\a and writes the result on the standard output."
"	Comments that span multiple lines are replaced by \bnewline\b"
"	characters to retain the original source line numbering. If \afile\a"
"	is omitted then the standard input is read.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bcc\b(1), \bwc\b(1)]"
;

#include <ast.h>
#include <error.h>

#include "nocomment.c"

int
main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	sp;

	NoP(argc);
	error_info.id = "nocom";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!*argv)
	{
		if (nocomment(sfstdin, sfstdout) < 0)
			error(ERROR_SYSTEM|2, "write error");
	}
	else while (s = *argv++)
	{
		if (!(sp = sfopen(NiL, s, "r")))
			error(ERROR_SYSTEM|2, "%s: cannot read", s);
		else
		{
			if (nocomment(sp, sfstdout) < 0)
				error(ERROR_SYSTEM|2, "%s: write error", s);
			sfclose(sp);
		}
	}
	return error_info.errors != 0;
}
