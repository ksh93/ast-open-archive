/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 2003-2004 AT&T Corp.                *
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

/*
 * return the codex method for name
 */

#include <codex.h>
#include <dlldefs.h>

Codexmeth_t*
codexmeth(const char* name)
{
	register Codexmeth_t*	meth;
	register const char*	s;
	register const char*	t;
	Codexmeth_t*		last;
	void*			dll;
	int			plugin;
	Codexlib_f		lib;
	char			tmp[CODEX_NAME];

	if (!name)
		return CODEXERROR->meth;
	plugin = 0;
	meth = codexstate.first;
	while (meth)
	{
		if (!codexcmp(name, meth->name))
			return meth;
		last = meth;
		if (!(meth = meth->next) && !plugin)
		{
			plugin = 1;
			for (s = name, t = 0; *s && *s != '+'; s++)
				if (*s == '-')
				{
					if (t)
						break;
					t = s;
				}
			do
			{
				sfsprintf(tmp, sizeof(tmp), "%-.*s", s - name, name);
				if (dll = dllplug("codex", tmp, NiL, RTLD_LAZY, NiL, 0))
				{
					if ((lib = (Codexlib_f)dlllook(dll, "codex_lib")) && (meth = (*lib)(name)))
						last->next = meth;
					break;
				}
			} while (s != t && (s = t));
		}
	}
	return meth;
}
