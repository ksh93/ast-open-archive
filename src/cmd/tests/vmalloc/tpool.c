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

int	Release = 0;

#if __STD_C
Void_t*	memory(Vmalloc_t* vm, Void_t* caddr,
		size_t oldsize, size_t newsize, Vmdisc_t* disc)
#else
Void_t*	memory(vm, caddr, oldsize, newsize, disc)
Vmalloc_t*	vm;
Void_t*		caddr;
size_t		oldsize;
size_t		newsize;
Vmdisc_t*	disc;
#endif
{
	if(caddr)
	{	if(newsize != 0)
			return NIL(Void_t*);
		Release += 1;
		free(caddr);
		return caddr;
	}
	return vmalloc(Vmheap,newsize);
}

Vmdisc_t	Disc = {memory, NIL(Vmexcept_f), 64};

main()
{
	Void_t*		addr[10];
	Vmalloc_t*	vm;
	int		i;

	if(!(vm = vmopen(&Disc,Vmpool,0)) )
		terror("Can't open");

	for(i = 0; i < 10; ++i)
	{	addr[i] = vmalloc(vm,15);
		if((((Vmulong_t)addr[i])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	for(i = 0; i < 10; ++i)
		if(vmfree(vm,addr[i]) < 0)
			terror("can't free an element?");

	for(i = 0; i < 10; ++i)
		addr[i] = vmalloc(vm,15);

	if(vmresize(vm,addr[0],16,1) != NIL(Void_t*))
		terror("Resize to a different size succeeds?");

	vmfree(vm,addr[0]);
	if(vmsize(vm,addr[0]) >= 0)
		terror("Wrong size for free pool element?");

	if(vmsize(vm,addr[9]) != 15)
		terror("Wrong size for pool element?");

#ifdef DEBUG
	for(i = 0; i < 10; ++i)
		printf("size[%d]=%d\n",i,vmsize(vm,addr[i]));
	printf("vmextent=%d\n",vmsize(vm,NIL(Void_t*)));
#endif

	return 0;
}
