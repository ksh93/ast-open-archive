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
