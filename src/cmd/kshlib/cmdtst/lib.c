/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <ast.h>
#include <shcmd.h>

extern int	b_grep(int, char**, Shbltin_t*);
extern int	b_egrep(int, char**, Shbltin_t*);
extern int	b_fgrep(int, char**, Shbltin_t*);
extern int	b_xgrep(int, char**, Shbltin_t*);
extern int	b_xargs(int, char**, Shbltin_t*);

void
lib_init(int flag, void* context)
{
	error(-1, "AHA cmdtst lib_init(%d,%p)", flag, context);
	if (!flag)
	{
		sh_builtin(context, "grep", b_grep, 0); 
		sh_builtin(context, "egrep", b_egrep, 0); 
		sh_builtin(context, "fgrep", b_fgrep, 0); 
		sh_builtin(context, "xgrep", b_xgrep, 0); 
		sh_builtin(context, "xargs", b_xargs, 0); 
	}
}

SHLIB(test)
