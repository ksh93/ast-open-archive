/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2008 AT&T Intellectual Property          *
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
#pragma prototyped

#include "ptvlib.h"

/*
 * return the maximum prefix of a limited to m bits
 */

unsigned char*
ptvmax(int z, unsigned char* r, const unsigned char* a, int m)
{
	int		i;
	int		n;

	if (m)
	{
		fvcpy(z, r, a);
		m = z * 8 - m;
		n = m / 8;
		m -= n * 8;
		if (!m)
			n++;
		for (i = 1; i < n; i++)
			r[z - i] = 0xFF;
		if (m)
			r[z - n] |= ((1<<m) - 1);
	}
	else
		fvset(z, r, 0xFF);
	return r;
}
