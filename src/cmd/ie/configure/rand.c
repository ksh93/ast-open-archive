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
 * limit random numbers to the range 0 to 2^15
 * some pre-processors require an argument so a dummy one is used
 */

main()
{
	unsigned  max=0;
	int n=100;
	unsigned x =0;
	if(sizeof(int)==2)
	{
		printf("#define sh_rand(x) rand(x)\n");
		exit(0);
	}
	srand(getpid());
	while(n--)
	{
		x=rand();
		if(x>max)
			max=x;
	}
	if(max > 077777)
		printf("#define sh_rand(x) ((x),(rand()>>3)&077777)\n");
	else
		printf("#define sh_rand(x) ((x),rand())\n");
	exit(0);
}
