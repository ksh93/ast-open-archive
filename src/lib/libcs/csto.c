/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
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
