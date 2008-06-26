/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
 * syscall message server side receipt
 */

#include "msglib.h"

#include <cs.h>

/*
 * read a message from fd into buf,siz
 * handles record boundaries on fd
 * siz must be > MSG_SIZE_SIZE
 */

ssize_t
msgread(int fd, char* buf, size_t siz)
{
	register ssize_t	n;
	register int		o;

	if ((n = cspeek(&cs, fd, buf, MSG_SIZE_SIZE)) >= 0) o = 0;
	else
	{
		n = csread(&cs, fd, buf, MSG_SIZE_SIZE, CS_EXACT);
		o = MSG_SIZE_SIZE;
	}
	if (n != MSG_SIZE_SIZE) return n ? -1 : 0;
	n = msggetsize(buf);
	if (n <= MSG_SIZE_SIZE || n > siz || csread(&cs, fd, buf + o, n - o, CS_EXACT) != n - o) return -1;
	return n;
}
