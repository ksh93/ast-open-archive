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

/*	Change method for a context
**
**	Written by Kiem-Phong Vo (07/29/96).
*/

#if __STD_C
Rsmethod_t* rsmethod(Rs_t* rs, Rsmethod_t* meth)
#else
Rsmethod_t* rsmethod(rs, meth)
Rs_t*		rs;
Rsmethod_t*	meth;
#endif
{
	reg Rsmethod_t*	old = rs->meth;
	reg Void_t*	methdata;

	if(!meth)
		return old;

	if (RSNOTIFY(rs,RS_METHOD,meth,0,rs->disc) < 0)
		return NIL(Rsmethod_t*);

	if(meth->size <= 0)
		methdata = NIL(Void_t*);
	else if(!(methdata = vmresize(Vmheap,NIL(Void_t*),meth->size,VM_RSZERO)) )
		return NIL(Rsmethod_t*);

	if(rs->methdata)
		vmfree(Vmheap,rs->methdata);

	rs->meth = meth;
	rs->methdata = methdata;

	return old;
}
