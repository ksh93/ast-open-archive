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
*    If you have copied or used this software without agreeing     *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

#include "3d.h"

#include "dir_3d.h"

static int
rmfiles(char* path, char* endpath, ino_t ino)
{
	register DIRDIR*		dp;
	register struct DIRdirent*	ep;
	register char*			s;
	register char*			t;
	int				pass = 0;

	*endpath = 0;
	if (!(dp = OPENDIR(path))) return(0);
	*endpath++ = '/';
	for (;;)
	{
		if (!(ep = (struct DIRdirent*)READDIR(dp)))
		{
			if (pass++)
			{
				CLOSEDIR(dp);
				return(1);
			}
			SEEKDIR(dp, 0L);
		}
		else
		{
#if _mem_d_fileno_dirent
			if (ep->d_fileno != ino)
#else
			struct stat	st;

			strcpy(endpath, ep->d_name);
			if (STAT(path, &st)) break;
			if (st.st_ino != ino)
#endif
			{
				s = state.opaque;
				t = ep->d_name;
				while (*s && *s++ == *t) t++;
				if (*s || *t) break;
			}
			else if (pass)
			{
#if _mem_d_fileno_dirent
				strcpy(endpath, ep->d_name);
#endif
				if (UNLINK(path)) break;
			}
		}
	}
	CLOSEDIR(dp);
	return(0);
}

int
rmdir3d(const char* path)
{
	register char*	sp;
	register int	r;
	int		pass = 0;
	struct stat	st;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_rmdir, 0, path))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_PATHONLY, NiL)))
		return(-1);
	if (state.path.level)
		return(0);
 retry:
	r = RMDIR(sp);
	if (!r)
	{
#if FS
		if (mp) fscall(mp, MSG_rmdir, 0, path);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_rmdir))
				fscall(mp, MSG_rmdir, 0, path);
#endif
	}
	else
	{
		if (errno == ENOENT && pathreal(path, 0, NiL))
			return(0);
		if (errno == ENOTEMPTY && !pass++)
		{
			register char*	slast = sp;

			while (*slast) slast++;
			*slast = '/';
			strcpy(slast+1, state.opaque);
			if (!LSTAT(sp, &st))
			{
				if (rmfiles(sp, slast, st.st_ino))
				{
					*slast = 0;
					goto retry;
				}
			}
			else errno = ENOTEMPTY;
		}
	}
	return(r);
}
