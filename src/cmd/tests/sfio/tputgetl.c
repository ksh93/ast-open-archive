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
	int	i, r;
	Sfio_t	*fp;

	if(!(fp = sftmp(8)))
		terror("Can't open temp file\n");

	for(i = -5448; i <= 5448; i += 101)
		if(sfputl(fp,(long)i) < 0)
			terror("Writing %d\n",i);

	sfseek(fp,(Sfoff_t)0,0);

	for(i = -5448; i <= 5448; i += 101)
		if((r = (int)sfgetl(fp)) != i)
			terror("Input=%d, Expect=%d\n",r,i);
	return 0;
}
