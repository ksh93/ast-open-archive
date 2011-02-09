/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
	int	n;

	if(pipe(Fd) < 0)
		terror("Can't make pipe\n");

	if(sfnew(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND,Fd[0],SF_READ) != sfstdin)
		terror("Can't renew stdin\n");
	sfset(sfstdin,SF_SHARE,1);

	if(sfpkrd(Fd[0],(Void_t*)buf,10,-1,1000,1) >= 0)
		terror("There isn't any data yet\n");

	if((n = sfpkrd(Fd[0],(Void_t*)buf,sizeof(buf),-1,0L,0)) >= 0)
		terror("Wrong data size %d, expecting < 0", n);

	if(write(Fd[1],"abcd",4) != 4)
		terror("Couldn't write to pipe");

	if((n = sfpkrd(Fd[0],(Void_t*)buf,sizeof(buf),-1,0L,0)) != 4)
		terror("Wrong data size %d, expecting 4", n);

	signal(SIGALRM,alarmhandler);
	alarm(2);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"01234") != 0)
		terror("Expecting 01234\n");

	if(sfstdin->_next < sfstdin->_endb)
		terror("Sfgetr read too much\n");

	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"56789") != 0)
		terror("Expecting 56789\n");

	TSTEXIT(0);
}
