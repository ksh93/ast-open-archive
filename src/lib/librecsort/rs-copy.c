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
/*
 * copy input records in original order
 * presumably for the benefit of discipline
 * function alterations
 */

#include "rshdr.h"

typedef struct Copy_s
{	
	Rsobj_t*	head;
	Rsobj_t*	tail;
} Copy_t;

#if __STD_C
static int copyinsert(Rs_t* rs, reg Rsobj_t* obj)
#else
static int copyinsert(rs, obj)
Rs_t*		rs;
reg Rsobj_t*	obj;
#endif
{
	reg Copy_t*	copy = (Copy_t*)rs->methdata;

	if (copy->tail)
		copy->tail->right = obj;
	else
		copy->head = obj;
	copy->tail = obj;
	return 0;
}

#if __STD_C
static Rsobj_t* copylist(Rs_t* rs)
#else
static Rsobj_t* copylist(rs)
Rs_t*	rs;
#endif
{
	reg Copy_t*	copy = (Copy_t*)rs->methdata;

	if (copy->tail)
	{
		copy->tail->right = 0;
		copy->tail = 0;
	}
	return copy->head;
}

static Rsmethod_t _Rscopy =
{	
	copyinsert,
	copylist,
	sizeof(Copy_t),
	RS_MTCOPY,
	"copy",
	"Copy (no sort)."
};

__DEFINE__(Rsmethod_t*, Rscopy, &_Rscopy);

#ifdef NoF
NoF(rscopy)
#endif
