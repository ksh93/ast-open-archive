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
