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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide chmod
#else
#define chmod        ______chmod
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide chmod
#else
#undef  chmod
#endif

#include "FEATURE/syscall"

int
chmod3d(const char* path, mode_t mode)
{
	register char*	sp;
	register int	r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_chmod, 0, path, mode))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, state.safe ? (P_SAFE|P_TOP) : P_TOP, NiL)))
		return(-1);
	r = CHMOD(sp, mode);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_chmod, 0, state.path.name, mode);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_chmod))
				fscall(mp, MSG_chmod, 0, state.path.name, mode);
	}
#endif
	return(r);
}
