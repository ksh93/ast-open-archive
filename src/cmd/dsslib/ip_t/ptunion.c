/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2009 AT&T Intellectual Property          *
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

#include "ptlib.h"

/*
 * return the table union of a and b
 */

Pt_t*
ptunion(Pt_t* a, Pt_t* b)
{
	Ptprefix_t*	bp;

	if (!(a = ptcopy(a)))
		return 0;
	for (bp = (Ptprefix_t*)dtfirst(b->dict); bp; bp = (Ptprefix_t*)dtnext(b->dict, bp))
		if (!ptinsert(a, bp->min, bp->max))
			break;
	return a;
}
