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
	int		i;

	Vmdcheap->round = 64;
	if(!(vm = vmopen(Vmdcheap, Vmbest, 0)) )
		terror("Open failed");

	for(i = 0; i < 10; ++i)
		if(!(addr[i] = vmalloc(vm,15)) )
			terror("vmalloc failed");
	for(i = 0; i < 10; ++i)
		if(vmfree(vm,addr[i]) < 0)
			terror("vmfree failed");
	for(i = 0; i < 10; ++i)
		if(!(addr[i] = vmalloc(vm,15)) )
			terror("vmalloc failed");
#ifdef DEBUG
	for(i = 0; i < 10; ++i)
		printf("size[%d]=%d\n",i,vmsize(vm,addr[i]));
	printf("Extent=%d\n",vmsize(vm,NIL(Void_t*)));
#endif

	return 0;
}
