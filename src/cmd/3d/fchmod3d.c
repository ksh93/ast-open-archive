/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped

#include "3d.h"

#ifdef fchmod3d

int
fchmod3d(int fd, mode_t mode)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fchmod, 0, fd, mode))
		return(state.ret);
	mp = monitored();
#endif
	if (FCHMOD(fd, mode))
		return(-1);
#if FS
	if (mp)
		fscall(mp, MSG_fchmod, 0, fd, mode);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fchmod))
			fscall(mp, MSG_fchmod, 0, fd, mode);
#endif
	return(0);
}

#else

NoN(fchmod)

#endif
