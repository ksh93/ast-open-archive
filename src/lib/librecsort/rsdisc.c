/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#include	"rshdr.h"

/*	Change discipline for a context
**
**	Written by Kiem-Phong Vo (07/29/96).
*/

#if __STD_C
Rsdisc_t* rsdisc(Rs_t* rs, Rsdisc_t* disc)
#else
Rsdisc_t* rsdisc(rs, disc)
Rs_t*		rs;
Rsdisc_t*	disc;
#endif
{
	reg Rsdisc_t*	old = rs->disc;

	if(disc)
	{	if(old && old->eventf &&
		   (*old->eventf)(rs,RS_DISC,(Void_t*)disc,old) < 0)
			return NIL(Rsdisc_t*);

		rs->type &= ~(RS_DSAMELEN|RS_KSAMELEN);
		rs->type |= disc->type&(RS_DSAMELEN|RS_KSAMELEN);

		if((disc->type&RS_DSAMELEN) && !disc->defkeyf)
			rs->type |= RS_KSAMELEN;

		rs->disc = disc;
	}

	return old;
}
