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
