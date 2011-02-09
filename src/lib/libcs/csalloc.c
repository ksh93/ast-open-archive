/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
