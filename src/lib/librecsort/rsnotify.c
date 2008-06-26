/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
