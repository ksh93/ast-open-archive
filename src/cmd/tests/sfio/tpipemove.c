/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2003 AT&T Corp.                *
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
#include	"sftest.h"

MAIN()
{
	int	fd[2];
	Sfio_t	*fr, *fw;
	char	*s;
	int	n, w;

	if(pipe(fd) < 0)
		terror("Can't open pipe\n");

	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open pipe streams\n");
	sfset(fr,SF_SHARE,1);

	if(sfopen(sfstdout,tstfile(0),"w") != sfstdout)
		terror("Can't open for write\n");
	if(sfopen(sfstdin,tstfile(0),"r") != sfstdin)
		terror("Can't open for read\n");

	for(n = 0; n < 100; ++n)
		if((w = sfwrite(fw,"123456789\n",10)) != 10)
			terror("Writing to pipe w=%d\n",w);

	if((n = (int)sfmove(fr,sfstdout,(Sfoff_t)100,'\n')) != 100)
		terror("sfmove failed n=%d\n", n);
	sfclose(sfstdout);

	for(n = 0; n < 100; ++n)
	{	if(!(s = sfgetr(sfstdin,'\n',1)) )
			terror("Can't read data\n");
		if(strcmp(s,"123456789") != 0)
			terror("Wrong data\n");
	}

	TSTEXIT(0);
}
