/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                         Pat Sullivan                         *
*                                                              *
***************************************************************/
/*
 * This program tests whether fcntl exists on the system and
 * does what the shell requires it to do
 * Also, if R_OK is defined in <fcntl.h>, prints define R_OK_fcntl_h
 */

#ifdef _fcntl_
#  include <fcntl.h>
#else
#  define F_DUPFD	0
#endif

main()
{
	int fn;
	close(6);
	fn =  fcntl(0, F_DUPFD, 6);
	if(fn < 6)
		printf("#define NOFCNTL\t1\n");
#ifdef R_OK
	else
		printf("#define R_OK_fcntl_h\t1\n");
#endif /* R_OK */
}
