/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
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
