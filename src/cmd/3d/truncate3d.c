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

#ifdef truncate3d

#if !_nosys_truncate64 && _typ_off64_t

#undef	off_t
#undef	ftruncate

typedef int (*Real_f)(const char*, off64_t);

int
truncate643d(const char* path, off64_t size)
{
	register char*	sp;
	register int	r;
	int		n;
	int		m;
	static Real_f	realf;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_truncate, 0, path, size))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_TOP, NiL)))
		return -1;
	if (!realf)
		realf = (Real_f)sysfunc(SYS3D_truncate64);
	for (m = state.trap.size - 1; m >= 0; m--)
		if (MSG_MASK(MSG_truncate) & state.trap.intercept[m].mask)
			break;
	if (m >= 0)
	{
		n = state.trap.size;
		state.trap.size = m;
		r = (*state.trap.intercept[m].call)(&state.trap.intercept[m], MSG_truncate, SYS3D_truncate64, (void*)sp, (void*)&size, NiL, NiL, NiL, NiL);
		state.trap.size = n;
	}
	else
		r = (*realf)(sp, size);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_truncate, 0, path, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_truncate))
				fscall(NiL, MSG_truncate, 0, path, size);
	}
#endif
	return r;
}

#endif

int
truncate3d(const char* path, off_t size)
{
	register char*	sp;
	register int	r;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_truncate, 0, path, size))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_TOP, NiL)))
		return -1;
	r = TRUNCATE(sp, size);
#if FS
	if (!r)
	{
		if (mp)
			fscall(NiL, MSG_truncate, 0, path, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_truncate))
				fscall(NiL, MSG_truncate, 0, path, size);
	}
#endif
	return r;
}

#else

NoN(truncate)

#endif
