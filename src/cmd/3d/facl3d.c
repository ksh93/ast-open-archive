/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped

#include "3d.h"

#if !defined(ACL3D) || !defined(SETACL)

#define aclent_t	char

#endif

#define MSG_facl	MSG_INIT(MSG_control, 051131, MSG_VAR_FILE)

int
facl3d(int fd, int cmd, int cnt, aclent_t* buf)
{
	register int	r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_facl, 0, fd, cmd, cnt, buf))
		return(state.ret);
	mp = monitored();
#endif
	r = FACL(fd, cmd, cnt, buf);
#if FS
	if (r >= 0)
	{
		if (mp)
			fscall(mp, MSG_facl, 0, fd, cmd, cnt, buf);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_facl))
				fscall(mp, MSG_facl, 0, fd, cmd, cnt, buf);
	}
#endif
	return(r);
}
