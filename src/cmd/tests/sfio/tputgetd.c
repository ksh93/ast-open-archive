/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"sftest.h"

MAIN()
{
	double	f, v;
	int	i;
	Sfio_t	*fp;

	if(!(fp = sfopen(NIL(Sfio_t*), tstfile(0), "w+")) )
		terror("Can't open temp file\n");

	for(f = 1e-10; f < 1e-10 + 1.; f += .001)
		if(sfputd(fp,f) < 0)
			terror("Writing %f\n",f);

	sfseek(fp,(Sfoff_t)0,0);
	for(f = 1e-10, i = 0; f < 1e-10 + 1.; f += .001, ++i)
		if((v = sfgetd(fp)) != f)
			terror("Element=%d Input=%f, Expect=%f\n",i,v,f);

	TSTEXIT(0);
}
