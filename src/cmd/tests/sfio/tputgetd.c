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
	double	f, v;
	int	i;
	Sfio_t	*fp;

	if(!(fp = sfopen(NIL(Sfio_t*), sftfile(0), "w+")) )
		terror("Can't open temp file\n");

	for(f = 1e-10; f < 1e-10 + 1.; f += .001)
		if(sfputd(fp,f) < 0)
			terror("Writing %f\n",f);

	sfseek(fp,(Sfoff_t)0,0);
	for(f = 1e-10, i = 0; f < 1e-10 + 1.; f += .001, ++i)
		if((v = sfgetd(fp)) != f)
			terror("Element=%d Input=%f, Expect=%f\n",i,v,f);

	sftcleanup();
	return 0;
}
