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

#if __STD_C
ssize_t myread(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t myread(f, buf, n, disc)
Sfio_t*	f;
Void_t*	buf;
size_t	n;
Sfdisc_t* disc;
#endif
{
	return sfrd(f,buf,n,disc);
}

Sfdisc_t Disc = {myread, NIL(Sfwrite_f), NIL(Sfseek_f), NIL(Sfexcept_f)};

MAIN()
{
	int	fd[2];

	if(pipe(fd) < 0)
		terror("Can't open pipe\n");

	if(sfnew(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ) != sfstdin)
		terror("Can't initialize sfstdin\n");
	sfset(sfstdin,SF_SHARE,1);
	sfdisc(sfstdin,&Disc);

	if(sfnew(sfstdout,NIL(Void_t*),0,fd[1],SF_WRITE) != sfstdout)
		terror("Can't initialize sfstdout\n");
	sfputr(sfstdout,"111\n222\n333\n",-1);
	sfsync(sfstdout);

	if(strcmp(sfgetr(sfstdin,'\n',1),"111") != 0)
		terror("sfgetr failed1\n");
	if(sfstdin->_endb > sfstdin->_next)
		terror("sfgetr reads too much1\n");

	if(strcmp(sfgetr(sfstdin,'\n',1),"222") != 0)
		terror("sfgetr failed2\n");
	if(sfstdin->_endb > sfstdin->_next)
		terror("sfgetr reads too much2\n");

	TSTEXIT(0);
}
