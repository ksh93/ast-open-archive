/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library
 */

#include "exlib.h"

/*
 * copy input token error context into buf of n chars and reset the context
 * end of buf returned
 */

char*
excontext(Expr_t* p, char* buf, int n)
{
	register char*	s;
	register char*	t;
	register char*	e;

	s = buf;
	if (p->linep > p->line || p->linewrap)
	{
		e = buf + n - 5;
		if (p->linewrap)
		{
			t = p->linep + 1;
			while (t < &p->line[sizeof(p->line)] && isspace(*t))
				t++;
			if ((n = (sizeof(p->line) - (t - (p->linep + 1))) - (e - s)) > 0)
			{
				if (n > &p->line[sizeof(p->line)] - t)
					t = &p->line[sizeof(p->line)];
				else t += n;
			}
			while (t < &p->line[sizeof(p->line)])
				*s++ = *t++;
		}
		t = p->line;
		if (p->linewrap)
			p->linewrap = 0;
		else while (t < p->linep && isspace(*t))
			t++;
		if ((n = (p->linep - t) - (e - s)) > 0)
			t += n;
		while (t < p->linep)
			*s++ = *t++;
		p->linep = p->line;
		t = "<<< ";
		while (*s = *t++)
			s++;
	}
	*s = 0;
	return s;
}
