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
