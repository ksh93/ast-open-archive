/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include	"sftest.h"

/* This test checks to see if sfread() will always fill the buffer
** from a piped-stream correctly even if the other end of the pipe
** is writing odd amounts of bytes.
*/
#define RBUF	16
#define ITER	1024

main()
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
