/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2009 AT&T Intellectual Property          *
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

#if __STD_C
int walkf(Sfio_t* f, Void_t* cntp)
#else
int walkf(f, cntp)
Sfio_t*	f;
Void_t* cntp;
#endif
{
	return (*((int*)cntp) += 1);
}

MAIN()
{
	Sfio_t	*f;
	int	c, count = 0;

	if((c = sfwalk(walkf, &count, 0)) != 3) /* counting sfstdin/out/err */
		terror("Bad count c=%d", c);

	if(!(f = sfopen(0, 0, "sw")) )
		terror("Sfopen failed");

	if((c = sfwalk(walkf, &count, 0)) != 7) /* recount std-streams and 1 more */
		terror("Bad count c=%d", c);

	TSTEXIT(0);
}
