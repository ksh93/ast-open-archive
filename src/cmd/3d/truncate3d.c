/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

#include "3d.h"

#ifdef truncate3d

#if !_nosys_truncate64

#if _UWIN
#define _truncate64	truncate64
#endif

extern int	_truncate64(const char*, off64_t);

int
truncate643d(const char* path, off64_t size)
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
	r = _truncate64(sp, size);
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
