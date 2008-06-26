/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
