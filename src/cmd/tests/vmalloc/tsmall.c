/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2008 AT&T Intellectual Property          *
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
#include	"vmtest.h"

main()
{
	Vmalloc_t*	vm;
	Void_t*		addr[10];

	Vmdcheap->round = 64;
	if(!(vm = vmopen(Vmdcheap, Vmbest, 0)) )
		terror("Open failed");

	if(!(addr[0] = vmalloc(vm, 8)) )
		terror("vmalloc failed");
	if(!(addr[1] = vmalloc(vm, 8)) )
		terror("vmalloc failed");
	if(!(addr[2] = vmalloc(vm, 8)) )
		terror("vmalloc failed");

	if(!(addr[3] = vmalloc(vm, 12)) )
		terror("vmalloc failed");
	if(!(addr[4] = vmalloc(vm, 12)) )
		terror("vmalloc failed");
	if(!(addr[5] = vmalloc(vm, 12)) )
		terror("vmalloc failed");

	if(vmfree(vm, addr[1]) < 0)
		terror("vmfree failed");
	if(vmdbcheck(vm) < 0)
		terror("vmdbcheck failed");

	if(vmfree(vm, addr[4]) < 0)
		terror("vmfree failed");
	vmdbcheck(vm);

	if(vmcompact(vm) < 0)
		terror("vmcompact failed");
	if(vmdbcheck(vm) < 0)
		terror("vmdbcheck failed");

	return 0;
}
