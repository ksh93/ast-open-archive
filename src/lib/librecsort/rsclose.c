/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2004 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"rshdr.h"

/*	Close a sorting context
**
**	Written by Kiem-Phong Vo (07/18/96)
*/

#if __STD_C
int rsclose(Rs_t* rs)
#else
int rsclose(rs)
Rs_t*	rs;
#endif
{
	reg int	rv;

	while (rsdisc(rs, NIL(Rsdisc_t*), RS_POP));
	if ((rv = RSNOTIFY(rs,RS_CLOSE,0,0,rs->disc)) < 0)
		return rv;

	if(rs->vm)
		vmclose(rs->vm);
	if(rs->methdata)
		vmfree(Vmheap,rs->methdata);
	vmfree(Vmheap,rs);

	return 0;
}
