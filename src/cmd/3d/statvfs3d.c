/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
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
