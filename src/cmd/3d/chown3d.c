/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
