/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"sftest.h"

MAIN()
{
	Sfdouble_t	f, v;
	int	i, flag;
	Sfio_t	*fp;

	if(!(fp = sfopen(NIL(Sfio_t*), tstfile(0), "w+")) )
		terror("Can't open temp file");

#define BEGV	(Sfdouble_t)(1e-10)
#define ENDV	(Sfdouble_t)(1e-10 + 1)
#define INCR	(Sfdouble_t)(1e-3)

	for(f = BEGV; f < ENDV; f += INCR)
		if(sfputd(fp,f) < 0)
			terror("Writing %.12Lf",f);

	sfseek(fp,(Sfoff_t)0,0);
	for(flag = 0, f = BEGV, i = 0; f < ENDV; ++i, f += INCR)
	{	if((v = sfgetd(fp)) == f)
			continue;
		if(v <= (f - 1e-10) || v >= (f + 1e-10) )
			terror("Element=%d Input=%.12Lf, Expect=%.12Lf",i,v,f);
		else if(!flag)
		{	twarn("Element=%d Input=%.12Lf, Expect=%.12Lf\n",i,v,f);
			flag = 1;
		}
	}

	TSTEXIT(0);
}
