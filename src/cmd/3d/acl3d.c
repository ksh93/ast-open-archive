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
#pragma prototyped

#include "3d.h"

#if !defined(ACL3D) || !defined(SETACL)

#define aclent_t	char
#define SETACL		1

#endif

#define MSG_acl		MSG_INIT(MSG_control, 051121, MSG_VAR_FILE)

int
acl3d(const char* path, int cmd, int cnt, aclent_t* buf)
{
	register char*		sp;
	register int		r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_acl, 0, path, cmd, cnt, buf))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, cmd == SETACL ? (state.safe ? (P_SAFE|P_TOP) : P_TOP) : 0, NiL)))
		return -1;
	r = ACL(sp, cmd, cnt, buf);
#if FS
	if (r >= 0)
	{
		if (mp)
			fscall(mp, MSG_acl, 0, state.path.name, cmd, cnt, buf);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_acl))
				fscall(mp, MSG_acl, 0, state.path.name, cmd, cnt, buf);
	}
#endif
	return r;
}
