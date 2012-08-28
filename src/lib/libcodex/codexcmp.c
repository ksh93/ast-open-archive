/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
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
			if (!*t || *t == '-' || *t == '+' || *t == '<' || *t == '>' || *t == '|' || *t == '^')
				return 0;
			break;
		}
		if (!*t)
		{
			if (*s == '-' || *s == '+' || *s == '<' || *s == '>' || *s == '|' || *s == '^')
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
