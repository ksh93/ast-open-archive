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
 * return the number of addresses covered by a
 */

int
ptvaddresses(Ptv_t* a, unsigned char* r)
{
	Ptvprefix_t*	ap;

	fvset(a->size, r, 0);
	fvset(a->size, a->r[1], 1);
	for (ap = (Ptvprefix_t*)dtfirst(a->dict); ap; ap = (Ptvprefix_t*)dtnext(a->dict, ap))
	{
		fvsub(a->size, a->r[0], ap->max, ap->min);
		fvadd(a->size, a->r[0], a->r[0], a->r[1]);
		fvadd(a->size, r, r, a->r[0]);
	}
	return 0;
}