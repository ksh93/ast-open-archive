/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
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
