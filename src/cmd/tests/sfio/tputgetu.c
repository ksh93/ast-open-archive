/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include	"sftest.h"

MAIN()
{
	unsigned int	i, r;
	Sfio_t	*fp;

	if(!(fp = sftmp(8)))
		terror("Can't open temp file\n");

	for(i = 10000; i <= 100000; i += 9)
		if(sfputu(fp,i) < 0)
			terror("Writing %u\n",i);

	sfseek(fp,(Sfoff_t)0,0);

	for(i = 10000; i <= 100000; i += 9)
		if((r = (unsigned int)sfgetu(fp)) != i)
			terror("Input=%u, Expect=%u\n",r,i);

	TSTEXIT(0);
}
