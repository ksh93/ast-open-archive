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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide lseek
#else
#define lseek        ______lseek
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide lseek
#else
#undef  lseek
#endif

#include "FEATURE/syscall"

#ifdef	lseek3d

#if !_nosys_lseek64

typedef off64_t (*Seek64_f)(int, off64_t, int);

#undef	off_t
#undef	lseek

off64_t
lseek64(int fd, off64_t off, int op)
{
	off64_t		r;

	static Seek64_f	seekf;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_seek, 0, fd, off, op))
		return state.ret;
	mp = monitored();
#endif
	if (!seekf)
		seekf = (Seek64_f)sysfunc(SYS_lseek64);
	if ((r = (*seekf)(fd, off, op)) == -1)
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_seek, r, fd, off, op);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_seek))
			fscall(mp, MSG_seek, r, fd, off, op);
#endif
	return r;
}

#endif

typedef off_t (*Seek_f)(int, off_t, int);

off_t
lseek3d(int fd, off_t off, int op)
{
	off_t		r;

	static Seek_f	seekf;

#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_seek, 0, fd, off, op))
		return state.ret;
	mp = monitored();
#endif
	if (sizeof(off_t) > sizeof(long))
	{
		if (!seekf)
			seekf = (Seek_f)sysfunc(SYS_lseek);
		r = (*seekf)(fd, off, op);
	}
	else if ((r = LSEEK(fd, off, op)) == -1)
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_seek, r, fd, off, op);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_seek))
			fscall(mp, MSG_seek, r, fd, off, op);
#endif
	return r;
}

#else

NoN(lseek)

#endif
