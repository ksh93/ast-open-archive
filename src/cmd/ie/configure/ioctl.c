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
 * see whether <termios.h> or < termio.h> has a chance of working
 */

#ifdef TERMIOS
#   include	<termios.h>
#   define termio termios
#   ifndef	TCSANOW
#	define TCSANOW	TCSETS
#   endif
#else
#   include	<termio.h>
#endif /* TERMIOS */

main()
{
	int fd;
	struct termio term;
#ifdef TERMIOS
	fd = TCSANOW;
#else
	if ((fd=open("/dev/tty",0)) < 0)
		exit(1);
#   ifdef VEOL2
	if(ioctl(fd,TCGETA,&term) < 0)
#   endif /* VEOL2 */
	/* don't use termio if no VEOL2 or if TCGETA doesn't work */
	{
		printf("#undef _termio_\n");
		printf("#undef _sys_termio_\n");
		exit(0);
	}
#endif /* TERMIOS */
	exit(0);
}
	
