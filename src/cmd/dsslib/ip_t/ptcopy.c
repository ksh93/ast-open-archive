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
 * return a copy of table a
 */

Pt_t*
ptcopy(Pt_t* a)
{
	Pt_t*		b;
	Ptprefix_t*	ap;

	if (b = ptopen(a->disc))
	{
		b->entries = a->entries;
		for (ap = (Ptprefix_t*)dtfirst(a->dict); ap; ap = (Ptprefix_t*)dtnext(a->dict, ap))
			dtinsert(b->dict, ap);
	}
	return b;
}
