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

#ifdef ftruncate3d

#if !_nosys_ftruncate64

#undef	off_t
#undef	ftruncate

#if _UWIN
#define _ftruncate64	ftruncate64
#endif

extern int	_ftruncate64(int, off64_t);

int
ftruncate64(int fd, off64_t size)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_ftruncate, 0, fd, size))
		return state.ret;
	mp = monitored();
#endif
	if (_ftruncate64(fd, size))
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
