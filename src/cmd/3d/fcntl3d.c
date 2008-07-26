/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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

#ifdef	fcntl3d

#include <stdarg.h>

int
fcntl3d(int fd, int op, ...)
{
	register int	r;
	void*		arg;
	va_list		ap;

	initialize();
	va_start(ap, op);
	arg = va_arg(ap, void*);
	va_end(ap);
	if (op == F_DUPFD && state.file[((int)arg)].reserved) close((int)arg);
	r = FCNTL(fd, op, arg);
#if FS
	if (r >= 0 && r < elementsof(state.file)) switch (op)
	{
	case F_DUPFD:
		fs3d_dup(fd, r);
		break;
	case F_SETFD:
		if (state.cache)
		{
			if (!(((int)arg) & FD_CLOEXEC))
				state.file[fd].flags &= ~FILE_CLOEXEC;
			else if (!(state.file[fd].flags & FILE_OPEN))
				fileinit(fd, NiL, NiL, 1);
			else state.file[fd].flags |= FILE_CLOEXEC;
		}
		break;
	}
#endif
	return(r);
}

#else

NoN(fcntl)

#endif
