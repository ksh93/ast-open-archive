/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * write n bytes to fd, using multiple write(2) if necessary
 */

#include "cslib.h"

ssize_t
cswrite(register Cs_t* state, int fd, const void* buf, register size_t n)
{
	register char*		p = (char*)buf;
	register ssize_t	i;

	while (n > 0)
	{
		messagef((state->id, NiL, -9, "write(%d,%d) `%-.*s'", fd, n, n - 1, (n > 0 && *((char*)buf + n - 1) == '\n') ? (char*)buf : "..."));
		if ((i = write(fd, p, n)) <= 0)
		{
			messagef((state->id, NiL, -9, "write(%d,%d) [%d]", fd, n, i));
			if (i && p == (char*)buf)
				return i;
			break;
		}
		n -= i;
		p += i;
	}
	return p - (char*)buf;
}

ssize_t
_cs_write(int fd, const void* buf, size_t n)
{
	return cswrite(&cs, fd, buf, n);
}
