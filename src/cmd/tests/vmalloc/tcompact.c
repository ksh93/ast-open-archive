/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
		vmfree(Vmheap,caddr);
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

	if(!(vm = vmopen(&Disc,Vmbest,0)) )
		terror("Can't open");

	for(i = 0; i < 10; ++i)
	{	addr[i] = vmalloc(vm,15);
		if((((Vmulong_t)addr[i])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	for(i = 0; i < 10; ++i)
		if(vmfree(vm,addr[i]) < 0)
			terror("can't free an element?");

	if(!(addr[0] = vmalloc(vm,30)) )
		terror("Can't alloc");

	for(i = 0; i < 10; ++i)
		addr[i] = vmalloc(vm,15);

	if(vmresize(vm,addr[0],16,1) != addr[0])
		terror("Location should not have changed");

#ifdef DEBUG
	for(i = 0; i < 10; ++i)
		printf("size[%d]=%d\n",i,vmsize(vm,addr[i]));
	printf("vmextent=%d\n",vmsize(vm,NIL(Void_t*)));
#endif

	return 0;
}
