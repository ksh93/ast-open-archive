/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
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
