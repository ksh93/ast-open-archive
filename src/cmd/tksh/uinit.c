/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2002 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

#include "tksh.h"

static char **av;
static int ac;

static void tksh_userinit(int subshell)
{
	char *end = av[0] + strlen(av[0]);
	int len = strlen(av[0]);
	char *args[2];

	if (subshell)
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
