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
