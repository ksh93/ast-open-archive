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
