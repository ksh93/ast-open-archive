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


static int	Count;
static int	Size;

#if __STD_C
ssize_t writef(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t writef(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	Count += 1;
	if((n % Size) != 0)
		terror("Wrong record size\n");
	return write(f->file,buf,n);
}

Sfdisc_t	Disc = {(Sfread_f)0, writef, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};

main()
{
	Sfio_t*	f;
	char	buf[550];
	int	i;
	char*	s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	Count = 0;
	Size = 52;

	if(!(f = sfopen(NIL(Sfio_t*), sftfile(0), "w")) )
		terror("Opening to write\n");
	sfsetbuf(f,buf,sizeof(buf));
	sfset(f,SF_WHOLE,1);
	sfdisc(f,&Disc);

	for(i = 0; i < 100; ++i)
		if(sfwrite(f,s,52) != 52)
			terror("sfwrite failed\n");
	sfclose(f);
	if(Count != 10)
		terror("Wrong number of writes1\n");

	Count = 0;
	Size = 53;

	if(!(f = sfopen(NIL(Sfio_t*), sftfile(0),"w")) )
		terror("Opening to write\n");
	sfsetbuf(f,buf,sizeof(buf));
	sfset(f,SF_WHOLE,1);
	sfdisc(f,&Disc);

	for(i = 0; i < 100; ++i)
		if(sfputr(f,s,'\n') != 53)
			terror("sfputr failed\n");
	sfclose(f);
	if(Count != 10)
		terror("Wrong number of writes2\n");

	sftcleanup();
	return 0;
}
