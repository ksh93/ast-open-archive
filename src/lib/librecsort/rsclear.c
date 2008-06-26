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

/*	Clear space in a region
**
**	Written by Kiem-Phong Vo (07/18/96)
*/

#if __STD_C
int rsclear(Rs_t* rs)
#else
int rsclear(rs)
Rs_t*	rs;
#endif
{
	reg uchar	*m, *endm;

	for(m = (uchar*)rs->methdata, endm = m+rs->meth->size; m < endm; ++m)
		*m = 0;

	if(rs->vm)
		vmclear(rs->vm);
	rs->c_size = 0;
	rs->type &= RS_TYPES;
	rs->free = rs->sorted = NIL(Rsobj_t*);

	return 0;
}
