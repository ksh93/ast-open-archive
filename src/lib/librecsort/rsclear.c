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
