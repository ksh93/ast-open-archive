/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
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
 * ping host by name
 */

#include "cslib.h"

#define M	"yo\n"
#define N	(sizeof(M)-1)

int
csping(register Cs_t* state, const char* name)
{
	register int	fd;
	register int	n;

	sfsprintf(state->temp, sizeof(state->path), "/dev/tcp/%s/inet.echo", name);
	if ((fd = csopen(state, state->temp, 0)) < 0) return -1;
	n = (cswrite(state, fd, M, N) != N || csread(state, fd, state->temp, N, CS_LINE) != N || strncmp(M, state->temp, N)) ? -1 : 0;
	close(fd);
	if (n) messagef((state->id, NiL, -1, "ping: %s: no contact", name));
	return n;
}

int
_cs_ping(const char* name)
{
	return csping(&cs, name);
}
