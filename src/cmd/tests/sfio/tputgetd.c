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
	Sfdouble_t	f, v;
	int	i, flag;
	Sfio_t	*fp;

	if(!(fp = sfopen(NIL(Sfio_t*), tstfile(0), "w+")) )
		terror("Can't open temp file\n");

#define BEGV	(Sfdouble_t)(1e-10)
#define ENDV	(Sfdouble_t)(1e-10 + 1)
#define INCR	(Sfdouble_t)(1e-3)

	for(f = BEGV; f < ENDV; f += INCR)
		if(sfputd(fp,f) < 0)
			terror("Writing %.12Lf\n",f);

	sfseek(fp,(Sfoff_t)0,0);
	for(flag = 0, f = BEGV, i = 0; f < ENDV; ++i, f += INCR)
	{	if((v = sfgetd(fp)) == f)
			continue;
		if(v <= (f - 1e-10) || v >= (f + 1e-10) )
			terror("Element=%d Input=%.12Lf, Expect=%.12Lf\n",i,v,f);
		else if(!flag)
		{	twarn("Element=%d Input=%.12Lf, Expect=%.12Lf\n",i,v,f);
			flag = 1;
		}
	}

	TSTEXIT(0);
}
