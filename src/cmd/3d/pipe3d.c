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

#ifdef pipe3d

int
pipe3d(int* fds)
{
	int		r;
#if !_mangle_syscall
	int		fd;
#endif

	initialize();
#if !_mangle_syscall
	if (r = (state.fs[FS_option].flags & FS_ON) != 0) state.fs[FS_option].flags &= ~FS_ON;
	for (fd = 0; fd < OPEN_MAX; fd++)
		if ((fds[0] = DUP(fd)) >= 0)
		{
			fds[1] = DUP(fds[0]);
			CLOSE(fds[1]);
			CLOSE(fds[0]);
			break;
		}
	if (r) state.fs[FS_option].flags |= FS_ON;
#endif
	r = PIPE(fds);
#if FS
	if (!r && !state.in_2d)
	{
		Mount_t*	mp;

		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_pipe))
				fscall(mp, MSG_pipe, 0, fds);
	}
#endif
	return(r);
}

#else

NoN(pipe)

#endif
