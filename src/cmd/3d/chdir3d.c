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

int
chdir3d(const char* path)
{
	register char*	sp;
	char		buf[PATH_MAX + 1];

#if FS
	if (!fscall(NiL, MSG_stat, 0, path, &state.path.st))
	{
		if (state.ret)
			return -1;
		if (!S_ISDIR(state.path.st.st_mode))
		{
			errno = ENOTDIR;
			return -1;
		}
		state.level = 1;
	}
	else
#else
	initialize();
#endif
	{
		if (state.level > 0 && state.pwd && !CHDIR(state.pwd))
			state.level = 0;
		if (!(sp = pathreal(path, P_SAFE, NiL)))
			return -1;
		if (CHDIR(sp))
			return -1;
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
		else
			sp = buf;
		strcpy(sp, path);
#if 0
		state.path.level = 0;
#endif
		if ((sp = pathcanon(buf, 0)) && *(sp - 1) == '.' && *(sp - 2) == '/')
			*(sp -= 2) = 0;
		state.pwdsize = strcopy(state.pwd, buf) - state.pwd;
		memcpy(state.envpwd + sizeof(var_pwd) - 1, state.pwd, state.pwdsize);
		state.level = state.path.level;
		message((-1, "chdir: %s [%d]", state.pwd, state.level));
	}
	return 0;
}
