/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
	if ((rv = RSNOTIFY(rs,RS_CLOSE,0,rs->disc)) < 0)
		return rv;

	if(rs->vm)
		vmclose(rs->vm);
	if(rs->methdata)
		vmfree(Vmheap,rs->methdata);
	vmfree(Vmheap,rs);

	return 0;
}
