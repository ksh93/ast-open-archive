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

#include "msglib.h"

#include <ctype.h>

/*
 * return call index given name
 * !isalnum() chars at end of name are ignored
 * 0 returned if not found
 */

int
msgindex(const char* name)
{
	register const char**	lo;
	register const char**	hi;
	register const char**	mid;
	register unsigned char*	n;
	register unsigned char*	m;
	register int		v;

 again:
	lo = msg_info.name;
	hi = msg_info.name + MSG_STD;
	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		n = (unsigned char*)name;
		m = (unsigned char*)*mid;
		for (;;)
		{
			if (!*m)
			{
				if (!isalnum(*n))
					return mid - msg_info.name;
				return 0;
			}
			if ((v = *n++ - *m++) > 0)
			{
				lo = mid + 1;
				break;
			}
			if (v < 0)
			{
				hi = mid - 1;
				break;
			}
		}
	}
	if (name[0] == 'f')
	{
		name++;
		goto again;
	}
	if (name[0] == 'i' && name[1] == 'p' && name[2] == 'c')
	{
		name += 3;
		goto again;
	}
	if (name[0] == 's' && name[1] == 'y' && name[2] == 'm')
	{
		name += 3;
		goto again;
	}
	return 0;
}
