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
 * install error message handler for fatal malloc exceptions
 */

#include <ast.h>
#include <error.h>
#include <vmalloc.h>

#include "FEATURE/vmalloc"

#if _std_malloc

void
memfatal(void)
{
}

#else

/*
 * print message and fail on VM_BADADDR,VM_NOMEM
 */

static int
nomalloc(Vmalloc_t* region, int type, void* obj, Vmdisc_t* disc)
{
	Vmstat_t	st;

	NoP(disc);
	switch (type)
	{
	case VM_BADADDR:
		error(ERROR_SYSTEM|3, "invalid pointer %p passed to free or realloc", obj);
		return(-1);
	case VM_NOMEM:
		vmstat(region, &st);
		error(ERROR_SYSTEM|3, "storage allocator out of space on %lu byte request ( region %lu segments %lu busy %lu:%lu:%lu free %lu:%lu:%lu )", (size_t)obj, st.extent, st.n_seg, st.n_busy, st.s_busy, st.m_busy, st.n_free, st.s_free, st.m_free);
		return(-1);
	}
	return(0);
}

/*
 * initialize the malloc exception handler
 */

void
memfatal(void)
{
	Vmdisc_t*	disc;

	if (disc = vmdisc(Vmheap, NiL))
		disc->exceptf = nomalloc;
}

#endif
