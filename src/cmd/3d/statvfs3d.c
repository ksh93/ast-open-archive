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

#if defined(statvfs3d) && defined(_sys_statvfs)

int
statvfs3d(const char* path, struct statvfs* fs)
{
	register char*	sp;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_statfs, 0, path, fs))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, 0, NiL)))
		return(-1);
	if (STATVFS(sp, fs))
		return(-1);
#if FS
	if (mp)
		fscall(mp, MSG_statfs, 0, state.path.name, fs);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_statfs))
			fscall(mp, MSG_statfs, 0, state.path.name, fs);
#endif
	return(0);
}

#else

NoN(statvfs)

#endif
