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

/*
 * 3d license via agent to license server
 */

#include "3d.h"

#if LICENSED

#define LICENSE_OPEN(s,m,l)	licopen(s,m,l)
#define LICENSE_CLOSE(f)	close(f)

static int
licopen(const char* svc, const char* msg, int len)
{
	char*	service;
	char*	env;
	int	n;
	int	fd;
	int	lic;
	char	buf[64];

	if (state.in_2d) return(-1);
	service = "/dev/fdp/local/nam/user";
	env = *environ;
	*environ = "_3D_DISABLE=1";
	if ((fd = cslocal(&cs, service)) < 0)
	{
		error(ERROR_SYSTEM|2, "%s: cannot connect to server", service);
		return(-1);
	}
	*environ = env;
	n = strlen(svc) + 1;
	if (cswrite(&cs, fd, svc, n) != n)
	{
		error(ERROR_SYSTEM|2, "%s: cannot write to server", service);
		close(fd);
		return(-1);
	}
	if (read(fd, buf, sizeof(buf)) <= 0)
	{
		error(ERROR_SYSTEM|2, "%s: cannot read from server", service);
		close(fd);
		return(-1);
	}
	if (csrecv(&cs, fd, NiL, &lic, 1) != 1)
	{
		error(ERROR_SYSTEM|2, "%s: cannot connect to %s", service, svc);
		close(fd);
		return(-1);
	}
	close(fd);
	if (cswrite(&cs, lic, msg, len) != len)
	{
		error(ERROR_SYSTEM|2, "%s: cannot write to service", svc);
		close(lic);
		return(-1);
	}
	fcntl(lic, F_SETFD, FD_CLOEXEC);
	state.in_2d = 0;
	return(lic);
}

#define tokscan		_3d_tokscan

#include "tokscan.c"
#include "../license/service.c"

#else

NoN(license)

#endif
