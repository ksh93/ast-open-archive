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

#include "ptlib.h"

/*
 * return the number of addresses covered by a
 */

Ptcount_t
ptaddresses(Pt_t* a)
{
	Ptcount_t	n;
	Ptprefix_t*	ap;

	n = 0;
	for (ap = (Ptprefix_t*)dtfirst(a->dict); ap; ap = (Ptprefix_t*)dtnext(a->dict, ap))
		n += (Ptcount_t)(ap->max - ap->min) + 1;
	return n;
}
