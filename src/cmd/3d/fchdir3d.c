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

#if defined(fchdir3d)

int
fchdir3d(int fd)
{
	Dir_t*		dp;
	struct stat	st;

	if (FCHDIR(fd))
		return -1;
	if (fd >= 0 && fd < elementsof(state.file) && (dp = state.file[fd].dir) && !FSTAT(fd, &st) && dp->dev == st.st_dev && dp->ino == st.st_ino)
		chdir(dp->path);
	return 0;
}

#else

NoP(fchdir)

#endif
