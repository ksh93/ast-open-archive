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
 * make sure that there is a memset in the library and that  it works
 */

#ifdef BZERO
    extern char bzero();
#else
    extern char *memset();
#endif

char foobar[] = "abcdefg";
main()
{
	register int i;
#ifdef BZERO
	bzero(foobar,sizeof(foobar));
#else
	memset(foobar,0,sizeof(foobar));
#endif
	for(i=0; i < sizeof(foobar); i++)
		if(foobar[i])
			exit(1);
	exit(0);
}
