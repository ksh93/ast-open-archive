/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"sftest.h"

MAIN()
{
	Sfio_t	*f;
	int	fd;
	off_t	sk;

	if(!(f = sfopen((Sfio_t*)0,tstfile(0),"w+")))
		terror("Opening file");
	fd = sffileno(f);

	if(sfsetfd(f,-1) != -1 || sffileno(f) != -1)
		terror("setfd1");
	if(sfputc(f,'a') >= 0)
		terror("sfputc");

	if(sfsetfd(f,fd) != fd)
		terror("setfd2");

	if(sfwrite(f,"123456789\n",10) != 10)
		terror("sfwrite");

	sfseek(f,(Sfoff_t)0,0);
	if(sfgetc(f) != '1')
		terror("sfgetc1");

	if(sfsetfd(f,-1) != -1 || sffileno(f) != -1)
		terror("setfd2");
	if((sk = lseek(fd, (off_t)0, 1)) != (off_t)1)
		terror("Bad seek address %lld", (Sfoff_t)sk );
	if(sfgetc(f) >= 0)
		terror("sfgetc2");

	if(sfsetfd(f,fd) != fd)
		terror("setfd2");
	if(sfgetc(f) != '2')
		terror("sfgetc3");

	TSTEXIT(0);
}
