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

	if(!(f = sfopen((Sfio_t*)0,sftfile(0),"w")))
		terror("Opening to write\n");
	if(sfputc(f,'a') != 'a')
		terror("sfputc\n");
	if(sfgetc(f) >= 0)
		terror("sfgetc\n");
	
	if(!(f = sfopen(f,sftfile(0),"r")))
		terror("Opening to read\n");
	if(sfgetc(f) != 'a')
		terror("sfgetc2\n");
	if(sfputc(f,'b') >= 0)
		terror("sfputc2\n");

	if(!(f = sfopen(f,sftfile(0),"r+")))
		terror("Opening to read/write\n");

	if(sfgetc(f) != 'a')
		terror("sfgetc3\n");
	if(sfputc(f,'b') != 'b')
		terror("sfputc3\n");
	if(sfclose(f) < 0)
		terror("sfclose\n");

	if(!(f = sfpopen(NIL(Sfio_t*),sfprints("cat %s",sftfile(0)),"r")))
		terror("sfpopen\n");
	if(sfgetc(f) != 'a')
		terror("sfgetc4\n");
	if(sfgetc(f) != 'b')
		terror("sfgetc5\n");
	if(sfgetc(f) >= 0)
		terror("sfgetc6\n");

	if(!(f = sfopen(f,sftfile(0),"w")) )
		terror("sfopen\n");
	if(sfputc(f,'a') != 'a')
		terror("sfputc1\n");
	sfsetfd(f,-1);
	if(sfputc(f,'b') >= 0)
		terror("sfputc2\n");
	if(sfclose(f) < 0)
		terror("sfclose\n");

	if(!(f = sfopen(NIL(Sfio_t*),sftfile(0),"a+")) )
		terror("sfopen2\n");
	sfset(f,SF_READ,0);
	if(!sfreserve(f,0,-1) )
		terror("Failed on buffer getting\n");
	if(sfvalue(f) <= 0)
		terror("There is no buffer?\n");

	sftcleanup();
	return 0;
}
