/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
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
