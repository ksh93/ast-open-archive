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
