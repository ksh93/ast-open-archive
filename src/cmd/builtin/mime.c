/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * mime capability interface
 */

static const char usage[] =
"[-?\n@(#)$Id: mime (AT&T Labs Research) 1999-08-11 $\n]"
USAGE_LICENSE
"[+NAME?mime - list mime capabilities]"
"[+DESCRIPTION?\bmime\b matches and/or lists mime capability entries. With"
"	no operands all mime entries are listed. With just a \atype\a operand"
"	the mime entry for \atype\a is listed. Otherwise the expanded"
"	\bsh\b(1) command for the \aview\a operation on the object \aname\a"
"	with mime type \atype\a and optional \aattribute\a(s) is written to"
"	the standard output. \aview\a may be \b-\b for the most common"
"	action.]"

"[s:silent|quiet?Exit 0 if match found, 1 otherwise, with no diagnostics.]"

"\n\n"
"type\n"
"view name type [ attribute ... ]\n"
"\n"

"[+SEE ALSO?\bfile\b(1), \bmailx\b(1), \bmagic\b(3), \bmime\b(3)]"
;

#include <cmd.h>
#include <mime.h>
#include <sfstr.h>

int
b_mime(int argc, char** argv, void* context)
{
	Mime_t*		mp;
	char*		s;
	Sfio_t*		sp;
	int		silent;
	Mimedisc_t	disc;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG, 0);
	silent = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 's':
			silent = 1;
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
	argc -= opt_info.index;
	argv += opt_info.index;
	if (error_info.errors || argc > 1 && argc < 3)
		error(ERROR_usage(2), "%s", optusage(NiL));
	disc.version = MIME_VERSION;
	disc.flags = 0;
	disc.errorf = errorf;
	if (!(mp = mimeopen(&disc)))
		error(ERROR_exit(1), "mime library error");
	if (mimeload(mp, NiL, 0))
	{
		mimeclose(mp);
		error(ERROR_exit(1), "mime load error");
	}
	if (argc <= 1)
		mimelist(mp, sfstdout, argv[0]);
	else if (!(sp = sfstropen()))
		error(ERROR_exit(1), "out of space");
	else
	{
		for (argc = 3; s = argv[argc]; argc++)
			sfputr(sp, s, ';');
		s = sfstruse(sp);
		if (!(s = mimeview(mp, argv[0], argv[1], argv[2], s)))
		{
			if (silent)
				exit(1);
			error(ERROR_exit(1), "no %s view for mime type %s", argv[0], argv[2]);
		}
		else
			sfputr(sfstdout, s, '\n');
		sfclose(sp);
	}
	mimeclose(mp);
	return 0;
}
