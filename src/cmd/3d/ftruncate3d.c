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

#ifdef ftruncate3d

#if !_nosys_ftruncate64 && _typ_off64_t

#undef	off_t
#undef	ftruncate

typedef int (*Real_f)(int, off64_t);

int
ftruncate64(int fd, off64_t size)
{
	static Real_f	realf;
	int		r;
	int		n;
	int		m;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_ftruncate, 0, fd, size))
		return state.ret;
	mp = monitored();
#endif
	if (!realf)
		realf = (Real_f)sysfunc(SYS3D_ftruncate64);
	for (m = state.trap.size - 1; m >= 0; m--)
		if (MSG_MASK(MSG_truncate) & state.trap.intercept[m].mask)
			break;
	if (m >= 0)
	{
		n = state.trap.size;
		state.trap.size = m;
		r = (*state.trap.intercept[m].call)(&state.trap.intercept[m], MSG_truncate, SYS3D_ftruncate64, (void*)fd, (void*)&size, NiL, NiL, NiL, NiL);
		state.trap.size = n;
	}
	else
		r = (*realf)(fd, size);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_ftruncate, 0, fd, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_ftruncate))
				fscall(mp, MSG_ftruncate, 0, fd, size);
	}
#endif
	return r;
}

#endif

int
ftruncate3d(int fd, off_t size)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_ftruncate, 0, fd, size))
		return state.ret;
	mp = monitored();
#endif
	if (FTRUNCATE(fd, size))
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_ftruncate, 0, fd, size);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_ftruncate))
			fscall(mp, MSG_ftruncate, 0, fd, size);
#endif
	return 0;
}

#else

NoN(ftruncate)

#endif
