/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2000-2006 AT&T Corp.                  *
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

#include "ptlib.h"

/*
 * return non-zero if table a is a subset of table b
 */

int
ptsubset(Pt_t* a, Pt_t* b)
{
	Ptprefix_t*	ap;
	Ptprefix_t*	bp;

	ap = (Ptprefix_t*)dtfirst(a->dict);
	bp = (Ptprefix_t*)dtfirst(b->dict);
	while (ap)
	{
		if (!bp || ap->max < bp->min || ap->min < bp->min)
			return 0;
		if (ap->max < bp->max)
			ap = (Ptprefix_t*)dtnext(a->dict, ap);
		else if (ap->max > bp->max)
			bp = (Ptprefix_t*)dtnext(b->dict, bp);
		else
		{
			ap = (Ptprefix_t*)dtnext(a->dict, ap);
			bp = (Ptprefix_t*)dtnext(b->dict, bp);
		}
	}
	return 1;
}
