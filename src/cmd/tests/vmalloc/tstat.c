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
	Void_t*		addr;
	Vmstat_t	st;

	if(!(vm = vmopen(Vmdcheap,Vmbest,0)) )
		terror("Can't open Vmbest region");
	if(!(addr = vmalloc(vm,123)) )
		terror("vmalloc failed1");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed11");
	if(st.n_busy != 1 || st.s_busy < 123 || st.s_busy > (123+32))
		terror("Wrong statistics1");
	if(vmfree(vm,addr) < 0)
		terror("vmfree failed1");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed12");
	if(st.n_busy != 0 || st.s_busy > 0 )
		terror("Wrong statistics12");
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmpool,0)) )
		terror("Can't open Vmpool region");
	if(!(addr = vmalloc(vm,13)) )
		terror("vmalloc failed2");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed21");
	if(st.n_busy != 1 || st.s_busy != 13 )
		terror("Wrong statistics2 (n_busy=%d s_busy=%d)", st.n_busy, st.s_busy);
	if(vmfree(vm,addr) < 0)
		terror("vmfree failed2");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed22");
	if(st.n_busy != 0 || st.s_busy > 0 )
		terror("Wrong statistics22");
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmlast,0)) )
		terror("Can't open Vmlast region");
	if(!(addr = vmalloc(vm,123)) )
		terror("vmalloc failed3");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed31");
	if(st.n_busy != 1 || st.s_busy < 123 )
		terror("Wrong statistics3");
	if(vmfree(vm,addr) < 0)
		terror("vmfree failed3");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed32");
	if(st.n_busy != 0 || st.s_busy > 0 )
		terror("Wrong statistics32");
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmdebug,0)) )
		terror("Can't open Vmdebug region");
	if(!(addr = vmalloc(vm,123)) )
		terror("vmalloc failed4");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed41");
	if(st.n_busy != 1 || st.s_busy != 123 )
		terror("Wrong statistics4");
	if(vmfree(vm,addr) < 0)
		terror("vmfree failed4");
	if(vmstat(vm,&st) < 0 )
		terror("vmstat failed42");
	if(st.n_busy != 0 || st.s_busy > 0 )
		terror("Wrong statistics42");
	vmclose(vm);

	return 0;
}
