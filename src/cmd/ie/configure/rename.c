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
#include	<stdio.h>
/*
 * This program changes every occurence of "read" into "rEAd" from
 * standard input and writes the file to standard output
 * The return value is 0, if some change was made.
 */

main(argc,argv)
int argc;
char *argv[];
{
	register int c, state = 0, exitval=1;
	while((c = getchar()) != EOF)
	{
		switch(c)
		{
			case 'r':
				if(state)
					fwrite("read",1,state,stdout);
				state = 1;
				continue;
			case 'e':
				if(state!=1)
					break;
				state = 2;
				continue;
			case 'a':
				if(state!=2)
					break;
				state = 3;
				continue;
			case 'd':
				if(state!=3)
					break;
				fwrite("rEAd",1,4,stdout);
				state = 0;
				exitval = 0;
				continue;
		}
		if(state)
			fwrite("read",1,state,stdout);
		state = 0;
		putchar(c);
	}
	exit(exitval);
}
