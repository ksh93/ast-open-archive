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
 * return the table union of a and b
 */

Ptv_t*
ptvunion(Ptv_t* a, Ptv_t* b)
{
	Ptvprefix_t*	bp;

	if (!(a = ptvcopy(a)))
		return 0;
	for (bp = (Ptvprefix_t*)dtfirst(b->dict); bp; bp = (Ptvprefix_t*)dtnext(b->dict, bp))
		if (ptvinsert(a, bp->min, bp->max))
			break;
	return a;
}
