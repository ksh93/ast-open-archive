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
 * allocate cs state context
 */

#include "cslib.h"

Cs_t*
csalloc(Csdisc_t* disc)
{
	register Cs_t*	state;

	if (state = newof(0, Cs_t, 1, 0))
	{
		state->id = cs.id;
		state->disc = disc ? disc : cs.disc;
#if _stream_peek
		state->nostream = -1;
#endif
#if _socket_peek
		state->nosocket = -1;
#endif
	}
	return state;
}
