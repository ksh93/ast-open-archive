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
 * This program sets the VFORK options if appropriate
 * It compare the time run fork/exec against the time to run vfork/exec
 */

#define MAX 50

char dummy[10000];

#include <stdio.h>

main()
{
	long t1, t2, t3;
	int delta;
	int n,p;
	time(&t1);
	n = MAX;
	while(n--)
	{
		if((p=fork())==0)
		{
			execl("/bin/sh","sh", "-c", ":", 0);
			exit(0);
		}
		if(p>0)
			wait(&p);
	}
	time(&t2);
	n = MAX;
	while(n--)
	{
		if((p=vfork(1))==0)
		{
			execl("/bin/sh","sh", "-c", ":", 0);
			exit(0);
		}
		if(p>0)
			wait(&p);
	}
	time(&t3);
	delta = t3-t2;
	if(delta==0)
		delta = 1;
	n = (t2-t1)/delta;
	exit(n<5);
}
	
