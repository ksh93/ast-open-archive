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
