/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1995-2001 AT&T Corp.                *
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
 * glob test harness
 */

static const char usage[] =
"[-?\n@(#)$Id: testglob (AT&T Labs Research) 1999-11-04 $\n]"
USAGE_LICENSE
"[+NAME?testglob - test harness for \bglob\b(3)]"
"[+DESCRIPTION?\btestglob\b is a test harness for the \bglob\b(3) library"
"	function. It provides access to the \bast\b extension to \bglob\b(3)."
"	More than one \apattern\a argument exercises \bGLOB_APPEND\b. Be sure"
"	to quote the \apattern\a arguments, otherwise you will be testing"
"	\bsh\b(1) and not \bglob\b(3).]"

"[c:completion?Perform shell command completion.]"
"[d:drop?Drop directory entries matching pattern.]:[pattern]"
"[e!:escape?\b\\\b quotes the next character.]"
"[i:ignorecase?Ignore case when matching patterns.]"
"[l:list?Return a linked list of matched paths rather than a 0 terminated"
"	array. The list is not sorted.]"
"[m:mark?Append \b/\b to matched directories.]"
"[s!:sort?Sort the matched paths. Ignored for \b--list\b.]"

"\n"
"\npattern ...\n"
"\n"
"[+SEE ALSO?\btestmatch\b(1), \btestre\b(1), \bregex\b(3), \bstrmatch\b(3)]"
;

#include <ast.h>
#include <ctype.h>
#include <glob.h>
#include <error.h>

static char*
globerror(int code)
{
	static char	msg[32];

	switch (code)
	{
	case GLOB_ABORTED:	return "GLOB_ABORTED";
	case GLOB_NOMATCH:	return "GLOB_NOMATCH";
	case GLOB_NOSPACE:	return "GLOB_NOSPACE";
	case GLOB_INTR:		return "GLOB_INTR";
	case GLOB_APPERR:	return "GLOB_APPERR";
	case GLOB_NOSYS:	return "GLOB_NOSYS";
	}
	sfsprintf(msg, sizeof(msg), "unknown error %d", code);
	return msg;
}

int
main(int argc, char** argv)
{
	register char*		pattern;
	register char**		vp;
	register globlist_t*	gl;
	register int		i;
	glob_t			gs;

	int			flags = GLOB_NOCHECK|GLOB_STACK;

	NoP(argc);
	error_info.id = "testglob";
	memset(&gs, 0, sizeof(gs));
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			if (opt_info.num)
				flags |= GLOB_COMPLETE;
			continue;
		case 'd':
			gs.gl_fignore = opt_info.arg;
			continue;
		case 'e':
			if (!opt_info.num)
				flags |= GLOB_NOESCAPE;
			continue;
		case 'i':
			if (opt_info.num)
				flags |= GLOB_ICASE;
			continue;
		case 'l':
			if (opt_info.num)
				flags |= GLOB_LIST;
			continue;
		case 'm':
			if (opt_info.num)
				flags |= GLOB_MARK;
			continue;
		case 's':
			if (!opt_info.num)
				flags |= GLOB_NOSORT;
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
	if (error_info.errors || !*argv)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	while (pattern = *argv++)
	{
		if (i = glob(pattern, flags, 0, &gs))
			error(2, "%s: glob error %s", pattern, globerror(i));
		else
			flags |= GLOB_APPEND;
	}
	sfprintf(sfstdout, "%d match%s\n", gs.gl_pathc, gs.gl_pathc == 1 ? "" : "es");
	if (flags & GLOB_LIST)
		for (gl = gs.gl_list; gl; gl = gl->gl_next)
			sfprintf(sfstdout, "%s\n", gl->gl_path);
	else
		for (vp = gs.gl_pathv; *vp; vp++)
			sfprintf(sfstdout, "%s\n", *vp);
	globfree(&gs);
	return error_info.errors != 0;
}
