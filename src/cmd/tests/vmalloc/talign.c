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

static char Mem[13*1024];

#if __STD_C
static Void_t* alignmem(Vmalloc_t* vm, Void_t* ca, size_t cs, size_t ns, Vmdisc_t* dc)
#else
static Void_t* alignmem(vm, ca, cs, ns, dc)
Vmalloc_t*	vm;
Void_t*		ca;
size_t		cs;
size_t		ns;
Vmdisc_t*	dc;
#endif
{
	return (Void_t*)(Mem+13);
}

static Vmdisc_t Disc = { alignmem, NIL(Vmexcept_f), 10*1024};

MAIN()
{
	Vmalloc_t*	vm;
	Void_t*		data;
	int		i, j;

	if(!(vm = vmopen(&Disc,Vmbest,0)) )
		terror("Opening region0");
	if(!(data = vmalign(vm,10,13)) )
		terror("vmalign0");
	if(((Vmulong_t)data%13) != 0)
		terror("Not aligned0");
	vmclose(vm);

	if(!(vm = vmopen(&Disc,Vmlast,0)) )
		terror("Opening region0.1");
	if(!(data = vmalign(vm,10,13)) )
		terror("vmalign0.1");
	if(((Vmulong_t)data%13) != 0)
		terror("Not aligned0.1");

	Vmdcheap->round = ALIGN;
	if(!(vm = vmopen(Vmdcheap,Vmbest,0)) )
		terror("Opening region1");
	if(!(data = vmalloc(vm,10)) )
		terror("vmalloc1");
	if(!(data = vmalign(vm,10,13)) )
		terror("vmalign1");
	if(((Vmulong_t)data%13) != 0)
		terror("Not aligned1");
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmdebug,0)) )
		terror("Opening region");
	if(!(data = vmalloc(vm,10)) )
		terror("vmalloc3");
	if(!(data = vmalign(vm,10,13)) )
		terror("vmalign3");
	if(((Vmulong_t)data%13) != 0)
		terror("Not aligned3");
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmbest,0)) )
		terror("Opening region2");
	for(i = 0; i < 100; ++i)
	{	for(j = 0; j < 10; ++j)
			if(!(data = vmalloc(vm,17)) )
				terror("vmalloc failed");
		if(!(data = vmalign(vm,64,4096)) )
			terror("vmalign failed");
		if(((Vmulong_t)data % 4096) != 0)
			terror("Not aligned");
	}
	vmclose(vm);

	if(!(vm = vmopen(Vmdcheap,Vmdebug,0)) )
		terror("Opening region2");
	for(i = 0; i < 100; ++i)
	{	for(j = 0; j < 10; ++j)
			if(!(data = vmalloc(vm,17)) )
				terror("vmalloc failed");
		if(!(data = vmalign(vm,64,4096)) )
			terror("vmalign failed");
		if(((Vmulong_t)data % 4096) != 0)
			terror("Not aligned");
	}
	vmclose(vm);

	return 0;
}
