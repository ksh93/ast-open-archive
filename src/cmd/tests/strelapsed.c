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
#pragma prototyped

/*
 * AT&T Labs Research
 *
 * test harness for strelapsed
 */

#include <ast.h>

main(int argc, char** argv)
{
	char*		s;
	char*		e;
	unsigned long	t;
	int		n;

	while (s = *++argv)
	{
		n = (int)strtol(s, &e, 0);
		if (*e)
		{
			sfprintf(sfstderr, "%s: number expected", s);
			break;
		}
		if (!(s = *++argv))
		{
			sfprintf(sfstderr, "elapsed time expression expected");
			break;
		}
		t = strelapsed(s, &e, n);
		sfprintf(sfstdout, "strelapsed   \"%s\" \"%s\" %lu %d\n", s, e, t, n);
	}
	return 0;
}
