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
#define sbrk	____sbrk____
#include	"sftest.h"
#undef	sbrk

_BEGIN_EXTERNS_
extern void*	sbrk _ARG_((int));
_END_EXTERNS_

#if _PACKAGE_ast
#undef	_Sfpage
#define _Sfpage	PAGE_SIZE
#endif

main()
{
	char	*s = "1234567890\n";
	Sfoff_t	n, i;
	Sfio_t	*f;
	char	buf[1024];
	char*	addr;

	if(sfopen(sfstdout,sftfile(0),"w+") != sfstdout)
		terror("Opening output file\n");
	for(i = 0; i < 10000; ++i)
		if(sfputr(sfstdout,s,-1) < 0)
			terror("Writing data\n");

	if(!(f = sfopen((Sfio_t*)0,sftfile(1),"w")))
		terror("Opening output file \n");

	sfseek(sfstdout,(Sfoff_t)0,0);
	if((n = sfmove(sfstdout,f,(Sfoff_t)SF_UNBOUND,'\n')) != i)
		terror("Move %d lines, Expect %d\n",n,i);

	sfseek(sfstdout,(Sfoff_t)0,0);
	sfseek(f,(Sfoff_t)0,0);
	sfsetbuf(sfstdout,buf,sizeof(buf));
	if((n = sfmove(sfstdout,f,(Sfoff_t)SF_UNBOUND,'\n')) != i)
		terror("Move %d lines, Expect %d\n",n,i);

	sfopen(sfstdin,sftfile(0),"r");
	sfopen(sfstdout,sftfile(1),"w");
	sfmove(sfstdin,sfstdout,(Sfoff_t)SF_UNBOUND,-1);
	if(!sfeof(sfstdin))
		terror("Sfstdin is not eof\n");
	if(sferror(sfstdin))
		terror("Sfstdin is in error\n");
	if(sferror(sfstdout))
		terror("Sfstdout is in error\n");

	sfseek(sfstdin,(Sfoff_t)0,0);
	sfseek(sfstdout,(Sfoff_t)0,0);
	sfsetbuf(sfstdin,buf,sizeof(buf));

	addr = (char*)sbrk(0);
	sfmove(sfstdin,sfstdout,(Sfoff_t)((unsigned long)(~0L)>>1),-1);
	if((ssize_t)((char*)sbrk(0)-addr) > 16*(_Sfpage > 0 ? _Sfpage : SF_PAGE) )
		terror("Too much space allocated in sfmove\n");

	if(!sfeof(sfstdin))
		terror("Sfstdin is not eof2\n");
	if(sferror(sfstdin))
		terror("Sfstdin is in error2\n");
	if(sferror(sfstdout))
		terror("Sfstdout is in error2\n");

	sftcleanup();
	return 0;
}
