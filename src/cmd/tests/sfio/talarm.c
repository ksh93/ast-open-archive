/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2000 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include	"sftest.h"
#include	<signal.h>

#define HANDLER	"Handler"
char	Buf[16];
int	Except;

#if __STD_C
void alrmhandler(int sig)
#else
void alrmhandler(sig)
int	sig;
#endif
{
	strcpy(Buf,HANDLER);
	signal(sig,alrmhandler);
}

#if __STD_C
exceptf(Sfio_t* f, int type, Void_t* data, Sfdisc_t* disc)
#else
exceptf(f, type, data, disc)
Sfio_t* 	f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_ATEXIT)
		return 0;

	if(type != SF_READ)
		terror("Bad Io type\n");
	if(errno != EINTR)
		terror("Bad exception\n");
	Except = 1;

	return -1;
}

Sfdisc_t Disc = {NIL(Sfread_f), NIL(Sfwrite_f), NIL(Sfseek_f), exceptf};

MAIN()
{
	int	fd[2];

	if(pipe(fd) < 0)
		terror("Can't make pipe\n");
	if(sfnew(sfstdin,NIL(char*),(size_t)SF_UNBOUND,fd[0],SF_READ) != sfstdin)
		terror("Can't renew stdin\n");
	sfdisc(sfstdin,&Disc);
	sfset(sfstdin,SF_SHARE,1);

	signal(SIGALRM,alrmhandler);
	alarm(2);
	if(sfreserve(sfstdin,1,1))
		terror("Unexpected data\n");
	if(strcmp(Buf,HANDLER) != 0)
		terror("Handler wasn't called\n");
	if(!Except)
		terror("Exception handler wasn't called1\n");

	Buf[0] = 0;
	Except = 0;
	signal(SIGALRM,alrmhandler);
	alarm(2);
	if(sfgetr(sfstdin,'\n',0))
		terror("Unexpected data2\n");
	if(strcmp(Buf,HANDLER) != 0)
		terror("Handler wasn't called2\n");
	if(!Except)
		terror("Exception handler wasn't called2\n");

	TSTRETURN(0);
}
