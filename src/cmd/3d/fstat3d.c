/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma noprototyped

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide fstat
#else
#define fstat		______fstat
#endif

#define _def_syscall_3d 1

#include "3d.h"

#undef	_def_syscall_3d

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide fstat
#else
#undef	fstat
#endif

#include "FEATURE/syscall"

/* the 3 arg _fxstat() disrupts our proto game -- every party needs one */
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
int fstat3d(int fd, struct stat* st)
#else
#if defined(_FSTAT)
int _fstat(fd, st) int fd; struct stat* st; { return fstat(fd, st); }
#endif
int fstat(fd, st) int fd; struct stat* st;
#endif
{
	int		oerrno;
#if FS
	Mount_t*	mp;

	if (!state.kernel)
	{
		if (!fscall(NiL, MSG_fstat, 0, fd, st))
			return state.ret;
		mp = monitored();
	}
#endif
#ifdef _3D_STAT_VER
	if (FXSTAT(_3d_ver, fd, st))
		return -1;
#else
	if (FSTAT(fd, st))
		return -1;
#endif
	if (state.kernel)
		return 0;
#if FS
	if (mp)
		fscall(mp, MSG_fstat, 0, fd, st);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fstat))
			fscall(mp, MSG_fstat, 0, fd, st);
#endif
	oerrno = errno;
#ifdef _3D_STAT64_VER
	if (_3d_ver == _3D_STAT64_VER)
		IVIEW(((struct stat64*)st), getfdview(fd));
	else
#endif
	IVIEW(st, getfdview(fd));
	errno = oerrno;
	return 0;
}

#if defined(_LARGEFILE64_SOURCE) && defined(STAT643D) && !defined(_3D_STAT64_VER)

int
fstat643d(int fd, struct stat64* st)
{
	int		oerrno;
	struct stat	ss;
#if FS
	Mount_t*	mp;

	if (!state.kernel)
	{
		if (!fscall(NiL, MSG_fstat, 0, fd, &ss))
			return state.ret;
		mp = monitored();
	}
#endif
	if (FSTAT64(fd, st))
		return -1;
	if (state.kernel)
		return 0;
#if FS
	if (mp)
		fscall(mp, MSG_fstat, 0, fd, st);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fstat))
			fscall(mp, MSG_fstat, 0, fd, st);
#endif
	oerrno = errno;
	IVIEW(st, getfdview(fd));
	errno = oerrno;
	return 0;
}

#endif
