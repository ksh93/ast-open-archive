/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
 * create bi-directional local pipe
 */

#include "cslib.h"

int
cspipe(Cs_t* state, int* fds)
{

	csprotect(&cs);

#if CS_LIB_SOCKET_UN

	return socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

#else

	return pipe(fds);

#endif

}

int
_cs_pipe(int* fds)
{
	return cspipe(&cs, fds);
}
