/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"sftest.h"
#include	<signal.h>

MAIN()
{
	Sfio_t	*f;
	char	*s, *endos, *os = "one\ntwo\nthree\n";
	int	n;
	void(* handler)_ARG_((int));

	if(argc > 1)
	{	sfmove(sfstdin,sfstdout,(Sfoff_t)(-1),-1);
		return 0;
	}

	if(!(f = sfpopen((Sfio_t*)0, sfprints("%s -p > %s", argv[0], tstfile(0)), "w")))
		terror("Opening for write\n");
	if(sfwrite(f,os,strlen(os)) != (ssize_t)strlen(os))
		terror("Writing\n");

#ifdef SIGPIPE
	if((handler = signal(SIGPIPE,SIG_DFL)) == SIG_DFL)
		terror("Wrong signal handler\n");
	if((handler = signal(SIGPIPE,handler)) != SIG_DFL)
		terror("Weird signal handling");
#endif

	sfclose(f);

	if(!(f = sfpopen((Sfio_t*)0, sfprints("%s -p < %s", argv[0], tstfile(0)), "r")))
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

	TSTEXIT(0);
}
