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
 * AT&T Research
 *
 * write datagram
 */

#include "cslib.h"

ssize_t
csto(register Cs_t* state, int fd, const void* buf, size_t siz, Csaddr_t* addr)
{

#if CS_LIB_V10

	struct udpaddr	udp;

	udp.host = addr->addr[0];
	udp.port = addr->addr[1];
	if (cswrite(state, fd, &udp, sizeof(udp)) != sizeof(udp))
	{
		messagef((state->id, NiL, -1, "to: %d: hdr write error", fd));
		return -1;
	}
	return cswrite(state, fd, buf, siz);

#else

#if CS_LIB_SOCKET

	struct sockaddr_in	nam;

	memzero(&nam, sizeof(nam));
	nam.sin_family = AF_INET;
	nam.sin_addr.s_addr = addr->addr[0];
	nam.sin_port = addr->addr[1];
	return sendto(fd, buf, siz, 0, (struct sockaddr*)&nam, sizeof(nam));

#else

	errno = EINVAL;
	messagef((state->id, NiL, -1, "to: %d: not supported", fd));
	return -1;

#endif

#endif

}

ssize_t
_cs_to(int fd, const void* buf, size_t siz, Csaddr_t* addr)
{
	return csto(&cs, fd, buf, siz, addr);
}
