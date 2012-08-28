/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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

#include "msglib.h"

#include <ctype.h>

/*
 * convert msg name list s to mask
 * *s=='!' inverts the mask
 */

unsigned long
msgsetmask(register const char* s)
{
	register int		c;
	register unsigned long m;
	int			invert;

	m = 0;
	invert = *s == '!';
	for (;;)
	{
		do if (!(c = *s++)) goto done; while (!isalnum(c));
		m |= MSG_MASK(msgindex(--s));
		do if (!(c = *s++)) goto done; while (isalnum(c));
	}
 done:
	return invert ? ~m : m;
}
