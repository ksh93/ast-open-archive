/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped

#include "3d.h"

#include <cs.h>

/*
 * peek siz chars from fd into buf
 * if not peekable but seekable then chars are peeked at offset 0
 */

ssize_t
peek(int fd, void* buf, size_t siz)
{
	register int	n;

	n = cspeek(&cs, fd, buf, siz);
	if (n >= 0)
		return(n);
	cspeek(&cs, -1, NiL, 0);
	if (!FSTAT(fd, &state.path.st) && state.path.st.st_size <= siz)
	{
		while ((n = read(fd, buf, siz)) != state.path.st.st_size && lseek(fd, 0L, 1) > 0 && lseek(fd, 0L, 0) == 0);
		if (n == state.path.st.st_size || n > 0 && !state.path.st.st_size)
			return(n);
	}
	return(-1);
}
