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

/*	Discipline event notification.
*/

#if __STD_C
int rsnotify(Rs_t* rs, int op, Void_t* data, Void_t* arg, reg Rsdisc_t* disc)
#else
int rsnotify(rs, op, data, arg, disc)
Rs_t*		rs;
int		op;
Void_t*		data;
Void_t*		arg;
reg Rsdisc_t*	disc;
#endif
{
	int	r;

	r = 0;
	if (rs->events & op)
		for (; disc; disc = disc->disc)
			if ((disc->events & op) &&
			    (r = (*disc->eventf)(rs, op, data, arg, disc)))
				break;
	return r;
}
