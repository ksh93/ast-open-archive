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
 * like csname() except `.' qualifications retained
 */

#include "cslib.h"

char*
csfull(register Cs_t* state, unsigned long addr)
{
	char*	s;

	state->flags |= CS_ADDR_FULL;
	s = csname(state, addr);
	state->flags &= ~CS_ADDR_FULL;
	messagef((state->id, NiL, -8, "full(%s) = %s", csntoa(state, addr), s));
	return s;
}

char*
_cs_full(unsigned long addr)
{
	return csfull(&cs, addr);
}
