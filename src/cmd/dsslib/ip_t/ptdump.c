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
 * dump table a on sp
 */

int
ptdump(Pt_t* a, Sfio_t* sp)
{
	Ptprefix_t*	p;

	for (p = (Ptprefix_t*)dtfirst(a->dict); p; p = (Ptprefix_t*)dtnext(a->dict, p))
		sfprintf(sp, "%0*I*x %0*I*x %-16s %-16s\n", sizeof(p->min) * 2, sizeof(p->min), p->min, sizeof(p->max) * 2, sizeof(p->max), p->max, fmtip4(p->min, -1), fmtip4(p->max, -1));
	return sfsync(sp);
}
