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

#ifdef chown3d

int
chown3d(const char* path, uid_t uid, gid_t gid)
{
	register char*	sp;
	register int	r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_chown, 0, path, uid, gid))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_SAFE|P_TOP, NiL)))
		return(-1);
	r = CHOWN(sp, uid, gid);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_chown, 0, path, uid, gid);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_chown))
				fscall(mp, MSG_chown, 0, path, uid, gid);
	}
#endif
	return(r);
}

#else

NoN(chown)

#endif
