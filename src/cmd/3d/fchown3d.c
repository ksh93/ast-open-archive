/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

#include "3d.h"

#ifdef fchown3d

int
fchown3d(int fd, uid_t uid, gid_t gid)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fchown, 0, fd, uid, gid))
		return state.ret;
	mp = monitored();
#endif
	if (FCHOWN(fd, uid, gid))
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_fchown, 0, fd, uid, gid);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fchown))
			fscall(mp, MSG_fchown, 0, fd, uid, gid);
#endif
	return 0;
}

#else

NoN(fchown)

#endif
