/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                         Pat Sullivan                         *
*                                                              *
***************************************************************/
#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<signal.h>

/*
 * only define _sys_timeb_ if ftime is there and works
 */

int sigsys();

main()
{
	struct timeb tb,ta;
	signal(SIGSYS,sigsys);
	ftime(&tb);
	sleep(2);
	ftime(&ta);
	if((ta.time-tb.time)>=1)
	{
		printf("#define _sys_timeb_	1\n");
		exit(0);
	}
	exit(1);
}

sigsys()
{
	exit(1);
}
