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

#define atimeof(p)	(((p)+0)->tv_sec)
#define mtimeof(p)	(((p)+1)->tv_sec)

int
utimes3d(const char* path, const struct timeval* tms)
{
	register char*	sp;
	register int	r;
	time_t		atime;
	time_t		mtime;
#if FS
	Mount_t*	mp;
#endif

	if (state.in_2d)
		return(UTIMES(path, tms));
	if (tms)
	{
		atime = atimeof(tms);
		mtime = mtimeof(tms);
	}
	else atime = mtime = time((time_t*)0);
#if FS
	if (!fscall(NiL, MSG_utime, 0, path, atime, mtime))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_TOP, NiL)))
		return(-1);
	r = UTIMES(sp, tms);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_utime, 0, path, atime, mtime);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_utime))
				fscall(mp, MSG_utime, 0, path, atime, mtime);
	}
#endif
	return(r);
}
