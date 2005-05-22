/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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

/*	Opening sorting contexts
**
**	Written by Kiem-Phong Vo (07/08/96)
*/

static const char id[] = "\n@(#)$Id: recsort library (AT&T Research) 2005-05-15 $\0\n";

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
		goto bad;

	rs->meth = meth;
	rs->c_max = c_max;
	rs->type = type;

	rsdisc(rs,disc,RS_DISC);

	if (RSNOTIFY(rs, RS_OPEN, 0, 0, rs->disc) < 0)
		goto bad;

	return rs;
 bad:
	vmclose(rs->vm);
	vmfree(Vmheap,rs);
	return NIL(Rs_t*);
}
