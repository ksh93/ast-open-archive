/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1999-2000 AT&T Corp.              *
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
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#include	"sftest.h"

main()
{
	Sfoff_t	m;

	if(sfopen(sfstdout, sftfile(0), "w") != sfstdout)
		terror("Can't open %s to write\n", sftfile(0));
	if(sfputr(sfstdout,"012345678\n",-1) != 10)
		terror("Can't write to %s\n", sftfile(0));

	if(sfopen(sfstdout, sftfile(1),"w") != sfstdout)
		terror("Can't open %s to write\n", sftfile(1));

	if(sfopen(sfstdin, sftfile(0), "r") != sfstdin)
		terror("Can't open %s to read\n", sftfile(0));

	if((m = sfmove(sfstdin,sfstdout, (Sfoff_t)SF_UNBOUND, -1)) != 10)
		terror("Can't move data from %s to %s\n", sftfile(0), sftfile(1));

	sftcleanup();
	return 0;
}
