/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2002 AT&T Corp.                *
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
#define _SFIO_H_ONLY	1
#include	"sftest.h"

/*	This test causes mmap() to fail so that read() must be used.
	On a system such as BSDI, malloc uses mmap() so if mmap()
	fails, not much else will work. In such a case, we make this
	test automatically success.
*/

static int	Success = 1;

#if __STD_C
void* mmap(void* addr, size_t size, int x, int y, int z, Sfoff_t offset)
#else
void* mmap()
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
