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
 * note host status change for name
 * this is the daemon side of csstat()
 *
 * NOTE: name must already exist and pwd must be CS_STAT_DIR
 */

#include "cslib.h"

/*
 * encode 4 byte int into 2 bytes
 */

static unsigned short
encode(register unsigned long n)
{
	register int	e;

	e = 0;
	while (n > 03777)
	{
		n >>= 1;
		e++;
	}
	return n | (e << 11);
}

int
csnote(register Cs_t* state, const char* name, register Csstat_t* sp)
{
	unsigned long	idle;
	long		up;

	if (sp->up < 0)
	{
		idle = -sp->up;
		up = 0;
	}
	else
	{
		idle = sp->idle;
		up = sp->up;
	}
	return touch(name, (encode(up) << 16) | encode(idle), (((sp->load >> 3) & 0377) << 24) | ((sp->pctsys & 0377) << 16) | ((sp->pctusr & 0377) << 8) | (sp->users & 0377), -1);
}

int
_cs_note(const char* name, Csstat_t* sp)
{
	return csnote(&cs, name, sp);
}
