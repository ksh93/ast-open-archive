/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped

#include "3d.h"

int
close3d(int fd)
{
	initialize();
	if (fd >= 0 && fd < elementsof(state.file))
	{
		register short*		rp;
#if FS
		register Mount_t*	mp;
#endif

		if (rp = state.file[fd].reserved)
		{
			if ((*rp = FCNTL(fd, F_DUPFD, RESERVED_FD)) >= 0)
			{
				state.file[*rp].reserved = rp;
				state.file[*rp].flags = FILE_LOCK;
			}
			state.file[fd].reserved = 0;
		}
#if FS
		if ((mp = state.file[fd].mount) && fssys(mp, MSG_close))
		{
			if (FSTAT(fd, &state.path.st))
				state.path.st.st_mtime = 0;
			fscall(mp, MSG_close, 0, fd, state.path.st.st_mtime);
		}
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_close))
			{
				if ((state.file[fd].flags & FILE_OPEN) ? FSTAT(fd, &state.path.st) : fileinit(fd, NiL, NiL, 0))
					state.path.st.st_mtime = 0;
				fscall(mp, MSG_close, 0, fd, state.path.st.st_mtime);
			}
		if (state.file[fd].flags)
		{
			state.file[fd].flags = 0;
			state.file[fd].mount = 0;
			if (state.cache == fd)
				while (state.cache > 1 && !state.file[--state.cache].flags);
		}
#endif
	}
	return(CLOSE(fd));
}
