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

#ifdef	write3d

ssize_t
write3d(int fd, const void* buf, size_t n)
{
	register ssize_t 	r;
#if FS
	Mount_t*		mp;
	int			pos = 0;
	off_t			off;
	
	if (!fscall(NiL, MSG_write, 0, fd, buf, n))
		return(state.ret);
	mp = monitored();
#endif
	r = WRITE(fd, buf, n);
#if FS
	if (r >= 0)
	{
		if (mp)
		{
			if (!pos)
			{
				pos = 1;
				off = LSEEK(fd, 0, SEEK_CUR) - r;
			}
			fscall(mp, MSG_write3d, r, fd, buf, n, off);
		}
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_write))
			{
				if (!pos)
				{
					pos = 1;
					off = LSEEK(fd, 0, SEEK_CUR) - r;
				}
				fscall(mp, MSG_write3d, r, fd, buf, n, off);
			}
	}
#endif
	return(r);
}

#else

NoN(write)

#endif
