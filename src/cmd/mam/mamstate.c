/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
 * AT&T Bell Laboratories
 * make abstract machine file state support
 *
 * mamstate reference [ file ... | <files ]
 *
 * stdout is list of <file,delta> pairs where delta
 * is diff between reference and file times
 * non-existent files are not listed
 */

static const char id[] = "\n@(#)$Id: mamstate (AT&T Bell Laboratories) 1989-06-26 $\0\n";

#include <ast.h>
#include <ls.h>
#include <error.h>

main(int argc, register char** argv)
{
	register char*	s;
	long		ref;
	struct stat	st;

	NoP(argc);
	error_info.id = "mamstate";
	if (!(s = *++argv) || stat(s, &st))
		error(ERROR_USAGE|4, optusage("reference [ file ... | <files ]"));
	ref = (long)st.st_mtime;
	if (s = *++argv) do
	{
		if (!stat(s, &st))
			sfprintf(sfstdout, "%s %ld\n", s, (long)st.st_mtime - ref);
	} while (s = *++argv);
	else while (s = sfgetr(sfstdin, '\n', 1))
		if (!stat(s, &st))
			sfprintf(sfstdout, "%s %ld\n", s, (long)st.st_mtime - ref);
	exit(0);
}
