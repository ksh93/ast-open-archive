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

ssize_t
readlink3d(const char* path, char* buf, register size_t size)
{
	size_t		r;
#if FS
	Mount_t*	mp;
#endif

	if (state.in_2d)
		return(READLINK(path, buf, size));
#if FS
	if (!fscall(NiL, MSG_readlink, 0, path, buf, size))
		return(state.ret);
	mp = monitored();
#endif
	if (!pathreal(path, P_READLINK, NiL))
		return(-1);

	/*
	 * see if link text is already in memory
	 */

	if (r = state.path.linksize)
	{
		if (r > state.path.linksize)
			r = state.path.linksize;
		memcpy(buf, state.path.linkname, r);
#if FS
		if (mp) fscall(mp, MSG_readlink, r, path, buf, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_readlink))
				fscall(mp, MSG_readlink, r, path, buf, size);
#endif
		return(r);
	}

	/*
	 * exists but not a symbolic link
	 */

	errno = EINVAL;
	return(-1);
}
