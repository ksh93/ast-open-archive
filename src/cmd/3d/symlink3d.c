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

int
symlink3d(const char* path, const char* target)
{
	register char*	sp;
	register char*	tp;
	register int	r;
	char*		t;
	int		oerrno;
	char		buf[PATH_MAX + 1];
#if FS
	char		tmp[PATH_MAX + 1];
	Mount_t*	mp;

	if (!fscall(NiL, MSG_symlink, 0, path, target))
		return(state.ret);
#endif
	if (!state.safe) sp = (char*)path;
	else if (!(sp = pathreal(path, P_PATHONLY|P_SAFE, NiL))) return(-1);
	else sp = strncpy(buf, sp, PATH_MAX);
#if FS
	mp = monitored();
#endif
	if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE, NiL)))
		return(-1);
	oerrno = errno;
	if ((r = SYMLINK(sp, tp)) && errno == ENOENT && (t = strrchr(tp, '/')))
	{
		*t = 0;
		r = fs3d_mkdir(tp, S_IRWXU|S_IRWXG|S_IRWXO);
		*t = '/';
		if (!r)
		{
			errno = oerrno;
			r = SYMLINK(sp, tp);
		}
	}
#if FS
	if (!r)
	{
		if (mp)
		{
			if (tp != tmp)
			{
				if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
					return(r);
				tp = strncpy(tmp, tp, PATH_MAX);
			}
			fscall(mp, MSG_symlink, 0, sp, tp);
		}
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_symlink))
			{
				if (tp != tmp)
				{
					if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
						return(r);
					tp = strncpy(tmp, tp, PATH_MAX);
				}
				fscall(mp, MSG_symlink, 0, sp, tp);
			}
	}
#endif
	return(r);
}
