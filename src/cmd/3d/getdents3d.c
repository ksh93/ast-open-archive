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

#ifdef	getdents3d

ssize_t
getdents3d(int fd, void* buf, size_t n)
{
	ssize_t		r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_getdents, 0, fd, buf, n))
		return(state.ret);
	mp = monitored();
#endif
	if ((r = GETDENTS(fd, buf, n)) >= 0)
	{
#if FS
		if (mp)
			fscall(mp, MSG_getdents, r, fd, buf, n);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_getdents))
				fscall(mp, MSG_getdents, r, fd, buf, n);
#endif
	}
	return(r);
}

#else

NoN(getdents)

#endif
