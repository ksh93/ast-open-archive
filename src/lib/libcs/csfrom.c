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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * read datagram
 */

#include "cslib.h"

ssize_t
csfrom(register Cs_t* state, int fd, void* buf, size_t siz, Csaddr_t* addr)
{

#if CS_LIB_V10

	ssize_t		n;
	struct udpaddr	udp;

	if (read(fd, &udp, sizeof(udp)) != sizeof(udp))
	{
		messagef((state->id, NiL, -1, "from: %d: udp header read error", fd));
		return -1;
	}
	if (addr)
	{
		addr->addr[0] = udp.host;
		addr->addr[1] = udp.port;
		addr->addr[2] = 0;
	}
	if ((n = read(fd, buf, siz)) < 0) messagef((state->id, NiL, -1, "from: %d: udp data read error", fd));
	else messagef((state->id, NiL, -8, "from(%d,*,%d) = %d, data = `%-.*s', addr = %s, port = %u", fd, siz, n, n, buf, csntoa(state, udp.host), udp.port));
	return n;

#else

#if CS_LIB_SOCKET

	ssize_t			n;
	Sock_size_t		len;
	struct sockaddr_in	nam;

	len = sizeof(nam);
	if ((n = recvfrom(fd, buf, siz, 0, (struct sockaddr*)&nam, &len)) < 0)
	{
		messagef((state->id, NiL, -1, "from: %d: recvfrom error", fd));
		return -1;
	}
	if (addr)
	{
		addr->addr[0] = (unsigned long)nam.sin_addr.s_addr;
		addr->addr[1] = (unsigned long)nam.sin_port;
	}
	messagef((state->id, NiL, -8, "from(%d,*,%d) = %d, data = `%-.*s', addr = %s, port = %u", fd, siz, n, n, buf, csntoa(state, (unsigned long)nam.sin_addr.s_addr), (unsigned long)nam.sin_port));
	return n;

#else

	errno = EINVAL;
	messagef((state->id, NiL, -1, "from: %d: udp read not supported", fd));
	return -1;

#endif

#endif

}

ssize_t
_cs_from(int fd, void* buf, size_t siz, Csaddr_t* addr)
{
	return csfrom(&cs, fd, buf, siz, addr);
}
