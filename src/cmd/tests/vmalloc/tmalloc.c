/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
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
