/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
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
