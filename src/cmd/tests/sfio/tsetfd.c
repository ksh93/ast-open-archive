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
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"sftest.h"

MAIN()
{
	Sfio_t	*f;
	int	fd;
	off_t	sk;

	if(!(f = sfopen((Sfio_t*)0,tstfile(0),"w+")))
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
	if((sk = lseek(fd, (off_t)0, 1)) != (off_t)1)
		terror("Bad seek address %lld\n", (Sfoff_t)sk );
	if(sfgetc(f) >= 0)
		terror("sfgetc2\n");

	if(sfsetfd(f,fd) != fd)
		terror("setfd2\n");
	if(sfgetc(f) != '2')
		terror("sfgetc3\n");

	TSTEXIT(0);
}
