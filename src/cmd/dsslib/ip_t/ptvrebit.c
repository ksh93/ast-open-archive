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
 * return a copy of table a with prefixes limited to a maximum of m bits
 */

Ptv_t*
ptvrebit(Ptv_t* a, int m)
{
	Ptv_t*		b;
	Ptvprefix_t*	ap;

	if (b = ptvopen(a->disc, a->size))
		for (ap = (Ptvprefix_t*)dtfirst(a->dict); ap; ap = (Ptvprefix_t*)dtnext(a->dict, ap))
			if (ptvinsert(b, ptvmin(a->size, b->r[0], ap->min, m), ptvmin(a->size, b->r[1], ap->max, m)))
			{
				ptvclose(b);
				return 0;
			}
	return b;
}