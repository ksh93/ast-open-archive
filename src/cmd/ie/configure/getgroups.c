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
 * Check getgroups
 */

#include	<signal.h>
#ifdef _parm_
#   include	<param.h>
#endif
#ifdef _sysparm_
#   include	<sys/param.h>
#endif


extern int getgroups();
int sigsys();

main()
{
	int groups[100];
	int n;
	signal(SIGSYS,sigsys);
	n = getgroups(0,groups);
	if(n>0)
	{
		printf("#define MULTIGROUPS\t0\n");
		exit(2);
	}
	n = getgroups(100,groups);
	if(n>0)
	{
#ifdef NGROUPS
		printf("#define MULTIGROUPS\t%d\n",NGROUPS);
#endif /* NGROUPS */
		exit(0);
	}
	exit(1);
}

sigsys()
{
	exit(1);
}
