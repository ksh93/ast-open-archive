/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1999-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#include	"sftest.h"

main()
{
	Sfio_t	*f;
	int	fd;

	if(!(f = sfopen((Sfio_t*)0,sftfile(0),"w+")))
		terror("Opening file\n");
	fd = sffileno(f);

	if(sfsetfd(f,-1) != -1 || sffileno(f) != -1)
		terror("setfd1\n");
	if(sfputc(f,'a') >= 0)
		terror("sfputc\n");

	if(sfsetfd(f,fd) != fd)
		terror("setfd2\n");

	if(sfwrite(f,"123456789\n",10) != 10)
		terror("sfwrite\n");

	sfseek(f,(Sfoff_t)0,0);
	if(sfgetc(f) != '1')
		terror("sfgetc1\n");

	if(sfsetfd(f,-1) != -1 || sffileno(f) != -1)
		terror("setfd2\n");
	if(lseek(fd,0L,1) != 1L)
		terror("Bad seek address %d\n",lseek(fd,0L,1));
	if(sfgetc(f) >= 0)
		terror("sfgetc2\n");

	if(sfsetfd(f,fd) != fd)
		terror("setfd2\n");
	if(sfgetc(f) != '2')
		terror("sfgetc3\n");

	sftcleanup();
	return 0;
}
