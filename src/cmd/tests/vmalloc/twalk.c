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

static Vmuchar_t	Buf[4*1024];
static Vmuchar_t*	Avail = Buf;
static int		Count = 0;
static int		Walk = 0;

#if __STD_C
static Void_t*	memory(Vmalloc_t* vm, Void_t* caddr,
		size_t oldsize, size_t newsize, Vmdisc_t* disc)
#else
static Void_t*	memory(vm, caddr, oldsize, newsize, disc)
Vmalloc_t*	vm;
Void_t*		caddr;
size_t		oldsize;
size_t		newsize;
Vmdisc_t*	disc;
#endif
{
	if(oldsize)
		return NIL(Void_t*);

	Count += 1;
	caddr = (Void_t*)Avail;
	Avail += newsize;
	if(Avail >= Buf+sizeof(Buf))
		terror("No more buffer");

	return caddr;
}

static Vmdisc_t	Disc = {memory, NIL(Vmexcept_f), 64};

#if __STD_C
static walk(Vmalloc_t* vm, Void_t* addr, size_t size, Vmdisc_t* disc)
#else
static walk(vm, addr, size, disc)
Vmalloc_t*	vm;
Void_t*		addr;
size_t		size;
Vmdisc_t*	disc;
#endif
{
	if(disc == &Disc)
		Walk += 1;
	return 0;
}

main()
{
	reg Vmalloc_t*	vm1 = vmopen(&Disc,Vmbest,0);
	reg Vmalloc_t*	vm2 = vmopen(&Disc,Vmbest,0);
	reg Vmalloc_t*	vm3 = vmopen(&Disc,Vmbest,0);
	reg Vmalloc_t*	vm4 = vmopen(&Disc,Vmbest,0);
	reg Void_t	*m1, *m2, *m3, *m4;

	m1 = vmalloc(vm1,258);
	m2 = vmalloc(vm2,258);
	m3 = vmalloc(vm3,258);
	m4 = vmalloc(vm4,258);

	if(Count != 8)
		terror("Wrong count\n");

	vmwalk(NIL(Vmalloc_t*),walk);
	if(Walk != 8)
		terror("Wrong walk count\n");

	vmfree(vm1,m1);
	vmfree(vm2,m2);
	vmfree(vm3,m3);
	vmfree(vm4,m4);

	return 0;
}