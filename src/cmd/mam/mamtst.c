/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * convert a make abstract machine stream on stdin
 * to an oldmake makefile on stdout
 */

static const char id[] = "\n@(#)$Id: mamtst (gsf@research.att.com) 1992-08-11 $\0\n";

#include <mam.h>
#include <ctype.h>
#include <error.h>

static void
dumprule(register struct rule* r)
{
	sfprintf(sfstdout, "\t%s\n", r->name);
}

static void
dumpproc(register struct proc* pp)
{
	register struct list*	p;
	register struct proc*	cp;

	if (pp->parent) sfputc(sfstdout, '\n');
	sfprintf(sfstdout, "process %s pid %d ppid %d start %lu finish %lu status %d\n", pp->pwd, pp->pid, pp->parent ? pp->parent->pid : 0, pp->start, pp->finish, pp->status);
	for (p = pp->root->prereqs; p; p = p->next)
		dumprule(p->rule);
	for (cp = pp->child; cp; cp = cp->sibling)
		dumpproc(cp);
}

main(int argc, char** argv)
{
	register int	c;
	struct mam*	mp;

	NoP(argc);
	error_info.id = "mamtst";
	while (c = optget(argv, "d#[debug]")) switch (c)
	{
	case 'd':
		error_info.trace = -opt_info.num;
		break;
	case '?':
		error(ERROR_USAGE|4, opt_info.arg);
		break;
	case ':':
		error(2, opt_info.arg);
		break;
	}
	if (error_info.errors) error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(mp = mamalloc()))
		error(3, "cannot initialize");
	if (mamscan(mp, NiL) < 0)
		error(3, "invalid input");
	dumpproc(mp->main);
	exit(error_info.errors != 0);
}
