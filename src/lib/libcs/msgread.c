/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2003 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
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
