/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * cs remote shell service
 * no tty/pty but good enough for nt
 * if its the only way in
 */

static const char id[] = "@(#)$Id: cs.rsh (AT&T Bell Laboratories) 1995-10-13 $\0\n";

#include <cs.h>
#include <proc.h>
#include <wait.h>

static int
svc_connect(void* handle, int fd, Cs_id_t* id, int clone, char** argv)
{
	Proc_t*		p;
	int		n;
	long		ops[4];

	static char*	args[] = { "sh", "-i", 0 };

	NoP(handle);
	NoP(clone);
	waitpid(-1, NiL, WNOHANG);
	n = 0;
	ops[n++] = PROC_FD_DUP(fd, 0, 0);
	ops[n++] = PROC_FD_DUP(fd, 1, 0);
	ops[n++] = PROC_FD_DUP(fd, 2, PROC_FD_CHILD);
	ops[n] = 0;
	if (!(p = procopen(NiL, args, NiL, ops, 0)))
		return(-1);
	procfree(p);
	csfd(fd, CS_POLL_CLOSE);
	return(0);
}

int
main(int argc, char** argv)
{
	NoP(argc);
	csserve(NiL, argv[1], NiL, NiL, svc_connect, NiL, NiL, NiL);
	exit(1);
}
