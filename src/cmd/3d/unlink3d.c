/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2004 AT&T Corp.                *
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
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped

#include "3d.h"

int
unlink3d(register const char* path)
{
	register char*	sp;
	register int	r;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_remove, 0, path))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_PATHONLY|P_SAFE, NiL)))
		return -1;
	if (state.path.level)
		return 0;
	if (!(r = LSTAT(sp, &state.path.st)))
	{
		if (S_ISLNK(state.path.st.st_mode) && !checklink(sp, &state.path.st, P_PATHONLY|P_LSTAT) && state.path.linksize > 0)
		{
			/*
			 * remove instance if not default
			 */

			r = strlen(sp) - (sizeof(state.vdefault) - 1);
			if (r > 3 && streq(sp + r, state.vdefault))
				return 0;
		}
		r = UNLINK(sp);
	}
	if (!r)
	{
#if FS
		if (mp)
			fscall(mp, MSG_remove, 0, path);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_remove))
				fscall(mp, MSG_remove, 0, path);
#endif
	}
	else if (errno == ENOENT && pathreal(path, 0, NiL))
		r = 0;
	return r;
}

#undef	remove

int
remove(const char* path)
{
	return unlink(path);
}
