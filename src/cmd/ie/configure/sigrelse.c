/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                         Pat Sullivan                         *
*                                                              *
***************************************************************/
/*
 * See if sigrelse is needed to exit from signal handler
 */

#include	<sys/types.h>
#include	<signal.h>
#ifdef SIGBLOCK
# define sigrelse(s)	sigsetmask(sigblock(0)&~(1<<((s)-1)))
#endif

int handler();
int timeout();

main()
{
	int parent;
	int status;
	if((parent=fork())==0)
	{
		signal(SIGQUIT,handler);
		signal(SIGALRM,timeout);
		/* set a timeout */
		alarm(5);
		/* send a SIQUIT to myself */
		kill(getpid(),SIGQUIT);
		pause();
		exit(0);
	}
	else if(parent < 0)
		exit(1);
	wait(&status);
	if((status&077)==SIGQUIT)
	{
#ifdef SIGBLOCK
		printf("#define sigrelease(s)	sigsetmask(sigblock(0)&~(1<<((s)-1)))\n");
		printf("#define sig_begin()	(sigblock(0),sigsetmask(0))\n");
#else
		printf("#define sig_begin()\n");	
#   ifdef sigrelse
		printf("#define sigrelease(s)\n");	
#   else
		printf("#define sigrelease	sigrelse\n");
#   endif sigrelease
#endif
		exit(0);
	}
	exit(1);
}

handler(sig)
{
	/* reset the signal handler to default */
	signal(sig, SIG_DFL);
	sigrelse(sig);
	kill(getpid(),SIGQUIT);
	pause();
	exit(0);
}

timeout(sig)
{
	exit(1);
}
