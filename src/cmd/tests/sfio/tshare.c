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
	char		buf[1024], *s;
	Sfio_t*	f;

	f = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,-1,SF_WRITE|SF_STRING);
	sfsetbuf(sfstdout,buf,sizeof(buf));
	sfsetbuf(sfstderr,buf,sizeof(buf));
	sfset(sfstdout,SF_SHARE,0);
	sfset(sfstderr,SF_SHARE,0);

	if(!sfpool(sfstdout,f,SF_SHARE) || !sfpool(sfstderr,f,SF_SHARE) )
		terror("Pooling\n");

	if(sfputr(sfstdout,"01234",-1) != 5)
		terror("Writing to stderr\n");
	if(sfputr(sfstderr,"56789",-1) != 5)
		terror("Writing to stdout\n");

	if(sfputc(f,'\0') < 0)
		terror("Writing to string stream\n");

	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,1)) )
		terror("Peeking\n");
	sfwrite(f,s,0);
	if(strcmp(s,"0123456789") != 0)
		terror("Data is wrong\n");

	TSTEXIT(0);
}
