/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
 * return an inverted copy of table a
 */

Ptv_t*
ptvinvert(Ptv_t* a)
{
	Ptv_t*		t;
	Ptvprefix_t*	ap;

	if (t = ptvopen(a->disc, a->size))
	{
		fvset(a->size, a->r[0], 0);
		fvset(a->size, a->r[1], 1);
		for (ap = (Ptvprefix_t*)dtfirst(a->dict); ap; ap = (Ptvprefix_t*)dtnext(a->dict, ap))
		{
			if (fvcmp(a->size, a->r[0], ap->min) < 0)
			{
				fvsub(a->size, a->r[2], ap->min, a->r[1]);
				if (!ptvinsert(t, a->r[0], a->r[2]))
					break;
			}
			fvadd(a->size, a->r[0], ap->max, a->r[1]);
		}
		fvset(a->size, a->r[2], 0);
		if (fvcmp(a->size, a->r[0], a->r[1]) > 0 || !dtsize(a->dict))
		{
			fvset(a->size, a->r[2], 0);
			fvsub(a->size, a->r[3], a->r[2], a->r[1]);
			ptvinsert(t, a->r[0], a->r[3]);
		}
	}
	return t;
}
