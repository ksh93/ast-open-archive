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

#if _hdr_alloca

#include <alloca.h>

#else

void
fix(void)
{
	if (state.brk.beg)
	{
		ssize_t	n;

		if (state.brk.end && (n = state.brk.end - state.brk.beg) > 0)
		{
			memset(state.brk.beg, 0, n);
			if ((char*)sbrk(0) == state.brk.end)
				sbrk(-n);
		}
		state.brk.beg = state.brk.end = 0;
	}
}

#endif

int
execve3d(const char* path, char* const* aargv, char* const* aarge)
{
	char**		argv = (char**)aargv;
	char**		arge = (char**)aarge;
	register char*	sp;
	register char*	ep;
	register int	size;
	register int	argn = 0;
	int		n;
	char*		tp;
	char		buf[PATH_MAX];
#if FS
	Mount_t*	mp;
#endif

	if (!(sp = pathreal(path, 0, NiL)))
		return(-1);
	if (state.safe && !state.path.level)
	{
		errno = EPERM;
		return(-1);
	}
	if (*sp == '/')
	{
#if HUH920211 /* mh corrupts static state strings */
		if (streq(sp, state.binsh)) sp = state.shell;
#else
		if (streq(sp, "/bin/sh")) sp = state.shell;
#endif
	}
	else if (!state.in_2d && --sp > state.path.name) *--sp = '.';
	sp = strncpy(buf, sp, sizeof(buf));
	if (state.level)
	{
		/*
		 * make sure dot is set correctly if not top level
		 */

		pathreal(state.pwd, 0, NiL);
	}
	if (arge == state.env + 1) arge--;
	else
	{
		register char**	op = arge;

		/*
		 * compute size of environment pointers
		 */

		while (*op++) argn++;
		argn += 2;
		argn *= sizeof(char*);
	}
	size = mapdump(NiL, NiL, MAP_EXEC);
	n = size + argn + 10;
	n = roundof(n, 32);
#if _hdr_alloca
	state.brk.beg = (char*)alloca(n);
#else
	state.brk.beg = (char*)sbrk(n);
	state.brk.end = (char*)sbrk(0);
#endif
	ep = state.brk.beg + argn + 10 - sizeof(var_3d) + 1;
	tp = strcopy(ep, var_3d);
	size = mapdump(NiL, tp, MAP_EXEC);
	if (!keep(tp, size, 1))
		reclaim();
	else
	{
		if (argn)
		{
			register char**	op = arge;
			register char**	np;

			arge = (char**)state.brk.beg;
			np = arge + 1;
			while (*np++ = *op++);
		}
		arge[0] = ep;
	}
#if FS
	for (mp = state.global; mp; mp = mp->global)
	{
		if (fssys(mp, MSG_exec))
			fscall(mp, MSG_exec, 0, sp, argv, arge);
		if (fssys(mp, MSG_close))
		{
			register File_t*	fp;

			for (fp = state.file; fp <= state.file + state.open; fp++)
				if ((fp->flags & FILE_OPEN) && ((fp->flags & FILE_CLOEXEC) || (size = FCNTL(fp - state.file, F_GETFD, NiL)) >= 0 && (size & FD_CLOEXEC)) && !FSTAT(fp - state.file, &state.path.st))
					fscall(mp, MSG_close, 0, fp - state.file, state.path.st.st_mtime);
		}
	}
#endif
	EXECVE(sp, argv, arge);
	reclaim();
	return(-1);
}
