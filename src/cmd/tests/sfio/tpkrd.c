/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
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
*******************************************************************/
#include	"sftest.h"
#include	<signal.h>

static int	Fd[2];

#if __STD_C
void alarmhandler(int sig)
#else
void alarmhandler(sig)
int	sig;
#endif
{
	if(write(Fd[1],"01234\n56789\n",12) != 12)
		terror("Writing to pipe\n");
}

MAIN()
{
	char*	s;
	char	buf[1024];

	if(pipe(Fd) < 0)
		terror("Can't make pipe\n");

	if(sfnew(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND,Fd[0],SF_READ) != sfstdin)
		terror("Can't renew stdin\n");
	sfset(sfstdin,SF_SHARE,1);

	if(sfpkrd(Fd[0],(Void_t*)buf,10,-1,1000,1) >= 0)
		terror("There isn't any data yet\n");

	signal(SIGALRM,alarmhandler);
	alarm(2);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"01234") != 0)
		terror("Expecting 01234\n");

	if(sfstdin->next < sfstdin->endb)
		terror("Sfgetr read too much\n");

	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"56789") != 0)
		terror("Expecting 56789\n");

	TSTEXIT(0);
}
