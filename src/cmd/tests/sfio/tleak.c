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
	Sfoff_t	m;

	if(sfopen(sfstdout, tstfile(0), "w") != sfstdout)
		terror("Can't open %s to write", tstfile(0));
	if(sfputr(sfstdout,"012345678\n",-1) != 10)
		terror("Can't write to %s", tstfile(0));

	if(sfopen(sfstdout, tstfile(1),"w") != sfstdout)
		terror("Can't open %s to write", tstfile(1));

	if(sfopen(sfstdin, tstfile(0), "r") != sfstdin)
		terror("Can't open %s to read", tstfile(0));

	if((m = sfmove(sfstdin,sfstdout, (Sfoff_t)SF_UNBOUND, -1)) != 10)
		terror("Moving data from %s to %s m=%lld",
			tstfile(0), tstfile(1), (Sflong_t)m);

	TSTEXIT(0);
}
