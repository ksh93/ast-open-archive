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

int
close3d(int fd)
{
	initialize();
	if (fd >= 0 && fd < elementsof(state.file))
	{
		register int		nfd;
		register short*		rp;
#if FS
		register Mount_t*	mp;
#endif

#if defined(fchdir3d)
		if (state.file[fd].dir)
		{
			free(state.file[fd].dir);
			state.file[fd].dir = 0;
		}
#endif
		if (rp = state.file[fd].reserved)
		{
			if ((nfd = FCNTL(fd, F_DUPFD, fd + 1)) < 0)
			{
				errno = EBADF;
				return -1;
			}
			*rp = nfd;
			state.file[nfd].reserved = rp;
			state.file[nfd].flags = FILE_LOCK;
			state.file[fd].reserved = 0;
			CLOSE(fd);
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
