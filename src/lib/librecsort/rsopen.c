/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"rshdr.h"

/*	Opening sorting contexts
**
**	Written by Kiem-Phong Vo (07/08/96)
*/

static const char id[] = "\n@(#)$Id: recsort library (AT&T Research) 2000-03-17 $\0\n";

#if __STD_C
Rs_t* rsopen(Rsdisc_t* disc, Rsmethod_t* meth, ssize_t c_max, int type)
#else
Rs_t* rsopen(disc, meth, c_max, type)
Rsdisc_t*	disc;	/* discipline describing record types	*/
Rsmethod_t*	meth;	/* sorting method			*/
ssize_t		c_max;	/* process about this much per chain	*/
int		type;	/* sort controls			*/
#endif
{
	reg Rs_t*	rs;
	ssize_t		round;
	static Vmdisc_t	vmdisc;

	if(!vmdisc.memoryf)
	{	vmdisc.memoryf = Vmdcheap->memoryf;
		vmdisc.exceptf = Vmdcheap->exceptf;
	}

	if(!(rs = (Rs_t*)vmresize(Vmheap,NIL(Void_t*),sizeof(Rs_t),VM_RSZERO)) )
		return NIL(Rs_t*);

	if((round = c_max) > 0)
		round /= 4;
	vmdisc.round = round <= 0 ? RS_RESERVE : round;
	if(!(rs->vm = (Vmalloc_t*)vmopen(&vmdisc, Vmbest, VM_TRUST)) )
	{	vmfree(Vmheap,(void*)rs);
		return NIL(Rs_t*);
	}

	if(!(rs->methdata = (Void_t*)vmresize(Vmheap,NIL(Void_t*),meth->size,VM_RSZERO)) )
	{	vmclose(rs->vm);
		vmfree(Vmheap,rs);
		return NIL(Rs_t*);
	}
	else	rs->meth = meth;

	rs->c_max = c_max;
	rs->type = type;

	rsdisc(rs,disc);

	return rs;
}
