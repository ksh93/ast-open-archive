/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2001 AT&T Corp.                *
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

#include <ast.h>
#include <error.h>
#include <sfdcgzip.h>

static const char usage[] =
"[-?\n@(#)$Id: funzip (AT&T Labs Research) 1998-08-11 $\n]"
USAGE_LICENSE
"[+NAME?funzip - fast gunzip]"
"[+DESCRIPTION?\bfunzip\b decompresses \bgzip\b(1) compressed files."
"	By default \bgzip\b crc32 cyclic redundancy checking is disabled."
"	This may speed up decompression by 25% or more over \bgunzip\b(1)."
"	Most data corruption errors are still caught even with crc disabled.]"

"[x:crc?Enable \agzip\a crc32 cyclic redundancy checking for decompress."
"	On some systems this can double the execution wall time."
"	Most data corruption errors are still caught even with \bnocrc\b.]"

"\n"
"\n[ file ]\n"
"\n"

"[+SEE ALSO?\bgzip\b(1), \bgunzip\b(1), \blibz\b(3)]"
;

main(int argc, char** argv)
{
	Sfio_t*	sp;
	char*	file;
	int	r;

	int	flags = SFGZ_NOCRC;

	error_info.id = "funzip";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'x':
			flags &= ~SFGZ_NOCRC;
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
	if (error_info.errors || *argv && *(argv + 1))
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(file = *argv))
	{
		file = "/dev/stdin";
		sp = sfstdin;
	}
	else if (!(sp = sfopen(NiL, file, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read", file);
	if ((r = sfdcgzip(sp, flags)) < 0)
		error(3, "sfdcgzip discipline push error");
	else if (!r)
		error(3, "input not a gzip file");
	if (sfmove(sp, sfstdout, SF_UNBOUND, -1) < 0 || sfsync(sfstdout))
		error(ERROR_SYSTEM|3, "sfdcgzip io error");
	return 0;
}
