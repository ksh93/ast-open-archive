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
 * return the string representation of addr
 */

#include "cslib.h"

char*
csntoa(register Cs_t* state, unsigned long addr)
{
	register unsigned char*	p;
	int_4			a;

	a = addr;
	p = (unsigned char*)&a;
	if ((!addr || p[0] == 127 && p[1] == 0 && p[2] == 0 && p[3] <= 1) && !state->ntoa[sizeof(state->ntoa)-1])
	{
		state->ntoa[sizeof(state->ntoa)-1] = 1;
		addr = csaddr(state, NiL);
		state->ntoa[sizeof(state->ntoa)-1] = 0;
	}
	sfsprintf(state->ntoa, sizeof(state->ntoa), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	messagef((state->id, NiL, -8, "ntoa(%s) call", state->ntoa));
	return state->ntoa;
}

char*
_cs_ntoa(unsigned long addr)
{
	return csntoa(&cs, addr);
}
