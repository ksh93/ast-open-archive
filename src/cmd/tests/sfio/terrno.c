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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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

/* errnos tested for xopen-compliance */

MAIN()
{
	Sfio_t*	fw;
	Sfio_t*	fr;
	int	fds[2];

	if(!(fw = sfopen(NIL(Sfio_t*), tstfile(0), "w")) )
		terror("Can't create temp file %s to write", tstfile(0));
	if(!(fr = sfopen(NIL(Sfio_t*), tstfile(0), "r")) )
		terror("Can't open temp file %s to read", tstfile(0));

	sfseek(fr, (Sfoff_t)0, SEEK_END);
	if(sfgetc(fr) >= 0 || !sfeof(fr))
		terror("Should have seen eof");

	if(sfwrite(fr, "a", 1) == 1)
		terror("sfwrite should have failed on a read stream");
	if(errno != EBADF)
		terror("Wrong errno after sfwrite failed");

	sfseek(fw, (Sfoff_t)(-1), SEEK_SET);
	if(errno != EINVAL)
		terror("Wrong errno after sfseek to -1");

	errno = 0;
	if(sfseek(fw, (Sfoff_t)0, SEEK_SET|SEEK_CUR|SEEK_END) >= 0)
		terror("sfseek should not have succeeded");
	if(errno != EINVAL)
		terror("Wrong errno after bad sfseek call");

	if(pipe(fds) < 0)
		terror("Can't create pipes");

	if(!(fw = sfnew(fw, NIL(Void_t*), -1, fds[1], SF_WRITE)) )
		terror("Can't create stream for pipe");

	if(sfseek(fw, (Sfoff_t)0, SEEK_SET) >= 0)
		terror("sfseek should have failed on a pipe");
	if(errno != ESPIPE)
		terror("Wrong errno after sfseek failed");

	close(sffileno(fw));
	errno = 0;
	if(sfseek(fw, (Sfoff_t)0, SEEK_END) >= 0)
		terror("sfseek should have failed on a closed file descriptor");
	if(errno != EBADF)
		terror("Wrong errno after sfseek failed");

	TSTRETURN(0);
}
