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

#define NUM	10
#define INIT	8

main()
{
	Void_t*		addr[10];
	Void_t*		rv;
	Vmalloc_t*	vm;
	int		i;

	Vmdcheap->round = 127;
	if(!(vm = vmopen(Vmdcheap,Vmlast,0)) )
		terror("Can't open");

	for(i = 0; i < NUM; ++i)
	{	addr[i] = vmalloc(vm,INIT);
		if((((Vmulong_t)addr[i])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	if(vmfree(vm,addr[0]) >= 0)
		terror("Free non-last element succeeds?");

	if((rv = vmresize(vm,addr[1],16*INIT,1)) == NIL(Void_t*))
		terror("Resize non-last element fails?");
	if(rv == addr[1])
		terror("Resized non-last element has same address?");

	if(vmresize(vm,addr[NUM-1],16*INIT,1) == NIL(Void_t*))
		terror("Resize last element does not succeed?");

#ifdef DEBUG
	for(i = 0; i < NUM; ++i)
		printf("size[%d]=%d\n",i,vmsize(vm,addr[i]));
	printf("vmextent=%d\n",vmsize(vm,NIL(Void_t*)));
#endif

	return 0;
}
