/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * dtopen() with handle placed in specific vm region
 */

#include <dt.h>

typedef struct Dc_s
{
	Dtdisc_t	ndisc;
	Dtdisc_t*	odisc;
	Vmalloc_t*	vm;
} Dc_t;

static int
eventf(Dt_t* dt, int op, void* data, Dtdisc_t* disc)
{
	Dc_t*	dc = (Dc_t*)disc;
	int	r;

	if (dc->odisc->eventf && (r = (*dc->odisc->eventf)(dt, op, data, dc->odisc)))
		return r;
	return op == DT_ENDOPEN ? 1 : 0;
}

static void*
memoryf(Dt_t* dt, void* addr, size_t size, Dtdisc_t* disc)
{
	return vmresize(((Dc_t*)disc)->vm, addr, size, VM_RSMOVE);
}

/*
 * open a dictionary using disc->memoryf if set or vm otherwise
 */

Dt_t*
dtnew(Vmalloc_t* vm, Dtdisc_t* disc, Dtmethod_t* meth)
{
	Dt_t*		dt;
	Dc_t		dc;

	dc.odisc = disc;
	dc.ndisc = *disc;
	dc.ndisc.eventf = eventf;
	if (!dc.ndisc.memoryf)
		dc.ndisc.memoryf = memoryf;
	dc.vm = vm;
	if (dt = dtopen(&dc.ndisc, meth))
		dtdisc(dt, disc, DT_SAMECMP|DT_SAMEHASH);
	return dt;
}
