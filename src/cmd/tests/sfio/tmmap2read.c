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
#include	"sftest.h"

#undef	off_t
#undef	mmap
#undef	mmap64

/*	This test causes mmap() to fail so that read() must be used.
	On a system such as BSDI, malloc uses mmap() so if mmap()
	fails, not much else will work. In such a case, we make this
	test automatically success.
*/

static int	Success = 1;

#if __STD_C
void* mmap(void* addr, size_t size, int x, int y, int z, off_t offset)
#else
void* mmap()
#endif
{
	if(Success)
		TSTEXIT(0);

	return (void*)(-1);
}

#if __STD_C
void* mmap64(void* addr, size_t size, int x, int y, int z, Sfoff_t offset)
#else
void* mmap64()
#endif
{
	if(Success)
		TSTEXIT(0);

	return (void*)(-1);
}

MAIN()
{
	Sfio_t*	f;
	char	buf[1024], buf2[1024], *data;
	int	n, r;

	/* test to see if malloc() winds up calling mmap() */
	if(!(data = (char*)malloc(8*1024)) )
		terror("Malloc failed\n");
	free(data);
	Success = 0;

	/* our real work */
	if(!(f = sfopen(NIL(Sfio_t*), tstfile(0),"w")) )
		terror("Can't open to write\n");

	for(n = 0; n < sizeof(buf); ++n)
		buf[n] = '0' + (n%10);

	for(n = 0; n < 10; ++n)
		sfwrite(f,buf,sizeof(buf));

	if(!(f = sfopen(f, tstfile(0),"r")) )
		terror("Can't open to read\n");

	for(n = 0; n < 10; ++n)
	{	if((r = sfread(f,buf2,sizeof(buf))) != sizeof(buf))
			terror("Bad read size=%d\n",r);
		if(strncmp(buf,buf2,sizeof(buf)) != 0)
			terror("Get wrong data\n");
	}

	TSTEXIT(0);
}
