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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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

MAIN()
{
	Sfoff_t	m;

	if(sfopen(sfstdout, tstfile(0), "w") != sfstdout)
		terror("Can't open %s to write\n", tstfile(0));
	if(sfputr(sfstdout,"012345678\n",-1) != 10)
		terror("Can't write to %s\n", tstfile(0));

	if(sfopen(sfstdout, tstfile(1),"w") != sfstdout)
		terror("Can't open %s to write\n", tstfile(1));

	if(sfopen(sfstdin, tstfile(0), "r") != sfstdin)
		terror("Can't open %s to read\n", tstfile(0));

	if((m = sfmove(sfstdin,sfstdout, (Sfoff_t)SF_UNBOUND, -1)) != 10)
		terror("Moving data from %s to %s m=%lld\n",
			tstfile(0), tstfile(1), (Sflong_t)m);

	TSTRETURN(0);
}
