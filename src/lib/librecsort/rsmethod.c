/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2004 AT&T Corp.                *
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
