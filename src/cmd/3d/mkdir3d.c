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
__STDPP__directive pragma pp:hide mkdir
#else
#define mkdir        ______mkdir
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mkdir
#else
#undef  mkdir
#endif

#include "FEATURE/syscall"

/*
 * mkdir() will always create a directory on the highest layer
 * mkdir() will create intermediate directories if they virtually exist
 * mkdir() will do a chdir() if a virtual dot directory is created
 */

int
mkdir3d(const char* path, mode_t mode)
{
	register char*	sp;
	register char*	cp;
	register int	r;
	char		buf[PATH_MAX + 1];
#if FS
	Mount_t*	mp;
#endif

	if (state.real)
	{
#if FS
		mp = 0;
#endif
		sp = (char*)path;
	}
#if FS
	else if (!fscall(NiL, MSG_mkdir, 0, path, mode))
		return(state.ret);
#endif
	else
	{
#if FS
		mp = monitored();
#endif
		if (!(sp = pathreal(path, P_PATHONLY|P_NOOPAQUE, NiL)))
			return(-1);
	}
	if (state.path.level || state.level && (sp == state.pwd || streq(sp, state.pwd)) || (r = MKDIR(sp, mode)) && errno == ENOENT)
	{
		/*
		 * copy canonicalized pathname into buf
		 */

		if (*sp != '/') sp = state.path.name;
		cp = buf;
		while (*cp = *sp++) cp++;
		do
		{
			while (*--cp != '/');
			if (cp <= buf) return(-1);
			*cp = 0;
			r = !pathreal(buf, 0, NiL) || !streq(buf, state.path.name);
			*cp = '/';
		} while (r);
		*cp = '/';
		do
		{
			r = *++cp;
			if (r == 0 || r == '/')
			{
				*cp = 0;
				sp = pathreal(buf, P_PATHONLY, NiL);
				*cp = r;
				if (!sp) return(-1);
				if (sp == state.dot) sp = state.pwd;
				if (r = MKDIR(sp, mode)) return(r);
#if FS
				if (fsmonitored(state.path.name) && !LSTAT(state.path.name, &state.path.st))
					fscall(state.path.monitor, MSG_mkdir, 0, state.path.name, state.path.st.st_mode);
#endif
				if (sp == state.pwd || streq(sp, state.pwd))
				{
					state.level = 0;
					CHDIR(state.pwd);
				}
			}
		} while (*cp);
	}
#if FS
	if (!r)
	{
		if (mp) fscall(mp, MSG_mkdir, 0, path, mode);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_mkdir))
				fscall(mp, MSG_mkdir, 0, path, mode);
	}
#endif
	return(r);
}

/*
 * 3d internal mkdir to skip pathreal(path, ...)
 */

int
fs3d_mkdir(const char* path, mode_t mode)
{
	int	r;

	state.real++;
	r = mkdir(path, mode);
	state.real--;
	return(r);
}
