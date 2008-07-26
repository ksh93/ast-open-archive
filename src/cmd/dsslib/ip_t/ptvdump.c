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
 * dump table a on sp
 */

int
ptvdump(Ptv_t* a, Sfio_t* sp)
{
	Ptvprefix_t*	p;

	for (p = (Ptvprefix_t*)dtfirst(a->dict); p; p = (Ptvprefix_t*)dtnext(a->dict, p))
		sfprintf(sp, "%s %s %-24s %-24s\n", fmtfv(a->size, p->min, 16, 0, 0), fmtfv(a->size, p->max, 16, 0, 0), fmtip6(p->min, -1), fmtip6(p->max, -1));
	return sfsync(sp);
}
