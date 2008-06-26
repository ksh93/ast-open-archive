/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
