/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
