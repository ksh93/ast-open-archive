/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
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
