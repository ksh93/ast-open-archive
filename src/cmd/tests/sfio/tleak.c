/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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

	TSTEXIT(0);
}
