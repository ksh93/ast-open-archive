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
