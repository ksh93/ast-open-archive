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
/* see whether you can use ioctl() to get window size */

#include	<sys/types.h>

#ifdef TERMIOS
#	include	<termios.h>
#endif
#ifdef TERMIO
#	include	<termio.h>
#endif
#ifdef SGTTY
#	include	<sgtty.h>
#endif
#ifdef STREAM
#	include	<sys/stream.h>
#endif
#ifdef PTEM
#	include	<sys/ptem.h>
#endif
#ifdef JIOCTL
#	include	<sys/jioctl.h>
#	define winsize		jwinsize
#	define ws_col		bytesx
#	define TIOCGWINSZ	JWINSIZE
#endif

main()
{
	struct winsize size;
	int fd = open("/dev/tty",0);
	ioctl(fd, TIOCGWINSZ, &size);
	printf("#define WINSIZE	1\n");
#ifdef JIOCTL
	printf("#define _sys_jioctl_	1\n");
#endif /* JIOCTL */
#ifdef PTEM
	printf("#define _sys_ptem_	1\n");
#endif /* PTEM */
#ifdef STREAM
	printf("#define _sys_stream_	1\n");
#endif /* STREAM */
	exit(0);
}
