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

#include "msglib.h"

#include <ctype.h>

/*
 * convert msg name list s to mask
 * *s=='!' inverts the mask
 */

unsigned long
msgsetmask(register const char* s)
{
	register int		c;
	register unsigned long m;
	int			invert;

	m = 0;
	invert = *s == '!';
	for (;;)
	{
		do if (!(c = *s++)) goto done; while (!isalnum(c));
		m |= MSG_MASK(msgindex(--s));
		do if (!(c = *s++)) goto done; while (isalnum(c));
	}
 done:
	return invert ? ~m : m;
}
