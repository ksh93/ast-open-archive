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
#pragma noprototyped

#include "3d.h"

#if defined(lstat3d)

/* the 3 arg _lxstat() disrupts our proto game -- every party needs one */
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
int lstat3d(const char* path, struct stat* st)
#else
#if defined(_LSTAT)
int _lstat(path, st) char* path; struct stat* st; { return lstat(path, st); }
#endif
int lstat(path, st) char* path; struct stat* st;
#endif
{
	register char*	sp;
#if FS
	Mount_t*	mp;
#endif
#ifdef _3D_STAT_VER
	struct stat*	so;
	struct stat	sc;
#endif

#ifdef _3D_STAT_VER
	if (_3d_ver != _3D_STAT_VER)
	{
		so = st;
		st = &sc;
	}
#endif
#if FS
	if (!fscall(NiL, MSG_lstat, 0, path, st))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_LSTAT, st)))
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_lstat, 0, state.path.name, st);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_lstat))
			fscall(mp, MSG_lstat, 0, state.path.name, st);
#endif
#ifdef _3D_STAT_VER
	if (st == &sc)
	{
		st = so;
		if (LXSTAT(_3d_ver, sp, st))
			return -1;
#ifdef _3D_STAT64_VER
		if (_3d_ver == _3D_STAT64_VER)
			IVIEW(((struct stat64*)st), state.path.level);
		else
#endif
		IVIEW(st, state.path.level);
	}
#endif
	return 0;
}

#if defined(_LARGEFILE64_SOURCE) && defined(STAT643D) && !defined(_3D_STAT64_VER)

int
lstat643d(const char* path, struct stat64* st)
{
	register char*	sp;
	struct stat	ss;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_lstat, 0, path, &ss))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_LSTAT, &ss)))
		return -1;
	if (LSTAT64(sp, st))
		return -1;
	IVIEW(st, state.path.level);
#if FS
	if (mp)
		fscall(mp, MSG_lstat, 0, state.path.name, &ss);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_lstat))
			fscall(mp, MSG_lstat, 0, state.path.name, &ss);
#endif
	return 0;
}

#endif

#else

NoN(lstat)

#endif
