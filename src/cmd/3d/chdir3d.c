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

int
chdir3d(const char* path)
{
	register char*	sp;
	char		buf[PATH_MAX + 1];

#if FS
	if (!fscall(NiL, MSG_stat, 0, path, &state.path.st))
	{
		if (state.ret) return(-1);
		if (!S_ISDIR(state.path.st.st_mode))
		{
			errno = ENOTDIR;
			return(-1);
		}
		state.level = 1;
	}
	else
#else
	initialize();
#endif
	{
		if (state.level > 0 && state.pwd && !CHDIR(state.pwd)) state.level = 0;
		if (!(sp = pathreal(path, P_SAFE, NiL))) return(-1);
		if (CHDIR(sp)) return(-1);
	}
	if (state.pwd)
	{
		/*
		 * save absolute path in state.pwd
		 */

		if (*path != '/')
		{
			strcpy(buf, state.pwd);
			sp = buf + state.pwdsize;
			*sp++ = '/';
		}
		else sp = buf;
		strcpy(sp, path);
		if ((sp = pathcanon(buf, 0)) && *(sp - 1) == '.' && *(sp - 2) == '/') *(sp -= 2) = 0;
		state.pwdsize = strcopy(state.pwd, buf) - state.pwd;
		memcpy(state.envpwd + sizeof(var_pwd) - 1, state.pwd, state.pwdsize);
		state.level = state.path.level;
		message((-1, "chdir: %s [%d]", state.pwd, state.level));
	}
	return(0);
}
