/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                       Jeff Korn <@google.com>                        *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "tksh.h"

static char **av;
static int ac;

static void tksh_userinit(int subshell)
{
	char *end = av[0] + strlen(av[0]);
	int len = strlen(av[0]);
	char *args[2];
	Namval_t *np;

	if(np = nv_open("source",sh.alias_tree,NV_NOADD))
	{
		nv_unset(np);
		nv_close(np);
	}
	if (subshell < 0)
	{
		sh_trap("tkloop", 0);
		return;
	}
	else if (subshell > 0)
	{
		TkshSubShell();
		return;
	}

#ifndef DO_TK
	/* sfsetbuf(sfstdout, NULL, 0); */
	args[0] = av[0]; args[1] = NULL;
	if ((len >= 4) && (strcmp(end-4, "tksh") == 0))
		/* b_tkinit(0, (char **) 0, (void *) 0); */
		b_tkinit(1, args, (void *) 0);
	else if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                /* b_tclinit(0, (char **) 0, (void *) 0); */
                b_tclinit(1, args, (void *) 0);
	else
	{
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
		sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	}
#else
	sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                b_tclinit(0, (char **) 0, (void *) 0);
	else
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
#endif
}

int main(int argc, char *argv[])
{
	return (sh_main(ac=argc, av=argv, tksh_userinit));
}
