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

/* This test checks to see if sfread() will always fill the buffer
** from a piped-stream correctly even if the other end of the pipe
** is writing odd amounts of bytes.
*/
#define RBUF	16
#define ITER	1024

MAIN()
{
	Sfio_t		*fr;
	int		p[2];
	char		wbuf[1023], rbuf[RBUF*1023], *s;
	int		i, r, n;

	if(pipe(p) < 0 )
		terror("Making pipe for communication");

	if(!(fr = sfnew(0, 0, (size_t)SF_UNBOUND, p[0], SF_READ)) )
		terror("Making read stream");

	for(i = 0; i < sizeof(wbuf); ++i)
		wbuf[i] = (i%10) + '0';

	switch(fork())
	{
		case -1 :
			terror("fork() failed");
		case 0 :
			for(i = 0; i < RBUF*ITER; ++i)
				if(write(p[1], wbuf, sizeof(wbuf)) != sizeof(wbuf))
					terror("Write to pipe failed i=%d", i);
			break;
		default:
			for(i = 0; i < ITER; ++i)
			{	if(sfread(fr, rbuf, sizeof(rbuf)) != sizeof(rbuf))
					terror("Read from pipe failed i=%d", i);
				for(r = 0, s = rbuf; r < RBUF; r += 1, s += n)
				for(n = 0; n < sizeof(wbuf); ++n)
					if(s[n] != (n%10)+'0')
						terror("Bad data i=%d n=%d", i, n);
			}
			break;
	}

	TSTEXIT(0);
}
