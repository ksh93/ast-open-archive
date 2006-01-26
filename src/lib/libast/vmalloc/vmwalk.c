/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_vmwalk(){}

#else

#include	"vmhdr.h"

/*	Walks all segments created in region(s)
**
**	Written by Kiem-Phong Vo, kpv@research.att.com (02/08/96)
*/

#if __STD_C
int vmwalk(Vmalloc_t* vm, int(*segf)(Vmalloc_t*, Void_t*, size_t, Vmdisc_t*) )
#else
int vmwalk(vm, segf)
Vmalloc_t*	vm;
int(*		segf)(/* Vmalloc_t*, Void_t*, size_t, Vmdisc_t* */);
#endif
{	
	reg Seg_t*	seg;
	reg int		rv;

	if(!vm)
	{	for(vm = Vmheap; vm; vm = vm->next)
		{	if(!(vm->data->mode&VM_TRUST) && ISLOCK(vm->data,0) )
				continue;

			SETLOCK(vm->data,0);
			for(seg = vm->data->seg; seg; seg = seg->next)
			{	rv = (*segf)(vm, seg->addr, seg->extent, vm->disc);
				if(rv < 0)
					return rv;
			}
			CLRLOCK(vm->data,0);
		}
	}
	else
	{	if(!(vm->data->mode&VM_TRUST) && ISLOCK(vm->data,0) )
			return -1;

		SETLOCK(vm->data,0);
		for(seg = vm->data->seg; seg; seg = seg->next)
		{	rv = (*segf)(vm, seg->addr, seg->extent, vm->disc);
			if(rv < 0)
				return rv;
		}
		CLRLOCK(vm->data,0);
	}

	return 0;
}

#endif
