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
 * return an inverted copy of table a
 */

Pt_t*
ptinvert(Pt_t* a)
{
	Pt_t*		t;
	Ptprefix_t*	ap;
	Ptaddr_t	m;

	if (t = ptopen(a->disc))
	{
		m = 0;
		for (ap = (Ptprefix_t*)dtfirst(a->dict); ap; ap = (Ptprefix_t*)dtnext(a->dict, ap))
		{
			if (m < ap->min && ptinsert(t, m, ap->min - 1))
				break;
			m = ap->max + 1;
		}
		if (m || !dtsize(a->dict))
			ptinsert(t, m, ~((Ptaddr_t)0));
	}
	return t;
}
