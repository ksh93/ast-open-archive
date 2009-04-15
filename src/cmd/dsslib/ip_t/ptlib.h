/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2009 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * prefix table private interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _PTPREFIX_PRIVATE_

#define _PTPREFIX_PRIVATE_ \
	Dtlink_t	link;

#include <cdt.h>
#include <error.h>

#include "pt.h"

#define PTSCAN(t,x,b,m,s) \
	do								\
	{								\
		Ptprefix_t*	_pt_p;					\
		Ptaddr_t	_pt_min;				\
		Ptaddr_t	_pt_max;				\
		Ptaddr_t	m;					\
		Ptaddr_t	x;					\
		unsigned int	b;					\
		for (_pt_p = (Ptprefix_t*)dtfirst((t)->dict); _pt_p; _pt_p = (Ptprefix_t*)dtnext((t)->dict, _pt_p))				\
		{							\
			_pt_min = _pt_p->min;				\
			_pt_max = _pt_p->max;				\
			do						\
			{						\
				x = _pt_min;				\
				m = 1;					\
				b = PTBITS;				\
				while (m &&				\
				       !(x & ((1<<PTSHIFT)-1)) &&	\
				       (_pt_min|((m<<PTSHIFT)-1)) <= _pt_max) \
				{					\
					x >>= PTSHIFT;			\
					m <<= PTSHIFT;			\
					b -= PTSHIFT;			\
				}					\
				x = _pt_min;				\
				s;					\
				if (!b || (_pt_min += m) < x)		\
					break;				\
			} while (_pt_min <= _pt_max);			\
		}							\
	} while (0)

#endif
