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
