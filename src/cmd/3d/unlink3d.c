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
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
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
