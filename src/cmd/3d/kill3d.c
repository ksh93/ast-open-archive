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

#ifdef	kill3d

int
kill3d(pid_t pid, int sig)
{
#if FS
	Mount_t*	mp;

#endif
	if (KILL(pid, sig))
		return(-1);
#if FS
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_kill))
			fscall(mp, MSG_kill, 0, pid, sig);
#endif
	return(0);
}

#else

NoN(kill)

#endif
