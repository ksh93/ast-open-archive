/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide mknod
#else
#define mknod        ______mknod
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mknod
#else
#undef  mknod
#endif

#include "FEATURE/syscall"

#ifdef mknod3d

int
mknod3d(const char* path, mode_t mode, dev_t dev)
{
 	register char*	sp;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_mknod, 0, path, mode, dev))
		return(state.ret);
	mp = monitored();
#endif
 	if (!(sp = pathreal(path, P_PATHONLY, NiL)) || MKNOD(sp, mode, dev))
		return(-1);
#if FS
 	if (mp && !STAT(sp, &state.path.st))
 		fscall(mp, MSG_mknod, 0, path, state.path.st.st_mode, dev);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_mknod))
			fscall(mp, MSG_mknod, 0, path, mode, dev);
#endif
	return(0);
}

#else

NoN(mknod)

#endif
