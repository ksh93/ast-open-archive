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
	Sfio_t*	ip;
	Sfio_t*	op;
	int	n;

	if(!(ip = sfopen((Sfio_t*)0, "/dev/null", "r")))
		terror("/dev/null read open\n");
	if(!(op = sfopen((Sfio_t*)0, tstfile(0), "w")))
		terror("Write open\n");

	n = (int)sfmove(ip, op, SF_UNBOUND, -1);

	if(n)
		terror("move count %d != 0\n", n);
	if(!sfeof(ip))
		terror("sfeof(ip) expected\n");
	if(sfeof(op))
		terror("sfeof(op) not expected\n");

	if(sferror(ip))
		terror("sferror(ip) not expected\n");
	if(sferror(op))
		terror("sferror(op) not expected\n");

	if(sfclose(ip))
		terror("sfclose(ip)\n");
	if(sfclose(op))
		terror("sfclose(op)\n");

	TSTEXIT(0);
}
