/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

#include <ast_lib.h>

#include <sys/types.h>

#if _stream_peek
#include <ast_tty.h>
#include <stropts.h>
#endif

#if _socket_peek
#include <sys/socket.h>
#endif

extern int	pipe(int*);
extern int	printf(const char*, ...);
extern int	write(int, void*, int);

main()
{
	int		fds[2];
	static char	msg[] = "test";
#if _stream_peek
	struct strpeek	pk;
#endif

	if (pipe(fds) || write(fds[1], msg, sizeof(msg)) != sizeof(msg))
		return(0);
#if _stream_peek
	pk.flags = 0;
	pk.ctlbuf.maxlen = -1;
	pk.ctlbuf.len = 0;
	pk.ctlbuf.buf = 0;
	pk.databuf.maxlen = sizeof(msg);
	pk.databuf.buf = msg;
	pk.databuf.len = 0;
	if (ioctl(fds[0], I_PEEK, &pk) > 0 && pk.databuf.len == sizeof(msg))
	{
		printf("#undef	_socket_peek\n");
		return(0);
	}
#endif
#if _socket_peek
	if (recv(fds[0], msg, sizeof(msg), MSG_PEEK) == sizeof(msg))
	{
		printf("#undef	_stream_peek\n");
		return(0);
	}
#endif
	return(0);
}
