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

/*	Change discipline for a context
**
**	Written by Kiem-Phong Vo (07/29/96).
*/

#if __STD_C
Rsdisc_t* rsdisc(Rs_t* rs, Rsdisc_t* disc, int op)
#else
Rsdisc_t* rsdisc(rs, disc, op)
Rs_t*		rs;
Rsdisc_t*	disc;
int		op;
#endif
{
	reg Rsdisc_t*	old;
	reg Rsdisc_t*	cur;
	reg Rsdisc_t*	prv;
	reg Rsdisc_t*	top;

	switch (op)
	{
	case RS_DISC:
		old = rs->disc;
		if(disc)
		{	if(old && (old->events & RS_DISC) &&
			   (*old->eventf)(rs,RS_DISC,(Void_t*)disc,(Void_t*)0,old) < 0)
				return NIL(Rsdisc_t*);

			rs->type &= ~(RS_DSAMELEN|RS_KSAMELEN);
			rs->type |= disc->type&(RS_DSAMELEN|RS_KSAMELEN);

			if((disc->type&RS_DSAMELEN) && !disc->defkeyf)
				rs->type |= RS_KSAMELEN;

			rs->disc = disc;
			rs->events = rs->disc->events;
			for (cur = rs->disc; cur; cur = cur->disc)
				rs->events |= cur->events;
		}
		return old;
	case RS_NEXT:
		cur = rs->disc;
		if (disc)
			while (top = cur)
			{
				cur = cur->disc;
				if (disc == top)
					break;
			}
		return cur;
	case RS_POP:
		prv = 0;
		cur = rs->disc;
		if (disc)
			while (cur && cur != disc)
				cur = (prv = cur)->disc;
		if (cur)
		{
			disc = cur;
			if (prv)
				prv->disc = cur->disc;
			else
				rs->disc = cur->disc;
			if ((disc->events & RS_POP) &&
			    (*disc->eventf)(rs, RS_POP, (Void_t*)0, (Void_t*)0, disc) < 0)
				return 0;
			if (rs->disc)
			{
				rs->events = rs->disc->events;
				for (cur = rs->disc; cur; cur = cur->disc)
					rs->events |= cur->events;
			}
		}
		else
			disc = 0;
		return disc;
	case RS_PUSH:
		if (!disc)
			return 0;
		for (prv = 0, cur = rs->disc; cur; cur = (prv = cur)->disc)
			if (cur == disc)
			{
				if (prv)
				{
					/*
					 * move to front
					 */

					prv->disc = cur->disc;
					cur->disc = rs->disc;
					rs->disc = cur;
				}
				return disc;
			}
		disc->disc = rs->disc;
		rs->disc = disc;
		rs->events |= disc->events;
		return disc;
	}
	return 0;
}
