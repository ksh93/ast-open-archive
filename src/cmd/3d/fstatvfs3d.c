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

#if defined(fstatvfs3d) && defined(_sys_statvfs)

#include <sys/statvfs.h>

int
fstatvfs3d(int fd, struct statvfs* fs)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fstatfs, 0, fd, fs))
		return(state.ret);
	mp = monitored();
#endif
	if (FSTATVFS(fd, fs))
		return(-1);
#if FS
	if (mp)
		fscall(mp, MSG_fstatfs, 0, fd, fs);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fstatfs))
			fscall(mp, MSG_fstatfs, 0, fd, fs);
#endif
	return(0);
}

#else

NoN(fstatvfs)

#endif
