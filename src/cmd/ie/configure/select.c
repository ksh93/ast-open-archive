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
/*
 * see whether there is a select() that can be used for fine timing
 * This defines _SELECT4_ or _SELECT5_ if select() exists and delays
 */

#include	<sys/types.h>
#include	<sys/time.h>

main()
{
	time_t t1,t2;
#ifdef S4
	int milli = 2000;
#else
	struct timeval timeloc;
	timeloc.tv_sec = 2;
	timeloc.tv_usec = 0;
#endif /* S4 */
	time(&t1);
#ifdef S4
	select(0,(fd_set*)0,(fd_set*)0,milli);
#else
	select(0,(fd_set*)0,(fd_set*)0,(fd_set*)0,&timeloc);
#endif /* S4 */
	time(&t2);
	if(t2 > t1)
	{
#ifdef S4
		printf("#define _SELECT4_	1\n");
#else
		printf("#define _SELECT5_	1\n");
#endif /* S4 */
		exit(0);
	}
	exit(1);
}
