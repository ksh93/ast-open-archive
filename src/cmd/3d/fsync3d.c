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

#ifdef fsync3d

int
fsync3d(int fd)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fsync, 0, fd))
		return(state.ret);
	mp = monitored();
#endif
	if (FSYNC(fd))
		return(-1);
#if FS
	if (mp)
		fscall(mp, MSG_fsync, 0, fd);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fsync))
			fscall(mp, MSG_fsync, 0, fd);
#endif
	return(0);
}

#else

NoN(fsync)

#endif
