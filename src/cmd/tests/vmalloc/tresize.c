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

/* This tests the sequential placement of blocks by Vmbest
** and its effect on vmresize(). This test should be looked
** at every time the allocation algorithm in Vmbest changes.
*/

MAIN()
{
	Vmalloc_t	*vm;
	Void_t		*dt[10];
	int		i;

	if(!(vm = vmopen(Vmdcsbrk, Vmbest, 0)) )
		terror("Couldn't open region");

	if(!(dt[0] = vmalloc(vm, 32)) )
		terror("vmalloc failed1");
	if(!(dt[1] = vmalloc(vm, 32)) )
		terror("vmalloc failed2");
	if(!(dt[2] = vmalloc(vm, 32)) )
		terror("vmalloc failed3");
	vmfree(vm,dt[1]);
	vmfree(vm,dt[2]);
	if(vmresize(vm, dt[0], 60, 1) != dt[0])
		terror("bestresize did not extend correctly");
	if(vmresize(vm, dt[0], 32, 1) != dt[0])
		terror("bestresize did not reduce correctly2");

	if(!(dt[1] = vmalloc(vm, 16)) )
		terror("vmalloc failed4");
	if(!(dt[2] = vmalloc(vm, 16)) )
		terror("vmalloc failed5");
	if(!(dt[3] = vmalloc(vm, 24)) )
		terror("vmalloc failed6");
	if(!(dt[4] = vmalloc(vm, 32)) )
		terror("vmalloc failed7");
	if(!(dt[5] = vmalloc(vm, 40)) )
		terror("vmalloc failed8");
	vmfree(vm,dt[2]);
	vmfree(vm,dt[3]);
	vmfree(vm,dt[4]);
	vmfree(vm,dt[5]);
	if(vmresize(vm,dt[1],80,0) != dt[1])
		terror("vmresize failed");
	vmfree(vm,dt[1]);

	for(i = 6; i < 10; ++i)
		vmfree(vm, dt[i]);
	if(vmresize(vm, dt[0], 120, 1) != dt[0])
		terror("vmresize did not extend correctly3");

	exit(0);
}
