/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * return pointer to formatted elapsed time for t 1/n secs
 * compatible with strelapsed()
 * return value length is at most 6
 */

#include <ast.h>

char*
fmtelapsed(register unsigned long t, register int n)
{
	register unsigned long	s;
	char*			buf;
	int			z;

	static int		amt[] = { 1, 60, 60, 24, 7, 4, 12, 20 };
	static char		chr[] = "smhdwMYS";

	if (t == 0L)
		return "0";
	if (t == ~0L)
		return "%";
	buf = fmtbuf(z = 7);
	s = t / n;
	if (s < 60)
		sfsprintf(buf, z, "%d.%02ds", s % 100, (t * 100 / n) % 100);
	else
	{
		for (n = 1; n < elementsof(amt) - 1; n++)
		{
			if ((t = s / amt[n]) < amt[n + 1])
				break;
			s = t;
		}
		sfsprintf(buf, z, "%d%c%02d%c", (s / amt[n]) % 100, chr[n], s % amt[n], chr[n - 1]);
	}
	return buf;
}
