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

/*	Test for /dev/null and hole-preserving code */

main()
{
	Sfio_t*	null;
	Sfio_t*	f;
	char	buf[256*1024], b[256*1024];
	int	k, n;

	if(!(null = sfopen(NIL(Sfio_t*),"/dev/null","w")) )
		terror("Opening /dev/null");

	sfsetbuf(null,NIL(char*),(size_t)SF_UNBOUND);

	if(!SFISNULL(null) )
		terror("Not /dev/null?");

	if(!(f = sfopen(NIL(Sfio_t*), sftfile(0), "w+")) )
		terror("Creating %s", sftfile(0));
	sfwrite(f,"1234",4);
	sfseek(f,(Sfoff_t)1,0);
	sfsync(f);

	sfsetfd(null,-1);
	sfsetfd(null,sffileno(f));
	sfsync(null);

	sfseek(f,(Sfoff_t)0,0);
	if(sfread(f,buf,4) != 4 || strncmp(buf,"1234",4) != 0)
		terror("Bad data");

	for(k = 0; k < sizeof(buf); ++k)
		buf[k] = 1;
	for(k = sizeof(buf)/4; k < sizeof(buf)/2; ++k) /* make a big hole */
		buf[k] = 0;

	if(!(f = sfopen(f, sftfile(0), "w+")) )
		terror("Creating %s", sftfile(0));
	n = sizeof(buf)-127;
	if(sfwrite(f,buf,n) != n)
		terror("Writing large buffer");
	sfseek(f,(Sfoff_t)0,0);
	if(sfread(f,b,n) != n)
		terror("Reading large buffer");
	for(k = 0; k < n; ++k)
		if(b[k] != buf[k])
			terror("Bad data");

	sftcleanup();
	return 0;
}
