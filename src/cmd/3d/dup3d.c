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

#if FS

int
fs3d_dup(int ofd, int nfd)
{
	register Mount_t*	mp;

	if (nfd >= 0 && ofd >= 0 && ofd < elementsof(state.file))
	{
		if (state.cache)
		{
			if (!(state.file[ofd].flags & FILE_OPEN))
				fileinit(ofd, NiL, NiL, 0);
			state.file[nfd] = state.file[ofd];
			state.file[nfd].flags &= ~FILE_CLOEXEC;
			state.file[nfd].reserved = 0;
			if (nfd > state.cache)
				state.cache = nfd;
			if (nfd > state.open)
				state.open = nfd;
			if (mp = state.file[nfd].mount)
				fscall(mp, MSG_dup, nfd, ofd);
			for (mp = state.global; mp; mp = mp->global)
				if (fssys(mp, MSG_dup))
					fscall(mp, MSG_dup, nfd, ofd);
		}
#if defined(fchdir3d)
		if (state.file[nfd].dir)
		{
			free(state.file[nfd].dir);
			state.file[nfd].dir = 0;
		}
		if (state.file[ofd].dir && (state.file[nfd].dir = newof(0, Dir_t, 1, strlen(state.file[ofd].dir->path))))
		{
			strcpy(state.file[nfd].dir->path, state.file[ofd].dir->path);
			state.file[nfd].dir->dev = state.file[ofd].dir->dev;
			state.file[nfd].dir->ino = state.file[ofd].dir->ino;
		}
#endif
	}
	return 0;
}

#endif

#ifdef dup3d

int
dup3d(int fd)
{
	register int	r;

	initialize();
	r = DUP(fd);
#if FS
	if (r >= 0 && r < elementsof(state.file))
		fs3d_dup(fd, r);
#endif
	return r;
}

#else

NoN(dup)

#endif
