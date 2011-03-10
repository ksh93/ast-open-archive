/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
#include	<signal.h>

static int	line;

#define SYNC	line = __LINE__;

#if __STD_C
void alrmf(int sig)
#else
void alrmf(sig)
int	sig;
#endif
{
	terror("blocked at line %d\n", line);
}

MAIN()
{
	int	fd[2];
	Sfio_t	*fr, *fw;
	char	*s;
	int	i, j, n, to;
	char	buf[1024];

	to = argc > 1 ? 0 : 4;

	if(sfnew(sfstdout,buf,sizeof(buf),SF_UNBOUND,SF_STRING|SF_WRITE) != sfstdout)
		terror("Reopen sfstdout\n");

	if(pipe(fd) < 0)
		terror("Can't open pipe\n");
	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open stream\n");
	if(to)
	{	signal(SIGALRM,alrmf);
		alarm(4);
	}
	SYNC sfwrite(fw,"0123456789",10);
	if(sfread(fr,buf,10) != 10)
		terror("Can't read data from pipe\n");
	SYNC sfwrite(fw,"0123456789",10);
	SYNC if(sfmove(fr,fw,(Sfoff_t)10,-1) != 10)
		terror("sfmove failed\n");
	if(to)
		alarm(0);
	sfpurge(fw);
	sfclose(fw);
	sfpurge(fr);
	sfclose(fr);

	if(pipe(fd) < 0)
		terror("Can't open pipe2\n");
	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open stream\n");
	sfset(fr,SF_SHARE|SF_LINE,1);
	sfset(fw,SF_SHARE,1);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"1\n2\n3\n",6) != 6)
		terror("sfwrite failed\n");
	i = j = -1;
	SYNC if(sfscanf(fr,"%d%d\n%n",&i,&j,&n) != 2 || i != 1 || j != 2 || n != 4)
		terror("sfscanf failed\n");
	SYNC if(sfscanf(fr,"%d\n%n",&i,&n) != 1 || i != 3 || n != 2)
		terror("sfscanf failed\n");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"123\n",4) != 4)
		terror("sfwrite failed\n");
	SYNC if(!(s = sfreserve(fr,4,0)) )
		terror("sfreserve failed\n");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC sfputr(fw,"abc",'\n');
	SYNC if(sfmove(fr,fw,(Sfoff_t)1,'\n') != 1)
		terror("sfmove failed\n");
	SYNC if(!(s = sfgetr(fr,'\n',1)) || strcmp(s,"abc"))
		terror("sfgetr failed\n");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"111\n222\n333\n444\n",16) != 16)
		terror("Bad write to pipe\n");
	SYNC if(!(s = sfgetr(fr,'\n',1)) )
		terror("sfgetr failed\n");
	if(to)
		alarm(0);
	if(strcmp(s,"111"))
		terror("sfgetr got wrong string\n");

	if(to)
		alarm(4);
	SYNC if(sfmove(fr,sfstdout,(Sfoff_t)2,'\n') != 2)
		terror("sfmove failed\n");
	SYNC sfputc(sfstdout,0);
	SYNC if(strcmp("222\n333\n",buf))
		terror("sfmove got wrong data\n");
	SYNC if(sfmove(fr,NIL(Sfio_t*),(Sfoff_t)1,'\n') != 1)
		terror("sfmove failed\n");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"0123456789",11) != 11)
		terror("Bad write to pipe2\n");
	SYNC if(!(s = sfreserve(fr,11,0)) )
		terror("Bad peek size %d, expect 11\n",sfvalue(fr));
	if(to)
		alarm(0);
	if(strncmp(s,"0123456789",10))
		terror("Bad peek str %s\n",s);

	/* test for handling pipe error */
	if(pipe(fd) < 0)
		terror("Can't create pipe");
	close(fd[0]);
	if(!(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),sizeof(buf),fd[1],SF_WRITE)) )
		terror("Can't open stream");
	signal(SIGPIPE,SIG_IGN);

	for(i = 0; i < sizeof(buf); ++i)
		buf[i] = 'a';
	buf[sizeof(buf)-1] = 0;
	for(i = 0; i < 3; ++i)
	{	if(to)
		{	signal(SIGALRM,alrmf); /* do this to avoid infinite loop */
			alarm(4);
		}
		sfprintf(fw, "%s\n", buf); /* this should not block */
		if(to)
			alarm(0);
	}

	TSTEXIT(0);
}
