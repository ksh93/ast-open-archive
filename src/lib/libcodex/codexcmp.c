/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 2003-2004 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * codex method name comparison
 * strcmp() semantics
 */

#include <codex.h>

int
codexcmp(register const char* s, register const char* t)
{
	for (;;)
	{
		if (!*s)
		{
			if (!*t || *t == '-' || *t == '+')
				return 0;
			break;
		}
		if (!*t)
		{
			if (*s == '-' || *s == '+')
				return 0;
			break;
		}
		if (*s != *t)
			break;
		s++;
		t++;
	}
	return *((unsigned char*)s) - *((unsigned char*)t);
}
