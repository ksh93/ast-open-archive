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
#include	<signal.h>

main()
{
	Sfio_t	*f;
	char	*s, *endos, *os = "one\ntwo\nthree\n";
	int	n;

	if(!(f = sfpopen((Sfio_t*)0, sfprints("cat > %s",sftfile(0)), "w")))
		terror("Opening for write\n");
	if(sfwrite(f,os,strlen(os)) != (ssize_t)strlen(os))
		terror("Writing\n");

#ifdef SIGPIPE
	{	void(* handler)_ARG_((int));
		if((handler = signal(SIGPIPE,SIG_DFL)) == SIG_DFL)
			terror("Wrong signal handler\n");
		signal(SIGPIPE,handler);
	}
#endif

	sfclose(f);

#ifdef SIGPIPE
	{	void(* handler)_ARG_((int));
		if((handler = signal(SIGPIPE,SIG_DFL)) != SIG_DFL)
			terror("Wrong signal handler2\n");
		signal(SIGPIPE,handler);
	}
#endif

	if(!(f = sfpopen((Sfio_t*)0, sfprints("cat < %s",sftfile(0)), "r")))
		terror("Opening for read\n");
	sleep(1);

	endos = os + strlen(os);
	while(s = sfgetr(f,'\n',0))
	{	n = sfvalue(f);
		if(strncmp(s,os,n) != 0)
		{	s[n-1] = os[n-1] = 0;
			terror("Input=%s, Expect=%s\n",s,os);
		}
		os += n;
	}

	if(os != endos)
		terror("Does not match all data, left=%s\n",os);

	sftcleanup();
	return 0;
}
